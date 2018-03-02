//
// Created by sjw on 11/02/2018.
//

#ifndef DISKCLONE_HDTD_CONTEXT_H
#define DISKCLONE_HDTD_CONTEXT_H

#include "system.h"

/*
	Contexts
*/
typedef struct hd_alloc_context_s hd_alloc_context;
typedef struct hd_error_context_s hd_error_context;
typedef struct hd_error_stack_slot_s hd_error_stack_slot;
typedef struct hd_warn_context_s hd_warn_context;
typedef struct hd_disk_handler_context_s hd_disk_handler_context;
typedef struct hd_part_handler_context_s hd_part_handler_context;
typedef struct hd_context_s hd_context;

struct hd_alloc_context_s
{
    void *user;
    void *(*malloc)(void *, size_t);
    void *(*realloc)(void *, void *, size_t);
    void (*free)(void *, void *);
};

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

/*
	hd_malloc: Allocate a block of memory (with scavenging)

	size: The number of bytes to allocate.

	Returns a pointer to the allocated block. May return NULL if size is
	0. Throws exception on failure to allocate.
*/
void *hd_malloc(hd_context *ctx, size_t size);

/*
	hd_calloc: Allocate a zeroed block of memory (with scavenging)

	count: The number of objects to allocate space for.

	size: The size (in bytes) of each object.

	Returns a pointer to the allocated block. May return NULL if size
	and/or count are 0. Throws exception on failure to allocate.
*/
void *hd_calloc(hd_context *ctx, size_t count, size_t size);

/*
	hd_malloc_struct: Allocate storage for a structure (with scavenging),
	clear it, and (in Memento builds) tag the pointer as belonging to a
	struct of this type.

	CTX: The context.

	STRUCT: The structure type.

	Returns a pointer to allocated (and cleared) structure. Throws
	exception on failure to allocate.
*/
#define hd_malloc_struct(CTX, STRUCT) \
	((STRUCT *)Memento_label(hd_calloc(CTX,1,sizeof(STRUCT)), #STRUCT))

/*
	hd_malloc_no_throw: Allocate a block of memory (with scavenging)

	size: The number of bytes to allocate.

	Returns a pointer to the allocated block. May return NULL if size is
	0. Returns NULL on failure to allocate.
*/
void *hd_malloc_no_throw(hd_context *ctx, size_t size);

/*
	hd_free: Frees an allocation.

	Does not throw exceptions.
*/
void hd_free(hd_context *ctx, void *p);

/*
	hd_flush_warnings: Flush any repeated warnings.

	Repeated warnings are buffered, counted and eventually printed
	along with the number of repetitions. Call hd_flush_warnings
	to force printing of the latest buffered warning and the
	number of repetitions, for example to make sure that all
	warnings are printed before exiting an application.

	Does not throw exceptions.
*/
void hd_flush_warnings(hd_context *ctx);

struct hd_context_s
{
    const hd_alloc_context *alloc;
    hd_error_context *error;
    hd_warn_context *warn;
    hd_disk_handler_context *disk;
	hd_part_handler_context *part;
};

/*
	hd_new_context: Allocate context containing global state.

	The global state contains an exception stack, resource store,
	etc. Most functions in MuPDF take a context argument to be
	able to reference the global state. See hd_drop_context for
	freeing an allocated context.

	alloc: Supply a custom memory allocator through a set of
	function pointers. Set to NULL for the standard library
	allocator. The context will keep the allocator pointer, so the
	data it points to must not be modified or freed during the
	lifetime of the context.

	Does not throw exceptions, but may return NULL.
*/
hd_context *hd_new_context_imp(const hd_alloc_context *alloc, const char *version);

#define hd_new_context(alloc) hd_new_context_imp(alloc, HD_VERSION);

/*
	hd_drop_context: Free a context and its global state.

	The context and all of its global state is freed, and any
	buffered warnings are flushed (see hd_flush_warnings). If NULL
	is passed in nothing will happen.

	Does not throw exceptions.
*/
void hd_drop_context(hd_context *ctx);

/* Default allocator */
extern hd_alloc_context hd_alloc_default;
#endif //DISKCLONE_HDTD_CONTEXT_H
