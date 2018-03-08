//
// Created by sjw on 2018/3/7.
//

#ifndef DISKCLONE_HDTD_PTHREAD_H
#define DISKCLONE_HDTD_PTHREAD_H

#include "hdtd/system.h"
#include "hdtd/context.h"

#include "hdtd/partition.h"

#include <pthread.h>

typedef struct hd_pthread_s hd_pthread;
typedef struct hd_copy_cache_s hd_copy_cache;

#define CACHE_FLAG_FREE			0
#define CACHE_FLAG_AVAIL		1	//There are data cache
#define COPY_CACHE_SIZE			32 * 1024 * 1024
#define COPY_IO_UNIT_SIZE		128	//How many sectors are copied (read and written) each time
#define MAX_PRODUCER_CONSUMER	16

struct hd_copy_cache_s
{
    struct list_head list;
    hd_pthread *producer;
    hd_pthread *consumer;
    uint64_t firstblock;
    uint32_t block_num;	//Valid data's block
    uint32_t cache_size;
    uint32_t cache_flag;
    unsigned char *cache;
    unsigned char *iocache;
};

struct hd_pthread_s
{
    hd_context *ctx;
    hd_part *part;
    hd_disk *disk;

    pthread_t		threadid;
    int				index;
    unsigned int	active : 1;
    uint64_t		beginblock;
    uint64_t		endblock;
};

struct hd_pthread_context_s
{
    int refs;

    unsigned int	eof:1,
            producer_eof:1,
            consumer_eof:1,
            producer_err:1,
            consumer_err:1,
            copy_different:1;

    pthread_attr_t thread_attr;
    pthread_cond_t consumer_cond;
    pthread_cond_t producer_cond;
    pthread_mutex_t mutex;
    int consumer_thread_num;
    int producer_thread_num;

    struct hd_pthread_s producer;
    struct hd_pthread_s consumer;

    int cache_num;
    hd_copy_cache *copy_cache;
    struct list_head avail_cache;
    struct list_head free_cache;
};

void hd_pthread_init(hd_context *ctx, hd_disk *disk, hd_part *part);
void hd_pthread_deinit(hd_context *ctx);

void hd_producer_create(hd_context *ctx, hd_disk *disk, hd_part *part);
void hd_consumer_create(hd_context *ctx, hd_disk *disk, hd_part *part);
void hd_start_producer(hd_context *ctx, hd_disk *disk, hd_part *part);
void hd_while_pthread(hd_context *ctx);
#endif //DISKCLONE_HDTD_PTHREAD_H
