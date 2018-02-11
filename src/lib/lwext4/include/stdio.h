/*
 * \brief  Libc functions for lwext4
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _STDIO_H
#define _STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
# if __cplusplus
#  define NULL 0L
# else
#  define NULL ((void*)0)
# endif
#endif

typedef int FILE;

#define stdout ((FILE*)0)
#define stderr ((FILE*)2)

int fflush(FILE *);
int printf(char const *, ...);
int puts(char const*);

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H */
