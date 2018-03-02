//
// Created by sjw on 17/02/2018.
//

#ifndef DISKCLONE_HDTD_PARTITION_H
#define DISKCLONE_HDTD_PARTITION_H

#include "hdtd/system.h"
#include "hdtd/context.h"

/*
	part interface
*/
typedef struct hd_part_s hd_part;
typedef struct hd_part_handler_s hd_part_handler;

typedef void (hd_part_drop_fn)(hd_context *ctx, hd_part *part);
typedef int (hd_part_probe_fn)(hd_context *ctx, hd_part *disk);

struct hd_part_s
{
	int refs;

    uint8_t type;
    uint32_t scan_buffer_size;
    char *scan_buffer;
    uint64_t total_sector;
    uint32_t secperclr;

	hd_part_drop_fn *drop_part;
    hd_part_probe_fn *probe_part;
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

void hd_drop_part(hd_context *ctx, hd_part *part);

#endif //DISKCLONE_HDTD_PARTITION_H
