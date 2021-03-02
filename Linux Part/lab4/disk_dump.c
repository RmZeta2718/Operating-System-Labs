#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "defrag.h"

#define print_number(number) printf("\033[32m"#number": \033[39m%d\n", number)
#define print_highlight(str) printf("\033[32m%s\033[39m\n", str)


SUPERBLOCK *super;
char *disk;

// Given block index in data region, return block address on disk
static inline char* idx2addr(int block_idx) {
    return disk + BASE + ((super->data_offset + block_idx) * super->size);
}

static void print_file(INODE *in);
static void print_file_iblock(INODE *in);

int main(int argc, char *argv[]) {
    FILE *f = fopen(argv[1], "r");
    assert(f);
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    // print_number(size);

    disk = malloc(size);
    assert(disk);
    
    fread(disk, size, 1, f);
    super = (SUPERBLOCK*)(disk + BOOT_SIZE);
    // print_number(super->free_iblock);
    // print_number(size / super->size);

    // walk free block list
    // int idx_free_block = super->free_iblock;
    // do {
    //     print_number(*(int*)idx2addr(idx_free_block));
    //     idx_free_block = *(int*)idx2addr(idx_free_block);
    // } while (idx_free_block != -1);

    INODE *inode;
    inode = (INODE*)(disk + BASE + super->inode_offset);
    int inode_cnt = (super->data_offset - super->inode_offset) * super->size / sizeof(INODE);
    // print_number(inode_cnt);
    for (int file_idx = 0; file_idx < inode_cnt; ++file_idx) {
        if (!inode[file_idx].nlink) continue;
        print_number(file_idx);
        print_number(inode[file_idx].size);
        print_file(&inode[file_idx]);
        // print_file_iblock(&inode[file_idx]);
    }

    return 0;
}

static int print_data(char *block) {
    for (int i = 0; i < super->size; ++i)
        putchar(block[i]);
    puts("");
    return 1;
}

static int print_all_data(int level, int idx, int tot) {
    if (!level)
        return print_data(idx2addr(idx));
    int *iblock = (int*)idx2addr(idx);
    int cnt = 0;
    for (int i = 0; i < super->size / sizeof(int); ++i) {
        if (cnt >= tot) break;
        cnt += print_all_data(level - 1, iblock[i], tot - cnt);
    }
    return cnt;
}

static void print_file(INODE *in) {
    int cnt = 0, tot = (in->size + super->size - 1) / super->size;
    print_highlight("dblocks:");
    for (int i = 0; i < N_DBLOCKS; ++i) {
        if (cnt >= tot) break;
        print_number(i);
        cnt += print_all_data(0, in->dblocks[i], tot - cnt);
    }
    print_highlight("iblocks:");
    for (int i = 0; i < N_IBLOCKS; ++i) {
        if (cnt >= tot) break;
        print_number(i);
        cnt += print_all_data(1, in->iblocks[i], tot - cnt);
    }
    print_highlight("i2block:");
    cnt += print_all_data(2, in->i2block, tot - cnt);
    print_highlight("i3block:");
    cnt += print_all_data(3, in->i3block, tot - cnt);
    assert(cnt == tot);
    puts("\n");
}

static int print_iblock(int *block) {
    for (int i = 0; i < super->size / sizeof(int); ++i)
        printf("%d ", block[i]);
    puts("");
    return super->size / sizeof(int);
}

static int print_all_iblock(int level, int idx, int tot) {
    if (tot <= 0) return 0;
    if (level == 1)
        return print_iblock((int*)idx2addr(idx));
    int *iblock = (int*)idx2addr(idx);
    int cnt = 0;
    for (int i = 0; i < super->size / sizeof(int); ++i) {
        if (cnt >= tot) break;
        cnt += print_all_iblock(level - 1, iblock[i], tot - cnt);
    }
    print_iblock(iblock);
    return cnt;
}

static void print_file_iblock(INODE *in) {
    int cnt = N_DBLOCKS, tot = (in->size + super->size - 1) / super->size;
    print_highlight("iblocks:");
    for (int i = 0; i < N_IBLOCKS; ++i) {
        if (cnt >= tot) break;
        print_number(i);
        cnt += print_all_iblock(1, in->iblocks[i], tot - cnt);
    }
    print_highlight("i2block:");
    cnt += print_all_iblock(2, in->i2block, tot - cnt);
    print_highlight("i3block:");
    cnt += print_all_iblock(3, in->i3block, tot - cnt);
    assert(cnt >= tot);
    puts("\n");
}