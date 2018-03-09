//
// Created by sjw on 2018/2/28.
//

#include "hdtd.h"

void main()
{
    hd_context *ctx;
    hd_disk *disk;
    hd_part *part;
    ctx = hd_new_context(NULL);
    if (!ctx)
    {
        fprintf(stderr, "cannot create diskclone context\n");
        return ;
    }

    /* Register the default disk types to handle. */
    hd_try(ctx)
        hd_register_disk_handlers(ctx);
    hd_catch(ctx)
    {
        fprintf(stderr, "cannot register disk handlers: %s\n", hd_caught_message(ctx));
        hd_drop_context(ctx);
        return;
    }
    /* Open the part. */
    hd_try(ctx)
        disk = hd_open_disk(ctx, "F:/disk/mbr", "F:/disk/test");
    hd_catch(ctx) {
        fprintf(stderr, "cannot open disk: %s\n", hd_caught_message(ctx));
        hd_drop_context(ctx);
        return ;
    }
    /* Register the default part types to handle. */
    hd_try(ctx)
        hd_register_part_handlers(ctx);
    hd_catch(ctx)
    {
        fprintf(stderr, "cannot register part handlers: %s\n", hd_caught_message(ctx));
		hd_drop_disk(ctx, disk);
        hd_drop_context(ctx);
        return;
    }

    /* Open the disk. */
    hd_try(ctx)
        part = hd_open_part(ctx, disk, "F:/disk/ntfs1");
    hd_catch(ctx) {
        fprintf(stderr, "cannot open part: %s\n", hd_caught_message(ctx));
        hd_drop_disk(ctx, disk);
        hd_drop_context(ctx);
        return ;
    }

    hd_try(ctx)
    {
        hd_clone_part(ctx, disk, part);
        hd_clone_part_info(ctx, disk, part);
    }
    hd_catch(ctx)
    {
        fprintf(stderr, "cannot clone part: %s\n", hd_caught_message(ctx));
        hd_drop_part(ctx, part);
        hd_drop_disk(ctx, disk);
        hd_drop_context(ctx);
        return;
    }

	hd_drop_part(ctx, part);
    hd_drop_disk(ctx, disk);
	hd_drop_context(ctx);

    printf("main is end\n");
}