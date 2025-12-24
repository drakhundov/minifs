#include "disk.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "allocator.h"
#include "err.h"
#include "logging.h"
#include "super.h"

// TODO Add multiple disk support.
// TODO  - Possibly return an ID for each mounted disk.
// TODO  - Possibly use multithreading.

// --------------- LOCAL ---------------

typedef struct {
    FILE* fp;
    char img_fn[64];
    size_t size;
    // true - disk has been mounted (`fp` is an open file).
    // false - hasn't been mounted yet (`fp` is NULL).
    bool is_mounted;
} Disk;

static Disk disk = {NULL, {}, 0, false};

static void free_disk(void) {
    if (disk.fp) {
        fclose(disk.fp);
    }
    disk.fp = NULL;
    disk.img_fn[0] = '\0';
    disk.size = 0;
    disk.is_mounted = false;
}

static int open_disk(const char* disk_img_fn, const char* filemode) {
    if (disk.is_mounted) {
        logMsg(
            ERROR_LOG, "open_disk: there is a mounted disk at %s; unmount it first", disk.img_fn);
        return -1;
    } else {
        logMsg(INFO_LOG, "open_disk: resetting stale disk state (not mounted)");
        free_disk();
    }
    if (strlen(disk_img_fn) >= sizeof(disk.img_fn)) {
        logMsg(
            ERROR_LOG,
            "open_disk: disk image path too long; maximum length: %zu",
            sizeof(disk.img_fn) - 1);
        free_disk();
        return -1;
    }
    snprintf(disk.img_fn, sizeof(disk.img_fn), "%s", disk_img_fn);
    logMsg(INFO_LOG, "open_disk: mounting disk at %s", disk_img_fn);
    disk.fp = fopen(disk_img_fn, filemode);
    if (disk.fp == NULL) {
        logMsg(ERROR_LOG, "open_disk: failed to open the disk image");
        free_disk();
        return -1;
    }
    return 0;
}

// -------------------------------------

int mount_fs(const char* disk_img_fn) {
    logMsg(INFO_LOG, "mount_fs: mounting disk %s", disk_img_fn);
    if (open_disk(disk_img_fn, "rb+") != 0) {
        logMsg(ERROR_LOG, "mount_fs: failed to open disk at %s", disk_img_fn);
        return -1;
    }
    disk.is_mounted = true;
    // Use 'fseeko' in case the disk image is too large (>2GB).
    if (fseeko(disk.fp, 0, SEEK_END) == 0) {
        off_t sz = ftello(disk.fp);
        if (sz >= 0) {
            disk.size = (size_t)sz;
        }
        fseeko(disk.fp, 0, SEEK_SET);
    }
    load_super_from_disk();
    load_bitmap_from_disk();
    return 0;
}

void unmount_fs() {
    if (!disk.is_mounted) {
        err_exit("unmount_fs: disk is not mounted");
    }
    if (disk.fp == NULL) {
        err_exit("unmount_fs: disk file pointer is NULL");
    }
    logMsg(INFO_LOG, "unmount_fs: unmounting disk %s", disk_img_fn());
    free_disk();
}

int create_disk_fs(const char* disk_img_fn, size_t size) {
    logMsg(INFO_LOG, "create_disk_fs: creating disk at %s", disk_img_fn);
    if (open_disk(disk_img_fn, "wb+") != 0) {
        logMsg(ERROR_LOG, "create_disk_fs: failed to open disk at %s", disk_img_fn);
        return -1;
    }
    if (ftruncate(fileno(disk.fp), (off_t)size) != 0) {
        logMsg(ERROR_LOG, "create_disk_fs: `ftruncate` failed");
        free_disk();
        return 1;
    }
    disk.size = size;
    disk.is_mounted = true;
    return 0;
}

const char* disk_img_fn() {
    return disk.img_fn;
}

size_t disk_size() {
    return disk.size;
}

bool disk_is_mounted() {
    return disk.is_mounted;
}

void require_disk_is_mounted() {
    if (!disk.is_mounted || disk.fp == NULL) {
        err_exit("require_disk_is_mounted: disk hasn't been mounted yet");
    }
}

size_t read_from_disk_at(void* buf, size_t size, size_t count, off_t offset) {
    diskseek(offset, SEEK_SET);
    return fread(buf, size, count, disk.fp);
}

size_t write_to_disk_at(const void* buf, size_t size, size_t count, off_t offset) {
    diskseek(offset, SEEK_SET);
    return fwrite(buf, size, count, disk.fp);
}

size_t read_from_disk(void* buf, size_t size, size_t count) {
    require_disk_is_mounted();
    return fread(buf, size, count, disk.fp);
}

size_t write_to_disk(const void* buf, size_t size, size_t count) {
    require_disk_is_mounted();
    return fwrite(buf, size, count, disk.fp);
}

// A wrap function over fseeko. Validates arguments; warns if seeking beyond current disk size.
int diskseek(off_t offset, int whence) {
    require_disk_is_mounted();
    off_t base, target;
    char whence_macro_name[10];
    switch (whence) {
        case SEEK_SET:
            base = 0;
            strcpy(whence_macro_name, "SEEK_SET");
            break;
        case SEEK_CUR:
            base = ftello(disk.fp);
            if (base < 0) {
                err_exit("diskseek: `ftello` failed");
            }
            strcpy(whence_macro_name, "SEEK_CUR");
            break;
        case SEEK_END:
            base = (off_t)disk.size;
            strcpy(whence_macro_name, "SEEK_END");
            break;
        default:
            logMsg(ERROR_LOG, "diskseek: invalid `whence` value: %d", whence);
            return -1;
    }
    target = base + offset;
    if (target < 0 || target > (off_t)disk.size) {
        logMsg(
            ERROR_LOG,
            "diskseek: position out of range: offset=%d\twhence=%s",
            offset,
            whence_macro_name);
        return -1;
    }
    if (fseeko(disk.fp, offset, whence) != 0) {
        logMsg(ERROR_LOG, "fseeko(%d, %s) failed.", offset, whence_macro_name);
    }
}

bool flush_disk() {
    require_disk_is_mounted();
    logMsg(INFO_LOG, "flush_disk: flushing the disk");
    if (fflush(disk.fp) != 0) {
        logMsg(ERROR_LOG, "flush_disk: `fflush` failed");
        return false;
    }
    if (fsync(fileno(disk.fp)) != 0) {
        logMsg(ERROR_LOG, "flush_disk: `fsync` failed");
        return false;
    }
    return true;
}

bool disk_error_occurred() {
    require_disk_is_mounted();
    return ferror(disk.fp) != 0;
}
