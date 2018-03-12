//
// Created by sjw on 2018/3/12.
//

#include "hdtd.h"
#include "ext.h"

int
ext_probe_label(hd_context *ctx, hd_disk *disk, ext_part *part)
{
    /*get ext super block*/
    ext3_super_block *pext = (ext3_super_block *)(part->super.scan_buffer + 1024);

    if (*(uint16_t *)(part->super.scan_buffer + 0x1fe) == 0xaa55)
        return 0;

    if (pext->s_magic != 0xEF53)
        return 0;

    part->super.secperclr = (1 << (pext->s_log_block_size + 1));
    part->super.total_sector = (uint64_t)pext->s_blocks_count * part->super.secperclr;

    if (part->super.secperclr <= 2)
        part->group_desc_sector = part->super.secperclr * 2;
    else
        part->group_desc_sector = part->super.secperclr;

    part->blocks_per_group = pext->s_blocks_per_group;

    part->groups_count = ((uint64_t)pext->s_blocks_count + (uint64_t)pext->s_blocks_per_group - 1) /
                         pext->s_blocks_per_group;

    part->free_sectors = pext->s_free_blocks_count * part->super.secperclr;
    part->first_data_block = pext->s_first_data_block;

    /*FIXME:Associate the partition number in a simple way,without considering LVM*/
    int index = atoi(&part->super.name[strlen(part->super.name) - 1]);
    part->super.beginsector = disk->volume[index - 1].beginsector;
    part->super.partition_info_sector = disk->volume[index - 1].beginsector;

    return 1;
}

void
ext_part_clone(hd_context *ctx, hd_disk *disk, ext_part *part)
{
    hd_init_bitmap(ctx, &part->super);

    ext_scan_init(ctx, disk, part);

    hd_pthread_init(ctx, disk, &part->super);

    hd_producer_create(ctx, disk, &part->super);
    hd_consumer_create(ctx, disk, &part->super);

    hd_while_pthread(ctx);
    hd_pthread_deinit(ctx);
}