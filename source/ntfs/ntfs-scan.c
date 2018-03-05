//
// Created by sjw on 2018/3/5.
//
#include "hdtd.h"
#include "ntfs.h"

static void
ntfs_get_mft_head(hd_context *ctx, uint64_t addr, mft_head *mft, hd_disk *disk, ntfs_part *part)
{
    unsigned char buf[512];
    hd_read_write_device(ctx, part->dev_fd, false, buf, addr * disk->sector_size, disk->sector_size);
    memcpy(mft, buf, sizeof(mft_head));
}

static void
ntfs_search_root_dir(hd_context *ctx, hd_disk *disk, ntfs_part *part, ntfs_scan *scan)
{
    mft_head mft;
    uint64_t addr,size;
    memset(&mft, 0, sizeof(mft_head));

    addr = part->pntfs->MftStartLcn * part->pntfs->SecPerClr;
    ntfs_get_mft_head(ctx, addr, &mft, disk, part);

    scan->mft_size = mft.ulMFTAllocSize;

    if (scan->mft_size < 512 || (scan->mft_size % 512 != 0))
        hd_throw(ctx, HD_ERROR_GENERIC, "cannot find root, scan->mft_size is %d\n", scan->mft_size);


}

void
ntfs_scan_init(hd_context *ctx, hd_disk *disk, ntfs_part *part)
{
    ntfs_scan *scan;
    hd_var(scan);

    hd_try(ctx)
    {
        scan = hd_malloc(ctx, sizeof(ntfs_scan));
        ntfs_search_root_dir(ctx, disk, part, scan);
    }
    hd_always(ctx)
    {
        hd_free(ctx, scan);
    }
    hd_catch(ctx)
    {
        hd_rethrow(ctx);
    }

}