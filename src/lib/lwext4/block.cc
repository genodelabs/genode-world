/*
 * \brief  Block device backend for lwext4
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/allocator.h>
#include <base/allocator_avl.h>
#include <base/log.h>
#include <block_session/connection.h>
#include <util/string.h>

// #include <timer_session/connection.h>

/* library includes */
#include <lwext4/init.h>

/* compiler includes */
#include <stdarg.h>

/* local libc includes */
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* lwext4 includes */
#include <ext4.h>
#include <ext4_blockdev.h>


struct Blockdev
{
	struct ext4_blockdev       ext4_blockdev;
	struct ext4_blockdev_iface ext4_blockdev_iface;
	unsigned char              ext4_block_buffer[4096];

	Genode::Env           &_env;
	Genode::Allocator     &_alloc;
	Genode::Allocator_avl  _tx_alloc { &_alloc };

	Block::Connection          _block { _env, &_tx_alloc, 512*1024 };
	Block::sector_t            _block_count { 0 };
	Genode::size_t             _block_size  { 512 };
	Block::Session::Operations _block_ops   { };

	Blockdev(Genode::Env &env, Genode::Allocator &alloc)
	: _env(env), _alloc(alloc)
	{
		_block.info(&_block_count, &_block_size, &_block_ops);

		if (!_block_ops.supported(Block::Packet_descriptor::READ)) {
			throw -1;
		}
	}

	bool writeable()
	{
		return _block_ops.supported(Block::Packet_descriptor::WRITE);
	}

	Block::Connection& block()             { return _block; }
	Block::sector_t    block_count() const { return _block_count; }
	Genode::size_t     block_size()  const { return _block_size; }
};


static int blockdev_open(struct ext4_blockdev *bdev)  { return EOK; }
static int blockdev_close(struct ext4_blockdev *bdev) { return EOK; }


static int blockdev_bread(struct ext4_blockdev *bdev,
                          void                 *dest,
                          uint64_t              lba,
                          uint32_t              count)
{
	Blockdev          &bd = *reinterpret_cast<Blockdev*>(bdev);
	Block::Connection &b  = bd.block();

	Genode::size_t const size = bd.block_size() * count;

	Block::Packet_descriptor p(b.tx()->alloc_packet(size),
	                           Block::Packet_descriptor::READ,
	                           lba, count);
	b.tx()->submit_packet(p);
	p = b.tx()->get_acked_packet();

	int result = EIO;
	if (p.succeeded() && p.size() == size) {
		char const * const content = b.tx()->packet_content(p);
		Genode::memcpy(dest, content, size);
		result = EOK;
	} else {
		Genode::error("could not read lba: ", lba, " count: ", count);
	}

	b.tx()->release_packet(p);
	return result;
}


static int blockdev_bwrite(struct ext4_blockdev *bdev,
                           void const           *src,
                           uint64_t              lba,
                           uint32_t              count)
{
	Blockdev &bd = *reinterpret_cast<Blockdev*>(bdev);
	if (!bd.writeable()) { return EIO; }

	Block::Connection      &b = bd.block();
	Genode::size_t const size = bd.block_size() * count;

	Block::Packet_descriptor p(b.tx()->alloc_packet(size),
	                           Block::Packet_descriptor::WRITE,
	                           lba, count);

	char * const content = b.tx()->packet_content(p);
	Genode::memcpy(content, src, size);

	b.tx()->submit_packet(p);
	p = b.tx()->get_acked_packet();

	int result = EIO;
	if (p.succeeded() && p.size() == size) {
		result = EOK;
	} else {
		Genode::error("could not write lba: ", lba, " count: ", count);
	}

	b.tx()->release_packet(p);

	return result;
}

/*
 * Genode enviroment
 */
static Genode::Env                     *_global_env;
static Genode::Allocator               *_global_alloc;
static Genode::Constructible<Blockdev>  _blockdev;


struct ext4_blockdev *Lwext4::block_init(Genode::Env &env, Genode::Allocator &alloc)
{
	_global_env   = &env;
	_global_alloc = &alloc;

	try         { _blockdev.construct(env, alloc); }
	catch (...) { throw Block_init_failed(); }

	_blockdev->ext4_blockdev.bdif        = &_blockdev->ext4_blockdev_iface;
	_blockdev->ext4_blockdev.part_offset = 0;
	_blockdev->ext4_blockdev.part_size   = _blockdev->block_count()
	                                     * _blockdev->block_size();

	_blockdev->ext4_blockdev_iface.ph_bbuf  = _blockdev->ext4_block_buffer;
	_blockdev->ext4_blockdev_iface.ph_bcnt  = _blockdev->block_count();
	_blockdev->ext4_blockdev_iface.ph_bsize = _blockdev->block_size();

	_blockdev->ext4_blockdev_iface.bread  = blockdev_bread;
	_blockdev->ext4_blockdev_iface.bwrite = blockdev_bwrite;
	_blockdev->ext4_blockdev_iface.close  = blockdev_close;
	_blockdev->ext4_blockdev_iface.open   = blockdev_open;

	return reinterpret_cast<ext4_blockdev*>(&*_blockdev);
}
