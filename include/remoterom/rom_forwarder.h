/*
 * \brief  Interface used by the backend to transfer ROM content to the remote clients.
 * \author Johannes Schlatow
 * \date   2016-02-17
 */

#ifndef __INCLUDE__REMOTEROM__ROM_FORWARDER_H_
#define __INCLUDE__REMOTEROM__ROM_FORWARDER_H_

#include <base/stdint.h>

namespace Remoterom {
	using Genode::size_t;
	struct Rom_forwarder_base;
}

struct Remoterom::Rom_forwarder_base
{
	virtual const char *module_name() const = 0;
	virtual size_t content_size() const = 0;
	virtual size_t transfer_content(char *dst, size_t dst_len, size_t offset=0) const = 0;
};

#endif
