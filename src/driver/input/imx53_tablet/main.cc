/**
 * \brief  Input driver front-end
 * \author Norman Feske
 * \author Christian Helmuth
 * \author Stefan Kalkowski
 * \date   2006-08-30
 */

/*
 * Copyright (C) 2006-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/component.h>
#include <base/env.h>
#include <base/rpc_server.h>
#include <platform_session/connection.h>
#include <input/component.h>
#include <input/root.h>

/* local includes */
#include <driver.h>

using namespace Genode;


struct Main
{
	Genode::Env &env;

	Input::Session_component session  { env, env.ram() };
	Input::Root_component    root     { env.ep().rpc_ep(), session };
	Input::Tablet_driver     driver   { env, session.event_queue() };

	Main(Genode::Env &env) : env(env) {
		env.parent().announce(env.ep().manage(root)); }
};


void Component::construct(Genode::Env &env) { static Main main(env); }
