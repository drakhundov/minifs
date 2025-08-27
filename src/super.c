#include "super.h"

#include <stdio.h>

#include "disk.h"
#include "on-disk/super.h"

static SuperBlock sb;

void load_superblock() {
    require_disk_is_mounted();
    read_from_disk_at((void*)&sb, sizeof(SuperBlock), 1, 0);
}

void write_superblock() {
    require_disk_is_mounted();
    write_to_disk_at((void*)&sb, sizeof(SuperBlock), 1, 0);
}