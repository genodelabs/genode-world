/*
 * \brief   Nitpicker session component
 * \author  Christian Prochaska
 * \date    2019-03-12
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _NITPICKER_SESSION_COMPONENT_H_
#define _NITPICKER_SESSION_COMPONENT_H_

/* Genode includes */
#include <nitpicker_session/connection.h>

/* Qt includes */
#include <qnitpickerplatformwindow.h>
#include <qnitpickerviewwidget/qnitpickerviewwidget.h>


namespace Nitpicker {
	using namespace Genode;
	struct Session_component;
}


struct Nitpicker::Session_component : Rpc_object<Nitpicker::Session>
{
	Env                     &_env;
	Entrypoint              &_ep;
	QNitpickerViewWidget    &_nitpicker_view_widget;

	Nitpicker::Connection    _connection;

	Input::Session_component _input_component { _env, _env.ram() };

	typedef Nitpicker::Session::Command_buffer Command_buffer;
	Attached_ram_dataspace _command_ds;
	Command_buffer &_command_buffer = *_command_ds.local_addr<Command_buffer>();

	Nitpicker::Session::View_handle _view_handle;


	Input::Session_component &input_component() { return _input_component; }

	void _execute_command(Command const &command)
	{
		switch (command.opcode) {

		case Command::OP_GEOMETRY:
			{
				Nitpicker::Rect rect = command.geometry.rect;
				_nitpicker_view_widget.setNitpickerView(&_connection,
				                                         _view_handle,
				                                        0, 0,
				                                        rect.w(), rect.h());
				return;
			}

		case Command::OP_OFFSET:     return;
		case Command::OP_TO_FRONT:   return;
		case Command::OP_TO_BACK:    return;
		case Command::OP_BACKGROUND: return;
		case Command::OP_TITLE:      return;
		case Command::OP_NOP:        return;
		}
	}

	Session_component(Env &env, Entrypoint &ep,
	                  QNitpickerViewWidget &nitpicker_view_widget)
	:
		_env(env), _ep(ep),
		_nitpicker_view_widget(nitpicker_view_widget),
		_connection(env),
		_command_ds(env.ram(), env.rm(), sizeof(Command_buffer))
	{
		_ep.manage(_input_component);
		_ep.manage(*this);
		_input_component.event_queue().enabled(true);
	}

	~Session_component()
	{
		_ep.dissolve(*this);
		_ep.dissolve(_input_component);
	}

	Framebuffer::Session_capability framebuffer_session() override {
		return _connection.framebuffer_session(); }

	Input::Session_capability input_session() override {
		return _input_component.cap(); }

	View_handle create_view(View_handle parent) override
	{
		QNitpickerPlatformWindow *platform_window =
			dynamic_cast<QNitpickerPlatformWindow*>(_nitpicker_view_widget
				.window()->windowHandle()->handle());

		Nitpicker::Session::View_handle parent_view_handle =
			_connection.view_handle(platform_window->view_cap());

		_view_handle = _connection.create_view(parent_view_handle);

		_connection.release_view_handle(parent_view_handle);

		return _view_handle;
	}

	void destroy_view(View_handle view) override {
		_connection.destroy_view(view); }

	View_handle view_handle(View_capability view_cap, View_handle handle) override {
		return _connection.view_handle(view_cap, handle); }

	View_capability view_capability(View_handle view) override {
		return _connection.view_capability(view); }

	void release_view_handle(View_handle view) override {
		_connection.release_view_handle(view); }

	Dataspace_capability command_dataspace() override {
		return _command_ds.cap(); }

	void execute() override
	{
		for (unsigned i = 0; i < _command_buffer.num(); i++)
			_execute_command(_command_buffer.get(i));
	}

	Framebuffer::Mode mode() override
	{
		Framebuffer::Mode connection_mode { _connection.mode() };
		Framebuffer::Mode new_mode {
			Genode::min(connection_mode.width(),
			            _nitpicker_view_widget.maximumWidth()),
			Genode::min(connection_mode.height(),
			            _nitpicker_view_widget.maximumHeight()),
			connection_mode.format()
		};
		return new_mode;
	}

	void mode_sigh(Signal_context_capability sigh) override {
		_connection.mode_sigh(sigh); }

	void buffer(Framebuffer::Mode mode, bool use_alpha) override {
		_connection.buffer(mode, use_alpha); }

	void focus(Capability<Nitpicker::Session> session) override {
		_connection.focus(session); }
};

#endif /* _NITPICKER_SESSION_COMPONENT_H_ */
