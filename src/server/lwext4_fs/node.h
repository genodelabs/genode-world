/*
 * \brief  Lwext4 file system node
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _NODE_H_
#define _NODE_H_

/* Genode includes */
#include <base/log.h>
#include <file_system/node.h>
#include <os/path.h>

/* lwext4 includes */
#include <ext4.h>
#include <ext4_inode.h>


namespace Lwext4_fs {
	using namespace File_system;

	typedef Genode::Path<2047 + 1> Absolute_path;

	class Node;
}

#define NODE_DEBUG_MSG() \
	Genode::error(__func__, " called on generic Node object")

class Lwext4_fs::Node : public Node_base
{
	protected:

		struct ext4_inode _inode;
		unsigned int      _ino;

		Absolute_path _name;

	public:

		struct Eof : Genode::Exception { };

		Node(char const *name, bool create = false) : _name(name)
		{
			if (!create) {
				int const err = ext4_raw_inode_fill(_name.base(), &_ino, &_inode);
				/* silent error because the look up is allowed to fail */
				if (err) { throw Lookup_failed(); }
			}
		}

		virtual Status status()
		{
			int err = ext4_raw_inode_fill(_name.base(), &_ino, &_inode);
			if (err) {
				Genode::error(__func__, " ext4_raw_inode_fill: error: ", err);
				throw Lookup_failed();
			}

			struct ext4_sblock *sb;
			err = ext4_get_sblock(_name.base(), &sb);
			if (err) {
				Genode::error(__func__, " ext4_get_sblock: error: ", err);
				throw Lookup_failed();
			}

			Status status;
			status.size  = ext4_inode_get_size(sb, &_inode);
			status.inode = _ino;

			unsigned int const v = ext4_inode_get_mode(sb, &_inode) & 0xf000;

			switch (v) {
			case EXT4_INODE_MODE_DIRECTORY: status.mode = Status::MODE_DIRECTORY; break;
			case EXT4_INODE_MODE_SOFTLINK:  status.mode = Status::MODE_SYMLINK;   break;
			case EXT4_INODE_MODE_FILE:
			default:                        status.mode = Status::MODE_FILE;      break;
			}

			return status;
		}

		char const *name() { return _name.base(); }

		virtual size_t read(char *, size_t, seek_off_t)
		{
			NODE_DEBUG_MSG();
			return 0;
		}

		virtual size_t write(char const *, size_t, seek_off_t)
		{
			NODE_DEBUG_MSG();
			return 0;
		}

		virtual void truncate(file_size_t) { NODE_DEBUG_MSG(); }
};

#undef NODE_DEBUG_MSG

#endif /* _NODE_H_ */
