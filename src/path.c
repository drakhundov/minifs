#include "path.h"

#include <string.h>
#include <stdlib.h>

#include "iodisk.h"

int path_to_inode(const char* path, int* inode_no) {
  assert_disk_was_mounted();
  if (strcmp(path, "/") == 0) {
    // Root directory.
    *inode_no = 0;
    return 0;
  }
  // Use a copy of the path, since
  // you'll need to modify it.
  char path_cp[256];
  strncpy(path_cp, path, 256);
  char* token = strtok(path_cp, "/");
  int cur_inode_no = 0;  // Start from root.
  Inode cur_inode;
  read_inode(cur_inode_no, &cur_inode);
  DirectoryEntry dirents[DIRENTS_PER_BLOCK];
  while (token != NULL) {
    bool found = false;
    for (int i = 0; i < cur_inode.size; ++i) {
      int entry_index = i % DIRENTS_PER_BLOCK;
      if (i % DIRENTS_PER_BLOCK == 0) {
        int block_index = i / DIRENTS_PER_BLOCK;
        read_data_block(cur_inode.data_blocks[block_index], dirents, sizeof(dirents));
      }
      if (strcmp(dirents[entry_index].name, token) == 0) {
        cur_inode_no = dirents[entry_index].inode_number;
        read_inode(cur_inode_no, &cur_inode);
        found = true;
        break;
      }
    }
    if (!found) return -1;
    token = strtok(NULL, "/");
  }
  *inode_no = cur_inode_no;
  return 0;
}

char* get_parent_path(const char* full_path) {
  char* parent_path;
  size_t len;
  const char* last_slash = strrchr(full_path, '/');
  if (!last_slash || last_slash == full_path) {
    // Root or single-level path like "/file".
    len = 1;  // One character.
    parent_path = (char*)malloc(2);
    strcpy(parent_path, "/");
  } else {
    len = last_slash - full_path;
    parent_path = (char*)malloc(len + 1);
    strncpy(parent_path, full_path, len);
  }
  parent_path[len] = '\0';  // Null-terminate manually.
  return parent_path;
}