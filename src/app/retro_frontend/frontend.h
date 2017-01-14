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

#ifndef _RETRO_FRONTEND__FRONTEND_H_
#define _RETRO_FRONTEND__FRONTEND_H_

/* Genode includes */
#include <audio_out_session/connection.h>
#include <input_session/connection.h>
#include <input/event_queue.h>
#include <framebuffer_session/connection.h>
#include <timer_session/connection.h>
#include <os/reporter.h>
#include <base/attached_rom_dataspace.h>
#include <base/shared_object.h>
#include <base/heap.h>
#include <base/component.h>
#include <base/log.h>
#include <log_session/log_session.h>

/* Libc includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Local includes */
#include "audio.h"
#include "input.h"

namespace Retro_frontend {
	#include <libretro.h>
	struct Frontend;
	struct Framebuffer;
}

/* Global frontend instance */
static Retro_frontend::Frontend *global_frontend = nullptr;

struct Retro_frontend::Framebuffer
{
	::Framebuffer::Connection session;

	::Framebuffer::Mode mode;

	Genode::Attached_dataspace ds;

	Framebuffer(Genode::Env &env, ::Framebuffer::Mode mode)
	: session(env, mode), ds(env.rm(), session.dataspace())
	{ update_mode(); }

	void update_mode() { mode = session.mode(); }
};


static Genode::Constructible<Retro_frontend::Framebuffer> framebuffer;


/**
 * Object to encapsulate Genode services
 */
struct Retro_frontend::Frontend
{
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

	Libc::Env  &env;

	Genode::Attached_rom_dataspace config_rom { env, "config" };

	Genode::Constructible<Genode::Attached_rom_dataspace> var_rom;

	Genode::Heap heap { env.ram(), env.rm() };

	typedef Genode::String<128> Name;

	Name const name { config_rom.xml().attribute_value("core", Name()) };

	struct Dynamic_core : Genode::Shared_object
	{

		Dynamic_core(Genode::Env &env, Genode::Allocator &alloc, Name const &name)
		: Genode::Shared_object(env, alloc, name.string(), BIND_NOW, DONT_KEEP)
		{
			unsigned api_version = lookup<Retro_api_version>
				("retro_api_version")();
			if (api_version != RETRO_API_VERSION) {
				Genode::error("core ", name.string(),
				              " uses unsupported API version ", api_version);
				throw Genode::Shared_object::Invalid_rom_module();
			}
		}

	} shared_object { env, heap, name };

	Retro_init   retro_init =
		shared_object.lookup<Retro_init>("retro_init");
	Retro_deinit retro_deinit =
		shared_object.lookup<Retro_deinit>("retro_deinit");

	Retro_load_game retro_load_game =
		shared_object.lookup<Retro_load_game>("retro_load_game");
	Retro_unload_game retro_unload_game =
		shared_object.lookup<Retro_unload_game>("retro_unload_game");

	Retro_get_memory_data retro_get_memory_data =
		shared_object.lookup<Retro_get_memory_data>("retro_get_memory_data");
	Retro_get_memory_size retro_get_memory_size =
		shared_object.lookup<Retro_get_memory_size>("retro_get_memory_size");


	/***************
	 ** Game data **
	 ***************/

	typedef Genode::String<64> Rom_name;
	typedef Genode::String<128> Game_path;
	Rom_name rom_name;
	Game_path game_path;
	Rom_name rom_meta;

	Genode::Constructible<Genode::Attached_rom_dataspace> game_rom;

	retro_game_info game_info;

	/******************************
	 ** Persistent memory states **
	 ******************************/

	struct Memory_file
	{
		void   *data = nullptr;
		size_t  size = 0;
		unsigned const  id;
		char     const *path;
		int fd = -1;

		Genode::size_t file_size()
		{
			struct stat s;
			s.st_size = 0;
			stat(path, &s);
			return s.st_size;
		}

		bool open_file_for_data()
		{
			if (!(data && size))
				return false;

			if (fd == -1)
				fd = ::open(path, O_RDWR|O_CREAT);
			return fd != -1;
		}

		Memory_file(unsigned id, char const *filename)
		: id(id), path(filename)
		{ }

		~Memory_file() { if (fd != -1) close(fd); }

		void read()
		{
			if (open_file_for_data()) {
				lseek(fd, 0, SEEK_SET);
				size_t remain = Genode::min(size, file_size());
				size_t offset = 0;
				do {
					ssize_t n = ::read(fd, ((char*)data)+offset, remain);
					if (n == -1) {
						Genode::error("failed to read from ", path);
						break;
					}
					remain -= n;
					offset += n;
				} while (remain);
			}
		}

		void write()
		{
			if (open_file_for_data()) {
				lseek(fd, 0, SEEK_SET);
				ftruncate(fd, size);
				size_t remain = size;
				size_t offset = 0;
				do {
					ssize_t n = ::write(fd, ((char const *)data)+offset, remain);
					if (n == -1) {
						Genode::error("failed to write to ", path);
						break;
					}
					remain -= n;
					offset += n;
				} while (remain);
			}
		}
	};

	Memory_file   save_file { RETRO_MEMORY_SAVE_RAM,   "save" };
	Memory_file    rtc_file { RETRO_MEMORY_RTC,        "rtc" };
	Memory_file system_file { RETRO_MEMORY_SYSTEM_RAM, "system" };

	void refresh(Memory_file &memory)
	{
		memory.data = retro_get_memory_data(memory.id);
		memory.size = retro_get_memory_size(memory.id);
	}

	void load_memory()
	{
		refresh(save_file);
		refresh(rtc_file);
		refresh(system_file);

		save_file.read();
		rtc_file.read();
		system_file.read();
	}

	void save_memory()
	{
		refresh(save_file);
		save_file.write();

		refresh(rtc_file);
		rtc_file.write();

		refresh(system_file);
		system_file.write();	
	}

	/******************
	 ** Fronted exit **
	 ******************/

	void exit()
	{
		save_memory();
		retro_unload_game();
		retro_deinit();
	}

	/**
	 * Exit when the framebuffer is resized to zero
	 */
	void handle_mode()
	{
		framebuffer->update_mode();
		if ((framebuffer->mode.width() == 0) &&
		    (framebuffer->mode.height() == 0))
		{
			Genode::error("zero framebuffer mode received, exiting");
			exit();
			env.parent().exit(0);
		}
	}

	Genode::Signal_handler<Frontend> mode_handler
		{ env.ep(), *this, &Frontend::handle_mode };

	/*************************************************
	 ** Signal handler to advance core by one frame **
	 *************************************************/

	Retro_run retro_run =
		shared_object.lookup<Retro_run>("retro_run");

	void run() { retro_run(); }

	Genode::Signal_handler<Frontend> core_runner
		{ env.ep(), *this, &Frontend::run };

	Timer::Connection timer { env };

	Genode::Reporter      input_reporter { env, "input" };
	Genode::Reporter   variable_reporter { env, "variables" };
	Genode::Reporter  subsystem_reporter { env, "subsystems" };
	Genode::Reporter controller_reporter { env, "controllers" };

	Controller controller { env };

	int quarter_fps = 0;
	unsigned long sample_start;


	/***********************************
	 ** Frame counting signal handler **
	 ***********************************/

	int fb_sync_sample_count = 0;

	/**
	 * Sample the framebuffer sync for a quarter-second
	 * to determine if it matches the FPS
	 */
	void fb_sync_sample()
	{
		++fb_sync_sample_count;

		if (fb_sync_sample_count == 1) {
			sample_start = timer.elapsed_ms();
		} else if (fb_sync_sample_count == quarter_fps) {
			fb_sync_sample_count = 0;

			float sync_ms = (timer.elapsed_ms() - sample_start)/quarter_fps;
			float want_ms = 250.0 / quarter_fps;
			float diff = want_ms - sync_ms;

			/* allow a generous two millisecond difference in FPS */
			if (diff > 2.0 || diff < -2.0) {
				framebuffer->session.sync_sigh(Genode::Signal_context_capability());
				Genode::warning("framebuffer sync unsuitable, "
				                "using alternative timing source");
				timer.sigh(core_runner);
				timer.trigger_periodic(250000 / quarter_fps);
			} else {
				Genode::log("using framebuffer sync as timing source");
				framebuffer->session.sync_sigh(core_runner);
			}

			/* start the audio streams */
			if (stereo_out.constructed())
				stereo_out->start_stream();
		}
	}

	Genode::Signal_handler<Frontend> fb_sync_sampler
		{ env.ep(), *this, &Frontend::fb_sync_sample };


	/***************
	 ** Unpausing **
	 ***************/

	void handle_input()
	{
		if (controller.unpaused()) {
			controller.input.sigh(Genode::Signal_context_capability());
			fb_sync_sample_count = 0;
			framebuffer->session.sync_sigh(fb_sync_sampler);
		}
	}

	Genode::Signal_handler<Frontend> resume_handler
		{ env.ep(), *this, &Frontend::handle_input };

	Frontend(Libc::Env &env);

	~Frontend()
	{
		exit();
		global_frontend = nullptr;
	}

	void set_av_info(retro_system_av_info &av_info)
	{
		using namespace Retro_frontend;

		framebuffer.construct(
			env, ::Framebuffer::Mode(av_info.geometry.base_width,
			                         av_info.geometry.base_height,
			                         ::Framebuffer::Mode::RGB565));

		framebuffer->session.mode_sigh(mode_handler);

		if (stereo_out.constructed()) {
			double ratio = (double)Audio_out::SAMPLE_RATE / av_info.timing.sample_rate;

			audio_shift_factor = SHIFT_ONE / ratio;
			audio_input_period = Audio_out::PERIOD * (1.0f / ratio);
		}

		quarter_fps = av_info.timing.fps / 4;
		fb_sync_sample_count = 0;

		framebuffer->session.sync_sigh(fb_sync_sampler);
	}

	void flush_input()
	{
		controller.flush();
		if (controller.paused) {
			/* unset signal handler for rendering */
			framebuffer->session.sync_sigh(Genode::Signal_context_capability());
			timer.sigh(Genode::Signal_context_capability());

			/* stop playpack */
			stereo_out->stop_stream();

			/* set signal handler for unpausing */
			controller.input.sigh(resume_handler);

			/* a good time to flush memory to the FS */
			save_memory();
		}
	}

	Genode::Xml_node variables()
	{
		try {
			if (!var_rom.constructed())
				var_rom.construct(env, "variables");
			return var_rom->xml();
		} catch (...) {
			return Genode::Xml_node("<variables/>");
		}
	}

};

#endif
