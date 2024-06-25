/*
 * \brief  Back end used for obtaining multi-boot modules
 * \author Norman Feske
 * \author Markus Partheymueller
 * \date   2011-11-20
 */

/*
 * Copyright (C) 2011-2024 Genode Labs GmbH
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

#ifndef _BOOT_MODULE_PROVIDER_H_
#define _BOOT_MODULE_PROVIDER_H_

/* Genode includes */
#include <util/string.h>
#include <util/misc_math.h>
#include <util/xml_node.h>
#include <base/log.h>
#include <rom_session/connection.h>


class Boot_module_provider
{
	private:

		Genode::Xml_node const _multiboot_node;

		enum { MODULE_NAME_MAX_LEN = 48 };

		typedef Genode::String<MODULE_NAME_MAX_LEN> Name;

	public:

		/**
		 * Exception class
		 */
		class Destination_buffer_too_small { };
		class Module_loading_failed { };

		/**
		 * Constructor
		 *
		 * \param multiboot_node  XML node containing the list of boot modules
		 *                        as sub nodes
		 */
		Boot_module_provider(Genode::Xml_node const &multiboot_node)
		:
			_multiboot_node(multiboot_node)
		{ }


		/************************************
		 ** Boot_module_provider interface **
		 ************************************/

		/**
		 * Copy module data to specified buffer
		 *
		 * \return module size in bytes, or 0 if module does not exist
		 * \throw  Destination_buffer_too_small
		 */
		Genode::size_t data(Genode::Env &env, int module_index,
		                    void *dst, Genode::size_t dst_len) const
		{
			using namespace Genode;

			try {
				Xml_node mod_node = _multiboot_node.sub_node(module_index);

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
					Rom_connection rom(env, label.string());
					Dataspace_capability ds = rom.dataspace();
					Genode::size_t const src_len = Dataspace_client(ds).size();

					if (src_len > dst_len) {
						warning(__func__, ": src_len=", src_len, " dst_len=", dst_len);
						throw Destination_buffer_too_small();
					}

					Attached_dataspace tmp(env.rm(), ds);
					Genode::memcpy(dst, tmp.local_addr<void>(), src_len);

					return src_len;

				} else if (mod_node.has_type("inline")) {

					/*
					 * Copy inline content directly to destination buffer
					 */
					mod_node.with_raw_content([&] (char const *ptr, size_t size) {
						Genode::memcpy(dst, ptr, size); });

					return mod_node.content_size();
				}

				warning("XML node ", module_index, " in multiboot node has unexpected type");

				throw Module_loading_failed();
			}
			catch (Xml_node::Nonexistent_sub_node) { }
			catch (Destination_buffer_too_small) {
				error("Boot_module_provider: destination buffer too small"); }
			catch (Rom_connection::Rom_connection_failed) {
				error("Boot_module_provider: Rom_connection_failed"); }
			catch (...) {
				error("Boot_module_provider: Spurious exception");
				throw Module_loading_failed();
			}

			/*
			 * We should get here only if there are XML parsing errors
			 */
			return 0;
		}

		/**
		 * Copy command line to specified buffer
		 *
		 * \return length of command line in bytes, or 0 if module does not exist
		 */
		size_t cmdline(int module_index, char *dst, size_t dst_len) const
		{
			using namespace Genode;

			if (1ul + module_index > _multiboot_node.num_sub_nodes())
				return 0;

			auto const node = _multiboot_node.sub_node(module_index);

			if (!node.has_type("rom") && !node.has_type("inline")) {
				warning("XML node ", module_index, " has unexpected type");
				return 0;
			}

			auto const name = node.attribute_value("name", Name());

			if (!name.length() || name.length() > dst_len)
				return 0;

			/* copy name to command line including zero termination */
			::memcpy(dst, name.string(), name.length());

			if (!node.has_attribute("cmdline"))
				return name.length();

			typedef String<256> Cmdline;
			Cmdline const cmdline = node.attribute_value("cmdline", Cmdline());

			if (!cmdline.length() || cmdline.length() > dst_len - name.length())
				return 0;

			/* replace zero termination with space between name and arguments */
			dst[name.length() - 1] = ' ';

			/* copy arguments to command line including zero termination */
			::memcpy(dst + name.length(), cmdline.string(), cmdline.length());

			return name.length() + cmdline.length();
		}
};

#endif /* _BOOT_MODULE_PROVIDER_H_ */
