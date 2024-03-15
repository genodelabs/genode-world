/*
 * \brief  Block interface
 * \author Markus Partheymueller
 * \author Alexander Boettcher
 * \date   2012-09-15
 */

/*
 * Copyright (C) 2012 Intel Corporation
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

#ifndef _DISK_H_
#define _DISK_H_

/* Genode includes */
#include <block_session/connection.h>

/* Seoul includes */
#include <nul/motherboard.h>
#include <host/dma.h>

namespace Seoul {
	class Disk;
	class Disk_signal;
}

class Seoul::Disk_signal
{
	private:

		Disk     &_obj;
		unsigned  _id;

		void _signal();

	public:

		Genode::Signal_handler<Disk_signal> const sigh;

		Disk_signal(Genode::Entrypoint &ep, Disk &obj,
		            Block::Connection<> &block, unsigned disk_nr)
		:
		  _obj(obj), _id(disk_nr),
		  sigh(ep, *this, &Disk_signal::_signal)
		{
			block.tx_channel()->sigh_ack_avail(sigh);
			block.tx_channel()->sigh_ready_to_submit(sigh);
		}
};


class Seoul::Disk : public StaticReceiver<Seoul::Disk>
{
	private:

		struct Disk_session {
			Block::Connection<> *blk_con;
			Block::Session::Info info;
			Disk_signal         *signal;
		};

		struct Outstanding {
			unsigned long  usertag;
			unsigned       dmacount;
			DmaDescriptor *dma;
			unsigned long  physoffset;
		};

		/* SATA model has max 33 and IDE 1 as max outstanding requests atm */
		enum { MAX_DISKS = 4, MAX_OUTSTANDING = 48 };

		Genode::Env         &_env;
		Motherboard         &_mb;

		struct Outstanding   _outstanding[MAX_OUTSTANDING] { };
		struct Disk_session  _disks      [MAX_DISKS]       { };

		char        * const _backing_store_base;
		size_t        const _backing_store_size;

		Genode::Mutex       _mutex            { };
		bool                _resume_execution { };

		/*
		 * Noncopyable
		 */
		Disk             (Disk const &);
		Disk &operator = (Disk const &);

		bool execute(bool, Disk_session const &, MessageDisk &);

		bool _execute_write(Block::Session::Tx::Source       &,
		                    Block::Packet_descriptor   const &,
		                    unsigned long              const,
		                    Disk_session               const &,
		                    MessageDisk                const &);

		bool _execute_read (Block::Session::Tx::Source       &,
		                    Block::Packet_descriptor   const &,
		                    unsigned long              const,
		                    Disk_session               const &,
		                    MessageDisk                const &);

		bool for_each_dma_desc(auto   const &msg,
		                       auto   const &packet,
		                       char * const  source,
		                       auto   const &fn)
		{
			unsigned long offset = 0;

			/* check bounds for read and write operations */
			for (unsigned i = 0; i < msg.dmacount; i++) {
				char * const dma_addr = _backing_store_base +
				                        msg.dma[i].byteoffset +
				                        msg.physoffset;

				/* check for bounds */
				if (dma_addr >= _backing_store_base + _backing_store_size ||
				    dma_addr < _backing_store_base)
					return false;

				auto const bytecount = msg.dma[i].bytecount;

				if (!packet.size() || bytecount > packet.size() - offset)
					return false;

				fn(dma_addr, source + offset, bytecount);

				offset += bytecount;
			}

			return true;
		}

	public:

		static constexpr unsigned block_packetstream_size = 4*512*1024;

		Disk(Genode::Env &, Motherboard &, char *, Genode::size_t);

		void handle_disk(unsigned);

		bool receive(MessageDisk &msg);
};

#endif /* _DISK_H_ */
