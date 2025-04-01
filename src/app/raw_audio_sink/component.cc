/*
 * \brief  Raw audio terminal sink
 * \author Emery Hemingway
 * \date   2018-02-10
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <audio_out_session/connection.h>
#include <terminal_session/connection.h>
#include <os/static_root.h>
#include <base/attached_ram_dataspace.h>
#include <base/component.h>

/* local includes */
#include <magic_ring_buffer.h>

namespace Raw_audio {
	using namespace Genode;

	enum { LEFT, RIGHT, NUM_CHANNELS };

	enum { AUDIO_OUT_BUFFER_SIZE =
		Audio_out::QUEUE_SIZE
			* Audio_out::PERIOD
			* Audio_out::SAMPLE_SIZE
			* NUM_CHANNELS };

	struct Sink;
	class Terminal_component;
	struct Main;
}


struct Raw_audio::Sink
{
	Sink(Sink const &);
	Sink &operator = (Sink const &);

	template <typename FUNC>
	static void for_each_channel(FUNC const &func) {
		for (int i = 0; i < NUM_CHANNELS; ++i) func(i); }

	Genode::Env &_env;

	Audio_out::Connection _out_left  { _env, "left",  true };
	Audio_out::Connection _out_right { _env, "right", false };
	Audio_out::Connection *_out[NUM_CHANNELS];

	size_t _buffer_size()
	{
		size_t n = AUDIO_OUT_BUFFER_SIZE;
		enum { RESERVATION = AUDIO_OUT_BUFFER_SIZE+((sizeof(addr_t)*16)<<10) };
		Ram_quota avail = _env.pd().avail_ram();
		if (avail.value > RESERVATION)
			n = avail.value - RESERVATION;
		return n;
	}

	Magic_ring_buffer<char> _pcm { _env, _buffer_size() };

	/**
	 * Process client data, blocks until all data is consumed.
	 */
	void process(char const *src, size_t num_bytes)
	{
		size_t remain = num_bytes;
		size_t off = 0;
		while (remain > 0) {
			if (_pcm.read_avail() < Audio_out::SAMPLE_SIZE)
				for_each_channel([&] (int const c) {
					 _out[c]->start(); });

			char *dst = _pcm.write_addr();
			size_t const n = min(remain, _pcm.write_avail());
			memcpy(dst, &src[off], n);
			_pcm.fill(n);
			off += n;
			remain -= n;

			if (_pcm.write_avail() < remain)
				_env.ep().wait_and_dispatch_one_io_signal();
		}
	}

	void submit_audio();

	Io_signal_handler<Sink> _progress_handler {
		_env.ep(), *this, &Sink::submit_audio };

	Sink(Genode::Env &env) : _env(env)
	{
		_out[LEFT]  = &_out_left;
		_out[RIGHT] = &_out_right;

		_out_left.progress_sigh(_progress_handler);
	}
};


void Raw_audio::Sink::submit_audio()
{
	using namespace Audio_out;

	enum {
		STEREO_PERIOD = Audio_out::PERIOD*2,
		STEREO_CHUNK = STEREO_PERIOD * Audio_out::SAMPLE_SIZE
	};

	while (_pcm.read_avail() >= STEREO_CHUNK) {

		Audio_out::Packet *p[NUM_CHANNELS];

		while (true) {
			try { p[LEFT] = _out[LEFT]->stream()->alloc(); break; }
			catch (Audio_out::Stream::Alloc_failed) {
				_out[LEFT]->wait_for_alloc(); }
		}

		unsigned const ppos = _out[LEFT]->stream()->packet_position(p[LEFT]);
		p[RIGHT] = _out[RIGHT]->stream()->get(ppos);

		auto *content = (float const *)_pcm.read_addr();

		/* copy channel contents into sessions */
		for (unsigned i = 0; i < STEREO_PERIOD; i += NUM_CHANNELS) {
			for_each_channel([&] (int const c) {
				p[c]->content()[i/NUM_CHANNELS] = content[i+c]; });
		}

		for_each_channel([&] (int const c) {
			 _out[c]->submit(p[c]); });
		_pcm.drain(STEREO_CHUNK);
	}

	if (_pcm.read_avail() < Audio_out::SAMPLE_SIZE)
		for_each_channel([&] (int const c) {
			 _out[c]->stop(); });
}


class Raw_audio::Terminal_component :
	public Rpc_object<Terminal::Session, Terminal_component>
{
	private:

		Sink &_sink;

		Genode::Attached_ram_dataspace _io_buffer;

	public:

		Terminal_component(Genode::Env &env, Sink &sink)
		: _sink(sink), _io_buffer(env.ram(), env.rm(), AUDIO_OUT_BUFFER_SIZE) { }


		/********************************
		 ** Terminal session interface **
		 ********************************/

		Genode::Dataspace_capability _dataspace()
		{
			return _io_buffer.cap();
		}

		Size size() override { return Size(0, 0); }

		bool avail() override { return false; }

		Genode::size_t read(void *, Genode::size_t) override { return 0; }
		Genode::size_t _read(Genode::size_t) { return 0; }

		Genode::size_t write(void const *, Genode::size_t) override { return 0; }
		Genode::size_t _write(Genode::size_t num_bytes)
		{
			/* sanitize argument */
			num_bytes = Genode::min(num_bytes, _io_buffer.size());

			/* copy to sink */
			_sink.process(_io_buffer.local_addr<char>(), num_bytes);

			return num_bytes;
		}

		void connected_sigh(Genode::Signal_context_capability cap) override {
			Genode::Signal_transmitter(cap).submit(); }

		void read_avail_sigh(Genode::Signal_context_capability) override { }

		void size_changed_sigh(Genode::Signal_context_capability) override { }
};


struct Raw_audio::Main
{
	Genode::Env &_env;

	Sink _sink { _env };

	Terminal_component _terminal { _env, _sink };

	Static_root<Terminal::Session> _terminal_root {
		_env.ep().manage(_terminal) };

	Main(Genode::Env &env) : _env(env)
	{
		env.parent().announce(env.ep().manage(_terminal_root));
	}
};


/***************
 ** Component **
 ***************/

void Component::construct(Genode::Env &env) {
	static Raw_audio::Main _main(env); }
