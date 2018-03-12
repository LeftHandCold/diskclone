//
// Created by sjw on 2018/3/2.
//

#include "hdtd.h"
#include "ntfs.h"

int
ntfs_probe_label(hd_context *ctx, hd_disk *disk, ntfs_part *part)
{
    ntfs_bpb *pntfs = (ntfs_bpb *)part->super.scan_buffer;

    if (memcmp((char*)pntfs->OemID, "NTFS", 4) == 0)
    {
        part->super.total_sector = pntfs->TotalSectors;
        part->super.secperclr = pntfs->SecPerClr;
        part->super.type = NTFS;
        part->pntfs = pntfs;

        /*FIXME:Associate the partition number in a simple way,without considering LVM*/
        int index = atoi(&part->super.name[strlen(part->super.name) - 1]);
        part->super.beginsector = disk->volume[index - 1].beginsector;
        part->super.partition_info_sector = disk->volume[index - 1].beginsector;

        return 1;
    }
    return 0;
}

void
ntfs_part_clone(hd_context *ctx, hd_disk *disk, ntfs_part *part)
{
    hd_init_bitmap(ctx, &part->super);

    ntfs_scan_init(ctx, disk, part);

    hd_pthread_init(ctx, disk, &part->super);

    hd_producer_create(ctx, disk, &part->super);
    hd_consumer_create(ctx, disk, &part->super);

    hd_while_pthread(ctx);
    hd_pthread_deinit(ctx);
}