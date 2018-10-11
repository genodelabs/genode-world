/*
 * \brief  Interface used by the backend to write the ROM data received from the remote server
 * \author Johannes Schlatow
 * \date   2016-02-18
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef __INCLUDE__REMOTE_ROM__ROM_RECEIVER_H_
#define __INCLUDE__REMOTE_ROM__ROM_RECEIVER_H_

#include <base/stdint.h>
#include <util/interface.h>

namespace Remote_rom {
	using Genode::size_t;
	struct Rom_receiver_base;
}

struct Remote_rom::Rom_receiver_base : Genode::Interface
{
	virtual const char *module_name() const = 0;
	virtual char* start_new_content(size_t len) = 0;
	virtual void commit_new_content(bool abort=false) = 0;
};

#endif
