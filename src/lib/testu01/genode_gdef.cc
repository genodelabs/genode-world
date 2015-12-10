#include <os/config.h>

extern "C" {

#include <stdio.h>

void gdef_GetHostName (char machine[], int n)
{
	try { Genode::config()->xml_node().attribute("hostname").value(machine, n); }
	catch (...) { Genode::strncpy(machine, "genode", n); }
}

void gdef_WriteHostName (void)
{
	enum { MAXBYTES = 255 };

	char machine[1 + MAXBYTES] = {'\0'};
	gdef_GetHostName(machine, MAXBYTES);
	printf("%s\n", machine);
}

}