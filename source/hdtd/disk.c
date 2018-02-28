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
    ctx->src = hd_malloc_struct(ctx, hd_disk_handler_context);
    ctx->src->refs = 1;
    ctx->src->count = 0;
}

void hd_register_disk_handler(hd_context *ctx, const hd_disk_handler *handler)
{
    hd_disk_handler_context *dc;
    int i;

    if (!handler)
        return;

    dc = ctx->src;
    if (dc == NULL)
        hd_throw(ctx, HD_ERROR_GENERIC, "Document handler list not found");

    for (i = 0; i < dc->count; i++)
        if (dc->handler[i] == handler)
            return;

    if (dc->count >= HD_DISK_HANDLER_MAX)
        hd_throw(ctx, HD_ERROR_GENERIC, "Too many document handlers");

    dc->handler[dc->count++] = handler;
}

hd_disk *
hd_open_disk(hd_context *ctx, const char *diskname)
{
    int i;
    hd_disk_handler_context *dh;

    if (diskname == NULL)
        hd_throw(ctx, HD_ERROR_GENERIC, "no disk to open");

    dh = ctx->src;
    if (dh->count == 0)
        hd_throw(ctx, HD_ERROR_GENERIC, "No disk handlers registered");

    for (i = 0; i < dh->count; i++)
    {
        int rc;
        hd_disk *disk;
        disk = dh->handler[i]->open(ctx, diskname);

        rc = disk->probe_disk(ctx, disk);

        if(rc != 1)
        {
            hd_drop_disk(ctx, disk);
            continue;
        }

        return disk;
    }

    return NULL;
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
        hd_free(ctx, disk);
    }
}

void hd_drop_disk_handler_context(hd_context *ctx)
{
    if (!ctx)
        return;

    if (hd_drop_imp(ctx, ctx->src, &ctx->src->refs))
    {
        hd_free(ctx, ctx->src);
        ctx->src = NULL;
    }
}