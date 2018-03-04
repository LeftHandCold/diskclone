//
// Created by sjw on 2018/2/28.
//

#ifndef DISKCLONE_HDTD_IMP_H
#define DISKCLONE_HDTD_IMP_H

#include "hdtd.h"

void hd_new_disk_handler_context(hd_context *ctx);
void hd_drop_disk_handler_context(hd_context *ctx);

void hd_new_part_handler_context(hd_context *ctx);
void hd_drop_part_handler_context(hd_context *ctx);

static inline void *
hd_keep_imp(hd_context *ctx, void *p, int *refs)
{
    if (p)
    {
        (void)Memento_checkIntPointerOrNull(refs);
        if (*refs > 0)
        {
            (void)Memento_takeRef(p);
            ++*refs;
        }
    }
    return p;
}

static inline int
hd_drop_imp(hd_context *ctx, void *p, int *refs)
{
    if (p)
    {
        int drop;
        if (*refs > 0)
            (void)Memento_dropRef(p);
        if (*refs > 0)
            drop = --*refs == 0;
        else
            drop = 0;
        return drop;
    }
    return 0;
}

#endif //DISKCLONE_HDTD_IMP_H
