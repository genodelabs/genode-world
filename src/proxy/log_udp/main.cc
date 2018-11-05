/*
 * \brief  Receive LOG messages via UDP and forward to LOG session.
 * \author Johannes Schlatow
 * \date   2018-11-02
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/env.h>
#include <base/heap.h>
#include <base/log.h>

#include "receiver.h"

#include <base/component.h>
#include <base/attached_rom_dataspace.h>


using namespace Net;

namespace Log_udp {
	using Genode::size_t;
	using Genode::Attached_rom_dataspace;
	using Genode::Xml_node;

	struct Main;
};

struct Log_udp::Main
{
	Genode::Env &env;
	Genode::Heap            heap     { &env.ram(), &env.rm() };
	Attached_rom_dataspace  config   { env, "config" };
	Receiver                receiver { env, heap, config.xml() };


	Main(Genode::Env &env) : env(env)
	{ }

};

namespace Component {
	Genode::size_t stack_size()    { return 2*1024*sizeof(long); }
	void construct(Genode::Env &env)
	{
		env.exec_static_constructors();
		static Log_udp::Main main(env);
	}
}
