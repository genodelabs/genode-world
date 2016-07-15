/*
 * \brief  Override the libc's (weak) default implementation of getenv
 * \author Norman Feske
 * \date   2014-09-03
 */

/* libc includes */
#include <stdlib.h>

/* Genode includes */
#include <base/log.h>


extern "C" char *getenv(const char *name)
{
	Genode::log("environment variable \"", name, "\" requested");

	return (char *)"";
};
