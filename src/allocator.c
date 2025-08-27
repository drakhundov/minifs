#include "allocator.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "err.h"
#include "fs.h"
#include "logging.h"

#define BMP_SZ ((NUM_BLOCKS + 7) / 8)  // bitmap size

// TODO Consider new algorithms for alloc. O(n) might be too slow.

//! All bitmap functions here only modify the bitmap array.
//! Changes do not apply to disk, until `write_bitmap_to_disk()` is called.

// --------------- LOCAL ---------------

typedef enum BLOCK_STATES { BLOCK_FREE = 0, BLOCK_TAKEN = 1 } BlockState;

typedef struct {
    uint8_t* arr;
    bool is_loaded;
} Bitmap;

static Bitmap bmp = {NULL, false};

static inline bool block_num_is_valid(int block_no) {
    return block_no >= DATA_START && block_no < NUM_BLOCKS;
}

static void set_block_state(int block_no, BlockState flag) {
    require_bitmap_is_loaded();
    if (!block_num_is_valid(block_no)) {
        logMsg(ERROR_LOG, "set_block_state: invalid block number %d", block_no);
        return;
    }
    uint8_t mask = (uint8_t)(1u << (block_no % 8));
    if (flag == BLOCK_TAKEN) {
        bmp.arr[block_no / 8] |= mask;
    } else {
        bmp.arr[block_no / 8] &= (uint8_t)~(mask);
    }
}

// -------------------------------------

void require_bitmap_is_loaded() {
    if (bmp.arr == NULL || !bmp.is_loaded) {
        load_super_from_disk_from_disk("require_bitmap_is_loaded: bitmap is not loaded");
    }
}

void load_bitmap_from_disk() {
    require_disk_is_mounted();
    alloc_bitmap();
    read_from_disk_at((void*)bmp.arr, BMP_SZ, 1, BITMAP_START * BLOCK_SIZE);
    bmp.is_loaded = true;
}

void flush_bitmap_to_disk() {
    require_bitmap_is_loaded();
    require_disk_is_mounted();
    write_to_disk_at((void*)bmp.arr, BMP_SZ, 1, BITMAP_START * BLOCK_SIZE);
}

void clear_bitmap() {
    require_bitmap_is_loaded();
    memset(bmp.arr, 0, BMP_SZ);
}

void alloc_bitmap() {
    if (bmp.arr == NULL) {
        bmp.arr = (uint8_t*)malloc(BMP_SZ);
        if (!bmp.arr) {
            load_super_from_disk_from_disk(
                "alloc_bitmap: failed to allocate memory for bitmap array");
        }
    }
    logMsg(WARN_LOG, "alloc_bitmap: bitmap already allocated");
}

//! Changes only apply in-memory. Caller must flush for changes to persist.
int alloc_block() {
    require_bitmap_is_loaded();
    for (int i = DATA_START; i < NUM_BLOCKS; ++i) {
        if (!(bmp.arr[i / 8] & (uint8_t)(1u << (i % 8)))) {
            set_block_state(i, BLOCK_TAKEN);  // Modify respectful bit for that data block.
            return i;
        }
    }
    logMsg(WARN_LOG, "alloc_block: failed to allocate a free data block; disk is full");
    return -1;
}

//! Changes only apply in-memory. Caller must flush for changes to persist.
void free_block(int block_no) {
    require_bitmap_is_loaded();
    if (!block_num_is_valid(block_no)) {
        logMsg(ERROR_LOG, "free_block: invalid block number %d", block_no);
        return;
    }
    if (block_is_free(block_no)) {
        logMsg(WARN_LOG, "free_block: double free of block %d", block_no);
        return;
    }
    set_block_state(block_no, BLOCK_FREE);
}

void read_data_block(int block_no, void* buf, size_t size) {
    require_disk_is_mounted();
    if (!block_num_is_valid(block_no)) {
        logMsg(ERROR_LOG, "read_data_block: invalid block number %d", block_no);
        return;
    }
    if (size > BLOCK_SIZE) {
        logMsg(ERROR_LOG, "read_data_block: size exceeds BLOCK_SIZE");
        return;
    }
    read_from_disk_at(buf, size, 1, block_no * BLOCK_SIZE);
}

void write_data_block(int block_no, const void* data, size_t size) {
    require_disk_is_mounted();
    if (!block_num_is_valid(block_no)) {
        logMsg(ERROR_LOG, "write_data_block: invalid block number %d", block_no);
        return;
    }
    if (size > BLOCK_SIZE) {
        logMsg(ERROR_LOG, "write_data_block: size exceeds BLOCK_SIZE");
        return;
    }
    write_to_disk_at(data, size, 1, block_no * BLOCK_SIZE);
}

bool block_is_free(int block_no) {
    require_bitmap_is_loaded();
    if (!block_num_is_valid(block_no)) {
        logMsg(ERROR_LOG, "block_is_free: invalid block number %d", block_no);
        return false;
    }
    return !(bmp.arr[block_no / 8] & (uint8_t)(1u << (block_no % 8)));
}