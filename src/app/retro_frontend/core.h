/*
 * \brief  Libretro frontend
 * \author Emery Hemingway
 * \date   2017-11-04
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__CORE_H_
#define _RETRO_FRONTEND__CORE_H_

#include <base/debug.h>

/* Genode includes */
#include <base/shared_object.h>
#include <base/component.h>
#include <base/log.h>

#include <libretro.h>

namespace Retro_frontend {

	static Genode::Env *genv;

	typedef void (*Retro_set_environment)(retro_environment_t);
	typedef void (*Retro_set_video_refresh)(retro_video_refresh_t);
	typedef void (*Retro_set_audio_sample)(retro_audio_sample_t);
	typedef void (*Retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
	typedef void (*Retro_set_input_poll)(retro_input_poll_t);
	typedef void (*Retro_set_input_state)(retro_input_state_t);
	typedef void (*Retro_init)(void);
	typedef void (*Retro_deinit)(void);
	typedef unsigned (*Retro_api_version)(void);
	typedef void (*Retro_get_system_info)(struct retro_system_info *info);
	typedef void (*Retro_get_system_av_info)(struct retro_system_av_info *info);
	typedef void (*Retro_set_controller_port_device)(unsigned port, unsigned device);
	typedef void (*Retro_reset)(void);
	typedef void (*Retro_run)(void);
	typedef size_t (*Retro_serialize_size)(void);
	typedef bool (*Retro_serialize)(void *data, size_t size);
	typedef bool (*Retro_unserialize)(const void *data, size_t size);
	typedef void (*Retro_cheat_reset)(void);
	typedef void (*Retro_cheat_set)(unsigned index, bool enabled, const char *code);
	typedef bool (*Retro_load_game)(const struct retro_game_info *game);
	typedef bool (*Retro_load_game_special)(
		unsigned game_type,
		const struct retro_game_info *info, size_t num_info
	);
	typedef void (*Retro_unload_game)(void);
	typedef unsigned (*Retro_get_region)(void);
	typedef void *(*Retro_get_memory_data)(unsigned id);
	typedef size_t (*Retro_get_memory_size)(unsigned id);

	Retro_init retro_init;
	Retro_deinit retro_deinit;
	Retro_load_game retro_load_game;
	Retro_unload_game retro_unload_game;

	Retro_get_system_av_info retro_get_system_av_info;

	Retro_get_memory_data retro_get_memory_data;
	Retro_get_memory_size retro_get_memory_size;

	Retro_set_controller_port_device retro_set_controller_port_device;

	Retro_get_system_info retro_get_system_info;

	Retro_set_environment retro_set_environment;

	Retro_set_video_refresh retro_set_video_refresh;

	Retro_set_audio_sample retro_set_audio_sample;
	Retro_set_audio_sample_batch retro_set_audio_sample_batch;

	Retro_set_input_poll retro_set_input_poll;
	Retro_set_input_state retro_set_input_state;

	Retro_run retro_run;

	struct Core;	
};


/*************************
 ** callback prototypes **
 *************************/

bool environment_callback(unsigned cmd, void *data);

void input_poll_callback();

int16_t input_state_callback(unsigned port,  unsigned device,
                             unsigned index, unsigned id);

void video_refresh_callback(const void *data,
                            unsigned src_width, unsigned src_height,
                            size_t src_pitch);

void audio_sample_noop(int16_t left, int16_t right);
void audio_sample_callback(int16_t left, int16_t right);
size_t audio_sample_batch_noop(const int16_t *data, size_t frames);
size_t audio_sample_batch_callback(const int16_t *data, size_t frames);

void log_printf_callback(retro_log_level level, const char *fmt, ...);


struct Retro_frontend::Core
{
	typedef Genode::String<128> Name;

	Genode::Shared_object so;

	Name const name;

	Core(Genode::Allocator &alloc, Name const &name)
	: so(*genv, alloc, name.string(),
         Genode::Shared_object::BIND_LAZY,
	     Genode::Shared_object::DONT_KEEP),
	  name(name)
	{
		unsigned api_version = so.lookup<Retro_api_version>("retro_api_version")();
		if (api_version != RETRO_API_VERSION) {
			Genode::error("core ", name,
			              " uses unsupported API version ", api_version);
			throw Genode::Shared_object::Invalid_rom_module();
		}

		retro_init = so.lookup<Retro_init>("retro_init");
		retro_deinit = so.lookup<Retro_deinit>("retro_deinit");

		retro_load_game = so.lookup<Retro_load_game>("retro_load_game");
		retro_unload_game = so.lookup<Retro_unload_game>("retro_unload_game");

		retro_get_system_av_info = so.lookup<Retro_get_system_av_info>("retro_get_system_av_info");

		retro_get_memory_data = so.lookup<Retro_get_memory_data>("retro_get_memory_data");
		retro_get_memory_size = so.lookup<Retro_get_memory_size>("retro_get_memory_size");

		retro_set_controller_port_device = so.lookup<Retro_set_controller_port_device>("retro_set_controller_port_device");
		retro_run = so.lookup<Retro_run>("retro_run");

		retro_get_system_info = so.lookup<Retro_get_system_info>
			("retro_get_system_info");

		retro_set_environment = so.lookup<Retro_set_environment>
			("retro_set_environment");

		retro_init = so.lookup<Retro_init>("retro_init");

		retro_set_video_refresh = so.lookup<Retro_set_video_refresh>("retro_set_video_refresh");

		retro_set_audio_sample = so.lookup<Retro_set_audio_sample>("retro_set_audio_sample");
		retro_set_audio_sample_batch = so.lookup<Retro_set_audio_sample_batch>("retro_set_audio_sample_batch");

		retro_set_input_poll = so.lookup<Retro_set_input_poll>("retro_set_input_poll");

		retro_set_input_state = so.lookup<Retro_set_input_state>("retro_set_input_state");
	}
};

namespace Retro_frontend {

	static Genode::Constructible<Core> core;

	void toggle_pause();
	void shutdown();
}

#endif
