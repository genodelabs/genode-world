/**
 * \brief  Lwext4 file system symlink node
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SYMLINK_H_
#define _SYMLINK_H_

/* Genode includes */
#include <file_system/util.h>
#include <os/path.h>

/* lwext4 includes */
#include <ext4.h>

/* local includes */
#include <node.h>

namespace Lwext4_fs {
	class Symlink;
}

class Lwext4_fs::Symlink : public Node
{
	public:

		Symlink(char const *name, bool create) : Node(name, create) { }

		size_t write(char const *src, size_t len, seek_off_t) override
		{
			/* src may not be null-terminated */
			Genode::String<MAX_PATH_LEN> target(Genode::Cstring(src, len));

			int const err = ext4_fsymlink(target.string(), Node::name());
			/* on success return len to make _process_packet happy */
			return err == -1 ? 0 : len;
		}

		size_t read(char *dst, size_t len, seek_off_t) override
		{
			size_t bytes = 0;
			int const err = ext4_readlink(Node::name(), dst, len, &bytes);
			return err == -1 ? 0 : bytes;
		}
};

#endif /* _SYMLINK_H_ */
