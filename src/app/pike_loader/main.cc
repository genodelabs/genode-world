/*
 * \brief  Rio inspired launcher
 * \author Emery Hemingway
 * \date   2017-04-13
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <util/geometry.h>
#include <loader_session/connection.h>
#include <nitpicker_session/connection.h>
#include <input/event.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>

namespace Morse {

	typedef Nitpicker::Session::View_handle View_handle;
	typedef Nitpicker::Session::Command     Command;
	typedef Genode::String<80> String;
	struct Main;

};


struct Morse::Main
{
	Genode::Env &env;

	Genode::Attached_rom_dataspace config_rom { env, "config" };

	Nitpicker::Connection nitpicker { env, "input" };

	Input::Session_client &input = *nitpicker.input();

	Genode::Reconstructible<Loader::Connection> loader {
		env,
		Genode::Ram_quota{env.pd().avail_ram().value - 4096},
		Genode::Cap_quota{env.pd().avail_caps().value - 4} };

	/********************************
	 ** State held between signals **
	 ********************************/

	enum Phase { IDLE, ENTER, DRAG, EXIT };

	Phase phase = IDLE;

	int entry_key = Input::KEY_UNKNOWN;
	int entry_count;
	int entry_x;
	int entry_y;
	int exit_x;
	int exit_y;


	/***************
	 ** Rendering **
	 ***************/

	Nitpicker::Session::View_handle view_handle = nitpicker.create_view();

	Nitpicker::Point nit_point()
	{
		return Nitpicker::Point(
			entry_x < exit_x ? entry_x : exit_x,
			entry_y < exit_y ? entry_y : exit_y);
	}


	Nitpicker::Area nit_area()
	{
		return Nitpicker::Area(
			entry_x < exit_x ? exit_x - entry_x : entry_x - exit_x,
			entry_y < exit_y ? exit_y - entry_y : entry_y - exit_y);
	}

	Nitpicker::Rect nit_rect()
	{
		return Nitpicker::Rect(nit_point(), nit_area());
	}

	void render_drag()
	{
		nitpicker.enqueue<Command::Geometry>(view_handle, nit_rect());
		nitpicker.execute();
	}

	String loader_binary(Genode::Xml_node const &start_node)
	{
		using Genode::Xml_node;
		String name;

		try {
			Xml_node binary = start_node.sub_node("binary");
			binary.attribute("name").value(&name);
		} catch (Xml_node::Nonexistent_sub_node) {
			start_node.attribute("name").value(&name);
		}
		return name;
	}

	String loader_label(Genode::Xml_node const &start_node)
	{
		String name;
		start_node.attribute("name").value(&name);
		return name;
	}

	void render_loader()
	{
		using namespace Nitpicker;
		using namespace Genode;

		/* Reduce the view area to zero unit the loader is ready */
		nitpicker.enqueue<Command::Geometry>(
			view_handle, Nitpicker::Rect(nit_point(), Nitpicker::Area()));
		nitpicker.execute();

		/* If the selected view is too tiny, back out */
		Nitpicker::Area area = nit_area();
		if (area.w() < 16 && area.h() < 16)
			return;

		Xml_node start_node = config_rom.xml().sub_node("start");

		/* We are only patron of single load, so transfer the slack RAM */
		loader.construct(
			env,
			Genode::Ram_quota{env.pd().avail_ram().value - 4096},
			Genode::Cap_quota{env.pd().avail_caps().value - 4});

		try {
			Xml_node config_xml = start_node.sub_node("config");
			Attached_dataspace ds(
				env.rm(), loader->alloc_rom_module("config", config_xml.size()));

			strncpy(ds.local_addr<char>(), config_xml.addr(),
			        config_xml.size()+1);
			loader->commit_rom_module("config");
		} catch (Xml_node::Nonexistent_sub_node) { }


		loader->view_ready_sigh(view_ready_handler);
		loader->parent_view(nitpicker.view_capability(view_handle));

		loader->start(loader_binary(start_node).string(),
		              loader_label(start_node).string());
	}


	/*********************
	 ** Signal handling **
	 *********************/

	void handle_input()
	{
		using namespace Input;

		input.for_each_event([&] (Event const &ev) {
			switch (ev.type()) {
			case Event::MOTION:
				switch (phase) {
				case ENTER:
					phase = DRAG;
					nitpicker.enqueue<Command::To_front>(view_handle);
					entry_x = exit_x = ev.ax();
					entry_y = exit_y = ev.ay();
					break;
				case DRAG:
					exit_x = ev.ax();
					exit_y = ev.ay();
					break;
				default: break;
				}
				break;

			case Event::PRESS:
				switch (phase) {
				case IDLE:
					phase = ENTER;
					loader.destruct();
					entry_key = ev.code();
					entry_count = 1;
					entry_x = entry_y = exit_x = exit_y = 0;
					break;
				default: break;
				}
				break;

			case Event::RELEASE:
				switch (phase) {
				case IDLE:
					break;
				default:
					if (entry_key == ev.code()) {
						phase = EXIT;
						nitpicker.enqueue<Command::To_front>(view_handle);
					}
				}
				break;
			default: break;
			}
		});

		switch (phase) {
		case DRAG:
			render_drag();
			break;

		case EXIT:
			phase = IDLE;
			render_loader();
			break;

		default:
			break;
		}
	}

	Genode::Signal_handler<Main> input_handler {
		env.ep(), *this, &Main::handle_input };

	void handle_view_ready()
	{
		Loader::Area const la = loader->view_size();
		Nitpicker::Area const na = nit_area();
		{
			using namespace Loader;

			Area area(
				Genode::min(la.w(), na.w()),
				Genode::min(la.h(), na.h()));

			Rect const rect(Point(), area);

			{
				using namespace Nitpicker;

				Point const np = nit_point();

				Point const offset_point(
					rect.w() < na.w() ? np.x() + (na.w() - rect.w()) / 2 : np.x(),
					rect.h() < na.h() ? np.y() + (na.h() - rect.h()) / 2 : np.y());
				Rect const offset_rect(offset_point, area);
				nitpicker.enqueue<Command::Geometry>(
					view_handle, offset_rect);
				nitpicker.execute();
			}

			loader->view_geometry(rect, Point());
		}
	}

	Genode::Signal_handler<Main> view_ready_handler {
		env.ep(), *this, &Main::handle_view_ready };

	Main(Genode::Env &env) : env(env)
	{
		input.sigh(input_handler);
	}
};


void Component::construct(Genode::Env &env)
{
	static Morse::Main main(env);
}
