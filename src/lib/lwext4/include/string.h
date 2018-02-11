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


#ifndef _STRING_H
#define _STRING_H

#ifdef __cplusplus
extern "C" {
#endif

int   memcmp(void const *, void const *, size_t);
void *memcpy(void *, void const *, size_t);
void *memmove(void *, void const *, size_t);
void *memset(void *, int, size_t);

int     strcmp(char const *, char const *);
int     strncmp(char const *, char const *, size_t);
char   *strcpy(char *, char const *);
char   *strncpy(char *, char const *, size_t);
size_t  strlen(char const *);

#ifdef __cplusplus
}
#endif

#endif /* _STRING_H */
