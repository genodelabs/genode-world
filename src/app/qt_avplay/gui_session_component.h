/*
 * \brief   GUI session component
 * \author  Christian Prochaska
 * \date    2019-03-12
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _GUI_SESSION_COMPONENT_H_
#define _GUI_SESSION_COMPONENT_H_

/* Genode includes */
#include <gui_session/connection.h>
#include <os/dynamic_rom_session.h>

/* Qt includes */
#include <qpa_genode/qgenodeplatformwindow.h>
#include <qgenodeviewwidget/qgenodeviewwidget.h>


namespace Gui {
	using namespace Genode;
	struct Session_component;
}


struct Gui::Session_component : Rpc_object<Gui::Session>,
                                private Dynamic_rom_session::Xml_producer,
                                private Input::Session_component::Action
{
	Env               &_env;
	QGenodeViewWidget *_genode_view_widget;

	Gui::Connection _connection;

	Input::Session_component _input_component {
		_env.ep(), _env.ram(), _env.rm(), *this };

	/**
	 * Input::Session_component::Action interface
	 */
	void exclusive_input_requested(bool) override { };

	Dynamic_rom_session _info_rom { _env.ep(), _env.ram(), _env.rm(), *this };

	using Command_buffer = Gui::Session::Command_buffer;
	Attached_ram_dataspace _command_ds;
	Command_buffer &_command_buffer = *_command_ds.local_addr<Command_buffer>();

	Gui::View_ref          _view_ref { };
	Gui::View_ids::Element _view { _view_ref, _connection.view_ids };

	Input::Session_component &input_component() { return _input_component; }

	/**
	 * Dynamic_rom_session::Xml_producer interface
	 */
	void produce_xml(Xml_generator &xml) override
	{
		xml.attribute("width",  _genode_view_widget->maximumWidth());
		xml.attribute("height", _genode_view_widget->maximumHeight());
	}

	void _execute_command(Command const &command)
	{
	 	Libc::with_libc([&] {

			switch (command.opcode) {

			case Command::GEOMETRY:
				{
					Gui::Rect rect = command.geometry.rect;
					_genode_view_widget->setGenodeView(&_connection, _view.id(),
					                                   0, 0, rect.w(), rect.h());
					return;
				}

			case Command::OFFSET:
			case Command::FRONT:
			case Command::BACK:
			case Command::FRONT_OF:
			case Command::BEHIND_OF:
			case Command::BACKGROUND:
			case Command::TITLE:
			case Command::NOP:
				return;
			}
		});
	}

	Session_component(Env &env, QGenodeViewWidget *genode_view_widget)
	:
		Dynamic_rom_session::Xml_producer("panorama"),
		_env(env),
		_genode_view_widget(genode_view_widget),
		_connection(env),
		_command_ds(env.ram(), env.rm(), sizeof(Command_buffer))
	{
		_env.ep().manage(_input_component);
		_env.ep().manage(*this);
		_input_component.event_queue().enabled(true);
	}

	~Session_component()
	{
		_env.ep().dissolve(*this);
		_env.ep().dissolve(_input_component);
	}

	Framebuffer::Session_capability framebuffer() override {
		return _connection.cap().call<Rpc_framebuffer>(); }

	Input::Session_capability input() override {
		return _input_component.cap(); }

	View_result view(View_id /* ignored */, View_attr const &attr) override
	{
	 	Libc::with_libc([&] {

			QGenodePlatformWindow *platform_window =
				dynamic_cast<QGenodePlatformWindow*>(_genode_view_widget
					->window()->windowHandle()->handle());

			View_ref tmp_ref { };
			View_ids::Element tmp { tmp_ref, _connection.view_ids };

			_connection.associate(tmp.id(), platform_window->view_cap());
			_connection.child_view(_view.id(), tmp.id(), attr);
			_connection.release_view_id(tmp.id());
		});

		return View_result::OK;
	}

	Child_view_result child_view(View_id, View_id, View_attr const &) override
	{
		warning("unexpected call of create_child_view");
		return Child_view_result::OK;
	}

	void destroy_view(View_id view) override {
		_connection.destroy_view(view); }

	Associate_result associate(View_id id, View_capability cap) override
	{
		_connection.associate(id, cap);
		return Associate_result::OK;
	}

	View_capability_result view_capability(View_id view) override {
		return _connection.view_capability(view); }

	void release_view_id(View_id view) override {
		_connection.release_view_id(view); }

	Dataspace_capability command_dataspace() override {
		return _command_ds.cap(); }

	void execute() override
	{
		for (unsigned i = 0; i < _command_buffer.num(); i++)
			_execute_command(_command_buffer.get(i));
	}

	Info_result info() override
	{
		return _info_rom.cap();
	}

	Buffer_result buffer(Framebuffer::Mode mode) override
	{
		_connection.buffer(mode);
		return Buffer_result::OK;
	}

	void focus(Capability<Gui::Session> session) override {
		_connection.focus(session); }
};

#endif /* _GUI_SESSION_COMPONENT_H_ */
