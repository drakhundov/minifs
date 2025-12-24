#include "super.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "err.h"
#include "fs.h"
#include "logging.h"
#include "on-disk/super.h"

static SuperBlock sb = {0};
static bool is_loaded = false;
static bool is_dirty = false;

int load_super_from_disk() {
    require_disk_is_mounted();
    read_from_disk_at((void*)&sb, sizeof(SuperBlock), 1, 0);
    if (validate_super(&sb) != 0) {
        return -1;
    }
    is_loaded = true;
    is_dirty = false;
    logMsg(
        INFO_LOG,
        "load_super_from_diskblock: block_size=%u, nblocks=%u, ninodes=%u",
        sb.block_size,
        sb.num_blocks,
        sb.max_inodes);
    return 0;
}

void flush_super_to_disk() {
    require_disk_is_mounted();
    if (!is_loaded) {
        err_exit("flush_super_to_disk: super not loaded");
    }
    if (!is_dirty) {
        return;
    }
    write_to_disk_at((void*)&sb, sizeof(SuperBlock), 1, 0);
    is_dirty = false;
}

void set_super(const SuperConfig* cfg) {
    require_disk_is_mounted();
    if (!cfg) {
        err_exit("set_super: config is null");
    }
    if (validate_super(cfg) != 0) {
        err_exit("set_super: invalid super config");
    }

    memset(&sb, 0, sizeof sb);
    sb.magic_number = cfg->magic_number;
    sb.block_size = cfg->block_size;
    sb.num_blocks = cfg->num_blocks;
    sb.max_inodes = cfg->max_inodes;
    sb.bitmap_start = cfg->bitmap_start;
    sb.inode_start = cfg->inode_start;
    sb.data_start = cfg->data_start;

    is_loaded = true;
    is_dirty = true;
    logMsg(
        INFO_LOG,
        "format_super: new config (block_size=%u, nblocks=%u, max_inodes=%u)",
        sb.block_size,
        sb.num_blocks,
        sb.max_inodes);
}

int validate_super(const SuperConfig* sb) {
    if (!sb) return -1;
    if (sb->magic_number != MAGIC) {
        logMsg(ERROR_LOG, "super_validate: bad magic 0x%08x", sb->magic_number);
        return -1;
    }
    if (sb->block_size == 0 || (sb->block_size & (sb->block_size - 1)) != 0) {
        logMsg(ERROR_LOG, "super_validate: block_size must be a power of two");
        return -1;
    }
    if (!(sb->bitmap_start < sb->inode_start && sb->inode_start < sb->data_start)) {
        logMsg(ERROR_LOG, "super_validate: region ordering invalid");
        return -1;
    }
    if (sb->data_start >= sb->num_blocks) {
        logMsg(ERROR_LOG, "super_validate: data_start out of range");
        return -1;
    }
    return 0;
}
