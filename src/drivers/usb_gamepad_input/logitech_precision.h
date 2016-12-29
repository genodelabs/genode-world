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

#ifndef _LOGITECH_PRECISION_H_
#define _LOGITECH_PRECISION_H_

/* Genode includes */
#include <util/string.h>

/* local includes */
#include <hid_device.h>


struct Logitech_precision : Hid_device
{
	/*
	 * Supported devices
	 */
	struct {
		uint16_t vendor_id;
		uint16_t product_id;
	} devices[1] = {
		{0x046d, 0xc21a}
	};

	struct Report
	{
		uint8_t  x; /* x-axis [1,255] */
		uint8_t  y; /* y-axis [1,255] */
		uint16_t b; /* 8 buttons */
	}; __attribute__((packed));

	enum {
		IFACE_NUM = 0,
		ALT_NUM   = 0,
		EP_NUM    = 0,

		DATA_LENGTH = 4,

		X = 0,
		Y = 1,

		ORIGIN  = 0x80,

		BUTTONS = 10,

		RIGHT_PRESSED = 0xff,
		DOWN_PRESSED  = 0xff,
	};

	Input::Keycode button_mapping[BUTTONS] = {
		Input::Keycode::BTN_X,      /* 0x01 */
		Input::Keycode::BTN_A,      /* 0x02 */
		Input::Keycode::BTN_B,      /* 0x04 */
		Input::Keycode::BTN_Y,      /* 0x08 */
		Input::Keycode::BTN_TL,     /* 0x10 */
		Input::Keycode::BTN_TR,     /* 0x20 */
		Input::Keycode::BTN_TL2,    /* 0x40 */
		Input::Keycode::BTN_TR2,    /* 0x80 */
		Input::Keycode::BTN_SELECT, /* 0x100 */
		Input::Keycode::BTN_START,  /* 0x200 */
	};

	uint8_t last[DATA_LENGTH] = {};

	Logitech_precision(Input::Session_component &input_session)
	: Hid_device(input_session, "Logitech, Inc. Precision Gamepad")
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

	void parse(uint8_t const *data, size_t len) override
	{
		using namespace Genode;

		if (len != DATA_LENGTH) {
			error("new data invalid");
			throw -1;
		}

		bool const changed = memcpy(last, data, len);
		if (!changed) { return; }

		Report const * const o = reinterpret_cast<Report const*>(last);
		Report const * const n = reinterpret_cast<Report const*>(data);

		/* x-axis */
		uint8_t const ox = last[X];
		uint8_t const nx = data[X];
		bool    const x  = (ox != nx);
		if (x) {
			/* now pressed if last was origin */
			bool const press = (ox == ORIGIN);

			bool left = true;

			if (press && !(nx < ox)) {
				left = false;
			}

			if (ox == RIGHT_PRESSED) { left = false; }

			Input::Event ev(press ? Input::Event::PRESS : Input::Event::RELEASE,
			                left  ? Input::Keycode::BTN_LEFT : Input::Keycode::BTN_RIGHT,
			                0, 0, 0, 0);
			input_session.submit(ev);
		}

		/* y-axis*/
		uint8_t const oy = last[Y];
		uint8_t const ny = data[Y];
		bool    const y  = (oy != ny);
		if (y) {
			/* now pressed if last was origin */
			bool const press = oy == ORIGIN;

			bool up = true;

			if (press && !(ny < oy)) {
				up = false;
			}

			if (oy == DOWN_PRESSED) { up = false; }

			Input::Event ev(press ? Input::Event::PRESS : Input::Event::RELEASE,
			                up    ? Input::Keycode::BTN_FORWARD : Input::Keycode::BTN_BACK,
			                0, 0, 0, 0);
			input_session.submit(ev);
		}

		/* check buttons */
		uint16_t const ob = o->b;
		uint16_t const nb = n->b;
		bool     const b  = (ob != nb);
		if (b) {
			Utils::check_buttons(input_session, ob, nb, BUTTONS, button_mapping);
		}

		/* save for next poll */
		Genode::memcpy(last, data, len);
	}

	uint8_t iface() const { return IFACE_NUM; }
	uint8_t    ep() const { return EP_NUM; }
	uint8_t   alt() const { return ALT_NUM; }
};

#endif /* _LOGITECH_PRECISION_H_ */
