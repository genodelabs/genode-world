/*
 * \brief  Service fallback server
 * \author Emery Hemingway
 * \date   2016-07-21
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <rom_session/capability.h>
#include <base/service.h>
#include <base/heap.h>
#include <base/session_label.h>
#include <base/component.h>

namespace Rom_fallback {
	using namespace Genode;
	struct Main;
}

struct Rom_fallback::Main
{
	struct Fallback_label;
	typedef List<Fallback_label> Fallback_labels;

	struct Fallback_label : Session_label, Fallback_labels::Element
	{
		using Session_label::Session_label;
	};

	Fallback_labels fallbacks;

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

	Id_space<Parent::Server> server_id_space;

	Env &env;

	Attached_rom_dataspace config_rom       { env, "config" };
	Attached_rom_dataspace session_requests { env, "session_requests" };

	Heap heap { env.ram(), env.rm() };

	void load_config()
	{
		while (Fallback_label *f = fallbacks.first()) {
			fallbacks.remove(f);
			destroy(heap, f);
		}

		/* keep a pointer to last element of the list for inserting */
		Fallback_label *last = nullptr;
		config_rom.xml().for_each_sub_node("fallback", [&] (Xml_node node) {
			typedef String<Session_label::capacity()> Label;

			Fallback_label *fallback = new (heap)
				Fallback_label(node.attribute_value("label", Label()).string());
			fallbacks.insert(fallback, last);
			last = fallback;
		});
	}

	void handle_session_request(Xml_node request);

	void handle_session_requests()
	{
		if (config_sig_rec.pending()) {
			do { config_sig_rec.pending_signal(); }
			while (config_sig_rec.pending());
			config_rom.update();
			load_config();
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

	Main(Genode::Env &env) : env(env)
	{
		load_config();

		config_rom.sigh(config_sig_rec.manage(&config_sig_ctx));
		session_requests.sigh(session_request_handler);

		/* handle requests that have queued before or during construction */
		handle_session_requests();
	}

	~Main() {
		config_sig_rec.dissolve(&config_sig_ctx); }
};


void Rom_fallback::Main::handle_session_request(Xml_node request)
{
	if (!request.has_attribute("id"))
		return;

	Parent::Server::Id const server_id { request.attribute_value("id", 0UL) };

	if (request.has_type("create")) {

		if (!request.has_sub_node("args"))
			return;

		typedef Session_state::Args Args;
		Args const args = request.sub_node("args").decoded_content<Args>();

		Session_label const original = label_from_args(args.string());

		enum { ARGS_MAX_LEN = 256 };
		char new_args[ARGS_MAX_LEN];

		for (Fallback_label *f = fallbacks.first(); f; f = f->next()) {
			Session_label &prefix = *f;

			/* create new label */
			Session_label const new_label = prefix == "" ?
				original : prefixed_label(prefix, original);

			/* create a new argument set */
			strncpy(new_args, args.string(), sizeof(new_args));
			Arg_string::set_arg_string(new_args, sizeof(new_args), "label",
			                           new_label.string());

			/* try this service */
			Session *session = nullptr;
			try {
				session = new (heap)
					Session(env.id_space(), server_id_space, server_id);

				Affinity aff;
				Session_capability cap = 
					env.session("ROM", session->client_id.id(), new_args, aff);

				env.parent().deliver_session_cap(server_id, cap);
				return;
			}

			catch (Parent::Service_denied) {
				warning("'", new_label, "' was denied"); }

			catch (Service::Unavailable) {
				warning("'", new_label, "' is unavailable"); }

			catch (Service::Invalid_args)   {
				warning("'", new_label, "' received invalid args"); }

			catch (Service::Quota_exceeded) {
				warning("'", new_label, "' quota donation was insufficient"); }

			if (session)
				destroy(heap, session);
		}

		error("no service found for ROM '", original.string(), "'");
		env.parent().session_response(server_id, Parent::INVALID_ARGS);
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


void Component::construct(Genode::Env &env)
{
	static Rom_fallback::Main inst(env);

	env.parent().announce("ROM");
}