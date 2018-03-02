//
// Created by sjw on 2018/2/28.
//

#include "hdtd.h"
#include "hdtd-imp.h"

enum
{
    HD_DISK_HANDLER_MAX = 10
};

struct hd_disk_handler_context_s
{
    int refs;
    int count;
    const hd_disk_handler *handler[HD_DISK_HANDLER_MAX];
};

void hd_new_disk_handler_context(hd_context *ctx)
{
    ctx->disk = hd_malloc_struct(ctx, hd_disk_handler_context);
    ctx->disk->refs = 1;
    ctx->disk->count = 0;
}

void hd_register_disk_handler(hd_context *ctx, const hd_disk_handler *handler)
{
    hd_disk_handler_context *dc;
    int i;

    if (!handler)
        return;

    dc = ctx->disk;
    if (dc == NULL)
        hd_throw(ctx, HD_ERROR_GENERIC, "Document handler list not found");

    for (i = 0; i < dc->count; i++)
        if (dc->handler[i] == handler)
            return;

    if (dc->count >= HD_DISK_HANDLER_MAX)
        hd_throw(ctx, HD_ERROR_GENERIC, "Too many document handlers");

    dc->handler[dc->count++] = handler;
}

int
hd_open_dest_disk(hd_context *ctx, hd_disk *disk, const char *diskname)
{
    int dev_fd;
    dev_fd = hd_open_dev(ctx, diskname);
    if (dev_fd < 0)
        return 0;

    hd_disk_dest *disk_dest = hd_new_derived_disk(ctx, hd_disk_dest);

    disk_dest->dev_fd = dev_fd;
    disk_dest->name = diskname;
    disk_dest->refs = 1;
    disk_dest->size = 0;

    disk->disk_dest = disk_dest;

    return 1;
}

hd_disk *
hd_open_disk(hd_context *ctx, const char *src, const char *dest)
{
    int i;
    hd_disk_handler_context *dh;

    if ((src == NULL) || (dest == NULL))
        hd_throw(ctx, HD_ERROR_GENERIC, "no disk to open");

    dh = ctx->disk;
    if (dh->count == 0)
        hd_throw(ctx, HD_ERROR_GENERIC, "No disk handlers registered");

    for (i = 0; i < dh->count; i++)
    {
        int rc;
        hd_disk *disk;
        disk = dh->handler[i]->open(ctx, src);

        rc = disk->probe_disk(ctx, disk);


        if(rc != 1)
        {
            hd_drop_disk(ctx, disk);
            continue;
        }

        hd_try(ctx)
            hd_open_dest_disk(ctx, disk, dest);
        hd_catch(ctx)
        {
            hd_drop_disk(ctx, disk);
            hd_rethrow(ctx);
        }
        return disk;
    }

    hd_throw(ctx, HD_ERROR_GENERIC, "Could not find the specified disk handler");
}

void *
hd_new_disk_of_size(hd_context *ctx, int size)
{
    hd_disk *disk = hd_calloc(ctx, 1, size);
    disk->refs = 1;
    return disk;
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
        hd_drop_dest_disk(ctx, disk->disk_dest);
        hd_free(ctx, disk);
    }
}

void
hd_drop_dest_disk(hd_context *ctx, hd_disk_dest *disk)
{
    if (hd_drop_imp(ctx, disk, &disk->refs))
    {
        SAFE_DISK_CLOSE(disk->dev_fd);
        hd_free(ctx, disk);
    }
}

void
hd_drop_disk_handler_context(hd_context *ctx)
{
    if (!ctx)
        return;

    if (hd_drop_imp(ctx, ctx->disk, &ctx->disk->refs))
    {
        hd_free(ctx, ctx->disk);
        ctx->disk = NULL;
    }
}