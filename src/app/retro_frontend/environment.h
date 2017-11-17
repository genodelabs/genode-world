/*
 * \brief  Interface to Genode services
 * \author Emery Hemingway
 * \date   2016-07-14
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__ENVIRONMENT_H_
#define _RETRO_FRONTEND__ENVIRONMENT_H_

/* local includes */
#include "framebuffer.h"
#include "input.h"
#include "log.h"
#include "core.h"

/* Genode includes */
#include <os/reporter.h>
#include <base/sleep.h>


namespace Retro_frontend {
	static unsigned variables_version = 0;
}


bool environment_callback(unsigned cmd, void *data)
{
	using namespace Retro_frontend;

	switch (cmd) {

	case RETRO_ENVIRONMENT_GET_OVERSCAN:
		Genode::warning("instructing core not to overscan");
		*((bool *)data) = false;
		return true;

	case RETRO_ENVIRONMENT_GET_CAN_DUPE:
	{
		bool *b = (bool*)data;
		*b = true;
		return true;
	}

	case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
	{
		const unsigned *level = (const unsigned*)data;
		Genode::warning("frontend reports a suggested performance level of \"", *level, "\"");
		return true;
	}

	case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
	case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
	case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
	{
		char **path = (char **)data;
		*path = (char *)"/";
		return true;
	}

	case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
	{
		const retro_pixel_format *format = (retro_pixel_format *)data;
		if (*format == RETRO_PIXEL_FORMAT_RGB565)
			return true;
		else {
			char const *s = "";
			switch (*format) {
			case RETRO_PIXEL_FORMAT_0RGB1555: s = "0RGB1555"; break;
			case RETRO_PIXEL_FORMAT_XRGB8888: s = "XRGB8888"; break;
			case RETRO_PIXEL_FORMAT_RGB565:   s = "RGB565";   break;
			case RETRO_PIXEL_FORMAT_UNKNOWN:  break;
			}
			Genode::error("core uses unsupported pixel format ", s);
			return false;
		}
	}

	case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
	{
		static Genode::Reporter input_reporter { *genv, "inputs", "inputs", 8192 };

		try { input_reporter.enabled(true); }
		catch (...) {
			Genode::error("input descriptors not reported");
			return false;
		}

		/* TODO: translate device, index, and id to a string descriptor */

		Genode::Reporter::Xml_generator gen(input_reporter, [&] () {
			for (const struct retro_input_descriptor *desc = (retro_input_descriptor*)data;
			     (desc && (desc->description != NULL)); ++desc)
				{
					char const *device_str = "UNKNOWN";
					switch (desc->device) {
					case RETRO_DEVICE_JOYPAD:
						device_str = "JOYPAD"; break;
					case RETRO_DEVICE_MOUSE:
						device_str = "MOUSE"; break;
					case RETRO_DEVICE_KEYBOARD:
						device_str = "KEYBOARD"; break;
					case RETRO_DEVICE_LIGHTGUN:
						device_str = "LIGHTGUN"; break;
					case RETRO_DEVICE_ANALOG:
						device_str = "ANALOG"; break;
					case RETRO_DEVICE_POINTER:
						device_str = "POINTER"; break;
					default: break;
					}

					gen.node("descriptor", [&] () {
						gen.attribute("port",        desc->port);
						gen.attribute("device",      device_str);
						gen.attribute("index",       desc->index);
						gen.attribute("id", lookup_input_text(desc->device, desc->id));
						gen.attribute("description", desc->description);
					});
				}
		});

		return true;
	}

	case RETRO_ENVIRONMENT_SHUTDOWN:
		shutdown();
		/* can't return here, this is a callback */
		Genode::sleep_forever();

	case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
	{
		auto *out = (retro_keyboard_callback const *)data;
		keyboard_callback = out->callback;
		return true;
	}

	case RETRO_ENVIRONMENT_GET_VARIABLE:
	{
		retro_variable *out = (retro_variable*)data;
		Genode::warning("RETRO_ENVIRONMENT_GET_VARIABLE ", out->key);

		out->value = NULL;

		static char var_buf[256];

		typedef Genode::String<32> String;
		auto const f = [&] (Genode::Xml_node const &in) {
			if (in.attribute_value("key", String()) == out->key &&
			    in.has_attribute("value"))
			{
				in.attribute("value").value(var_buf, sizeof(var_buf));
				out->value = var_buf;
			}
		};

		config_variables().for_each_sub_node("variable", f);

		variables_version = config_version;
		return out->value != NULL;
	}

	case RETRO_ENVIRONMENT_SET_VARIABLES:
	{
		/**********************************
		 ** Report variables set by core **
		 **********************************/

		static Genode::Reporter variable_reporter { *genv, "variables", "variables", 8192 };
		try { variable_reporter.enabled(true); }
		catch (...) {
			Genode::error("core variables not reported");
			return false;
		}

		Genode::Reporter::Xml_generator gen(variable_reporter, [&] () {
			for (const struct retro_variable *var = (retro_variable*)data;
			     (var && (var->key != NULL) && (var->value != NULL)); ++var)
				{
					gen.node("variable", [&] () {
						gen.attribute("key", var->key);
						gen.attribute("value", var->value);
					});
				}
		});

		return true;
	}

	case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
		*((bool *)data) = (variables_version != config_version);
		return true;

	case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
		return false;

	case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
		return false;

	case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
	{
		retro_log_callback *cb = (retro_log_callback*)data;
		cb->log = log_printf_callback;
		return true;
	}

	case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
	{
		static Genode::Reporter  subsystem_reporter { *genv, "subsystems", "subsystems", 8192 };
		try { subsystem_reporter.enabled(true); }
		catch (...) {
			Genode::error("core subsystems not reported");
			return false;
		}

		Genode::Reporter::Xml_generator gen(subsystem_reporter, [&] () {
			for (const retro_subsystem_info *info = (retro_subsystem_info*)data;
			     (info && (info->desc)); ++info)
				{
					gen.node("subsystem", [&] () {
						gen.attribute("desc", info->desc);
						gen.attribute("num_roms", info->num_roms);
						gen.attribute("id", info->id);
						for (unsigned i = 0; i < info->num_roms; ++i) {
							const retro_subsystem_rom_info *rom_info =
								&info->roms[i];
							gen.node("rom", [&] () {
								gen.attribute("desc", rom_info->desc);
								gen.attribute("valid_extensions", rom_info->valid_extensions);
								gen.attribute("need_fullpath", rom_info->need_fullpath);
								gen.attribute("block_extract", rom_info->block_extract);
								gen.attribute("required", rom_info->required);
								for (unsigned j = 0; j < rom_info->num_memory; ++j) {
									const retro_subsystem_memory_info *memory =
										&rom_info->memory[i];
									gen.node("memory", [&] () {
										gen.attribute("extension", memory->extension);
										gen.attribute("type", memory->type);
									});
								}
							});
						}
					});
				}
		});
		return true;
	}

	case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
	{
		controller_info = (const retro_controller_info*)data;

		static Genode::Reporter controller_reporter { *genv, "controllers" };
		try { controller_reporter.enabled(true); }
		catch (...) {
			Genode::error("controller info not reported");
			return true;
		}

		Genode::Reporter::Xml_generator gen(controller_reporter, [&] () {
			unsigned index = 0;
			for (const retro_controller_info *info = (retro_controller_info*)data;
			     (info && (info->types)); ++info)
			{
				gen.node("controller", [&] () {
					gen.attribute("port", index);
					for (unsigned i = 0; i < info->num_types; ++i) {
						const retro_controller_description *type =
							&info->types[i];
						gen.node("type", [&] () {
							gen.attribute("desc", type->desc);
							gen.attribute("id", type->id);
						});
					}
				});
			}			
		});

		return true;
	}

	case RETRO_ENVIRONMENT_GET_USERNAME:
	{
		*(char const **)data = NULL;
		return true;
	}

	case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
	{
		retro_framebuffer *fb = (retro_framebuffer*)data;

		if (!framebuffer.constructed())
			return false;

		::Framebuffer::Mode mode = framebuffer->mode;

		fb->width  = (unsigned)mode.width();
		fb->height = (unsigned)mode.height();
		fb->data = framebuffer->ds.local_addr<void>();
		fb->pitch = mode.width() * 2;
		fb->format = RETRO_PIXEL_FORMAT_RGB565;
		fb->memory_flags = RETRO_MEMORY_TYPE_CACHED;
		return true;
	}
	}

	Genode::warning(__func__, "(",cmd,") is unhandled");
	return false;
}


#endif
