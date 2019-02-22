/*
 * \brief  FLIF viewer for Nitpicker
 * \author Emery Hemingway
 * \date   2017-12-02
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* FLIF includes */
#include <flif_dec.h>

/* Genode includes */
#include <base/component.h>
#include <base/heap.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <nitpicker_session/connection.h>
#include <util/misc_math.h>
#include <nitpicker_gfx/texture_painter.h>
#include <timer_session/connection.h>
#include <base/attached_dataspace.h>
#include <util/reconstructible.h>
#include <os/texture_rgb565.h>

/* gems includes */
#include <gems/texture_utils.h>
#include <gems/chunky_texture.h>

/* libc includes */
#include <libc/component.h>
extern "C" {
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
}


namespace Flif_view {
	using namespace Genode;
	struct Main;

	using Framebuffer::Mode;
	typedef Nitpicker::Session::Command Command;
}


struct Flif_view::Main
{
	Main(Main const &);
	Main &operator = (Main const &);

	Libc::Env &env;

	Attached_ram_dataspace back_ds { env.pd(), env.rm(), 0 };

	Nitpicker::Connection nitpicker { env };

	Signal_handler<Main> config_handler {
		env.ep(), *this, &Main::handle_config_signal };

	Signal_handler<Main> app_handler {
		env.ep(), *this, &Main::handle_app_signal };

	Io_signal_handler<Main> input_handler {
		env.ep(), *this, &Main::handle_input_signal };

	Io_signal_handler<Main> sync_handler {
		env.ep(), *this, &Main::handle_sync_signal };

	/* signal transmitter to wake application from input handling */
	Signal_transmitter app_transmitter { app_handler };

	Mode nit_mode = nitpicker.mode();

	Surface_base::Area img_area { };

	Constructible<Attached_dataspace> nit_ds { };

	template <typename PT, typename FN>
	void apply_to_texture(int width, int height, FN const &fn)
	{
		if (nit_mode.width() < width || nit_mode.height() < height) {
			Mode new_mode(max(nit_mode.width(), width),
			              max(nit_mode.height(), height),
			              Mode::RGB565);
			Genode::log("resize nitpicker buffer to ", new_mode);
			if (nit_ds.constructed())
				nit_ds.destruct();
			nitpicker.buffer(new_mode, false);
			nit_ds.construct(env.rm(), nitpicker.framebuffer()->dataspace());
			nit_mode = nitpicker.mode();
			Genode::log("rebuffering complete");
		} else if (!nit_ds.constructed()) {
			nitpicker.buffer(nit_mode, false);
			nit_ds.construct(env.rm(), nitpicker.framebuffer()->dataspace());
		}

		Genode::size_t const buffer_size =
			nit_mode.height() * nit_mode.width() * nit_mode.bytes_per_pixel();

		if (buffer_size > back_ds.size())
			back_ds.realloc(&env.pd(), buffer_size);

		Surface_base::Area nit_area(nit_mode.width(), nit_mode.height());
		Texture<PT> texture(back_ds.local_addr<PT>(), nullptr, nit_area);

		fn(texture);
	}

	void handle_sync_signal()
	{
		typedef Pixel_rgb565 PT;

		Surface_base::Area nit_area(nit_mode.width(), nit_mode.height());

		Texture<PT> texture(back_ds.local_addr<PT>(), nullptr, nit_area);
		Surface<PT> surface(nit_ds->local_addr<PT>(), nit_area);

		Texture_painter::paint(
			surface, texture, Color(), Surface_base::Point(),
			Texture_painter::SOLID, true);

		nitpicker.enqueue<Command::Geometry>(
			view_handle, Nitpicker::Rect(Nitpicker::Point(), img_area));
		nitpicker.enqueue<Command::To_front>(view_handle);
		nitpicker.execute();

		nitpicker.framebuffer()->sync_sigh(Signal_context_capability());
	}

	Genode::Attached_rom_dataspace config_rom { env, "config" };

	Input::Session_client &input = *nitpicker.input();

	Timer::Connection timer { env, "animation" };

	void render_animation(Genode::Duration);

	Timer::One_shot_timeout<Main> render_timeout {
		timer, *this, &Main::render_animation };

	Nitpicker::Session::View_handle view_handle = nitpicker.create_view();

	void render(FLIF_IMAGE *img);

	FLIF_DECODER *flif_dec = NULL;

	struct dirent **namelist = NULL;
	int page_count = 0;

	int cur_page_index = 0;
	int pending_page_index = 0;
	int cur_frame = 0;

	unsigned long last_ms = 0;

	bool progressive = false;
	bool verbose = false;

	bool render_page();

	void next_page()
	{
		while (true) {
			++cur_page_index;
			if (cur_page_index > page_count-1)
				cur_page_index = 0;
			if (render_page())
				break;
		}
	}

	void prev_page()
	{
		while (true) {
			--cur_page_index;
			if (cur_page_index < 0)
				cur_page_index = page_count-1;
			if (render_page())
				break;
		}
	}

	void handle_app_signal()
	{
		if (cur_page_index < pending_page_index) {
			Libc::with_libc([&] () { next_page(); });
		} else
		if (cur_page_index > pending_page_index) {
			Libc::with_libc([&] () { prev_page(); });
		}
	}

	void handle_config()
	{
		progressive = config_rom.xml().attribute_value(
			"progressive", progressive);
		verbose = config_rom.xml().attribute_value(
			"verbose", verbose);

		while (page_count > 0) {
			--page_count;
			free(namelist[page_count]);
		}
		if (namelist != NULL) {
			free(namelist);
			namelist = NULL;
		}

		page_count = scandir(".", &namelist, NULL, alphasort);

		render_page();
	}

	void handle_config_signal()
	{
		config_rom.update();
		Libc::with_libc([&] () { handle_config(); });
	}

	/**
	 * I/O handler for input. May interrupt rendering during I/O
	 * dispatch because the application is not executed from
	 * this signal handler.
	 */
	void handle_input_signal()
	{
		input.for_each_event([&] (Input::Event const &ev) {
			if (ev.key_press(Input::KEY_PAGEDOWN)) {
				++pending_page_index;
				app_transmitter.submit();
			} else
			if (ev.key_press(Input::KEY_PAGEUP)) {
				--pending_page_index;
				app_transmitter.submit();
			}
		});
	}

	Main(Libc::Env &env) : env(env)
	{
		input.sigh(input_handler);
		config_rom.sigh(config_handler);

		handle_config();
	}
};


static ::uint32_t progressive_render(::uint32_t quality, ::int64_t bytes_read, ::uint8_t /*decode_over*/, void *user_data, void *context)
{
	auto *main = (Flif_view::Main*)user_data;

	if (main->verbose) {
		unsigned long const now_ms = main->timer.elapsed_ms();
		double dur_s = double(now_ms - main->last_ms) / 1000.0;
		main->last_ms = now_ms;
		Genode::log((double(bytes_read) / (1 << 20)) / dur_s, " MiB/s");
	}

	if (main->progressive) {
		flif_decoder_generate_preview(context);

		FLIF_IMAGE *img = flif_decoder_get_image(main->flif_dec, 0);
		main->render(img);
	}

	/* abort decoding if input has moved the page index */
	if (main->cur_page_index != main->pending_page_index) {
		flif_abort_decoder(main->flif_dec);
		return 0;
	}

	return ++quality;
}


void Flif_view::Main::render_animation(Genode::Duration)
{
	cur_frame = (cur_frame+1) % flif_decoder_num_images(flif_dec);

	FLIF_IMAGE *img = flif_decoder_get_image(flif_dec, cur_frame);
	render(img);
	auto x = Microseconds(flif_image_get_frame_delay(img)*100);
	render_timeout.schedule(x);
}


void Flif_view::Main::render(FLIF_IMAGE *img)
{
	int const f_width  = flif_image_get_width(img);
	int const f_height = flif_image_get_height(img);

	typedef Pixel_rgb565 PT;
	apply_to_texture<PT>(f_width, f_height, [&] (Texture<PT> &texture) {
		/* fill texture with FLIF image data */
		char row[f_width*4];
		for (int y = 0; y < f_height; ++y) {
			flif_image_read_row_RGBA8(img, y, &row, sizeof(row));
			texture.rgba((unsigned char*)&row, f_width, y);
		}
	});

	/* flush to surface on next sync signal */
	img_area = Surface_base::Area(f_width, f_height);
	nitpicker.framebuffer()->sync_sigh(sync_handler);
}


bool Flif_view::Main::render_page()
{
	pending_page_index = cur_page_index;

	struct dirent *d = namelist[cur_page_index];
	char const *filename = d->d_name;

	if (flif_dec != NULL) {
		flif_destroy_decoder(flif_dec);
		flif_dec = NULL;
	}

	flif_dec = flif_create_decoder();
	flif_decoder_set_resize(flif_dec, nit_mode.width(), nit_mode.height());
	flif_decoder_set_callback(flif_dec, &(progressive_render), this);

	nitpicker.enqueue<Nitpicker::Session::Command::Title>(view_handle, filename);

	if (verbose)
		last_ms = timer.elapsed_ms();

	if (!flif_decoder_decode_file(flif_dec, filename)) {
		Genode::error("decode '", filename, "' failed");
		return false;
	}

	Genode::log(filename);

	FLIF_IMAGE *img = flif_decoder_get_image(flif_dec, 0);
	render(img);

	if (flif_decoder_num_images(flif_dec) > 1) {
		render_timeout.schedule(
			Microseconds(flif_image_get_frame_delay(img)));
	}

	return true;
}


void Libc::Component::construct(Libc::Env &env) {
	with_libc([&env] () { static Flif_view::Main application(env); }); }
