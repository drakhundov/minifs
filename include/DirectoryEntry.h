#ifndef DIRENT_H
#define DIRENT_H

#define MAX_NAME_LEN 27
#define DIRENTS_PER_BLOCK (BLOCK_SIZE / sizeof(DirectoryEntry))

/*
 * Used to represent the directory structure
 * of the file system. Stores both directory
 * and regular file information.
 */
typedef struct
{
    int inode_number;
    char name[MAX_NAME_LEN + 1];  // 27 ASCII chars + null terminator.
} DirectoryEntry;

#endif