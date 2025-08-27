#pragma once

#include "fs.h"
#include "on-disk/dirent.h"

#define DIRENTS_PER_BLOCK (BLOCK_SIZE / sizeof(DirectoryEntry))

/*
 * Add a DirectoryEntry for a newly created
 * Inode (to the parent directory's data block.)
 */
void add_dirent(int parent_inode_no, DirectoryEntry dirent);
void remove_dirent(int inode);
