#include "fs.h"

#include <string.h>

#include "disk.h"
#include "err.h"
#include "logging.h"
#include "path.h"

// TODO: add logs.
// TODO: Completed for mkfs, mount, unmount.

// ! Information about the FS.
const int MAGIC_NUMBER = 0x20240604;
const int BLOCK_SIZE = 1024;
const int NUM_BLOCKS = 1024;
const int DISK_SIZE = (BLOCK_SIZE * NUM_BLOCKS);  // 1MB
// Since block size and # of blocks the same, one block is devoted to be bitmap.
const int BITMAP_START = 1;
const int INODE_START = 2;
const int DATA_START = 11;
const int MAX_INODES = 128;

SuperBlock sb;

void mkfs(const char* disk_img_fn) {
    void write_superblock();  // Implemented in fs_helper.
    if (disk_img_fn == NULL) {
        err_exit("mkfs: `disk_img_fn` should contain the path to the disk image.");
    }
    logMsg(INFO_LOG, "mkfs: Opening disk file. path=%s.", disk_img_fn);
    create_disk_fs(disk_img_fn);
    logMsg(INFO_LOG, "Formatting disk.");
    uint8_t zeros[BLOCK_SIZE] = {0};
    for (int i = 0; i < NUM_BLOCKS; i++) {
        write_to_disk((void*)zeros, BLOCK_SIZE, 1);
    }
    sb.magic_number = MAGIC_NUMBER;
    sb.num_blocks = NUM_BLOCKS;
    sb.num_inodes = MAX_INODES;
    sb.bitmap_start = BITMAP_START;
    sb.inode_start = INODE_START;
    sb.data_start = DATA_START;
    logMsg(INFO_LOG, "mkfs: Writing superblock.");
    write_superblock();
    logMsg(INFO_LOG, "mkfs: Clearing bitmap.");
    clear_bitmap();
    logMsg(INFO_LOG, "mkfs: Initializing the inode table.");
    init_inode_table();
    //* Create root directory.
    logMsg(INFO_LOG, "mkfs: Creating root directory.");
    Inode root = {.f = IS_VALID_FLAG | IS_DIR_FLAG, .size = 0};
    write_to_disk_at(
      (const void*)&root,
      sizeof(Inode),
      1,
      INODE_START * BLOCK_SIZE
    );
}

int mkdir_fs(const char* path) { return create_fs(path, true); }
int mkfile_fs(const char* path) { return create_fs(path, false); }

int create_fs(const char* path, bool is_dir) {
    // TODO: bug
    require_disk_is_mounted();
    logMsg(INFO_LOG, "Creating %s is_dir=%d", path, is_dir);
    Inode file;
    int inode_no = alloc_inode();
    if (inode_no == -1) return -1;
    // Preemptively allocate data blocks for directories.
    // For files, do it on first write.
    if (is_dir && alloc_block() != 0) {
        return -1;
    }
    inode_set_valid(&file);
    if (is_dir) inode_set_dir(&file);
    file.size = 0;
    write_inode(inode_no, file);
    return 0;
}

int write_fs(const char* path, const char* data) {
    require_disk_is_mounted();
    size_t nbytes = strlen(data) / 8;
    char* name = strrchr(path, '/');
    if (!name || *(name + 1) == '\0') {
        logMsg(ERROR_LOG, "Invalid file name in path: %s\n", path);
        return -1;
    }
    name++;
    int p_inode_no;  // Parent inode num.
    if (path_to_inode(get_parent_path(path), &p_inode_no) != 0) {
        // Invalid path.
        return -1;
    }
    int inode_no = alloc_inode();
    if (inode_no == -1) {
        return -1;
    }
    Inode inode;
    read_inode(p_inode_no, &inode);
    if (inode.size % BLOCK_SIZE == 0) {
        // Need to allocate a new data block.
        int block = alloc_block();
        if (block == -1) return -1;
        inode.data_blocks[0] = block;
        write_data_block(block, (void*)data, strlen(data));
    } else {
        int last_block_no = inode.size / BLOCK_SIZE;
        write_data_block(inode.data_blocks[last_block_no], (void*)data, nbytes);
    }
    inode.size += strlen(data);
    write_inode(p_inode_no, inode);
    DirectoryEntry dirent;
    dirent.inode_number = inode_no;
    strcpy(dirent.name, name);
    add_dirent(p_inode_no, dirent);
    return 0;
}

int read_fs(const char* path, char* buf, size_t bufsize) {
    require_disk_is_mounted();
    Inode inode;
    int inode_no = 1;  // hardcoded
    read_inode(inode_no, &inode);
    read_data_block(inode.data_blocks[0], buf, bufsize);
    return inode.size;
}

int delete_fs(const char* path) {
    require_disk_is_mounted();
    int inode_no = 1;  // hardcoded
    Inode file = {0};
    write_inode(inode_no, file);
    return 0;
}

int rmdir_fs(const char* path) {
    return delete_fs(path);  // same as delete_fs for now
}

int ls_fs(const char* path, DirectoryEntry* entries, size_t max_entries) {
    require_disk_is_mounted();
    int count = 0;
    Inode inode;
    for (int i = 0; i < sb.num_inodes && count < max_entries; ++i) {
        read_inode(i, &inode);
        if (inode.f & IS_VALID_FLAG) {
            entries[count].inode_number = i;
            snprintf(entries[count].name, sizeof(entries[count].name), "inode%d", i);
            count++;
        }
    }
    return count;
}
