/*
 * \brief  Back end used for obtaining multi-boot modules
 * \author Norman Feske
 * \author Markus Partheymueller
 * \date   2011-11-20
 */

/*
 * Copyright (C) 2011-2025 Genode Labs GmbH
 * Copyright (C) 2012 Intel Corporation
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Vancouver VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

#pragma once

/* Genode includes */
#include <util/misc_math.h>
#include <base/node.h>
#include <base/log.h>
#include <rom_session/connection.h>


using Genode::Node;


class Boot_module_provider
{
	private:

		enum { MODULE_NAME_MAX_LEN = 48 };

		typedef Genode::String<MODULE_NAME_MAX_LEN> Name;

		Genode::size_t _import_module(Genode::Env    & env,
		                              Node     const & mod_node,
		                              void           * dst,
		                              Genode::size_t dst_len) const
		{
			using namespace Genode;

			if (mod_node.has_type("rom")) {

				/*
				 * Determine ROM file name, which is specified as 'label'
				 * attribute of the 'rom' node. If no 'label' argument is
				 * provided, use the 'name' attribute as file name.
				 */
				Name const label = mod_node.has_attribute("label")
				                 ? mod_node.attribute_value("label", Name())
				                 : mod_node.attribute_value("name",  Name());
				/*
				 * Open ROM session
				 */
				Genode::Rom_connection rom(env, label.string());
				auto ds = rom.dataspace();
				Genode::size_t const src_len = Dataspace_client(ds).size();

				if (src_len > dst_len) {
					warning(__func__, ": src_len=", src_len, " dst_len=", dst_len);
					error("Boot_module_provider: destination buffer too small");
					return 0;
				}

				Attached_dataspace tmp(env.rm(), ds);
				Genode::memcpy(dst, tmp.local_addr<void>(), src_len);

				return src_len;
			}

			warning("XML node ", mod_node.type(), " in multiboot unknown");

			return 0;
		}

		/**
		 * Copy command line to specified buffer
		 *
		 * \return length of command line in bytes, or 0 if module does not exist
		 */
		size_t _cmdline(Node const & node, char *dst, size_t dst_len) const
		{
			using namespace Genode;

			auto const name = node.attribute_value("name", Name());

			if (!name.length() || name.length() > dst_len)
				return 0;

			/* copy name to command line including zero termination */
			::memcpy(dst, name.string(), name.length());

			if (!node.has_attribute("cmdline"))
				return name.length();

			typedef String<256> Cmdline;
			auto const cmdline = node.attribute_value("cmdline", Cmdline());

			if (!cmdline.length() || cmdline.length() > dst_len - name.length())
				return 0;

			/* replace zero termination with space between name and arguments */
			dst[name.length() - 1] = ' ';

			/* copy arguments to command line including zero termination */
			::memcpy(dst + name.length(), cmdline.string(), cmdline.length());

			return name.length() + cmdline.length();
		}

	public:

		Boot_module_provider() { }

		/**
		 * Copy module data to specified buffer
		 */
		void import_module_to_vm(Genode::Env & env, int module_index,
		                         char * const dst, Genode::size_t dst_len,
		                         auto const & fn) const
		{
			using namespace Genode;

			Attached_rom_dataspace config(env, "config");

			if (!config.valid())
				return;

			config.node().with_optional_sub_node("multiboot", [&](auto const & node) {
				int idx = 0;
				node.for_each_sub_node([&](auto const &module_node) {
					if (idx++ != module_index)
						return;

					auto const data_len = _import_module(env, module_node, dst,
				                                         dst_len);

					if (!data_len)
						return;

					/*
					 * Determine command line offset relative to the start of
					 * the loaded boot module. The command line resides right
					 * behind the module data, aligned on a page boundary.
					 */
					auto const cmdline_offset = Genode::align_addr(data_len, 12);

					if (cmdline_offset >= dst_len) {
						error("destination buffer too small for command line");
						fn(data_len, (char *)nullptr, 0);
						return;
					}

					/*
					 * Copy command line to guest RAM
					 */
					auto const cmd_len = _cmdline(module_node,
					                              dst + cmdline_offset,
					                              dst_len - cmdline_offset);

					fn(data_len, cmd_len ? dst + cmdline_offset : 0, cmd_len);
				});
			});
		}
};
