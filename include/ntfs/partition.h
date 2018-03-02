//
// Created by sjw on 2018/3/2.
//

#ifndef DISKCLONE_NTFS_PARTITION_H
#define DISKCLONE_NTFS_PARTITION_H

typedef struct ntfs_part_s ntfs_part;

#define COPY_BLOCK_SIZE 64	//64 sectors as a basic block
#define SCAN_BUFFER_SIZE 0x10000

struct ntfs_part_s
{
    hd_part super;

    int dev_fd;         /* device descriptor */
    uint32_t bitmap_size;
};


int ntfs_probe_label(hd_context *ctx, ntfs_part *part);
#endif //DISKCLONE_NTFS_PARTITION_H
