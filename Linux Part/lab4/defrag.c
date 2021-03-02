#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defrag.h"

// A fancy way to print a number with its name
// #define DEBUG
#ifdef DEBUG
#define debug(number) printf("\033[32m"#number": \033[39m%d\n", number)
// #define debug(number) printf(#number": %d\n", number)
#else
#define debug(x) do {} while (0)
#endif

// A fancy way to print warnings and errors with color
#define printfw(args...) \
    do { \
        printf("\033[35;1mdefrag warning: \033[39;0m"); \
        printf(args); \
        puts(""); \
    } while (0)
#define printfe(args...) \
    do { \
        printf("\033[31;1mdefrag error: \033[39;0m"); \
        printf(args); \
        puts(""); \
    } while (0)

#define MAXLEN 100  // max file name length

FILE *fin, *fout;
SUPERBLOCK *super;
int block_shift;    // idx << block_shift == idx * block_size
int block_tot;      // data block number (swap region ommited)
void *buffer;       // buffer for random data
int *iblock[4];     // buffer for indirect block
int ib_tot_item;    // #int contains in a indirect block

// wrapper functions to manipulate data blocks according to inode.
static int copy_data(INODE *in);
static int write_inode(INODE *in, const int idx_data, int idx_ib);

int main(int argc, char *argv[]) {
    char src_disk[MAXLEN];
    char dst_disk[MAXLEN];

    if (argc != 2) {
        printf("Usage: ./defrag <fragmented disk file>\n");
        return 1;
    }

    // open files
    strcpy(src_disk, argv[1]);
    fin = fopen(src_disk, "r");
    if (!fin) {
        printfe("Cannot open source disk");
        perror("");
        return 1;
    }
    fseek(fin, 0, SEEK_END);        // goto end of file
    int disk_size = ftell(fin);     // in bytes
    debug(disk_size);
    fseek(fin, 0, SEEK_SET);        // back to the beginning of file
    strcpy(dst_disk, argv[1]);
    strcat(dst_disk, "-defrag");
    // try open without create
    fout = fopen(dst_disk, "r");
    if (fout) {
        printfw("Output file [%s] already exists. "
                "It will be truncated.", dst_disk);
        fclose(fout);
    }
    fout = fopen(dst_disk, "w+");

    // copy bootblock
    buffer = malloc(BOOT_SIZE);
    fread(buffer, BOOT_SIZE, 1, fin);
    fwrite(buffer, BOOT_SIZE, 1, fout);
    free(buffer);

    // read superblock
    super = malloc(SUPER_SIZE);
    fread(super, SUPER_SIZE, 1, fin);
    fwrite(super, SUPER_SIZE, 1, fout);     // expand fout size
    if (__builtin_popcount(super->size) != 1) {
        // popcount counts number of 1-bits
        printfe("Block size (%d) is not power of 2.", super->size);
        return 1;
    }
    if (disk_size % super->size) {
        printfw("Disk size is not block-aligned.\n"
            "Something might be wrong, but defrag will run normally.");
    }
    // calculate global constants associate with block size
    ib_tot_item = super->size / sizeof(int);
    debug(ib_tot_item);
    // clz counts leading zeros
    block_shift = 31 - __builtin_clz(super->size);
    debug(block_shift);
    block_tot = ((disk_size - BASE) >> block_shift) - super->data_offset;
    debug(block_tot);
    // see what's in superblock
    debug(super->size);
    debug(super->inode_offset);
    debug(super->data_offset);
    debug(super->swap_offset);
    debug(super->free_inode);
    debug(super->free_iblock);
    debug(disk_size / super->size);
    // TODO: larger than disk size?
    // debug((super->swap_offset << block_shift) + BASE);

    // read inode region
    int inode_size = (super->data_offset - super->inode_offset) << block_shift;
    INODE *inode = malloc(inode_size);
    fread(inode, inode_size, 1, fin);
    fwrite(inode, inode_size, 1, fout);     // expand fout size

    // walk through file inodes, copy data, update inodes
    // allocate buffers
    buffer = malloc(super->size);
    iblock[1] = malloc(super->size);
    iblock[2] = malloc(super->size);
    iblock[3] = malloc(super->size);
    int inode_tot = inode_size / sizeof(INODE);     // round down
    int cur = 0;        // current data block index
    for (int i = 0; i < inode_tot; ++i) {
        // see what's in inode
        // debug(i);
        // if (!inode[i].size) debug(inode[i].next_inode);
        // debug(inode[i].size);
        // debug(inode[i].dblocks[0]);
        // debug(inode[i].dblocks[1]);
        // if (inode[i].size > N_DBLOCKS << block_shift)
        //     debug(inode[i].iblocks[0]);

        if (!inode[i].nlink) continue;  // unused inode
        int data_cnt = copy_data(&inode[i]);
        int ib_cnt = write_inode(&inode[i], cur, cur + data_cnt);
        cur += data_cnt + ib_cnt;
    }

    // fill the rest of the disk with free blocks
    // cur to the end is free blocks
    if (cur >= block_tot) {
        // no free blocks left
        super->free_iblock = -1;
    } else {
        super->free_iblock = cur;
        // free block is filled with EMPTY_DATA (defined in defrag.h)
        memset(buffer, EMPTY_DATA, super->size);
        for (; cur < block_tot - 1; ++cur) {
            // first 4 bytes of buffer is index of next free block
            *(int*)buffer = cur + 1;
            fwrite(buffer, super->size, 1, fout);
        }
        // the last block is -1
        *(int*)buffer = -1;
        fwrite(buffer, super->size, 1, fout);
    }
    
    // update superblock and inode region
    fseek(fout, BOOT_SIZE, SEEK_SET);
    fwrite(super, SUPER_SIZE, 1, fout);
    // fseek(fout, BASE + (super->inode_offset << block_shift), SEEK_SET);
    fwrite(inode, inode_size, 1, fout);

    // clean up
    fclose(fin);
    fclose(fout);
    free(super);
    free(inode);
    free(buffer);
    free(iblock[1]);
    free(iblock[2]);
    free(iblock[3]);

    return 0;
}

static int copy_block(int level, int idx, int tot);

// Find all data blocks of inode in fin, write into fout.
// Return number of data blocks written.
static int copy_data(INODE *in) {
    // number of blocks contained in file *in (round up)
    int tot = (in->size + super->size - 1) >> block_shift;
    // debug(tot);
    int cnt = 0;
    for (int i = 0; i < N_DBLOCKS; ++i)
        cnt += copy_block(0, in->dblocks[i], tot - cnt);
    for (int i = 0; i < N_IBLOCKS; ++i)
        cnt += copy_block(1, in->iblocks[i], tot - cnt);
    cnt += copy_block(2, in->i2block, tot - cnt);
    cnt += copy_block(3, in->i3block, tot - cnt);
    // if (cnt != tot) printfe("not clean");
    return cnt;
}

static int write_iblock(int level, int *pidx_data, int end_data, int *pidx_ib);

/* Update block index in inode. Write iblocks (if need) to fout.
Assume that data (in fout) start from `idx_data` and occupies a
    continuous region until `idx_ib`.
Return the number of indirect blocks written.
*/
static int write_inode(INODE *in, int idx_data, int idx_ib) {
    int end_data = idx_ib;
    for (int i = 0; i < N_DBLOCKS; ++i)
        in->dblocks[i] = write_iblock(0, &idx_data, end_data, &idx_ib);
    for (int i = 0; i < N_IBLOCKS; ++i) 
        in->iblocks[i] = write_iblock(1, &idx_data, end_data, &idx_ib);
    in->i2block = write_iblock(2, &idx_data, end_data, &idx_ib);
    in->i3block = write_iblock(3, &idx_data, end_data, &idx_ib);
    return idx_ib - end_data;
}

// Given block index in data region, return block address on disk
static inline int idx2addr(int block_idx) {
    return BASE + ((super->data_offset + block_idx) << block_shift);
}
// Copy a direct data block
// Return the number of blocks written (which is always 1)
static int copy_db(int db_idx) {
    fseek(fin, idx2addr(db_idx), SEEK_SET);
    fread(buffer, super->size, 1, fin);
    fwrite(buffer, super->size, 1, fout);
    return 1;
}

/* This function copy data block.
If a direct block (level == 0) is given, it is read from fin
    and written to the end of fout (by calling copy_db).
If a indirect block (level > 0) is given, it is processed
    recursively. Note that the iblock is NOT copied, ONLY
    direct data blocks are copied (in order). iblocks will
    be copied in the other function.
`idx` is the index of the dblock or iblock in data region
At most `tot` blocks will be copied, that is, if `tot` is
    less than blocks governed by iblock, only `tot` blocks
    will be copied; otherwise, all blocks in iblock will be
    copied.
Return the number of blocks written.
*/
static int copy_block(int level, int idx, int tot) {
    if (tot <= 0)
        return 0;       // block unused
    if (level == 0)
        return copy_db(idx);     // direct block
    // read indirect block
    fseek(fin, idx2addr(idx), SEEK_SET);
    fread(iblock[level], super->size, 1, fin);
    int cnt = 0;
    for (int i = 0; i < ib_tot_item; ++i)
        cnt += copy_block(level - 1, iblock[level][i], tot - cnt);
    // if (cnt > tot) printfe("cnt > tot");
    return cnt;
}

/* This function write iblock to fout
idx_data and idx_ib is maintained during the process and always
    point to one after current data/iblock (ie. end iterator).
    In C++, we can pass reference, which will make the code prettier.
    Here in C, we do this trick in an old-fashioned way.
Return the index of the iblock
*/
static int write_iblock(int level, int *pidx_data, const int end_data, int *pidx_ib) {
    if (*pidx_data >= end_data)
        return EMPTY_DATA;      // iblock unused
    if (level == 0)
        return (*pidx_data)++;      // dblock, return data idx
    // fill in iblock recursively
    for (int i = 0; i < ib_tot_item; ++i)
        iblock[level][i] = write_iblock(level - 1, pidx_data, end_data, pidx_ib);
    fwrite(iblock[level], super->size, 1, fout);
    return (*pidx_ib)++;
}
