#pragma once

// Sets `inode_num` to the last inode in the path.
// Returns 0 on success, -1 on failure.
int get_inode_no_from_path(const char* path, int* inode_num);

char* get_parent_path(const char* full_path);
