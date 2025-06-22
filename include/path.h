#ifndef PATH_H
#define PATH_H

// Sets `inode_num` to the last inode in the path.
// Returns 0 on success, -1 on failure.
int path_to_inode(const char* path, int* inode_num);
char *get_parent_path(const char* full_path);

#endif