//
// Created by sjw on 2018/2/28.
//

#include "hdtd.h"
#include "dos.h"

#define AIX_MAGIC_STRING	"\xC9\xC2\xD4\xC1"
#define AIX_MAGIC_STRLEN	(sizeof(AIX_MAGIC_STRING) - 1)

int dos_probe_label(hd_context *ctx, dos_disk *disk)
{
    if (memcmp(disk->first_sector, AIX_MAGIC_STRING, AIX_MAGIC_STRLEN) == 0)
        return 0;

    if (!mbr_is_valid_magic(disk->first_sector))
        return 0;

    disk->super.nparts_max = 4;

    return 1;
}

static bool
verify_partition(com_parptition *prt)
{
    if ((prt->pid == 0) ||
        ((prt->id != 0x80) && (prt->id != 0)) ||
            (prt->totalsectors <= 63 || prt->totalsectors >= 0x0ffffff00))
        return false;

    return true;
}

static uint8_t
parse_extend_partition(hd_context *ctx, dos_disk *disk, com_parptition *prt, uint8_t curpart, uint32_t beginsector)
{
    /*TODO*/
    return 0;
}

static bool
parse_master_partition(hd_context *ctx, dos_disk *disk, com_parptition *prt, uint8_t curpart, uint32_t beginsector)
{
    if (!verify_partition(prt))
        return false;

    disk->super.volume[curpart].beginsector = beginsector + prt->relativeSectors;
    disk->super.volume[curpart].partition_info_sector = beginsector;
    disk->super.volume[curpart].total_sectors = prt->totalsectors;
    return true;
}

int dos_get_volume_label(hd_context *ctx, dos_disk *disk)
{
    com_mbr *mbr = (com_mbr *)disk->first_sector;

    uint8_t ret,curpart;
    curpart = 0;

    for (int i = 0; i < 4; i++)
    {
        ret = 0;
        if ( mbr->partition[i].pid == 0x05 || mbr->partition[i].pid == 0x0f )
        {
            ret = parse_extend_partition(ctx, disk, &(mbr->partition[i]), curpart, 0);
            if (ret)
                curpart = ret;
        } else if (mbr->partition[i].pid == 0x42)
        {
            ret = parse_extend_partition(ctx, disk, &(mbr->partition[i]), curpart, 0);
            if (ret)
                curpart = ret;
            else
            {
                if (parse_master_partition(ctx, disk, &(mbr->partition[i]), curpart, 0))
                    curpart++;
            }
        } else if (mbr->partition[i].pid != 0 && mbr->byEnding == 0xAA55)
        {
            if (parse_master_partition(ctx, disk, &(mbr->partition[i]), curpart, 0))
                curpart++;
        }
    }

    return curpart;
}