//
// Created by sjw on 2018/3/1.
//

#include "hdtd.h"
#include "gpt.h"

struct gpt_record {
    uint8_t             boot_indicator; /* unused by EFI, set to 0x80 for bootable */
    uint8_t             start_head; /* unused by EFI, pt start in CHS */
    uint8_t             start_sector; /* unused by EFI, pt start in CHS */
    uint8_t             start_track;
    uint8_t             os_type; /* EFI and legacy non-EFI OS types */
    uint8_t             end_head; /* unused by EFI, pt end in CHS */
    uint8_t             end_sector; /* unused by EFI, pt end in CHS */
    uint8_t             end_track; /* unused by EFI, pt end in CHS */
    uint32_t            starting_lba; /* used by EFI - start addr of the on disk pt */
    uint32_t            size_in_lba; /* used by EFI - size of pt in LBA */
} __attribute__ ((packed));

/* Protected MBR and legacy MBR share same structure */
struct gpt_legacy_mbr {
    uint8_t             boot_code[440];
    uint32_t            unique_mbr_signature;
    uint16_t            unknown;
    struct gpt_record   partition_record[4];
    uint16_t            signature;
} __attribute__ ((packed));

static ssize_t read_lba(gpt_disk *disk, uint64_t lba,
                        void *buffer, const size_t bytes)
{
    off_t offset = lba * disk->super.sector_size;

    if (lseek(disk->dev_fd, offset, SEEK_SET) == (off_t) -1)
        return -1;
    return read(disk->dev_fd, buffer, bytes) != (ssize_t)bytes;
}

/* Check if there is a valid header signature */
static int gpt_check_signature(gpt_header *header)
{
    return header->signature == cpu_to_le64(GPT_HEADER_SIGNATURE);
}

static inline uint32_t count_crc32(const unsigned char *buf, size_t len)
{
    return (crc32(~0L, buf, len) ^ ~0L);
}

/*
 * Recompute header and partition array 32bit CRC checksums.
 * This function does not fail - if there's corruption, then it
 * will be reported when checksuming it again (ie: probing or verify).
 */
static void gpt_recompute_crc(gpt_header *header, gpt_entry *ents)
{
    uint32_t crc = 0;
    size_t entry_sz = 0;

    if (!header)
        return;

    /* header CRC */
    header->crc32 = 0;
    crc = count_crc32((unsigned char *) header, le32_to_cpu(header->size));
    header->crc32 = cpu_to_le32(crc);

    /* partition entry array CRC */
    header->partition_entry_array_crc32 = 0;
    entry_sz = le32_to_cpu(header->npartition_entries) *
               le32_to_cpu(header->sizeof_partition_entry);

    crc = count_crc32((unsigned char *) ents, entry_sz);
    header->partition_entry_array_crc32 = cpu_to_le32(crc);
}

/*
 * Compute the 32bit CRC checksum of the partition table header.
 * Returns 1 if it is valid, otherwise 0.
 */
static int gpt_check_header_crc(gpt_header *header, gpt_entry *ents)
{
    uint32_t crc, orgcrc = le32_to_cpu(header->crc32);

    header->crc32 = 0;
    crc = count_crc32((unsigned char *) header, le32_to_cpu(header->size));
    header->crc32 = cpu_to_le32(orgcrc);

    if (crc == le32_to_cpu(header->crc32))
        return 1;

    /*
     * If we have checksum mismatch it may be due to stale data,
     * like a partition being added or deleted. Recompute the CRC again
     * and make sure this is not the case.
     */
    if (ents) {
        gpt_recompute_crc(header, ents);
        orgcrc = le32_to_cpu(header->crc32);
        header->crc32 = 0;
        crc = count_crc32((unsigned char *) header, le32_to_cpu(header->size));
        header->crc32 = cpu_to_le32(orgcrc);

        return crc == le32_to_cpu(header->crc32);
    }

    return 0;
}

/* Returns the GPT entry array */
static gpt_entry *gpt_read_entries(gpt_disk *disk,
                                          gpt_header *header)
{
    ssize_t sz;
    gpt_entry *ret = NULL;
    off_t offset;

    assert(disk);
    assert(header);

    sz = le32_to_cpu(header->npartition_entries) *
         le32_to_cpu(header->sizeof_partition_entry);

    ret = calloc(1, sz);
    if (!ret)
        return NULL;
    offset = le64_to_cpu(header->partition_entry_lba) *
            disk->super.sector_size;

    if (offset != lseek(disk->dev_fd, offset, SEEK_SET))
        goto fail;
    if (sz != read(disk->dev_fd, ret, sz))
        goto fail;

    return ret;

fail:
    free(ret);
    return NULL;
}

/*
 * It initializes the partition entry array.
 * Returns 1 if the checksum is valid, otherwise 0.
 */
static int gpt_check_entryarr_crc(gpt_header *header,
                                  gpt_entry *ents)
{
    int ret = 0;
    ssize_t entry_sz;
    uint32_t crc;

    if (!header || !ents)
        goto done;

    entry_sz = le32_to_cpu(header->npartition_entries) *
               le32_to_cpu(header->sizeof_partition_entry);

    if (!entry_sz)
        goto done;

    crc = count_crc32((unsigned char *) ents, entry_sz);
    ret = (crc == le32_to_cpu(header->partition_entry_array_crc32));
    done:
    return ret;
}

static int gpt_check_lba_sanity(gpt_disk *cxt, gpt_header *header)
{
    int ret = 0;
    uint64_t lu, fu;

    fu = le64_to_cpu(header->first_usable_lba);
    lu = le64_to_cpu(header->last_usable_lba);

    /* check if first and last usable LBA make sense */
    if (lu < fu) {
        goto done;
    }

    /* TODO:check if first and last usable LBAs with the disk's last LBA */

    /* the header has to be outside usable range */
    if (fu < GPT_PRIMARY_PARTITION_TABLE_LBA &&
        GPT_PRIMARY_PARTITION_TABLE_LBA < lu) {
        goto done;
    }

    ret = 1; /* sane */
    done:
    return ret;
}

/*
 * Return the specified GPT Header, or NULL upon failure/invalid.
 * Note that all tests must pass to ensure a valid header,
 * we do not rely on only testing the signature for a valid probe.
 */
static gpt_header *gpt_read_header(gpt_disk *disk,
                                          uint64_t lba,
                                          gpt_entry **_ents)
{
    gpt_header *header = NULL;
    gpt_entry *ents = NULL;
    uint32_t hsz;

    if (!disk)
        return NULL;

    /* always allocate all sector, the area after GPT header
     * has to be fill by zeros */
    assert(disk->super.sector_size >= sizeof(gpt_header));

    header = calloc(1, disk->super.sector_size);
    if (!header)
        return NULL;

    /* read and verify header */
    if (read_lba(disk, lba, header, disk->super.sector_size) != 0)
        goto invalid;

    if (!gpt_check_signature(header))
        goto invalid;

    /* make sure header size is between 92 and sector size bytes */
    hsz = le32_to_cpu(header->size);
    if (hsz < GPT_HEADER_MINSZ || hsz > disk->super.sector_size)
        goto invalid;

    if (!gpt_check_header_crc(header, NULL))
        goto invalid;

    /* read and verify entries */
    ents = gpt_read_entries(disk, header);
    if (!ents)
        goto invalid;

    if (!gpt_check_entryarr_crc(header, ents))
        goto invalid;

    if (!gpt_check_lba_sanity(disk, header))
        goto invalid;

    /* valid header must be at MyLBA */
    if (le64_to_cpu(header->my_lba) != lba)
        goto invalid;


    if (_ents)
        *_ents = ents;
    else
        free(ents);

    return header;

invalid:
    free(header);
    free(ents);

    return NULL;
}

/*
 * Checks if there is a valid protective MBR partition table.
 * Returns 0 if it is invalid or failure. Otherwise, return
 * GPT_MBR_PROTECTIVE or GPT_MBR_HYBRID, depeding on the detection.
 */
static int valid_pmbr(gpt_disk *disk)
{
    int i, part = 0, ret = 0; /* invalid by default */
    struct gpt_legacy_mbr *pmbr = NULL;
    uint32_t sz_lba = 0;

    if (!disk->first_sector)
        goto done;

    pmbr = (struct gpt_legacy_mbr *) disk->first_sector;

    if (le16_to_cpu(pmbr->signature) != MSDOS_MBR_SIGNATURE)
        goto done;

    /* seems like a valid MBR was found, check DOS primary partitions */
    for (i = 0; i < 4; i++) {
        if (pmbr->partition_record[i].os_type == EFI_PMBR_OSTYPE) {
            /*
             * Ok, we at least know that there's a protective MBR,
             * now check if there are other partition types for
             * hybrid MBR.
             */
            part = i;
            ret = GPT_MBR_PROTECTIVE;
            break;
        }
    }

    if (ret != GPT_MBR_PROTECTIVE)
        goto done;

    /* LBA of the GPT partition header */
    if (pmbr->partition_record[part].starting_lba !=
        cpu_to_le32(GPT_PRIMARY_PARTITION_TABLE_LBA))
        goto done;

    for (i = 0 ; i < 4; i++) {
        if ((pmbr->partition_record[i].os_type != EFI_PMBR_OSTYPE) &&
            (pmbr->partition_record[i].os_type != 0x00))
            ret = GPT_MBR_HYBRID;
    }

    done:
    return ret;
}

int gpt_probe_label(hd_context *ctx, gpt_disk *disk)
{
    int mbr_type;
    mbr_type = valid_pmbr(disk);
    if (!mbr_type)
        return 0;

    /* primary header */
    disk->pheader = gpt_read_header(disk, GPT_PRIMARY_PARTITION_TABLE_LBA,
                                   &disk->ents);

    /*TODO: bheader also needs to be read */
    if (!disk->pheader)
        return 0;


    return 1;
}