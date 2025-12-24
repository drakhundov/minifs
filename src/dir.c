#include "dir.h"

#include <stddef.h>

#include "allocator.h"
#include "fs.h"
#include "inode.h"
#include "logging.h"

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
