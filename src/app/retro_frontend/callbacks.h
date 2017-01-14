/*
 * \brief  Interface to Genode services
 * \author Emery Hemingway
 * \date   2016-07-14
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__CALLBACKS_H_
#define _RETRO_FRONTEND__CALLBACKS_H_

#include "frontend.h"

/* vsnprintf */
#include <stdio.h>


static
void log_printf_callback(Retro_frontend::retro_log_level level, const char *fmt, ...)
{
	using namespace Retro_frontend;

	char buf[Genode::Log_session::MAX_STRING_LEN];

	va_list vp;
	va_start(vp, fmt);
	::vsnprintf(buf, sizeof(buf), fmt, vp);
	va_end(vp);

	char const *msg = buf;

	switch (level) {
	case RETRO_LOG_DEBUG: Genode::log("Debug: ", msg); return;
	case RETRO_LOG_INFO:  Genode::log(msg);            return;
	case RETRO_LOG_WARN:  Genode::warning(msg);        return;
	case RETRO_LOG_ERROR: Genode::error(msg);          return;
	case RETRO_LOG_DUMMY: Genode::log("Dummy: ", msg); return;
	}
}


static
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
		try {
			global_frontend->input_reporter.enabled(true);
		} catch (...) {
			Genode::error("input descriptors not reported");
			return false;
		}

		/* TODO: translate device, index, and id to a string descriptor */

		Genode::Reporter::Xml_generator gen(global_frontend->input_reporter, [&] () {
			for (const struct retro_input_descriptor *desc = (retro_input_descriptor*)data;
			     (desc && (desc->description != NULL)); ++desc)
				{
					gen.node("descriptor", [&] () {
						gen.attribute("port",        desc->port);
						gen.attribute("device",      desc->device);
						gen.attribute("index",       desc->index);
						gen.attribute("id",          desc->id);
						gen.attribute("description", desc->description);
					});
				}
		});

		return true;
	}

	case RETRO_ENVIRONMENT_SHUTDOWN:
		global_frontend->env.parent().exit(0);
		return true;

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

		global_frontend->variables().for_each_sub_node("variable", f);

		return out->value != NULL;
	}

	case RETRO_ENVIRONMENT_SET_VARIABLES:
	{
		/**********************************
		 ** Report variables set by core **
		 **********************************/

		try {
			global_frontend->variable_reporter.enabled(true);
		} catch (...) {
			Genode::error("core variables not reported");
			return false;
		}

		Genode::Reporter::Xml_generator gen(global_frontend->variable_reporter, [&] () {
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
		return false;

	//case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
	//	global_frontend->core.supports_null_load = data;
	//	return true;

	case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
		Genode::warning("Genode rumble interface not implemented");
		return false;

	case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
	{
		retro_log_callback *cb = (retro_log_callback*)data;
		cb->log = log_printf_callback;
		return true;
	}

	case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
	{
		try {
			global_frontend->subsystem_reporter.enabled(true);
		} catch (...) {
			Genode::error("core subsystems not reported");
			return false;
		}

		Genode::Reporter::Xml_generator gen(global_frontend->subsystem_reporter, [&] () {
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
		try {
			global_frontend->controller_reporter.enabled(true);
		} catch (...) {
			Genode::error("core controllers not reported");
			return false;
		}

		Genode::Reporter::Xml_generator gen(global_frontend->controller_reporter, [&] () {
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


static
void video_refresh_callback(const void *data,
                            unsigned src_width, unsigned src_height,
                            size_t src_pitch)
{
	if (!framebuffer.constructed())
		return; /* tyrquake does this during 'retro_load_game' */

	using namespace Retro_frontend;
	using namespace Genode;

	if (data == NULL) /* frame duping? */ {
		return;
	}

	uint8_t const *src = (uint8_t const*)data;
	uint8_t *dst = framebuffer->ds.local_addr<uint8_t>();

	unsigned const dst_width  = framebuffer->mode.width();
	unsigned const dst_height = framebuffer->mode.height();

	unsigned const width  = min(src_width,  dst_width);
	unsigned const height = min(src_height, dst_height);

	if (dst != src) {
		::size_t dst_pitch = dst_width<<1; /* x2 */

		for (unsigned y = 0; y < height; ++y)
			memcpy(&dst[y*dst_pitch], &src[y*src_pitch], dst_pitch);
	}

	framebuffer->session.refresh(0, 0, width, height);
}


static
void input_poll_callback() { global_frontend->flush_input(); }


static
int16_t input_state_callback(unsigned port,  unsigned device,
                             unsigned index, unsigned id)
{
	return global_frontend->controller.event(device, index, id);
}

#endif
