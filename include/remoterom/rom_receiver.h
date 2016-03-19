/*
 * \brief  Interface used by the backend to write the ROM data received from the remote server
 * \author Johannes Schlatow
 * \date   2016-02-18
 */

#ifndef __INCLUDE__REMOTEROM__ROM_RECEIVER_H_
#define __INCLUDE__REMOTEROM__ROM_RECEIVER_H_

#include <base/stdint.h>

namespace Remoterom {
	using Genode::size_t;
	struct Rom_receiver_base;
}

struct Remoterom::Rom_receiver_base
{
	virtual const char *module_name() const = 0;
	virtual char* start_new_content(size_t len) = 0;
	virtual void commit_new_content(bool abort=false) = 0;
};

#endif
