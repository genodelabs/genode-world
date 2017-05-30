extern "C" {

#include <stdio.h>
#include <string.h>

void gdef_GetHostName (char machine[], int n)
{
	strncpy(machine, "genode", n);
}

void gdef_WriteHostName (void) { }

}