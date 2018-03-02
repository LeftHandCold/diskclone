cmake_minimum_required(VERSION 3.8)
project(diskclone)

include_directories("include")

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c include/hdtd/context.h include/hdtd/system.h include/memento.h include/hdtd.h source/hdtd/error.c include/hdtd/config.h include/hdtd/version.h include/hdtd/string-util.h source/hdtd/string.c include/hdtd/disk.h include/hdtd/partition.h main.c source/dos/dos-disk.c include/dos.h include/dos/disk.h include/dos/mbr.h source/dos/dos-op.c source/hdtd/disk.c source/hdtd/memory.c source/hdtd/hdtd-imp.h source/hdtd/disk-op.c source/hdtd/disk-all.c source/hdtd/context.c include/gpt/disk.h include/gpt.h source/gpt/gpt-disk.c include/gpt/gpt.h source/gpt/gpt-op.c include/hdtd/bitops.h include/hdtd/crc32.h source/hdtd/crc32.c source/hdtd/partition.c)
add_executable(diskclone ${SOURCE_FILES})