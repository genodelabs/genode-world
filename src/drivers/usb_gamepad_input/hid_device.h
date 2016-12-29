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

#ifndef _HID_DEVICE_H_
#define _HID_DEVICE_H_

/* Genode includes */
#include <base/fixed_stdint.h>
#include <util/string.h>


struct Hid_device
{
	typedef Genode::uint8_t  uint8_t;
	typedef Genode::uint16_t uint16_t;
	typedef Genode::size_t   size_t;
	typedef signed short     int16_t;

	enum { MAX_DATA = 256, };
	uint8_t data[MAX_DATA] = {};

	typedef Genode::String<64> Name;
	Name name { "<generic USB HID gamepad>" };

	Input::Session_component &input_session;

	Hid_device(Input::Session_component &input_session, Name const &name)
	: name(name), input_session(input_session) { }

	Hid_device(Input::Session_component &input_session)
	: input_session(input_session) { }

	virtual ~Hid_device() { }

	/**************************
	 ** HID device interface **
	 **************************/

	virtual bool probe(uint16_t vendor_id, uint16_t product_id) const
	{
		Genode::warning(__func__, "(): not implemented");
		return false;
	}

	virtual void parse(uint8_t const *new_data, size_t len)
	{
		using namespace Genode;

		if (MAX_DATA < len) {
			warning("limit data len: ", len, " to: ", (int)MAX_DATA);
			len = MAX_DATA;
		}

		log("generic USB HID dump data:");
		for (size_t i = 0; i < len; i++) {
			log(Hex(i), ": ", Hex(new_data[i]), " (", Hex(data[i]), ")");
		}

		/* save for next poll */
		Genode::memcpy(data, new_data, len);
	}

	virtual uint8_t iface() const
	{
		Genode::warning(__func__, "(): not implemented, returning 0");
		return 0;
	}

	virtual uint8_t ep() const
	{
		Genode::warning(__func__, "(): not implemented, returning 0");
		return 0;
	}

	virtual uint8_t alt() const
	{
		Genode::warning(__func__, "(): not implemented, returning 0");
		return 0;
	}
};

#endif /* _HID_DEVICE_H_ */
