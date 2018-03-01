//
// Created by sjw on 2018/3/1.
//

#ifndef DISKCLONE_HDTD_CRC32_H
#define DISKCLONE_HDTD_CRC32_H

#include "hdtd/system.h"
#include "hdtd/context.h"

/*
 * This a generic crc32() function, it takes seed as an argument,
 * and does __not__ xor at the end. Then individual users can do
 * whatever they need.
 */
uint32_t crc32(uint32_t seed, const unsigned char *buf, size_t len);

#endif //DISKCLONE_HDTD_CRC32_H
