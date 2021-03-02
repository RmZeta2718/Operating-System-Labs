#ifndef __DEFRAG_H__
#define __DEFRAG_H__

#define BOOT_SIZE 512
#define SUPER_SIZE 512
#define BASE 1024           // BASE = BOOT_SIZE + SUPER_SIZE

typedef struct {
    int size; /* size of blocks in bytes */
    int inode_offset; /* offset of inode region in bytes blocks */
    int data_offset; /* data region offset in blocks */
    int swap_offset; /* swap region offset in blocks */
    int free_inode; /* head of free inode list */
    int free_iblock; /* head of free block list */
} SUPERBLOCK;

#define N_DBLOCKS 10
#define N_IBLOCKS 4

typedef struct {
    int next_inode; /* list for free inodes */
    int protect; /* protection field */
    int nlink; /* number of links to this file */
    int size; /* numer of bytes in file */
    int uid; /* owner's user ID */
    int gid; /* owner's group ID */
    int ctime; /* time field */
    int mtime; /* time field */
    int atime; /* time field */
    int dblocks[N_DBLOCKS]; /* pointers to data blocks */
    int iblocks[N_IBLOCKS]; /* pointers to indirect blocks */
    int i2block; /* pointer to doubly indirect block */
    int i3block; /* pointer to triply indirect block */
} INODE;

// empty blocks are filled with EMPYT_DATA
#define EMPTY_DATA -1       // recommend value: 0 or -1

#endif