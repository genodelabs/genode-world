/*
 * \brief  Backend GUI helper setup
 * \author Alexander Boettcher
 * \date   2021-04-10
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#ifndef _GUI_H_
#define _GUI_H_

struct Backend_gui : Genode::List<Backend_gui>::Element
{
	Gui::Connection              gui;
	unsigned               const id;
	size_t                       fb_size { 0 };
	Framebuffer::Mode            fb_mode { };
	Genode::Dataspace_capability fb_ds   { };

	Input::Session_client &input;

	Backend_gui(Genode::Env &env,
	            Genode::List<Backend_gui> &guis,
	            unsigned id, Gui::Area area,
	            Genode::Signal_context_capability const input_signal,
	            char const *name)
	:
		gui(env, name), id(id),
		input(*gui.input())
	{
		gui.buffer(Framebuffer::Mode { .area = area }, false);

		fb_ds   = gui.framebuffer()->dataspace();
		fb_mode = gui.framebuffer()->mode();

		fb_size = Genode::align_addr(fb_mode.area.count() *
		                             fb_mode.bytes_per_pixel(), 12);

		typedef Gui::Session::View_handle View_handle;
		typedef Gui::Session::Command Command;

		View_handle view = gui.create_view();
		Gui::Rect rect(Gui::Point(0, 0), fb_mode.area);

		gui.enqueue<Command::Geometry>(view, rect);
		gui.enqueue<Command::To_front>(view, View_handle());
		gui.execute();

		input.sigh(input_signal);
		guis.insert(this);
	}

	void refresh()
	{
		gui.framebuffer()->refresh(0, 0, fb_mode.area.w(), fb_mode.area.h());
	}

	void refresh(unsigned x, unsigned y, unsigned width, unsigned height)
	{
		gui.framebuffer()->refresh(x, y, width, height);
	}
};

#endif /* _GUI_H_ */
