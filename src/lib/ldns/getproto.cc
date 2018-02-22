#include <netdb.h>

#include <base/log.h>
#include <util/string.h>

struct protoent *getprotobynumber(int proto)
{
	static protoent pe;

	Genode::error(__func__, "(", proto, ") not implemented");

	Genode::memset(&pe, 0x00, sizeof(pe));
	return &pe;
}
