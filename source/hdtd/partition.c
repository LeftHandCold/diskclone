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
hd_new_part_handler_context(hd_context *ctx)
{
	ctx->part = hd_malloc_struct(ctx, hd_part_handler_context);
	ctx->part->refs = 1;
	ctx->part->count = 0;
}

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

void *
hd_new_part_of_size(hd_context *ctx, int size)
{
    hd_part *part = hd_calloc(ctx, 1, size);
    part->refs = 1;
    return part;
}

hd_part *
hd_open_part(hd_context *ctx, hd_disk *disk, const char *partname)
{
    int i;
    hd_part_handler_context *ph;

    if (partname == NULL)
        hd_throw(ctx, HD_ERROR_GENERIC, "no part to open");

    ph = ctx->part;
    if (ph->count == 0)
        hd_throw(ctx, HD_ERROR_GENERIC, "No part handlers registered");

    for (i = 0; i < ph->count; i++)
    {
        int rc = 0;
        hd_part *part;
        part = ph->handler[i]->open(ctx, disk, partname);
        part->name = partname;
        if (part->probe_part)
            rc = part->probe_part(ctx, disk, part);


        if(rc != 1)
        {
            hd_drop_disk(ctx, disk);
            continue;
        }

        return part;
    }

    hd_throw(ctx, HD_ERROR_GENERIC, "Could not find the specified part handler");
}

void
hd_clone_part(hd_context *ctx, hd_disk *disk, hd_part *part)
{
    if (part->clone_part)
        part->clone_part(ctx, disk, part);
    else
        hd_throw(ctx, HD_ERROR_GENERIC, "Could not find the specified clone_part");
}

void
hd_clone_part_info(hd_context *ctx, hd_disk *disk, hd_part *part)
{
    unsigned char *buf;
    hd_try(ctx)
    {
        /*TODO:test mbr*/
        buf = hd_malloc(ctx, 2048 * 512);
        hd_read_write_device(ctx, disk->dev_fd, false, buf, 0, 2 * disk->sector_size);
        hd_read_write_device(ctx, disk->disk_dest->dev_fd, true, buf, 0, 2 * disk->sector_size);
    }
    hd_catch(ctx)
    {
        hd_free(ctx, buf);
        hd_rethrow(ctx);
    }
}

void
hd_drop_part(hd_context *ctx, hd_part *part)
{
    if (hd_drop_imp(ctx, part, &part->refs))
    {
        if (part->drop_part)
            part->drop_part(ctx, part);
        hd_free(ctx, part);
    }
}

void
hd_drop_part_handler_context(hd_context *ctx)
{
	if (!ctx)
		return;

	if (hd_drop_imp(ctx, ctx->part, &ctx->part->refs))
	{
		hd_free(ctx, ctx->part);
		ctx->part = NULL;
	}
}

static bool
mark_bit(uint64_t block,unsigned char *bitmap)
{
    uint64_t byte = (block >> 3);//block/8;
    uint8_t mask = (1 << ((uint8_t)block&7));//block%8
    //check capacity of bitmap?
    bitmap[byte] |= mask;
    return true;
}

bool
hd_read_bit(unsigned char *bitmap, uint64_t block)
{
    uint64_t byte = (block >> 3);	//block/8;
    uint8_t mask = (1 << ((uint8_t)block&7));//block%8

    if (mask & bitmap[byte])
        return true;
    else
        return false;
}

void
hd_set_data_bitmap(hd_context *ctx, hd_part *part, uint64_t sector, uint64_t num)
{
    uint64_t firstblk;
    uint64_t lastblk;

    firstblk = sector / COPY_BLOCK_SIZE;
    lastblk = (sector + num - 1) / COPY_BLOCK_SIZE;

    while (firstblk<=lastblk)
    {
        if (!hd_read_bit(part->bitmap,firstblk))
        {
            mark_bit(firstblk, part->bitmap);
        }
        firstblk++;
    }
}