/*
 * \brief  Backend GUI helper setup
 * \author Alexander Boettcher
 * \date   2021-04-10
 */

/*
 * Copyright (C) 2021-2024 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#ifndef _GUI_H_
#define _GUI_H_

#include <base/id_space.h>
#include <base/attached_ram_dataspace.h>
#include <os/reporter.h>
#include <pointer/shape_report.h>

#include <gui_session/connection.h>

struct Backend_gui : Genode::List<Backend_gui>::Element
{
	typedef Gui::Session::Command Command;

	Gui::Connection              gui;
	unsigned short         const id;
	Framebuffer::Mode            fb_mode { };
	Genode::Dataspace_capability fb_ds   { };
	Genode::addr_t               pixels  { };
	Gui::View_id           const view    { };

	Report::Connection         shape_report;
	Genode::Attached_dataspace shape_attached;

	bool visible { false };

	void _attach_fb_ds(Genode::Env &env, Genode::Dataspace_capability ds)
	{
		Genode::Region_map::Attr attr { };
		attr.writeable = true;

		pixels = env.rm().attach(ds, attr).convert<Genode::addr_t>(
			[&] (auto const &range) { return range.start; },
			[&] (auto const &error) {
				Genode::error("gui creation failed");
				return 0ul;
			});
	}

	Backend_gui(Genode::Env &env,
	            Genode::List<Backend_gui> &guis,
	            unsigned short id, Gui::Area area,
	            Genode::Signal_context_capability const input_signal,
	            char const *name)
	:
		gui(env, name), id(id),
		shape_report(env, "shape", sizeof(Pointer::Shape_report)),
		shape_attached(env.rm(), shape_report.dataspace())
	{
		gui.buffer(Framebuffer::Mode { .area = area }, false);

		fb_ds   = gui.framebuffer.dataspace();
		fb_mode = gui.framebuffer.mode();

		_attach_fb_ds(env, fb_ds);

		gui.input.sigh(input_signal);
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

		fb_ds = gui.framebuffer.dataspace();

		Gui::Rect rect(Gui::Point(0, 0), fb_mode.area);

		gui.enqueue<Command::Geometry>(view, rect);
		gui.execute();

		_attach_fb_ds(env, fb_ds);
	}

	void refresh(unsigned x, unsigned y, unsigned width, unsigned height)
	{
		if (!visible) {
			gui.view(view, { .title = "",
			                 .rect  = { { 0, 0 }, fb_mode.area },
			                 .front = true });
			visible = true;
		}

		gui.framebuffer.refresh(x, y, width, height);
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
		using Type = Genode::Id_space<Pixel_buffer>;

		Type::Element                  id_element;
		Genode::Attached_ram_dataspace ram;

		Pixel_buffer(Genode::Env &env, Type &space, Type::Id id, size_t size)
		:
			id_element(*this, space, id),
			ram(env.ram(), env.rm(), size)
		{ }
	};

	Genode::Id_space<Pixel_buffer> pixel_buffers { };

	void free_buffer(Pixel_buffer::Type::Id const &id, auto const &fn)
	{
		pixel_buffers.apply<Pixel_buffer>(id, [&](Pixel_buffer &buffer) {
			fn(buffer);
		});

		if (!pixel_buffers.apply_any<Pixel_buffer>([](auto &){}))
			hide();
	}
};

#endif /* _GUI_H_ */
