//
// Created by sjw on 2018/2/28.
//

#include "hdtd.h"

void main()
{
    hd_context *ctx;
    hd_disk *disk;
    ctx = hd_new_context(NULL);
    if (!ctx)
    {
        fprintf(stderr, "cannot create diskclone context\n");
        return ;
    }

    /* Register the default file types to handle. */
    hd_try(ctx)
        hd_register_disk_handlers(ctx);
    hd_catch(ctx)
    {
        fprintf(stderr, "cannot register disk handlers: %s\n", hd_caught_message(ctx));
        hd_drop_context(ctx);
        return;
    }

    /* Open the disk. */
    hd_try(ctx)
        disk = hd_open_disk(ctx, "/dev/sdb");
    hd_catch(ctx) {
        fprintf(stderr, "cannot open document: %s\n", hd_caught_message(ctx));
        hd_drop_context(ctx);
        return ;
    }
    printf("main is end\n");
}