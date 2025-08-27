#pragma once

#include <stdint.h>

#define MAGIC 0x20240604

/*
 * There is only one single SuperBlock.
 * It's basically a single metadata provider
 * for the created disk.
 */
typedef struct {
    uint32_t magic_number;  // filesystem identifier
    uint32_t block_size;
    uint32_t num_blocks;
    uint32_t max_inodes;
    uint32_t bitmap_start;  // block index of bitmap
    uint32_t inode_start;   // block index of inode table
    uint32_t data_start;    // block index of first data block
} SuperBlock;
