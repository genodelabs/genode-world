/*
 * \brief  USB HID gamepad to Input session translator
 * \author Josef Soentgen
 * \date   2016-10-12
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/allocator_avl.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/log.h>
#include <base/heap.h>
#include <input/component.h>
#include <input/keycodes.h>
#include <input_session/connection.h>
#include <os/reporter.h>
#include <os/static_root.h>
#include <timer_session/connection.h>
#include <usb/types.h>
#include <usb_session/connection.h>

/* local includes */
#include <utils.h>
#include <hid_device.h>

/* include known gamepads */
#include <buffalo_snes.h>
#include <logitech_precision.h>
#include <microsoft_xbox360.h>
#include <microsoft_xboxone.h>
#include <retrolink_n64.h>
#include <sony_ds3.h>
#include <sony_ds4.h>
#include <gravis_gamepadpro.h>


static bool const verbose_intr = false;
static bool const verbose      = false;
static bool const debug        = false;
static bool const dump_dt      = false;


namespace Usb {
	using namespace Genode;

	struct Hid;
	struct Main;
}


/**********************************
 ** USB HID Input implementation **
 **********************************/

/**
 * USB HID
 */
struct Usb::Hid
{
	Env                      &env;
	Input::Session_component &input_session;

	/*
	 * Supported USB HID gamepads
	 */
	Hid_device         generic            { input_session };
	Buffalo_snes       buffalo_snes       { input_session };
	Logitech_precision logitech_precision { input_session };
	Microsoft_xbox360  xbox360            { input_session };
	Microsoft_xboxone  xboxone            { input_session };
	Retrolink_n64      retrolink_n64      { input_session };
	Sony_ds3           sony_ds3           { input_session };
	Sony_ds4           sony_ds4           { input_session };
	Gravis_gamepadpro  gravis_gamepadpro  { input_session };

	enum { MAX_DEVICES = 9, };
	Hid_device *devices[MAX_DEVICES] {
		&generic,
		&buffalo_snes,
		&logitech_precision,
		&retrolink_n64,
		&xbox360,
		&xboxone,
		&sony_ds3,
		&sony_ds4,
		&gravis_gamepadpro
	};

	Hid_device *device = &generic;

	Timer::Connection timer { env };

	unsigned polling_us = 0;

	void state_change()
	{
		if (usb.plugged()) {
			log("Gamepad plugged in");
			probe_device();
			return;
		}

		log("Gamepad unplugged");
	}

	/* construct before Usb::Connection so the dispatcher is valid */
	Signal_handler<Hid> state_dispatcher { env.ep(), *this, &Hid::state_change };

	Allocator_avl    usb_alloc;
	Usb::Connection  usb { env, &usb_alloc, "usb_gamepad", 32*1024, state_dispatcher };

	Usb::Config_descriptor    config_descr;
	Usb::Device_descriptor    device_descr;
	Usb::Interface_descriptor iface_descr;
	Usb::Endpoint_descriptor  ep_descr;

	void handle_alt_setting(Packet_descriptor &p) { warning(__func__, ": not implemented"); }

	void handle_config_packet(Packet_descriptor &p) { _claim_device(); }

	struct Hid_report
	{
		struct Invalid_report : Genode::Exception { };
	};

	Hid_report hid_report;

	Hid_report parse_hid_report(uint8_t const *r, size_t len)
	{
		if (dump_dt) {
			log("HID report dump:");
			/* using i+=4 is save since we use a 256 byte buffer */
			for (size_t i = 0; i < len; i+=4) {
				log(Hex(r[i], Hex::PREFIX, Hex::PAD),
				    " ", Hex(r[i+1], Hex::PREFIX, Hex::PAD),
				    " ", Hex(r[i+2], Hex::PREFIX, Hex::PAD),
				    " ", Hex(r[i+3], Hex::PREFIX, Hex::PAD));
			}
		}

		return Hid_report();
	}

	struct Hid_report_descriptor
	{
		Usb::Hid &hid;

		Hid_report_descriptor(Usb::Hid &hid) : hid(hid) { }

		void request()
		{
			enum {
				USB_REQUEST_TO_HOST         = 0x80,
				USB_REQUEST_RCPT_IFACE      = 0x01,
				USB_REQUEST_GET_DESCRIPTOR  = 0x06,
				USB_REQUEST_DT              = 0x22,

				REQUEST = USB_REQUEST_TO_HOST | USB_REQUEST_RCPT_IFACE,
			};

			/* XXX read size from interface */
			Usb::Packet_descriptor p = hid.alloc_packet(256);

			p.type                 = Usb::Packet_descriptor::CTRL;
			p.control.request      = USB_REQUEST_GET_DESCRIPTOR;
			p.control.request_type = REQUEST;
			p.control.value        = USB_REQUEST_DT<<8;
			p.control.index        = 0;
			p.control.timeout      = 1000;

			/* alloc_packet checks readiness */
			hid.usb.source()->submit_packet(p);
		}
	};

	Hid_report_descriptor hid_report_descr { *this };

	void handle_ctrl(Packet_descriptor &p)
	{
		uint8_t const * const data = (uint8_t*)usb.source()->packet_content(p);
		size_t           const len = p.control.actual_size > 0
		                           ? p.control.actual_size : 0;

		try {
			/* XXX only parse HID report if not already known */
			hid_report = parse_hid_report(data, len);
		} catch (Hid_report::Invalid_report) {
			error("HID report is invalid");
			return;
		}

		/* kick-off polling */
		timer.trigger_once(polling_us);
	}

	void handle_irq_packet(Packet_descriptor &p)
	{
		if (!p.read_transfer()) { return; }

		uint8_t const * const data = (uint8_t*)usb.source()->packet_content(p);
		size_t           const len = p.transfer.actual_size > 0
		                           ? p.transfer.actual_size : 0;

		try {
			device->parse(data, len);
		}
		catch (...) {
			error("input data is invalid, reconnect device");
			return;
		}

		/* keep on r^Wpolling */
		if (polling_us) { timer.trigger_once(polling_us); }
	}

	struct String_descr
	{
		Usb::Hid &hid;

		char const * const name;

		enum { MAX_STRING_LENGTH = 128, };
		char string[MAX_STRING_LENGTH] {};

		uint8_t index = 0xff; /* hopefully invalid */

		String_descr(Usb::Hid &hid, char const *name) : hid(hid), name(name) { }

		void request(uint8_t i)
		{
			index = i;

			Usb::Packet_descriptor p = hid.alloc_packet(MAX_STRING_LENGTH);

			p.type          = Usb::Packet_descriptor::STRING;
			p.string.index  = index;
			p.string.length = MAX_STRING_LENGTH;

			hid.usb.source()->submit_packet(p);
		}
	};

	void handle_string_packet(Usb::Packet_descriptor &p)
	{
		String_descr *s = nullptr;
		if (p.string.index == manufactorer_string.index) {
			s = &manufactorer_string;
		} else if (p.string.index == product_string.index) {
			s = &product_string;
		} else if (p.string.index == serial_number_string.index) {
			s = &serial_number_string;
		}

		if (!s) { return; }

		uint16_t const * const u = (uint16_t*)usb.source()->packet_content(p);

		if (p.string.length < 0) { p.string.length = 0; }
		int const len = min((unsigned)p.string.length,
		                    (unsigned)String_descr::MAX_STRING_LENGTH - 1);

		for (int i = 0; i < len; i++) { s->string[i] = u[i] & 0xff; }
		s->string[len] = 0;

		log(s->name, ": ", (char const*)s->string);
	}

	String_descr manufactorer_string  { *this, "Manufactorer" };
	String_descr product_string       { *this, "Product" };
	String_descr serial_number_string { *this, "Serial_number" };

	struct Completion : Usb::Completion
	{
		enum State { VALID, FREE, CANCELED };
		State state = FREE;

		void complete(Usb::Packet_descriptor &p) override { }

		void complete(Usb::Hid &hid, Usb::Packet_descriptor &p)
		{
			if (state != VALID)
				return;

			if (!p.succeded) {
				/*
				 * We might end up here b/c the generic driver was used and a vendor
				 * did not fill the queried string index. It is, however, more likely
				 * that a IRQ or CTRL packet failed. For better or worse that is quite
				 * often the case when a gamepad is unplugged. Therefore we never print
				 * any error in case of a failure. If something goes wrong, one has to
				 * debug anyway...
				 */
				return;
			}

			switch (p.type) {
				case Usb::Packet_descriptor::IRQ:         hid.handle_irq_packet(p);    break;
				case Usb::Packet_descriptor::CTRL:        hid.handle_ctrl(p);          break;
				case Usb::Packet_descriptor::STRING:      hid.handle_string_packet(p); break;
				case Usb::Packet_descriptor::CONFIG:      hid.handle_config_packet(p); break;
				case Usb::Packet_descriptor::ALT_SETTING: hid.handle_alt_setting(p);   break;
				/* ignore other packets */
				case Usb::Packet_descriptor::BULK:
				default: break;
			}
		}
	} completions[Usb::Session::TX_QUEUE_SIZE];

	void ack_avail()
	{
		while (usb.source()->ack_avail()) {
			Usb::Packet_descriptor p = usb.source()->get_acked_packet();
			dynamic_cast<Completion *>(p.completion)->complete(*this, p);
			free_packet(p);
		}
	}

	Signal_handler<Hid> ack_avail_dispatcher { env.ep(), *this, &Hid::ack_avail };

	void probe_device()
	{
		try {
			usb.config_descriptor(&device_descr, &config_descr);
		} catch (Usb::Session::Device_not_found) {
			error("cound not read config descriptor");
			throw -1;
		}

		for (int i = 1; i < MAX_DEVICES; i++) {
			bool const found = devices[i]->probe(device_descr.vendor_id,
			                                     device_descr.product_id);
			if (found) {
				device = devices[i];
				log("Driver found for device: ", device->name);
				break;
			}
		}

		if (device == &generic) {
			warning("no matching driver found, falling back to generic driver");
		}

		Usb::Packet_descriptor p = alloc_packet(0);

		p.type   = Packet_descriptor::CONFIG;
		p.number = 1; /* XXX read from device */

		usb.source()->submit_packet(p);
	}

	bool _claim_device()
	{
		try {
			usb.config_descriptor(&device_descr, &config_descr);
			if (verbose) { Utils::Dump::device(device_descr); }
		} catch (Usb::Session::Device_not_found) {
			error("cound not read config descriptor");
			return false;
		}

		uint8_t const iface = device->iface();
		uint8_t const   alt = device->alt();
		uint8_t const    ep = device->ep();

		try { usb.claim_interface(iface); }
		catch (Usb::Session::Interface_already_claimed) {
			error("could not claim device");
			return false;
		}

		try {
			usb.interface_descriptor(iface, alt, &iface_descr);
			if (verbose) { Utils::Dump::iface(iface_descr); }
		} catch (Usb::Session::Interface_not_found) {
			error("could not read interface descriptor");
			return false;
		}

		try {
			usb.endpoint_descriptor(iface, alt, ep, &ep_descr);
			if (verbose) { Utils::Dump::ep(ep_descr); }
		} catch (Usb::Session::Interface_not_found) {
			error("could not read endpoint descriptor");
			return false;
		}

		polling_us = 1000 * ep_descr.polling_interval;

		if (debug) { polling_us = 1000 * 1000; }

		/*
		 * Request HID report descriptor here because certain devices,
		 * e.g., XBox 360 controller, will not respond otherwise.
		 */
		hid_report_descr.request();

		/*
		 * Execute device specific quirk method which in most cases simply
		 * sends an USB packet to get the device in working order.
		 */
		if (device == &xboxone) {
			log("Enable XBox One quirk");

			Usb::Endpoint_descriptor ep_out_descr;
			usb.endpoint_descriptor(0, 0, 0, &ep_out_descr);

			Usb::Packet_descriptor p = alloc_packet(5);

			p.type                      = Usb::Packet_descriptor::IRQ;
			p.transfer.ep               = ep_out_descr.address;
			p.transfer.polling_interval = 100;

			uint8_t *data = reinterpret_cast<uint8_t*>(usb.source()->packet_content(p));
			data[0] = 0x05;
			data[1] = 0x20;
			data[2] = 0x00; /* serial */
			data[3] = 0x00;
			data[4] = 0x00;

			usb.source()->submit_packet(p);
		} else if (device == &sony_ds3) {
			log("Enable DS3 quirk");

			enum {
				USB_REQUEST_TO_DEVICE       = 0x00,
				USB_REQUEST_TYPE_CLASS      = 0x20,
				USB_REQUEST_RCPT_IFACE      = 0x01,
				USB_REQUEST_GET_DESCRIPTOR  = 0x06,

				USB_HID_REQUEST_GET_REPORT = 0x01,
				USB_HID_REQUEST_SET_REPORT = 0x09,
				USB_HID_FEATURE_REPORT     = 0x02,

				DS3_REPORT = 0xf4,

				REQUEST = USB_REQUEST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RCPT_IFACE,
			};

			uint8_t const cmd[4] = { 0x42, 0x0C, 0x00, 0x00 };

			Usb::Packet_descriptor p = alloc_packet(sizeof(cmd));

			uint8_t *data = reinterpret_cast<uint8_t*>(usb.source()->packet_content(p));
			Genode::memcpy(data, cmd, sizeof(cmd));

			p.type                 = Usb::Packet_descriptor::CTRL;
			p.control.request      = USB_HID_REQUEST_SET_REPORT;
			p.control.request_type = REQUEST;
			p.control.value        = ((USB_HID_FEATURE_REPORT + 1) << 8) | DS3_REPORT;
			p.control.index        = 0;
			p.control.timeout      = 1000;

			usb.source()->submit_packet(p);
		}

		/* if we do not know the device, at least query vendor information */
		if (device == &generic) {
			manufactorer_string.request(device_descr.manufactorer_index);
			product_string.request(device_descr.product_index);
			serial_number_string.request(device_descr.serial_number_index);
		}

		return true;
	}

	void handle_polling()
	{
		Usb::Packet_descriptor p = alloc_packet(ep_descr.max_packet_size);

		p.type                      = Usb::Packet_descriptor::IRQ;
		p.succeded                  = false;
		p.transfer.ep               = ep_descr.address;
		p.transfer.polling_interval = 10;

		usb.source()->submit_packet(p);
	}

	Signal_handler<Hid> polling_dispatcher = {
		env.ep(), *this, &Hid::handle_polling };

	Completion *_alloc_completion()
	{
		for (unsigned i = 0; i < Usb::Session::TX_QUEUE_SIZE; i++)
			if (completions[i].state == Completion::FREE) {
				completions[i].state = Completion::VALID;
				return &completions[i];
			}

		return nullptr;
	}

	struct Queue_full         : Genode::Exception { };
	struct No_completion_free : Genode::Exception { };

	Usb::Packet_descriptor alloc_packet(int length)
	{
		if (!usb.source()->ready_to_submit()) { throw Queue_full(); }

		Usb::Packet_descriptor p = usb.source()->alloc_packet(length);

		p.completion = _alloc_completion();
		if (!p.completion) {
			usb.source()->release_packet(p);
			throw No_completion_free();
		}

		return p;
	}

	void free_packet(Usb::Packet_descriptor &packet)
	{
		dynamic_cast<Completion *>(packet.completion)->state = Completion::FREE;
		usb.source()->release_packet(packet);
	}

	/**
	 * Constructor
	 *
	 * \param env    environment
	 * \param alloc  allocator used by Usb::Connection
	 * \param input  Input session
	 */
	Hid(Env &env, Genode::Allocator &alloc, Input::Session_component &input)
	: env(env), input_session(input), usb_alloc(&alloc)
	{
		usb.tx_channel()->sigh_ack_avail(ack_avail_dispatcher);

		timer.sigh(polling_dispatcher);

		/* HID gets initialized by state_change() */
	}
};


struct Usb::Main
{
	Env  &env;
	Heap  heap { env.ram(), env.rm() };

	Input::Session_component    input_session { env, env.pd() };
	Static_root<Input::Session> input_root { env.ep().manage(input_session) };

	Usb::Hid hid { env, heap, input_session };

	Main(Env &env) : env(env)
	{
		input_session.event_queue().enabled(true);
		env.parent().announce(env.ep().manage(input_root));
	}
};


void Component::construct(Genode::Env &env) { static Usb::Main main(env); }
