/*
 * \brief   Block shredder
 * \author  Emery Hemingway
 * \date    2017-08-26
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/**
 * TODO:
 * - Progress report
 */

/* Genode includes */
#include <block_session/connection.h>
#include <timer_session/connection.h>
#include <base/heap.h>
#include <base/component.h>
#include <base/sleep.h>

/* Jitterentropy includes */
#include <jitterentropy.h>

/* RDRAND utility */
#include <world/rdrand.h>

/* PCG includes */
#include <pcg_variants.h>


namespace Blk_shred {
	using namespace Genode;
	using namespace Block;
	struct Main;

	enum {
		/* XXX: part_blk has a fixed backend buffer that limits our packet size */
		PKT_SIZE = 1 << 20,
		PKT_BUF_SIZE = PKT_SIZE*2 + (32<<10)
	};

	uint64_t pcg_init[2] PCG32_INITIALIZER;
}


struct Blk_shred::Main
{
	Main(Main const &);
	Main &operator = (Main const &);

	Genode::Env &env;

	Timer::Connection timer { env };

	Heap heap { env.ram(), env.rm() };

	Allocator_avl packet_alloc { &heap };

	Block::Connection<> blk {
		env, &packet_alloc, PKT_BUF_SIZE };

	Block::Session::Tx::Source &pkt_source = *blk.tx();

	Block::Session::Info const info { blk.info() };

	rand_data *jent { nullptr };
	pcg32_random_t pcg PCG32_INITIALIZER;

	template <typename... ARGS>
	void die(ARGS &&... args)
	{
		/* wipe noise state */
		seed_noise();

		error(args...);
		env.parent().exit(~0);
		sleep_forever();
	}

	void seed_noise()
	{
		/* read entropy into initialization variables */

		{
			/* XOR in jitter entropy */
			uint64_t buf[2] { 0 };
			jent_read_entropy(jent, (char*)&buf, sizeof(buf));
			pcg_init[0] ^= buf[0];
			pcg_init[1] ^= buf[1];
		}

		if (Genode::Rdrand::supported()) {
			/* XOR in RDRAND */
			pcg_init[0] ^= Genode::Rdrand::random64();
			pcg_init[1] ^= Genode::Rdrand::random64();
		}

		/* low bit must be set */
		pcg_init[1] |= 1;

		pcg32_srandom_r(&pcg, pcg_init[0], pcg_init[1]);
	}

	Main(Genode::Env &env) : env(env)
	{
		/***********************
		 ** Seed noise source **
		 ***********************/

		jitterentropy_init(heap);
		if (jent_entropy_init() != 0)
			die("jitterentropy library could not be initialized!");

		jent = jent_entropy_collector_alloc(0, 0);
		if (!jent)
			die("jitterentropy could not allocate entropy collector!");

		seed_noise();

		if (!info.writeable)
			die("block device not writeable!");
	}

	~Main()
	{
		/* wipe noise state */
		seed_noise();

		jent_entropy_collector_free(jent);
	}

	void submit_noise(Block::Packet_descriptor const &pkt)
	{
		uint32_t *buffer = (uint32_t*)pkt_source.packet_content(pkt);
		for (size_t i = 0; i < ((pkt.block_count()*info.block_size) / sizeof(uint32_t)); ++i)
			buffer[i] = pcg32_random_r(&pcg);
		pkt_source.submit_packet(pkt);
	}

	void shred()
	{
		float const mbytes = (float(info.block_count) * float(info.block_size)) / (1<<20);
		log("shredding ", mbytes/(1<<10), " GiB...");
		auto start_ms = timer.elapsed_ms();

		size_t const blk_per_pkt = PKT_SIZE / info.block_size;
		size_t const bytes_per_pkt =  blk_per_pkt * info.block_size;

		/*
		 * send two packets in succession, the first write
		 * aligns those that follow with end of the device
		 */
		Block::sector_t blk_offset = info.block_count % blk_per_pkt;
		if (blk_offset == 0)
			blk_offset = blk_per_pkt;

		{
			Block::Packet_descriptor pkt(
				pkt_source.alloc_packet(bytes_per_pkt),
				Block::Packet_descriptor::WRITE,
				0, blk_offset);
			submit_noise(pkt);
		}

		{
			Block::Packet_descriptor pkt(
				pkt_source.alloc_packet(bytes_per_pkt),
				Block::Packet_descriptor::WRITE,
				blk_offset, blk_per_pkt);
			submit_noise(pkt);
			blk_offset += blk_per_pkt;
		}

		while (true) {
			Block::Packet_descriptor const ack = pkt_source.get_acked_packet();
			if (!ack.succeeded())
				error("ack indicates failure ", ack.block_number(),"/",info.block_count);

			pkt_source.release_packet(ack);

			if (blk_offset >= info.block_count) break;

			/* reuse packet buffer region */
			Block::Packet_descriptor const pkt(
				ack, Block::Packet_descriptor::WRITE,
				blk_offset, blk_per_pkt);
			submit_noise(pkt);
			blk_offset += blk_per_pkt;
		}

		Block::Packet_descriptor const ack = pkt_source.get_acked_packet();
		if (!ack.succeeded())
			error("ack indicates failure ", ack.block_number(),"/",info.block_count);

		float seconds = (timer.elapsed_ms() - start_ms) / 1000;
		log("shred complete, ", mbytes / seconds, " MiB/s");
	}

	/**
	 * Verify a block, PCG must first be seeked into position
	 */
	bool verify_block(Block::sector_t sector)
	{
		Block::Packet_descriptor pkt(
			pkt_source.alloc_packet(info.block_size),
			Block::Packet_descriptor::READ, sector, 1);

		pkt_source.submit_packet(pkt);
		pkt = pkt_source.get_acked_packet();
		pkt_source.release_packet(pkt);

		if (!pkt.succeeded())
			die("error while reading back sector ", sector);

		uint32_t *buffer = (uint32_t*)pkt_source.packet_content(pkt);
		for (size_t i = 0; i < (info.block_size / sizeof(uint32_t)); ++i) {
			if (buffer[i] != pcg32_random_r(&pcg))
				die("sector ", sector, " is invalid");
		}
		return true;
	}

	/**
	 * Verify random blocks
	 */
	void verify()
	{
		/* a weak RNG to generate skips */
		pcg32_random_t skip_gen;
		pcg32_srandom_r(&skip_gen, pcg32_random_r(&pcg), pcg32_random_r(&pcg));

		/* reset the noise generator */
		pcg32_srandom_r(&pcg, pcg_init[0], pcg_init[1]);

		/* make jumps of approximately 1 Mib */
		int const max_jump = (2<<20) / info.block_size;
		int const step = info.block_size / sizeof(uint32_t);
		Block::sector_t sector_offset = 0;
		unsigned long count = 0;

		/* verify the first block and loop */
		if (!verify_block(sector_offset)) return;
		++count;

		/* random jump loop */
		while (true) {
			/* make a bounded random jumps */
			Block::sector_t skip =
				pcg32_boundedrand_r(&skip_gen, max_jump)+1;

			if (sector_offset+skip >= info.block_count)
				break;

			/* move the sector offset ahead */
			sector_offset += skip;

			/* seek the noise source ahead for the next read */
			pcg32_advance_r(&pcg, (skip-1)*step);

			if (!verify_block(sector_offset)) return;
			++count;
		}

		/* verify the last block */
		pcg32_advance_r(&pcg, (info.block_count-(sector_offset+2))*step);
		if (!verify_block(info.block_count-1)) return;
		++count;

		log(count, " blocks passed random spot check");
	}
};


void Component::construct(Genode::Env &env)
{
	Blk_shred::Main main(env);

	main.shred();
	main.verify();

	env.parent().exit(0);
}
