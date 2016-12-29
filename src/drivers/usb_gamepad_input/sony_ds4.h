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

#ifndef _SONY_DS4_H_
#define _SONY_DS4_H_

/* Genode includes */
#include <util/string.h>

/* local includes */
#include <utils.h>
#include <hid_device.h>


struct Sony_ds4 : Hid_device
{
	/*
	 * Supported devices
	 */
	struct {
		uint16_t vendor_id;
		uint16_t product_id;
	} devices[2] = {
		{0x054c, 0x05c4}, /* Sony Corp. */
	};

	struct Report
	{
		uint8_t  rid;
		uint8_t  x;
		uint8_t  y;
		uint8_t  z;
		uint8_t  rz;
		uint8_t  dbuttons;
		uint8_t  buttons;
		uint8_t  ps_button;
		uint8_t  lt;
		uint8_t  rt;
		/*
		uint8_t  unused2[3];
		uint8_t  bat_level;
		*/
	}; __attribute__((packed));

	enum {
		IFACE_NUM = 0,
		ALT_NUM   = 0,
		EP_NUM    = 0, /* first IRQ EP IN */

		DATA_LENGTH = 64,

		B_NUM = 16,

		DATA_CMD = 0x20,

		AXIS_XY  = 0,
		AXIS_ZRZ = 1,
		AXIS_LT  = 2,
		AXIS_RT  = 3,
	};

#if 0
	data [0] rid
	data [1] x
	data [2] y
	data [3] z
	data [4] rz
	data [5] 0x10 square, 0x20 cross, 0x40 circle, 0x80 triangle
	         dpad: 7=NW 6=W 5=SW 4=S 3=SE 2=E 1=NE 0=N
	data [6] 0x01 L1, 0x02 R1, 0x04 L2, 0x08 R2, 0x10 SH, 0x20 OPT, 0x40 L3, 0x80 R3
	data [7] 0x01 PS button
	data [8] L2 trigger
	data [9] R2 trigger
	data [12] BAT level
#endif

	/*
	 * The buttons are ordered after the reshuffling of data:
	 * v = (dbuttons & 0xf0) | (ps_button & 0x01) | (buttons << 8)
	 */
	Input::Keycode b_mapping[B_NUM] = {
		Input::Keycode::BTN_MODE,    /* 0x0001 */
		Input::Keycode::KEY_UNKNOWN, /* 0x0002 */
		Input::Keycode::KEY_UNKNOWN, /* 0x0004 */
		Input::Keycode::KEY_UNKNOWN, /* 0x0008 */
		Input::Keycode::BTN_X,       /* 0x0010 */
		Input::Keycode::BTN_A,       /* 0x0020 */
		Input::Keycode::BTN_B,       /* 0x0040 */
		Input::Keycode::BTN_Y,       /* 0x0080 */
		Input::Keycode::BTN_TL,      /* 0x0100 */
		Input::Keycode::BTN_TR,      /* 0x0200 */
		Input::Keycode::BTN_TL2,     /* 0x0400 */
		Input::Keycode::BTN_TR2,     /* 0x0800 */
		Input::Keycode::BTN_SELECT,  /* 0x1000 */
		Input::Keycode::BTN_START,   /* 0x2000 */
		Input::Keycode::BTN_THUMBL,  /* 0x4000 */
		Input::Keycode::BTN_THUMBR,  /* 0x8000 */
	};

	typedef signed short int16_t;

	uint8_t last[DATA_LENGTH] = {};

	bool left_stick_enabled  = false;
	bool right_stick_enabled = false;

	bool verbose = false;

	void dump_state(Report const * const o, Report const * const n)
	{
		using namespace Genode;

		log("dump state:");
		log("dbuttons:  ", Hex(n->dbuttons),  " (", Hex(o->dbuttons),  ")");
		log("buttons:   ", Hex(n->buttons),   " (", Hex(o->buttons),   ")");
		log("ps_button: ", Hex(n->ps_button), " (", Hex(o->ps_button), ")");
		log("x:         ", Hex(n->x),         " (", Hex(o->x),         ")");
		log("y:         ", Hex(n->y),         " (", Hex(o->y),         ")");
		log("z:         ", Hex(n->z),         " (", Hex(o->z),         ")");
		log("rz:        ", Hex(n->rz),        " (", Hex(o->rz),        ")");
		log("lt:        ", Hex(n->lt),        " (", Hex(o->lt),        ")");
		log("rt:        ", Hex(n->rt),        " (", Hex(o->rt),        ")");
	}

	Sony_ds4(Input::Session_component &input_session)
	: Hid_device(input_session, "Sony Corp. PlayStation(R) 4 Controller")
	{
		last[5] = 0x08;
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

		Report const * const n = reinterpret_cast<Report const*>(data);

		if (len != DATA_LENGTH) { return; }

		Report const * const o = reinterpret_cast<Report const*>(last);

		/* ignore ever changing report counter */
		Report * const hack = reinterpret_cast<Report *>(const_cast<uint8_t*>(data));
		hack->ps_button &= 0x01;

		/* we only care about the actual input data */
		bool const changed = memcmp(data, last, sizeof(Report));
		if (!changed) { return; }

		if (verbose) { dump_state(o, n); }

		/* check analog sticks */
		if (left_stick_enabled) {
			uint8_t const ox = o->x;
			uint8_t const nx = n->x;
			uint8_t const oy = o->y;
			uint8_t const ny = n->y;

			int16_t const oxv = Utils::convert_u8_to_s16(ox);
			int16_t const nxv = Utils::convert_u8_to_s16(nx);
			int16_t const oyv = Utils::convert_u8_to_s16(oy);
			int16_t const nyv = Utils::convert_u8_to_s16(ny);

			Utils::check_axis(input_session, oxv, nxv, oyv, nyv, AXIS_XY);
		}

		if (right_stick_enabled) {
			uint8_t const oz  = o->z;
			uint8_t const nz  = n->z;
			uint8_t const orz = o->rz;
			uint8_t const nrz = n->rz;

			int16_t const ozv  = Utils::convert_u8_to_s16(oz);
			int16_t const nzv  = Utils::convert_u8_to_s16(nz);
			int16_t const orzv = Utils::convert_u8_to_s16(orz);
			int16_t const nrzv = Utils::convert_u8_to_s16(nrz);

			Utils::check_axis(input_session, ozv, nzv, orzv, nrzv, AXIS_ZRZ);
		}

		/*
		 * check analog triggers
		 *
		 * XXX not sure if 1024 -> [-2^15,2^15] is a good idea
		 */
		uint8_t const olt = o->lt;
		uint8_t const nlt = n->lt;
		if (olt != nlt) {
			int16_t const oltv = Utils::convert_u8_to_s16(olt);
			int16_t const nltv = Utils::convert_u8_to_s16(nlt);
			Utils::check_axis(input_session, oltv, nltv, 0, 0, AXIS_LT);
		}

		uint16_t const ort = o->rt;
		uint16_t const nrt = n->rt;
		if (ort != nrt) {
			int16_t const ortv = Utils::convert_u8_to_s16(ort);
			int16_t const nrtv = Utils::convert_u8_to_s16(nrt);
			Utils::check_axis(input_session, ortv, nrtv, 0, 0, AXIS_RT);
		}

		uint8_t const od = o->dbuttons & 0x0f;
		uint8_t const nd = n->dbuttons & 0x0f;
		if (od != nd) { Utils::check_hat(input_session, od, nd); }

		/* check buttons */
		uint16_t const ob = (o->dbuttons & 0xf0) | (o->ps_button & 0x01) | (o->buttons << 8);
		uint16_t const nb = (n->dbuttons & 0xf0) | (n->ps_button & 0x01) | (n->buttons << 8);
		if (ob != nb) { Utils::check_buttons(input_session, ob, nb, B_NUM, b_mapping); }

		/* save for next poll */
		Genode::memcpy(last, data, len);
	}

	uint8_t iface() const { return IFACE_NUM; }
	uint8_t    ep() const { return EP_NUM; }
	uint8_t   alt() const { return ALT_NUM; }
};

#endif /* _SONY_DS4_H_ */
