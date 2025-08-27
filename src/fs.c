#include "fs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "dir.h"
#include "disk.h"
#include "err.h"
#include "inode.h"
#include "logging.h"
#include "on-disk/super.h"
#include "path.h"
#include "super.h"

void mkfs(const char* disk_img_fn) {
    if (!disk_img_fn) {
        load_super_from_disk_from_disk(
            "mkfs: `disk_img_fn` should contain the path to the disk image.");
    }
    logMsg(INFO_LOG, "mkfs: Opening disk file. path=%s.", disk_img_fn);
    size_t disk_size = (size_t)BLOCK_SIZE * (size_t)NUM_BLOCKS;
    if (create_disk_fs(disk_img_fn, disk_size) != 0) {
        load_super_from_disk_from_disk("mkfs: failed to create disk image");
    }
    logMsg(INFO_LOG, "mkfs: zeroing disk");
    uint8_t zeros[1024] = {0};
    for (uint32_t i = 0; i < NUM_BLOCKS; i++) {
        write_to_disk((void*)zeros, BLOCK_SIZE, 1);
    }
    // Write superblock to LBA 0
    SuperConfig sb = {
        .magic_number = MAGIC,
        .block_size = BLOCK_SIZE,
        .num_blocks = NUM_BLOCKS,
        .max_inodes = MAX_INODES,
        .bitmap_start = BITMAP_START,
        .inode_start = INODE_START,
        .data_start = DATA_START};
    logMsg(INFO_LOG, "mkfs: writing superblock");
    set_super(&sb);
    flush_super_to_disk();

    // Initialize bitmap (RAM) then persist
    logMsg(INFO_LOG, "mkfs: initializing bitmap");
    alloc_bitmap();
    clear_bitmap();
    flush_bitmap_to_disk();

    // Initialize inode table
    logMsg(INFO_LOG, "mkfs: initializing inode table");
    init_inode_table();

    // Create root directory inode (inode #0)
    logMsg(INFO_LOG, "mkfs: creating root directory");
    int root_ino = alloc_inode();
    if (root_ino < 0) {
        load_super_from_disk_from_disk("mkfs: failed to allocate root inode");
    }
    Inode root = {0};
    inode_set_valid(&root);
    inode_set_dir(&root);
    root.size = 0;
    int blk = alloc_block();
    if (blk < 0) {
        load_super_from_disk_from_disk("mkfs: failed to allocate root data block");
    }
    root.data_blocks[0] = blk;
    write_inode(root_ino, root);
    flush_bitmap_to_disk();
}

int mkdir_fs(const char* path) {
    logMsg(INFO_LOG, "mkdir_fs: path=%s", path ? path : "(null)");
    int rc = create_fs(path, true);
    if (rc != 0) {
        logMsg(ERROR_LOG, "mkdir_fs failed: path=%s rc=%d", path ? path : "(null)", rc);
    } else {
        logMsg(INFO_LOG, "mkdir_fs succeeded: path=%s", path ? path : "(null)");
    }
    return rc;
}

int mkfile_fs(const char* path) {
    logMsg(INFO_LOG, "mkfile_fs: path=%s", path ? path : "(null)");
    int rc = create_fs(path, false);
    if (rc != 0) {
        logMsg(ERROR_LOG, "mkfile_fs failed: path=%s rc=%d", path ? path : "(null)", rc);
    } else {
        logMsg(INFO_LOG, "mkfile_fs succeeded: path=%s", path ? path : "(null)");
    }
    return rc;
}

int create_fs(const char* path, bool is_dir) {
    // TODO: bug
    require_disk_is_mounted();
    logMsg(INFO_LOG, "create_fs: path=%s is_dir=%d", path ? path : "(null)", is_dir);
    Inode file = (Inode){0};
    int inode_no = alloc_inode();
    if (inode_no == -1) {
        logMsg(ERROR_LOG, "create_fs: alloc_inode failed for path=%s", path ? path : "(null)");
        return -1;
    }
    // Preemptively allocate data blocks for directories.
    // For files, do it on first write.
    if (is_dir) {
        int block_no = alloc_block();
        if (block_no < 0) {
            logMsg(
                ERROR_LOG, "create_fs: alloc_block failed for dir path=%s", path ? path : "(null)");
            return -1;
        }
        file.data_blocks[0] = block_no;
    }
    inode_set_valid(&file);
    if (is_dir) inode_set_dir(&file);
    file.size = 0;
    write_inode(inode_no, file);
    logMsg(
        INFO_LOG,
        "create_fs: created inode=%d path=%s is_dir=%d",
        inode_no,
        path ? path : "(null)",
        is_dir);
    return 0;
}

// ! CONCERNING read_fs and write_fs
// ! As of now, these function assume there is only one block per inode
// ! and if the data is greater than BLOCK_SIZE, it will truncate it.
// * Return the number of bytes operated on.
int read_fs(const char* path, char* buf, size_t bufsize) {
    require_disk_is_mounted();
    logMsg(INFO_LOG, "read_fs: path=%s bufsize=%zu", path ? path : "(null)", bufsize);
    if (!path) {
        logMsg(ERROR_LOG, "read_fs: path is null");
        return -1;
    }
    if (!buf) {
        logMsg(ERROR_LOG, "read_fs: buf is null");
        return -1;
    }
    int inode_no;
    if (get_inode_no_from_path(path, &inode_no) != 0) {
        logMsg(ERROR_LOG, "read_fs: invalid path=%s", path);
        return -1;
    }
    Inode inode;
    if (read_inode(inode_no, &inode) != 0) {
        logMsg(ERROR_LOG, "read_fs: error reading inode no %d", inode_no);
        return -1;
    }
    size_t nbytes_to_read = bufsize < inode.size ? bufsize : inode.size;
    read_data_block(inode.data_blocks[0], buf, nbytes_to_read);
    logMsg(INFO_LOG, "read_fs: read bytes=%d from inode=%d", inode.size, inode_no);
    return nbytes_to_read;
}

int write_fs(const char* path, const char* data) {
    require_disk_is_mounted();
    if (!path) {
        logMsg(ERROR_LOG, "write_fs: path is null");
        return -1;
    }
    if (!data) {
        logMsg(ERROR_LOG, "write_fs: data is null");
        return -1;
    }
    logMsg(INFO_LOG, "write_fs: path=%s bytes=%zu", path, strlen(data));
    size_t nbytes = strlen(data);
    char* name = strrchr(path, '/');
    if (!name || *(name + 1) == '\0') {
        logMsg(ERROR_LOG, "write_fs: invalid file name in path: %s", path);
        return -1;
    }
    name++;
    int p_inode_no;  // Parent inode num.
    char* parent_path = get_parent_path(path);
    if (get_inode_no_from_path(parent_path, &p_inode_no) != 0) {
        // Invalid path.
        logMsg(ERROR_LOG, "write_fs: invalid parent path for %s", path);
        free(parent_path);
        return -1;
    }
    free(parent_path);
    int inode_no = alloc_inode();
    if (inode_no == -1) {
        logMsg(ERROR_LOG, "write_fs: alloc_inode failed for %s", path);
        return -1;
    }
    Inode finode = (Inode){0};
    inode_set_valid(&finode);
    int block_no = alloc_block();
    if (block_no == -1) {
        logMsg(ERROR_LOG, "write_fs: alloc_block failed for %s", path);
        return -1;
    }
    finode.data_blocks[0] = block_no;
    size_t nbytes_to_write = nbytes > BLOCK_SIZE ? BLOCK_SIZE : nbytes;
    write_data_block(block_no, (void*)data, nbytes_to_write);
    finode.size = nbytes_to_write;
    write_inode(inode_no, finode);
    DirectoryEntry dirent;
    dirent.inode_number = inode_no;
    strncpy(dirent.name, name, MAX_DIRNAME_LEN);
    dirent.name[MAX_DIRNAME_LEN] = '\0';
    add_dirent(p_inode_no, dirent);
    logMsg(
        INFO_LOG,
        "write_fs: wrote file name=%s inode=%d parent_inode=%d size=%zu",
        name,
        inode_no,
        p_inode_no,
        nbytes_to_write);
    return nbytes_to_write;
}

int delete_fs(const char* path) {
    require_disk_is_mounted();
    logMsg(INFO_LOG, "delete_fs: path=%s", path ? path : "(null)");
    if (!path) {
        logMsg(ERROR_LOG, "delete_fs: path is null");
        return -1;
    }
    int inode_no;
    if (get_inode_no_from_path(path, &inode_no) != 0) {
        logMsg(ERROR_LOG, "delete_fs: invalid path=%s", path);
        return -1;
    }
    Inode inode;
    if (read_inode(inode_no, &inode) != 0) {
        logMsg(ERROR_LOG, "delete_fs: error reading inode no %d", inode_no);
        return -1;
    }
    for (int i = 0; i < MAX_INODE_DATA_BLOCKS; ++i) {
        if (inode.data_blocks[i] > 0) {
            free_block(inode.data_blocks[i]);
        }
    }
    inode_set_invalid(&inode);
    write_inode(inode_no, inode);
    logMsg(INFO_LOG, "delete_fs: cleared inode=%d", inode_no);
    return 0;
}

int rmdir_fs(const char* path) {
    logMsg(INFO_LOG, "rmdir_fs: path=%s", path ? path : "(null)");
    int rc = delete_fs(path);  // same as delete_fs for now
    if (rc != 0) {
        logMsg(ERROR_LOG, "rmdir_fs failed: path=%s rc=%d", path ? path : "(null)", rc);
    } else {
        logMsg(INFO_LOG, "rmdir_fs succeeded: path=%s", path ? path : "(null)");
    }
    return rc;
}

int ls_fs(const char* path, DirectoryEntry* entries, size_t max_entries) {
    require_disk_is_mounted();
    logMsg(INFO_LOG, "ls_fs: path=%s max_entries=%zu", path ? path : "(null)", max_entries);
    int count = 0;
    Inode inode;
    for (uint32_t i = 0; i < MAX_INODES && count < (int)max_entries; ++i) {
        read_inode(i, &inode);
        if (inode.f & IS_VALID_FLAG) {
            entries[count].inode_number = i;
            snprintf(entries[count].name, sizeof(entries[count].name), "inode%d", i);
            count++;
        }
    }
    logMsg(INFO_LOG, "ls_fs: entries_found=%d", count);
    return count;
}