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

#include <base/id_space.h>
#include <os/reporter.h>
#include <pointer/shape_report.h>

#include <gui_session/connection.h>

struct Backend_gui : Genode::List<Backend_gui>::Element
{
	typedef Gui::Session::View_handle View_handle;
	typedef Gui::Session::Command     Command;

	Gui::Connection              gui;
	unsigned               const id;
	Framebuffer::Mode            fb_mode { };
	Genode::Dataspace_capability fb_ds   { };
	Genode::addr_t               pixels  { 0 };
	View_handle                  view    { };

	Input::Session_client       &input;

	Report::Connection         shape_report;
	Genode::Attached_dataspace shape_attached;

	Genode::Point<unsigned>    last_host_pos { 0, 0};

	bool visible { false };

	Backend_gui(Genode::Env &env,
	            Genode::List<Backend_gui> &guis,
	            unsigned id, Gui::Area area,
	            Genode::Signal_context_capability const input_signal,
	            char const *name)
	:
		gui(env, name), id(id),
		input(*gui.input()),
		shape_report(env, "shape", sizeof(Pointer::Shape_report)),
		shape_attached(env.rm(), shape_report.dataspace())
	{
		gui.buffer(Framebuffer::Mode { .area = area }, false);

		fb_ds   = gui.framebuffer()->dataspace();
		fb_mode = gui.framebuffer()->mode();

		pixels = env.rm().attach(fb_ds);

		input.sigh(input_signal);
		guis.insert(this);
	}

	size_t fb_size()
	{
		return Genode::align_addr(fb_mode.area.count() *
		                          fb_mode.bytes_per_pixel(), 12);
	}

	void resize(Genode::Env &env, Framebuffer::Mode const &mode)
	{
		fb_mode = mode;

		if (!visible)
			refresh(0, 0, 1, 1);

		if (pixels)
			env.rm().detach(pixels);

		gui.buffer(fb_mode, false);

		fb_ds = gui.framebuffer()->dataspace();

		Gui::Rect rect(Gui::Point(0, 0), fb_mode.area);

		gui.enqueue<Command::Geometry>(view, rect);
		gui.execute();

		pixels = env.rm().attach(fb_ds);
	}

	void refresh(unsigned x, unsigned y, unsigned width, unsigned height)
	{
		if (!visible) {
			view = gui.create_view();
			Gui::Rect rect(Gui::Point(0, 0), fb_mode.area);

			gui.enqueue<Command::Geometry>(view, rect);
			gui.enqueue<Command::To_front>(view, View_handle());
			gui.execute();

			visible = true;
		}

		gui.framebuffer()->refresh(x, y, width, height);
	}

	void hide()
	{
		if (!visible)
			return;

		gui.destroy_view(view);
		visible = false;
	}

	void mouse_shape(bool visible, unsigned x, unsigned y,
	                 unsigned width, unsigned height)
	{
		Pointer::Shape_report * shape = shape_attached.local_addr<Pointer::Shape_report>();

		shape->visible = visible;
		shape->x_hot   = x;
		shape->y_hot   = y;
		shape->width   = width;
		shape->height  = height;
		shape_report.submit(sizeof(*shape));
	}

	char * shape_ptr() {
		return reinterpret_cast<char *>(shape_attached.local_addr<Pointer::Shape_report>()->shape); }

	unsigned shape_size() const {
		return sizeof(Pointer::Shape_report::shape) / sizeof(Pointer::Shape_report::shape[0]); }

	/************************************************
	 * Pixel buffer handling, copied out from guest *
	 ************************************************/

	struct Pixel_buffer
	{
		using Id = Genode::Id_space<Pixel_buffer>::Id;

		Genode::Id_space<Pixel_buffer>::Element id_element;
		Genode::Ram_dataspace_capability  ds;

		Pixel_buffer(Genode::Id_space<Pixel_buffer> &space, Id id, Genode::Ram_dataspace_capability ds)
		: id_element(*this, space, id), ds(ds) { }
	};

	Genode::Id_space<Pixel_buffer> pixel_buffers { };

	template <typename T>
	void free_buffer(Pixel_buffer::Id const id, T const &fn) {
		return pixel_buffers.apply<Pixel_buffer>(id, [&](Pixel_buffer &buffer) {
			fn(buffer);
		});
	}
};

#endif /* _GUI_H_ */
