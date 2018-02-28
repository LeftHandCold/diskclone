//
// Created by sjw on 2018/2/28.
//

#include "hdtd.h"

void
hd_free(hd_context *ctx, void *p)
{
    ctx->alloc->free(ctx->alloc->user, p);
}

static void *
do_scavenging_malloc(hd_context *ctx, size_t size)
{
    void *p;
    int phase = 0;

    p = ctx->alloc->malloc(ctx->alloc->user, size);
    if (p != NULL)
        return p;

    return NULL;
}

static void *
do_scavenging_realloc(hd_context *ctx, void *p, size_t size)
{
    void *q;
    int phase = 0;

    q = ctx->alloc->realloc(ctx->alloc->user, p, size);
    if (q != NULL)
        return q;

    return NULL;
}

void *
hd_calloc(hd_context *ctx, size_t count, size_t size)
{
    void *p;

    if (count == 0 || size == 0)
        return 0;

    if (count > SIZE_MAX / size)
    {
        hd_throw(ctx, HD_ERROR_MEMORY, "calloc (" FMT_zu " x " FMT_zu " bytes) failed (size_t overflow)", count, size);
    }

    p = do_scavenging_malloc(ctx, count * size);
    if (!p)
    {
        hd_throw(ctx, HD_ERROR_MEMORY, "calloc (" FMT_zu " x " FMT_zu " bytes) failed", count, size);
    }
    memset(p, 0, count*size);
    return p;
}

void *
hd_malloc_no_throw(hd_context *ctx, size_t size)
{
    return do_scavenging_malloc(ctx, size);
}

void *
hd_malloc(hd_context *ctx, size_t size)
{
    void *p;

    if (size == 0)
        return NULL;

    p = do_scavenging_malloc(ctx, size);
    if (!p)
        hd_throw(ctx, HD_ERROR_MEMORY, "malloc of " FMT_zu " bytes failed", size);
    return p;
}

static void *
hd_malloc_default(void *opaque, size_t size)
{
    return malloc(size);
}

static void *
hd_realloc_default(void *opaque, void *old, size_t size)
{
    return realloc(old, size);
}

static void
hd_free_default(void *opaque, void *ptr)
{
    free(ptr);
}

hd_alloc_context hd_alloc_default =
{
        NULL,
        hd_malloc_default,
        hd_realloc_default,
        hd_free_default
};