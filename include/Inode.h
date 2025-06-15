#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include <stddef.h>

// TODO: implement permissions.
#define IS_VALID_FLAG 0x01  // 0b00000001
#define IS_DIR_FLAG 0x02    // 0b00000010

const int MAX_INODE_DATA_BLOCKS = 4;

typedef struct
{
    uint8_t f;    // InodeFlags.
    size_t size;  // bytes (file) or entry count (directory)
    // Stores indices of the data blocks on the
    // disk where the file's contents are located.
    int data_blocks[MAX_INODE_DATA_BLOCKS];
    uint16_t owner_id;
} Inode;

#endif