//
// Created by sjw on 2018/3/2.
//

#include "hdtd.h"
#include "hdtd-imp.h"

enum
{
    HD_PART_HANDLER_MAX = 10
};

struct hd_part_handler_context_s
{
    int refs;
    int count;
    const hd_part_handler *handler[HD_PART_HANDLER_MAX];
};

void
hd_register_part_handler(hd_context *ctx, const hd_part_handler *handler)
{
    hd_part_handler_context *pc;
    int i;

    if (!handler)
        return;

    pc = ctx->part;
    if (pc == NULL)
        hd_throw(ctx, HD_ERROR_GENERIC, "part handler list not found");

    for (i = 0; i < pc->count; i++)
        if (pc->handler[i] == handler)
            return;

    if (pc->count >= HD_PART_HANDLER_MAX)
        hd_throw(ctx, HD_ERROR_GENERIC, "Too many part handlers");

    pc->handler[pc->count++] = handler;
}
