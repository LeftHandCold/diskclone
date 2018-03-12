//
// Created by sjw on 2018/3/12.
//

#include "hdtd.h"
#include "ext.h"

static void
ext_drop_part_imp(hd_context *ctx, ext_part *part)
{
    hd_try(ctx)
    {
        SAFE_DEV_CLOSE(part->super.dev_fd);
        hd_free(ctx, part->super.bitmap);
        hd_free(ctx, part->super.scan_buffer);
    }
    hd_catch(ctx)
    {
        hd_rethrow(ctx);
    }
}

static ext_part *
ext_new_part(hd_context *ctx, int dev_fd)
{
    ext_part *part = hd_new_derived_disk(ctx, ext_part);
    part->super.drop_part = (hd_part_drop_fn *)ext_drop_part_imp;
    part->super.probe_part = (hd_part_probe_fn *)ext_probe_label;
    part->super.clone_part = (hd_part_clone_fn *)ext_part_clone;

    part->super.scan_buffer_size = SCAN_BUFFER_SIZE;
    part->super.scan_buffer = hd_malloc(ctx, part->super.scan_buffer_size);


    part->super.dev_fd = dev_fd;

    return part;
}

ext_part *
ext_open_part (hd_context *ctx, hd_disk *disk, const char *partname)
{
    ext_part *part = NULL;

    int dev_fd;
    hd_var(part);

    hd_try(ctx)
    {
        dev_fd = hd_open_dev(ctx, partname);
        part = ext_new_part(ctx, dev_fd);
        hd_read_write_device(ctx, part->super.dev_fd, false, part->super.scan_buffer, 0, 8 * disk->sector_size);

    }
    hd_catch(ctx)
    {
        hd_drop_part(ctx, &part->super);
        hd_rethrow(ctx);
    }
    return part;
}

int
ext_recognize(hd_context *ctx, const char *magic)
{
    return 1;
}

hd_part_handler ext_part_handler =
{
        ext_recognize,
        (hd_part_open_fn *) ext_open_part
};
