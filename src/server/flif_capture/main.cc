/*
 * \brief  Screen capture utility
 * \author Emery Hemingway
 * \date   2019-01-22
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* FLIF includes */
#include <flif_enc.h>

/* Libc includes */
#include <time.h>

/* Genode includes */
#include <libc/component.h>
#include <framebuffer_session/connection.h>
#include <input_session/connection.h>
#include <input/component.h>
#include <base/attached_dataspace.h>
#include <os/static_root.h>

namespace Flif_capture {
	using namespace Genode;

	class Framebuffer_session_component;
	class Encoder;
	class Main;

	using Framebuffer::Mode;

	/**
	 * Endian work-around struct
	 */
	struct RGBA
	{
		uint8_t r,g,b,a;
	} __attribute__((packed));
}


class Flif_capture::Encoder : Genode::Thread
{
	private:

		Genode::Env &_env;
		FLIF_IMAGE  *_image = nullptr;
		Semaphore    _semaphore { };
		Lock         _lock { Lock::LOCKED };

		static size_t stack_size()
		{
			auto info = Thread::mystack();
			return info.top - info.base;
		}

	protected:

		void entry()
		{
			for (;;) {
				while (_image == nullptr) {
					_lock.unlock();
					_semaphore.down();
					_lock.lock();
				}

				char filename[32] { '\0' };

				Libc::with_libc([&] () {
					/* calculate Sumerian time, hopefully */
					time_t now = time(NULL);
					struct tm more_now { };
					localtime_r(&now, &more_now);
					strftime(filename, sizeof(filename), "%T.flif", &more_now);
				});

				FLIF_ENCODER* encoder = flif_create_encoder();
				if (!encoder) {
					Genode::error("failed to create FLIF encoder");
					continue;
				}

				flif_encoder_add_image(encoder, _image);

				Genode::log("capture to ", (char const *)filename);
				Libc::with_libc([&] () {
					if (!flif_encoder_encode_file(encoder, filename))
						Genode::error("file encoding failed");
				});

				flif_destroy_encoder(encoder);
				flif_destroy_image(_image);
				_image = nullptr;
			}
		}

	public:

		bool capture_pending = false;

		/* safety noise */
		Encoder(Encoder const &);
		Encoder &operator = (Encoder const &);

		Encoder(Genode::Env &env)
		:
			Thread(env, Thread::Name("encoder"), stack_size(),
			       env.cpu().affinity_space().location_of_index(1),
			       (Thread::Weight)Thread::Weight::DEFAULT_WEIGHT,
			       env.cpu()),
			_env(env)
		{
			start();
		}

		void queue(Dataspace_capability fb_cap, Mode const mode)
		{
			Lock::Guard guard(_lock);

			if (_image != nullptr)
				return;

			/* TODO: attach/detach each capture? */
			Genode::Attached_dataspace fb_ds(_env.rm(), fb_cap);
			if ((mode.format() != Mode::RGB565)
			 || (fb_ds.size() < (mode.width() * mode.height() * mode.bytes_per_pixel()))) {
				Genode::error("invalid framebuffer for capture");
				return;
			}

			RGBA row[mode.width()];

			_image = flif_create_image(mode.width(), mode.height());
			if (!_image) {
				Genode::error("failed to create image buffer");
				return;
			}

			uint16_t const *pixels = fb_ds.local_addr<uint16_t const>();

			for (int y = 0; y < mode.height(); y++) {
				for (int x = 0; x < mode.width(); x++) {
					uint16_t px = pixels[y*mode.width()+x];
					row[x] = RGBA {
						(uint8_t)((px & 0xf800) >> 8),
						(uint8_t)((px & 0x07e0) >> 3),
						(uint8_t)((px & 0x1f) << 3),
						(uint8_t)(0xff)
					};
				}
				flif_image_write_row_RGBA8(_image, y, row, sizeof(row));
			}

			_semaphore.up();
		}
};


class Flif_capture::Framebuffer_session_component
:
	public Genode::Rpc_object<Framebuffer::Session>
{
	private:

		Genode::Env                  &_env;
		Framebuffer::Session         &_parent;
		Flif_capture::Encoder         &_encoder;

		Genode::Dataspace_capability  _dataspace { };
		Framebuffer::Mode             _mode { };

	public:

		/**
		 * Constructor
		 */
		Framebuffer_session_component(Genode::Env &env,
		                              Framebuffer::Session &client,
		                              Flif_capture::Encoder &encoder)
		: _env(env), _parent(client), _encoder(encoder) { }


		/************************************
		 ** Framebuffer::Session interface **
		 ************************************/

		Genode::Dataspace_capability dataspace() override
		{
			_mode = _parent.mode();
			_dataspace = _parent.dataspace();
			return _dataspace;
		}

		Mode mode() const override
		{
			return _parent.mode();
		}

		void mode_sigh(Genode::Signal_context_capability sigh) override
		{
			_parent.mode_sigh(sigh);
		}

		void refresh(int x, int y, int w, int h) override
		{
			_parent.refresh(x, y, w, h);
			if (_encoder.capture_pending) {
				if (_dataspace.valid()) {
					_encoder.queue(_dataspace, _mode);
					_encoder.capture_pending = false;
				}
			}
		}

		void sync_sigh(Genode::Signal_context_capability sigh) override
		{
			_parent.sync_sigh(sigh);
		}
};


class Flif_capture::Main
{
	private:

		Genode::Env &_env;

		Flif_capture::Encoder   _encoder      { _env };
		Framebuffer::Connection _parent_fb    { _env, Mode(0, 0, Mode::RGB565) };
		Input::Connection       _parent_input { _env };

		Framebuffer_session_component _fb_session {
			_env, _parent_fb, _encoder };

		Input::Session_component _input_session {
			_env, _env.ram() };

		Framebuffer::Session_capability _fb_cap { _env.ep().manage(_fb_session) };
		Input::Session_capability    _input_cap { _env.ep().manage(_input_session) };

		Genode::Signal_handler<Main> _input_handler {
			_env.ep(), *this, &Main::_handle_input };

		Input::Keycode _capture_code = Input::KEY_PRINT;

		void _handle_input()
		{
			using namespace Input;

			Event_queue &queue = _input_session.event_queue();

			_parent_input.for_each_event([&] (Event const &e) {
				enum { SUBMIT_LATER = false };
				queue.add(e, SUBMIT_LATER);

				if (e.key_release(_capture_code)) {
					_encoder.capture_pending = true;
				}
			});

			queue.submit_signal();
		}

		Genode::Static_root<Framebuffer::Session> _fb_root {
			_env.ep().manage(_fb_session) };

		Genode::Static_root<Input::Session> _input_root {
			_env.ep().manage(_input_session) };

	public:

		/**
		 * Constructor
		 */
		Main(Libc::Env &env) : _env(env)
		{
			/* register input handler */
			_parent_input.sigh(_input_handler);

			_input_session.event_queue().enabled(true);

			env.parent().announce(env.ep().manage(_fb_root));
			env.parent().announce(env.ep().manage(_input_root));

			Genode::log("--- screenshot capture key is ", Input::key_name(_capture_code), " ---");
		}
};


/***************
 ** Component **
 ***************/

void Libc::Component::construct(Libc::Env &env) {
		static Flif_capture::Main inst(env); }
