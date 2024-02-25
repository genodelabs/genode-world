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


#include "audio.h"


Seoul::Audio::Audio(Genode::Env &env, Motherboard &mb)
:
	_mb(mb),
	_left (env, "left" ),
	_right(env, "right"),
	_timer(env),
	_timer_sigh(env.ep(), *this, &Audio::_audio_out)
{
	_timer.sigh(_timer_sigh);
}


void Seoul::Audio::_audio_out()
{
	unsigned continue_id = 0;

	{
		Genode::Mutex::Guard guard(_mutex);

		if (_samples < max_samples())
			return;

		if (_audio_running) {
			_time_window = _left.schedule_and_enqueue(_time_window, { _period_us }, [&] (auto &submit) {
				for (auto i = 0u; i < _frames_per_period; i++)
					submit(_left_data[i]);
			});

			_right.enqueue(_time_window, [&] (auto &submit) {
				for (auto i = 0u; i < _frames_per_period; i++)
					submit(_right_data[i]);
			});
		}

		_samples = 0;

		continue_id = _data_id;

		_data_id ++;
	}

	MessageAudio msg(MessageAudio::Type::AUDIO_CONTINUE_TX, continue_id);
	_mb.bus_audio.send(msg);
}

bool Seoul::Audio::receive(MessageAudio &msg)
{
	if ((msg.type == MessageAudio::Type::AUDIO_CONTINUE_TX) ||
	    (msg.type == MessageAudio::Type::AUDIO_DRAIN_TX))
		return false;

	unsigned const guest_sample_size = sizeof(float);

	Genode::Mutex::Guard guard(_mutex);

	switch (msg.type) {
	case MessageAudio::Type::AUDIO_START:
		_audio_start();

		if (msg.period_bytes()) {
			_frames_per_period = msg.period_bytes() / CHANNELS / guest_sample_size;

			if (_frames_per_period > MAX_CACHED_FRAMES)
				_frames_per_period = MAX_CACHED_FRAMES;

			_period_us = _frames_per_period * 1'000'000u / 44'100;
		}

		if (false)
			Genode::log("start ", msg.period_bytes(), " -> ",
			            _frames_per_period, " frames -> ", _period_us, " us");

		_timer.trigger_periodic(_period_us);

		return true;
	case MessageAudio::Type::AUDIO_STOP:
		_audio_stop();
		_timer.trigger_periodic(0);

		if (false)
			Genode::log("stop");

		return true;
	case MessageAudio::Type::AUDIO_OUT:
		break;
	default:
		Genode::warning("audio: unsupported type ", unsigned(msg.type));
		return true;
	}

	/* case MessageAudio::Type::AUDIO_OUT */

	if (_samples >= max_samples()) {
		msg.id = _data_id;
		return true;
	}

	if (msg.consumed >= msg.size)
		Logging::panic("audio corrupt consumed=%u msg.size=%u entry\n",
		               msg.consumed, msg.size);

	if (msg.consumed >= msg.size)
		Logging::panic("audio corrupt consumed=%u msg.size=%u loop\n",
		               msg.consumed, msg.size);

	uintptr_t const data_ptr = msg.data + msg.consumed;

	auto samples = (((msg.size - msg.consumed) / guest_sample_size) > max_samples())
	             ? max_samples() : ((msg.size - msg.consumed) / guest_sample_size);

	if (samples > max_samples() - _samples)
		samples = max_samples() - _samples;

	if (msg.consumed >= msg.size)
		Logging::panic("audio corrupt consumed=%u msg.size=%u\n",
		               msg.consumed, msg.size);

	for (unsigned i = 0; i < samples / CHANNELS; i++) {
		unsigned const sample_pos = _samples / CHANNELS + i;

		float f_left  = ((float *)data_ptr)[i * CHANNELS + 0];
		float f_right = ((float *)data_ptr)[i * CHANNELS + 1];


		if (f_left >  1.0f) f_left =  1.0f;
		if (f_left < -1.0f) f_left = -1.0f;

		if (f_right >  1.0f) f_right =  1.0f;
		if (f_right < -1.0f) f_right = -1.0f;

		_left_data [sample_pos] = f_left;
		_right_data[sample_pos] = f_right;
	}

	msg.consumed += samples * guest_sample_size;
	msg.id        = _data_id;

	_samples += samples;

	return true;
}
