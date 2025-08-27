#pragma once

#include "on-disk/super.h"

// For now, use the on-disk type. Later, SuperConfig could be further extended to include version
// information etc.
typedef SuperBlock SuperConfig;

void set_super(const SuperConfig* cfg);
int load_super_from_disk();
void flush_super_to_disk();

int validate_super(const SuperConfig* s);

// // Getters.
// const SuperBlock* get_super();
// uint32_t get_block_size(void);
// uint32_t get_num_blocks(void);
// uint32_t get_max_inodes(void);
// uint32_t get_bitmap_start(void);
// uint32_t get_inode_start(void);
// uint32_t get_data_start(void);
