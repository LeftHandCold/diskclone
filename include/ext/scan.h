//
// Created by sjw on 2018/3/12.
//

#ifndef DISKCLONE_EXT_SCAN_H
#define DISKCLONE_EXT_SCAN_H

typedef struct ext_scan_s ext_scan;
struct ext_scan_s
{
    uint32_t group_desc_size;
    char *group_desc;
    char *current_group_desc;
};

void ext_scan_init(hd_context *ctx, hd_disk *disk, ext_part *part);
#endif //DISKCLONE_EXT_SCAN_H
