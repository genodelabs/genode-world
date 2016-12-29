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

#ifndef _BUFFALO_SNES_H_
#define _BUFFALO_SNES_H_

/* Genode includes */
#include <util/string.h>

/* local includes */
#include <hid_device.h>

struct Buffalo_snes : Hid_device
{
	/*
	 * Supported devices
	 */
	struct {
		uint16_t vendor_id;
		uint16_t product_id;
	} devices[1] = {
		{0x0583, 0x2060}
	};

	enum {
		IFACE_NUM = 0,
		ALT_NUM   = 0,
		EP_NUM    = 0,

		DATA_LENGTH = 8,

		X = 0,
		Y = 1,
		B = 2,

		ORIGIN        = 0x80,
		LEFT_PRESSED  = 0x00,
		RIGHT_PRESSED = 0xff,
		UP_PRESSED    = 0x00,
		DOWN_PRESSED  = 0xff,
	};

	Input::Keycode button_mapping[8] = {
		Input::Keycode::BTN_A,      /* 0x01 */
		Input::Keycode::BTN_B,      /* 0x02 */
		Input::Keycode::BTN_X,      /* 0x03 */
		Input::Keycode::BTN_Y,      /* 0x04 */
		Input::Keycode::BTN_TL,     /* 0x05 */
		Input::Keycode::BTN_TR,     /* 0x06 */
		Input::Keycode::BTN_SELECT, /* 0x07 */
		Input::Keycode::BTN_START,  /* 0x08 */
		/* clear / turbo buttons omitted */
	};

	uint8_t last[DATA_LENGTH] = {};

	/* XXX instead checking for false positives check absolute values */
	bool false_positive(uint8_t o, uint8_t n)
	{
		return (o == 0x7f && n == 0x80) || (o == 0x80 && n == 0x7f);
	}

	Buffalo_snes(Input::Session_component &input_session)
	: Hid_device(input_session, "iBuffalo classic USB gamepad (SNES)")
	{
		/* initial values */
		last[X] = ORIGIN;
		last[Y] = ORIGIN;
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

	void parse(uint8_t const *new_data, size_t len) override
	{
		using namespace Genode;

		if (len != DATA_LENGTH) {
			error("new data invalid");
			throw -1;
		}

		bool const changed = memcmp(last, new_data, len) != 0;

		if (!changed) { return; }

		/* x-axis */
		if (last[X] != new_data[X]) {
			if (false_positive(last[X], new_data[X])) { return; }

			/* now pressed if last was origin */
			bool const press = last[X] == ORIGIN;

			bool left = true;

			if (press && !(new_data[X] < last[X])) {
				left = false;
			}

			if (last[X] == RIGHT_PRESSED) { left = false; }

			Input::Event ev(press ? Input::Event::PRESS : Input::Event::RELEASE,
			                left  ? Input::Keycode::BTN_LEFT : Input::Keycode::BTN_RIGHT,
			                0, 0, 0, 0);
			input_session.submit(ev);
		}

		/* y-axis*/
		if (last[Y] != new_data[Y]) {
			if (false_positive(last[Y], new_data[Y])) { return; }

			/* now pressed if last was origin */
			bool const press = last[Y] == ORIGIN;

			bool up = true;

			if (press && !(new_data[Y] < last[Y])) {
				up = false;
			}

			if (last[Y] == DOWN_PRESSED) { up = false; }

			Input::Event ev(press ? Input::Event::PRESS : Input::Event::RELEASE,
			                up    ? Input::Keycode::BTN_FORWARD : Input::Keycode::BTN_BACK,
			                0, 0, 0, 0);
			input_session.submit(ev);
		}

		if (last[B] != new_data[B]) {
			uint8_t const prev = last[B];
			uint8_t const curr = new_data[B];

			for (int i = 0; i < 8; i++) {
				uint8_t const idx = 1u << i;

				if ((prev & idx) == (curr & idx)) { continue; }

				bool const press = !(prev & idx) && (curr & idx);

				Input::Event ev(press ? Input::Event::PRESS : Input::Event::RELEASE,
				                button_mapping[i], 0, 0, 0, 0);
				input_session.submit(ev);
			}
		}

		/* save for next poll */
		Genode::memcpy(last, new_data, len);
	}

	uint8_t iface() const { return IFACE_NUM; }
	uint8_t    ep() const { return EP_NUM; }
	uint8_t   alt() const { return ALT_NUM; }
};

#endif /* _BUFFALO_SNES_H_ */
