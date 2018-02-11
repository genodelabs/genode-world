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

#ifndef _STDINT_H
#define _STDINT_H

#include <base/fixed_stdint.h>

typedef unsigned long   size_t;

typedef genode_uint8_t  uint8_t;
typedef genode_uint16_t uint16_t;
typedef genode_uint32_t uint32_t;
typedef genode_uint64_t uint64_t;

typedef genode_int32_t int32_t;
typedef genode_int64_t int64_t;

#endif /* _STDINT_H */
