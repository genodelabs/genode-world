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
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <parent/parent.h>
#include <base/service.h>
#include <base/allocator_avl.h>
#include <base/heap.h>

namespace Chroot {
	using namespace Genode;
	struct Main;

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
}


struct Chroot::Main
{
	enum { PATH_MAX_LEN = 128 };
	typedef Genode::Path<PATH_MAX_LEN> Path;

	struct Session : Parent::Server
	{
		Parent::Client parent_client;

		Id_space<Parent::Client>::Element client_id;
		Id_space<Parent::Server>::Element server_id;

		Session(Id_space<Parent::Client> &client_space,
		        Id_space<Parent::Server> &server_space,
		        Parent::Server::Id server_id)
		:
			client_id(parent_client, client_space),
			server_id(*this, server_space, server_id) { }
	};

	Genode::Env &env;

	Id_space<Parent::Server> server_id_space;

	Heap heap { env.ram(), env.rm() };

	Allocator_avl           fs_tx_block_alloc { &heap };
	File_system::Connection fs { fs_tx_block_alloc, 1 };

	Attached_rom_dataspace session_requests { env, "session_requests" };

	Attached_rom_dataspace config_rom { env, "config" };

	void handle_session_request(Xml_node request);

	void handle_session_requests()
	{
		if (config_sig_rec.pending()) {
			do { config_sig_rec.pending_signal(); }
			while (config_sig_rec.pending());
			config_rom.update();
		}

		session_requests.update();

		Xml_node const requests = session_requests.xml();

		requests.for_each_sub_node([&] (Xml_node request) {
			handle_session_request(request);
		});
	}

	Signal_handler<Main> session_request_handler {
		env.ep(), *this, &Main::handle_session_requests };

	Signal_context  config_sig_ctx;
	Signal_receiver config_sig_rec;

	/**
	 * Constructor
	 */
	Main(Genode::Env &env) : env(env)
	{
		config_rom.sigh(config_sig_rec.manage(&config_sig_ctx));
		session_requests.sigh(session_request_handler);

		/* handle requests that have queued before or during construction */
		handle_session_requests();
	}

	~Main() {
		config_sig_rec.dissolve(&config_sig_ctx); }

	Session_capability request_session(Parent::Client::Id  const &id,
	                                   Session_state::Args const &args)
	{
		char tmp[PATH_MAX_LEN];
		Path root_path;

		Session_label label = label_from_args(args.string());
		char const *label_str = label.string();

		/* if policy specifies a merge, use a truncated label */
		try {
			Session_policy policy(label, config_rom.xml());

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

		/* create a new path from the label */
		path_from_label(root_path, label_str);

		/* extract and append the orginal root */
		Arg_string::find_arg(args.string(), "root").string(
			tmp, sizeof(tmp), "/");
		root_path.append_element(tmp);
		root_path.remove_trailing('/');

		char const *new_root = root_path.base();

		using namespace File_system;

		/* create the new root directory if it is missing */
		try { fs.close(ensure_dir(fs, new_root)); }
		catch (Node_already_exists) { }
		catch (Permission_denied)   {
			Genode::error(new_root,": permission denied"); throw; }
		catch (Name_too_long)       {
			Genode::error(new_root,": new root too long"); throw; }
		catch (No_space)            {
			Genode::error(new_root,": no space");          throw; }
		catch (...)                 {
			Genode::error(new_root,": unknown error");     throw; }

		/* rewrite the root session argument */
		enum { ARGS_MAX_LEN = 256 };
		char new_args[ARGS_MAX_LEN];

		strncpy(new_args, args.string(), ARGS_MAX_LEN);

		/* sacrifice the label to make space for the root argument */
		Arg_string::remove_arg(new_args, "label");

		Arg_string::set_arg_string(new_args, ARGS_MAX_LEN, "root", new_root);

		Affinity affinity;
		return env.session("File_system", id, new_args, affinity);
	}
};


void Chroot::Main::handle_session_request(Xml_node request)
{
	if (!request.has_attribute("id"))
		return;

	Parent::Server::Id const server_id { request.attribute_value("id", 0UL) };

	if (request.has_type("create")) {

		if (!request.has_sub_node("args"))
			return;

		typedef Session_state::Args Args;
		Args const args = request.sub_node("args").decoded_content<Args>();

		Session *session = nullptr;
		try {
			session = new (heap)
				Session(env.id_space(), server_id_space, server_id);

			Session_capability cap = request_session(session->client_id.id(), args);

			env.parent().deliver_session_cap(server_id, cap);
		}

		catch (...) {
			if (session)
				destroy(heap, session);
			env.parent().session_response(server_id, Parent::INVALID_ARGS);
		}
	}

	if (request.has_type("upgrade")) {

		server_id_space.apply<Session>(server_id, [&] (Session &session) {

			size_t ram_quota = request.attribute_value("ram_quota", 0UL);

			char buf[64];
			snprintf(buf, sizeof(buf), "ram_quota=%ld", ram_quota);

			// XXX handle Root::Invalid_args
			env.upgrade(session.client_id.id(), buf);
			env.parent().session_response(server_id, Parent::SESSION_OK);
		});
	}

	if (request.has_type("close")) {
		server_id_space.apply<Session>(server_id, [&] (Session &session) {
			env.close(session.client_id.id());
			destroy(heap, &session);
			env.parent().session_response(server_id, Parent::SESSION_CLOSED);
		});
	}
}


/***************
 ** Component **
 ***************/

Genode::size_t Component::stack_size() {
	return 2*1024*sizeof(Genode::addr_t); }

void Component::construct(Genode::Env &env)
{
	static Chroot::Main inst(env);
	env.parent().announce("File_system");
}
