#ifndef IODISK
#define IODISK

/*
 * This module is used by fs to
 * easily write and read from the
 * disk.
 */

#include <stdint.h>
#include <stdio.h>

#include "fs.h"  // Inode, DirectoryEntry, SuperBlock.

//* --Inode
void read_inode(int inode_no, Inode* inode);
void write_inode(int inode_no, Inode inode);
void init_inode_table();
int alloc_inode();
void free_inode(int inode_no);
bool is_directory(Inode inode);

//* --DirectoryEntry
void add_dirent(int parent_inode_no, DirectoryEntry dirent);

//* --SuperBlock
//! SuperBlock is a global variable implicitly
//! used by these functions.
void load_superblock();
void write_superblock();

//* --Bitmap & Data Blocks
void clear_bitmap(); // Sets all bits to zero.
int alloc_block();
void free_block(int block_no);
void set_bmp(int block_no, char flag);  // 0 - free.
void read_data_block(int block_no, void* buf, size_t size);
void write_data_block(int block_no, void* data, size_t size);

//* --Path
int resolve_path(const char* path, int* inode_num);

#endif