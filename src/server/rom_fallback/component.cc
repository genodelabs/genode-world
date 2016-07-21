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

struct Rom_fallback::Main : Rpc_object<Typed_root<Rom_session>>
{
	struct Fallback_label;
	typedef List<Fallback_label> Fallback_labels;

	struct Fallback_label : Session_label, Fallback_labels::Element
	{
		using Session_label::Session_label;
	};

	Fallback_labels fallbacks;

	Env &env;

	Attached_rom_dataspace config_rom { env, "config" };

	Heap heap { env.ram(), env.rm() };
	Slab slab { sizeof(Fallback_label), 4096, &heap };

	Parent_service parent_service { "ROM" };

	void load_config()
	{
		while (Fallback_label *f = fallbacks.first()) {
			fallbacks.remove(f);
			destroy(slab, f);
		}

		/* keep a pointer to last element of the list for inserting */
		Fallback_label *last = nullptr;
		config_rom.xml().for_each_sub_node("fallback", [&] (Xml_node node) {
			typedef String<Session_label::capacity()> Label;

			Fallback_label *fallback = new (slab)
				Fallback_label(node.attribute_value("label", Label()).string());
			fallbacks.insert(fallback, last);
			last = fallback;
		});
	}

	void update_config()
	{
		config_rom.update();
		load_config();
	}

	Genode::Signal_handler<Main> update_handler
		{ env.ep(), *this, &Main::update_config };

	Main(Genode::Env &env) : env(env)
	{
		config_rom.sigh(update_handler);
		load_config();

		env.parent().announce(env.ep().manage(*this));
	}


	/****************************
	 ** Root session interface **
	 ****************************/

	Session_capability session(Session_args const &args,
	                           Affinity     const &affinity) override
	{
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
			try { return parent_service.session(new_args, affinity); }

			catch (Parent::Service_denied) {
				warning("'", new_label, "' was denied"); }

			catch (Service::Unavailable) {
				warning("'", new_label, "' is unavailable"); }

			catch (Service::Invalid_args)   {
				warning("'", new_label, "' received invalid args"); }

			catch (Service::Quota_exceeded) {
				warning("'", new_label, "' quota donation was insufficient"); }
		}

		error("no service found for ROM '", original.string(), "'");
		throw Root::Unavailable();
	}

	void upgrade(Session_capability, Upgrade_args const &args) override
	{
		warning("discarding rom upgrade '", args.string(), "'");
	}

	void close(Session_capability session) override
	{
		parent_service.close(session);
	}
};


void Component::construct(Genode::Env &env)
{
	static Rom_fallback::Main inst(env);
}