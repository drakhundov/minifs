#ifndef INODE_H
#define INODE_H

#include <stddef.h>
#include <stdint.h>

// TODO: implement permissions.
#define IS_VALID_FLAG 0x01  // 0b00000001
#define IS_DIR_FLAG 0x02    // 0b00000010

#define MAX_INODE_DATA_BLOCKS 4

/* Stores metadata for file entries,
 * except doesn't store names. All Inodes
 * are stored consecutively, starting at
 * the same memory address.
 */
typedef struct {
  uint8_t f;    // InodeFlags.
  size_t size;  // bytes (file) or entry count (directory)
  // Stores indices of the data blocks on the
  // disk where the file's contents are located.
  int data_blocks[MAX_INODE_DATA_BLOCKS];
  uint16_t owner_id;
} Inode;

static inline bool inode_is_valid(Inode inode) { return inode.f & IS_VALID_FLAG; }
static inline bool inode_is_dir(Inode inode) { return inode.f & IS_DIR_FLAG; }
static inline void inode_set_valid(Inode *inode) { inode->f |= IS_VALID_FLAG; }
static inline void inode_set_invalid(Inode *inode) { inode->f &= ~IS_VALID_FLAG; }
static inline void inode_set_dir(Inode *inode) { inode->f |= IS_DIR_FLAG; }

#endif