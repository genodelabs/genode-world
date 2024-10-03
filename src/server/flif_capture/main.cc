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
#include <os/pixel_rgb888.h>

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


class Flif_capture::Encoder
{
	private:

		Genode::Env &_env;

		// TODO: reuse FLIF_IMAGE, do not reallocate
		FLIF_IMAGE  *_image = nullptr;
		Semaphore    _semaphore { };
		Mutex        _mutex { };

	public:

		bool capture_pending = false;

		/* safety noise */
		Encoder(Encoder const &);
		Encoder &operator = (Encoder const &);

		Encoder(Genode::Env &env) : _env(env) { }

		/**
		 * Encode loop called from initial thread
		 */
		void entry()
		{
			_mutex.acquire();

			for (;;) {
				while (_image == nullptr) {
					_mutex.release();
					_semaphore.down();
					_mutex.acquire();
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
				flif_encoder_set_lookback(encoder, 0);
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

		/**
		 * Cross-thread copy called by service entrypoint
		 */
		void queue(Dataspace_capability fb_cap, Mode const mode)
		{

			/* drop this frame if there is encoding in progress */
			if (_image != nullptr)
				return;

			Mutex::Guard guard(_mutex);

			/* TODO: attach/detach each capture? */
			Genode::Attached_dataspace fb_ds(_env.rm(), fb_cap);
			if (fb_ds.size() < mode.num_bytes()) {
				Genode::error("invalid framebuffer for capture");
				return;
			}

			RGBA row[mode.area.w];

			_image = flif_create_image(mode.area.w, mode.area.h);
			if (!_image) {
				Genode::error("failed to create image buffer");
				return;
			}

			using Pixel = Pixel_rgb888;
			Pixel const *pixels = fb_ds.local_addr<Pixel const>();

			for (unsigned y = 0; y < mode.area.h; y++) {
				for (unsigned x = 0; x < mode.area.w; x++) {
					Pixel const px = pixels[y*mode.area.w+x];
					row[x] = RGBA { (uint8_t)px.r(),
					                (uint8_t)px.g(),
					                (uint8_t)px.b(),
					                255 };
				}
				flif_image_write_row_RGBA8(_image, y, row, sizeof(row));
			}

			capture_pending = false;
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
		Flif_capture::Encoder        &_encoder;

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

		void refresh(Framebuffer::Rect rect) override
		{
			_parent.refresh(rect);
			if (_encoder.capture_pending)
				_encoder.queue(_dataspace, _mode);
		}

		void sync_sigh(Genode::Signal_context_capability sigh) override
		{
			_parent.sync_sigh(sigh);
		}

		void sync_source(Genode::Session_label const &) override { }

		Blit_result blit(Framebuffer::Blit_batch const &batch) override
		{
			return _parent.blit(batch);
		}

		void panning(Framebuffer::Point pos) override
		{
			_parent.panning(pos);
		}
};


class Flif_capture::Main : private Input::Session_component::Action
{
	private:

		Genode::Env &_env;

		/*
		 * Allocate an additional entrypoint to
		 * separate Libc and RPC activity. Offset the
		 * affinity by one to place the entrypoint on
		 * the next processor core.
		 *
		 * TODO: place the initial entrypoint instead
		 * to avoid cross-CPU RPC?
		 */
		enum { SERVICE_STACK_SIZE = sizeof(addr_t) << 12 };
		Entrypoint _service_ep {
			_env, SERVICE_STACK_SIZE, "service-entrypoint",
			_env.cpu().affinity_space().location_of_index(1)
		};

		Flif_capture::Encoder   _encoder      { _env };
		Framebuffer::Connection _parent_fb    { _env, Mode { } };
		Input::Connection       _parent_input { _env };

		Framebuffer_session_component _fb_session {
			_env, _parent_fb, _encoder };

		Input::Session_component _input_session {
			_env.ep(), _env.ram(), _env.rm(), *this };

		void exclusive_input_requested(bool) override { };

		Framebuffer::Session_capability _fb_cap { _service_ep.manage(_fb_session) };

		Genode::Signal_handler<Main> _input_handler {
			_service_ep, *this, &Main::_handle_input };

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
			_service_ep.manage(_fb_session) };

		Genode::Static_root<Input::Session> _input_root { _input_session.cap() };

	public:

		/**
		 * Constructor
		 */
		Main(Libc::Env &env) : _env(env)
		{
			/* register input handler */
			_parent_input.sigh(_input_handler);

			_input_session.event_queue().enabled(true);

			env.parent().announce(_service_ep.manage(_fb_root));
			env.parent().announce(_service_ep.manage(_input_root));

			Genode::log("--- screenshot capture key is ", Input::key_name(_capture_code), " ---");
		}

		void spin() { _encoder.entry(); }
};


/***************
 ** Component **
 ***************/

void Libc::Component::construct(Libc::Env &env)
{
		static Flif_capture::Main inst(env);
		inst.spin();
}
