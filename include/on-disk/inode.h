#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Stores metadata for file entries,
 * except doesn't store names. All Inodes
 * are located consecutively, starting at
 * the same memory address.
 */
typedef struct {
    uint8_t f;    // InodeFlags.
    size_t size;  // bytes (file) or entry count (directory)
    // Stores indices of the data blocks on the
    // disk where the file's contents are located.
    int data_blocks[4];
} Inode;
