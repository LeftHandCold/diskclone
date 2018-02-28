//
// Created by sjw on 2018/2/28.
//

#include "hdtd.h"
#include "hdtd-imp.h"

void *
hd_new_disk_of_size(hd_context *ctx, int size)
{
    hd_disk *disk = hd_calloc(ctx, 1, size);
    disk->refs = 1;
    return disk;
}

int
hd_open_dev(hd_context *ctx, const char *diskname)
{
    int dev_fd;
    dev_fd = open(diskname, O_RDWR);
    return dev_fd;
}

hd_disk *
hd_keep_disk(hd_context *ctx, hd_disk *disk)
{
    return hd_keep_imp(ctx, disk, &disk->refs);
}

void
hd_drop_disk(hd_context *ctx, hd_disk *disk)
{
    if (hd_drop_imp(ctx, disk, &disk->refs))
    {
        if (disk->drop_disk)
            disk->drop_disk(ctx, disk);
        hd_free(ctx, disk);
    }
}