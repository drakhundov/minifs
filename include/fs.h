#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "super.h"
#include "inode.h"
#include "dir.h"

// ! Information about the FS.
extern const int MAGIC_NUMBER;
extern const int BLOCK_SIZE;
extern const int NUM_BLOCKS;
extern const int DISK_SIZE;  // 1MB
// Since block size and # of blocks the
// same, one block is devoted to be bitmap.
extern const int BITMAP_START;
extern const int INODE_START;
extern const int INODE_BLOCKS;
extern const int DATA_START;
extern const int MAX_INODES;

/*
 * Used by read/write functions since `disk`
 * will be opened once by `mount_fs` and later
 * closed by `unmount_fs`
 */
void mkfs(const char* disk_img_fn);  // make filesystem (formats the disk).
int mkdir_fs(const char* path);
int mkfile_fs(const char* path);
int create_fs(const char* path, bool is_dir);
int write_fs(const char* path, const char* data);
int read_fs(const char* path, char* buf, size_t bufsize);
int delete_fs(const char* path);
int rmdir_fs(const char* path);
int ls_fs(const char* path, DirectoryEntry* entries, size_t max_entries);
