/*
 * \brief  Libretro frontend
 * \author Emery Hemingway
 * \date   2017-11-04
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__LOG_H_
#define _RETRO_FRONTEND__LOG_H_

/* Genode includes */
#include <base/log.h>

/* vsnprintf */
#include <stdio.h>

#include "core.h"

void log_printf_callback(retro_log_level level, const char *fmt, ...)
{
	using namespace Retro_frontend;

	char buf[Genode::Log_session::MAX_STRING_LEN];

	va_list vp;
	va_start(vp, fmt);
	int n = ::vsnprintf(buf, sizeof(buf), fmt, vp);
	va_end(vp);

	if (n)
		buf[n-1] = '\0'; /* trim newline */
	char const *msg = buf;

	switch (level) {
	case RETRO_LOG_DEBUG: Genode::log("Debug: ", msg); return;
	case RETRO_LOG_INFO:  Genode::log(msg);            return;
	case RETRO_LOG_WARN:  Genode::warning(msg);        return;
	case RETRO_LOG_ERROR: Genode::error(msg);          return;
	case RETRO_LOG_DUMMY: Genode::log("Dummy: ", msg); return;
	}
}

#endif
