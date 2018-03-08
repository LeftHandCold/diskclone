//
// Created by sjw on 2018/3/2.
//

#include "ntfs.h"

static void
ntfs_drop_part_imp(hd_context *ctx, ntfs_part *part)
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

static ntfs_part *
ntfs_new_part(hd_context *ctx, int dev_fd)
{
    ntfs_part *part = hd_new_derived_disk(ctx, ntfs_part);
    part->super.drop_part = (hd_part_drop_fn *)ntfs_drop_part_imp;
    part->super.probe_part = (hd_part_probe_fn *)ntfs_probe_label;
    part->super.clone_part = (hd_part_clone_fn *)ntfs_part_clone;

    part->super.scan_buffer_size = SCAN_BUFFER_SIZE;
    part->super.scan_buffer = hd_malloc(ctx, part->super.scan_buffer_size);


    part->super.dev_fd = dev_fd;

    return part;
}

ntfs_part *
ntfs_open_part (hd_context *ctx, hd_disk *disk, const char *partname)
{
    ntfs_part *part = NULL;

    int dev_fd;
    hd_var(part);

    hd_try(ctx)
    {
        dev_fd = hd_open_dev(ctx, partname);
        part = ntfs_new_part(ctx, dev_fd);
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
ntfs_recognize(hd_context *ctx, const char *magic)
{
    return 1;
}

hd_part_handler ntfs_part_handler =
{
    ntfs_recognize,
    (hd_part_open_fn *) ntfs_open_part
};