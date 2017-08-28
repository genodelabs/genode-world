#include <testu01/genode_init.h>

#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <base/heap.h>
#include <base/log.h>

extern "C" {
#include "gdef.h"
#include "swrite.h"
#include "bbattery.h"
}

/* This is a big one */
Genode::size_t Libc::Component::stack_size() { return 64*1024*sizeof(Genode::addr_t); }


void Libc::Component::construct(Libc::Env &env)
{
	using namespace Genode;

	Attached_rom_dataspace config_rom(env, "config");
	Xml_node const config = config_rom.xml();

	enum {
		    MIN_NBITS = 500,
		DEFAULT_NBITS = 1U << 20
	};

	unsigned nbits = max((unsigned)MIN_NBITS,
	                     config.attribute_value("nbits", (unsigned)DEFAULT_NBITS));
	String<128> file;

	try {
		config.attribute("file").value(&file);
	} catch (...) {
		error("'file' not declared in config");
		env.parent().exit(~0);
		return;
	}

	Heap heap(env.pd(), env.rm());
	Timer::Connection timer(env);

	testu01_init(heap, timer);

	Libc::with_libc([&] () {
		bbattery_RabbitFile((char *)file.string(), nbits); });

	env.parent().exit(0);
}
