//
// Created by sjw on 2018/2/28.
//
#include "hdtd.h"

extern hd_disk_handler gpt_disk_handler;
extern hd_disk_handler dos_disk_handler;

/**
 * hd_register_disk_handlers: Must first register the gpt handler
 * @param ctx
 */
void hd_register_disk_handlers(hd_context *ctx)
{
#if HD_ENABLE_GPT
    hd_register_disk_handler(ctx, &gpt_disk_handler);
#endif /* HD_ENABLE_GPT */
#if HD_ENABLE_DOS
    hd_register_disk_handler(ctx, &dos_disk_handler);
#endif /* HD_ENABLE_DOS */
}

extern hd_part_handler ntfs_part_handler;

void hd_register_part_handlers(hd_context *ctx)
{
#if HD_ENABLE_NTFS
    hd_register_part_handler(ctx, &ntfs_part_handler);
#endif /* HD_ENABLE_NTFS */
}
