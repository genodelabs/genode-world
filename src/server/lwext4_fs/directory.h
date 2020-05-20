/*
 * \brief  Lwext4 file system directory node
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

/* lwext4 includes */
#include <ext4.h>

/* local includes */
#include <node.h>


namespace Lwext4_fs {
	using namespace Genode;
	class Directory;
}

class Lwext4_fs::Directory : public Node
{
	private:

		ext4_dir _dir { };
		int64_t  _prev_index { -1 };

		void _open(char const *path, bool create)
		{
			/* always try to open the directory first */
			int err = ext4_dir_open(&_dir, path);
			if (err && !create) {
					error("ext4_dir_open failed: ", err);
					throw Permission_denied();
			} else if (err && create) {
				err = ext4_dir_mk(path);
				if (err) {
					error("ext4_dir_mk failed: ", err);
					throw Permission_denied();
				}
				err = ext4_mode_set(path, 0777);
				if (err) {
					error("ext4_mode_set failed: ", err);
					ext4_dir_rm(path);
					throw Permission_denied();
				}
			} else if (!err && create) {
				/* it already exists but we were advised to create it */
				ext4_dir_close(&_dir);
				throw Node_already_exists();
			}
		}

	public:

		Directory(char const *name, bool create = false) : Node(name, create)
		{
			_open(name, create);
		}

		size_t read(char *dest, size_t len, seek_off_t seek_offset)
		{
			if (len < sizeof(Directory_entry)) {
				error("read buffer too small for directory entry");
				return 0;
			}

			if (seek_offset % sizeof(Directory_entry)) {
				error("seek offset not alighed to sizeof(Directory_entry)");
				return 0;
			}

			Directory_entry * const e = (Directory_entry *)(dest);

			int64_t const index = seek_offset / sizeof(Directory_entry);

			/*
			 * Manipulate ext4_dir struct directly which AFAICT is
			 * okay and let lwext4 deal with it to safe CPU time.
			 */
			if (index != (_prev_index + 1)) {
				_dir.next_off = index > 0 ? index : 0;
			}

			ext4_direntry const *dentry = nullptr;
			while (true) {
				dentry = ext4_dir_entry_next(&_dir);
				if (!dentry) { break; }

				/* ignore entries without proper inode */
				if (!dentry->inode) {
					warning("skip dentry with empty inode");
					continue;
				}

				size_t const len = (size_t)(dentry->name_length + 1) > sizeof(e->name)
				                 ? sizeof(e->name) : dentry->name_length + 1;
				copy_cstring(e->name.buf, reinterpret_cast<char const*>(dentry->name), len);

				e->inode = dentry->inode;
				break;
			}

			if (!dentry) { throw Node::Eof(); }

			_prev_index = index;

			switch (dentry->inode_type) {
			case EXT4_DE_DIR:     e->type = Node_type::DIRECTORY;       break;
			case EXT4_DE_SYMLINK: e->type = Node_type::SYMLINK;         break;
			default:              e->type = Node_type::CONTINUOUS_FILE; break;
			}

			return sizeof(Directory_entry);
		}

		size_t write(char const *src, size_t len, seek_off_t) { return 0; }
};

#endif /* _DIRECTORY_H_ */
