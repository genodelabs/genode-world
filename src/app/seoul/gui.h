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
	Gui::Connection              gui;
	unsigned short         const id;
	Gui::Area                    fb_area;
	Genode::Dataspace_capability fb_ds  { };
	Genode::addr_t               pixels { };
	Gui::View_id           const view   { };

	Report::Connection         shape_report;
	Genode::Attached_dataspace shape_attached;

	bool visible { false };

	Framebuffer::Mode _mode() const { return { .area = fb_area, .alpha = false }; }

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

	Gui::Rect gui_window()
	{
		return gui.window().convert<Gui::Rect>(
			[&] (Gui::Rect rect) { return rect; },
			[&] (Gui::Undefined) { return gui.panorama().convert<Gui::Rect>(
				[&] (Gui::Rect rect) { return rect; },
				[&] (Gui::Undefined) { return Gui::Rect { { }, { 640, 480 } }; }); });
	}

	Backend_gui(Genode::Env &env,
	            Genode::List<Backend_gui> &guis,
	            unsigned short id, Gui::Area area,
	            Genode::Signal_context_capability const input_signal,
	            char const *name)
	:
		gui(env, name), id(id), fb_area(area),
		shape_report(env, "shape", sizeof(Pointer::Shape_report)),
		shape_attached(env.rm(), shape_report.dataspace())
	{
		gui.buffer(_mode());
		fb_ds = gui.framebuffer.dataspace();
		_attach_fb_ds(env, fb_ds);

		gui.input.sigh(input_signal);
		guis.insert(this);
	}

	size_t fb_size() { return Genode::align_addr(_mode().num_bytes(), 12); }

	void resize(Genode::Env &env, Gui::Area const area)
	{
		fb_area = area;

		if (!visible)
			refresh(0, 0, 1, 1);

		if (pixels)
			env.rm().detach(pixels);

		gui.buffer({ .area = area, .alpha = false });

		fb_ds = gui.framebuffer.dataspace();

		Gui::Rect rect(Gui::Point(0, 0), area);

		gui.enqueue<Gui::Session::Command::Geometry>(view, rect);
		gui.execute();

		_attach_fb_ds(env, fb_ds);
	}

	void refresh(unsigned x, unsigned y, unsigned width, unsigned height)
	{
		if (!visible) {
			gui.view(view, { .title = "",
			                 .rect  = { { 0, 0 }, fb_area },
			                 .front = true });
			visible = true;
		}

		gui.framebuffer.refresh({ { int(x), int(y) }, { width, height } });
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
