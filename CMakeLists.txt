cmake_minimum_required(VERSION 4.0)
project(minifs C)

set(CMAKE_C_STANDARD 11)

add_library(minifs_lib
        src/allocator.c
        src/dir.c
        src/disk.c
        src/err.c
        src/fs.c
        src/inode.c
        src/logging.c
        src/path.c
        src/super.c
)

target_include_directories(minifs_lib PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/on-disk"
)

add_executable(minifs_app
        src/main.c
)

target_link_libraries(minifs_app PUBLIC
        minifs_lib
)
