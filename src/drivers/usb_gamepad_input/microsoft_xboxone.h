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

#ifndef _MICROSOFT_XBOX_ONE_H_
#define _MICROSOFT_XBOX_ONE_H_

/* Genode includes */
#include <util/string.h>

/* local includes */
#include <utils.h>
#include <hid_device.h>


struct Microsoft_xboxone : Hid_device
{
	/*
	 * Supported devices
	 */
	struct {
		uint16_t vendor_id;
		uint16_t product_id;
	} devices[2] = {
		{0x045e, 0x02d1}, /* Microsoft XBox One wired controller */
		{0x045e, 0x02dd}  /* Microsoft XBox One wired controller (new FW) */
	};

	struct Report
	{
		uint8_t  cmd;
		uint8_t  reserved1; /* always 0x00 */
		uint8_t  packet_nr;
		uint8_t  reserved2; /* always 0x0e */
		uint16_t buttons;
		uint16_t  lt;
		uint16_t  rt;
		int16_t  x;
		int16_t  y;
		int16_t  z;
		int16_t  rz;
	}; __attribute__((packed));

	enum {
		IFACE_NUM = 0,
		ALT_NUM   = 0,
		EP_NUM    = 1,

		DATA_LENGTH = 18,
		DATA_OFFSET = 4, /* ignore cmd + packet_nr */

		B_NUM = 16,

		DATA_CMD = 0x20,

		AXIS_XY  = 0,
		AXIS_ZRZ = 1,
		AXIS_LT  = 2,
		AXIS_RT  = 3,
	};

	Input::Keycode b_mapping[B_NUM] = {
		Input::Keycode::KEY_UNKNOWN, /* 0x0001 */ /* unused */
		Input::Keycode::KEY_UNKNOWN, /* 0x0002 */ /* unused */
		Input::Keycode::BTN_START,   /* 0x0004 */
		Input::Keycode::BTN_SELECT,  /* 0x0008 */
		Input::Keycode::BTN_A,       /* 0x0010 */
		Input::Keycode::BTN_B,       /* 0x0020 */
		Input::Keycode::BTN_X,       /* 0x0040 */
		Input::Keycode::BTN_Y,       /* 0x0080 */
		Input::Keycode::BTN_FORWARD, /* 0x0100 */
		Input::Keycode::BTN_BACK,    /* 0x0200 */
		Input::Keycode::BTN_LEFT,    /* 0x0400 */
		Input::Keycode::BTN_RIGHT,   /* 0x0800 */
		Input::Keycode::BTN_TL,      /* 0x1000 */ /* LB */
		Input::Keycode::BTN_TR,      /* 0x2000 */ /* RB */
		Input::Keycode::BTN_THUMBL,  /* 0x4000 */
		Input::Keycode::BTN_THUMBR,  /* 0x8000 */
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
		log("cmd:       ", Hex(n->cmd),       " (", Hex(o->cmd),       ")");
		log("packet_nr: ", Hex(n->packet_nr), " (", Hex(o->packet_nr), ")");
		log("buttons:   ", Hex(n->buttons),   " (", Hex(o->buttons),   ")");
		log("lt:        ", Hex(n->lt),        " (", Hex(o->lt),        ")");
		log("rt:        ", Hex(n->rt),        " (", Hex(o->rt),        ")");
		log("x:         ", Hex(n->x),         " (", Hex(o->x),         ")");
		log("y:         ", Hex(n->y),         " (", Hex(o->y),         ")");
		log("z:         ", Hex(n->z),         " (", Hex(o->z),         ")");
		log("rz:        ", Hex(n->rz),        " (", Hex(o->rz),        ")");
	}

	Microsoft_xboxone(Input::Session_component &input_session)
	: Hid_device(input_session, "Microsoft Corp. Xbox One Controller") { }


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

		/* ignore the rest */
		if (n->cmd != DATA_CMD) { return; }

		if (len != DATA_LENGTH) {
			warning("drop invalid packet ", n->packet_nr);
			return;
		}

		Report const * const o = reinterpret_cast<Report const*>(last);

		bool const changed = memcmp(data+DATA_OFFSET, last+DATA_OFFSET, len-DATA_OFFSET);
		if (!changed) { return; }

		if (verbose) { dump_state(o, n); }
		/* check analog sticks */
		if (left_stick_enabled) {
			Utils::check_axis(input_session, o->x, n->x, o->y, n->y, AXIS_XY);
		}
		if (right_stick_enabled) {
			Utils::check_axis(input_session, o->z, n->z, o->rz, n->rz, AXIS_ZRZ);
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

#endif /* _MICROSOFT_XBOX_ONE_H_ */
