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

    return 1;
}