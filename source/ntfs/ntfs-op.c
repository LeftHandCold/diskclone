//
// Created by sjw on 2018/3/2.
//

#include "hdtd.h"
#include "ntfs.h"

static void
ntfs_init_bitmap(hd_context *ctx, ntfs_part *part)
{
    uint64_t total_block;
    total_block = (part->super.total_sector + COPY_BLOCK_SIZE - 1) / COPY_BLOCK_SIZE;
    part->bitmap_size = (uint32_t)(((total_block + 7)/8 + 0xfff) & ~0xfff);

    part->bitmap = hd_malloc(ctx, part->bitmap_size);
    if (!part->bitmap)
        hd_throw(ctx, HD_ERROR_GENERIC, "failed to malloc bitmap, size %d errno %d", part->bitmap_size, errno);
    memset(part->bitmap, 0, part->bitmap_size);
}

int
ntfs_probe_label(hd_context *ctx, ntfs_part *part)
{
    ntfs_bpb *pntfs = (ntfs_bpb *)part->super.scan_buffer;

    if (memcmp((char*)pntfs->OemID, "NTFS", 4) == 0)
    {
        part->super.total_sector = pntfs->TotalSectors;
        part->super.secperclr = pntfs->SecPerClr;
        part->super.type = NTFS;
        part->pntfs = pntfs;
        return 1;
    }
    return 0;
}

void
ntfs_part_clone(hd_context *ctx, hd_disk *disk, ntfs_part *part)
{
    ntfs_init_bitmap(ctx, part);

    ntfs_scan_init(ctx, disk, part);
}