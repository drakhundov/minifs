#include "disk.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "err.h"
#include "logging.h"
#include "super.h"

// TODO Add multiple disk support.
// TODO  - Possibly return an ID for each mounted disk.
// TODO  - Possibly use multithreading.

typedef struct {
    FILE* fp;
    char img_fn[64];
    size_t size;
    // true - disk has been mounted (`fp` is an open file).
    // false - hasn't been mounted yet (`fp` is NULL).
    bool is_mounted;
} Disk;

static Disk* disk = NULL;

// 0 - freed
// 1 - disk is NULL
static int free_disk() {
    if (disk != NULL) {
        if (disk->fp != NULL) {
            fclose(disk->fp);
        }
        free(disk);
        disk = NULL;
        return 0;
    } else {
        return 1;
    }
}

static int open_disk(const char* disk_img_fn, const char* filemode) {
    if (disk != NULL) {
        if (disk->is_mounted) {
            logMsg(
                ERROR_LOG,
                "open_disk: There is a mounted disk at %s. Unmount it first.",
                disk->img_fn);
            return -1;
        } else {
            logMsg(INFO_LOG, "open_disk: An unmounted disk still referenced. Freeing the pointer.");
            free_disk();
        }
    }
    disk = (Disk*)malloc(sizeof(Disk));
    if (strlen(disk_img_fn) > sizeof(disk->img_fn)) {
        logMsg(
            INFO_LOG,
            "open_disk: Disk image path too long. Maximum length: %d.",
            sizeof(disk->img_fn));
        free_disk();
        return -1;
    }
    snprintf(disk->img_fn, sizeof(disk->img_fn), "%s", disk_img_fn);
    logMsg(INFO_LOG, "open_disk: Mounting disk at %s.", disk_img_fn);
    disk->fp = fopen(disk_img_fn, "rb+");
    if (disk->fp == NULL) {
        logMsg(INFO_LOG, "open_disk: Failed to open the disk image.");
        free_disk();
        return -1;
    }
    return 0;
}

// Wrap function around system call. Used to be sure disk is not corrupt.
void diskseek(long offset, int start) {
    if (disk == NULL || disk->fp == NULL) {
        err_exit("disk == NULL || disk->fp == NULL");
    }
    if (fseek(disk->fp, offset, start) != 0) {
        err_exit("Disk is corrupted.");
    }
}

int mount_fs(const char* disk_img_fn) {
    if (open_disk(disk_img_fn, "rb+") != 0) {
        logMsg(ERROR_LOG, "mount_fs: Failed to open disk at %s.", disk_img_fn);
    }
    // Use 'fseeko' because the disk
    // image is too large (>2GB).
    if (fseeko(disk->fp, 0, SEEK_END) == 0) {
        off_t sz = ftello(disk->fp);
        if (sz >= 0) {
            disk->size = (size_t)sz;
        }
        fseeko(disk->fp, 0, SEEK_SET);
    }
    load_superblock();
    load_bitmap_from_memory();
    disk->is_mounted = true;
    return 0;
}

void unmount_fs() {
    if (disk == NULL) {
        err_exit("Disk is NULL.");
    }
    if (!disk->is_mounted) {
        err_exit("The disk is unmounted.");
    }
    logMsg(INFO_LOG, "unmount_fs: Unmounting disk %s.", disk->img_fn);
    free_disk();
}

int create_disk_fs(const char* disk_img_fn) { return open_disk(disk_img_fn, "wb+"); }

void require_disk_is_mounted() {
    if (disk == NULL) {
        err_exit("Disk is NULL.");
    }
    if (!disk->is_mounted) {
        err_exit("Disk hasn't been mounted yet.");
    }
}

size_t read_from_disk_at(void* buf, size_t size, size_t count, long offset) {
    diskseek(offset, SEEK_SET);
    return fread(buf, size, count, disk->fp);
}

size_t write_to_disk_at(const void* buf, size_t size, size_t count, long offset) {
    diskseek(offset, SEEK_SET);
    return fwrite(buf, size, count, disk->fp);
}

size_t read_from_disk(void* buf, size_t size, size_t count) {
    return fread(buf, size, count, disk->fp);
}

size_t write_to_disk(const void* buf, size_t size, size_t count) {
    return fwrite(buf, size, count, disk->fp);
}