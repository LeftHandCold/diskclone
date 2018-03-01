//
// Created by sjw on 2018/3/1.
//

#ifndef DISKCLONE_GPT_GPT_H
#define DISKCLONE_GPT_GPT_H

typedef struct gpt_guid_s gpt_guid;
typedef struct gpt_entry_s gpt_entry;
typedef struct gpt_header_s gpt_header;
typedef struct gpt_disk_s gpt_disk;

/**
 * SECTION: gpt
 * @title: UEFI GPT
 * @short_description: specific functionality
 */

#define GPT_HEADER_SIGNATURE 0x5452415020494645LL /* EFI PART */
#define GPT_HEADER_REVISION_V1_02 0x00010200
#define GPT_HEADER_REVISION_V1_00 0x00010000
#define GPT_HEADER_REVISION_V0_99 0x00009900
#define GPT_HEADER_MINSZ          92 /* bytes */

#define GPT_PMBR_LBA        0
#define GPT_MBR_PROTECTIVE  1
#define GPT_MBR_HYBRID      2

#define GPT_PRIMARY_PARTITION_TABLE_LBA 0x00000001

#define EFI_PMBR_OSTYPE     0xEE
#define MSDOS_MBR_SIGNATURE 0xAA55
#define GPT_PART_NAME_LEN   (72 / sizeof(uint16_t))
#define GPT_NPARTITIONS     128

/* Globally unique identifier */
struct gpt_guid_s {
    uint32_t   time_low;
    uint16_t   time_mid;
    uint16_t   time_hi_and_version;
    uint8_t    clock_seq_hi;
    uint8_t    clock_seq_low;
    uint8_t    node[6];
};

/* The GPT Partition entry array contains an array of GPT entries. */
struct gpt_entry_s {
    gpt_guid     type; /* purpose and type of the partition */
    gpt_guid     partition_guid;
    uint64_t            lba_start;
    uint64_t            lba_end;
    uint64_t            attrs;
    uint16_t            name[GPT_PART_NAME_LEN];
}  __attribute__ ((packed));

/* GPT header */
struct gpt_header_s {
    uint64_t            signature; /* header identification */
    uint32_t            revision; /* header version */
    uint32_t            size; /* in bytes */
    uint32_t            crc32; /* header CRC checksum */
    uint32_t            reserved1; /* must be 0 */
    uint64_t            my_lba; /* LBA of block that contains this struct (LBA 1) */
    uint64_t            alternative_lba; /* backup GPT header */
    uint64_t            first_usable_lba; /* first usable logical block for partitions */
    uint64_t            last_usable_lba; /* last usable logical block for partitions */
    gpt_guid            disk_guid; /* unique disk identifier */
    uint64_t            partition_entry_lba; /* LBA of start of partition entries array */
    uint32_t            npartition_entries; /* total partition entries - normally 128 */
    uint32_t            sizeof_partition_entry; /* bytes for each GUID pt */
    uint32_t            partition_entry_array_crc32; /* partition CRC checksum */
    uint8_t             reserved2[512 - 92]; /* must all be 0 */
} __attribute__ ((packed));

int gpt_probe_label(hd_context *ctx, gpt_disk *disk);

#endif //DISKCLONE_GPT_GPT_H
