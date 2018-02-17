//
// Created by sjw on 16/02/2018.
//

#ifndef DISKCLONE_HDTD_DISK_H
#define DISKCLONE_HDTD_DISK_H

#include "hdtd/system.h"
#include "hdtd/context.h"

/*
	Disk interface
*/
typedef struct hd_disk_s hd_disk;
typedef struct hd_disk_handler_s hd_disk_handler;


typedef void (hd_disk_drop_fn)(hd_context *ctx, hd_disk *doc);

struct hd_disk_s
{
	int refs;
	hd_disk_drop_fn *drop_disk;
};

typedef hd_disk *(hd_disk_open_fn)(hd_context *ctx, const char *diskname);

typedef int (hd_disk_recognize_fn)(hd_context *ctx, const char *diskname);

struct hd_disk_handler_s
{
	hd_disk_recognize_fn *recognize;
	hd_disk_open_fn *open;
};


void hd_register_disk_handler(hd_context *ctx, const hd_disk_handler *handler);

/*
	hd_register_disk_handler: Register handlers
	for all the standard disk types supported in
	this build.
*/
void hd_register_disk_handlers(hd_context *ctx);

hd_disk *hd_open_disk(hd_context *ctx, const char *diskname);


void hd_drop_disk(hd_context *ctx, hd_disk *disk);


#endif //DISKCLONE_HDTD_DISK_H
