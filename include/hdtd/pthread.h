//
// Created by sjw on 2018/3/7.
//

#ifndef DISKCLONE_HDTD_PTHREAD_H
#define DISKCLONE_HDTD_PTHREAD_H

#include "hdtd/system.h"
#include "hdtd/context.h"

#include "hdtd/partition.h"

#include <pthread.h>

struct hd_pthread_context_s
{
    int refs;

    pthread_attr_t thread_attr;
    pthread_cond_t consumer_cond;
    pthread_cond_t producer_cond;
    pthread_mutex_t mutex;
    int consumer_thread_num;
    int producer_thread_num;
};

void hd_pthread_init(hd_context *ctx, hd_part *part);
void hd_pthread_deinit(hd_context *ctx);
#endif //DISKCLONE_HDTD_PTHREAD_H
