//
// Created by sjw on 11/02/2018.
//

#include "hdtd.h"

/* Warning context */

void hd_var_imp(void *var)
{
	/* Do nothing */
}

void hd_flush_warnings(hd_context *ctx)
{
	if (ctx->warn->count > 1)
	{
		fprintf(stderr, "warning: ... repeated %d times ...\n", ctx->warn->count);
	}
	ctx->warn->message[0] = 0;
	ctx->warn->count = 0;
}

void hd_vwarn(hd_context *ctx, const char *fmt, va_list ap)
{
	char buf[sizeof ctx->warn->message];

	snprintf(buf, sizeof buf, fmt, ap);
	buf[sizeof(buf) - 1] = 0;
#ifdef USE_OUTPUT_DEBUG_STRING
	OutputDebugStringA(buf);
	OutputDebugStringA("\n");
#endif

	if (!strcmp(buf, ctx->warn->message))
	{
		ctx->warn->count++;
	}
	else
	{
		hd_flush_warnings(ctx);
		fprintf(stderr, "warning: %s\n", buf);
		hd_strlcpy(ctx->warn->message, buf, sizeof ctx->warn->message);
		ctx->warn->count = 1;
	}
}

void hd_warn(hd_context *ctx, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	hd_vwarn(ctx, fmt, ap);
	va_end(ap);
}

/* Only called when we hit the bottom of the exception stack.
 * Do the same as hd_throw, but don't actually throw. */
static int hd_fake_throw(hd_context *ctx, int code, const char *fmt, ...)
{
	va_list args;
	ctx->error->errcode = code;
	va_start(args, fmt);
	snprintf(ctx->error->message, sizeof ctx->error->message, fmt, args);
	ctx->error->message[sizeof(ctx->error->message) - 1] = 0;
	va_end(args);

	if (code != HD_ERROR_ABORT)
	{
		hd_flush_warnings(ctx);
		fprintf(stderr, "error: %s\n", ctx->error->message);
#ifdef USE_OUTPUT_DEBUG_STRING
		OutputDebugStringA("error: ");
		OutputDebugStringA(ctx->error->message);
		OutputDebugStringA("\n");
#endif
	}

	/* We need to arrive in the always/catch block as if throw
	 * had taken place. */
	ctx->error->top++;
	ctx->error->top->code = 2;
	return 0;
}

HD_NORETURN static void throw(hd_context *ctx)
{
	if (ctx->error->top >= ctx->error->stack)
	{
		ctx->error->top->code += 2;
		hd_longjmp(ctx->error->top->buffer, 1);
	}
	else
	{
		fprintf(stderr, "uncaught exception: %s\n", ctx->error->message);
#ifdef USE_OUTPUT_DEBUG_STRING
		OutputDebugStringA("uncaught exception: ");
		OutputDebugStringA(ctx->error->message);
		OutputDebugStringA("\n");
#endif
		exit(EXIT_FAILURE);
	}
}

void hd_vthrow(hd_context *ctx, int code, const char *fmt, va_list ap)
{
	ctx->error->errcode = code;
	snprintf(ctx->error->message, sizeof ctx->error->message, fmt, ap);
	ctx->error->message[sizeof(ctx->error->message) - 1] = 0;

	if (code != HD_ERROR_ABORT)
	{
		hd_flush_warnings(ctx);
		fprintf(stderr, "error: %s\n", ctx->error->message);
#ifdef USE_OUTPUT_DEBUG_STRING
		OutputDebugStringA("error: ");
		OutputDebugStringA(ctx->error->message);
		OutputDebugStringA("\n");
#endif
	}

	throw(ctx);
}

void hd_throw(hd_context *ctx, int code, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	hd_vthrow(ctx, code, fmt, ap);
	va_end(ap);
}

void hd_rethrow(hd_context *ctx)
{
	assert(ctx && ctx->error && ctx->error->errcode >= HD_ERROR_NONE);
	throw(ctx);
}

void hd_rethrow_if(hd_context *ctx, int err)
{
	assert(ctx && ctx->error && ctx->error->errcode >= HD_ERROR_NONE);
	if (ctx->error->errcode == err)
		hd_rethrow(ctx);
}

int hd_caught(hd_context *ctx)
{
	assert(ctx && ctx->error && ctx->error->errcode >= HD_ERROR_NONE);
	return ctx->error->errcode;
}

const char *hd_caught_message(hd_context *ctx)
{
	assert(ctx && ctx->error && ctx->error->errcode >= HD_ERROR_NONE);
	return ctx->error->message;
}

int hd_push_try(hd_context *ctx)
{
	/* If we would overflow the exception stack, throw an exception instead
	 * of entering the try block. We assume that we always have room for
	 * 1 extra level on the stack here - i.e. we throw the error on us
	 * starting to use the last level. */
	if (ctx->error->top + 2 >= ctx->error->stack + nelem(ctx->error->stack))
		return hd_fake_throw(ctx, HD_ERROR_GENERIC, "exception stack overflow!");

	ctx->error->top++;
	ctx->error->top->code = 0;
	return 1;
}