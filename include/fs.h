#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "on-disk/dirent.h"

// Filesystem layout constants (compile-time configuration)
// Shared across modules (allocator, inode, dir).
// If you later add dynamic sizing, replace these with getters.
#define BLOCK_SIZE 1024
#define NUM_BLOCKS 1024
#define DISK_SIZE (BLOCK_SIZE * NUM_BLOCKS)  // 1MB
#define BITMAP_START 1
#define INODE_START 2
#define DATA_START 11
#define MAX_INODES 128

/*
 * Used by read/write functions since `disk`
 * will be opened once by `mount_fs` and later
 * closed by `unmount_fs`
 */
void mkfs(const char* disk_img_fn);  // make filesystem (formats the disk).
int mkdir_fs(const char* path);
int mkfile_fs(const char* path);
int create_fs(const char* path, bool is_dir);
int read_fs(const char* path, char* buf, size_t bufsize);
int write_fs(const char* path, const char* data);
int delete_fs(const char* path);
int rmdir_fs(const char* path);
int ls_fs(const char* path, DirectoryEntry* entries, size_t max_entries);
