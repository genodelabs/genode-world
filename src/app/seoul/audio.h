/*
 * \brief  Audio handling
 * \author Alexander Boettcher
 * \date   2022-06-19
 */

/*
 * Copyright (C) 2022-2024 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Seoul VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <play_session/connection.h>
#include <timer_session/connection.h>

#include <nul/motherboard.h>


namespace Seoul {
	class Audio;
}

class Seoul::Audio
{
	private:

		Motherboard &_mb;

		Play::Connection  _left;
		Play::Connection  _right;
		Play::Time_window _time_window { };

		Genode::Mutex     _mutex { };
		Timer::Connection _timer;

		unsigned _samples { };
		unsigned _data_id { 1 };

		unsigned _frames_per_period { };
		unsigned _period_us         { };

		bool _audio_running { false };
		bool _audio_verbose { false };

		Genode::Signal_handler<Audio> const _timer_sigh;

		enum { MAX_CACHED_FRAMES = 2500 };

		float _left_data [MAX_CACHED_FRAMES] { };
		float _right_data[MAX_CACHED_FRAMES] { };

		void _audio_out();

		void _audio_start()
		{
			if (_audio_verbose)
				Genode::log(__func__, " ", _audio_running);

			if (_audio_running)
				return;

			_audio_running = true;
		}

		void _audio_stop()
		{
			if (_audio_verbose)
				Genode::log(__func__, " ", _audio_running);

			if (!_audio_running)
				return;

			_audio_running = false;

			_samples = 0;

			_time_window = { };

			_right.stop();
			_left .stop();
		}

		enum { CHANNELS = 2 };

		unsigned max_samples() const { return _frames_per_period * CHANNELS; }

		/*
		 * Noncopyable
		 */
		Audio(Audio const &);
		Audio &operator = (Audio const &);

	public:

		Audio(Genode::Env &, Motherboard &);

		bool receive(MessageAudio &);

		void verbose(bool onoff) { _audio_verbose = onoff; }
};

#endif /* _AUDIO_H_ */
