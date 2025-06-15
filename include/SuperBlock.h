#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

/* There is only one single SuperBlock.
 * It's basically a single metadata provider
 * for the created disk.
 */
typedef struct
{
    int magic_number;  // filesystem identifier
    int num_blocks;    // total blocks (1024)
    int num_inodes;    // total inodes (e.g., 128)
    int bitmap_start;  // block index of bitmap
    int inode_start;   // block index of inode table
    int data_start;    // block index of first data block
} SuperBlock;

#endif