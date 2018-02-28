//
// Created by sjw on 2018/2/28.
//
#include "hdtd.h"

extern hd_disk_handler dos_disk_handler;

void hd_register_disk_handlers(hd_context *ctx)
{
#if HD_ENABLE_DOS
    hd_register_disk_handler(ctx, &dos_disk_handler);
#endif /* HD_ENABLE_DOS */
}