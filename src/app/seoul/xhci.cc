/*
 * \brief  XHCI model powered by Qemu USB library
 * \author Alexander Boettcher
 * \date   2024-05-03
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Seoul VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

#include "xhci.h"

/* qemu-usb includes */
#include <qemu/usb.h>

#include "model/pci.h"
#include "model/simplemem.h"

static Qemu::Controller *qemu_xhci;

class Timer_queue : public Qemu::Timer_queue, public StaticReceiver<Timer_queue>
{
	private:

		struct Timer : public Genode::List<Timer>::Element
		{
			timevalue timeout_abs { };
			bool      pending     { };

			void  * const qtimer;
			void (* const cb)(void*);
			void  * const data;

			Timer(void *qtimer, void (*cb)(void*), void *data)
			: qtimer(qtimer), cb(cb), data(data) { }
		};

		Genode::Mutex         _mutex  { };
		Genode::List<Timer>   _timers { };
		Motherboard         & _mb;
		unsigned  const       _timer_id;

		timevalue             _current_timeout { };

		void for_each_timer(auto const &fn)
		{
			for (auto * t = _timers.first(); t; t = t->next())
				fn(*t);
		}

		void for_pending_timer(auto const &fn)
		{
			bool restart_loop { };

			do {
				restart_loop = false;

				for (auto * t = _timers.first(); t; t = t->next()) {
					if (!t->pending)
						continue;

					restart_loop = fn(*t);
					if (restart_loop)
						break;
				}
			} while (restart_loop);
		}

		void with_timer(void const *qtimer, auto const &fn)
		{
			for (auto * t = _timers.first(); t; t = t->next()) {
				if (t->qtimer == qtimer) {
					fn(*t);
					return;
				}
			}
		}

		unsigned _register_timer()
		{
			MessageTimer timer { };

			if (!_mb.bus_timer.send(timer))
				Logging::panic("xhci: could not register timer");

			return timer.nr;
		}

		void _program_timeout(timevalue abs_tsc)
		{
			_current_timeout = abs_tsc;

			MessageTimer msg(_timer_id, abs_tsc);
			if (!_mb.bus_timer.send(msg))
				Logging::panic("xhci: could not program timer.");
		}

		void _with_min_pending(auto const &fn)
		{
			Timer * next_timer { };

			for_each_timer([&](auto &timer) {
				if (!timer.pending)
					return;

				if (!next_timer)
					next_timer = &timer;
				else
				if (timer.timeout_abs < next_timer->timeout_abs)
					next_timer = &timer;
			});

			if (next_timer)
				fn(*next_timer);
		}

	public:

		Timer_queue(Motherboard &mb) : _mb(mb), _timer_id(_register_timer())
		{
			_mb.bus_timeout.add(this, &Timer_queue::receive_static<MessageTimeout>);
		}

		void register_timer(void *qtimer, void (*cb)(void*), void *data) override
		{
			Genode::Mutex::Guard guard(_mutex);

			auto timer = new Timer(qtimer, cb, data);
			_timers.insert(timer);
		}

		void delete_timer(void *qtimer) override
		{
			Genode::Mutex::Guard guard(_mutex);

			with_timer(qtimer, [&](auto &t) {
				_timers.remove(&t);
				delete &t;
			});

			_with_min_pending([&](auto &t) {
				if (t.timeout_abs != _current_timeout)
					_program_timeout(t.timeout_abs);
			});
		}

		void activate_timer(void *qtimer, long long int expire_abs_ns) override
		{
			Genode::Mutex::Guard guard(_mutex);

			with_timer(qtimer, [&](auto &t) {
				t.timeout_abs =_mb.clock()->convert_to_tsc(1'000'000'000ull,
				                                           expire_abs_ns);
				t.pending     = true;
			});

			_with_min_pending([&](auto &t) {
				if (t.timeout_abs != _current_timeout)
					_program_timeout(t.timeout_abs);
			});
		}

		void deactivate_timer(void *qtimer) override
		{
			Genode::Mutex::Guard guard(_mutex);

			with_timer(qtimer, [&](auto &t) {
				t.pending = false;
			});

			_with_min_pending([&](auto &t) {
				if (t.timeout_abs != _current_timeout)
					_program_timeout(t.timeout_abs);
			});
		}

		Qemu::int64_t get_ns() override
		{
			auto const now = _mb.clock()->time();
			return _mb.clock()->convert_tsc_to(1'000'000'000ull, now);
		}

		bool receive(MessageTimeout &msg)
		{
			if (msg.nr != _timer_id)
				return false;

			Genode::Mutex::Guard guard(_mutex);

			auto const tsc_now = _mb.clock()->time();

			for_pending_timer([&](auto &timer) {
				if (timer.timeout_abs > tsc_now)
					return false; /* continue looping through list */

				timer.pending = false;

				auto cb   = timer.cb;
				auto data = timer.data;

				_mutex.release();

				Qemu::usb_timer_callback(cb, data);

				_mutex.acquire();

				return true; /* restart timer loop iteration from beginning */
			});

			_with_min_pending([&](auto &t) {
				if (_current_timeout != t.timeout_abs)
					_program_timeout(t.timeout_abs);
			});

			return true;
		}
};

struct Pci_device : public Qemu::Pci_device
{
	private:

		Motherboard &_mb;
		uint8 const  _irq;

	public:

		Pci_device(Motherboard &mb, uint8 irq) : _mb(mb), _irq(irq) { }

		virtual ~Pci_device() { }

		void raise_interrupt(int const assert) override
		{
			MessageIrqLines msg_irq(assert ? MessageIrq::ASSERT_IRQ
			                               : MessageIrq::DEASSERT_IRQ, _irq);

			bool result = _mb.bus_irqlines.send(msg_irq);

			if (!result)
				Genode::error(__func__, " failed ",
				              assert ? "assert" : "deassert");
		}

		int read_dma(Qemu::addr_t addr, void *buf, Qemu::size_t size) override
		{
			auto const result = copy_in(_mb.bus_memregion, _mb.bus_mem,
			                            addr, buf, size);
			if (!result)
				Genode::error(__func__, " failed");

			return result ? 0 : 1;
		}

		int write_dma(Qemu::addr_t addr, void const *buf, Qemu::size_t size) override
		{
			auto const result = copy_out(_mb.bus_memregion, _mb.bus_mem, addr,
			                             buf, size);
			if (!result)
				Genode::error(__func__, " failed");

			return result ? 0 : 1;
		}

};

Seoul::Xhci::Xhci(Genode::Env &env, Genode::Heap &heap,
                  Genode::Xml_node const &config, Motherboard &mb)
{
	static Timer_queue timer_queue { mb };
	static Pci_device  pci_device  { mb, 13 /* XXX */ };

	qemu_xhci = Qemu::usb_init(timer_queue, pci_device, env.ep(),
	                           heap, env, config);
}

class Xhci_model: public StaticReceiver<Xhci_model>
{
	private:

		uint64       _phys_bar0;
		uint16 const _bdf;
		uint8  const _pin =  4; /* PCI INTD */
		uint8  const _irq = 13; /* defined by acpicontroller dsdt for INTD# */

		uint32       _control   { };
		uint32       _status    { 1u << 20 /* cap list support */ };

		bool         _bar0_size { };

	public:

		Xhci_model(uint16 bdf, uint64 bar) : _phys_bar0(bar), _bdf(bdf) { }

		bool receive(MessagePciConfig &msg)
		{
			if (msg.bdf != _bdf) return false;

			bool const verbose = false;

			switch (msg.type) {
			case MessagePciConfig::TYPE_READ:
				switch (msg.dword * 4) {
				case 0x00: { /* device & vendor */
					auto const info = qemu_xhci->info();

					msg.value = (unsigned(info.product_id) << 16)
					          | (unsigned(info.vendor_id) & 0xffffu);
					break;
				}
				case 0x04: /* status & control */
					msg.value = _status | _control;
					break;
				case 0x08: /* class code, sub class, prog if, rev. id */
					msg.value = 0x0c033000;
					break;
				case 0x0c: /* bist, header type, lat timer, cache */
					msg.value = 0;
					break;
				case 0x10: /* BAR 0 */
					if (_bar0_size) {
						msg.value = unsigned(qemu_xhci->mmio_size());
						_bar0_size = false;
					} else {
						if (_phys_bar0 >= (1ull << 32))
							Logging::panic("phys bar addr too large\n");
						msg.value = unsigned(_phys_bar0);
					}
					break;
				case 0x30: /* Serial Bus Release Number Register */
					msg.value = 0x60;
					break;
				case 0x34: /* capability pointer */
					msg.value = 0x40;
					break;
				case 0x3c: /* max lat, min grant, intr pin, intr line */
					msg.value = (unsigned(_pin) << 8) | unsigned(_irq);
					break;
				case 0x40:
					msg.value = 0;
					break;
				default:
					msg.value = 0;
					if (verbose)
						Logging::printf("xhci: pci config read %x\n", msg.dword * 4);
				}
				break;
			case MessagePciConfig::TYPE_WRITE:
				switch (msg.dword * 4) {
				case 0x04:
					_control = msg.value & ((1u << 11) - 1);
					break;
				case 0x10: /* BAR0 */
					if (msg.value == ~0U)
						_bar0_size = true;
					else
						_phys_bar0 = msg.value;
					break;
				default:
					if (verbose)
						Logging::printf("xhci: pci config write %x\n", msg.dword * 4);
				}
				break;
			case MessagePciConfig::TYPE_PTR:
				if (verbose)
					Logging::printf("xhci: unsupported pci access %x\n", msg.dword * 4);
				break;
			}

			return true;
		}

		bool receive(MessageMem &msg)
		{
			if (!qemu_xhci || !in_range(msg.phys, _phys_bar0,
			                            qemu_xhci->mmio_size()))
				return false;

			auto const offset = unsigned(msg.phys - _phys_bar0);

			if (msg.read)
				qemu_xhci->mmio_read (offset, msg.ptr, 4);
			else
				qemu_xhci->mmio_write(offset, msg.ptr, 4);

			return true;
		}
};

PARAM_HANDLER(xhci, "xhci description")
{
	unsigned const bdf = PciHelper::find_free_bdf(mb.bus_pcicfg,
	                                              unsigned(argv[1]));
	if (bdf >= 1u << 16)
		Logging::panic("xhci: bdf invalid\n");

	auto const bar_base = argv[0];

	if (argv[0] == ~0UL)
		Logging::panic("xhci: missing bar address");

	auto dev = new Xhci_model(uint16(bdf), bar_base);

	mb.bus_pcicfg.add(dev, Xhci_model::receive_static<MessagePciConfig>);
	mb.bus_mem   .add(dev, Xhci_model::receive_static<MessageMem>);
}

extern "C" void _type_init_host_webcam_register_types(Genode::Env &,
                                                      Genode::Xml_node const &)
{
	Genode::warning(__func__, " ignored - webcam will not work");
}
