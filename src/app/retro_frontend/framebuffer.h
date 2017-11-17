/*
 * \brief  Libretro framebuffer
 * \author Emery Hemingway
 * \date   2017-11-03
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__FRAMEBUFFER_H_
#define _RETRO_FRONTEND__FRAMEBUFFER_H_

/* Genode includes */
#include <framebuffer_session/connection.h>
#include <base/attached_dataspace.h>
#include <log_session/log_session.h>

#include "core.h"

namespace Retro_frontend { struct Framebuffer; }

struct Retro_frontend::Framebuffer
{
	::Framebuffer::Connection session;

	::Framebuffer::Mode mode;

	Genode::Attached_dataspace ds;

	Genode::Signal_handler<Framebuffer> mode_handler
		{ genv->ep(), *this, &Framebuffer::update_mode };

	void update_mode()
	{
		 mode = session.mode();

		/* shutdown if the framebuffer is reduced to nil */
		if ((mode.width() == 0) && (mode.height() == 0))
			Libc::with_libc([&] () { shutdown(); });
	}

	Framebuffer(retro_game_geometry geom)
	: session(*genv, ::Framebuffer::Mode(
	          geom.base_width, geom.base_height,
	          ::Framebuffer::Mode::RGB565)),
	  ds(genv->rm(), session.dataspace())
	{
		session.mode_sigh(mode_handler);
		update_mode();
	}
};

namespace Retro_frontend {
	static Genode::Constructible<Framebuffer> framebuffer;
}


void video_refresh_callback(const void *data,
                            unsigned src_width, unsigned src_height,
                            size_t src_pitch)
{
	using namespace Retro_frontend;
	using namespace Genode;

	if (!framebuffer.constructed() || data == NULL) return;

	uint8_t const *src = (uint8_t const*)data;
	uint8_t *dst = framebuffer->ds.local_addr<uint8_t>();

	unsigned const dst_width  = framebuffer->mode.width();
	unsigned const dst_height = framebuffer->mode.height();

	unsigned const width  = min(src_width,  dst_width);
	unsigned const height = min(src_height, dst_height);

	if (dst != src) {
		::size_t dst_pitch = dst_width<<1; /* x2 */

		for (unsigned y = 0; y < height; ++y)
			memcpy(&dst[y*dst_pitch], &src[y*src_pitch], dst_pitch);
	}

	framebuffer->session.refresh(0, 0, width, height);
}

#endif
