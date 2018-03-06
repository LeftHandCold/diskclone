//
// Created by sjw on 17/02/2018.
//

#ifndef DISKCLONE_HDTD_PARTITION_H
#define DISKCLONE_HDTD_PARTITION_H

#include "hdtd/system.h"
#include "hdtd/context.h"


#define COPY_BLOCK_SIZE 64 //64 sectors as a basic block
#define SCAN_BUFFER_SIZE 0x10000

/*
	part interface
*/
typedef struct hd_part_s hd_part;
typedef struct hd_part_handler_s hd_part_handler;

typedef void (hd_part_drop_fn)(hd_context *ctx, hd_part *part);
typedef int (hd_part_probe_fn)(hd_context *ctx, hd_part *disk);
typedef void (hd_part_clone_fn)(hd_context *ctx, hd_disk *disk, hd_part *part);

struct hd_part_s
{
	int refs;

    uint8_t type;
    uint32_t scan_buffer_size;
    unsigned char *scan_buffer;
    uint64_t total_sector;
    uint32_t secperclr;

	uint32_t bitmap_size;
	unsigned char *bitmap;

	hd_part_drop_fn *drop_part;
    hd_part_probe_fn *probe_part;
	hd_part_clone_fn *clone_part;

};

typedef hd_part *(hd_part_open_fn)(hd_context *ctx, hd_disk *disk, const char *partname);

typedef int (hd_part_recognize_fn)(hd_context *ctx, const char *partname);

struct hd_part_handler_s
{
	hd_part_recognize_fn *recognize;
	hd_part_open_fn *open;
};

/*
	hd_new_part: Create and initialize a part structs.
*/
void *hd_new_part_of_size(hd_context *ctx, int size);

#define hd_new_derived_part(C,M) ((M*)Memento_label(hd_new_part_of_size(C, sizeof(M)), #M))

void hd_register_part_handler(hd_context *ctx, const hd_part_handler *handler);

void hd_register_part_handlers(hd_context *ctx);

hd_part *hd_open_part(hd_context *ctx, hd_disk *disk, const char *partname);
void hd_clone_part(hd_context *ctx, hd_disk *disk, hd_part *part);

void hd_drop_part(hd_context *ctx, hd_part *part);

/**
 * Set the partition data bitmap bit,
 * each for 64 sectors
 * @param ctx
 * @param sector
 * @param num
 */
void hd_set_data_bitmap(hd_context *ctx, hd_part *part, uint64_t sector, uint64_t num);

#endif //DISKCLONE_HDTD_PARTITION_H
