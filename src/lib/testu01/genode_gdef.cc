#include <base/attached_rom_dataspace.h>

extern "C" {

#include <stdio.h>
#include <string.h>

void gdef_GetHostName (char machine[], int n)
{
	Genode::Attached_rom_dataspace config("config");
	try { config.xml().attribute("hostname").value(machine, n); }
	catch (...) { Genode::strncpy(machine, "genode", n); }
}

void gdef_WriteHostName (void) { }

}
