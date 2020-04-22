/*
 * \brief  Pulse width modulation
 * \author Stefan Kalkowski
 * \date   2020-04-22
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/attached_io_mem_dataspace.h>
#include <base/component.h>
#include <base/env.h>
#include <util/mmio.h>

struct Pwm : Genode::Mmio
{
	enum Clk_src { OFF, IPG, IPG_HIGHFREQ, IPG_32K };

	struct Control : Register<0x0, 32>
	{
		struct Enable       : Bitfield<0,  1> {};
		struct Clock_source : Bitfield<16, 2> {};
		struct Dbgen        : Bitfield<22, 1> {};
		struct Waiten       : Bitfield<23, 1> {};
		struct Dozen        : Bitfield<24, 1> {};
		struct Stopen       : Bitfield<25, 1> {};
	};

	struct Sample  : Register<0xc, 32> {};
	struct Period  : Register<0x10,32> {};

	Pwm(Genode::addr_t const mmio_base,
	    unsigned period,
	    unsigned sample,
	    Clk_src clk_src)
	: Genode::Mmio(mmio_base)
	{
		write<Period>(period);
		write<Sample>(sample);

		Control::access_t ctrl = 0;
		Control::Enable::set(ctrl, 1);
		Control::Dbgen::set(ctrl,  1);
		Control::Waiten::set(ctrl, 1);
		Control::Dozen::set(ctrl,  1);
		Control::Stopen::set(ctrl, 1);
		Control::Clock_source::set(ctrl, clk_src);
		write<Control>(ctrl);
	}
};


struct Main
{
	Genode::Env                     & _env;
	Genode::Attached_rom_dataspace    _config { _env, "config" };
	/** FIXME: currently use PWM2 of i.MX53, use platform driver in future **/
	Genode::Attached_io_mem_dataspace _ds     { _env, 0x53fb8000, 0x4000 };
	Genode::Constructible<Pwm>        _pwm    {};

	Main(Genode::Env &env) : _env(env)
	{
		Genode::log("--- i.MX53 Pulse-width-modulation driver ---");

		Genode::Xml_node config = _config.xml();
		unsigned period = config.attribute_value<unsigned>("period", 0);
		unsigned sample = config.attribute_value<unsigned>("sample", 0);
		Genode::String<16> clk = config.attribute_value("clock_source",
		                                                Genode::String<16>());
		Pwm::Clk_src src = Pwm::OFF;
		if (clk == "ipg")          src = Pwm::IPG;
		if (clk == "ipg_highfreq") src = Pwm::IPG_HIGHFREQ;
		if (clk == "ipg_32k")      src = Pwm::IPG_32K;

		_pwm.construct((Genode::addr_t)_ds.local_addr<void>(), period, sample, src);
	}
};


void Component::construct(Genode::Env &env) { static Main main(env); }
