/*
 * \brief  Lwext4 file system file node
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _FILE_H_
#define _FILE_H_

/* local includes */
#include <node.h>

/* lwext4 includes */
#include <ext4.h>

namespace Lwext4_fs {
	using namespace Genode;
	class File;
}

class Lwext4_fs::File : public Node
{
	private:

		ext4_file _file;

	public:

		File(const char *name, Mode mode, bool create) : Node(name, create)
		{
			int flags = 0;
			if (create) { flags |= O_CREAT; }

			switch (mode) {
			case READ_ONLY:  flags |= O_RDONLY; break;
			case WRITE_ONLY: flags |= O_WRONLY; break;
			case READ_WRITE: flags |= O_RDWR;   break;
			default: break;
			}

			int err = ext4_fopen2(&_file, name, flags);
			if (err) {
				error("ext4_fopen2: error: ", err);
				throw Permission_denied();
			}

			if (create) {
				err = ext4_mode_set(name, 0666);
				if (err) {
					error("ext4_mode_set: error: ", err);
					throw Permission_denied();
				}
			}
		}

		~File()
		{
			ext4_fclose(&_file);
		}

		size_t read(char *dest, size_t len, seek_off_t seek_offset) override
		{
			bool const to_end = seek_offset == (seek_off_t)(~0);

			int err = ext4_fseek(&_file, to_end ? 0 : seek_offset,
			                             to_end ? SEEK_END : SEEK_SET);
			if (err) {
				error(__func__, ": invalid seek offset");
				return 0;
			}

			Genode::size_t bytes = 0;
			err = ext4_fread(&_file, dest, len, &bytes);
			if (err) {
				error(__func__, ": error: ", err);
				return 0;
			}

			if (!bytes) { throw Node::Eof(); }

			return bytes;
		}

		size_t write(char const *src, size_t len, seek_off_t seek_offset) override
		{
			bool const to_end = seek_offset == (seek_off_t)(~0);

			int err = ext4_fseek(&_file, to_end ? 0 : seek_offset,
			                             to_end ? SEEK_END : SEEK_SET);
			if (err) {
				error(__func__, ": invalid seek offset: ", seek_offset);
				return 0;
			}

			Genode::size_t bytes = 0;
			err = ext4_fwrite(&_file, src, len, &bytes);
			if (err) {
				error(__func__, ": error: ", err);
				return 0;
			}

			return bytes;
		}

		void truncate(file_size_t size) override
		{
			int const err = ext4_ftruncate(&_file, size);
			if (err) { error(__func__, ": error: ", err); }
		}

};

#endif /* _FILE_H_ */
