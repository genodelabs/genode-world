/*
 * \brief  Gravis Gamepad Pro USB
 * \author Emery Hemingway
 * \date   2017-11-12
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _GRAVIS_GAMEPADPRO_H_
#define _GRAVIS_GAMEPADPRO_H_

/* Genode includes */
#include <util/string.h>

/* local includes */
#include <hid_device.h>


struct Gravis_gamepadpro : Hid_device
{
	/*
	 * Supported devices
	 */
	struct {
		uint16_t vendor_id;
		uint16_t product_id;
	} devices[1] = {
		{0x0428, 0x4001}
	};

	enum { B_NUM = 8 };
	Input::Keycode butt_map[B_NUM] = {
		Input::Keycode::BTN_Y,   /* 0x01 Red    */
		Input::Keycode::BTN_B,   /* 0x02 Yellow */
		Input::Keycode::BTN_A,   /* 0x04 Green  */
		Input::Keycode::BTN_X,   /* 0x08 Blue   */
		Input::Keycode::BTN_TL,  /* 0x10 */
		Input::Keycode::BTN_TR,  /* 0x20 */
		Input::Keycode::BTN_TL2, /* 0x40 */
		Input::Keycode::BTN_TR2, /* 0x80 */
	};

	Input::Keycode ss_map[2] = {
		Input::Keycode::BTN_SELECT, /* 0x01 */
		Input::Keycode::BTN_START,  /* 0x02 */
	};

	enum {
		IFACE_NUM = 0,
		ALT_NUM   = 0,
		EP_NUM    = 0,

		DATA_LENGTH = 4,
	};

	uint8_t old_data[DATA_LENGTH] = { };

	Gravis_gamepadpro(Input::Session_component &input_session)
	: Hid_device(input_session, "Gravis Gamepad Pro") { }

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

	void parse(uint8_t const *new_data, size_t len) override
	{
		using namespace Genode;

		if (len < DATA_LENGTH) return;

		/* submit D-pad events */
		{
			uint8_t const xo = old_data[0];
			uint8_t const yo = old_data[1];
			uint8_t const xn = new_data[0];
			uint8_t const yn = new_data[1];

			if (xo != xn) {
				Input::Event ev(
					xn == 0x7f ? Input::Event::RELEASE : Input::Event::PRESS,
					(xn & xo) ? Input::Keycode::BTN_RIGHT : Input::Keycode::BTN_LEFT,
					0, 0, 0, 0);
				input_session.submit(ev);
			}
			if (yo != yn) {
				Input::Event ev(
					yn == 0x7f ? Input::Event::RELEASE : Input::Event::PRESS,
					(yn & yo) ? Input::Keycode::BTN_BACK : Input::Keycode::BTN_FORWARD,
					0, 0, 0, 0);
				input_session.submit(ev);
			}
		}

		/* submit button events */
		Utils::check_buttons(
			input_session, old_data[2], new_data[2], B_NUM, butt_map);

		/* Submit start/select events */
		Utils::check_buttons(
			input_session, old_data[3], new_data[3], 2, ss_map);

		/* save for next poll */
		Genode::memcpy(old_data, new_data, DATA_LENGTH);

		/* Simple! And it was only $1.99! */
	}

	uint8_t iface() const { return IFACE_NUM; }
	uint8_t    ep() const { return EP_NUM; }
	uint8_t   alt() const { return ALT_NUM; }
};

#endif /* _GRAVIS_GAMEPADPRO_H_ */
