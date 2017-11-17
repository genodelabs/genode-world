/*
 * \brief  Libretro configuration
 * \author Emery Hemingway
 * \date   2017-11-03
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__CONFIG_H_
#define _RETRO_FRONTEND__CONFIG_H_

/* Genode includes */
#include <base/attached_rom_dataspace.h>

namespace Retro_frontend {

	static unsigned config_version = 0;

	static Genode::Constructible<Genode::Attached_rom_dataspace> config_rom;

	Genode::Xml_node config_variables()
	{
		try {
			return config_rom->xml().sub_node("variables");
		} catch (...) {
			return Genode::Xml_node("<variables/>");
		}
	}
}

#endif
