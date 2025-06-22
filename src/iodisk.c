#include "iodisk.h"

#include <stdint.h>
#include <string.h>

#include "fs.h"
#include "logging.h"

// Variables defined in fs implementation
// that only fs_helper could access.
extern FILE* disk;
extern SuperBlock sb;
extern uint8_t bitmap[];

//* --Inode
void read_inode(int inode_no, Inode* inode) {
  assert_disk_was_mounted();
  logMsg(INFO_LOG, "Reading Inode # %d", inode_no);
  fseek(disk, INODE_START * BLOCK_SIZE + sizeof(Inode) * inode_no, SEEK_SET);
  fread(inode, sizeof(Inode), 1, disk);
}

void write_inode(int inode_no, Inode inode) {
  assert_disk_was_mounted();
  logMsg(INFO_LOG, "Writing to Inode # %d", inode_no);
  fseek(disk, INODE_START * BLOCK_SIZE + sizeof(Inode) * inode_no, SEEK_SET);
  fwrite(&inode, sizeof(Inode), 1, disk);
}

void init_inode_table() {
  assert_disk_was_mounted();
  Inode inode = {0};
  fseek(disk, INODE_START * BLOCK_SIZE, SEEK_SET);
  for (int i = 0; i < MAX_INODES; ++i) {
    fwrite(&inode, sizeof(Inode), 1, disk);
  }
}

int alloc_inode() {
  assert_disk_was_mounted();
  logMsg(INFO_LOG, "Allocating an Inode");
  Inode inode;
  // In `alloc_inode()`, skip using `read_inode()`
  // to prevent repeated `fseek` operations.
  fseek(disk, INODE_START * BLOCK_SIZE, SEEK_SET);
  // Go over each pre-allocated Inode
  // and find one that's free to use.
  for (int i = 0; i < MAX_INODES; ++i) {
    fread(&inode, sizeof(Inode), 1, disk);
    if (!inode_is_valid(inode)) {
      logMsg(INFO_LOG, "Found available inode: #%d", i);
      inode_set_valid(&inode);
      write_inode(i, inode);
      return i;
    }
  }
  return -1;
}

void free_inode(int inode_no) {
  assert_disk_was_mounted();
  logMsg(INFO_LOG, "Freeing Inode # %d", inode_no);
  Inode inode;
  read_inode(inode_no, &inode);
  inode_set_invalid(&inode);
  write_inode(inode_no, inode);
}

//* --DirectoryEntry
/*
 * Adds a DirectoryEntry for inode to a given parent inode.
 * Allocates a new block if necessary.
 */
void add_dirent(int parent_inode_no, DirectoryEntry dirent) {
  logMsg(
      INFO_LOG,
      "Adding a directory entry. [parent_inode_no=%d\tname=%s]",
      parent_inode_no,
      dirent.name);
  Inode inode;
  read_inode(parent_inode_no, &inode);
  // Here, inode.size represents the number of directory entries used.
  switch (inode.size) {
    case 0:
      // Don't read current dirents, just write one entry.
      // ! The first data block should be already
      // ! allocated during directory creation.
      logMsg(INFO_LOG, "Writing to the first data block.");
      write_data_block(inode.data_blocks[0], (void*)&dirent, sizeof(DirectoryEntry));
      inode.size = 1;
      break;
    default: {
      // Number of blocks that are used based on the
      // number of directory entries (inode.size).
      size_t nblocks = (inode.size * sizeof(DirectoryEntry) + BLOCK_SIZE - 1) / BLOCK_SIZE;
      if (inode.size % DIRENTS_PER_BLOCK == 0) {  // Need to allocate a new block.
        if (nblocks >= MAX_INODE_DATA_BLOCKS) {
          logMsg(ERROR_LOG, "Maximum number of directory entries reached.");
          return;
        }
        logMsg(INFO_LOG, "Allocating data block.");
        int block_no = alloc_block();
        if (block_no < 0) {
          logMsg(ERROR_LOG, "Failed to allocate new data block.");
          return;
        }
        inode.data_blocks[nblocks] = block_no;
        write_data_block(block_no, (void*)&dirent, sizeof(DirectoryEntry));
      } else {
        DirectoryEntry dirents[DIRENTS_PER_BLOCK];
        read_data_block(inode.data_blocks[nblocks - 1], dirents, BLOCK_SIZE);
        dirents[inode.size % DIRENTS_PER_BLOCK] = dirent;
        write_data_block(inode.data_blocks[nblocks - 1], dirents, BLOCK_SIZE);
      }
      inode.size += 1;
      break;
    }
  }
  // Write update inode back.
  write_inode(parent_inode_no, inode);
}

//* --SuperBlock
void load_superblock() {
  assert_disk_was_mounted();
  fseek(disk, 0, SEEK_SET);
  fread(&sb, sizeof(SuperBlock), 1, disk);
}

void write_superblock() {
  assert_disk_was_mounted();
  fseek(disk, 0, SEEK_SET);
  fwrite(&sb, sizeof(SuperBlock), 1, disk);
}

//* --Bitmap & Data Blocks
void load_bitmap() {
  assert_disk_was_mounted();
  fseek(disk, BITMAP_START * BLOCK_SIZE, SEEK_SET);
  fread(bitmap, 1, BLOCK_SIZE, disk);
}

void clear_bitmap() {
  assert_disk_was_mounted();
  memset(bitmap, 0, BLOCK_SIZE);
  fseek(disk, BITMAP_START * BLOCK_SIZE, SEEK_SET);
  fwrite(bitmap, BLOCK_SIZE, 1, disk);
}

void write_bitmap() {
  assert_disk_was_mounted();
  fseek(disk, BITMAP_START * BLOCK_SIZE, SEEK_SET);
  fwrite(bitmap, 1, BLOCK_SIZE, disk);
}

int alloc_block() {
  assert_disk_was_mounted();
  for (int i = DATA_START; i < NUM_BLOCKS; ++i) {
    if (!(bitmap[i / 8] & (1 << (i % 8)))) {
      set_bmp(i, 1);   // Modify respectful bit for that data block.
      write_bitmap();  // Store the changes in the disk.
      return i;
    }
  }
  return -1;
}

void free_block(int block_no) {
  assert_disk_was_mounted();
  set_bmp(block_no, 0);
  write_bitmap();
}

//! Only modifies globally defined bitmap.
//! Changes do not apply to disk, until
//! called `write_bitmap()`.
void set_bmp(int block_no, char flag) {
  assert_disk_was_mounted();
  if (flag) {
    bitmap[block_no / 8] |= (1 << (block_no % 8));
  } else {
    bitmap[block_no / 8] &= ~(1 << (block_no % 8));
  }
}

void read_data_block(int block_no, void* buf, size_t size) {
  assert_disk_was_mounted();
  fseek(disk, block_no * BLOCK_SIZE, SEEK_SET);
  fread(buf, 1, size, disk);
}

void write_data_block(int block_no, void* data, size_t size) {
  assert_disk_was_mounted();
  if (size > BLOCK_SIZE) {
    logMsg(ERROR_LOG, "write_data_block: size exceeds BLOCK_SIZE");
    return;
  }
  fseek(disk, block_no * BLOCK_SIZE, SEEK_SET);
  fwrite(data, 1, size, disk);
}

bool block_is_free(int block_no) {
  assert_disk_was_mounted();
  return !(bitmap[block_no / 8] & (1 << (block_no % 8)));
}
