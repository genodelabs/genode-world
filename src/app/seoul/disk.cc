/*
 * \brief  Block interface
 * \author Markus Partheymueller
 * \author Alexander Boettcher
 * \date   2012-09-15
 */

/*
 * Copyright (C) 2012      Intel Corporation
 * Copyright (C) 2013-2024 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Vancouver VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/log.h>
#include <base/heap.h>

/* local includes */
#include "disk.h"

static Genode::Heap * disk_heap(Genode::Ram_allocator *ram = nullptr,
                                Genode::Env::Local_rm *rm = nullptr)
{
	static Genode::Heap heap(ram, rm);
	return &heap;
}


Seoul::Disk::Disk(Genode::Env &env, Motherboard &mb,
                  char * backing_store_base, Genode::size_t backing_store_size)
:
	_env(env),
	_mb(mb),
	_backing_store_base(backing_store_base),
	_backing_store_size(backing_store_size)
{
	/* initialize disk heap */
	disk_heap(&env.ram(), &env.rm());

	mb.bus_disk.add(this, receive_static<MessageDisk>);
}


void Seoul::Disk_signal::_signal() { _obj.handle_disk(_id); }


void Seoul::Disk::handle_disk(unsigned disknr)
{
	_mutex.acquire();

	auto &source = *_disks[disknr].blk_con->tx();

	while (source.ack_avail())
	{
		auto const packet      = source.get_acked_packet();
		auto       disk_answer = MessageDisk::DISK_OK;
		auto       user_tag    = packet.tag().value;

		switch (packet.operation()) {
		case Block::Packet_descriptor::Opcode::END:
		case Block::Packet_descriptor::Opcode::SYNC:
		case Block::Packet_descriptor::Opcode::TRIM:
			disk_answer = MessageDisk::DISK_STATUS_DEVICE;
			break;
		case Block::Packet_descriptor::Opcode::WRITE:
			break;
		case Block::Packet_descriptor::Opcode::READ:
		{
			if (packet.tag().value >= MAX_OUTSTANDING)
				Logging::panic("in-sane outstanding tag");

			/* for read requests the tag().value is the entry in the array */
			auto &read = _outstanding[packet.tag().value];

			bool ok = for_each_dma_desc(read, packet, source.packet_content(packet),
			                            [&](auto dma_addr, auto src, auto count) {
				memcpy(dma_addr, src, count);
			});

			if (!ok)
				Genode::warning("Opcode::READ failed");

			destroy(disk_heap(), read.dma);

			/* replace outstanding array id with real user_tag */
			user_tag = read.usertag;
			read     = { }; /* mark entry as free */

			break;
		}
		}

		if (!packet.succeeded()) {
			disk_answer = MessageDisk::DISK_STATUS_BUSY;
			Genode::error("packet not successful");
		}

		_mutex.release();

		MessageDiskCommit mdc(disknr, user_tag, disk_answer);
		_mb.bus_diskcommit.send(mdc);

		_mutex.acquire();

		source.release_packet(packet);
	}

	if (_resume_execution) {
		_resume_execution = false;

		_mutex.release();

		MessageDiskCommit msg(disknr, ~0U, MessageDisk::DISK_STATUS_RESUME);
		_mb.bus_diskcommit.send(msg);

		_mutex.acquire();

		if (msg.status != MessageDisk::DISK_OK) {
			_resume_execution = true;
		}
	}

	_mutex.release();
}


bool Seoul::Disk::receive(MessageDisk &msg)
{
	if (msg.disknr >= MAX_DISKS)
		Logging::panic("You configured more disks than supported.\n");

	auto &disk = _disks[msg.disknr];

	if (!disk.info.block_size) {
		Genode::String<16> label("disk", msg.disknr);
		/*
		 * If we receive a message for this disk the first time, create the
		 * structure for it.
		 */
		try {
			Genode::Allocator_avl * block_alloc =
				new (disk_heap()) Genode::Allocator_avl(disk_heap());

			disk.blk_con =
				new (disk_heap()) Block::Connection<>(_env, block_alloc,
				                                      block_packetstream_size,
				                                      label.string());
			disk.signal =
				new (disk_heap()) Seoul::Disk_signal(_env.ep(), *this,
				                                     *disk.blk_con, msg.disknr);
		} catch (...) {
			/* there is none. */
			return false;
		}

		disk.info = disk.blk_con->info();

		if (!disk.info.block_size || disk.info.block_size != 512 ||
		     disk.info.align_log2 > 31)
			Logging::panic("unsupported block size %lu", disk.info.align_log2);
	}

	msg.error = MessageDisk::DISK_OK;

	switch (msg.type) {
	case MessageDisk::DISK_GET_PARAMS:
	{
		Genode::String<16> label("disk", msg.disknr);

		if (disk.info.block_count >= 1ull << 32 ||
		    disk.info.block_size  >= 1ull << 32)
			Logging::panic("disk: too many blocks");

		msg.params->flags           = DiskParameter::FLAG_HARDDISK;
		msg.params->sectors         = disk.info.block_count;
		msg.params->sectorsize      = unsigned(disk.info.block_size);
		msg.params->maxrequestcount = unsigned(disk.info.block_count);
		memcpy(msg.params->name, label.string(), label.length());

		return true;
	}
	case MessageDisk::DISK_WRITE:
		/* don't write on read only medium */
		if (!disk.info.writeable) {
			/* nevertheless confirm that commit got processed */
			Genode::warning("write denied to r/o disk", msg.disknr);
			MessageDiskCommit ro(msg.disknr, msg.usertag, MessageDisk::DISK_OK);
			_mb.bus_diskcommit.send(ro);
			return true;
		}

		[[fallthrough]];

	case MessageDisk::DISK_READ:
		/* read and write handling */
		return execute(msg.type == MessageDisk::DISK_WRITE, disk, msg);
	default:
		Genode::warning("unknown disk operation ", unsigned(msg.type));
		return false;
	}
}


bool Seoul::Disk::execute(bool const write, Disk_session const &disk,
                          MessageDisk &msg)
{
	auto const  total    = DmaDescriptor::sum_length(msg.dmacount, msg.dma);
	auto const  blk_size = disk.info.block_size;
	auto const  blocks   = total / blk_size + ((total % blk_size) ? 1 : 0);
	auto       &tx       = *disk.blk_con->tx();
	auto const  max_size = tx.bulk_buffer_size() / 512 * 512;

	if (total % blk_size)
		Genode::warning("unsupported data size");

	if (total > max_size)
	{
		/* notify model to retry with decreased amount */
		msg.more  = max_size;
		msg.error = MessageDisk::DISK_STATUS_DMA_TOO_LARGE;
		return true;
	}

	Genode::Mutex::Guard guard(_mutex);

	if (_resume_execution) {
		Genode::warning("unexpected resume state");
		return false;
	}

	auto const & fn_resume = [&]() {
		_resume_execution = true;
		msg.error         = MessageDisk::DISK_STATUS_BUSY;

		tx.wakeup();
	};

	if (!tx.ready_to_submit()) {
		fn_resume();
		return true;
	}

	auto result = tx.alloc_packet_attempt(blocks * blk_size,
	                                      unsigned(disk.info.align_log2));

	return result.convert<bool>([&](auto const p) {
		typedef Block::Packet_descriptor Pkg;

		bool success = write ? _execute_write(tx, p, blocks, disk, msg)
		                     : _execute_read (tx, p, blocks, disk, msg);

		if (!success) {
			if (write) {
				/* hint that you are doomed with high probability */
				Genode::error("write packet failed");
			} else {
				/* no sufficient outstanding array space is temporarily */
				fn_resume();
			}

			tx.release_packet(p);
		}

		if (!msg.more || !success)
			tx.wakeup();

		return success;
	}, [&](auto /* temporary insufficient space in packet stream */) {
		fn_resume();
		return true;
	});
}


bool Seoul::Disk::_execute_read(Block::Session::Tx::Source       &tx,
                                Block::Packet_descriptor   const &desc,
                                unsigned long              const  blocks,
                                Disk_session               const &disk,
                                MessageDisk                const &msg)
{
	for (unsigned i = 0; i < MAX_OUTSTANDING; i++) {
		auto & pending = _outstanding[i];

		if (pending.dma)
			continue;

		typedef Block::Packet_descriptor Pkg;

		Pkg packet(desc, Pkg::READ, msg.sector, blocks,
		           Block::Request::Tag { i });

		pending = { .usertag    = msg.usertag,
		            .dmacount   = msg.dmacount,
		            .dma        = new (disk_heap()) DmaDescriptor[msg.dmacount],
		            .physoffset = msg.physoffset };

		for (unsigned i = 0; i < pending.dmacount; i++)
			pending.dma[i] = msg.dma[i];

		tx.try_submit_packet(packet);

		return true;
	}

	/* temporarily effect, more space if some read requests are processed */
	return false;
}


bool Seoul::Disk::_execute_write(Block::Session::Tx::Source       &tx,
                                 Block::Packet_descriptor   const &desc,
                                 unsigned long              const  blocks,
                                 Disk_session               const &disk,
                                 MessageDisk                const &msg)
{
	typedef Block::Packet_descriptor Pkg;

	Pkg packet(desc, Pkg::WRITE, msg.sector, blocks,
	           Block::Request::Tag { msg.usertag });

	bool ok = for_each_dma_desc(msg, packet, tx.packet_content(packet),
	                            [&](auto dma_addr, auto src, auto count) {
		memcpy(src, dma_addr, count);
	});

	if (ok)
		tx.try_submit_packet(packet);

	return ok;
}
