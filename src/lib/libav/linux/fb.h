/*
 * \brief   Minimal definition of a Linux framebuffer device
 * \author  Martin Stein
 * \date    2016-02-10
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _LINUX__FB_H_
#define _LINUX__FB_H_

/* Genode includes */
#include <base/fixed_stdint.h>

/**
 * IOCTLs for Linux framebuffer devices
 */
enum {
	FBIOGET_VSCREENINFO=17920,
	FBIOGET_FSCREENINFO=17922,
};


/**
 * Single channel descriptor within a pixel descriptor
 */
struct fb_bitfield
{
	genode_uint32_t offset; /* bit offset in pixel descriptor */
};

/**
 * Dynamic configuration of a framebuffer device file
 */
struct fb_var_screeninfo
{
	genode_uint32_t xres;           /* visible resolution, width */
	genode_uint32_t yres;           /* visible resolution, height */
	genode_uint32_t xoffset;        /* X offset from virtual to visible res. */
	genode_uint32_t yoffset;        /* Y offset from virtual to visible res. */
	genode_uint32_t bits_per_pixel; /* length of a pixel descriptor */
	struct fb_bitfield red;         /* red channel bits in a pixel descr. */
	struct fb_bitfield green;       /* green channel bits in a pixel descr. */
	struct fb_bitfield blue;        /* blue channel bits in a pixel descr. */
};

/**
 * Fixed configuration of a framebuffe device file
 */
struct fb_fix_screeninfo
{
	unsigned long smem_start;    /* base of framebuffer */
	genode_uint32_t smem_len;    /* length of framebuffer in bytes */
	genode_uint32_t line_length; /* length of a framebuffer line in bytes */
};

#endif /* _INCLUDE__VFS__FB_H_ */
