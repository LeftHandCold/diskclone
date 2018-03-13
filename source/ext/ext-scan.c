//
// Created by sjw on 2018/3/12.
//

#include "hdtd.h"
#include "ext.h"

static uint64_t
get_ext_bitmap_sector(hd_context *ctx, hd_disk *disk, ext_part *part, uint32_t group_index, ext_scan *scan)
{
    ext3_group_desc *group_desc;

    if (!scan->current_group_desc)
    {
        uint64_t sector;
        uint32_t desc_size = part->groups_count * sizeof(ext3_group_desc);
        desc_size = ((desc_size + 0xfff) & ~0xfff);
        if(!scan->group_desc || desc_size > scan->group_desc_size) {
            hd_free(ctx, scan->group_desc);
            scan->group_desc_size = desc_size;
            scan->group_desc = malloc(scan->group_desc_size);
        }
        scan->current_group_desc = scan->group_desc;
        sector = part->group_desc_sector;

        hd_read_write_device(ctx, part->super.dev_fd, false, scan->current_group_desc, sector * disk->sector_size, desc_size);
    }

    group_desc = (ext3_group_desc *)scan->current_group_desc + group_index;

    /*No initialization, no backup*/
    if (group_desc->bg_flags & EXT4_BG_BLOCK_UNINIT)
        return 0;

    return (uint64_t)(group_desc->bg_block_bitmap * (uint64_t)part->super.secperclr);
}

void
ext_scan_init(hd_context *ctx, hd_disk *disk, ext_part *part)
{
    uint64_t total;
    uint32_t buffer_size, bitmap_size;
    uint32_t group_index, block_size;
    buffer_size = bitmap_size = group_index = total = 0;

    block_size = part->super.secperclr * disk->sector_size;
    buffer_size = (((part->blocks_per_group + 7)/8 + block_size - 1) & ~(block_size - 1));
    bitmap_size = buffer_size / disk->sector_size;
    total = part->groups_count * bitmap_size;

    printf("*** blockspergroup(%d), blocksize %d. bitmapsize %d. first data block %d.\n",
           part->blocks_per_group , block_size,bitmap_size, part->first_data_block);

    if (buffer_size > part->super.scan_buffer_size)
    {
        hd_free(ctx, part->super.scan_buffer);
        part->super.scan_buffer_size = buffer_size;
        hd_try(ctx)
        {
            part->super.scan_buffer = hd_malloc(ctx, part->super.scan_buffer_size);
        }
        hd_catch(ctx)
        {
            part->super.scan_buffer_size = 0;
            hd_rethrow(ctx);
        }
    }

    ext_scan *scan;
    uint64_t cluster,fat_sector;
    uint32_t i, count;
    hd_try(ctx)
    {
        scan = hd_malloc(ctx, sizeof(ext_scan));
        memset(scan, 0, sizeof(ext_scan));
        while (total > 0)
        {
            count = bitmap_size;
            fat_sector = get_ext_bitmap_sector(ctx, disk, part, group_index, scan);
            printf("*** groupindex(%d), fatSector(%lld), count %d.\n", group_index, fat_sector, count);
            if (fat_sector == 0)
            {
                total -= count;
                group_index++;
                continue;
            }

            hd_read_write_device(ctx, part->super.dev_fd, false, part->super.scan_buffer,
                                 fat_sector * disk->sector_size , count * disk->sector_size);

            total -= count;
            fat_sector += count;

            //If the starting data block does not start from 0, the 0th data block is also backed up
            if (cluster ==0 && part->first_data_block != 0)
                hd_set_data_bitmap(ctx, &part->super, cluster * part->super.secperclr,
                                   COPY_BLOCK_SIZE);

            cluster = (uint64_t)group_index * part->blocks_per_group + part->first_data_block;

            if (total == 0)
                count = ((part->super.total_sector / part->super.secperclr) % part->blocks_per_group);
            else
                count = part->blocks_per_group;
            if (count == 0)
                count = part->blocks_per_group;
            count = (count + 7) / 8;

            printf("*** cluster %lld. count %d. secperclr %d sector %lld..\n",
                    cluster, count, part->super.secperclr, cluster * part->super.secperclr);
            for (i = 0; i<count; i++)
            {
                if (part->super.scan_buffer[i] != 0)
                {
                    if (cluster * part->super.secperclr < part->super.total_sector)
                    {
                        if (cluster * part->super.secperclr + COPY_BLOCK_SIZE > part->super.total_sector)
                        {
                            hd_set_data_bitmap(ctx, &part->super, cluster * part->super.secperclr,
                                               part->super.total_sector - cluster * part->super.secperclr);
                        }
                        else
                        {
                            hd_set_data_bitmap(ctx, &part->super, cluster * part->super.secperclr, COPY_BLOCK_SIZE);
                        }
                    }
                }
                cluster += 8;
            }
            group_index++;
        }
    }
    hd_always(ctx)
    {
        hd_free(ctx, scan->group_desc);
        scan->group_desc = NULL;
        scan->current_group_desc = NULL;
        hd_free(ctx, scan);
        scan = NULL;
    }
    hd_catch(ctx)
    {
        hd_rethrow(ctx);
    }

}