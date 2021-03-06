//
// Created by sjw on 2018/3/5.
//
#include "hdtd.h"
#include "ntfs.h"

/**
 *  Parse runs.Get the start cluster and size of each run in runs
 * @param runs
 * @param addr
 * @param size
 * @param part
 */
static void
ntfs_parse_runs(unsigned char* runs, int64_t *addr, uint64_t *size, ntfs_part *part)
{
    uint16_t	cbsize, cbaddr;
    cbsize = LOB(runs[0]);
    cbaddr = HIB(runs[0]);

    runs++;

    switch (cbsize)
    {
        case 1:
            *size = (uint64_t) *((unsigned char*)runs);
            runs += 1;
            break;
        case 2:
            *size = (uint64_t) *((uint16_t*)runs);
            runs += 2;
            break;
        case 3:
            *size = (uint64_t) *((uint32_t*)runs);
            *size &= 0xFFFFFF;
            runs += 3;
            break;
        case 4:
            *size = (uint64_t) *((uint32_t*)runs);
            runs += 4;
            break;
        case 5:
            *size = ((uint64_t)*runs);
            *size &= 0xFFFFFFFFFFLL;
            runs += 5;
            break;
        case 6:
            *size = ((uint64_t)*runs);
            *size &= 0xFFFFFFFFFFFFLL;
            runs += 6;
            break;
        case 7:
            *size = ((uint64_t)*runs);
            *size &= 0xFFFFFFFFFFFFFFLL;
            runs += 7;
            break;
        case 8:
            *size = ((uint64_t)*runs);
            runs += 8;
            break;
        default:
            *size = 0;
            runs += cbsize;
    }

    switch (cbaddr)
    {
        case 1:
            *addr = (int64_t) *((char*)runs);
            break;
        case 2:
            *addr = (int64_t) *((short*)runs);
            break;
        case 3:
        {
            long lAddr;
            lAddr = *((unsigned long*)runs);

            if (0 != (lAddr & 0x800000))
            {
                *addr = lAddr |= 0xFF000000;
            }
            else
            {

                *addr = lAddr &= 0xFFFFFF;
            }
        }
            break;
        case 4:
            *addr = (int64_t) *((long*)runs);
            break;
        case 5:
        {
            int64_t lAddr;
            lAddr = *((uint64_t *)runs);

            if (0 != (lAddr & 0x8000000000LL))
            {
                *addr = lAddr |= 0xFFFFFF0000000000LL;
            }
            else
            {

                *addr = lAddr &= 0xFFFFFFFFFFLL;
            }

        }
        case 6:
        {
            int64_t lAddr;
            lAddr = *((uint64_t *)runs);

            if (0 != (lAddr & 0x800000000000LL))
            {
                *addr = lAddr |= 0xFFFF000000000000LL;
            }
            else
            {

                *addr = lAddr &= 0xFFFFFFFFFFFFLL;
            }

        }
        case 7:
        {
            int64_t lAddr;
            lAddr = *((uint64_t *)runs);

            if (0 != (lAddr & 0x80000000000000LL))
            {
                *addr = lAddr |= 0xFF00000000000000LL;
            }
            else
            {

                *addr = lAddr &= 0xFFFFFFFFFFFFFFLL;
            }

        }
        case 8:
            *addr = (int64_t) *((int64_t*)runs);
            break;
        default:
            *addr = 0;
            runs += cbaddr;
            break;
    }

    *size *= part->pntfs->SecPerClr;

}

static void
ntfs_parse_bitmap_record(hd_context *ctx, hd_disk *disk, ntfs_part *part, ntfs_scan *scan)
{
    mft_head mft;
    uint64_t addr;
    unsigned char buf[512];
    unsigned char	runs[MAX_RUNS];
    memset(&mft, 0, sizeof(mft_head));

    /* get $MFT head */
    addr = part->pntfs->MftStartLcn * part->pntfs->SecPerClr;
    hd_read_write_device(ctx, part->super.dev_fd, false, buf, addr * disk->sector_size, disk->sector_size);
    memcpy(&mft, buf, sizeof(mft_head));

    /*The size of each MFT is exactly the 2 sector*/
    scan->mft_size = mft.ulMFTAllocSize;

    if (scan->mft_size < 512 || (scan->mft_size % 512 != 0))
        hd_throw(ctx, HD_ERROR_GENERIC, "cannot find root, scan->mft_size is %d\n", scan->mft_size);

    /*get $BITMAP record*/
    scan->mft_record = (unsigned char *)hd_malloc(ctx, scan->mft_size);
    memset(scan->mft_record , 0, scan->mft_size);
    hd_read_write_device(ctx, part->super.dev_fd, false, scan->mft_record,
                         addr *  disk->sector_size + BITMAP_RECORD * scan->mft_size, scan->mft_size);

    /*get file runs*/
    mft_head *bitmap_head;
    mft_attr *attr;
    bitmap_head = (mft_head *)scan->mft_record;
    attr = (mft_attr *)(scan->mft_record + bitmap_head->usAttrOffset);

    while ($DATA != attr->dwAttrType && \
		0 != attr->dwAttrType && -1L != attr->dwAttrType)
        attr = (mft_attr *)((unsigned char*)attr + attr->usAttrSize);
    if ($DATA != attr->dwAttrType || 0 == attr->bISResident)
        hd_throw(ctx, HD_ERROR_GENERIC, "cannot find bitmap attribute of the $DATA");

    int size = (attr->usAttrSize) - (attr->unAttrib.NonResidAttr.usNrDataOffset);

    unsigned char* pruns = (unsigned char*)runs;
    memcpy(pruns, (unsigned char *)(attr)+attr->unAttrib.NonResidAttr.usNrDataOffset, size);
    /*end get file runs*/

    /*Init the bitmap_position*/
    pruns[MAX_RUNS - 1] = '\0';
    int64_t raddr = 0;
    uint64_t rsize, taddr;
    int16_t iseek;
    taddr = 0;
    memset(&part->bitmap_position, 0, sizeof(part->bitmap_position));
    uint32_t *p = part->bitmap_position;
    while (0 != pruns[0])
    {
        iseek = HIB(pruns[0]) + LOB(pruns[0]) + 1;
        ntfs_parse_runs(pruns, &raddr, &rsize, part);
        if (0 == raddr && 0 != rsize)
            hd_throw(ctx, HD_ERROR_GENERIC, "cannot parse runs");

        taddr += raddr;
        /*When the starting LCN is over 4 bytes, it can be removed directly.*/
        *p = (uint32_t)taddr;
        *(p + 1) = (uint32_t)rsize;
        p += 2;
        if (p - part->bitmap_position >= sizeof(part->bitmap_position) - 8)
            hd_throw(ctx, HD_ERROR_GENERIC, "$BITMAP too many fragments");

        pruns += iseek;
    }
}

static int
ntfs_get_cluster_val(hd_context *ctx, hd_disk *disk, ntfs_part *part, ntfs_scan *scan, uint32_t number)
{
    /*The current cluster is in a number of sectors*/
    uint32_t pos_sector = number / 4096;
    /*The current cluster is in a number of byte*/
    uint32_t pos_byte = (number % 4096) / 8;
    /*The current cluster is in a number of bit*/
    uint32_t pos_bit = (number % 4096) % 8;

    if (pos_sector != scan->pos_buf_sector_num)
    {
        uint32_t bitmap_sec_num,relative;
        uint32_t i;
        bitmap_sec_num = part->bitmap_position[1];
        i = 1;
        relative = pos_sector;

        while (relative + 1 > bitmap_sec_num)
        {
            relative -= part->bitmap_position[i];
            i += 2;
            bitmap_sec_num = part->bitmap_position[i];

            if (!part->bitmap_position[i])
                hd_throw(ctx, HD_ERROR_GENERIC, "cannot find bitmap LCN");
        }

        uint64_t start = relative +  part->bitmap_position[i - 1] * part->super.secperclr * disk->sector_size;
        hd_read_write_device(ctx, part->super.dev_fd, false, scan->sector_buf,
                             start,
                             disk->sector_size);

        scan->pos_buf_sector_num = pos_sector;

    }

    uint8_t b, val;
    b = scan->sector_buf[pos_byte];
    val = b >> pos_bit;
    return val&1;


}

void
ntfs_scan_init(hd_context *ctx, hd_disk *disk, ntfs_part *part)
{
    ntfs_scan *scan;
    hd_var(scan);

    hd_try(ctx)
    {
        scan = hd_malloc(ctx, sizeof(ntfs_scan));
        ntfs_parse_bitmap_record(ctx, disk, part, scan);

        /*Mark the part's bitmap by the bitmap of the NTFS*/
        scan->sector_buf = hd_malloc(ctx, disk->sector_size);
        scan->pos_buf_sector_num = 0xFFFFFFFF;
        uint32_t cluster_num;
        /*Clusters that are currently to be marked*/
        uint32_t i;
        cluster_num = (uint32_t) part->super.total_sector / part->super.secperclr;
        for (i = 0; i < cluster_num; i++)
        {
            if (!ntfs_get_cluster_val(ctx, disk, part, scan, i) ||
                    i * part->super.secperclr >= part->super.total_sector)
                continue;

            if (i * part->super.secperclr + COPY_BLOCK_SIZE > part->super.total_sector)
            {
                hd_set_data_bitmap(ctx, &part->super, i * part->super.secperclr,
                                   part->super.total_sector - i * part->super.secperclr);
                i += part->super.total_sector - i * part->super.secperclr;
                continue;
            }

            hd_set_data_bitmap(ctx, &part->super, i * part->super.secperclr, COPY_BLOCK_SIZE);
            i += 8;
        }

    }
    hd_always(ctx)
    {
        hd_free(ctx, scan->sector_buf);
        scan->sector_buf = NULL;
        hd_free(ctx, scan->mft_record);
        scan->mft_record = NULL;
        hd_free(ctx, scan);
        scan = NULL;
    }
    hd_catch(ctx)
    {
        hd_rethrow(ctx);
    }

}