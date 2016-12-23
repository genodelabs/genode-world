/*
 * \brief  Audio player
 * \author Josef Soentgen
 * \date   2015-11-19
 */

/*
 * Copyright (C) 2015-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <base/heap.h>
#include <base/printf.h>
#include <base/sleep.h>
#include <os/reporter.h>
#include <util/retry.h>
#include <util/xml_node.h>
#include <audio_out_session/connection.h>

/* local includes */
#include <list.h>
#include <ring_buffer.h>

extern "C" {
/*
 * UINT64_C is needed by libav headers
 *
 * Use the compiler's definition as fallback because the UINT64_C macro is only
 * defined in <machine/_stdint.h> when used with C.
 */
#ifndef UINT64_C
#define UINT64_C(c) __UINT64_C(c)
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libavresample/avresample.h>
}; /* extern "C" */


namespace Audio_player {
	class  Output;
	class  Playlist;
	class  Decoder;
	struct Main;

	typedef Util::Ring_buffer<64 * 1024> Frame_data;
	typedef Genode::String<1024>         Path;

	enum { LEFT, RIGHT, NUM_CHANNELS };
	enum { AUDIO_OUT_PACKET_SIZE = Audio_out::PERIOD * NUM_CHANNELS * sizeof(float) };
	enum { QUEUED_PACKET_THRESHOLD = 10 };
}

struct Audio_player::Output
{
	Audio_out::Connection  _left;
	Audio_out::Connection  _right;
	Audio_out::Connection *_out[NUM_CHANNELS];

	Audio_out::Packet     *_alloc_position;

	unsigned _packets_submitted = 0;

	template <typename FUNC>
	static void for_each_channel(FUNC const &func) {
		for (int i = 0; i < Audio_player::NUM_CHANNELS; i++) func(i); }

	/**
	 * Constructor
	 *
	 * \param sigh  progress signal handler
	 */
	Output(Genode::Signal_context_capability sigh)
	: _left("left", true, true), _right("right", false, false)
	{
		/*
		 * We only care about the left (first) channel and sync all other
		 * channels with it when needed
		 */
		_left.progress_sigh(sigh);

		_out[LEFT]  = &_left;
		_out[RIGHT] = &_right;
	}

	void start()
	{
		for_each_channel([&] (int const i) { _out[i]->start(); });
	}

	void stop()
	{
		for_each_channel([&] (int const i) { _out[i]->stop(); });
		_alloc_position = nullptr;
	}

	/**
	 * Return rounded estimate of packets per secound
	 */
	unsigned packets_per_sec() const { return Audio_out::SAMPLE_RATE / Audio_out::PERIOD; }

	/**
	 * Return the size of one output frame
	 */
	size_t frame_size() const { return AUDIO_OUT_PACKET_SIZE; }

	/**
	 * Return number of currently queued packets in Audio_out stream
	 */
	unsigned queued()
	{
		if (_alloc_position == nullptr) _alloc_position = _out[LEFT]->stream()->next();

		unsigned const packet_pos = _out[LEFT]->stream()->packet_position(_alloc_position);
		unsigned const play_pos   = _out[LEFT]->stream()->pos();
		unsigned const queued     = packet_pos < play_pos
		                            ? ((Audio_out::QUEUE_SIZE + packet_pos) - play_pos)
		                            : packet_pos - play_pos;
		return queued;
	}

	/**
	 * Fetch decoded frames from frame data buffer and fill Audio_out packets
	 */
	void drain_buffer(Frame_data &frame_data)
	{
		if (_alloc_position == nullptr) _alloc_position = _out[LEFT]->stream()->next();

		while (frame_data.read_avail() > (AUDIO_OUT_PACKET_SIZE)) {
			Audio_out::Packet *p[NUM_CHANNELS];

			p[LEFT] = _out[LEFT]->stream()->next(_alloc_position);

			unsigned const ppos = _out[LEFT]->stream()->packet_position(p[LEFT]);
			p[RIGHT]            = _out[RIGHT]->stream()->get(ppos);

			float tmp[Audio_out::PERIOD * NUM_CHANNELS];
			size_t const n = frame_data.read(tmp, sizeof(tmp));

			if (n != sizeof(tmp)) {
				Genode::warning("less frame data read than expected");
			}

			float *left_content  = p[LEFT]->content();
			float *right_content = p[RIGHT]->content();

			for (int i = 0; i < Audio_out::PERIOD; i++) {
					left_content[i]  = tmp[i * NUM_CHANNELS + LEFT];
					right_content[i] = tmp[i * NUM_CHANNELS + RIGHT];
			}

			for_each_channel([&] (int const i) { _out[i]->submit(p[i]); });

			_alloc_position = p[LEFT];

			_packets_submitted++;
		}
	}

	/**
	 * Return number of already submitted Audio_out packets
	 */
	unsigned packets_submitted() { return _packets_submitted; }
};


class Audio_player::Playlist
{
	public:

		struct Track : public Util::List<Track>::Element
		{
			Path     path;
			unsigned id = 0;

			Track() { }
			Track(char const *path, unsigned id) : path(path), id(id) { }
			Track(Track const &track) : path(track.path), id(track.id) { }

			bool valid() { return path.length(); }
		};

		template <typename FUNC>
		void for_each_track(FUNC const &func) {
			for (Track *t = _track_list.first(); t; t = t->next())
				func(*t);
		}

	private:

		Genode::Allocator &_alloc;
		Util::List<Track>  _track_list;
		Track             *_curr_track = nullptr;

		typedef enum { MODE_ONCE, MODE_REPEAT } Mode;
		Mode _mode;

		void _insert(char const *path, unsigned id)
		{
			try {
				Track *t = new (&_alloc) Track(path, id);
				_track_list.append(t);
			} catch (...) { Genode::warning("could not insert track"); }
		}

		void _remove_all()
		{
			while (Track *t = _track_list.first()) {
				_track_list.remove(t);
				Genode::destroy(&_alloc, t);
			}
		}

	public:

		Playlist(Genode::Allocator &alloc) : _alloc(alloc), _mode(MODE_ONCE) { }

		~Playlist() { _remove_all(); }

		/**
		 * Update playlist
		 */
		void update(Genode::Xml_node &node)
		{
			/* handle tracks */
			_remove_all();
			_curr_track = nullptr;

			unsigned count = 0;
			auto add_track = [&] (Genode::Xml_node &track) {
				try {
					Path path;
					track.attribute("path").value(&path);
					_insert(path.string(), ++count);
				} catch (...) { Genode::warning("invalid file node in playlist"); }
			};

			try {
				node.for_each_sub_node("track", add_track);
			} catch (...) { }

			/* handle playlist mode */
			_mode = MODE_ONCE;
			try {
				Genode::String<16> mode;
				node.attribute("mode").value(&mode);
				if (mode == "repeat") _mode = MODE_REPEAT;
			} catch (...) { }

			Genode::log("update playlist: ", count,
			            " tracks mode: ", _mode == MODE_ONCE ? "once" : "repeat");
		}

		/**
		 * Return track with given id
		 */
		Track get_track(unsigned id)
		{
			for (Track *t = _track_list.first(); t; t = t->next())
				if (t->id == id) {
					_curr_track = t;
					return *_curr_track;
				}

			return Track();
		}

		/**
		 * Return next track from playlist
		 */
		Track next_track()
		{
			_curr_track = _curr_track ? _curr_track->next()
			                          : _track_list.first();

			/* try to loop at the end of the playlist */
			if (_curr_track == nullptr && _mode == MODE_REPEAT) {
					_curr_track = _track_list.first();
			}

			return _curr_track ? *_curr_track : Track();
		}
};


class Audio_player::Decoder
{
	public:

		/*
		 * Thrown as exception if the decoder could not be initialized
		 */
		struct Not_initialized { };

		/*
		 * File_info contains metadata of the file that is currently decoded
		 */
		struct File_info : public Playlist::Track
		{
			typedef Genode::String<256> Artist;
			typedef Genode::String<256> Album;
			typedef Genode::String<256> Title;
			typedef Genode::String<16>  Track;

			Artist  artist;
			Album   album;
			Title   title;
			Track   track;
			int64_t duration;

			File_info(Playlist::Track const &playlist_track,
			           char const *artist, char const *album,
			           char const *title, char const *track,
			           int64_t duration)
			:
				Playlist::Track(playlist_track),
				artist(artist), album(album), title(title), track(track),
				duration(duration) { }
		};

	private:

		Genode::Allocator &_alloc;

		AVFrame         *_frame      = nullptr;
		AVFrame         *_conv_frame = nullptr;
		AVStream        *_stream     = nullptr;
		AVFormatContext *_format_ctx = nullptr;
		AVCodecContext  *_codec_ctx  = nullptr;
		AVPacket         _packet;

		AVAudioResampleContext *_avr = nullptr;

		unsigned _samples_decoded = 0;

		Playlist::Track const &_track;

		File_info *_track_info = nullptr;

		void _close()
		{
			avformat_close_input(&_format_ctx);
			av_free(_conv_frame);
			av_free(_frame);
		}

		/**
		 * Initialize libav once before it gets used
		 */
		static void init()
		{
			static bool registered = false;
			if (registered) { return; }

			/* initialise libav first so that all decoders are present */
			av_register_all();

			/* make libav quiet so we do not need stderr access */
			av_log_set_level(AV_LOG_QUIET);

			registered = true;
		}

	public:

		/*
		 * Constructor
		 */
		Decoder(Genode::Allocator &alloc, Playlist::Track const &playlist_track)
		: _alloc(alloc), _track(playlist_track)
		{
			Decoder::init();

			_frame = av_frame_alloc();
			if (!_frame) throw Not_initialized();

			_conv_frame = av_frame_alloc();
			if (!_conv_frame) {
				av_free(_frame);
				throw Not_initialized();
			}

			int err = 0;
			err = avformat_open_input(&_format_ctx, _track.path.string(), NULL, NULL);
			if (err != 0) {
				Genode::error("could not open '", _track.path.string(), "'");
				av_free(_conv_frame);
				av_free(_frame);
				throw Not_initialized();
			}

			err = avformat_find_stream_info(_format_ctx, NULL);
			if (err < 0) {
				Genode::error("could not find the stream info");
				_close();
				throw Not_initialized();
			}

			for (unsigned i = 0; i < _format_ctx->nb_streams; ++i)
				if (_format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
					_stream = _format_ctx->streams[i];
					break;
				}

			if (_stream == nullptr) {
				Genode::error("could not find any audio stream");
				_close();
				throw Not_initialized();
			}

			_codec_ctx        = _stream->codec;
			_codec_ctx->codec = avcodec_find_decoder(_codec_ctx->codec_id);
			if (_codec_ctx->codec == NULL) {
				Genode::error("could not find decoder");
				_close();
				throw Not_initialized();
			}

			err = avcodec_open2(_codec_ctx, _codec_ctx->codec, NULL);
			if (err != 0) {
				Genode::error("could not open decoder");
				_close();
				throw Not_initialized();
			}

			_avr = avresample_alloc_context();
			if (!_avr) {
				_close();
				throw Not_initialized();
			}

			av_opt_set_int(_avr, "in_channel_layout",  AV_CH_LAYOUT_STEREO,     0);
			av_opt_set_int(_avr, "out_channel_layout", AV_CH_LAYOUT_STEREO,     0);
			av_opt_set_int(_avr, "in_sample_rate",     _codec_ctx->sample_rate, 0);
			av_opt_set_int(_avr, "out_sample_rate",    Audio_out::SAMPLE_RATE,  0);
			av_opt_set_int(_avr, "in_sample_fmt",      _codec_ctx->sample_fmt,  0);
			av_opt_set_int(_avr, "out_sample_fmt",     AV_SAMPLE_FMT_FLT,       0);

			if (avresample_open(_avr) < 0) {
				_close();
				throw Not_initialized();
			}

			_conv_frame->channel_layout = AV_CH_LAYOUT_STEREO;
			_conv_frame->sample_rate    = Audio_out::SAMPLE_RATE;
			_conv_frame->format         = AV_SAMPLE_FMT_FLT;

			av_init_packet(&_packet);

			/* extract metainformation */
			bool const is_vorbis = _codec_ctx->codec_id == AV_CODEC_ID_VORBIS;

			AVDictionary *md = is_vorbis ? _stream->metadata : _format_ctx->metadata;
			int const flags  = AV_DICT_IGNORE_SUFFIX;

			AVDictionaryEntry *artist = av_dict_get(md, "artist", NULL, flags);
			AVDictionaryEntry *album  = av_dict_get(md, "album", NULL, flags);
			AVDictionaryEntry *title  = av_dict_get(md, "title", NULL, flags);
			AVDictionaryEntry *track  = av_dict_get(md, "track", NULL, flags);

			_track_info = new (&_alloc) File_info(_track,
				artist ? artist->value : "", album ? album->value : "",
				title ? title->value : "", track ? track->value : "",
				_format_ctx->duration / 1000);
		}

		/**
		 * Destructor
		 */
		~Decoder()
		{
			avresample_close(_avr);
			avresample_free(&_avr);
			avcodec_close(_codec_ctx);
			avformat_close_input(&_format_ctx);
			av_free(_conv_frame);
			av_free(_frame);

			Genode::destroy(&_alloc, _track_info);
		}

		/**
		 * Dump format information - needs <libc stderr="/dev/log"> configuration
		 */
		void dump_info() const { av_dump_format(_format_ctx, 0, _track.path.string(), 0); }

		/**
		 * Return metainformation of the file
		 */
		File_info const & file_info() const { return *_track_info; }

		/**
		 * Return decoding progress in ms
		 */
		uint64_t progress() const
		{
			/*
			 * Until there is need for seeking support that is enough albeit
			 * inaccurate and do not get me started on the float abuse...
			 *
			 * XXX We could also count the Audio_out packets submitted per
			 * track to get a more accurate playback progress
			 * (see Out::packets_submitted()).
			 */
			return (float)_samples_decoded / _codec_ctx->sample_rate * 1000;
		}

		/**
		 * Fill frame data buffer with decoded frames
		 *
		 * \param frame_data reference to destination buffer
		 * \param min minimal number of bytes that have to be decoded at least
		 */
		int fill_buffer(Frame_data &frame_data, size_t min)
		{
			size_t written = 0;
			while (written < min
			       && (av_read_frame(_format_ctx, &_packet) == 0)) {

				if (_packet.stream_index == _stream->index) {
					int finished = 0;
					avcodec_decode_audio4(_codec_ctx, _frame, &finished, &_packet);

					if (finished) {

						if (avresample_convert_frame(_avr, _conv_frame, _frame) < 0) {
							Genode::error("could not resample frame");
							return 0;
						}

						void   const *data  = _conv_frame->extended_data[0];
						size_t const  bytes = _conv_frame->linesize[0];
						size_t const      s = 2 /* stereo */ * sizeof(float);

						_samples_decoded += bytes / s;

						written += frame_data.write(data, bytes);
					}
				}

				av_free_packet(&_packet);
			}

			return written;
		}
};


struct Audio_player::Main
{
	Genode::Env        &env;

	Genode::Heap        alloc { env.ram(), env.rm() } ;

	void handle_progress();

	Genode::Signal_handler<Main> progress_dispatcher = {
		env.ep(), *this, &Main::handle_progress };

	Output      output { progress_dispatcher };
	Frame_data  frame_data;
	Decoder    *decoder = nullptr;

	Playlist        playlist { alloc };
	Playlist::Track track;

	void scan_playlist();

	Genode::Reporter playlist_reporter { "playlist" };
	bool report_playlist = false;

	void handle_playlist();

	Genode::Signal_handler<Main> playlist_dispatcher = {
		env.ep(), *this, &Main::handle_playlist };

	Genode::Attached_rom_dataspace playlist_rom { "playlist" };

	bool is_paused  = false;
	bool is_stopped = false;

	bool state_changed = false;

	Genode::Reporter reporter { "current_track" };

	unsigned report_progress_interval = 0; /* in Audio_out packets */
	bool     report_progress          = false;
	unsigned packet_count             = 0;

	void report_track(Decoder const *d);

	Genode::Attached_rom_dataspace config_rom { env, "config" };

	void handle_config();

	Genode::Signal_handler<Main> config_dispatcher = {
		env.ep(), *this, &Main::handle_config };

	Main(Genode::Env &env) : env(env)
	{
		Genode::Signal_transmitter(config_dispatcher).submit();
		config_rom.sigh(config_dispatcher);

		Genode::Signal_transmitter(playlist_dispatcher).submit();
		playlist_rom.sigh(playlist_dispatcher);

		reporter.enabled(true);
		playlist_reporter.enabled(true);

		Genode::log("--- start Audio_player ---");
	}
};


void Audio_player::Main::scan_playlist()
{
	try {
		Genode::Reporter::Xml_generator xml(playlist_reporter, [&] () {
			playlist.for_each_track([&] (Playlist::Track const &t) {
				Decoder d(alloc, t);
				Decoder::File_info const &info = d.file_info();

				xml.node("track", [&] () {
					xml.attribute("id",       info.id);
					xml.attribute("path",     info.path);
					xml.attribute("artist",   info.artist);
					xml.attribute("album",    info.album);
					xml.attribute("title",    info.title);
					xml.attribute("track",    info.track);
					xml.attribute("duration", info.duration);
				});
			});
		});
	} catch (...) { Genode::warning("could not report playlist"); }
}


void Audio_player::Main::handle_playlist()
{
	playlist_rom.update();

	if (!playlist_rom.is_valid()) { return; }

	Genode::Xml_node node(playlist_rom.local_addr<char>(),
	                      playlist_rom.size());

	playlist.update(node);

	track = playlist.next_track();

	if (report_playlist) { scan_playlist(); }

	/* trigger playback because playlist has changed */
	handle_progress();
}


void Audio_player::Main::report_track(Decoder const *d)
{
	try {
		Genode::Reporter::Xml_generator xml(reporter, [&] () {
			/*
			 * There is no valid decoder, create empty report to notify
			 * agents.
			 */
			if (d == nullptr) { return; }

			Decoder::File_info const &info = d->file_info();
			xml.attribute("id",       info.id);
			xml.attribute("path",     info.path);
			xml.attribute("artist",   info.artist);
			xml.attribute("album",    info.album);
			xml.attribute("title",    info.title);
			xml.attribute("track",    info.track);
			xml.attribute("progress", d->progress());
			xml.attribute("duration", info.duration);

			char const *s = "playing";

			if (is_paused)  { s = "paused"; }
			if (is_stopped) { s = "stopped"; }

			xml.attribute("state", s);
		});
	} catch (...) { Genode::warning("could not report current track"); }
}


void Audio_player::Main::handle_progress()
{
	if (is_stopped) {
		Genode::destroy(&alloc, decoder);
		decoder    = nullptr;
		is_stopped = false;

		report_track(nullptr);
	}

	if (is_paused) { return; }

	/* do not bother, that is not the track you are looking for */
	if (!track.valid()) { return; }

	/* track is valid but there is no decoder yet */
	if (decoder == nullptr) {
		Genode::retry<Decoder::Not_initialized>(
		[&] {
			if (track.valid()) {
				decoder = new (&alloc) Decoder(alloc, track);
				report_track(decoder);
				packet_count = 0;
			}
		},
		[&] {
			/* in case it did not work try next track */
			track = playlist.next_track();
		});

		report_track(decoder);
	}

	/* update current track progress */
	if (report_progress
	    && decoder
	    && (++packet_count == report_progress_interval)) {
		report_track(decoder);
		packet_count = 0;
	}

	/* only decode and play if we are below the threshold */
	if (output.queued() < QUEUED_PACKET_THRESHOLD) {

		if (decoder && frame_data.read_avail() <= output.frame_size()) {

			size_t const req_size = output.frame_size();
			int const n = decoder->fill_buffer(frame_data, req_size);
			if (n == 0) {
				Genode::destroy(&alloc, decoder);
				decoder = nullptr;

				track = playlist.next_track();
				if (!track.valid()) {
					Genode::warning("reached end of playlist");
					report_track(nullptr);
				}
			}
		}

		if (frame_data.read_avail() >= AUDIO_OUT_PACKET_SIZE)
			output.drain_buffer(frame_data);
	}
}


void Audio_player::Main::handle_config()
{
	config_rom.update();

	if (!config_rom.valid()) { return; }

	Genode::Xml_node config(config_rom.local_addr<char>(), config_rom.size());

	/* handle playlist scan request */
	try {
		config.attribute("scan_playlist").has_value("yes");
		scan_playlist();
	} catch (...) { }

	/* handle state */
	bool state_changed = false;
	try {
		static Genode::String<16> last_state { "-" };

		Genode::String<16> state;
		config.attribute("state").value(&state);

		state_changed = state != last_state;

		if (state_changed) {
			if (state == "playing") {
				is_paused = false;
				output.start();
			}
			else if (state == "paused") {
				is_paused = true;
				output.stop();
			}
			else /*(state == "stopped")*/ {
				Genode::error("state == stopped, should not happen");
				is_stopped = true;
				is_paused  = true;
				output.stop();
			}

			last_state = state;
		}

		report_track(decoder);
	} catch (...) {
		/* if there is no state attribute we are stopped */
		Genode::warning("player state invalid, player is stopped");
		is_stopped = true;
		is_paused  = true;
		output.stop();
	}

	/* handle selected track */
	try {
		unsigned id = 0;
		config.attribute("selected_track").value(&id);
		if (id != track.id) {  /* XXX what happens if the playlist changed? */
			is_stopped = true; /* XXX do not abuse this flag */
			track = playlist.get_track(id);

			if (!track.valid()) {
				Genode::error("invalid track ", id, " selected");
			}
		}
	} catch (...) { }

	/* handle progress report */
	enum { DEFAULT_SEC = 5 };
	try {
		Genode::Xml_node node = config.sub_node("report");

		report_progress = node.attribute("progress").has_value("yes");
		unsigned v = node.attribute_value<unsigned>("interval", DEFAULT_SEC);
		report_progress_interval = output.packets_per_sec() * v;

		report_playlist = node.attribute("playlist").has_value("yes");
	} catch (...) {
		report_progress = false;
		report_progress_interval = output.packets_per_sec() * DEFAULT_SEC;
		report_playlist = false;
	}

	/* trigger playback because state might has changed */
	if (state_changed) { handle_progress(); }
}


/***************
 ** Component **
 ***************/

void Libc::Component::construct(Genode::Env &env)
{
	static Audio_player::Main main(env);
}
