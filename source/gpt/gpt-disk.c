//
// Created by sjw on 2018/3/1.
//

#include "gpt.h"

static void
gpt_read_first_sector(hd_context *ctx, gpt_disk *disk)
{
    disk->sector_size = DEFAULT_SECTOR_SIZE;
    disk->first_sector = hd_calloc(ctx, 1, disk->sector_size);
    if (!disk->first_sector)
    {
        disk->first_sector = NULL;
        return;
    }
    memset(disk->first_sector, 0, disk->sector_size);
    hd_read_write_device(ctx, disk->dev_fd, false, disk->first_sector, 0, disk->sector_size);
}

static void
gpt_drop_disk_imp(hd_context *ctx, gpt_disk *disk)
{
    hd_try(ctx)
    {
        if (disk->first_sector != NULL)
            hd_free(ctx, disk->first_sector);

        free(disk->ents);
        free(disk->pheader);

        disk->ents = NULL;
        disk->pheader = NULL;

        SAFE_DISK_CLOSE(disk->dev_fd);
    }
    hd_catch(ctx)
        hd_rethrow(ctx);
}

static gpt_disk *
gpt_new_disk(hd_context *ctx, int dev_fd)
{
    gpt_disk *disk = hd_new_derived_disk(ctx, gpt_disk);
    disk->super.drop_disk = (hd_disk_drop_fn *)gpt_drop_disk_imp;
    disk->super.probe_disk = (hd_disk_probe_fn *)gpt_probe_label;

    disk->dev_fd = dev_fd;

    return disk;
}

gpt_disk *
gpt_open_disk (hd_context *ctx, const char *diskname)
{
    gpt_disk *disk = NULL;
    int dev_fd;
    hd_var(disk);

    hd_try(ctx)
    {
        dev_fd = hd_open_dev(ctx, diskname);
        disk = gpt_new_disk(ctx, dev_fd);
        gpt_read_first_sector(ctx, disk);
    }
    hd_catch(ctx)
    {
        hd_drop_disk(ctx, &disk->super);
        hd_rethrow(ctx);
    }
    return disk;
}

int
gpt_recognize(hd_context *doc, const char *magic)
{
    return 1;
}

hd_disk_handler gpt_disk_handler =
{
   gpt_recognize,
   (hd_disk_open_fn *) gpt_open_disk
};