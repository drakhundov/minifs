#ifndef IODISK_H
#define IODISK_H

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
void set_bmp(int block_no, char flag);  // 0 - free.

int alloc_block();
void free_block(int block_no);

void read_data_block(int block_no, void* buf, size_t size);
void write_data_block(int block_no, void* data, size_t size);
void write_blocks(Inode inode, void* data, size_t size);

bool block_is_free(int block_no);

// Wrap function for system call.
void diskseek(FILE *f, long pos, int start);

#endif