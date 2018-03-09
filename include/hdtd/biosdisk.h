//
// Created by sjw on 2018/3/2.
//

#ifndef DISKCLONE_HDTD_BIOSDISK_H
#define DISKCLONE_HDTD_BIOSDISK_H

#define CACHE_DOWN						0		//PRE-READ
#define CACHE_UP						1

#define MAX_SECTORS_PER_IO				2048
#define SECTOR_SIZE						512
#define MAX_EXCEPTION_FUNCTIONS			4

//define partition type
#ifndef PARTITION_ENTRY_UNUSED
#define PARTITION_ENTRY_UNUSED			0x00	// Entry unused
#define PARTITION_FAT_12				0x01	// 12-bit FAT entries
#define PARTITION_XENIX_1				0x02	// Xenix
#define PARTITION_XENIX_2				0x03	// Xenix
#define PARTITION_FAT_16				0x04	// 16-bit FAT entries
#define PARTITION_EXTENDED				0x05	// Extended partition entry
#define PARTITION_HUGE					0x06	// Huge partition MS-DOS V4
#define PARTITION_IFS					0x07	// IFS Partition
#define PARTITION_FAT32					0x0B	// FAT32
#define PARTITION_FAT32_XINT13			0x0C	// FAT32 using extended int13 services
#define PARTITION_XINT13				0x0E	// Win95 partition using extended int13 services
#define PARTITION_XINT13_EXTENDED		0x0F	// Same as type 5 but uses extended int13 services
#define PARTITION_PREP					0x41	// PowerPC Reference Platform (PReP) Boot Partition
#define PARTITION_LDM					0x42	// Logical Disk Manager partition
#define PARTITION_UNIX					0x63	// Unix
#define VALID_NTFT						0xC0	// NTFT uses high order bits
#define PARTITION_UNUSED  0x00

// The high bit of the partition type code indicates that a partition is part of an NTFT mirror or striped array.
#define PARTITION_NTFT					0x80	// NTFT partition
#endif

#define	FAT12							1
#define FAT16							0x06
#define FAT16_E							0x0E
#define BIG_FAT16_PARTITION				FAT16_E
#define NTFS							0x07
#define FAT32							0x0B
#define FAT32_E							0x0C
#define BIG_FAT32_PARTITION				FAT32_E
#define EXTEND_PARTITION				0x05
#define BIG_EXTEND_PARTITION			0x0F
#define PARTITION_DOS_12				0x01	// 12-BIT FAT ENTRIES
#define PARTITION_XENIX_ROOT			0x02	// XENIX
#define PARTITION_XENIX_USR				0x03	// XENIX
#define PLAN9_PARTITION					0x039	// PLAN 9 PARTITION ID
#define DM6_AUX1PARTITION				0x051	// NO DDO:  USE XLATED GEOM
#define DM6_AUX3PARTITION				0x053	// NO DDO:  USE XLATED GEOM
#define DM6_PARTITION					0x054	// HAS DDO: USE XLATED GEOM & OFFSET
#define EZD_PARTITION					0x055	// EZ-DRIVE
#define PARTITION_NOVELL_NETWARE		0x064
#define MINIX_PARTITION					0x081	// MINIX PARTITION ID
#define PARTITION_LINUX_SWAP			0x082
#define PARTITION_LINUX					0x083
#define LINUX_EXTENDED_PARTITION		0x085
#define PARTITION_NTFS_VOL_SET			0x086
#define PARTITION_BSD_386				0x0A5
#define PARTITION_OPENBSD				0x0A6
#define NETBSD_PARTITION				0x0A9	// NETBSD PARTITION ID
#define BSDI_PARTITION					0x0B7	// BSDI PARTITION ID
#define	GPT_PROTECTIVE_PARTITION		0xEE
#define	GPT_SYSTEM_PARTITION			0xEF
#define LINUX_RAID_PARTITION			0x0FD	// AUTODETECT RAID PARTITION
#define UNKNOWN_PARTITION				0xff

typedef struct com_parptition_s com_parptition;
typedef struct com_mbr_s com_mbr;

#pragma pack(1)
struct com_parptition_s
{
    uint8_t	    id;
    uint8_t	    beginhead;
    uint16_t	beginCylnAndsector;
    uint8_t	    pid;
    uint8_t	    endHead;
    uint16_t	endCylnAndSector;
    uint32_t	relativeSectors;
    uint32_t	totalsectors;
};

struct com_mbr_s
{
    uint8_t			        byRedundance[446];
    com_parptition		    partition[4];
    uint16_t			    byEnding;
};
#pragma pack()
#endif //DISKCLONE_HDTD_BIOSDISK_H
