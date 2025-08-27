#include "inode.h"

#include <stdbool.h>

#include "disk.h"
#include "fs.h"
#include "logging.h"

// TODO Incorporate owner_id.

static bool check_inode_no_bounds(int inode_no) {
    if (inode_no < 0 || inode_no >= MAX_INODES) {
        logMsg(ERROR_LOG, "Invalid inode number: %d", inode_no);
        return false;
    }
    return true;
}

void init_inode_table() {
    require_disk_is_mounted();
    Inode inode = { 0 };
    diskseek(INODE_START * BLOCK_SIZE, SEEK_SET);
    for (int i = 0; i < MAX_INODES; ++i) {
        write_to_disk((void*)&inode, sizeof(Inode), 1);
    }
}

int read_inode(int inode_no, Inode* inode) {
    if (!check_inode_no_bounds(inode_no)) {
        return -1;
    }
    require_disk_is_mounted();
    logMsg(INFO_LOG, "Reading Inode # %d", inode_no);
    return read_from_disk_at(
        (void*)inode,
        sizeof(Inode),
        1,
        INODE_START * BLOCK_SIZE + sizeof(Inode) * inode_no
    );
}

int write_inode(int inode_no, Inode inode) {
    if (!check_inode_no_bounds(inode_no)) {
        return -1;
    }
    require_disk_is_mounted();
    logMsg(INFO_LOG, "Writing to Inode # %d", inode_no);
    return write_to_disk_at(
        (const void*)&inode,
        sizeof(Inode),
        1,
        INODE_START * BLOCK_SIZE + sizeof(Inode) * inode_no
    );
}

int alloc_inode() {
    require_disk_is_mounted();
    logMsg(INFO_LOG, "Allocating an Inode");
    Inode inode = { 0 };
    // In `alloc_inode()`, skip using `read_inode()` to prevent repeated `diskseek` operations.
    diskseek(INODE_START * BLOCK_SIZE, SEEK_SET);
    // Go over each pre-allocated Inode and find one that's free to use.
    for (int i = 0; i < MAX_INODES; ++i) {
        read_from_disk((void*)&inode, sizeof(Inode), 1);
        if (!inode_is_valid(inode)) {
            logMsg(INFO_LOG, "Found available inode: #%d", i);
            inode_set_valid(&inode);
            write_inode(i, inode);
            return i;
        }
    }
    logMsg(INFO_LOG, "Failed to allocate a free inode.");
    return -1;
}

void free_inode(int inode_no) {
    if (!check_inode_no_bounds(inode_no)) {
        return;
    }
    require_disk_is_mounted();
    logMsg(INFO_LOG, "Freeing Inode # %d", inode_no);
    Inode inode;
    read_inode(inode_no, &inode);
    inode_set_invalid(&inode);
    write_inode(inode_no, inode);
}