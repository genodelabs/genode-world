/*
 * \brief  Gpio driver for the OMAP4
 * \author Ivan Loskutov <ivan.loskutov@ksyslabs.org>
 * \author Stefan Kalkowski <stefan.kalkowski@genode-labs.com>
 * \date   2012-06-23
 */

/*
 * Copyright (C) 2012 Ksys Labs LLC
 * Copyright (C) 2012-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _DRIVERS__GPIO__SPEC__OMAP4__DRIVER_H_
#define _DRIVERS__GPIO__SPEC__OMAP4__DRIVER_H_

/* Genode includes */
#include <gpio/driver.h>
#include <irq_session/connection.h>
#include <timer_session/connection.h>

/* local includes */
#include "gpio.h"


class Omap4_driver : public Gpio::Driver
{
	using Pin = Gpio::Pin;
	private:

		enum {
			PIN_SHIFT = 5,
			MAX_BANKS = 6,
			MAX_PINS  = 32
		};

		enum {
			GPIO1_MMIO_BASE = 0x4a310000,
			GPIO1_MMIO_SIZE = 0x1000,
			GPIO1_IRQ       = 29 + 32,
			GPIO2_MMIO_BASE = 0x48055000,
			GPIO2_MMIO_SIZE = 0x1000,
			GPIO2_IRQ       = 30 + 32,
			GPIO3_MMIO_BASE = 0x48057000,
			GPIO3_MMIO_SIZE = 0x1000,
			GPIO3_IRQ       = 31 + 32,
			GPIO4_MMIO_BASE = 0x48059000,
			GPIO4_MMIO_SIZE = 0x1000,
			GPIO4_IRQ       = 32 + 32,
			GPIO5_MMIO_BASE = 0x4805b000,
			GPIO5_MMIO_SIZE = 0x1000,
			GPIO5_IRQ       = 33 + 32,
			GPIO6_MMIO_BASE = 0x4805d000,
			GPIO6_MMIO_SIZE = 0x1000,
			GPIO6_IRQ       = 34 + 32,
		};

		class Gpio_bank
		{
			private:

				Gpio_reg                             _reg;
				Genode::Irq_connection               _irq;
				Genode::Signal_handler<Gpio_bank>    _dispatcher;
				Genode::Signal_context_capability    _sig_cap[MAX_PINS];
				bool                                 _irq_enabled[MAX_PINS];

				void _handle()
				{
					_reg.write<Gpio_reg::Irqstatus_0>(0xffffffff);

					unsigned long status = _reg.read<Gpio_reg::Irqstatus_0>();

					for(unsigned i = 0; i < MAX_PINS; i++) {
						if ((status & (1 << i)) && _irq_enabled[i] &&
						    _sig_cap[i].valid())
							Genode::Signal_transmitter(_sig_cap[i]).submit();
					}

					_irq.ack_irq();
				}


			public:

				Gpio_bank(Genode::Env &env,
				          Genode::addr_t base, Genode::size_t size,
				          unsigned irq)
				: _reg(env, base, size), _irq(env, irq),
				  _dispatcher(env.ep(), *this, &Gpio_bank::_handle)
				{
					for (unsigned i = 0; i < MAX_PINS; i++)
						_irq_enabled[i] = false;

					_irq.sigh(_dispatcher);
					_irq.ack_irq();
				}

				Gpio_reg* regs() { return &_reg; }

				void irq(int pin, bool enable)
				{
					if (enable) {
						_reg.write<Gpio_reg::Irqstatus_0>(1 << pin);
						_reg.write<Gpio_reg::Irqstatus_set_0>(1 << pin);
					}
					else
						_reg.write<Gpio_reg::Irqstatus_clr_0>(1 << pin);
					_irq_enabled[pin] = enable;
				}

				void ack_irq(int) { Genode::warning(__func__, " not implemented"); }

				void sigh(int pin, Genode::Signal_context_capability cap) {
					_sig_cap[pin] = cap; }
		};

		Gpio_bank _gpio_bank_0;
		Gpio_bank _gpio_bank_1;
		Gpio_bank _gpio_bank_2;
		Gpio_bank _gpio_bank_3;
		Gpio_bank _gpio_bank_4;
		Gpio_bank _gpio_bank_5;

		Gpio_bank *_gpio_bank(Pin gpio)
		{
			switch (gpio.value >> PIN_SHIFT) {
			case 0:
				return &_gpio_bank_0;
			case 1:
				return &_gpio_bank_1;
			case 2:
				return &_gpio_bank_2;
			case 3:
				return &_gpio_bank_3;
			case 4:
				return &_gpio_bank_4;
			case 5:
				return &_gpio_bank_5;
			}

			Genode::error("no Gpio_bank for pin ", gpio, " available");
			return 0;
		}

		unsigned _gpio_index(Pin gpio)       { return gpio.value & 0x1f; }

		Omap4_driver(Genode::Env &env)
		:
			_gpio_bank_0(env, GPIO1_MMIO_BASE, GPIO1_MMIO_SIZE, GPIO1_IRQ),
			_gpio_bank_1(env, GPIO2_MMIO_BASE, GPIO2_MMIO_SIZE, GPIO2_IRQ),
			_gpio_bank_2(env, GPIO3_MMIO_BASE, GPIO3_MMIO_SIZE, GPIO3_IRQ),
			_gpio_bank_3(env, GPIO4_MMIO_BASE, GPIO4_MMIO_SIZE, GPIO4_IRQ),
			_gpio_bank_4(env, GPIO5_MMIO_BASE, GPIO5_MMIO_SIZE, GPIO5_IRQ),
			_gpio_bank_5(env, GPIO6_MMIO_BASE, GPIO6_MMIO_SIZE, GPIO6_IRQ)
		{ }

	public:

		static Omap4_driver& factory(Genode::Env &env);


		/******************************
		 **  Gpio::Driver interface  **
		 ******************************/

		void direction(Pin gpio, bool input) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			gpio_reg->write<Gpio_reg::Oe>(input ? 1 : 0, _gpio_index(gpio));
		}

		void write(Pin gpio, bool level) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();

			if (level)
				gpio_reg->write<Gpio_reg::Setdataout>(1 << _gpio_index(gpio));
			else
				gpio_reg->write<Gpio_reg::Cleardataout>(1 << _gpio_index(gpio));
		}

		bool read(Pin gpio) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			return gpio_reg->read<Gpio_reg::Datain>(_gpio_index(gpio));
		}

		void debounce_enable(Pin gpio, bool enable) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			gpio_reg->write<Gpio_reg::Debounceenable>(enable ? 1 : 0,
			                                          _gpio_index(gpio));
		}

		void debounce_time(Pin gpio, unsigned long us) override
		{
			unsigned char debounce;

			if (us < 32)
				debounce = 0x01;
			else if (us > 7936)
				debounce = 0xff;
			else
				debounce = (unsigned char)((us / 0x1f) - 1);

			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			gpio_reg->write<Gpio_reg::Debouncingtime::Time>(debounce);
		}

		void falling_detect(Pin gpio) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			gpio_reg->write<Gpio_reg::Leveldetect0> (0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Leveldetect1> (0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Fallingdetect>(1, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Risingdetect> (0, _gpio_index(gpio));
		}

		void rising_detect(Pin gpio) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			gpio_reg->write<Gpio_reg::Leveldetect0> (0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Leveldetect1> (0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Fallingdetect>(0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Risingdetect> (1, _gpio_index(gpio));
		}

		void high_detect(Pin gpio) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			gpio_reg->write<Gpio_reg::Leveldetect0> (0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Leveldetect1> (1, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Fallingdetect>(0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Risingdetect> (0, _gpio_index(gpio));
		}

		void low_detect(Pin gpio) override
		{
			Gpio_reg *gpio_reg = _gpio_bank(gpio)->regs();
			gpio_reg->write<Gpio_reg::Leveldetect0> (1, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Leveldetect1> (0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Fallingdetect>(0, _gpio_index(gpio));
			gpio_reg->write<Gpio_reg::Risingdetect> (0, _gpio_index(gpio));
		}

		void irq_enable(Pin gpio, bool enable) override
		{
			_gpio_bank(gpio)->irq(_gpio_index(gpio), enable);
		}

		void ack_irq(Pin gpio) override
		{
			_gpio_bank(gpio)->ack_irq(_gpio_index(gpio));
		}

		void register_signal(Pin gpio,
		                     Genode::Signal_context_capability cap) override
		{
			_gpio_bank(gpio)->sigh(_gpio_index(gpio), cap); }

		void unregister_signal(Pin gpio) override
		{
			Genode::Signal_context_capability cap;
			_gpio_bank(gpio)->sigh(_gpio_index(gpio), cap);
		}

		bool gpio_valid(Pin gpio) override { return gpio.value < (MAX_PINS*MAX_BANKS); }
};

#endif /* _DRIVERS__GPIO__SPEC__OMAP4__DRIVER_H_ */
