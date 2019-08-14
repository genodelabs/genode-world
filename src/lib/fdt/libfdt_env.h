/**
 * \brief  Stand alone definitions for FDI
 * \author Sebastian Sumpf
 * \date   2019-07-19
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef LIBFDT_ENV_H
#define LIBFDT_ENV_H

#include <base/fixed_stdint.h>


/******************
 ** libfdt_env.h **
 ******************/

#define NULL ((void *)0)
#define INT_MAX 0x7fffffff

typedef genode_uint16_t fdt16_t;
typedef genode_uint32_t fdt32_t;
typedef genode_uint64_t fdt64_t;

typedef genode_uint8_t  uint8_t;
typedef genode_uint16_t uint16_t;
typedef genode_uint32_t uint32_t;
typedef genode_uint64_t uint64_t;

typedef unsigned long size_t;
typedef unsigned long uintptr_t;

uint16_t fdt16_to_cpu(fdt16_t x);
uint32_t fdt32_to_cpu(fdt32_t x);
uint64_t fdt64_to_cpu(fdt64_t x);

fdt16_t cpu_to_fdt16(uint16_t x);
fdt32_t cpu_to_fdt32(uint32_t x);
fdt64_t cpu_to_fdt64(uint64_t x);


/**************
 ** string.h **
 **************/

void *memchr(const void *s, int c, size_t n);
int   memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

char  *strchr(const char *s, int c);
char  *strrchr(const char *s, int c);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);


/**************
 ** stdlib.h **
 **************/

unsigned long int strtoul(const char *nptr, char **endptr, int base);

#endif /* LIBFDT_ENV_H */
