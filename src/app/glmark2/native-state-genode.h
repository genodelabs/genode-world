#ifndef _NATIVE_STATE_GENODE_H_
#define _NATIVE_STATE_GENODE_H_

#include <native-state.h>
#include <base/attached_dataspace.h>
#include <base/env.h>
#include <base/log.h>
#include <gui_session/connection.h>

#include <EGL/eglplatform.h>

class NativeStateGenode : public NativeState
{
	private:

		Genode::Env &_env;

		struct Window : Genode_egl_window
		{
			using View_handle = Gui::Session::View_handle;
			using Command = Gui::Session::Command;

			Genode::Env      &env;
			Framebuffer::Mode mode;
			Gui::Connection   gui { env };
			Genode::Constructible<Genode::Attached_dataspace> ds { };
			View_handle       view { };

			Window(Genode::Env &env, int w, int h)
			:
				env(env), mode { .area = Gui::Area(w, h) }
			{
				width  = w;
				height = h;
				type   = WINDOW;

				gui.buffer(mode, false);
				view = gui.create_view();
				mode_change();

				gui.enqueue<Command::Title>(view, "glMark2");
				gui.enqueue<Command::To_front>(view, View_handle());
				gui.execute();
			}

			void refresh()
			{
				gui.framebuffer()->refresh(0, 0, mode.area.w(), mode.area.h());
			}

			void mode_change()
			{
				//mode = gui.mode();
				//gui.buffer(mode, false);

				if (ds.constructed())
					ds.destruct();

				ds.construct(env.rm(), gui.framebuffer()->dataspace());

				addr = ds->local_addr<unsigned char>();

				Gui::Rect rect { Gui::Point { 0, 0 }, mode.area };
				gui.enqueue<Command::Geometry>(view, rect);
				gui.execute();
			}
		};

		Genode::Constructible<Window> _win { };

	public:

		NativeStateGenode(Genode::Env &env)
		:
		  _env(env) { }

		bool init_display() override
		{
			return true;
		}

		void* display() override
		{
			return nullptr;
		}

		bool create_window(WindowProperties const& properties) override
		{
			if (_win.constructed()) return true;

			_win.construct(_env, properties.width, properties.height);
			return true;
		}

		void* window(WindowProperties& properties) override
		{
			if (!_win.constructed()) return nullptr;

			properties.width  = _win->width;
			properties.height = _win->height;

			return &*_win;
		}

		void visible(bool v) override
		{
		}

		bool should_quit() override
		{
			return false;
		}

		void flip() override
		{
			if (!_win.constructed()) return;

			_win->refresh();
		}

		static NativeStateGenode &native_state(Genode::Env *env = 0);
};

#endif /* _NATIVE_STATE_GENODE_H_ */
