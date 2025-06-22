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
/*
 * Add a DirectoryEntry for a newly created
 * Inode (to the parent directory's data block.)
 */
void add_dirent(int parent_inode_no, DirectoryEntry dirent);

//* --SuperBlock
//! SuperBlock is a global variable implicitly
//! used by these functions.
void load_superblock();
void write_superblock();

//* --Bitmap & Data Blocks
//! Like SuperBlock, bitmap is a globally
//! defined array. It is read once during
//! mount, and could be written back to memory.
void load_bitmap();
void clear_bitmap();  // Sets all bits to zero.
void write_bitmap();
int alloc_block();
void free_block(int block_no);
void set_bmp(int block_no, char flag);  // 0 - free.
void read_data_block(int block_no, void* buf, size_t size);
void write_data_block(int block_no, void* data, size_t size);
bool block_is_free(int block_no);

//* --Path
int resolve_path(const char* path, int* inode_num);

#endif