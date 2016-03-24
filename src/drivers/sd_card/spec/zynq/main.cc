/*
 * \brief  SD-card driver for Zynq boards
 * \author Norman Feske
 * \authot Timo Wischer
 * \date   2014-09-21
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <os/server.h>

/* local includes */
#include <driver.h>


struct Main
{
	Server::Entrypoint &ep;


	struct Factory : Block::Driver_factory
	{
		Block::Driver *create() {
			return new (Genode::env()->heap()) Block::Sdhci_driver(false, true); }

		void destroy(Block::Driver *driver) {
			Genode::destroy(Genode::env()->heap(),
			                static_cast<Block::Sdhci_driver *>(driver)); }
	} factory;

	Block::Root root;

	Main(Server::Entrypoint &ep)
	: ep(ep), root(ep, Genode::env()->heap(), factory)
	{
		Genode::printf("--- SD card driver ---\n");

		Genode::env()->parent()->announce(ep.manage(root));
	}
};


/************
 ** Server **
 ************/

namespace Server {
	char const *name()             { return "sd_card_ep";        }
	size_t stack_size()            { return 2*1024*sizeof(long); }
	void construct(Entrypoint &ep) { static Main server(ep);     }
}

