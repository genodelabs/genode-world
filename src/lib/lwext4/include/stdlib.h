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

#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void  free(void *);

void qsort(void *, size_t, size_t, int (*)(void const*, void const *));

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_H */
