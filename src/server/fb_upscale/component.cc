/*
 * \brief  Linear framebuffer upscaler
 * \author Emery Hemingway
 * \date   2016-07-20
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <framebuffer_session/connection.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_dataspace.h>
#include <root/component.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/log.h>


namespace Fb_scaler {

	using namespace Framebuffer;

	class Session_component;
	class Root_component;

	typedef
		Genode::Root_component<Session_component, Genode::Single_client>
		Root_component_base;

}


class Fb_scaler::Session_component : public Genode::Rpc_object<Framebuffer::Session>
{
	private:

		Genode::Env &_env;

		Framebuffer::Connection _parent_fb { _env, Framebuffer::Mode() };
		Framebuffer::Mode       _parent_mode = _parent_fb.mode();
		Framebuffer::Mode       _client_mode;

		Genode::Attached_ram_dataspace _client_ds
			{ _env.ram(), _env.rm(),
			  Genode::size_t(_client_mode.width()*_client_mode.height())*2
			};

		Genode::Constructible<Genode::Attached_dataspace> _parent_ds;

		Genode::Signal_context_capability _client_sig_cap;

		enum { SHIFT = 16 };

		int _scale_factor;
		int _scale_ratio;
		int _x_offset;
		int _y_offset;

		void _rescale()
		{
			/* get a new dataspace */
			if (_parent_ds.constructed())
				_parent_ds.destruct();
			_parent_ds.construct(_env.rm(), _parent_fb.dataspace());
			Genode::memset(_parent_ds->local_addr<char>(),
			               0x00, _parent_ds->size());

			/* calculate the scaling using the FPU */
			float x_factor =
				float(_parent_mode.width()) /
				float(_client_mode.width());

			float y_factor =
				float(_parent_mode.height()) /
				float(_client_mode.height());

			float factor = Genode::min(x_factor, y_factor);

			_x_offset =
				(_parent_mode.width() -
				 (_client_mode.width()*factor)) / 2;
			_y_offset =
				(_parent_mode.height() -
				 (_client_mode.height()*factor)) / 2;

			/* shift so the scaling can be done with integeral math */
			_scale_factor = (1<<SHIFT)*factor;
			_scale_ratio  = ((1 << SHIFT) / factor)+1;
		}

		void _handle_mode()
		{
			_parent_mode = _parent_fb.mode();

			if (_parent_mode.width() && _parent_mode.height()) {
				_rescale();
				refresh(0,0, _client_mode.width(), _client_mode.height());
			} else {
				/* notify the client of the null mode */
				if (_client_sig_cap.valid())
					Genode::Signal_transmitter(_client_sig_cap).submit();
			}
		}

		Genode::Signal_handler<Session_component> _mode_handler
			{ _env.ep(), *this, &Session_component::_handle_mode };

	public:

		Session_component(Genode::Env &env, Mode client_mode)
		:
			_env(env),
			_client_mode(client_mode.width() && client_mode.height() ?
			             client_mode : _parent_mode)
		{
			if (!(_client_mode.width() && _client_mode.height())) {
				/* use the parent mode maybe enlarge later */
				_client_mode = _parent_mode;
				_handle_mode();
			}

			_rescale();
			_parent_fb.mode_sigh(_mode_handler);
		}


		/***********************************
		 ** Framebuffer session interface **
		 ***********************************/

		Genode::Dataspace_capability dataspace() override {
			return _client_ds.cap(); }

		Mode mode() const override
		{
			Mode parent_mode = _parent_fb.mode();
			return (!parent_mode.width() && !parent_mode.height()) ?
				parent_mode : _client_mode;
		}

		void refresh(int cx, int cy, int cw, int ch) override
		{
			using Genode::uint16_t;

			/* the shifting is to round down by dropping bits */

			int px = (cx * _scale_factor)>>SHIFT;
			int py = (cy * _scale_factor)>>SHIFT;
			int pw = (cw * _scale_factor)>>SHIFT;
			int ph = (ch * _scale_factor)>>SHIFT;

			uint16_t const *src = _client_ds.local_addr<uint16_t const>();
			uint16_t *dst = _parent_ds->local_addr<uint16_t>();

			for (int y = py; y < ph; ++y) {
				int src_y = ((y*_scale_ratio) >> SHIFT)*_client_mode.width();

				for (int x = px; x < pw; ++x) {
					dst[(_y_offset+y)*_parent_mode.width()+x+_x_offset] =
						src[src_y+((x*_scale_ratio) >> SHIFT)];
				}
			}

			_parent_fb.refresh(_x_offset+px, _y_offset+py, pw, ph);
		}


		void mode_sigh(Genode::Signal_context_capability sig_cap) override {
			_client_sig_cap = sig_cap; }

		/* client gets sync signals from parent session */
		void sync_sigh(Genode::Signal_context_capability sig_cap) override {
			_parent_fb.sync_sigh(sig_cap); }

};


class Fb_scaler::Root_component : Root_component_base
{
	private:

		Genode::Env &_env;

	protected:

		Session_component *_create_session(char const *args) override
		{
			using namespace Genode;

			unsigned      width = Arg_string::find_arg(args, "fb_width").ulong_value(0);
			unsigned     height = Arg_string::find_arg(args, "fb_width").ulong_value(0);

			return new (md_alloc())
				Session_component(_env, Mode(width, height, Mode::INVALID));
		}

	public:

		Root_component(Genode::Env &env, Genode::Allocator &md_alloc)
		: Root_component_base(env.ep(), md_alloc),
			_env(env)
		{
			env.parent().announce(env.ep().manage(*this));
		}
};


/***************
 ** Component **
 ***************/

Genode::size_t Component::stack_size() { return 4*1024*sizeof(Genode::addr_t); }

void Component::construct(Genode::Env &env)
{
	static Genode::Sliced_heap sliced_heap(env.ram(), env.rm());

	static Fb_scaler::Root_component root(env, sliced_heap);
}
