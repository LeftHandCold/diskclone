//
// Created by sjw on 17/02/2018.
//

#ifndef DISKCLONE_HDTD_PARTITION_H
#define DISKCLONE_HDTD_PARTITION_H

#include "hdtd/system.h"
#include "hdtd/context.h"

/*
	partition interface
*/
typedef struct hd_partition_s hd_partition;
typedef struct hd_partition_handler_s hd_partition_handler;


typedef void (hd_partition_drop_fn)(hd_context *ctx, hd_partition *doc);

struct hd_partition_s
{
	int refs;
	hd_partition_drop_fn *drop_partition;
};

typedef hd_partition *(hd_partition_open_fn)(hd_context *ctx, const char *partitionname);

typedef int (hd_partition_recognize_fn)(hd_context *ctx, const char *partitionname);

struct hd_partition_handler_s
{
	hd_partition_recognize_fn *recognize;
	hd_partition_open_fn *open;
};


void hd_register_partition_handler(hd_context *ctx, const hd_partition_handler *handler);

void hd_register_partition_handlers(hd_context *ctx);

hd_partition *hd_open_partition(hd_context *ctx, const char *diskname);


void hd_drop_partition(hd_context *ctx, hd_partition *partition);

#endif //DISKCLONE_HDTD_PARTITION_H
