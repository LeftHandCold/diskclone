//
// Created by sjw on 16/02/2018.
//

#ifndef DISKCLONE_HDTD_DISK_H
#define DISKCLONE_HDTD_DISK_H

#include "hdtd/system.h"
#include "hdtd/context.h"

#define DEFAULT_SECTOR_SIZE       512

/*
	Disk interface
*/
typedef struct hd_disk_dest_s hd_disk_dest;
typedef struct hd_disk_s hd_disk;
typedef struct hd_disk_handler_s hd_disk_handler;

struct hd_disk_dest_s
{
	int refs;

	const char *name;
	int dev_fd;
	uint64_t size;
};

typedef void (hd_disk_drop_fn)(hd_context *ctx, hd_disk *disk);

typedef int (hd_disk_probe_fn)(hd_context *ctx, hd_disk *disk);

struct hd_disk_s
{
	int refs;

	hd_disk_dest *disk_dest;
	hd_disk_drop_fn *drop_disk;
    hd_disk_probe_fn *probe_disk;
};

typedef hd_disk *(hd_disk_open_fn)(hd_context *ctx, const char *diskname);

typedef int (hd_disk_recognize_fn)(hd_context *ctx, const char *diskname);

struct hd_disk_handler_s
{
	hd_disk_recognize_fn *recognize;
	hd_disk_open_fn *open;
};

/*
	hd_new_disk: Create and initialize a disk structs.
*/
void *hd_new_disk_of_size(hd_context *ctx, int size);

#define hd_new_derived_disk(C,M) ((M*)Memento_label(hd_new_disk_of_size(C, sizeof(M)), #M))

void hd_register_disk_handler(hd_context *ctx, const hd_disk_handler *handler);

/**
 * hd_register_disk_handler: Register handlers
 * for all the standard disk types supported in
 * this build.
 * @param ctx
 */
void hd_register_disk_handlers(hd_context *ctx);

/**
 * hd_open_disk: Open or created a disk structs.
 * @param ctx
 * @param diskname
 * @return hd_disk
 */
hd_disk *hd_open_disk(hd_context *ctx, const char *src, const char *dest);

void hd_drop_disk(hd_context *ctx, hd_disk *disk);

int hd_open_dest_disk(hd_context *ctx, hd_disk *disk, const char *diskname);

void hd_drop_dest_disk(hd_context *ctx, hd_disk_dest *disk);

/**
 * hd_open_dev: Open the device.
 * @param ctx
 * @param diskname : device's name.
 * @return handle
 */
int hd_open_dev(hd_context *ctx, const char *diskname);

void hd_read_write_device(hd_context *ctx, int fd, bool bwrite, unsigned char *buf, uint64_t start, size_t size);

void hd_safe_disk_close(int dev_fd);

#endif //DISKCLONE_HDTD_DISK_H
