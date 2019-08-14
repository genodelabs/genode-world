/**
 * \brief  Stand alone environment for FDI
 * \author Sebastian Sumpf
 * \date   2019-07-19
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <util/endian.h>
#include <util/string.h>

extern "C" {
#include "libfdt_env.h"
}


/******************
 ** libfdt_env.h **
 ******************/

uint16_t fdt16_to_cpu(fdt16_t x) { return host_to_big_endian(x); }
uint32_t fdt32_to_cpu(fdt32_t x) { return host_to_big_endian(x); }
uint64_t fdt64_to_cpu(fdt64_t x) { return host_to_big_endian(x); }

fdt16_t cpu_to_fdt16(uint16_t x) { return host_to_big_endian(x); }
fdt32_t cpu_to_fdt32(uint32_t x) { return host_to_big_endian(x); }
fdt64_t cpu_to_fdt64(uint64_t x) { return host_to_big_endian(x); }


/**************
 ** string.h **
 **************/


void *memchr(const void *s, int c, size_t n)
{
	const unsigned char *p = (unsigned char *)s;
	while (n-- != 0) {
		if ((unsigned char)c == *p++) {
			return (void *)(p - 1);
		}
	}
	return nullptr;
}


int memcmp(const void *s1, const void *s2, size_t n) {
	return Genode::memcmp(s1, s2, n); }


void *memcpy(void *dest, const void *src, size_t n) {
	return Genode::memcpy(dest, src, n); }


void *memmove(void *dest, const void *src, size_t n) {
	return Genode::memmove(dest, src, n); }


void *memset(void *s, int c, size_t n) {
	return Genode::memset(s, c, n); }


char *strchr(const char *s, int c)
{
	char ch = c;

	for (;; ++s) {
		if (*s == ch)
			return ((char *)s);
		if (*s == '\0')
			break;
	}

	return nullptr;
}


char *strrchr(const char *s, int c)
{
	char *save;
	char ch = c;

	for (save = nullptr;; ++s) {
		if (*s == ch)
			save = (char *)s;
		if (*s == '\0')
			return (save);
	}
	/* NOTREACHED */
}


size_t strlen(const char *s) {
	return Genode::strlen(s); }


size_t strnlen(const char *s, size_t maxlen)
{
	for (size_t c = 0; c < maxlen; c++)
		if (!s[c])
			return c;

	return maxlen;
}


/**************
 ** stdlib.h **
 **************/

unsigned long int strtoul(const char *nptr, char **endptr, int base)
{
	unsigned long r = 0;
	size_t read = Genode::ascii_to_unsigned(nptr, r, base);

	if (endptr)
		*endptr = (char *)(nptr + read);

	return r;
}

