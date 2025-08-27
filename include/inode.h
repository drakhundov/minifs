#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "on-disk/inode.h"

// TODO: implement permissions.
#define IS_VALID_FLAG 0x01  // 0b00000001
#define IS_DIR_FLAG 0x02    // 0b00000010

#define MAX_INODE_DATA_BLOCKS 4

void init_inode_table();
int alloc_inode();
void free_inode(int inode_no);
int read_inode(int inode_no, Inode* inode);
int write_inode(int inode_no, const Inode inode);

static inline bool inode_is_valid(Inode inode) {
    return inode.f & IS_VALID_FLAG;
}
static inline bool inode_is_dir(Inode inode) {
    return inode.f & IS_DIR_FLAG;
}
static inline void inode_set_valid(Inode* inode) {
    inode->f |= IS_VALID_FLAG;
}
static inline void inode_set_invalid(Inode* inode) {
    inode->f &= ~IS_VALID_FLAG;
}
static inline void inode_set_dir(Inode* inode) {
    inode->f |= IS_DIR_FLAG;
}
