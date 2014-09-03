/*
 * \brief  Override the libc's (weak) default implementation of getenv
 * \author Norman Feske
 * \date   2014-09-03
 */

/* libc includes */
#include <stdlib.h>

/* Genode includes */
#include <base/printf.h>


extern "C" char *getenv(const char *name)
{
	PINF("environment variable \"%s\" requested", name);

	return (char *)"";
};
