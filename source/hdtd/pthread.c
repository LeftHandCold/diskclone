//
// Created by sjw on 2018/3/7.
//

#include "hdtd.h"
#include "hdtd-imp.h"

static void *
producer_thread(void * arg)
{

}

static void *
consumer_thread(void * arg)
{

}

void
hd_new_pthread_context(hd_context *ctx)
{
    ctx->pthread = hd_malloc_struct(ctx, hd_part_handler_context);
    ctx->pthread->refs = 1;
}

void
hd_pthread_init(hd_context *ctx, hd_part *part)
{
    pthread_attr_init(&ctx->pthread->thread_attr);
    pthread_attr_setdetachstate(&ctx->pthread->thread_attr, PTHREAD_CREATE_DETACHED);

    pthread_mutex_init(&ctx->pthread->mutex, NULL);
    if ((pthread_cond_init(&ctx->pthread->producer_cond, NULL) != 0) ||
            pthread_cond_init(&ctx->pthread->consumer_cond, NULL) != 0)
        hd_throw(ctx, HD_ERROR_GENERIC, "pthread_cond_init is failed");

    /*FIXME:Set a single producer thread*/
    ctx->pthread->producer_thread_num = ctx->pthread->consumer_thread_num = 1;
}

void
hd_pthread_deinit(hd_context *ctx)
{
    if (!ctx->pthread)
        return;

    pthread_mutex_destroy(&ctx->pthread->mutex);
    pthread_cond_destroy(&ctx->pthread->producer_cond);
    pthread_cond_destroy(&ctx->pthread->consumer_cond);
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