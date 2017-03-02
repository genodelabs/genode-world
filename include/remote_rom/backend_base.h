/*
 * \brief  Backend interface used by the Remote_rom client and server
 * \author Johannes Schlatow
 * \date   2016-02-17
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef __INCLUDE__REMOTE_ROM__BACKEND_BASE_H_
#define __INCLUDE__REMOTE_ROM__BACKEND_BASE_H_

#include <rom_forwarder.h>
#include <rom_receiver.h>

namespace Remote_rom {
	struct Backend_server_base;
	struct Backend_client_base;

	class Exception : public ::Genode::Exception { };
	Backend_server_base &backend_init_server(Genode::Env &env, Genode::Allocator &alloc);
	Backend_client_base &backend_init_client(Genode::Env &env, Genode::Allocator &alloc);
};

struct Remote_rom::Backend_server_base
{
	virtual void send_update() = 0;
	virtual void register_forwarder(Rom_forwarder_base *forwarder) = 0;
};

struct Remote_rom::Backend_client_base
{
	virtual void register_receiver(Rom_receiver_base *receiver) = 0;
};

#endif
