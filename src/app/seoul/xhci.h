/*
 * \brief  XHCI model powered by Qemu USB library
 * \author Alexander Boettcher
 * \date   2024-05-03
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Seoul VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

#pragma once

#include <base/env.h>
#include <base/heap.h>

#include <nul/motherboard.h>


namespace Seoul {
	class Xhci;
}

class Seoul::Xhci : public StaticReceiver<Xhci>
{
	public:

		Xhci(Genode::Env &, Genode::Heap &, Genode::Xml_node const &, Motherboard &);
};
