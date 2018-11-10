#ifndef __FAKE_STRING_H__
#define __FAKE_STRING_H__

#include "stdint.h"

static inline int memcmp(const void *p0, const void *p1, size_t size)
{
	const unsigned char *c0 = (const unsigned char *)p0;
	const unsigned char *c1 = (const unsigned char *)p1;

	size_t i;
	for (i = 0; i < size; i++)
		if (c0[i] != c1[i]) return c0[i] - c1[i];

	return 0;
}

void *memcpy(void *dst, const void *src, size_t size);
void *memset(void *dst, int i, size_t size);

typedef void FILE;

static inline void assert() { }
static inline int fprintf(FILE * restrict stream, const char	* restrict format, ...) { return -1; }
static inline int fputc(int c, FILE *stream) { return -1; }

#endif
