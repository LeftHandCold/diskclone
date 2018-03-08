//
// Created by sjw on 2018/2/28.
//

#include "hdtd-imp.h"

void
hd_drop_context(hd_context *ctx)
{
    if (!ctx)
        return;

    /* Other finalisation calls go here (in reverse order) */
    hd_drop_disk_handler_context(ctx);
    hd_drop_part_handler_context(ctx);
    hd_drop_pthread_context(ctx);

    if (ctx->warn)
    {
        hd_flush_warnings(ctx);
        hd_free(ctx, ctx->warn);
    }

    if (ctx->error)
    {
        //assert(ctx->error->top == ctx->error->stack - 1);
        hd_free(ctx, ctx->error);
    }

    /* Free the context itself */
    ctx->alloc->free(ctx->alloc->user, ctx);
}

/* Allocate new context structure, and initialise allocator, and sections
 * that aren't shared between contexts.
 */
static hd_context *
new_context_phase1(const hd_alloc_context *alloc)
{
    hd_context *ctx;

    ctx = alloc->malloc(alloc->user, sizeof(hd_context));
    if (!ctx)
        return NULL;
    memset(ctx, 0, sizeof *ctx);
    ctx->alloc = alloc;

    ctx->error = Memento_label(hd_malloc_no_throw(ctx, sizeof(hd_error_context)), "hd_error_context");
    if (!ctx->error)
        goto cleanup;
    ctx->error->top = ctx->error->stack - 1;
    ctx->error->errcode = HD_ERROR_NONE;
    ctx->error->message[0] = 0;

    ctx->warn = Memento_label(hd_malloc_no_throw(ctx, sizeof(hd_warn_context)), "hd_warn_context");
    if (!ctx->warn)
        goto cleanup;
    ctx->warn->message[0] = 0;
    ctx->warn->count = 0;


    return ctx;

    cleanup:
    fprintf(stderr, "cannot create context (phase 1)\n");
    hd_drop_context(ctx);
    return NULL;
}

hd_context *
hd_new_context_imp(const hd_alloc_context *alloc, const char *version)
{
    hd_context *ctx;

    if (strcmp(version, HD_VERSION))
    {
        fprintf(stderr, "cannot create context: incompatible header (%s) and library (%s) versions\n", version, HD_VERSION);
        return NULL;
    }


    if (!alloc)
        alloc = &hd_alloc_default;

    ctx = new_context_phase1(alloc);
    if (!ctx)
        return NULL;


    /* Now initialise sections that are shared */
    hd_try(ctx)
    {
        hd_new_disk_handler_context(ctx);
        hd_new_part_handler_context(ctx);
        hd_new_pthread_context(ctx);
    }
    hd_catch(ctx)
    {
        fprintf(stderr, "cannot create context (phase 2)\n");
        hd_drop_context(ctx);
        return NULL;
    }
    return ctx;
}