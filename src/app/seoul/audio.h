/*
 * \brief  Audio handling
 * \author Alexander Boettcher
 * \date   2022-06-19
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Seoul VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <audio_out_session/connection.h>

#include "synced_motherboard.h"

namespace Seoul {
	class Audio;
}

class Seoul::Audio
{
	private:

		Genode::Signal_handler<Audio> const _sigh_processed;

		Genode::Mutex         _mutex { };

		Synced_motherboard   &_motherboard;
		Motherboard          &_unsynchronized_motherboard;

		Audio_out::Connection  _audio_left;
		Audio_out::Connection  _audio_right;

		Audio_out::Packet * p_left   { nullptr };
		unsigned            sample_offset { 0 };

		struct Pkg {
			unsigned value    { ~0U };
			bool valid()      { return value != ~0U; }
			void invalidate() { value = ~0U; }
			void advance()    { value = (value + 1) % Audio_out::QUEUE_SIZE; }
		};

		Pkg _pkg_head { };
		Pkg _pkg_tail { };

		bool _audio_running { false };
		bool _audio_verbose { false };

		void _audio_out();

		void _audio_start()
		{
			if (_audio_running)
				return;

			if (_audio_verbose)
				Genode::log(__func__);

			_audio_running = true;

			_pkg_head.invalidate();
			_pkg_tail.invalidate();

			_audio_left.stream()->reset();
			_audio_right.stream()->reset();

			_audio_left .start();
			_audio_right.start();
		}

		void _audio_stop()
		{
			if (!_audio_running)
				return;

			if (_audio_verbose)
				Genode::log(__func__);

			_audio_left .stop();
			_audio_right.stop();

			_audio_running = false;

			p_left = nullptr;
			sample_offset = 0;

			_audio_left .stream()->invalidate_all();
			_audio_right.stream()->invalidate_all();
		}

		/*
		 * Noncopyable
		 */
		Audio(Audio const &);
		Audio &operator = (Audio const &);

	public:

		Audio(Genode::Env &, Synced_motherboard &, Motherboard &);

		bool receive(MessageAudio &);

		void verbose(bool onoff) { _audio_verbose = onoff; }
};

#endif /* _AUDIO_H_ */
