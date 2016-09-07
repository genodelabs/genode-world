/*
 * \brief  Component bootstrap
 * \author Martin Stein
 * \date   2016-09-07
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/component.h>

Genode::size_t Component::stack_size() { return 128*1024*sizeof(long); }
