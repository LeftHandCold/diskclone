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


typedef struct _PARTITION_ENTRY
{
    uint8_t	    id;					//0
    uint8_t	    beginhead;			//0
    uint16_t	begin_cylnAndsector;	//0
    uint8_t	    pid;				//0
    uint8_t	    endHead;			//0
    uint16_t	endCylnAndSector;	//0
    uint32_t	frondsectortotal;	//0
    uint32_t	totalsector;		//0
} PARTITION_ENTRY, *PPARTITION_ENTRY;

typedef struct _NTFS_BOOT_SECTOR
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
} NTFS_BOOT_SECTOR, *PNTFS_BOOT_SECTOR, NTFS_BPB, *PNTFS_BPB;

typedef struct _FAT32_BOOT_SECTOR	//FAT32格式的BPB参数块
{
    uint8_t	    Jumpto[3];		//0,  DB 0EBH, 03CH, 090H
    uint8_t	    OemID[8];		//3,  uint8_t "MSDOS5.0"
    uint16_t	SecInByte;		//11,  dw 0200H
    uint8_t	    SecPerClr;		//13  db 00H
    uint16_t	ResSec;			//14  dw 0001H
    uint8_t	    NumOfFat;		//16  db 02H
    uint32_t	Reserved1;		//17	dd 00H
    uint8_t	    FormatID;		//21  db 0F8H
    uint16_t	SecPerFat;		//22  dw 0000H
    uint16_t	SecPerTrk;		//24  dw 003fH
    uint16_t	Sides;			//26  dw 0000H
    uint32_t	Hsector;		//28  dd 003fH
    uint32_t	BigTotalSec;	//32  dd 00000000H     ; Big total number of sectors
    uint32_t	BigSecPerFat;	//36  dd 00000000H
    uint16_t	flags;			//40
    uint16_t	fs_version;		//42		/* file system version number */
    uint32_t	RootCluster;	//44		/* cluster number of the first cluster of root */
    uint16_t	info_sec;		//48		/* file system information sector */
    uint16_t	bootbackup_sec;	//50		/* backup boot sector sector */
    uint8_t	    Free[12];		//52
    uint8_t	    Phydrvnum;		//64  db 80H       ; physical drive number
    uint8_t	    Dirty;			//65
    uint8_t	    Signature;		//66 db  029H        ; Extended Boot Record Signature
    uint32_t	SerialNumber;	//67  dd  015470ff4h  ; Volume Serial Number
    uint8_t	    DISKLabel[11];	//71     uint8_t  "DISK1_VOL1 "
    uint8_t	    FileSystem[8];	//82  uint8_t  "FAT16   "
} FAT32_BOOT_SECTOR, *PFAT32_BOOT_SECTOR, FAT32_BPB, *PFAT32_BPB;

#endif //DISKCLONE_HDTD_BIOSDISK_H
