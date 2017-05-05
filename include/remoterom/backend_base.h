/*
 * \brief  Backend interface used by the Remoterom client and server
 * \author Johannes Schlatow
 * \date   2016-02-17
 */

#ifndef __INCLUDE__REMOTEROM__BACKEND_BASE_H_
#define __INCLUDE__REMOTEROM__BACKEND_BASE_H_

#include <rom_forwarder.h>
#include <rom_receiver.h>

namespace Remoterom {
	struct Backend_server_base;
	struct Backend_client_base;

	class Exception : public ::Genode::Exception { };
	Backend_server_base &backend_init_server();
	Backend_client_base &backend_init_client();
};

struct Remoterom::Backend_server_base
{
	virtual void send_update() = 0;
	virtual void register_forwarder(Rom_forwarder_base *forwarder) = 0;
};

struct Remoterom::Backend_client_base
{
	virtual void register_receiver(Rom_receiver_base *receiver) = 0;
};

#endif
