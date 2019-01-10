/*
 * Text logging abstraction.
 */

/*
 * This file is part of Tox, the free peer to peer instant messenger.
 *
 * Tox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef _TOXCORE__GENODE_LOGGER_H_
#define _TOXCORE__GENODE_LOGGER_H_


/* Libc includes */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Genode includes */
#include <base/log.h>


namespace Toxcore {

/* Toxcore includes */
extern "C" {
#include "toxcore/logger.h"
}

	static void logger_genode_handler(void *context, Logger_Level level,
		                              const char *file, int line, const char *func,
	                                  const char *message, void *userdata)
	{
		(void)context;
		(void)userdata;

		switch (level) {
			case LOGGER_LEVEL_ERROR:
				Genode::error(file, ":", line, "(", func, "): ", message);\
				break;
			case LOGGER_LEVEL_WARNING:
				Genode::warning(file, ":", line, "(", func, "): ", message);
				break;
			default:
				Genode::log(file, ":", line, "(", func, "): ", message);
				break;
		}
	}

	Logger *new_genode_logger()
	{
		Logger *l = logger_new();
		logger_callback_log(l,  logger_genode_handler, nullptr, nullptr);
		return l;
	}

}

#endif
