/**
 * \brief  Lwext4 file system initialization
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */
#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

/* Genode includes */
#include <base/exception.h>
#include <os/reporter.h>
#include <util/xml_node.h>


struct ext4_blockdev;

namespace File_system {

	struct Init_failed    : Genode::Exception { };
	struct Mount_failed   : Genode::Exception { };
	struct Unmount_failed : Genode::Exception { };
	struct Sync_failed    : Genode::Exception { };

	void init(ext4_blockdev*);
	void mount_fs(Genode::Xml_node);
	void unmount_fs();
	void sync();
	void stats_update(Genode::Reporter &);
}

#endif /* _FILE_SYSTEM_H_ */
