/*
 * \brief  Retro_frontend audio
 * \author Emery Hemingway
 * \date   2016-12-13
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__AUDIO_H_
#define _RETRO_FRONTEND__AUDIO_H_

/* Genode includes */
#include <audio_out_session/connection.h>
#include <base/attached_ram_dataspace.h>
#include <base/thread.h>
#include <util/reconstructible.h>

#include "core.h"

namespace Retro_frontend {

	template <typename TYPE, Genode::size_t CAPACITY>
	struct Ring_buffer;
	struct Stereo_out;

	enum { LEFT, RIGHT, NUM_CHANNELS };

	enum {
		SHIFT = 16,
		SHIFT_ONE = 1 << SHIFT
	};

	unsigned audio_shift_factor = SHIFT_ONE;
	static unsigned audio_input_period = 0;
}


/**
 * Ring buffer using virtual addressing and Buffer Overflow™ technology
 */
template <typename TYPE, Genode::size_t CAPACITY>
struct Retro_frontend::Ring_buffer : Genode::Lock
{
	enum { BUFFER_SIZE = sizeof(TYPE)*CAPACITY };

	Genode::addr_t map_first;
	Genode::addr_t map_second;

	TYPE *buffer;

	Genode::size_t wpos = 0;
	Genode::size_t rpos = 0;

	Genode::Ram_dataspace_capability buffer_ds = genv->ram().alloc(BUFFER_SIZE);

	Ring_buffer()
	{
		{
			/* a hack to find the right sized void in the address space */
			Genode::Attached_ram_dataspace filler(genv->ram(), genv->rm(), BUFFER_SIZE*2);
			map_first = (Genode::addr_t)filler.local_addr<TYPE>();
		}

		map_second = map_first+BUFFER_SIZE;

		/* attach the buffer in two consecutive regions */
		map_first = genv->rm().attach_at(buffer_ds, map_first, BUFFER_SIZE);
		map_second = genv->rm().attach_at(buffer_ds, map_second,  BUFFER_SIZE);
		if ((map_first+BUFFER_SIZE) != map_second) {
			Genode::error("failed to map ring buffer to consecutive regions");
			throw Genode::Exception();
		}

		buffer = (TYPE *)map_first;
	}

	~Ring_buffer()
	{
		genv->rm().detach(map_second);
		genv->rm().detach(map_first);
		genv->ram().free(buffer_ds);
	}

	Genode::size_t read_avail() const
	{
		if (wpos > rpos) return wpos - rpos;
		else             return (wpos - rpos + CAPACITY) % CAPACITY;
	}

	Genode::size_t write_avail() const
	{
		if      (wpos > rpos) return ((rpos - wpos + CAPACITY) % CAPACITY) - 2;
		else if (wpos < rpos) return rpos - wpos;
		else                  return CAPACITY - 2;
	}

	Genode::size_t write(TYPE const *src, Genode::size_t len)
	{
		using Genode::size_t;

		len = Genode::min(len, write_avail());

		TYPE *wbuf = &buffer[wpos];

		for (size_t i = 0; i < len; ++i)
			wbuf[i] = src[i];

		wpos = (wpos + len) % CAPACITY;
		return len;
	}

	void drain_period(float *periodl, float *periodr)
	{
		using namespace Genode;

		size_t const avail = read_avail();

		if (avail < audio_input_period*2) {
			Genode::memset(periodl, 0x00, sizeof(float)*Audio_out::PERIOD);
			Genode::memset(periodr, 0x00, sizeof(float)*Audio_out::PERIOD);
			return;
		}

		int16_t *rbuf = &buffer[rpos];

		size_t buf_off;
		for (size_t pkt_off = 0; pkt_off < Audio_out::PERIOD; ++pkt_off)
		{
			buf_off = 2*((pkt_off*audio_shift_factor)>>SHIFT);
			periodl[pkt_off] = rbuf[buf_off+0] / 32768.0f;
			periodr[pkt_off] = rbuf[buf_off+1] / 32768.0f;
		}

		rpos = (rpos+audio_input_period*2) % CAPACITY;
	}
};


/**
 * Thread for converting samples at the core sample rate
 * to the native sample rate
 */
struct Retro_frontend::Stereo_out : Genode::Thread
{
	Audio_out::Connection  left;
	Audio_out::Connection right;

	Ring_buffer<int16_t, 4096> buffer;

	Genode::Lock run_lock { Genode::Lock::LOCKED };

	bool running = false;

	void entry() override;

	Stereo_out()
	:
		Genode::Thread(*genv, "audio-sync", 8*1024,
		               genv->cpu().affinity_space().location_of_index(1),
		               Weight(Genode::Cpu_session::Weight::DEFAULT_WEIGHT-1),
		               genv->cpu()),
		left( *genv,  "left", false, true),
		right(*genv, "right", false, true)
	{
		start();
	}

	void start_stream()
	{
		running = true;
		run_lock.unlock();
	}

	void stop_stream()
	{
		running = false;
	}
};


static Genode::Constructible<Retro_frontend::Stereo_out> stereo_out;


void audio_sample_noop(int16_t left, int16_t right) { }


/* not called in pratice */
void audio_sample_callback(int16_t left, int16_t right)
{
	stereo_out->buffer.lock();
	stereo_out->buffer.write(&left, 1);
	stereo_out->buffer.write(&right, 1);
	stereo_out->buffer.unlock();
}


size_t audio_sample_batch_noop(const int16_t *data, size_t frames) { return 0; }


size_t audio_sample_batch_callback(const int16_t *data, size_t frames)
{
	Genode::Lock::Guard guard(stereo_out->buffer);
	return stereo_out->buffer.write(data, frames*2)/2;
}


void Retro_frontend::Stereo_out::entry()
{
	Audio_out::Packet *p[NUM_CHANNELS];

	for (;;) {
		run_lock.lock();

		p[LEFT] = left.stream()->next();

		/* stuff the buffer a bit */
		for (auto i = 0; i < 4; ++i)
			p[LEFT] = left.stream()->next(p[LEFT]);

		left.start();
		right.start();

		while (running) {
			unsigned const ppos =  left.stream()->packet_position(p[LEFT]);
			p[RIGHT]            = right.stream()->get(ppos);

			buffer.lock();
			buffer.drain_period(p[LEFT]->content(), p[RIGHT]->content());
			buffer.unlock();

			left.submit(p[LEFT]);
			right.submit(p[RIGHT]);

			p[LEFT] = left.stream()->next(p[LEFT]);

			left.wait_for_progress();
		}

		/* empty the buffer to resync when started again */
		while (!p[LEFT]->played())
			left.wait_for_progress();

		left.stop();
		right.stop();
	}
}

#endif
