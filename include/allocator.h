#pragma once

#include <stdbool.h>
#include <stddef.h>

// Bitmap itself is encapsulated.

void load_bitmap_from_disk();
void flush_bitmap_to_disk();

// Sets all bits to zero.
//! Modifies RAM only. Call 'write' for changes to take effect.
void clear_bitmap();
void alloc_bitmap();

// Throws an error if the requirement is not met.
void require_bitmap_is_loaded();

int alloc_block();
void free_block(int block_no);

bool block_is_free(int block_no);

void read_data_block(int block_no, void* buf, size_t size);
void write_data_block(int block_no, const void* data, size_t size);
