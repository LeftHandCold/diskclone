//
// Created by sjw on 11/02/2018.
//

#ifndef DISKCLONE_HDTD_SYSTEM_H
#define DISKCLONE_HDTD_SYSTEM_H

#include <stddef.h> // needed for size_t

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <assert.h>
#include <errno.h>
#include <limits.h> /* INT_MAX & co */
#include <float.h> /* FLT_EPSILON, FLT_MAX & co */
#include <fcntl.h> /* O_RDONLY & co */
#include <time.h>

#include <stdbool.h>

#include <setjmp.h>

#include "memento.h"
#define nelem(x) (sizeof(x)/sizeof((x)[0]))

typedef unsigned short hd_wchar_t;

#if defined(_MSC_VER) && (_MSC_VER < 1700) /* MSVC older than VS2012 */
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef __int64 int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#ifndef INT64_MAX
#define INT64_MAX 9223372036854775807i64
#endif
#else
#include <stdint.h> /* needed for int64_t */
#endif

#include <sys/stat.h>
/*
	Some differences in libc can be smoothed over
*/

#ifdef __APPLE__

#include <unistd.h>
#define HAVE_SIGSETJMP
#elif defined(__unix) && !defined(__NACL__)
#define HAVE_SIGSETJMP
#endif

/*
	Where possible (i.e. on platforms on which they are provided), use
	sigsetjmp/siglongjmp in preference to setjmp/longjmp. We don't alter
	signal handlers within hdcontents, so there is no need for us to
	store/restore them - hence we use the non-restoring variants. This
	makes a large speed difference on MacOSX (and probably other
	platforms too.
*/
#ifdef HAVE_SIGSETJMP
#define hd_setjmp(BUF) sigsetjmp(BUF, 0)
#define hd_longjmp(BUF,VAL) siglongjmp(BUF, VAL)
#define hd_jmp_buf sigjmp_buf
#else
#define hd_setjmp(BUF) setjmp(BUF)
#define hd_longjmp(BUF,VAL) longjmp(BUF,VAL)
#define hd_jmp_buf jmp_buf
#endif

#ifndef hd_fopen
#define hd_fopen fopen
#endif
#ifndef hd_remove
#define hd_remove remove
#endif
#define hd_fseek fseek
#define hd_ftell ftell
typedef int hd_off_t;
#define hd_OFF_MAX INT_MAX
#define hd_atoo_imp atoi

/* Portable way to format a size_t */
#if defined(_WIN64)
#define FMT_zu "%llu"
#elif defined(_WIN32)
#define FMT_zu "%u"
#else
#define FMT_zu "%zu"
#endif

/* GCC can do type checking of printf strings */
#ifndef __printflike
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7
#define __printflike(fmtarg, firstvararg) \
	__attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#else
#define __printflike(fmtarg, firstvararg)
#endif
#endif

/* noreturn is a GCC extension */
#ifdef __GNUC__
#define HD_NORETURN __attribute__((noreturn))
#else
#ifdef _MSC_VER
#define HD_NORETURN __declspec(noreturn)
#else
#define HD_NORETURN
#endif
#endif

/* Flag unused parameters, for use with 'static inline' functions in headers. */
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7
#define HD_UNUSED __attribute__((__unused__))
#else
#define HD_UNUSED
#endif

static inline int64_t hd_max64(int64_t a, int64_t b)
{
	return (a > b ? a : b);
}

#endif //DISKCLONE_HDTD_SYSTEM_H
