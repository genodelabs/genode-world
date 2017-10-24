/*
 * \brief  Genode signal support for Nim
 * \author Emery Hemingway
 * \date   2015-10-15
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


#ifndef _GENODE_CPP__SIGNALS_H_
#define _GENODE_CPP__SIGNALS_H_

#include <libc/component.h>
#include <base/signal.h>
#include <util/reconstructible.h>
#include <libc/component.h>

extern "C" void nimHandleSignal(void *arg);

struct SignalDispatcherCpp
{
	struct _Dispatcher : Genode::Signal_dispatcher_base
	{
		void *arg;

		void dispatch(unsigned num) override
		{
			Libc::with_libc([&] () {
				nimHandleSignal(arg); });
		}

		_Dispatcher(void *arg) : arg(arg) {
			 Genode::Signal_context::_level = Genode::Signal_context::Level::App; }
	};

	Genode::Constructible<_Dispatcher> _dispatcher;
	Genode::Entrypoint                 *entrypoint;
	Genode::Signal_context_capability   cap;

	void init(Genode::Entrypoint *ep, void *arg)
	{
		_dispatcher.construct(arg);
		entrypoint = ep;
		cap = entrypoint->manage(*_dispatcher);
	}

	void deinit()
	{
		entrypoint->dissolve(*_dispatcher);
		_dispatcher.destruct();
	}
};

#endif
