/*
 * \brief  MP3 audio decoder
 * \author Emery Hemingway
 * \date   2018-03-24
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/*
 * TODO:
 * - Metadata report
 * - Configure the mpg123 volume control and equalizer
 * - Optimize buffer sizes
 */

/* Genode includes */
#include <os/static_root.h>
#include <libc/component.h>
#include <audio_out_session/connection.h>
#include <terminal_session/connection.h>
#include <base/attached_rom_dataspace.h>
#include <base/attached_ram_dataspace.h>
#include <base/sleep.h>

/* Mpg123 includes */
#include <stdlib.h>
#include <sys/types.h>
#include <mpg123.h>

/* local includes */
#include <magic_ring_buffer.h>

namespace Mp3_audio_sink {

	enum { LEFT, RIGHT, NUM_CHANNELS };

	enum {
		FEED_POOL_SIZE = 2,
		CLIENT_BUFFER_SIZE = 1 << 14, /* 16 KiB */
	};

	enum {
		AUDIO_OUT_BUFFER_SIZE = NUM_CHANNELS
			* Audio_out::QUEUE_SIZE
			* Audio_out::PERIOD
			* Audio_out::SAMPLE_SIZE
	};

	using namespace Genode;

	struct Decoder;

	class Terminal_component;
	struct Main;
}


struct Mp3_audio_sink::Decoder
{
	template <typename FUNC>
	static void for_each_channel(FUNC const &func) {
		for (int i = 0; i < NUM_CHANNELS; ++i) func(i); }

	Genode::Env &_env;

	Attached_rom_dataspace _config_rom { _env, "config" };

	Audio_out::Connection _out_left  { _env, "left",  true, true };
	Audio_out::Connection _out_right { _env, "right", false, false };
	Audio_out::Connection *_out[NUM_CHANNELS];

	void die_mpg123(mpg123_handle *mh, char const *msg)
	{
		int code = mpg123_errcode(mh);
		Genode::error(msg, ", ", mpg123_strerror(mh));

		mpg123_close(mh);
		mpg123_delete(mh);
		mpg123_exit();

		_env.parent().exit(code);
		Genode::sleep_forever();
	}

	mpg123_handle *create_mpg123_handle()
	{
		mpg123_handle *mh = NULL;

		int err = mpg123_init();
		if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
			Genode::error("Mpg123 setup failed, ", mpg123_plain_strerror(err));

		mpg123_param(mh, MPG123_FEEDPOOL, FEED_POOL_SIZE, 0);
		mpg123_param(mh, MPG123_FEEDBUFFER, CLIENT_BUFFER_SIZE, 0);

		/* Set mpg123 output format to match Audio_out exactly */
		mpg123_param(mh, MPG123_FORCE_RATE,
		             Audio_out::SAMPLE_RATE, Audio_out::SAMPLE_RATE);
		mpg123_format_none(mh);
		if (mpg123_format(mh, Audio_out::SAMPLE_RATE,
		                  MPG123_STEREO, MPG123_ENC_FLOAT_32) != MPG123_OK)
			die_mpg123(mh, "Audio_out format is unsupported");

		if (mpg123_open_feed(mh) != MPG123_OK)
			die_mpg123(mh, "mpg123 feeder mode failed");
		return mh;
	}

	mpg123_handle *_mh = create_mpg123_handle();

	/* last error code logged */
	int _mh_err = MPG123_OK;

	/**
	 * Use half of the available RAM as buffer
	 */
	size_t _pcm_buffer_size()
	{
		size_t n = _env.pd().avail_ram().value / 2;
		if (n > (1<<20))
			log("internal buffer size is ", n>>20, "MiB");
		else if (n > (1<<10))
			log("internal buffer size is ", n>>10, "KiB");
		else
			log("internal buffer size is ", n, " bytes");
		return n;
	}

	Magic_ring_buffer<float> _pcm { _env, _pcm_buffer_size() };

	void _log_error()
	{
		int code = mpg123_errcode(_mh);
		if (code != MPG123_OK && _mh_err != code) {
			_mh_err = code;
			error(mpg123_plain_strerror(code));
		}
	}

	Genode::size_t feedbuffer_size()
	{
		long value = 0;
		double fvalue = 0;
		if (mpg123_getparam(_mh, MPG123_FEEDBUFFER, &value, &fvalue))
			die_mpg123(_mh, "failed to get feed buffer size");
		return value;
	}

	void submit_audio();

	/**
	 * Process client data, blocks until all data is consumed.
	 */
	void process(unsigned char const *src, size_t num_bytes)
	{
		/* TODO: patch mpgg123 not to use stdio for printing errors */

		Libc::with_libc([&] () {

			/* feed client data into mpg123 buffer */
			if (mpg123_feed(_mh, src, num_bytes))
				die_mpg123(_mh, "failed to feed");

			::off_t num = 0;
			unsigned char *audio = nullptr;
			size_t bytes = 0;

			while (mpg123_decode_frame(_mh, &num, &audio, &bytes) == MPG123_OK) {
				Genode::size_t const samples = bytes / Audio_out::SAMPLE_SIZE;

				while (_pcm.write_avail() < samples) {
					/* submit audio blocks for packet allocation */
					submit_audio();
				}

				Genode::memcpy(_pcm.write_addr(), audio, bytes);
				_pcm.fill(samples);

			}

			if (mpg123_errcode(_mh) != MPG123_ERR_READER
			 || mpg123_errcode(_mh) != MPG123_OK)
				_log_error();
		});
	}

	Io_signal_handler<Decoder> _progress_handler {
		_env.ep(), *this, &Decoder::submit_audio };

	Signal_handler<Decoder> _config_handler {
		_env.ep(), *this, &Decoder::_handle_config };

	void _handle_config()
	{
		_config_rom.update();
		Xml_node const config = _config_rom.xml();

		enum { EQ_COUNT = 32 };

		mpg123_reset_eq(_mh);
		config.for_each_sub_node("eq", [&] (Xml_node const &node) {
			unsigned band = node.attribute_value("band", 32U);
			double value = node.attribute_value("value", 0.0);
			if (band < EQ_COUNT && value != 0.0) {
				mpg123_eq(_mh, MPG123_LR , band, value);
				log("EQ ", band, ": ", mpg123_geteq(_mh, MPG123_LR , band));
			}
		});

		double volume = 0.5;
		config.for_each_sub_node("volume", [&] (Xml_node const &node) {
			volume = node.attribute_value("linear", volume); });
		mpg123_volume(_mh, 0.5);
	}

	Decoder(Genode::Env &env) : _env(env)
	{
		_out[LEFT]  = &_out_left;
		_out[RIGHT] = &_out_right;
		_out_left.progress_sigh(_progress_handler);
		_config_rom.sigh(_config_handler);
		_handle_config();
	}
};


void Mp3_audio_sink::Decoder::submit_audio()
{
	using namespace Audio_out;

	enum { STEREO_PERIOD = Audio_out::PERIOD*NUM_CHANNELS };

	if (_out_left.stream()->empty()) {
		for_each_channel([&] (int const c) {
			_out[c]->start(); });
		log("Audio_out streams started");
	}

	while (_pcm.read_avail() > STEREO_PERIOD) {
		Audio_out::Packet *p[NUM_CHANNELS];

		while (true) {
			try { p[LEFT] = _out[LEFT]->stream()->alloc(); break; }
			catch (Audio_out::Stream::Alloc_failed) {
				_out[LEFT]->wait_for_alloc(); }
		}

		unsigned const ppos = _out[LEFT]->stream()->packet_position(p[LEFT]);
		p[RIGHT] = _out[RIGHT]->stream()->get(ppos);

		float const *content = _pcm.read_addr();

		/* copy channel contents into sessions */
		for (unsigned i = 0; i < STEREO_PERIOD; i += NUM_CHANNELS) {
			for_each_channel([&] (int const c) {
				p[c]->content()[i/NUM_CHANNELS] = content[i+c]; });
		}

		for_each_channel([&] (int const c) {
			 _out[c]->submit(p[c]); });
		_pcm.drain(STEREO_PERIOD);
	}

	if (_out_left.stream()->empty()) {
		log("Audio_out queue underrun, stopping stream");
		for_each_channel([&] (int const c) {
			_out[c]->stop(); });
	}
}


class Mp3_audio_sink::Terminal_component :
	public Rpc_object<Terminal::Session, Terminal_component>
{
	private:

		Decoder &_decoder;

		Genode::Attached_ram_dataspace _io_buffer;

	public:

		Terminal_component(Genode::Env &env, Decoder &decoder)
		:
			_decoder(decoder),
			_io_buffer(env.ram(), env.rm(), _decoder.feedbuffer_size())
		{ }


		/********************************
		 ** Terminal session interface **
		 ********************************/

		Genode::Dataspace_capability _dataspace() {
			return _io_buffer.cap(); }

		Size size() { return Size(0, 0); }

		bool avail() { return false; }

		Genode::size_t read(void *, Genode::size_t) { return 0; }
		Genode::size_t _read(Genode::size_t dst_len) { return 0; }

		Genode::size_t write(void const *, Genode::size_t) { return 0; }
		Genode::size_t _write(Genode::size_t num_bytes)
		{
			/* sanitize argument */
			num_bytes = Genode::min(num_bytes, _io_buffer.size());

			/* copy to decoder */
			_decoder.process(
				_io_buffer.local_addr<unsigned char>(), num_bytes);

			return num_bytes;
		}

		void connected_sigh(Genode::Signal_context_capability cap) {
			Genode::Signal_transmitter(cap).submit(); }

		void read_avail_sigh(Genode::Signal_context_capability) { }

		void size_changed_sigh(Genode::Signal_context_capability) { }
};


struct Mp3_audio_sink::Main
{
	Genode::Env &_env;

	Decoder _decoder { _env };

	Terminal_component _terminal { _env, _decoder };

	Static_root<Terminal::Session> _terminal_root {
		_env.ep().manage(_terminal) };

	Main(Libc::Env &env) : _env(env)
	{
		env.parent().announce(env.ep().manage(_terminal_root));
	}
};


/***************
 ** Component **
 ***************/

void Libc::Component::construct(Libc::Env &env) {
	static Mp3_audio_sink::Main _main(env); }
