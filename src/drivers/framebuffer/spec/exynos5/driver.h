/*
 * \brief  Framebuffer driver for Exynos5 HDMI
 * \author Martin Stein
 * \date   2013-08-09
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _DRIVER_H_
#define _DRIVER_H_

/* Genode includes */
#include <base/component.h>
#include <base/stdint.h>
#include <base/log.h>

namespace Framebuffer
{
	using namespace Genode;

	/**
	 * Framebuffer driver
	 */
	class Driver;
}

class Framebuffer::Driver
{
	public:

		enum Output { OUTPUT_LCD, OUTPUT_HDMI };

	private:

		Genode::Env &_env;

		size_t _fb_width;
		size_t _fb_height;

	public:

		/**
		 * Constructor
		 */
		Driver(Genode::Env &env)
		:
			_env(env),
			_fb_width(0),
			_fb_height(0)
		{ }

		/**
		 * Return amount of bytes that is used for one pixel
		 */
		static size_t bytes_per_pixel()
		{
			return 4; /* 32-bit RGB */
		}

		/**
		 * Return size of framebuffer in bytes
		 *
		 * \param width   width of framebuffer in pixel
		 * \param height  height of framebuffer in pixel
		 *
		 * \retval  0  failed
		 * \retval >0  succeeded
		 */
		size_t buffer_size(size_t width, size_t height)
		{
			return bytes_per_pixel() * width * height;
		}

		/**
		 * Initialize driver for HDMI output
		 *
		 * \param width    width of screen and framebuffer in pixel
		 * \param height   height of screen and framebuffer in pixel
		 * \param fb_phys  physical base of framebuffer
		 *
		 * \retval -1  failed
		 * \retval  0  succeeded
		 */
		int init(size_t width, size_t height, addr_t fb_phys);
};

#endif /* _DRIVER_H_ */
