//
// Created by sjw on 2018/2/28.
//

#ifndef DISKCLONE_DOS_DISK_H
#define DISKCLONE_DOS_DISK_H

typedef struct dos_disk_s dos_disk;

struct dos_disk_s
{
    hd_disk super;

    int dev_fd;         /* device descriptor */
};

#endif //DISKCLONE_DOS_DISK_H
