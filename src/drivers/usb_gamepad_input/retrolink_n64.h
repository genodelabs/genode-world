/*
 * \brief  USB HID to Input translator
 * \author Josef Soentgen
 * \date   2016-10-12
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETROLINK_N64_H_
#define _RETROLINK_N64_H_

/* Genode includes */
#include <util/string.h>

/* local includes */
#include <utils.h>
#include <hid_device.h>


struct Retrolink_n64 : Hid_device
{
	/*
	 * Supported devices
	 */
	struct {
		uint16_t vendor_id;
		uint16_t product_id;
	} devices[1] = {
		{0x0079, 0x0006}
	};

	/*
	 * The actual report data coming from the device does not correspond
	 * to the HID report, unless I missed something. The digipad and
	 * the camera buttons are in fact reported in byte 6, 4Bit each.
	 */
	struct Report
	{
		uint8_t x; /* analog x-axis [0,255] */
		uint8_t y; /* analog y-axis [0,255] */
		uint8_t z; /* - */
		uint8_t w; /* - */
		uint8_t v; /* - */
		uint8_t h; /* high nibble pad, low nibble camera */
		uint8_t b; /* 6 buttons (l, r, z, a, b, start) */
		uint8_t r; /* - */
	}; __attribute__((packed));

	enum {
		IFACE_NUM = 0,
		ALT_NUM   = 0,
		EP_NUM    = 0,

		DATA_LENGTH = 8,

		X = 0,
		Y = 1,
		H = 5,
		B = 6,

		AXIS_XY = 0,

		ORIGIN     = 0x80,
		HAT_ORIGIN = 0x0f,

		B_NUM = 6,
		C_NUM = 4,
		H_NUM = 8,
	};

	Input::Keycode b_mapping[B_NUM] = {
		Input::Keycode::BTN_TL,     /* 0x01 */
		Input::Keycode::BTN_TR,     /* 0x02 */
		Input::Keycode::BTN_A,      /* 0x04 */
		Input::Keycode::BTN_Z,      /* 0x08 */
		Input::Keycode::BTN_B,      /* 0x10 */
		Input::Keycode::BTN_START,  /* 0x20 */
	};

	Input::Keycode c_mapping[C_NUM] = {
		Input::Keycode::BTN_0,  /* 0x01 */
		Input::Keycode::BTN_1,  /* 0x02 */
		Input::Keycode::BTN_2,  /* 0x04 */
		Input::Keycode::BTN_3,  /* 0x08 */
	};

	char const *h_name(uint8_t v)
	{
		switch (v) {
		case 0: return "BTN_FORWARD";
		case 2: return "BTN_RIGHT";
		case 4: return "BTN_BACK";
		case 6: return "BTN_LEFT";
		}

		return "<unknown button>";
	}

	uint8_t last[DATA_LENGTH] = {};

	Retrolink_n64(Input::Session_component &input_session)
	: Hid_device(input_session, "Retrolink N64 gamepad")
	{

		/* initial values */
		last[X] = ORIGIN;
		last[Y] = ORIGIN;
		last[H] = HAT_ORIGIN;
	}


	/**************************
	 ** HID device interface **
	 **************************/

	bool probe(uint16_t vendor_id, uint16_t product_id) const override
	{
		for (size_t i = 0; i < sizeof(devices)/sizeof(devices[0]); i++) {
			if (   vendor_id  == devices[i].vendor_id
				&& product_id == devices[i].product_id) {
				return true;
			}
		}
		return false;
	}

	void parse(uint8_t const *data, size_t len) override
	{
		using namespace Genode;

		if (len != DATA_LENGTH) {
			error("new data invalid");
			throw -1;
		}

		Report const * const o = reinterpret_cast<Report const*>(last);
		Report const * const n = reinterpret_cast<Report const*>(data);

		bool const changed = (n->x != o->x)
		                   ||(n->y != o->y)
		                   ||(n->h != o->h)
		                   ||(n->b != o->b);
		if (!changed) { return; }

		// log("dump state:");
		// log("x: ", Hex(n->x), " (", Hex(o->x), ")");
		// log("y: ", Hex(n->y), " (", Hex(o->y), ")");
		// log("h: ", Hex(n->h), " (", Hex(o->h), ")");
		// log("b: ", Hex(n->b), " (", Hex(o->b), ")");

		/* check analog */
		uint8_t const ox = o->x;
		uint8_t const nx = n->x;
		uint8_t const oy = o->y;
		uint8_t const ny = n->y;

		int16_t const oxv = Utils::convert_u8_to_s16(ox);
		int16_t const nxv = Utils::convert_u8_to_s16(nx);
		int16_t const oyv = Utils::convert_u8_to_s16(oy);
		int16_t const nyv = Utils::convert_u8_to_s16(ny);

		Utils::check_axis(input_session, oxv, nxv, oyv, nyv, AXIS_XY);

		/* check digipad */
		uint8_t const od = o->h & 0x0f;
		uint8_t const nd = n->h & 0x0f;
		if (od != nd) { Utils::check_hat(input_session, od, nd); }

		/* check camera buttons */
		uint8_t const oc = (o->h >> 4) & 0x0f;
		uint8_t const nc = (n->h >> 4) & 0x0f;
		if (oc != nc) { Utils::check_buttons(input_session, oc, nc, C_NUM, c_mapping); }

		/* check buttons */
		uint8_t const ob = o->b;
		uint8_t const nb = n->b;
		if (o->b != n->b) { Utils::check_buttons(input_session, ob, nb, B_NUM, b_mapping); }

		/* save for next poll */
		Genode::memcpy(last, data, len);
	}

	uint8_t iface() const { return IFACE_NUM; }
	uint8_t    ep() const { return EP_NUM; }
	uint8_t   alt() const { return ALT_NUM; }
};

#endif /* _RETROLINK_N64_H_ */
