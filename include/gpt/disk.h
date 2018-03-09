//
// Created by sjw on 2018/3/1.
//

#ifndef DISKCLONE_GPT_DISK_H
#define DISKCLONE_GPT_DISK_H

struct gpt_disk_s
{
    hd_disk super;

    unsigned char *first_sector; /* buffer with master boot record */

    /* gpt specific part */
    gpt_header	*pheader;	/* primary header */
    gpt_header	*bheader;	/* backup header */
    gpt_entry	*ents;		/* entries (partitions) */
};

#endif //DISKCLONE_GPT_DISK_H
