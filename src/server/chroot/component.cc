/*
 * \brief  Change session root server
 * \author Emery Hemingway
 * \date   2016-03-10
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <file_system/util.h>
#include <file_system_session/connection.h>
#include <os/path.h>
#include <os/session_policy.h>
#include <root/root.h>
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <base/rpc_server.h>
#include <base/service.h>
#include <base/allocator_avl.h>
#include <base/heap.h>

using namespace Genode;


template <unsigned MAX_LEN>
static void path_from_label(Path<MAX_LEN> &path, char const *label)
{
	char tmp[MAX_LEN];
	size_t len = strlen(label);

	size_t i = 0;
	for (size_t j = 1; j < len; ++j) {
		if (!strcmp(" -> ", label+j, 4)) {
			path.append("/");

			strncpy(tmp, label+i, (j-i)+1);
			/* rewrite any directory seperators */
			for (size_t k = 0; k < MAX_LEN; ++k)
				if (tmp[k] == '/')
					tmp[k] = '_';
			path.append(tmp);

			j += 4;
			i = j;
		}
	}
	path.append("/");
	strncpy(tmp, label+i, MAX_LEN);
	/* rewrite any directory seperators */
	for (size_t k = 0; k < MAX_LEN; ++k)
		if (tmp[k] == '/')
			tmp[k] = '_';
	path.append(tmp);
}

struct Proxy : Rpc_object<Typed_root<File_system::Session>>
{
	Genode::Attached_rom_dataspace _config_rom;
	Parent_service          _parent_service;
	Heap                    _heap;
	Allocator_avl           _fs_tx_block_alloc { &_heap };
	File_system::Connection _fs { _fs_tx_block_alloc, 1024 };

	/**
	 * Constructor
	 */
	Proxy(Genode::Env &env)
	:
		_config_rom(env, "config"),
		_parent_service("File_system"),
		_heap(env.ram(), env.rm())
	{
		env.parent().announce(env.ep().rpc_ep().manage(this));
	}

	Session_capability chroot(char const *args, char const *path, Affinity const &affinity)
	{
		enum { ARGS_MAX_LEN = 256 };
		char new_args[ARGS_MAX_LEN];

		strncpy(new_args, args, ARGS_MAX_LEN);
		Arg_string::set_arg_string(new_args, ARGS_MAX_LEN, "root", path);

		try { return _parent_service.session(new_args, affinity); }
		catch (Service::Invalid_args)   { throw Root::Invalid_args();   }
		catch (Service::Quota_exceeded) { throw Root::Quota_exceeded(); }
		catch (...) { }
		throw Root::Unavailable();
	}


	/********************
	 ** Root interface **
	 ********************/

	Session_capability session(Root::Session_args const &session_args,
	                           Affinity           const &affinity)
	{
		enum { MAX_LEN = 128 };
		char tmp[MAX_LEN];
		Path<MAX_LEN> root_path;

		Session_label label = label_from_args(session_args.string());
		char const *label_str = label.string();

		try {
			Session_policy policy(label, _config_rom.xml());

			if (policy.has_attribute("label_prefix")
			 && policy.attribute_value("merge", false))
			{
				/* merge at the next element */
				size_t offset = policy.attribute("label_prefix").value_size();
				for (size_t i = offset; i < label.length()-4; ++i) {
					if (strcmp(label_str+i, " -> ", 4))
						continue;

					strncpy(tmp, label_str, min(sizeof(tmp), i+1));
					label_str = tmp;
					break;
				}
			}

		} catch (Session_policy::No_policy_defined) { }
		path_from_label(root_path, label_str);

		Arg_string::find_arg(session_args.string(), "root").string(
			tmp, sizeof(tmp), "/");
		root_path.append_element(tmp);
		root_path.remove_trailing('/');

		char const *args = session_args.string();
		char const *new_root = root_path.base();

		using namespace File_system;
		Dir_handle handle;
		char const *errstr;
		try {
			_fs.close(ensure_dir(_fs, new_root));
			return chroot(args, new_root, affinity);
		}

		catch (Node_already_exists) { return chroot(args, new_root, affinity); }

		catch (Permission_denied)   { errstr = "permission denied"; }
		catch (Name_too_long)       { errstr = "new root too long"; }
		catch (No_space)            { errstr = "no space";          }
		catch (...)                 { errstr = "unknown error";     }

		Genode::error(new_root, ": ", errstr);
		throw Root::Unavailable();
	}

	void upgrade(Session_capability        cap,
	             Root::Upgrade_args const &args) override {
		_parent_service.upgrade(cap, args.string()); }

	void close(Session_capability cap) override {
		_parent_service.close(cap); }
};


/***************
 ** Component **
 ***************/

Genode::size_t Component::stack_size() { return 2*1024*sizeof(Genode::addr_t); }

void Component::construct(Genode::Env &env) { static Proxy inst(env); }
