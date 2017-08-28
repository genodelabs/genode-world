#include <base/attached_rom_dataspace.h>

extern "C" {

#include <stdio.h>
#include <string.h>

void gdef_GetHostName (char machine[], int n)
{
	if (n) *machine = '\0';
}

void gdef_WriteHostName (void) { }

}
