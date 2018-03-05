//
// Created by sjw on 2018/3/2.
//

#include "hdtd.h"
#include "ntfs.h"

int ntfs_probe_label(hd_context *ctx, ntfs_part *part)
{
    ntfs_bpb *pntfs = (ntfs_bpb *)part->super.scan_buffer;

    if (memcmp((char*)pntfs->OemID, "NTFS", 4) == 0)
    {
        part->super.total_sector = pntfs->TotalSectors;
        part->super.secperclr = pntfs->SecPerClr;
        part->super.type = NTFS;
        return 1;
    }
    return 0;
}