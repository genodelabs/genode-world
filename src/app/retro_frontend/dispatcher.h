/*
 * \brief  Libretro signal dispatcher
 * \author Emery Hemingway
 * \date   2017-11-03
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__DISPATCHER_H_
#define _RETRO_FRONTEND__DISPATCHER_H_

/* Genode includes */
#include <timer_session/connection.h>
#include <base/heap.h>
#include <libc/component.h>

/* Local includes */
#include "audio.h"
#include "config.h"
#include "input.h"
#include "framebuffer.h"
#include "memory.h"

namespace Retro_frontend { struct Dispatcher; }


struct Retro_frontend::Dispatcher
{
	/***************
	 ** Game data **
	 ***************/

	struct Game_failed { };

	retro_game_info game_info;

	typedef Genode::String<64> Rom_name;
	typedef Genode::String<128> Game_path;
	Rom_name rom_name;
	Game_path game_path;
	Game_path game_meta;

	/**
	 * Class to contain game ROM and RAM
	 */
	struct Cartridge {

		Genode::Attached_rom_dataspace rom;

		typedef Genode::String<128> Filename;

		Filename const save_filename;
		Filename const rtc_filename;

		Memory_file save_file {
			RETRO_MEMORY_SAVE_RAM, save_filename.string() };
		Memory_file rtc_file {
			RETRO_MEMORY_RTC, rtc_filename.string() };

		Cartridge(Rom_name const &rom_name)
		:
			rom(*genv, rom_name.string()),
			save_filename(  "/", rom_name.string(), ".save"),
			rtc_filename(   "/", rom_name.string(), ".rtc")
		{
			load_memory();
		}

		~Cartridge() { save_memory(); }

		void refresh(Memory_file &memory)
		{
			void *data = retro_get_memory_data(memory.id);
			auto  size = retro_get_memory_size(memory.id);
			if (data && size) {
				memory.data = data;
				memory.size = size;
			}
		}

		void load_memory()
		{
			Genode::log("loading RAM from ", save_filename);
			refresh(save_file);
			refresh(rtc_file);

			save_file.read();
			rtc_file.read();
		}

		void save_memory()
		{
			Genode::log("saving RAM to ", save_filename);
			refresh(save_file);
			save_file.write();

			refresh(rtc_file);
			rtc_file.write();
		}
	};

	Genode::Constructible<Cartridge> cartridge;


	/**
	 * Run core initialization
	 */
	void init_core(Genode::Xml_node const &config)
	{
		if (framebuffer.constructed())
			framebuffer.destruct();

		if (stereo_out.constructed())
			stereo_out.destruct();

		retro_system_info sys_info;

		/****************
		 ** Initialize **
		 ****************/
		{
			retro_get_system_info(&sys_info);

			Genode::log("Name: ", sys_info.library_name,
	            "\nVersion: ", sys_info.library_version,
	            "\nExtensions: ", sys_info.valid_extensions ?
				                  sys_info.valid_extensions : "");

			/* reset keyboard callback */
			keyboard_callback = nullptr;
			retro_set_environment(environment_callback);

			retro_init();
		}


		/***********************
		 ** Install callbacks **
		 ***********************/
		{
			retro_set_video_refresh(video_refresh_callback);

			retro_set_audio_sample(audio_sample_noop);
			retro_set_audio_sample_batch(audio_sample_batch_noop);

			retro_set_input_poll(input_poll_callback);
			retro_set_input_state(input_state_callback);
		}


		/******************************
		 ** Load game data into core **
		 ******************************/

		{
			game_info.path = "";
			game_info.data = NULL;
			game_info.size = 0;
			game_info.meta = "";

			try {
				Genode::Xml_node game_node = config.sub_node("game");

				rom_name = game_node.attribute_value("rom", Rom_name());
				game_path = game_node.attribute_value("path", Game_path());
				game_meta = game_node.attribute_value("meta", Rom_name());
			} catch (Genode::Xml_node::Nonexistent_sub_node) {}

			if (rom_name != "") {
				Genode::log("loading game from ROM '", rom_name, "'");
				cartridge.construct(rom_name);
				game_info.data = cartridge->rom.local_addr<const void>();
				game_info.size = cartridge->rom.size();
			} else if (game_path != "") {
				Genode::log("loading game from path '", game_path, "'");
				game_info.path = game_path.string();
			} else {
				game_info.path = "/";
			}

			game_info.meta = game_meta.string();

			if (sys_info.need_fullpath && (game_info.path == NULL))
				game_info.path = "/";

			if (!(game_info.path || game_info.data)) {
				/* some cores don't need game data */
				Genode::error("no game content loaded");
			}

			if (!(retro_load_game(&game_info))) {
				Genode::error("failed to load game data");
				throw Game_failed();
			}
		}

		/******************
		 ** Get A/V info **
		 ******************/
		{
			retro_system_av_info av_info;
			retro_get_system_av_info(&av_info);

			Genode::log("game geometry: ", av_info.geometry.max_width, "x", av_info.geometry.max_height);

			Genode::log("FPS of video content: ",   av_info.timing.fps, "Hz");
			Genode::log("Sampling rate of audio: ", av_info.timing.sample_rate, "Hz");

			framebuffer.construct(av_info.geometry);

			if (av_info.timing.sample_rate > 0.0) try {
				stereo_out.construct();
				retro_set_audio_sample(audio_sample_callback);
				retro_set_audio_sample_batch(audio_sample_batch_callback);

				double ratio = (double)Audio_out::SAMPLE_RATE / av_info.timing.sample_rate;

				audio_shift_factor = SHIFT_ONE / ratio;
				audio_input_period = Audio_out::PERIOD * (1.0f / ratio);
			} catch (...) {
				Genode::error("failed to initialize Audio_out sessions");
			}

			quarter_fps = av_info.timing.fps / 4;
			fb_sync_sample_count = 0;

			framebuffer->session.sync_sigh(fb_sync_sampler);
		}
	}


	/***********************
	 ** Deinitialize core **
	 ***********************/

	void deinit()
	{
		/* XXX: what happens to SAVE_RAM as the game is unloaded? */
		if (cartridge.constructed())
			cartridge->save_memory();
		retro_unload_game();
		if (cartridge.constructed())
			cartridge.destruct();
		retro_deinit();
	}

	Genode::Heap heap { genv->ram(), genv->rm() };


	/************
	 ** Config **
	 ************/

	Genode::Signal_handler<Dispatcher> config_handler {
		genv->ep(), *this, &Dispatcher::handle_config };

	void handle_config()
	{
		using namespace Genode;

		++config_version;
		auto const config = config_rom->xml();


		/***********************
		 ** Load and map core **
		 ***********************/
		{
			auto const core_name = config.attribute_value("core", Core::Name());

			if (core.constructed() && core->name != core_name) {
				deinit();
				core.destruct();
			}

			if (!core.constructed())
				core.construct(heap, core_name);

			Libc::with_libc([&] () { init_core(config); });
		}

		/* initialize controller sessions and mappings */
		initialize_controllers(heap, config);
	}


	/*************************************************
	 ** Signal handler to advance core by one frame **
	 *************************************************/

	Timer::Connection timer { *genv };

	/* switch to application context and advance the core */
	void run() { Libc::with_libc([&] () { retro_run(); }); }

	Genode::Signal_handler<Dispatcher> core_runner
		{ genv->ep(), *this, &Dispatcher::run };


	/***********************************
	 ** Frame counting signal handler **
	 ***********************************/

	int quarter_fps = 0;
	unsigned long sample_start;
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
			if (stereo_out.constructed()) {
				stereo_out->start_stream();
			}
		}
	}

	Genode::Signal_handler<Dispatcher> fb_sync_sampler
		{ genv->ep(), *this, &Dispatcher::fb_sync_sample };


	/***********
	 ** Pause **
	 ***********/

	bool paused = false;

	void poll_controllers()
	{
		/* this callback pauses and unpauses the frontend */
		Libc::with_libc([] () { input_poll_callback(); });
	}

	Genode::Signal_handler<Dispatcher> unpause_handler
		{ genv->ep(), *this, &Dispatcher::poll_controllers };

	void pause()
	{
		paused = true;

		/* save the game RAM */
		if (cartridge.constructed())
			cartridge->save_memory();

		/* let the audio buffer drain and stop */
		if (stereo_out.constructed())
			stereo_out->stop_stream();

		/* disable the framebuffer signals */
		framebuffer->session.sync_sigh(
			Genode::Signal_context_capability());

		/* disable the timer signals */
		timer.sigh(Genode::Signal_context_capability());

		/* install a handler to unpause from the controllers */
		input_sigh(unpause_handler);
	}

	void unpause()
	{
		paused = false;

		if (cartridge.constructed())
			cartridge->load_memory();

		/* disable the unpause signaling */
		input_sigh(Genode::Signal_context_capability());

		/* resync and run */
		framebuffer->session.sync_sigh(fb_sync_sampler);
	}

	void toggle_pause()
	{
		if (paused)
			unpause();
		else
			pause();
	}

	Dispatcher()
	{
		config_rom->sigh(config_handler);
	}
};

#endif
