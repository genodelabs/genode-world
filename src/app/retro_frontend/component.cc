/*
 * \brief  Libretro frontend
 * \author Emery Hemingway
 * \date   2016-07-03
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* libc includes */
#include <libc/component.h>

#include "frontend.h"
#include "callbacks.h"

Retro_frontend::Frontend::Frontend(Libc::Env &env) : env(env)
{
	/* set the global frontend pointer for callbacks */
	global_frontend = this;

	/****************
	 ** Initialize **
	 ****************/

	retro_system_info sys_info;

	shared_object.lookup<Retro_get_system_info>
		("retro_get_system_info")(&sys_info);

	Genode::log("Name: ", sys_info.library_name,
	            "\nVersion: ", sys_info.library_version,
	            "\nExtensions: ", sys_info.valid_extensions ?
				                  sys_info.valid_extensions : "");

	shared_object.lookup<Retro_set_environment>
		("retro_set_environment")(environment_callback);

	shared_object.lookup<Retro_init>("retro_init")();


	/*******************
	 ** Set callbacks **
	 *******************/

	shared_object.lookup<Retro_set_video_refresh>
		("retro_set_video_refresh")(video_refresh_callback);

	try {
		stereo_out.construct(env);

		shared_object.lookup<Retro_set_audio_sample>
			("retro_set_audio_sample")(audio_sample_callback);

		shared_object.lookup<Retro_set_audio_sample_batch>
			("retro_set_audio_sample_batch")(audio_sample_batch_callback);
	} catch (...) {

		shared_object.lookup<Retro_set_audio_sample>
			("retro_set_audio_sample")(audio_sample_noop);

		shared_object.lookup<Retro_set_audio_sample_batch>
			("retro_set_audio_sample_batch")(audio_sample_batch_noop);
	}

	shared_object.lookup<Retro_set_input_poll>
		("retro_set_input_poll")(input_poll_callback);

	shared_object.lookup<Retro_set_input_state>
		("retro_set_input_state")(input_state_callback);


	/********************
	 ** Load game data **
	 ********************/

	game_info.path = NULL;
	game_info.data = NULL;
	game_info.size = 0;

	game_info.meta = "";

	try {
		Genode::Xml_node game_node = config_rom.xml().sub_node("game");

		rom_name = game_node.attribute_value("rom", Rom_name());
		game_path = game_node.attribute_value("path", Game_path());
		rom_meta = game_node.attribute_value("name", Rom_name());

		if (rom_name != "") {
			game_rom.construct(env, rom_name.string());
			game_info.data = game_rom->local_addr<const void>();
			game_info.size = game_rom->size();
			game_info.path = "";
			Genode::log("loading game from ROM '", rom_name, "'");
		} else if (game_path != "") {
			game_info.path = game_path.string();
			Genode::log("loading game from path '", game_path, "'");
		}
		game_info.meta =
			(rom_meta != "") ? rom_meta.string() :
			(rom_name != "") ? rom_name.string() :
			game_path.string();
	}
	catch (...) { }

	if (sys_info.need_fullpath && (game_info.path == NULL))
		game_info.path = "/";

	if (!(game_info.path || game_info.data)) {
		/* some cores don't need game data */
		Genode::error("no game content loaded");
	}

	if (!(retro_load_game(&game_info))) {
		Genode::error("failed to load game data");
		retro_deinit();
		throw ~0;
	}

	load_memory();


	/******************
	 ** Get A/V info **
	 ******************/

	retro_system_av_info av_info;
	shared_object.lookup<Retro_get_system_av_info>
		("retro_get_system_av_info")(&av_info);

	Genode::log("FPS of video content: ",   (int)av_info.timing.fps);
	Genode::log("Sampling rate of audio: ", (int)av_info.timing.sample_rate);

	set_av_info(av_info);

	framebuffer->session.mode_sigh(mode_handler);

	/* flush out the input events that may have piled up */
	flush_input();
}


/* each core will drive the stack differently, so be generous */
Genode::size_t Component::stack_size() { return 64*1024*sizeof(Genode::addr_t); }

void Libc::Component::construct(Libc::Env &env)
{
	using namespace Genode;
	using namespace Retro_frontend;

	init_keyboard_map();

	try { static Frontend inst(env); }

	catch (Shared_object::Invalid_rom_module) {
		error("failed to load core"); }

	catch (Shared_object::Invalid_symbol) {
		error("failed to load required symbols from core"); }

	catch (...) {
		error("failed to init core");
		env.parent().exit(-1);
	}
}
