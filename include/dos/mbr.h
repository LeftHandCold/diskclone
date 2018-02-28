//
// Created by sjw on 2018/2/28.
//

#ifndef DISKCLONE_DOS_MBR_H
#define DISKCLONE_DOS_MBR_H

static inline int mbr_is_valid_magic(const unsigned char *mbr)
{
    return mbr[510] == 0x55 && mbr[511] == 0xaa ? 1 : 0;
}

int dos_probe_label(hd_context *ctx, dos_disk *disk);

#endif //DISKCLONE_DOS_MBR_H
