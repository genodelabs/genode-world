/*
 * \brief  Rom-session server for ISO file systems
 * \author Sebastian Sumpf <Sebastian.Sumpf@genode-labs.com>
 * \date   2010-07-26
 */

/*
 * Copyright (C) 2010-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/component.h>
#include <base/heap.h>
#include <base/log.h>
#include <base/rpc_server.h>
#include <dataspace/client.h>
#include <root/component.h>
#include <util/dictionary.h>
#include <base/attached_ram_dataspace.h>
#include <base/session_label.h>
#include <block_session/connection.h>
#include <rom_session/rom_session.h>

/* local includes */
#include "iso9660.h"

using namespace Genode;


/*****************
 ** ROM service **
 *****************/

namespace Iso {

	class File;

	using File_name  = String<PATH_LENGTH>;
	using File_cache = Dictionary<File, File_name>;

	class Rom_component;
	typedef Genode::Root_component<Rom_component> Root_component;

	class Root;
}


/**
 * File abstraction
 */
class Iso::File : public File_cache::Element
{
	private:

		/*
		 * Noncopyable
		 */
		File(File const &);
		File &operator = (File const &);

		Genode::Allocator        &_alloc;

		File_info                *_info;
		Attached_ram_dataspace    _ds;

	public:

		File(Genode::Env &env, Genode::Allocator &alloc, File_cache &file_cache,
		     Block::Connection<> &block, char const *path)
		:
			File_cache::Element(file_cache, path), _alloc(alloc),
			_info(Iso::file_info(_alloc, block, path, env.ep())),
			_ds(env.ram(), env.rm(), align_addr(_info->page_sized(), 12))
		{
			Iso::read_file(block, _info, 0, _ds.size(), _ds.local_addr<void>(), env.ep());
		}
		
		~File() { destroy(_alloc, _info); }

		Dataspace_capability dataspace() { return _ds.cap(); }
};


class Iso::Rom_component : public Genode::Rpc_object<Rom_session>
{
	private:

		/*
		 * Noncopyable
		 */
		Rom_component(Rom_component const &);
		Rom_component &operator = (Rom_component const &);

		File *_file_ptr = nullptr;

	public:

		Rom_dataspace_capability dataspace() override {
			return static_cap_cast<Rom_dataspace>(_file_ptr->dataspace()); }

		void sigh(Signal_context_capability) override { }

		Rom_component(Genode::Env &env, Genode::Allocator &alloc,
		              File_cache &file_cache, Block::Connection<> &block,
		              char const *path)
		{
			file_cache.with_element(path,

				[&] (File &file) {
					log("cache hit for file ", path);
					_file_ptr = &file;
				},

				[&] {
					log("request for file ", path);
					_file_ptr = new (alloc) File(env, alloc, file_cache, block, path);
				});
		}
};


class Iso::Root : public Iso::Root_component
{
	private:

		Genode::Env       &_env;
		Genode::Allocator &_alloc;

		Allocator_avl       _block_alloc { &_alloc };
		Block::Connection<> _block       { _env, &_block_alloc };

		Genode::Io_signal_handler<Root>  sigh { _env.ep(), *this, &Root::_signal };

		/*
		 * Entries in the cache are never freed, even if the ROM session
		 * gets destroyed.
		 */
		File_cache _cache { };

		char _path[PATH_LENGTH];

		void _signal() { }

	protected:

		Rom_component *_create_session(const char *args) override
		{
			size_t ram_quota =
				Arg_string::find_arg(args, "ram_quota").ulong_value(0);
			/* account for opening a new file */
			size_t md_size = sizeof(File) + sizeof(File_info);
			if (md_size > ram_quota)
				throw Insufficient_ram_quota();

			Session_label const label = label_from_args(args);
			copy_cstring(_path, label.last_element().string(), sizeof(_path));

			_block.tx_channel()->sigh_ack_avail(sigh);
			_block.tx_channel()->sigh_ready_to_submit(sigh);

			if (verbose)
				Genode::log("Request for file ", Cstring(_path), " len ", strlen(_path));

			try {
				return new (_alloc) Rom_component(_env, _alloc, _cache, _block, _path);
			}
			catch (Io_error)       { throw Service_denied(); }
			catch (Non_data_disc)  { throw Service_denied(); }
			catch (File_not_found) { throw Service_denied(); }
		}

	public:

		Root(Genode::Env &env, Allocator &alloc)
		:
			Root_component(&env.ep().rpc_ep(), &alloc),
			_env(env), _alloc(alloc)
		{ }
};


struct Main
{
	Genode::Env  &_env;
	Genode::Heap  _heap { _env.ram(), _env.rm() };

	Iso::Root     _root { _env, _heap };

	Main(Genode::Env &env) : _env(env)
	{
		_env.parent().announce(_env.ep().manage(_root));
	}
};


void Component::construct(Genode::Env &env) { static Main main(env); }
