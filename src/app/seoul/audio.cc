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


#include "audio.h"


Seoul::Audio::Audio(Genode::Env &env, Synced_motherboard &motherboard,
                    Motherboard &unsynch_motherboard)
:
	_sigh_processed(env.ep(), *this, &Audio::_audio_out),
	_motherboard(motherboard),
	_unsynchronized_motherboard(unsynch_motherboard),
	_audio_left (env, "front left",  false, false),
	_audio_right(env, "front right", false, false)
{
	/* server processed one packet */
	_audio_left.progress_sigh(_sigh_processed);
}


void Seoul::Audio::_audio_out()
{
	Genode::Mutex::Guard guard(_mutex);

	if (!_pkg_head.valid())
		return;

	do {
		Audio_out::Packet *pkg = _audio_left.stream()->get(_pkg_head.value);
		if (!pkg || !pkg->played())
			break;

		MessageAudio msg(MessageAudio::Type::AUDIO_CONTINUE_TX, _pkg_head.value);

		VMM_MEMORY_BARRIER;

		_mutex.release();

		_motherboard()->bus_audio.send(msg);

		_mutex.acquire();

		VMM_MEMORY_BARRIER;

		if (msg.type == MessageAudio::Type::AUDIO_DRAIN_TX) {
			/* already drained, we just stop and invalidate */
			_audio_stop();
			_pkg_head.invalidate();
			return;
		}

		if (_pkg_head.value == _pkg_tail.value) {
			_pkg_head.invalidate();
			break;
		} else
			_pkg_head.advance();
	} while (true);
}


bool Seoul::Audio::receive(MessageAudio &msg)
{
	if ((msg.type == MessageAudio::Type::AUDIO_CONTINUE_TX) ||
	    (msg.type == MessageAudio::Type::AUDIO_DRAIN_TX))
		return false;

	Genode::Mutex::Guard guard(_mutex);

	switch (msg.type) {
	case MessageAudio::Type::AUDIO_START:
		return true;
	case MessageAudio::Type::AUDIO_STOP:
		_audio_stop();
		return true;
	case MessageAudio::Type::AUDIO_OUT:
		break;
	default:
		Genode::warning("audio: unsupported type ", unsigned(msg.type));
		return true;
	}

	/* MessageAudio::Type::AUDIO_OUT */

	_audio_start();

	if (msg.consumed >= msg.size)
		Logging::panic("audio corrupt consumed=%u msg.size=%u entry\n",
		               msg.consumed, msg.size);

	while (true) {

		#define USE_FLOAT

#ifdef USE_FLOAT
		auto const guest_sample_size = sizeof(float);
#else
		auto const guest_sample_size = sizeof(Genode::int16_t);
#endif

		enum { CHANNELS = 2 };

		if (msg.consumed >= msg.size)
			Logging::panic("audio corrupt consumed=%u msg.size=%u loop\n",
			               msg.consumed, msg.size);

		if (!p_left && _audio_left.stream()->full()) {
			/* under run case */
			Genode::log("reset/drain - stream full ?!");

			_audio_stop();
			_pkg_head.invalidate();

			msg.type = MessageAudio::Type::AUDIO_DRAIN_TX;
			return true;
		}

		if (!p_left) {
			try {
				p_left = _audio_left.stream()->alloc();
			} catch (...) {
				if (_audio_verbose)
					Genode::log("audio - allocation exception");
				return true;
			}
		}

		if (!p_left) {
			if (_audio_verbose)
				Genode::log("audio - no packet, should not happen");
			return true;
		}

		uintptr_t const data_ptr = msg.data + msg.consumed;

		auto const max_samples = Audio_out::PERIOD * CHANNELS;
		unsigned samples = (((msg.size - msg.consumed) / guest_sample_size) > max_samples)
		                 ? max_samples : ((msg.size - msg.consumed) / guest_sample_size);

		if (samples > max_samples - sample_offset)
			samples = max_samples - sample_offset;

		unsigned const pos         = _audio_left .stream()->packet_position(p_left);
		Audio_out::Packet *p_right = _audio_right.stream()->get(pos);

		if (msg.consumed >= msg.size)
			Logging::panic("audio corrupt consumed=%u msg.size=%u\n",
			               msg.consumed, msg.size);

		for (unsigned i = 0; i < samples / CHANNELS; i++) {
			unsigned sample_pos = sample_offset / CHANNELS + i;
#ifdef USE_FLOAT
			float f_left  = ((float *)data_ptr)[i*CHANNELS+0];
			float f_right = ((float *)data_ptr)[i*CHANNELS+1];
#else
			float f_left  = float(((Genode::int16_t *)data_ptr)[i*CHANNELS+0]) / 32768.0f;
			float f_right = float(((Genode::int16_t *)data_ptr)[i*CHANNELS+1]) / 32768.0f;
#endif

			if (f_left >  1.0f) f_left =  1.0f;
			if (f_left < -1.0f) f_left = -1.0f;

			if (f_right >  1.0f) f_right =  1.0f;
			if (f_right < -1.0f) f_right = -1.0f;

			p_left ->content()[sample_pos] = f_left;
			p_right->content()[sample_pos] = f_right;
		}

		msg.consumed += samples * guest_sample_size;

		msg.id = _audio_left.stream()->packet_position(p_left);
		_pkg_tail.value = msg.id;

		if (!_pkg_head.valid())
			_pkg_head = _pkg_tail;

		if ((sample_offset + samples) != max_samples) {
			sample_offset = (sample_offset + samples) % max_samples;
			/* delay packet until we get enough data */
			return true;
		}

		_audio_right.submit(p_right);
		_audio_left .submit(p_left);

		p_left = p_right = nullptr;
		sample_offset = 0;

		if (msg.consumed >= msg.size)
			break;
	}

	return true;
}
