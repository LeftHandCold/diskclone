//
// Created by sjw on 2018/3/12.
//

#ifndef DISKCLONE_EXT_PARTITION_H
#define DISKCLONE_EXT_PARTITION_H

typedef struct ext_part_s ext_part;

typedef struct ext3_super_block_s ext3_super_block;
typedef struct ext3_group_desc_s ext3_group_desc;

#define EXT4_BG_INODE_UNINIT	1
#define EXT4_BG_BLOCK_UNINIT	2
#define EXT4_BG_INODE_ZEROED	4

#pragma pack(1)
struct ext3_super_block_s
{
    uint32_t		s_inodes_count;			//00	inodes count
    uint32_t		s_blocks_count;			//		blocks count
    uint32_t		s_r_blocks_count;		//		Reserved blocks count
    uint32_t		s_free_blocks_count;	//		Free blocks count
    uint32_t		s_free_inodes_count;	//10	Free inodes count
    uint32_t		s_first_data_block;		//		The first data block
    uint32_t		s_log_block_size;		//		block 的大小
    uint32_t		s_log_frag_size;		//		Can be ignored
    uint32_t		s_blocks_per_group;		//20	The number of blocks per block group
    uint32_t		s_frags_per_group;		//		Can be ignored
    uint32_t		s_inodes_per_group;		//		The number of inodes per block group
    uint32_t		s_mtime;				//		Mount time
    uint32_t		s_wtime;				//30	Write time
    uint16_t		s_mnt_count;			//		Mount count
    uint16_t		s_max_mnt_count;		//		Maximal mount count
    uint16_t		s_magic;				//0x38
    uint16_t		s_state;				//		File system state
    uint16_t		s_errors;				//		Behaviour when detecting errors
    uint16_t		s_minor_rev_level;		//		minor revision level
    uint32_t		s_lastcheck;			//40	time of last check
    uint32_t		s_checkinterval;		//		max. time between checks
    uint32_t		s_creator_os;			//		Can be ignored
    uint32_t		s_rev_level;			//		Revision level
    uint8_t		    unknown[0xAE];			//50
    uint16_t		s_desc_size;			//0xfe  Size of group descriptors, in bytes, if the 64bit
};

struct ext3_group_desc_s
{
    uint32_t		bg_block_bitmap;		//block pointer to block bitmap
    uint32_t		bg_inode_bitmap;		//block pointer to inode bitmap
    uint32_t		bg_inode_table;			//block pointer to inodes table
    uint16_t		bg_free_blocks_count;
    uint16_t		bg_free_inodes_count;
    uint16_t		bg_used_dirs_count;
    uint16_t		bg_flags;				//flags, whether the group desc is initialized
    uint32_t		bg_reserved[3];			//Can be ignored
};
#pragma pack()

struct ext_part_s
{
    hd_part super;

    uint64_t        free_sectors;

    uint32_t		groupindex;
    uint32_t		group_desc_sector;	//The sector of the group descriptor, relative to the start of the partition
    uint32_t		blocks_per_group;
    uint32_t		groups_count;

    /*
	s_first_data_block specifies in which of the device block the superblock is present.
	The superblock is always present at the fixed offset 1024, but the device block numbering can differ.
	For example, if the block size is 1024, the superblock will be at block 1 with respect to the device.
	However, if the block size is 4096, offset 1024 is included in block 0 of the device,
	and in that case s_first_data_block will contain 0. At least this is how I understood this variable.
	*/
    uint32_t		first_data_block;        //The location of the first data block
};

int ext_probe_label(hd_context *ctx, hd_disk *disk, ext_part *part);
void ext_part_clone(hd_context *ctx, hd_disk *disk, ext_part *part);
#endif //DISKCLONE_EXT_PARTITION_H
