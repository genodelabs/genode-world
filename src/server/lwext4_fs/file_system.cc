/*
 * \brief  Lwext4 file system
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
#include <base/log.h>
#include <util/string.h>

/* library includes */
#include <ext4.h>

/* local includes */
#include <file_system.h>


static char const *_fs_name = "ext4";
static char const *_fs_mp   = "/";
static bool        _cache_write_back = false;


void File_system::init(ext4_blockdev *bd)
{
	int err = ext4_device_register(bd, _fs_name);
	if (err) { throw Init_failed(); }
}


void File_system::mount_fs(Genode::Xml_node config)
{
	int err = ext4_mount(_fs_name, _fs_mp, false);
	if (err) {
		Genode::error("could not mount file system, err: ", err);
		throw Mount_failed();
	}

	_cache_write_back = config.attribute_value("cache_write_back", false);
	if (_cache_write_back) {
		err = ext4_cache_write_back(_fs_mp, 1);
		if (err) {
			Genode::warning("could not enable cache write-back mode, err: ", err);
			_cache_write_back = false;
		}
	}

	err = ext4_recover(_fs_mp);
	if (err && err != ENOTSUP) {
		Genode::error("could not recover file system (FSCK needed!), err:", err);
		throw Mount_failed();
	}

	err = ext4_journal_start(_fs_mp);
	if (err) {
		Genode::error("could not start journal, err: ", err);
		throw Mount_failed();
	}
}


void File_system::unmount_fs()
{
	int err = ext4_journal_stop(_fs_mp);
	if (err) {
		Genode::error("could not stop journal, err: ", err);
		// throw Genode::Exception();
	}

	if (_cache_write_back) {
		err = ext4_cache_write_back(_fs_mp, 0);
		if (err) {
			Genode::error("could not disable cache write-back mode, err: ", err);
		}
	}

	err = ext4_umount(_fs_mp);
	if (err) {
		Genode::error("could not unmount file system, err: ", err);
		throw Unmount_failed();
	}
}


void File_system::sync()
{
	int const err = ext4_cache_flush(_fs_mp);
	if (err) {
		Genode::error("could not flush cache, err: ", err);
		throw Sync_failed();
	}
}


void File_system::stats_update(Genode::Reporter &reporter)
{
	using namespace Genode;

	struct ext4_mount_stats stats { };
	int const err = ext4_mount_point_stats(_fs_mp, &stats);
	if (err) {
		Genode::error("could not get mount point stats");
		return;
	}

	try {
		Reporter::Xml_generator xml(reporter, [&] () {
			xml.node("blocks", [&] () {
				xml.attribute("used",  stats.blocks_count-stats.free_blocks_count);
				xml.attribute("avail", stats.free_blocks_count);
				xml.attribute("size",  stats.block_size);
			});
			xml.node("inodes", [&] () {
				xml.attribute("used",  stats.inodes_count);
				xml.attribute("avail", stats.free_inodes_count);
			});
		});
	} catch (...) { }
}
