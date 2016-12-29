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

#ifndef _UTILS_H_
#define _UTILS_H_

/* Genode includes */
#include <base/log.h>
#include <usb/types.h>
#include <input/keycodes.h>
#include <input_session/connection.h>


namespace Utils {

	using namespace Genode;
	using namespace Usb;

	namespace Dump {

		void device(Device_descriptor &);
		void iface(Interface_descriptor &);
		void ep(Endpoint_descriptor &);
	}

	typedef signed short int16_t;
	int16_t convert_u8_to_s16(uint8_t);

	template <typename T> void check_buttons(Input::Session_component&,
	                   T const, T const, uint8_t const, Input::Keycode[]);

	void check_axis(Input::Session_component&,
	                int16_t const, int16_t const, int16_t const, int16_t const, int const);

	void check_hat(Input::Session_component &, uint8_t const, uint8_t const);
}

void Utils::Dump::device(Device_descriptor &descr)
{
	log("Device: "
	    "len: ",             Hex(descr.length),    " "
	    "type: " ,           Hex(descr.type),      " "
	    "class: ",           Hex(descr.dclass),    " "
	    "sub-class: ",       Hex(descr.dsubclass), " "
	    "proto: ",           Hex(descr.dprotocol), " "
	    "max_packet_size: ", Hex(descr.max_packet_size));
	log("        "
	    "vendor: ",      Hex(descr.vendor_id),  " "
	    "product: ",     Hex(descr.product_id), " "
	    "num_configs: ", Hex(descr.num_configs));
}


void Utils::Dump::iface(Interface_descriptor &descr)
{
	log("Interface: ",
	    "len: ",           Hex(descr.length),        " "
	    "type: ",          Hex(descr.type),          " "
	    "number: ",        Hex(descr.number),        " "
	    "alt_settings: ",  Hex(descr.alt_settings),  " "
	    "num_endpoints: ", Hex(descr.num_endpoints), " "
	    "iclass: ",        Hex(descr.iclass),        " "
	    "isubclass: ",     Hex(descr.isubclass),     " "
	    "iprotocol: ",     Hex(descr.iprotocol),     " "
	    "str_index: ",     Hex(descr.interface_index));
}


void Utils::Dump::ep(Endpoint_descriptor &descr)
{
	log("Endpoint: ",
	    "len: ",              Hex(descr.length),          " "
	    "type: ",             Hex(descr.type),            " "
	    "address: ",          Hex(descr.address),         " "
	    "attributes: ",       Hex(descr.attributes),      " "
	    "max_packet_size: ",  Hex(descr.max_packet_size), " "
	    "polling_interval: ", descr.polling_interval);
}


Utils::int16_t Utils::convert_u8_to_s16(uint8_t val)
{
	if (val == 0) { return -0x7fff; }

	return val * 0x0101 - 0x8000;
}


template <typename T>
void Utils::check_buttons(Input::Session_component &input_session,
                          T const prev, T const curr,
                          uint8_t const count, Input::Keycode mapping[])
{
	for (uint8_t i = 0; i < count; i++) {
		uint16_t const idx = 1u << i;

		if ((prev & idx) == (curr & idx)) { continue; }

		bool const press = !(prev & idx) && (curr & idx);

		Input::Event ev(press ? Input::Event::PRESS : Input::Event::RELEASE,
		                mapping[i], 0, 0, 0, 0);
		input_session.submit(ev);
	}
}


void Utils::check_axis(Input::Session_component &input_session,
                       int16_t const ox, int16_t const nx,
                       int16_t const oy, int16_t const ny,
                       int const axis)
{
	bool const x = (ox != nx);
	bool const y = (oy != ny);

	if (!x && !y) { return; }

	Input::Event ev(Input::Event::MOTION, axis, x ? nx : ox, y ? ny : oy , 0, 0);
	input_session.submit(ev);
}


void Utils::check_hat(Input::Session_component &input_session, uint8_t const o, uint8_t const n)
{
	static struct Axis_mapping
	{
		signed char x;
		signed char y;
	} hat_to_dpad[9] = {
		{  0, -1 }, /* N  */
		{  1, -1 }, /* NE */
		{  1,  0 }, /* E  */
		{  1,  1 }, /* SE */
		{  0,  1 }, /* S  */
		{ -1,  1 }, /* SW */
		{ -1,  0 }, /* W  */
		{ -1, -1 }, /* NW */
		{  0,  0 }, /* O  */
	};

	static Input::Keycode axis_keymap_x[3] = {
		Input::Keycode::BTN_LEFT,
		Input::Keycode::KEY_UNKNOWN,
		Input::Keycode::BTN_RIGHT,
	};

	static Input::Keycode axis_keymap_y[3] = {
		Input::Keycode::BTN_FORWARD,
		Input::Keycode::KEY_UNKNOWN,
		Input::Keycode::BTN_BACK,
	};

	Axis_mapping ao = hat_to_dpad[o];
	Axis_mapping an = hat_to_dpad[n];

	if (ao.x != an.x) {
		if (an.x != 0) {
			Input::Event ev(Input::Event::PRESS, axis_keymap_x[an.x+1], 0, 0, 0, 0);
			input_session.submit(ev);
		}
		if (ao.x != 0) {
			Input::Event ev(Input::Event::RELEASE, axis_keymap_x[ao.x+1], 0, 0, 0, 0);
			input_session.submit(ev);
		}
	}

	if (ao.y != an.y) {
		if (an.y != 0) {
			Input::Event ev(Input::Event::PRESS, axis_keymap_y[an.y+1], 0, 0, 0, 0);
			input_session.submit(ev);
		}

		if (ao.y != 0) {
			Input::Event ev(Input::Event::RELEASE, axis_keymap_y[ao.y+1], 0, 0, 0, 0);
			input_session.submit(ev);
		}
	}
}

#endif /* _UTILS_H_ */
