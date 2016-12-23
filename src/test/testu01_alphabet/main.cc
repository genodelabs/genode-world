#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <base/log.h>

extern "C" {
#include "bbattery.h"
}

void Libc::Component::construct(Genode::Env &env)
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

	bbattery_AlphabitFile((char *)file.string(), nbits);

	env.parent().exit(0);
}
