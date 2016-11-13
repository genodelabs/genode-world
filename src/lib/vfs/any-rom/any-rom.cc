/*
 * \brief  Any ROM filesystem
 * \author Emery Hemingway
 * \date   2015-10-28
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <vfs/file_system_factory.h>
#include <vfs/vfs_handle.h>
#include <base/attached_rom_dataspace.h>
#include <base/session_label.h>
#include <util/list.h>

namespace Vfs { class Any_rom_file_system; };

class Vfs::Any_rom_file_system : public File_system
{
	private:

		struct Rom : Genode::Attached_rom_dataspace, Genode::List<Rom>::Element
		{
			Genode::Session_label const _name;

			int _ref_count = 0;

			Rom(Genode::Env &env, Genode::Session_label const &rom_label )
			: Attached_rom_dataspace(env, rom_label.string()), _name(rom_label.last_element()) { }

			bool operator == (char const *other) const { return _name == other; }

			void incr() { ++_ref_count; }
			void decr() { --_ref_count; }

			bool unused() const { return _ref_count <= 0; }
		};

		class Rom_vfs_handle : public Vfs_handle
		{
			private:

				Rom &_rom;

			public:

				Rom_vfs_handle(Any_rom_file_system &fs,
				               Genode::Allocator   &alloc,
				               Rom                 &rom)
				:
					Vfs_handle(fs, fs, alloc, OPEN_MODE_RDONLY),
					_rom(rom)
				{ _rom.incr(); }

				~Rom_vfs_handle() { _rom.decr(); }

				file_size read(char *buf, file_size buf_size)
				{
					file_size size = _rom.size();
					file_size offset = min(size, seek());
					file_size len    = min(buf_size, (size - offset));

					memcpy(buf, _rom.local_addr<char>()+offset, len);
					return len;
				}
		};

		Genode::Env       &_env;
		Genode::Allocator &_alloc;
		Genode::List<Rom>  _roms;

		Genode::Session_label const _label;

		Rom *lookup(char const *filename)
		{
			using namespace Genode;

			if (*filename == '/') ++filename;

			for (Rom *rom = _roms.first(); rom; rom = rom->next()) {
				if (*rom == filename) {
					/* if the ROM dataspace is not in use, update it */
					if (rom->unused()) {
						_roms.remove(rom);
						destroy(_alloc, rom);
						/* fallthru and try again */
					} else
						return rom;
				}
			}

			try {
				Rom *rom = new (_alloc)
					Rom(_env, prefixed_label(_label, Session_label(filename)));
				_roms.insert(rom);
				return rom;
			} catch (...) { }
			return 0;
		}

	public:

		Any_rom_file_system(Genode::Env &env,
		                    Genode::Allocator &alloc,
		                    Genode::Xml_node node)
		:
			_env(env), _alloc(alloc),
			_label(node.attribute_value("label",
			                            Genode::String<Genode::Session_label::capacity()>()).string())
		{ }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		file_size num_dirent(char const *path) override { return 0; }

		bool directory(char const *path) override { return false; }

		char const *leaf_path(char const *path) override {
			return lookup(path) ? path : 0; }

		Dataspace_capability dataspace(char const *path) override
		{
			if (Rom *rom = lookup(path)) {
				rom->incr();
				return rom->cap();
			}
			return Dataspace_capability();
		}

		void release(char const *path,
		             Dataspace_capability ds_cap) override
		{
			if (Rom *rom = lookup(path))
				rom->decr();
		}

		Open_result open(char        const *path,
		                 unsigned           mode,
		                 Vfs_handle       **handle,
		                 Genode::Allocator &alloc) override
		{
			if ((mode & OPEN_MODE_ACCMODE) != OPEN_MODE_RDONLY)
				return OPEN_ERR_NO_PERM;

			Rom *rom = lookup(path);
			if (!rom)
				return (mode & OPEN_MODE_CREATE) ?
					OPEN_ERR_NO_PERM : OPEN_ERR_UNACCESSIBLE;

			if (mode & OPEN_MODE_CREATE)
				return OPEN_ERR_EXISTS;

			*handle = new (alloc) Rom_vfs_handle(*this, alloc, *rom);
			return OPEN_OK;
		}

		void close(Vfs_handle *vfs_handle) override
		{
			Rom_vfs_handle *handle =
				static_cast<Rom_vfs_handle *>(vfs_handle);
			if (handle)
				destroy(handle->alloc(), handle);	
		}

		Stat_result stat(char const *path, Stat &stat) override
		{
			stat = Stat();
			Rom *rom = lookup(path);
			if (!rom) return STAT_ERR_NO_ENTRY;

			stat.size = rom->size();
			return STAT_OK;
		}

		Dirent_result dirent(char const *, file_offset, Dirent &dirent) override {
			return DIRENT_ERR_INVALID_PATH; }

		Unlink_result unlink(char const *path)
		{
			if (Rom *rom = lookup(path)) {
				if (rom->unused()) {
					/* why not free some memory */
					_roms.remove(rom);
					destroy(_alloc, rom);
					return UNLINK_OK;
				}
				return UNLINK_ERR_NO_PERM;
			}
			return UNLINK_ERR_NO_ENTRY;
		}

		Readlink_result readlink(char const*, char*,
	                             file_size, file_size&) override {
			return READLINK_ERR_NO_ENTRY; }

		Rename_result rename(char const *path, char const*) override
		{
			if (lookup(path))
				return RENAME_ERR_NO_PERM;
			return RENAME_ERR_NO_ENTRY;
		}

		Symlink_result symlink(char const*, char const*) override {
			return SYMLINK_ERR_NO_PERM; }

		Mkdir_result mkdir(char const*, unsigned) override {
			return MKDIR_ERR_NO_PERM; }

		/**
		 * Clean out unused ROM connections
		 */
		void sync(char const *path) override
		{
			for (Rom *rom = _roms.first(); rom;) {
				if (rom->unused()) {
					_roms.remove(rom);
					destroy(_alloc, rom);
					rom = _roms.first();
				} else {
					rom = rom->next();
				}
			}
		}

		/**********************
		 ** File I/O service **
		 **********************/

		Write_result write(Vfs_handle*,
		                   char const*, file_size,
		                   file_size&) override {
			return WRITE_ERR_INVALID; }

		Read_result read(Vfs_handle *vfs_handle,
		                 char *buf, file_size buf_size,
		                 file_size &out) override
		{
			Rom_vfs_handle *handle =
				static_cast<Rom_vfs_handle *>(vfs_handle);
			if (handle) {
				out = handle->read(buf, buf_size);
				return READ_OK;
			} else {
				out = 0;
				return READ_ERR_INVALID;
			}
		}

		Ftruncate_result ftruncate(Vfs_handle *, file_size) override {
			return FTRUNCATE_ERR_NO_PERM; }
};


extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	struct Any_rom_factory : Vfs::File_system_factory
	{
		Vfs::File_system *create(Genode::Env &env,
		                         Genode::Allocator &alloc,
		                         Genode::Xml_node node) override
		{
			return new (alloc) Vfs::Any_rom_file_system(env, alloc, node);
		}
	};

	static Any_rom_factory factory;
	return &factory;
}
