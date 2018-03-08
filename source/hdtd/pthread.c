//
// Created by sjw on 2018/3/7.
//

#include "hdtd.h"
#include "hdtd-imp.h"

/*linux的事件处理机制和windows的不同:
 * windows下唤醒一个事件时不管当前是否有线程等待会设置标志，
 * 如果有线程等待，则清除标志并唤醒该等待线程;
 * 而linux在没有线程等待的情况下不做处理，直接返回，
 * 没有标志的概念.
 */
/*Linux event handling mechanism and Windows different:
 *Windows wakes up an event regardless of whether there
 * is currently a thread waiting to set the flag,If there
 * is thread waiting, clear the flag and wake up the waiting
 * thread;Linux does not deal with the case of no thread to wait,
 * return directly,No sign concept.
 */

/*Waiting for producer production data*/
static int
wait_producer(hd_pthread_context *ctx)
{
    /*Consumers wait for producers' data,
     * using simple consumer producer-one-to-one correspondence*/
    pthread_cond_wait(&ctx->producer_cond, &ctx->mutex);
    return 0;
}

/*Wait for the consumer to consume data
 * (meaning to release a new free cache)*/
static int
wait_consumer(hd_pthread_context *ctx)
{
    /*Consumers wait for producers' data,
     * using simple consumer producer-one-to-one correspondence*/
    pthread_cond_wait(&ctx->consumer_cond, &ctx->mutex);
    return 0;
}

/*Wake up to wait for the production of data consumers*/
void
wakeup_consumer(hd_pthread_context *ctx)
{
    pthread_cond_signal(&ctx->producer_cond);
}

/*Wake up Producers waiting for available cache*/
void
wakeup_producer(hd_pthread_context *ctx)
{
    pthread_cond_signal(&ctx->consumer_cond);
}

static void
assign_producer_job(hd_pthread *producer, hd_part *part)
{
    /*FIXME:No consideration is given to LVM*/
    producer->beginblock = 0;
    producer->endblock = (part->total_sector + COPY_BLOCK_SIZE - 1) / COPY_BLOCK_SIZE;
}

void
set_cache(hd_context *ctx, hd_copy_cache *data, uint32_t cache_flag)
{
    pthread_mutex_lock(&ctx->pthread->mutex);

    data->cache_flag = cache_flag;
    if(cache_flag & CACHE_FLAG_AVAIL) {
        /*Producer's production data is complete,
         * add it to the available cache linked list*/
        list_add_tail(&data->list, &ctx->pthread->avail_cache);
        wakeup_consumer(ctx->pthread);
    } else {
        list_add_tail(&data->list, &ctx->pthread->free_cache);
        wakeup_producer(ctx->pthread);
    }
    pthread_mutex_unlock(&ctx->pthread->mutex);
}

static
hd_copy_cache *get_cache(hd_context *ctx, hd_pthread *pthread, uint32_t cache_flag)
{
    hd_copy_cache *data = NULL;
    struct list_head *p = NULL;

    pthread_mutex_lock(&ctx->pthread->mutex);

    if (cache_flag == CACHE_FLAG_FREE)
    {
        hd_pthread *producer = pthread;

        while(!ctx->pthread->consumer_eof && list_empty(&ctx->pthread->free_cache))
            wait_consumer(ctx->pthread);

        if(!list_empty(&ctx->pthread->free_cache))
        {
            p = list_first(&ctx->pthread->free_cache);
            if(p) {
                data = list_entry(p, hd_copy_cache, list);
                list_del(p);
                data->producer = producer;
                data->block_num = 0;
            }
        }
    } else if (cache_flag & CACHE_FLAG_AVAIL)
    {
        hd_pthread *consumer = pthread;
__retry:
        while(!ctx->pthread->producer_eof && list_empty(&ctx->pthread->avail_cache))
        {
            wait_producer(ctx->pthread);
        }

        if (!list_empty(&ctx->pthread->avail_cache))
        {
            p = list_first(&ctx->pthread->avail_cache);
            data = list_entry(p, hd_copy_cache, list);
            p = p->next;
            if(p == &ctx->pthread->avail_cache) {
                data = NULL;
                goto __retry;
            } else {
                data = list_entry(p, hd_copy_cache, list);
            }

            list_del(p);
            data->consumer = consumer;
        }
    }

    pthread_mutex_unlock(&ctx->pthread->mutex);

    return data;
}

static void *
producer_thread(void * arg)
{
    hd_pthread *producer = (hd_pthread *)arg;
    hd_context *ctx = producer->ctx;
    hd_part *part = producer->part;
    hd_disk *disk = producer->disk;

    uint64_t current_block;
    hd_copy_cache *buf = NULL;

    producer->active = true;
    assign_producer_job(producer, part);

    current_block = producer->beginblock;

    while (!ctx->pthread->eof && current_block <= producer->endblock)
    {
        buf = get_cache(ctx, producer, CACHE_FLAG_FREE);

        while (current_block <= producer->endblock)
        {
            if (hd_read_bit(part->bitmap, current_block))
            {
                size_t count;
                count = COPY_BLOCK_SIZE;
                if (current_block * COPY_BLOCK_SIZE + COPY_BLOCK_SIZE > part->total_sector)
                    count = part->total_sector - (current_block * COPY_BLOCK_SIZE);

                hd_try(ctx)
                    hd_read_write_device(
                        ctx,
                        part->dev_fd,
                        false,
                        buf->cache + buf->block_num * COPY_BLOCK_SIZE * disk->sector_size,
                        current_block * COPY_BLOCK_SIZE * disk->sector_size,
                        count * disk->sector_size);
                hd_catch(ctx)
                {
                    ctx->pthread->producer_eof = true;
                    producer->active = false;
                    ctx->pthread->producer_err = false;
                    wakeup_consumer(ctx->pthread);
                    return NULL;
                }

                if ( buf->block_num == 0)
                    buf->firstblock = current_block;
                buf->block_num++;
                current_block++;

                if ((buf->block_num * COPY_BLOCK_SIZE * disk->sector_size) >= buf->cache_size)
                {
                    set_cache(ctx, buf, CACHE_FLAG_AVAIL);
                    break;
                }
            } else
                current_block++;
        }
    }

    if(buf && buf->block_num)
    {
        set_cache(ctx, buf, CACHE_FLAG_AVAIL);
    }
    producer->active = false;

    /*FIXME:ctx->pthread->producer should be an array*/
    if (!ctx->pthread->producer.active)
    {
        ctx->pthread->producer_eof = true;
        if (ctx->pthread->consumer_eof == true)
            ctx->pthread->eof = true;

        wakeup_consumer(ctx->pthread);
    }

    return NULL;
}

static void *
consumer_thread(void * arg)
{
    hd_pthread *consumer = (hd_pthread *)arg;
    hd_context *ctx = consumer->ctx;
    hd_part *part = consumer->part;
    hd_disk *disk = consumer->disk;

    uint64_t current_block, begin;
    uint32_t i, num;
    hd_copy_cache *data = NULL;

    consumer->active = true;

    while (!ctx->pthread->eof)
    {
        data = get_cache(ctx, consumer, CACHE_FLAG_AVAIL);
        if (!data)
        {
            hd_warn(ctx, "consumer(%d):get_cache null", consumer->index);
            break;
        }
        i = num = begin = 0;
        current_block = data->firstblock;

        while (i < data->block_num)
        {
            if (hd_read_bit(part->bitmap, current_block))
            {
                if (num == 0)
                    begin = current_block;
                num++;
                i++;
            } else
            {
                if (num)
                {
                    if (hd_write_block(ctx,
                                       disk,
                                       part,
                                       data->cache + (i - num) * COPY_BLOCK_SIZE * disk->sector_size ,
                                       begin,
                                       num) != 0)
                    {
                        hd_warn(ctx, "consumer_thread hd_write_block is failed");
                        ctx->pthread->consumer_eof = true;
                        consumer->active = false;
                        ctx->pthread->consumer_err = false;
                        wakeup_producer(ctx->pthread);
                        return NULL;
                    }
                }

                num = 0;
            }
            current_block++;
        }
        if (num)
        {
            if (hd_write_block(ctx,
                               disk,
                               part,
                               data->cache + (i - num) * COPY_BLOCK_SIZE * disk->sector_size ,
                               begin,
                               num) != 0)
            {
                hd_warn(ctx, "consumer_thread hd_write_block is failed");
                ctx->pthread->consumer_eof = true;
                consumer->active = false;
                ctx->pthread->consumer_err = false;
                wakeup_producer(ctx->pthread);
                return NULL;
            }
        }

        set_cache(ctx, data, CACHE_FLAG_FREE);
    }

    consumer->active = false;

    /*FIXME:ctx->pthread->consumer should be an array*/
    if (!ctx->pthread->consumer.active)
    {
        ctx->pthread->consumer_eof = true;
        if (ctx->pthread->producer_eof == true)
            ctx->pthread->eof = true;

        wakeup_producer(ctx->pthread);
    }

    if (!ctx->pthread->producer_err)
    {
        ctx->pthread->consumer_eof = true;
        return NULL;
    }
    return NULL;
}

static void
alloc_copy_cache(hd_context *ctx, uint32_t sector_size)
{
    size_t len;
    hd_copy_cache *p_cache;
    char *cache_addr;
    int i;

    if(ctx->pthread->copy_cache)
    {
        if(ctx->pthread->cache_num >= ctx->pthread->producer_thread_num * 2)
            return;
        hd_free(ctx, ctx->pthread->copy_cache);
    }

    len = sizeof(hd_copy_cache) + COPY_CACHE_SIZE * sector_size + COPY_IO_UNIT_SIZE * sector_size;
    ctx->pthread->cache_num = ctx->pthread->producer_thread_num * 2;
    ctx->pthread->copy_cache = calloc(ctx->pthread->cache_num, len);
    if(!ctx->pthread->copy_cache)
        hd_throw(ctx, HD_ERROR_GENERIC, "alloc_copy_cache is failed");

    p_cache = ctx->pthread->copy_cache;
    cache_addr = (char*)ctx->pthread->copy_cache + (ctx->pthread->producer_thread_num * 2)*sizeof(hd_copy_cache);
    for(i=0; i<ctx->pthread->producer_thread_num*2; i++) {
        p_cache->firstblock = 0;
        p_cache->block_num = 0;
        p_cache->cache = cache_addr;
        p_cache->iocache = cache_addr + COPY_CACHE_SIZE * sector_size;
        p_cache->cache_size = COPY_CACHE_SIZE;
        p_cache->cache_flag = CACHE_FLAG_FREE;
        list_add_tail(&p_cache->list, &ctx->pthread->free_cache);

        p_cache++;
        cache_addr += COPY_CACHE_SIZE * sector_size + COPY_IO_UNIT_SIZE * sector_size;
    }
}

void
hd_new_pthread_context(hd_context *ctx)
{
    ctx->pthread = hd_malloc_struct(ctx, hd_pthread_context);
    ctx->pthread->refs = 1;
}

void
hd_pthread_init(hd_context *ctx, hd_disk *disk, hd_part *part)
{
    pthread_attr_init(&ctx->pthread->thread_attr);
    pthread_attr_setdetachstate(&ctx->pthread->thread_attr, PTHREAD_CREATE_DETACHED);

    pthread_mutex_init(&ctx->pthread->mutex, NULL);
    if ((pthread_cond_init(&ctx->pthread->producer_cond, NULL) != 0) ||
            pthread_cond_init(&ctx->pthread->consumer_cond, NULL) != 0)
        hd_throw(ctx, HD_ERROR_GENERIC, "pthread_cond_init is failed");

    /*FIXME:Set a single producer thread*/
    ctx->pthread->producer_thread_num = ctx->pthread->consumer_thread_num = 1;

    ctx->pthread->cache_num = 0;
    ctx->pthread->copy_cache = NULL;
    INIT_LIST_HEAD(&ctx->pthread->avail_cache);
    INIT_LIST_HEAD(&ctx->pthread->free_cache);

    alloc_copy_cache(ctx, disk->sector_size);

    ctx->pthread->eof = ctx->pthread->producer_eof = ctx->pthread->consumer_eof = false;
    ctx->pthread->producer_err = ctx->pthread->consumer_err = true;
}

void
hd_producer_create(hd_context *ctx, hd_disk *disk, hd_part *part)
{
    /*FIXME:ctx->pthread->producer should be an array*/
    int index = 1;
    ctx->pthread->producer.ctx = ctx;
    ctx->pthread->producer.part = part;
    ctx->pthread->producer.disk = disk;
    ctx->pthread->producer.index = 0;
    if (pthread_create(&ctx->pthread->producer.threadid, &ctx->pthread->thread_attr,
                       producer_thread, &ctx->pthread->producer) < 0)
        hd_throw(ctx,HD_ERROR_GENERIC, "pthread_create consumer failed	: %d\n", errno);

    ctx->pthread->producer_thread_num = index;

}
void
hd_consumer_create(hd_context *ctx, hd_disk *disk, hd_part *part)
{

    /*FIXME:ctx->pthread->consumer should be an array*/
    int index = 1;
    ctx->pthread->consumer.ctx = ctx;
    ctx->pthread->consumer.part = part;
    ctx->pthread->consumer.disk = disk;
    ctx->pthread->consumer.index = 0;
    if (pthread_create(&ctx->pthread->consumer.threadid, &ctx->pthread->thread_attr,
                       consumer_thread, &ctx->pthread->consumer) < 0)
        hd_throw(ctx,HD_ERROR_GENERIC, "pthread_create consumer failed	: %d\n", errno);

    ctx->pthread->consumer_thread_num = index;
}

void
hd_start_producer(hd_context *ctx, hd_disk *disk, hd_part *part)
{
    producer_thread(&ctx->pthread->producer);
}

void
hd_while_pthread(hd_context *ctx)
{
    while(!ctx->pthread->consumer_eof)
    {
        hd_warn(ctx, "consumer_eof = false, sending signal...");
        wakeup_consumer(ctx->pthread);
        xusleep(1000);
    }
}

void
hd_pthread_deinit(hd_context *ctx)
{
    if (!ctx->pthread)
        return;

    pthread_mutex_destroy(&ctx->pthread->mutex);
    pthread_cond_destroy(&ctx->pthread->producer_cond);
    pthread_cond_destroy(&ctx->pthread->consumer_cond);

    hd_free(ctx, ctx->pthread->copy_cache);
}

void
hd_drop_pthread_context(hd_context *ctx)
{
    if (!ctx)
        return;

    if (hd_drop_imp(ctx, ctx->pthread, &ctx->pthread->refs))
    {
        hd_free(ctx, ctx->pthread);
        ctx->pthread = NULL;
    }
}