//
// Created by sjw on 2018/3/2.
//

#ifndef DISKCLONE_NTFS_PARTITION_H
#define DISKCLONE_NTFS_PARTITION_H

typedef struct ntfs_part_s ntfs_part;

#define COPY_BLOCK_SIZE 64 //64 sectors as a basic block
#define SCAN_BUFFER_SIZE 0x10000
typedef struct ntfs_bpb_s ntfs_bpb;

#pragma pack(1)
struct ntfs_bpb_s
{
    uint8_t	    Jump[3];				//0
    uint8_t	    OemID[8];				//3
    uint16_t	BytesPerSector;			//11
    uint8_t	    SecPerClr;				//13
    uint16_t	BootSectors;			//14
    uint8_t	    Mbz1;					//16
    uint16_t	Mbz2;					//17
    uint16_t	Reserved1;				//19
    uint8_t	    MediaType;				//21
    uint16_t	Mbz3;					//22
    uint16_t	SecPerTrk;				//24
    uint16_t	Sides;					//26
    uint32_t	PartitionOffset;		//28
    uint32_t	Reserved2[2];			//32
    uint64_t    TotalSectors;			//40
    uint64_t    MftStartLcn;			//48
    uint64_t    Mft2StartLcn;			//56
    uint32_t	ClustersPerFileRecord;	//64
    uint32_t	ClustersPerIndexBlock;	//68
    uint32_t	SerialNumber;			//72
    uint32_t	SerialNumber1;			//76
    uint8_t	    Code[0x1AE];			//80
    uint16_t	BootSignature;
};
#pragma pack()

struct ntfs_part_s
{
    hd_part super;

    int dev_fd;         /* device descriptor */
    uint32_t bitmap_size;
    unsigned char *bitmap;

    ntfs_bpb *pntfs;
};


int ntfs_probe_label(hd_context *ctx, ntfs_part *part);
void ntfs_part_clone(hd_context *ctx, hd_disk *disk, ntfs_part *part);
#endif //DISKCLONE_NTFS_PARTITION_H
