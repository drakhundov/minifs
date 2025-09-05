# MiniFS

A simple file system implementation. I used **1024** bytes per block with **1024** blocks by default, but it could be manually set. I have also implemented logging for best practice.

Programming language: **C**.

## Overview

### SuperBlock

There is only one *SuperBlock* on the entire disk. It serves as metadata, storing the following information:

**Magic number** - tells the OS which file system was used to format the disk. Each FS must have a unique **magic number**.

**Number of blocks and block size** - used to navigate throughout the disk.

**Bitmap, inode, and data starting positions** - tells at which block each logical section of the disk starts.

### Inode

Represents a file’s (including directories) metadata, except for name.

**Flags** - permissions; whether the file represents a directory, whether inode is valid (after deleting, it would simply be removed from the general structure and its valid flag set to zero).

**Size** - tells how many bytes of data is written into the file. For directories, it’s the number of directory entries.

**Data blocks** - an array of references to data blocks, allocated separately. This is where the actual file content is stored. For directories, it stores an array of directory entries.

### DirectoryEntry

Used to store the file system structure and connect inodes to names.

**Inode**

**Name** - a string

## Build

To build locally, must have *clang* and *make* installed on your system.

To install dependencies:

```bash
brew install llvm make
```

To run **MiniFS**:

```bash
make && make run
```