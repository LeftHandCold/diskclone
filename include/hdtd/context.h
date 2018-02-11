//
// Created by sjw on 11/02/2018.
//

#ifndef DISKCLONE_HDTD_CONTEXT_H
#define DISKCLONE_HDTD_CONTEXT_H

#include "system.h"

typedef struct hd_error_context_s hd_error_context;
typedef struct hd_error_stack_slot_s hd_error_stack_slot;
typedef struct hd_warn_context_s hd_warn_context;
typedef struct hd_context_s hd_context;

struct hd_error_stack_slot_s
{
	int code;
	hd_jmp_buf buffer;
};

struct hd_error_context_s
{
	hd_error_stack_slot *top;
	hd_error_stack_slot stack[256];
	int errcode;
	char message[256];
};

void hd_var_imp(void *);
#define hd_var(var) hd_var_imp((void *)&(var))

/*
	Exception macro definitions. Just treat these as a black box - pay no
	attention to the man behind the curtain.
*/

#define hd_try(ctx) \
	{ \
		if (hd_push_try(ctx)) { \
			if (hd_setjmp((ctx)->error->top->buffer) == 0) do \

#define hd_always(ctx) \
			while (0); \
		} \
		if (ctx->error->top->code < 3) { \
			ctx->error->top->code++; \
			do \

#define hd_catch(ctx) \
			while (0); \
		} \
	} \
	if ((ctx->error->top--)->code > 1)

int hd_push_try(hd_context *ctx);
HD_NORETURN void hd_vthrow(hd_context *ctx, int errcode, const char *, va_list ap);
HD_NORETURN void hd_throw(hd_context *ctx, int errcode, const char *, ...) __printflike(3, 4);
HD_NORETURN void hd_rethrow(hd_context *ctx);
void hd_vwarn(hd_context *ctx, const char *fmt, va_list ap);
void hd_warn(hd_context *ctx, const char *fmt, ...) __printflike(2, 3);
const char *hd_caught_message(hd_context *ctx);
int hd_caught(hd_context *ctx);
void hd_rethrow_if(hd_context *ctx, int errcode);

enum
{
	HD_ERROR_NONE = 0,
	HD_ERROR_MEMORY = 1,
	HD_ERROR_GENERIC = 2,
	HD_ERROR_SYNTAX = 3,
	HD_ERROR_TRYLATER = 4,
	HD_ERROR_ABORT = 5,
	HD_ERROR_COUNT
};

struct hd_warn_context_s
{
	char message[256];
	int count;
};

struct hd_context_s
{
	void *user;
	hd_error_context *error;
	hd_warn_context *warn;
};

#endif //DISKCLONE_HDTD_CONTEXT_H
