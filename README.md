# MiniFS

A simple file system implementation. Uses **1024** bytes per block with **1024** blocks by default, but it could be manually set. Uses logging for best practice. Implemented in **C**.

**Purpose**: better understanding of operating systems.

## Overview

### SuperBlock

There is supposed to be only one *SuperBlock* in the entire disk. It serves as metadata, storing following information:

- **Magic number** - tells the OS which file system was used to format the disk. Each FS must have a unique **magic number**.
- **Number of blocks and block size** - used to navigate throughout the disk.
- **Bitmap, inode, and data starting positions** - tells at which block each logical section of the disk starts.

### Inode

Represents a file’s (including directories) metadata, except for name.

- **Flags** - permissions; whether the file represents a directory, whether inode is valid (after deleting, it would simply be removed from the general structure and its valid flag set to zero).
- **Size** - tells how many bytes of data is written into the file. For directories, it’s the number of directory entries.
- **Data blocks** - an array of references to data blocks, allocated separately. This is where the actual file content is stored. For directories, it stores an array of directory entries.
- **Owner ID**

### DirectoryEntry

Used to store the file system structure and connect inodes to names.

- **Inode**
- **Name**

## Usage

**Must have *clang* and *make* installed on your system.**

If not, run

```bash
brew install llvm make
```

To run **MiniFS**:

```bash
make && make run
```