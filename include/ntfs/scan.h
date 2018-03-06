//
// Created by sjw on 2018/3/5.
//

#ifndef DISKCLONE_NTFS_SCAN_H
#define DISKCLONE_NTFS_SCAN_H
/*
 * Scan NTFS valid data module,
 * Analyze NTFS metadata to mark
 * out the used clusters
 * */

typedef struct mft_head_s mft_head;
typedef struct resid_attr_s resid_attr;
typedef struct nonresid_attr_s nonresid_attr;
typedef struct mft_attr_s mft_attr;

typedef struct ntfs_scan_s ntfs_scan;

#define LOB( a )		( (uint8_t)(a) & 0xF )
#define HIB( a )		( ( (uint8_t)(a) & 0xF0 ) >> 4 )

#define MAX_RUNS 0x200

#define		BITMAP_RECORD       0x06

#define		$DATA				0x80
#define		$NAME				0x30
#define		$ATTR_LIST			0x20
#define		$INDEX_ALLOC		0xA0
#define		$INDEX_ROOT			0x90
#define		$BITMAP				0xb0

struct ntfs_scan_s
{
    uint16_t mft_size;
    unsigned char* mft_record;

    /*Buffering for reading BITMAP tables*/
    unsigned char* sector_buf;
    /*The sector currently in the buffer*/
    uint32_t pos_buf_sector_num;
};

#pragma pack(1)
struct mft_head_s
{
    uint8_t			    bHeadID[4];
    uint16_t			usFixupOffset;
    uint16_t			usFixupNum;
    uint8_t			    bReserve1[8];
    uint16_t			wUnknownSeqNum;
    uint16_t			usLinkNum;
    uint16_t			usAttrOffset;
    uint16_t			wResident;
    uint32_t			ulMFTSize;
    uint32_t			ulMFTAllocSize;
    uint64_t		    ullMainMFT;
    uint16_t			wNextFreeID;
    uint16_t			wFixup[0x10];
};

struct resid_attr_s
{

    uint32_t	ulDataSize;
    uint16_t	usRDataOffset;
    uint16_t	wUnknownAttrIndexID; // attribute is indexed

};

struct nonresid_attr_s
{

    uint64_t	ullVCNStart;
    uint64_t	ullVCNEnd;
    uint16_t	usNrDataOffset;
    uint16_t	wComprEngine;
    uint32_t	deReserve2;
    uint64_t	ullAllocSize;
    uint64_t	ullDataSize;
    uint64_t	ullInitSize;
    uint64_t	ullComprSize; //  is this runs if non-compressed

};

struct mft_attr_s
{

    uint32_t	dwAttrType;
    uint16_t	usAttrSize;
    uint16_t	wReserve1;
    uint8_t	    bISResident;
    uint8_t	    bLenName;
    uint16_t	usDataOffset; 	// offset to name or resident data;
    uint16_t	wISCompr;		// 1 compressed, 0 non-compressed;
    uint16_t	wAttrID;

    union unAttrib
    {
        resid_attr		ResidAttr;
        nonresid_attr	NonResidAttr;
    } unAttrib;

};
#pragma pack()

void ntfs_scan_init(hd_context *ctx, hd_disk *disk, ntfs_part *part);



#endif //DISKCLONE_NTFS_SCAN_H
