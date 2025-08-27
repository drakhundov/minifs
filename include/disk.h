#pragma once

// Disk itself is encapsulated.

int mount_fs(const char* disk_img_fn);
void unmount_fs();
int create_disk_fs(const char* disk_img_fn);

// Throws an error if the requirement is not met.
void require_disk_is_mounted();

//! Requires an offset.
size_t read_from_disk_at(void* buf, size_t size, size_t count, long offset);
size_t write_to_disk_at(const void* buf, size_t size, size_t count, long offset);

//! Uses current position.
size_t read_from_disk(void* buf, size_t size, size_t count);
size_t write_to_disk(const void* buf, size_t size, size_t count);

// A wrap function over the system call. Use it to ensure disk is not corrupt.
void diskseek(long offset, int start);
