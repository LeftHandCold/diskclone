cmake_minimum_required(VERSION 3.8)
project(diskclone)

include_directories("include")

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c include/hdtd/context.h include/hdtd/system.h include/memento.h include/hdtd.h source/hdtd/error.c include/hdtd/config.h include/hdtd/version.h include/hdtd/string-util.h source/hdtd/string.c)
add_executable(diskclone ${SOURCE_FILES})