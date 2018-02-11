//
// Created by sjw on 2018/1/15.
//

#ifndef DISKCLONE_HDTD_STRING_UTIL_H
#define DISKCLONE_HDTD_STRING_UTIL_H

#include "hdtd/system.h"

/* The Unicode character used to incoming character whose value is unknown or unrepresentable. */
#define HD_REPLACEMENT_CHARACTER 0xFFFD

/*
	Safe string functions
*/

/*
	hd_strsep: Given a pointer to a C string (or a pointer to NULL) break
	it at the first occurrence of a delimiter char (from a given set).

	stringp: Pointer to a C string pointer (or NULL). Updated on exit to
	point to the first char of the string after the delimiter that was
	found. The string pointed to by stringp will be corrupted by this
	call (as the found delimiter will be overwritten by 0).

	delim: A C string of acceptable delimiter characters.

	Returns a pointer to a C string containing the chars of stringp up
	to the first delimiter char (or the end of the string), or NULL.
*/
char *hd_strsep(char **stringp, const char *delim);

/*
	hd_strlcpy: Copy at most n-1 chars of a string into a destination
	buffer with null termination, returning the real length of the
	initial string (excluding terminator).

	dst: Destination buffer, at least n bytes long.

	src: C string (non-NULL).

	n: Size of dst buffer in bytes.

	Returns the length (excluding terminator) of src.
*/
size_t hd_strlcpy(char *dst, const char *src, size_t n);

/*
	hd_strlcat: Concatenate 2 strings, with a maximum length.

	dst: pointer to first string in a buffer of n bytes.

	src: pointer to string to concatenate.

	n: Size (in bytes) of buffer that dst is in.

	Returns the real length that a concatenated dst + src would have been
	(not including terminator).
*/
size_t hd_strlcat(char *dst, const char *src, size_t n);

/*
	hd_chartorune: UTF8 decode a single rune from a sequence of chars.

	rune: Pointer to an int to assign the decoded 'rune' to.

	str: Pointer to a UTF8 encoded string.

	Returns the number of bytes consumed. Does not throw exceptions.
*/
int hd_chartorune(int *rune, const char *str);



/*
	hd_utflen: Count how many runes the UTF-8 encoded string
	consists of.

	s: The UTF-8 encoded, NUL-terminated text string.

	Returns the number of runes in the string.
*/
int hd_utflen(const char *s);




#endif //DISKCLONE_HDTD_STRING_UTIL_H
