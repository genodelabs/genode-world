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

#ifndef _SONY_DS3_H_
#define _SONY_DS3_H_

/* Genode includes */
#include <util/string.h>

/* local includes */
#include <utils.h>
#include <hid_device.h>


struct Sony_ds3 : Hid_device
{
	/*
	 * Supported devices
	 */
	struct {
		uint16_t vendor_id;
		uint16_t product_id;
	} devices[2] = {
		{0x054c, 0x0268}, /* Sony Corp. Batoh Device / PlayStation 3 Controller */
	};

	struct Report
	{
		uint8_t  rid;
		uint8_t  unused1;
		uint16_t buttons;
		uint8_t  ps_button; /* currently unused */
		uint8_t  unused2;
		uint8_t  x;
		uint8_t  y;
		uint8_t  z;
		uint8_t  rz;
		uint8_t  unused3[8];
		uint8_t  lt;
		uint8_t  rt;
	}; __attribute__((packed));

	enum {
		IFACE_NUM = 0,
		ALT_NUM   = 0,
		EP_NUM    = 1,

		DATA_LENGTH = 49,

		B_NUM = 16,

		DATA_CMD = 0x20,

		AXIS_XY  = 0,
		AXIS_ZRZ = 1,
		AXIS_LT  = 2,
		AXIS_RT  = 3,
	};

#if 0
	data [0] rid
	data [1] 0x0
	data [2] = 0x01 select, 0x08 start buttons, 0x10 DU, 0x20 DR, 0x40 DD, 0x80 DL
	data [3] = 0x01 L2, 0x02 R2, 0x04 L1, 0x08 R1, 0x10 t, 0x20 o, 0x40 x, 0x80 s buttons
	data [4] = 0x01 PS button
	data [6] = x
	data [7] = y
	data [8] = z
	data [9] = rz
	data [18] = LT
	data [19] = RT
#endif

	Input::Keycode b_mapping[B_NUM] = {
		Input::Keycode::BTN_SELECT,  /* 0x0001 */
		Input::Keycode::BTN_THUMBL,  /* 0x0002 */
		Input::Keycode::BTN_THUMBR,  /* 0x0004 */
		Input::Keycode::BTN_START,   /* 0x0008 */
		Input::Keycode::BTN_FORWARD, /* 0x0010 */
		Input::Keycode::BTN_RIGHT,   /* 0x0020 */
		Input::Keycode::BTN_BACK,    /* 0x0040 */
		Input::Keycode::BTN_LEFT,    /* 0x0080 */
		Input::Keycode::BTN_TL2,     /* 0x0100 */
		Input::Keycode::BTN_TR2,     /* 0x0200 */
		Input::Keycode::BTN_TL,      /* 0x0400 */
		Input::Keycode::BTN_TR,      /* 0x0800 */
		Input::Keycode::BTN_Y,       /* 0x1000 */
		Input::Keycode::BTN_B,       /* 0x2000 */
		Input::Keycode::BTN_A,       /* 0x4000 */
		Input::Keycode::BTN_X,       /* 0x8000 */
	};

	typedef signed short int16_t;

	uint8_t last[DATA_LENGTH] = {};

	bool left_stick_enabled  = true;
	bool right_stick_enabled = true;

	bool verbose = false;

	void dump_state(Report const * const o, Report const * const n)
	{
		using namespace Genode;

		log("dump state:");
		log("buttons:   ", Hex(n->buttons),   " (", Hex(o->buttons),   ")");
		log("ps_button: ", Hex(n->ps_button), " (", Hex(o->ps_button), ")");
		log("x:         ", Hex(n->x),         " (", Hex(o->x),         ")");
		log("y:         ", Hex(n->y),         " (", Hex(o->y),         ")");
		log("z:         ", Hex(n->z),         " (", Hex(o->z),         ")");
		log("rz:        ", Hex(n->rz),        " (", Hex(o->rz),        ")");
		log("lt:        ", Hex(n->lt),        " (", Hex(o->lt),        ")");
		log("rt:        ", Hex(n->rt),        " (", Hex(o->rt),        ")");
	}

	Sony_ds3(Input::Session_component &input_session)
	: Hid_device(input_session, "Sony Corp. PlayStation(R) 3 Controller") { }


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

		bool const changed = memcmp(data, last, len);
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

		/* check buttons */
		uint16_t const ob = o->buttons;
		uint16_t const nb = n->buttons;
		if (ob != nb) { Utils::check_buttons(input_session, ob, nb, B_NUM, b_mapping); }

		/* save for next poll */
		Genode::memcpy(last, data, len);
	}

	uint8_t iface() const { return IFACE_NUM; }
	uint8_t    ep() const { return EP_NUM; }
	uint8_t   alt() const { return ALT_NUM; }
};

#endif /* _SONY_DS3_H_ */
