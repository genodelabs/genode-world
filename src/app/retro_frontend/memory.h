/*
 * \brief  Libretro persistent memory file
 * \author Emery Hemingway
 * \date   2017-11-17
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__MEMORY_H_
#define _RETRO_FRONTEND__MEMORY_H_

/* Libc includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace Retro_frontend {

	struct Memory_file
	{
		void   *data = nullptr;
		size_t  size = 0;
		unsigned const  id;
		char     const *path;
		int fd = -1;

		Genode::size_t file_size()
		{
			struct stat s;
			s.st_size = 0;
			stat(path, &s);
			return s.st_size;
		}

		bool open_file_for_data()
		{
			if (!(data && size))
				return false;

			if (fd == -1)
				fd = ::open(path, O_RDWR|O_CREAT);
			return fd != -1;
		}

		Memory_file(unsigned id, char const *filename)
		: id(id), path(filename)
		{ }

		~Memory_file() { if (fd != -1) close(fd); }

		void read()
		{
			if (open_file_for_data()) {
				lseek(fd, 0, SEEK_SET);
				size_t remain = Genode::min(size, file_size());
				size_t offset = 0;
				do {
					ssize_t n = ::read(fd, ((char*)data)+offset, remain);
					if (n == -1) {
						Genode::error("failed to read from ", path);
						break;
					}
					remain -= n;
					offset += n;
				} while (remain);
			}
		}

		void write()
		{
			if (open_file_for_data()) {
				lseek(fd, 0, SEEK_SET);
				ftruncate(fd, size);
				size_t remain = size;
				size_t offset = 0;
				do {
					ssize_t n = ::write(fd, ((char const *)data)+offset, remain);
					if (n == -1) {
						Genode::error("failed to write to ", path);
						break;
					}
					remain -= n;
					offset += n;
				} while (remain);
			}
		}
	};
}

#endif
