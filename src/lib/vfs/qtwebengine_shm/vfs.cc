/*
 * \brief  Process-local shared memory file system for qtwebengine:
 *         qtwebengine creates files in a single directory (the root
 *         directory of this plugin), allocates space once with
 *         'ftruncate()' and then maps the file contents with 'mmap()'
 *         as shared memory from different threads.
 * \author Christian Prochaska
 * \date   2019-03-29
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <vfs/env.h>
#include <vfs/file_system.h>
#include <vfs/file_system_factory.h>
#include <base/ram_allocator.h>
#include <dataspace/client.h>
#include <util/list.h>
#include <util/string.h>

using namespace Vfs;

class Dataspace_file_system : public Vfs::File_system
{
	public:

		enum { MAX_NAME_LEN = 128 };

	private:

		class Dataspace_vfs_file : public Genode::List<Dataspace_vfs_file>::Element
		{
			private:

				typedef Genode::String<MAX_NAME_LEN> Filename;
				Filename               _filename { };

				Genode::Allocator     &_alloc;
				Genode::Ram_allocator &_ram;

				Vfs::file_size         _length { 0 };

			public:

				unsigned int open_count   { 0 };
				bool unlink_on_last_close { false };

				bool matches(const char *path)
				{
					return (Genode::strlen(path) == (Genode::strlen(_filename.string()))) &&
					       (Genode::strcmp(path, _filename.string()) == 0);
				}

				Genode::Ram_dataspace_capability ds_cap { };

				Dataspace_vfs_file(char const *name, Genode::Allocator &alloc, Genode::Ram_allocator &ram)
				: _filename(name), _alloc(alloc), _ram(ram) { }

				~Dataspace_vfs_file()
				{
					if (_length > 0)
						_ram.free(ds_cap);
				}

				Genode::Allocator &alloc() { return _alloc; }

				Vfs::file_size length() { return _length; }

				Ftruncate_result truncate(Vfs::file_size size)
				{
					if (_length > 0) {
						Genode::error(__PRETTY_FUNCTION__, ": resizing not supported yet");
						return FTRUNCATE_ERR_NO_PERM;
					}

					_length = size;

					ds_cap = _ram.alloc((size_t)size);

					return FTRUNCATE_OK;
				}
		};


		class Dataspace_vfs_handle : public Vfs::Vfs_handle
		{
			public:

				using Vfs::Vfs_handle::Vfs_handle;

				virtual Read_result read(char *dst, file_size count,
				                         file_size &out_count) = 0;
		};


		class Dataspace_vfs_dir_handle : public Dataspace_vfs_handle
		{
			private:

				/*
				 * Noncopyable
				 */
				Dataspace_vfs_dir_handle(Dataspace_vfs_dir_handle const &);
				Dataspace_vfs_dir_handle &operator = (Dataspace_vfs_dir_handle const &);

			public:

				Dataspace_vfs_dir_handle(Directory_service &ds,
				                   File_io_service   &fs,
				                   Genode::Allocator &alloc)
				: Dataspace_vfs_handle(ds, fs, alloc, 0) { }

				Read_result read(char *dst, file_size count,
				                 file_size &out_count) override
				{
					Genode::error("Dataspace_vfs_dir_handle::read() called, not implemented");

					out_count = 0;

					if (count < sizeof(Dirent))
						return READ_ERR_INVALID;

					Dirent &out = *(Dirent*)dst;

					out = {
						.fileno = (Genode::addr_t)this,
						.type   = Dirent_type::END,
						.rwx    = { },
						.name   = { }
					};

					out_count = sizeof(Dirent);

					return READ_OK;
				}
		};


		class Dataspace_vfs_file_handle : public Dataspace_vfs_handle
		{
			private:

				/*
				 * Noncopyable
				 */
				Dataspace_vfs_file_handle(Dataspace_vfs_file_handle const &);
				Dataspace_vfs_file_handle &operator = (Dataspace_vfs_file_handle const &);

				Dataspace_vfs_file *_file;

			public:

				Dataspace_vfs_file_handle(Directory_service &ds,
				                    File_io_service   &fs,
				                    Genode::Allocator &alloc,
				                    Dataspace_vfs_file *file)
				: Dataspace_vfs_handle(ds, fs, alloc, 0), _file(file) { }

				Read_result read(char *, file_size, file_size &) override
				{
					return READ_ERR_INVALID;
				}

				Ftruncate_result truncate(file_size len)
				{
					return _file->truncate(len);
				}

				Dataspace_vfs_file *file() { return _file; }
		};

		Vfs::Env &_env;
		Genode::List<Dataspace_vfs_file> _files { };
		Genode::size_t             _num_dirent { 0 };

		bool _root(const char *path)
		{
			return (strcmp(path, "") == 0) || (strcmp(path, "/") == 0);
		}

		Dataspace_vfs_file *_lookup(char const *path)
		{
			for (Dataspace_vfs_file *file = _files.first(); file; file = file->next()) {
				if (file->matches(path))
					return file;
			}
			return nullptr;
		}

	public:

		Dataspace_file_system(Vfs::Env &env, Genode::Xml_node) : _env(env) { }

		~Dataspace_file_system() { }

		/*********************************
		 ** Directory service interface **
		 *********************************/

		Dataspace_capability dataspace(char const *path) override
		{
			Ram_dataspace_capability ds_cap;

			Dataspace_vfs_file *file = _lookup(path);

			if (!file)
				return ds_cap;

			return file->ds_cap;
		}

		void release(char const *, Dataspace_capability) override
		{
			/* the dataspace gets freed when the file is deleted */
		}

		Stat_result stat(char const *path, Stat &out) override
		{
			out = Stat { };
			out.device = (Genode::addr_t)this;

			if (_root(path)) {
				out.type = Node_type::DIRECTORY;

			} else if (_lookup(path)) {
				out.type  = Node_type::CONTINUOUS_FILE;
				out.rwx   = Node_rwx::rw();
				out.inode = (unsigned long)_lookup(path);
			} else {
				return STAT_ERR_NO_ENTRY;
			}
			return STAT_OK;
		}

		file_size num_dirent(char const *path) override
		{
			if (_root(path))
				return 1;
			else
				return _num_dirent;
		}

		bool directory(char const *path) override
		{
			if (_root(path))
				return true;

			return false;
		}

		char const *leaf_path(char const *path) override {
			return _lookup(path) ? path : nullptr; }

		Opendir_result opendir(char const  *path, bool create,
		                       Vfs_handle **handle,
		                       Allocator   &alloc) override
		{
			if (!_root(path))
				return OPENDIR_ERR_LOOKUP_FAILED;

			if (create)
				return OPENDIR_ERR_PERMISSION_DENIED;

			try {
				*handle = new (alloc) Dataspace_vfs_dir_handle(*this, *this, alloc);
				return OPENDIR_OK;
			}
			catch (Genode::Out_of_ram)  { return OPENDIR_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return OPENDIR_ERR_OUT_OF_CAPS; }
		}

		/*
		 * Sub directories are not supported.
		 */
		Open_result open(char const  *path, unsigned mode,
		                 Vfs_handle **handle,
		                 Allocator   &alloc) override
		{
			Dataspace_vfs_file *file { };

			bool const create = mode & OPEN_MODE_CREATE;

			if (create) {

				if (_lookup(path))
					return OPEN_ERR_EXISTS;

				if (strlen(path) >= MAX_NAME_LEN)
					return OPEN_ERR_NAME_TOO_LONG;

				try { file = new (alloc) Dataspace_vfs_file(path, alloc, _env.env().ram()); }
				catch (Allocator::Out_of_memory) { return OPEN_ERR_NO_SPACE; }

				_files.insert(file);
			} else {
				file = _lookup(path);
				if (!file) return OPEN_ERR_UNACCESSIBLE;
			}

			try {
				*handle = new (alloc) Dataspace_vfs_file_handle(*this, *this, alloc, file);
				file->open_count++;
				return OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

		void close(Vfs_handle *vfs_handle) override
		{
			if (vfs_handle && (&vfs_handle->ds() == this)) {

				Dataspace_vfs_file_handle *handle =
					dynamic_cast<Dataspace_vfs_file_handle *>(vfs_handle);

				if (!handle)
					return;

				Dataspace_vfs_file *file = handle->file();
				file->open_count--;

				if ((file->open_count == 0) && (file->unlink_on_last_close)) {
					_files.remove(file);
					destroy(file->alloc(), file);
				}

				destroy(vfs_handle->alloc(), vfs_handle);
			}
		}

		Rename_result rename(char const *, char const *) override
		{
			return RENAME_ERR_NO_PERM;
		}

		Unlink_result unlink(char const *path) override
		{
			Dataspace_vfs_file *file = _lookup(path);

			if (!file)
				return UNLINK_ERR_NO_ENTRY;

			if (file->open_count == 0) {
				_files.remove(file);
				destroy(file->alloc(), file);
			} else {
				file->unlink_on_last_close = true;
			}

			return UNLINK_OK;
		}


		/************************
		 ** File I/O interface **
		 ************************/

		Write_result write(Vfs_handle *, Const_byte_range_ptr const &, size_t &) override
		{
			return WRITE_ERR_INVALID;
		}

		Read_result complete_read(Vfs_handle *, Byte_range_ptr const &, size_t &) override
		{
			return READ_ERR_INVALID;
		}

		bool read_ready (Vfs_handle const &) const override { return false; }
		bool write_ready(Vfs_handle const &) const override { return false; }

		Ftruncate_result ftruncate(Vfs_handle *vfs_handle, file_size len) override
		{
			Dataspace_vfs_file_handle *handle =
				dynamic_cast<Dataspace_vfs_file_handle *>(vfs_handle);

			if (!handle)
				return FTRUNCATE_ERR_NO_PERM;

			try { handle->truncate(len); }
			catch (Allocator::Out_of_memory) { return FTRUNCATE_ERR_NO_SPACE; }

			return FTRUNCATE_OK;
		}

		/***************************
		 ** File_system interface **
		 ***************************/

		static char const *name()   { return "qtwebengine_shm"; }
		char const *type() override { return "qtwebengine_shm"; }
};

extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	struct Factory : Vfs::File_system_factory
	{
		Vfs::File_system *create(Vfs::Env &vfs_env, Genode::Xml_node config) override
		{
			return new (vfs_env.alloc()) Dataspace_file_system(vfs_env, config);
		}
	};

	static Factory f;
	return &f;
}
