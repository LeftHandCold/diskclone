//
// Created by sjw on 2018/2/28.
//
#include "dos.h"

static void
pdf_drop_disk_imp(hd_context *ctx, dos_disk *disk)
{
    hd_try(ctx)
    {

    }
    hd_catch(ctx)
        hd_rethrow(ctx);
}

static dos_disk *
dos_new_disk(hd_context *ctx, int dev_fd)
{
    dos_disk *disk = hd_new_derived_disk(ctx, dos_disk);
    disk->super.drop_disk = (hd_disk_drop_fn *)pdf_drop_disk_imp;
    disk->super.probe_disk = (hd_disk_probe_fn *)dos_probe_label;

    disk->dev_fd = dev_fd;

    return disk;
}

dos_disk *
dos_open_disk(hd_context *ctx, const char *diskname)
{
    dos_disk *disk = NULL;
    int dev_fd;
    hd_var(disk);

    hd_try(ctx)
    {
        dev_fd = hd_open_dev(ctx, diskname);
        disk = dos_new_disk(ctx, dev_fd);
    }
    hd_catch(ctx)
    {
        hd_drop_disk(ctx, &disk->super);
        hd_rethrow(ctx);
    }
    return disk;
}

int
dos_recognize(hd_context *doc, const char *magic)
{
    return 1;
}

hd_disk_handler dos_disk_handler =
{
    dos_recognize,
    (hd_disk_open_fn *) dos_open_disk
};