/*
 * \brief  Seoul component for Genode
 * \author Alexander Boettcher
 * \author Norman Feske
 * \author Markus Partheymueller
 * \author Benjamin Lamowski
 * \date   2011-11-18
 */

/*
 * Copyright (C) 2011-2024 Genode Labs GmbH
 * Copyright (C) 2012 Intel Corporation
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Seoul/Vancouver VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

/* base includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/heap.h>

#include <timer_session/connection.h>

#include <vm_session/connection.h>
#include <vm_session/handler.h>
#include <cpu/vcpu_state.h>

/* os includes */
#include <nic_session/connection.h>
#include <nic/packet_allocator.h>
#include <rtc_session/connection.h>

/* Seoul includes */
#include <nul/vcpu.h>
#include <nul/motherboard.h>

/* utilities includes */
#include <service/time.h>

/* local includes */
#include "device_model_registry.h"
#include "boot_module_provider.h"
#include "console.h"
#include "network.h"
#include "disk.h"
#include "state.h"
#include "guest_memory.h"
#include "timeout_late.h"
#include "gui.h"
#include "audio.h"
#include "xhci.h"


enum { verbose_debug = false };
enum { verbose_npt   = false };
enum { verbose_io    = false };
enum { verbose_audio = false };

enum {
	PAGE_SIZE_LOG2 = 12UL,
	PAGE_SIZE      = PAGE_SIZE_LOG2 << 12
};


extern void heap_init_env(Genode::Heap *);


using Genode::Attached_rom_dataspace;


class Timeouts
{
	private:

		Timer::Connection                 _timer;
		Motherboard                      &_motherboard;
		Genode::Signal_handler<Timeouts>  _timeout_sigh;

		Genode::Mutex                     _mutex    { };
		TimeoutList<32, void>             _timeouts { };

		Late_timeout                      _late { };

		Genode::uint64_t _check_and_wakeup()
		{
			Genode::Mutex::Guard guard(_mutex);

			auto      const timeout_remote = _late.reset();
			timevalue const now            = _motherboard.clock()->time();

			unsigned timer_nr      = 0;
			unsigned timeout_count = 0;

			while ((timer_nr = _timeouts.trigger(now))) {

				if (timeout_count == 0 && _late.apply(timeout_remote,
				                                      timer_nr, now))
				{
					return _motherboard.clock()->abstime(1, 1000);
				}

				if (_timeouts.cancel(timer_nr) < 0)
					Logging::printf("Timeout not cancelled.\n");

				auto const next_timeout = _timeouts.timeout();

				_mutex.release();

				MessageTimeout msg(timer_nr, next_timeout);
				_motherboard.bus_timeout.send(msg);

				_mutex.acquire();

				timeout_count++;
			}

			return _timeouts.timeout();
		}

		void check_timeouts()
		{
			auto const next = _check_and_wakeup();

			if (next == ~0ULL)
				return;

			timevalue rel_timeout_us = _motherboard.clock()->delta(next, 1000 * 1000);
			if (rel_timeout_us == 0)
				rel_timeout_us = 1;

			_timer.trigger_once(rel_timeout_us);
		}

	public:

		unsigned alloc()
		{
			Genode::Mutex::Guard guard(_mutex);
			return _timeouts.alloc();
		}

		void request(MessageTimer const &msg)
		{
			int res = -1;

			{
				Genode::Mutex::Guard guard(_mutex);
				res = _timeouts.request(msg.nr, msg.abstime);
			}

			if (res == 0) {
				_late.timeout(*_motherboard.clock(), msg);
				Genode::Signal_transmitter(_timeout_sigh).submit();
			}

			if (res < 0)
				Logging::printf("Could not program timeout.\n");
		}

		/**
		 * Constructor
		 */
		Timeouts(Genode::Env &env, Motherboard &mb)
		:
		  _timer(env),
		  _motherboard(mb),
		  _timeout_sigh(env.ep(), *this, &Timeouts::check_timeouts)
		{
			_timer.sigh(_timeout_sigh);

			_timeouts.init();
		}

};


class Vcpu : public StaticReceiver<Vcpu>
{

	private:

		Genode::Vcpu_handler<Vcpu>          _handler;
		Genode::Vm_connection::Exit_config  _exit_config { };

		Seoul::Guest_memory                &_guest_memory;
		Motherboard                        &_motherboard;
		VCpu                               &_vcpu;

		CpuState                            _seoul_state { };

		Genode::Semaphore                   _block   { 0 };
		Genode::Semaphore                   _started { 0 };

		bool const                          _vmx;
		bool const                          _svm;
		bool const                          _map_small;
		bool const                          _rdtsc_exit;
		bool const                          _cpuid_native;

		/* initialize after other members, vCPU gets runnable immediately */
		Genode::Vm_connection              &_vm_con;
		Genode::Vm_connection::Vcpu         _vm_vcpu;

		bool _init_cpuid_state(bool const cpuid_native, unsigned const vcpu_id)
		{
			_seoul_state.clear();
			_seoul_state.head.cpuid = vcpu_id;

			/* handle cpuid overrides */
			_vcpu.executor.add(this, receive_static<CpuMessage>);

			return cpuid_native;
		}

	public:

		Vcpu(Genode::Entrypoint    & ep,
		     Genode::Vm_connection & vm_con,
		     Genode::Allocator     & alloc,
		     Genode::Env           & env,
		     VCpu                  & vcpu,
		     Seoul::Guest_memory   & guest_memory,
		     Motherboard           & motherboard,
		     unsigned const          vcpu_id,
		     bool     const          vmx,
		     bool     const          svm,
		     bool     const          map_small,
		     bool     const          rdtsc,
		     bool     const          cpuid_native)
		:
			_handler(ep, *this, &Vcpu::_handle_vm_exception),
//			         vmx ? &Vcpu::exit_config_intel :
//			         svm ? &Vcpu::exit_config_amd : nullptr),
			_guest_memory(guest_memory),
			_motherboard(motherboard),
			_vcpu(vcpu),
			_vmx(vmx), _svm(svm), _map_small(map_small), _rdtsc_exit(rdtsc),
			_cpuid_native(_init_cpuid_state(cpuid_native, vcpu_id)),
			_vm_con(vm_con),
			_vm_vcpu(_vm_con, alloc, _handler, _exit_config)
		{
			if (!_svm && !_vmx)
				Logging::panic("no SVM/VMX available, sorry");

			/* note: don't initialize anything here, use _init_cpuid_native */
		}

		void start()   { _started.up(); }
		void block()   { _block.down(); }
		void unblock() { _block.up(); }
		void off()     { _seoul_state.head.cpuid = ~0U; recall(); }

		void recall() { _handler.local_submit(); }

		void show_host_cpu_info()
		{
			using Genode::log;
			using Genode::Hex;

			unsigned eax { }, ebx { }, ecx { }, edx { };
			unsigned cpp { }, family { }, top { };
			unsigned tpp = 1;

			eax = Cpu::cpuid (eax, ebx, ecx, edx);

			switch (uint8(eax)) {
			case 0x4 ... 0xff:
				eax = 4; ebx = ecx = edx = 0;
				eax = Cpu::cpuid (eax, ebx, ecx, edx);
				cpp = (eax >> 26 & 0x3f) + 1;
				[[fallthrough]];
			case 0x1 ... 0x3:
				eax = 1; ebx = ecx = edx = 0;
				eax = Cpu::cpuid (eax, ebx, ecx, edx);
				family = ((eax >> 8 & 0xf) + (eax >> 20 & 0xff)) & 0xff;
				tpp = ebx >> 16 & 0xff;
				top = ebx >> 24;
			}

			eax = 0x80000000; ebx = ecx = edx = 0;
			eax = Cpu::cpuid (eax, ebx, ecx, edx);

			unsigned smt = 0;

			if (_svm && eax & 0x80000000) {
				switch (uint8(eax)) {
				case 0x1e ... 0xff:
					if (family >= 0x17) {
						eax = 0x8000001e; ebx = ecx = edx = 0;
						eax = Cpu::cpuid (eax, ebx, ecx, edx);
						smt = ((ebx >> 8) & 0xff) + 1;
					}
					[[fallthrough]];
				default:
					[[fallthrough]];
				case 0x8 ... 0x9:
					if (smt) {
						eax = 0x80000008; ebx = ecx = edx = 0;
						eax = Cpu::cpuid (eax, ebx, tpp, edx);
						if ((tpp >> 12) & 0xf)
						    tpp = 1 << ((tpp >> 12) & 0xf);
						else
						    tpp = (tpp & 0xff) + 1;

						cpp = tpp / smt;
					}
				};
			}

			unsigned tpc = cpp ? tpp / cpp : 0;
			unsigned long t_bits = tpc ? Cpu::bsr(tpc - 1) + 1 : 0;
			unsigned long c_bits = cpp ? Cpu::bsr(cpp - 1) + 1 : 0;

			if (!t_bits || !c_bits || !cpp) {
				Genode::error("vcpu ", _seoul_state.head.cpuid,
				              " - package:core:thread - unknown");
				return;
			}

			auto thread   = (top            & ((1u << t_bits) - 1)) & 0xff;
			auto core     = (top >>  t_bits & ((1u << c_bits) - 1)) & 0xff;
			auto package  = (top >> (t_bits + c_bits)) & 0xff;

			log("vcpu ", _seoul_state.head.cpuid, " - package:core:thread ",
			    package, ":", core, ":", thread);
		}

		void _handle_vm_exception()
		{
			if (_seoul_state.head.cpuid == ~0U) {
				Genode::sleep_forever();
				return;
			}

			_vm_vcpu.with_state([this](Genode::Vcpu_state &state) -> bool {
				unsigned const exit = state.exit_reason;

				if (_svm) {
					switch (exit) {
					case 0x00 ... 0x1f: _svm_cr(state); break;
					case 0x62: _irqwin(state); break;
					case 0x64: _irqwin(state); break;
					case 0x6e: _svm_rdtsc(state); break;
					case 0x72: _svm_cpuid(state); break;
					case 0x78: _svm_hlt(state); break;
					case 0x7b: _svm_ioio(state); break;
					case 0x7c: _svm_msr(state); break;
					case 0x7f: _triple(state); break;
					case 0x8d:
						state.discharge();
						state.  ip.charge(state.ip.value() + 3);
						state.xcr0.charge(uint64(state.dx.value()) << 32 |
						                  uint64(unsigned(state.ax.value())));

						break;
					case 0xfd: _svm_invalid(state); break;
					case 0xfc: _svm_npt(state); break;
					case 0xfe: _svm_startup(state); break;
					case 0xff: _recall(state); break;
					default:
						Genode::error(__func__, " exit=", Genode::Hex(exit));
						/* no resume */
						return false;
					}
				}

				if (_vmx) {
					switch (exit) {
					case 0x02: _triple(state); break;
					case 0x03: _vmx_init(state); break;
					case 0x07: _irqwin(state); break;
					case 0x0a: _vmx_cpuid(state); break;
					case 0x0c: _vmx_hlt(state); break;
					case 0x10: _vmx_rdtsc(state); break;
					case 0x12: _vmx_vmcall(state); break;
					case 0x1c: _vmx_mov_crx(state); break;
					case 0x1e: _vmx_ioio(state); break;
					case 0x1f: _vmx_msr_read(state); break;
					case 0x20: _vmx_msr_write(state); break;
					case 0x21: _vmx_invalid(state); break;
					case 0x28: _vmx_pause(state); break;
					case 0x30: _vmx_ept(state); break;
					case 0x37:
						state.discharge();
						state.  ip.charge(state.ip.value() + 3);
						state.xcr0.charge(uint64(state.dx.value()) << 32 |
						                  uint64(unsigned(state.ax.value())));

						break;
					case 0xfe: _vmx_startup(state); break;
					case 0xff: _recall(state); break;
					default:
						Genode::error(__func__, " exit=", Genode::Hex(exit));
						/* no resume */
						return false;
					}
				}
				return true;
			});
		}

		void exit_config_intel(Genode::Vcpu_state &state, unsigned exit)
		{
			CpuState dummy_state;
			unsigned mtd = 0;

			/* touch the register state required for the specific vm exit */

			switch (exit) {
			case 0x02: /* _triple */
				mtd = MTD_ALL;
				break;
			case 0x03: /* _vmx_init */
				mtd = MTD_ALL;
				break;
			case 0x07: /* _vmx_irqwin */
				mtd = MTD_IRQ;
				break;
			case 0x0a: /* _vmx_cpuid */
				mtd = MTD_RIP_LEN | MTD_GPR_ACDB | MTD_STATE;
				break;
			case 0x0c: /* _vmx_hlt */
				mtd = MTD_RIP_LEN | MTD_IRQ;
				break;
			case 0x10: /* _vmx_rdtsc */
				mtd = MTD_RIP_LEN | MTD_GPR_ACDB | MTD_TSC | MTD_STATE;
				break;
			case 0x12: /* _vmx_vmcall */
				mtd = MTD_RIP_LEN | MTD_GPR_ACDB;
				break;
			case 0x1c: /* _vmx_mov_crx */
				mtd = MTD_ALL;
				break;
			case 0x1e: /* _vmx_ioio */
				mtd = MTD_RIP_LEN | MTD_QUAL | MTD_GPR_ACDB | MTD_GPR_BSD |
				      MTD_STATE | MTD_RFLAGS;
				break;
			case 0x28: /* _vmx_pause */
				mtd = MTD_RIP_LEN | MTD_STATE;
				break;
			case 0x1f: /* _vmx_msr_read */
			case 0x20: /* _vmx_msr_write */
				mtd  = MTD_RIP_LEN | MTD_GPR_ACDB | MTD_TSC | MTD_SYSENTER | MTD_STATE;
				/* 64bit guests */
				mtd |= MTD_FS_GS | MTD_EFER | MTD_SYSCALL_SWAPGS;
				mtd |= MTD_INJ | MTD_RFLAGS;
				break;
			case 0x21: /* _vmx_invalid */
			case 0x30: /* _vmx_ept */
			case 0xfe: /* _vmx_startup */
				mtd = MTD_ALL;
				break;
			case 0xff: /* _recall */
				mtd = MTD_IRQ | MTD_RIP_LEN | MTD_GPR_ACDB | MTD_GPR_BSD;
				break;
			default:
//				mtd = MTD_RIP_LEN;
				break;
			}

			Seoul::write_vcpu_state(dummy_state, mtd, state);
		}

		void exit_config_amd(Genode::Vcpu_state &state, unsigned exit)
		{
			CpuState dummy_state;
			unsigned mtd = 0;

			/* touch the register state required for the specific vm exit */

			switch (exit) {
			case 0x00 ... 0x1f: /* _svm_cr */
				mtd = MTD_RIP_LEN | MTD_CS_SS | MTD_GPR_ACDB | MTD_GPR_BSD |
				      MTD_CR | MTD_IRQ;
				break;
			case 0x72: /* _svm_cpuid */
				mtd = MTD_RIP_LEN | MTD_GPR_ACDB | MTD_IRQ;
				break;
			case 0x78: /* _svm_hlt */
				mtd = MTD_RIP_LEN | MTD_IRQ;
				break;
			case 0xff: /*_recall */
			case 0x62: /* _irqwin - SMI */
			case 0x64: /* _irqwin */
				mtd = MTD_IRQ;
				break;
			case 0x6e: /* _svm_rdtsc */
				mtd = MTD_RIP_LEN | MTD_GPR_ACDB | MTD_TSC | MTD_STATE;
				break;
			case 0x7b: /* _svm_ioio */
				mtd = MTD_RIP_LEN | MTD_QUAL | MTD_GPR_ACDB | MTD_GPR_BSD |
				      MTD_STATE | MTD_RFLAGS;
				break;
			case 0x7c: /* _svm_msr, MTD_ALL */
			case 0x7f: /* _triple, MTD_ALL */
			case 0xfd: /* _svm_invalid, MTD_ALL */
			case 0xfc: /*_svm_npt, MTD_ALL */
			case 0xfe: /*_svm_startup, MTD_ALL */
				mtd = MTD_ALL;
				break;
			default:
//				mtd = MTD_RIP_LEN;
				break;
			}

			Seoul::write_vcpu_state(dummy_state, mtd, state);
		}

		/***********************************
		 ** Virtualization event handlers **
		 ***********************************/

		static void _skip_instruction(CpuMessage &msg)
		{
			/* advance RIP */
			assert(msg.mtr_in & MTD_RIP_LEN);
			msg.cpu->rip += msg.cpu->inst_len;
			msg.mtr_out |= MTD_RIP_LEN;

			/* cancel sti and mov-ss blocking as we emulated an instruction */
			assert(msg.mtr_in & MTD_STATE);
			if (msg.cpu->intr_state & 3) {
				msg.cpu->intr_state &= ~3u;
				msg.mtr_out |= MTD_STATE;
			}
		}

		enum Skip { SKIP = true, NO_SKIP = false };

		void handle_vcpu(Genode::Vcpu_state & state, Skip skip, CpuMessage::Type type)
		{
			/* convert Genode VM state to Seoul state */
			unsigned mtd = Seoul::read_vcpu_state(state, _seoul_state);

			CpuMessage msg(type, &_seoul_state, mtd);

			if (skip == SKIP)
				_skip_instruction(msg);

			/**
			 * Send the message to the VCpu.
			 */
			if (!_vcpu.executor.send(msg, true))
				Logging::panic("nobody to execute %s at %x:%x\n",
				               __func__, msg.cpu->cs.sel, msg.cpu->eip);

			/**
			 * Check whether we should inject something...
			 */
			if (msg.mtr_in & MTD_INJ && msg.type != CpuMessage::TYPE_CHECK_IRQ) {
				msg.type = CpuMessage::TYPE_CHECK_IRQ;
				if (!_vcpu.executor.send(msg, true))
					Logging::panic("nobody to execute %s at %x:%x\n",
					               __func__, msg.cpu->cs.sel, msg.cpu->eip);
			}

			/**
			 * If the IRQ injection is performed, recalc the IRQ window.
			 */
			if (msg.mtr_out & MTD_INJ) {
				msg.type = CpuMessage::TYPE_CALC_IRQWINDOW;
				if (!_vcpu.executor.send(msg, true))
					Logging::panic("nobody to execute %s at %x:%x\n",
					               __func__, msg.cpu->cs.sel, msg.cpu->eip);
			}

			if (~mtd & msg.mtr_out)
				Genode::error("mtd issue !? exit=", Genode::Hex(state.exit_reason),
				              " ", Genode::Hex(mtd), "->", Genode::Hex(msg.mtr_out),
				              " ", Genode::Hex(~mtd & msg.mtr_out));

			/* convert Seoul state to Genode VM state */
			Seoul::write_vcpu_state(_seoul_state, msg.mtr_out, state);
		}

		bool _handle_map_memory(Genode::Vcpu_state & state, bool need_unmap)
		{
			auto const vm_fault_addr = state.qual_secondary.value();

			if (verbose_npt)
				Genode::log("--> request mapping at ", Genode::Hex(vm_fault_addr));

			if (need_unmap) {
				Genode::log(__func__, " need_unmap handled ",
				            Genode::Hex(vm_fault_addr), " ",
				            " cr0=", Genode::Hex(state.cr0.value()));
				_guest_memory.detach(vm_fault_addr & ~0xffful, 4096);
			}

			if (sizeof(uintptr_t) == 4 && vm_fault_addr >= (1ull << (32 + 12)))
				Logging::panic("unsupported guest fault on 32bit");

			MessageMemRegion mem_region(uintptr_t(vm_fault_addr >> PAGE_SIZE_LOG2),
			                            state.cr0.value());

			if (!mem_region.count && (!_motherboard.bus_memregion.send(mem_region, false) ||
			    !mem_region.ptr))
				return false;

			if (verbose_npt)
				Logging::printf("VM page 0x%lx in [0x%lx:0x%lx),"
				                " VMM area: [0x%lx:0x%lx) %s\n",
				                mem_region.page, mem_region.start_page,
				                mem_region.start_page + mem_region.count,
				                (Genode::addr_t)mem_region.ptr >> PAGE_SIZE_LOG2,
				                ((Genode::addr_t)mem_region.ptr >> PAGE_SIZE_LOG2)
				                + mem_region.count,
				                mem_region.read_only ? "readonly" : "writeable");

			assert(state.inj_info.charged());

			/* EPT violation during IDT vectoring? */
			if (state.inj_info.value() & 0x80000000U) {
				/* convert Genode VM state to Seoul state */
				unsigned mtd = Seoul::read_vcpu_state(state, _seoul_state);
				assert(mtd & MTD_INJ);

				if (verbose_debug)
					Logging::printf("EPT violation during IDT vectoring.\n");

				CpuMessage _win(CpuMessage::TYPE_CALC_IRQWINDOW, &_seoul_state, mtd);
				_win.mtr_out = MTD_INJ;
				if (!_vcpu.executor.send(_win, true))
					Logging::panic("nobody to execute %s at %x:%x\n",
					               __func__, _seoul_state.cs.sel, _seoul_state.eip);

				/* convert Seoul state to Genode VM state */
				Seoul::write_vcpu_state(_seoul_state, _win.mtr_out, state);
//				state.inj_info.charge(state.inj_info.value() & ~0x80000000U);
			} else
				state.discharge(); /* reset */

			_vm_con.with_upgrade([&]() {
				if (_map_small)
					_guest_memory.attach_to_vm(_vm_con,
					                           mem_region.page << PAGE_SIZE_LOG2,
					                           1 << PAGE_SIZE_LOG2, !mem_region.read_only);
				else
					_guest_memory.attach_to_vm(_vm_con,
					                           mem_region.start_page << PAGE_SIZE_LOG2,
					                           mem_region.count << PAGE_SIZE_LOG2, !mem_region.read_only);
			});

			return true;
		}

		void _handle_io(Genode::Vcpu_state & state, bool is_in, unsigned io_order, unsigned port)
		{
			if (verbose_io)
				Logging::printf("--> I/O is_in=%d, io_order=%d, port=%x\n",
				                is_in, io_order, port);

			/* convert Genode VM state to Seoul state */
			unsigned mtd = Seoul::read_vcpu_state(state, _seoul_state);

			Genode::addr_t ax = state.ax.value();

			CpuMessage msg(is_in, &_seoul_state, io_order,
			               port, &ax, mtd);

			_skip_instruction(msg);

			if (!_vcpu.executor.send(msg, true))
				Logging::panic("nobody to execute %s at %x:%x\n",
				               __func__, msg.cpu->cs.sel, msg.cpu->eip);

			if (ax != _seoul_state.rax)
				_seoul_state.rax = ax;

			/* convert Seoul state to Genode VM state */
			Seoul::write_vcpu_state(_seoul_state, msg.mtr_out, state);
		}

		/* SVM portal functions */
		void _svm_startup(Genode::Vcpu_state & state)
		{
			show_host_cpu_info();

			_started.down();

			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_CHECK_IRQ);
			state.ctrl_primary  .charge(_rdtsc_exit ? (1u << 14) : 0);
			state.ctrl_secondary.charge((1u << 13) /* XSETBV */);
		}

		void _svm_npt(Genode::Vcpu_state & state)
		{
			if (!_handle_map_memory(state, state.qual_primary.value() & 1))
				_svm_invalid(state);
		}

		void _svm_cr(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_SINGLE_STEP);
		}

		void _svm_invalid(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_SINGLE_STEP);
			state.ctrl_primary.charge(state.ctrl_primary.value() |
			                          (1u << 18) /* cpuid */ |
			                          (_rdtsc_exit ? (1u << 14) : 0));
			state.ctrl_secondary.charge(state.ctrl_secondary.value() |
			                            (1u << 0) /* vmrun */);
		}

		void _svm_ioio(Genode::Vcpu_state & state)
		{
			if (state.qual_primary.value() & 0x4) {
				/* in/out string ops handled by instruction emulator halifax */
				state.ip_len.charge(Genode::addr_t(state.qual_secondary.value() - state.ip.value()));
				handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_SINGLE_STEP);
			} else {
				unsigned order = unsigned(((state.qual_primary.value() >> 4) & 7) - 1);

				if (order > 2)
					order = 2;

				state.ip_len.charge(Genode::addr_t(state.qual_secondary.value() - state.ip.value()));

				_handle_io(state, state.qual_primary.value() & 1, order,
				           unsigned(state.qual_primary.value() >> 16));
			}
		}

		void _svm_cpuid(Genode::Vcpu_state & state)
		{
			state.ip_len.charge(2);
			handle_vcpu(state, SKIP, CpuMessage::TYPE_CPUID);
		}

		void _svm_hlt(Genode::Vcpu_state & state)
		{
			state.ip_len.charge(1);
			_vmx_hlt(state);
		}

		void _svm_rdtsc(Genode::Vcpu_state & state)
		{
			state.ip_len.charge(2);
			handle_vcpu(state, SKIP, CpuMessage::TYPE_RDTSC);
		}

		void _svm_msr(Genode::Vcpu_state & state)
		{
			_svm_invalid(state);
		}

		void _recall(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_CHECK_IRQ);
		}

		void _irqwin(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_CHECK_IRQ);
		}

		void _triple(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_TRIPLE);
		}

		void _vmx_init(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_INIT);
		}

		void _vmx_hlt(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, SKIP, CpuMessage::TYPE_HLT);
		}

		void _vmx_rdtsc(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, SKIP, CpuMessage::TYPE_RDTSC);
		}

		void _vmx_vmcall(Genode::Vcpu_state & state)
		{
			state.discharge(); /* reset */
			state.ip.charge(state.ip.value() + state.ip_len.value());
		}

		void _vmx_pause(Genode::Vcpu_state & state)
		{
			/* convert Genode VM state to Seoul state */
			unsigned mtd = Seoul::read_vcpu_state(state, _seoul_state);

			CpuMessage msg(CpuMessage::TYPE_SINGLE_STEP, &_seoul_state, mtd);

			_skip_instruction(msg);

			/* convert Seoul state to Genode VM state */
			Seoul::write_vcpu_state(_seoul_state, msg.mtr_out, state);
		}

		void _vmx_invalid(Genode::Vcpu_state & state)
		{
			state.flags.charge(state.flags.value() | 2);
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_SINGLE_STEP);
		}

		void _vmx_startup(Genode::Vcpu_state & state)
		{
			show_host_cpu_info();

			_started.down();

			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_HLT);
			state.ctrl_primary.charge(_rdtsc_exit ? (1U << 12) : 0);
			state.ctrl_secondary.charge(1u << 20 /* XSAVE */);
		}

		void _vmx_ioio(Genode::Vcpu_state & state)
		{
			if (state.qual_primary.value() & 0x10) {
				/* in/out string ops handled by instruction emulator halifax */
				handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_SINGLE_STEP);
			} else {
				unsigned order = state.qual_primary.value() & 7;
				/*
				 * Table 28-5. Exit Qualification for I/O Instructions
				 * order 0 == 8bit, 1 == 16bit, 3 == 32bit
				 */
				if (order > 2) order = 2;

				_handle_io(state, state.qual_primary.value() & 8, order,
				           unsigned(state.qual_primary.value() >> 16));
			}
		}

		void _vmx_ept(Genode::Vcpu_state & state)
		{
			if (!_handle_map_memory(state, state.qual_primary.value() & 0x38))
				/* this is an access to MMIO */
				handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_SINGLE_STEP);
		}

		void _vmx_cpuid(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, SKIP, CpuMessage::TYPE_CPUID);
		}

		void _vmx_msr_read(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, SKIP, CpuMessage::TYPE_RDMSR);
		}

		void _vmx_msr_write(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, SKIP, CpuMessage::TYPE_WRMSR);
		}

		void _vmx_mov_crx(Genode::Vcpu_state & state)
		{
			handle_vcpu(state, NO_SKIP, CpuMessage::TYPE_SINGLE_STEP);
		}

		/***********************************
		 ** Handlers for 'StaticReceiver' **
		 ***********************************/

		bool receive(CpuMessage &msg)
		{
			if (msg.type != CpuMessage::TYPE_CPUID || !msg.cpu)
				return false;

			/*
			 * handle_cpuid() in contrib vcpu.cc handles well known
			 * cpuid and the one we set here explicitly with set_cpuid().
			 *
			 * All others are forwarded here and can be overwritten if
			 * required
			 */

			auto &cpu = *msg.cpu;

			if (!_cpuid_native) {
				cpu.eax = cpu.ebx = cpu.ecx = cpu.edx = 0;
				msg.mtr_out |= MTD_GPR_ACDB;
				return true;
			}

			switch (cpu.eax) {
			case 0xd: {
				/*
				 * This cpuid contains a lot of FPU extended state information.
				 * Special is that it contains of a lot of subleafs
				 * (ecx=0...X). Provide all information from host by now.
				 *
				 * see Intel Spec
				 *  "Table 3-8. Information Returned by CPUID Instruction"
				 */
				auto ecx = cpu.ecx;
				cpu.eax = Cpu::cpuid(cpu.eax, cpu.ebx, cpu.ecx, cpu.edx);

				using namespace Genode;

				/*
				 * XEN (4.21) complains seeing a too large FPU size if solely
				 * X87 (BIT 0) and SSE (BIT 1) is enabled in xcr0
				 *
				 * xen/arch/x86/xstate.c
				 * -> compressed hw size 0x340 != xen size 0x240
				 */
				if ((cpu.xcr0 <= 3) && (ecx == 0 || ecx == 1)) {
					if (cpu.ebx != 576) {
						log("change xstate size: eax=0xd ecx=", Hex(ecx),
						    " -> eax=", Hex(cpu.eax), " ebx=", Hex(cpu.ebx),
						    " ecx=", Hex(cpu.ecx), " edx=", Hex(cpu.edx),
						    " xcr0=", Hex(cpu.xcr0));

						cpu.ebx = 576;

						log("change xstate size: eax=0xd ecx=", Hex(ecx),
						    " -> eax=", Hex(cpu.eax), " ebx=", Hex(cpu.ebx),
						    " ecx=", Hex(cpu.ecx), " edx=", Hex(cpu.edx),
						    " xcr0=", Hex(cpu.xcr0), " -> adjusted");
					}
				}
				break;
			}
			default:
				cpu.eax = cpu.ebx = cpu.ecx = cpu.edx = 0;
				break;
			}

			msg.mtr_out |= MTD_GPR_ACDB;
			return true;
		}
};


struct Vmm {
	Genode::Env          &env;
	Genode::Heap          heap   { env.ram(), env.rm() };
	Genode::Vm_connection vm_con { env, "Seoul vCPUs",
	                               Genode::Cpu_session::PRIORITY_LIMIT / 16 };

	Attached_rom_dataspace config { env, "config" };
	Attached_rom_dataspace info   { env, "platform_info" };

	Genode::Number_of_bytes vmm_size { 12 * 1024 * 1024 };

	bool map_small         { };
	bool rdtsc_exit        { };
	bool vmm_vcpu_same_cpu { };
	bool cpuid_native      { };
	bool memory_verbose    { };

	void read_config(Genode::Node const &node)
	{
		map_small         = node.attribute_value("map_small", map_small);
		rdtsc_exit        = node.attribute_value("exit_on_rdtsc", rdtsc_exit);
		vmm_vcpu_same_cpu = node.attribute_value("vmm_vcpu_same_cpu", vmm_vcpu_same_cpu);
		cpuid_native      = node.attribute_value("cpuid_native", cpuid_native);
		memory_verbose    = node.attribute_value("verbose_mem", memory_verbose);
		vmm_size          = node.attribute_value("vmm_memory", vmm_size);
	}

	Vmm(Genode::Env &env) : env(env)
	{
		using Genode::String;

		String<16> kernel("unknown");

		auto const & node_c = config.node();

		/* read default values */
		read_config(node_c);

		info.node().with_optional_sub_node("kernel", [&](auto const &node) {
			kernel = node.attribute_value("name", kernel); });

		if (kernel == "unknown")
			return;

		/* read per kernel overwrites */
		node_c.for_each_sub_node("kernel", [&](auto const &node) {
			String<16> check("unknown");
			check = node.attribute_value("name", check);
			if (check != kernel)
				return;

			read_config(node);
		});
	}
};


class Machine : public StaticReceiver<Machine>
{
	private:

		Vmm                   &_vmm;
		Genode::Mutex          _mutex { };
		Clock                  _clock;
		Motherboard            _motherboard;
		Seoul::Guest_memory   &_guest_memory;
		Timeouts               _timeouts { _vmm.env, _motherboard };
		unsigned short         _vcpus_up { };

		Rtc::Session          *_rtc          { nullptr };
		Seoul::Audio          *_audio        { nullptr };

		Genode::Constructible<Seoul::Xhci> _xhci { };

		using Vcpus_active = Genode::Bit_array<64>;

		Vcpu *                 _vcpus[16]    { nullptr };
		Vcpus_active           _vcpus_active { };

		static bool _all_inactive(Vcpus_active const &a)
		{
			return a.get(0, 64).convert<bool>([] (bool v) { return !v; },
			                                  [] (auto &) { return false; });
		};

		/*
		 * Noncopyable
		 */
		Machine(Machine const &);
		Machine &operator = (Machine const &);

		bool powered() { return !!_vcpus_up; }

	public:

		Motherboard &motherboard() { return _motherboard; }

		/*********************************************
		 ** Callbacks registered at the motherboard **
		 *********************************************/

		bool receive(MessageHostOp &msg)
		{
			switch (msg.type) {

			case MessageHostOp::OP_ALLOC_IOMEM:
			case MessageHostOp::OP_ALLOC_IOMEM_SMALL:
			{
				if (msg.len & 0xfff)
					return false;

				using namespace Genode;

				auto const guest_addr = msg.value;

				Region_map::Attr attr { };
				attr.writeable = true;

				auto const ds = _vmm.env.ram().alloc(msg.len);
				auto const local_addr = _vmm.env.rm().attach(ds, attr).convert<addr_t>(
					[&] (Env::Local_rm::Attachment &a) {
						a.deallocate = false; return addr_t(a.ptr); },
					[&] (Env::Local_rm::Error) -> addr_t { return 0ul; });

				if (!local_addr)
					return false;

				auto alloc_size = msg.type == MessageHostOp::OP_ALLOC_IOMEM_SMALL ?
				                  msg.len_short : msg.len;
				bool ok = _guest_memory.add_region(_vmm.heap, guest_addr,
				                                   local_addr, ds,
				                                   alloc_size);
				if (ok)
					msg.ptr = reinterpret_cast<char *>(local_addr);

				return ok;
			}
			/**
			 * Request available guest memory starting at specified address
			 */
			case MessageHostOp::OP_GUEST_MEM:

				if (verbose_debug)
					Logging::printf("OP_GUEST_MEM value=0x%lx\n", msg.value);

				if (msg.value != 0)
					Logging::panic ("legacy op_guest_mem mode not supported");

				msg.len = _guest_memory.backing_store_size() - msg.value;
				msg.ptr = _guest_memory.backing_store_local_base() + msg.value;

				if (verbose_debug)
					Logging::printf(" -> len=0x%zx, ptr=0x%p\n",
					                msg.len, msg.ptr);
				return true;

			/**
			 * Reserve IO MEM range for devices
			 */
			case MessageHostOp::OP_RESERVE_IO_RANGE:

				msg.phys = _guest_memory.alloc_io_memory(msg.value);
				return true;

			case MessageHostOp::OP_DETACH_MEM:

				_guest_memory.detach(msg.value, msg.len);
				return true;

			case MessageHostOp::OP_VCPU_CREATE_BACKEND:
				{
					enum { STACK_SIZE = 2*1024*sizeof(Genode::addr_t) };

					if (verbose_debug)
						Logging::printf("OP_VCPU_CREATE_BACKEND\n");

					if (_vcpus_up >= sizeof(_vcpus)/sizeof(_vcpus[0])) {
						Logging::panic("too many vCPUs");
						return false;
					}

					/* detect virtualization extension */
					auto has_feature = [&] (auto const &attr)
					{
						return _vmm.info.node().with_sub_node("hardware",
							[&] (Node const &node) {
								return node.with_sub_node("features",
									[&] (Node const &features) {
										return features.attribute_value(attr, false); },
									[] { return false; });
							}, [&] { return false; });
					};

					bool const has_svm = has_feature("svm");
					bool const has_vmx = has_feature("vmx");

					if (!has_svm && !has_vmx) {
						Logging::panic("no VMX nor SVM virtualization support found");
						return false;
					}

					using Genode::Entrypoint;
					using Genode::String;
					using Genode::Affinity;

					Affinity::Space space = _vmm.env.cpu().affinity_space();
					Affinity::Location location(space.location_of_index(_vcpus_up + (_vmm.vmm_vcpu_same_cpu ? 0 : 1)));

					String<16> * ep_name = new String<16>("vCPU EP ", _vcpus_up);
					Entrypoint * ep = new Entrypoint(_vmm.env, STACK_SIZE,
					                                 ep_name->string(),
					                                 location);

					(void)_vcpus_active.set(_vcpus_up, 1);

					auto vcpu = new Vcpu(*ep, _vmm.vm_con, _vmm.heap, _vmm.env,
					                     *msg.vcpu, _guest_memory, _motherboard,
					                     _vcpus_up, has_vmx, has_svm,
					                     _vmm.map_small, _vmm.rdtsc_exit,
					                     _vmm.cpuid_native);

					_vcpus[_vcpus_up] = vcpu;
					msg.value = _vcpus_up;

					Logging::printf("create vcpu %u affinity %u:%u\n",
					                _vcpus_up, location.xpos(), location.ypos());

					_vcpus_up ++;

					return true;
				}

			case MessageHostOp::OP_VCPU_RELEASE:
			{
				if (verbose_debug)
					Genode::log("- OP_VCPU_RELEASE ", Genode::Thread::myself()->name);

				auto const vcpu_id = msg.value;
				if ((_vcpus_up >= sizeof(_vcpus)/sizeof(_vcpus[0])))
					return false;

				if (!_vcpus[vcpu_id])
					return false;

				if (msg.len) {
					_vcpus[vcpu_id]->unblock();
					return true;
				}

				_vcpus[vcpu_id]->recall();
				return true;
			}
			case MessageHostOp::OP_VCPU_BLOCK:
				{
					if (verbose_debug)
						Genode::log("- OP_VCPU_BLOCK ", Genode::Thread::myself()->name);

					if (!powered())
						Genode::sleep_forever();

					auto const vcpu_id = msg.value;
					if ((_vcpus_up >= sizeof(_vcpus)/sizeof(_vcpus[0])))
						return false;

					if (!_vcpus[vcpu_id])
						return false;

					{
						Genode::Mutex::Guard guard(_mutex);

						(void)_vcpus_active.clear(vcpu_id, 1);

						if (_all_inactive(_vcpus_active)) {
							MessageConsole msgcon(MessageConsole::Type::TYPE_PAUSE,
							                      Seoul::Console::ID_VGA_VESA);
							_motherboard.bus_console.send(msgcon);
						}
					}

					_vcpus[vcpu_id]->block();

					if (!powered())
						Genode::sleep_forever();

					{
						Genode::Mutex::Guard guard(_mutex);

						if (_all_inactive(_vcpus_active)) {
							MessageConsole msgcon(MessageConsole::Type::TYPE_RESUME,
							                      Seoul::Console::ID_VGA_VESA);
							_motherboard.bus_console.send(msgcon);
						}

						(void)_vcpus_active.set(vcpu_id, 1);
					}

					return true;
				}

			case MessageHostOp::OP_GET_MODULE:
			{
				/*
				 * Module indices start with 1
				 */
				if (msg.module == 0)
					return false;

				int  const index    = msg.module - 1;
				auto const dst_len  = msg.size;

				/* by default no module is available */
				msg.size = 0;

				Boot_module_provider boot_modules { };

				boot_modules.import_module_to_vm(_vmm.env, index, msg.start, dst_len,
					[&](auto const data_len,
					    auto const cmdline,
					    auto const cmdline_len) {

					msg.size    = data_len;
					msg.cmdline = cmdline;
					msg.cmdlen  = cmdline_len;
				});

				return !!msg.size;
			}
			case MessageHostOp::OP_GET_MAC:
			{
				try {
					/* casted later to unsigned */
					if (msg.value >= 1ull << 32)
						return false;

					new (_vmm.heap) Seoul::Network(_vmm.env, _vmm.heap,
					                               _motherboard, msg);

				} catch (...) {
					Logging::printf("Creating network connection failed\n");
					return false;
				}

				return true;
			}
			case MessageHostOp::OP_VM_OFF:
			{
				for (auto &vcpu : _vcpus) { if (vcpu) vcpu->off(); }

				_vcpus_up = 0;

				_vmm.env.parent().exit(0);

				return true;
			}
			default:

				Logging::printf("HostOp %d not implemented\n", msg.type);
				return false;
			}
		}

		bool receive(MessageAudio &msg)
		{
			if (!_audio) {
				try {
					_audio = new (_vmm.heap) Seoul::Audio(_vmm.env, _motherboard);
					_audio->verbose(verbose_audio);
				} catch (...) {
					Genode::error("Creating audio backend failed");
					return false;
				}
			}

			return _audio->receive(msg);
		}

		bool receive(MessageTimer &msg)
		{
			switch (msg.type) {
			case MessageTimer::TIMER_NEW:

				if (verbose_debug)
					Logging::printf("TIMER_NEW\n");

				msg.nr = _timeouts.alloc();

				return true;

			case MessageTimer::TIMER_REQUEST_TIMEOUT:
			{
				_timeouts.request(msg);

				return true;
			}
			default:
				return false;
			};
		}

		bool receive(MessageTime &msg)
		{
			if (!_rtc) {
				try {
					_rtc = new Rtc::Connection(_vmm.env);
				} catch (...) {
					Logging::printf("No RTC present, returning dummy time.\n");
					msg.wallclocktime = msg.timestamp = 0;
					return true;
				}
			}

			Rtc::Timestamp rtc_ts = _rtc->current_time();
			tm_simple tms(rtc_ts.year, rtc_ts.month, rtc_ts.day, rtc_ts.hour,
			              rtc_ts.minute, rtc_ts.second);

			msg.wallclocktime = mktime(&tms) * MessageTime::FREQUENCY;
			msg.timestamp = _motherboard.clock()->clock(MessageTime::FREQUENCY);

			return true;
		}

		bool receive(MessagePciConfig &msg)
		{
			if (verbose_debug)
				Logging::printf("MessagePciConfig\n");
			return false;
		}

		bool receive(MessageAcpi &msg)
		{
			if (verbose_debug)
				Logging::printf("MessageAcpi\n");
			return false;
		}

		bool receive(MessageLegacy &msg)
		{
			if (msg.type == MessageLegacy::RESET) {
				Logging::printf("MessageLegacy::RESET requested\n");
				return true;
			}
			return false;
		}

		static unsigned long long _tsc_from_platform_info(Node const &platform_info)
		{
			return platform_info.with_sub_node("hardware",
				[&] (Node const &hardware) {
					return hardware.with_sub_node("tsc",
						[&] (Node const &tsc) {
							return tsc.attribute_value("freq_khz", 0ULL) * 1000ULL; },
						[] { return 0ULL; }); },
				[] { return 0ULL; });
		}

		/**
		 * Constructor
		 */
		Machine(Seoul::Guest_memory &guest_memory, Vmm &vmm)
		:
			_vmm(vmm),
			_clock(_tsc_from_platform_info(_vmm.info.node())),
			_motherboard(&_clock, nullptr),
			_guest_memory(guest_memory)
		{
			/* register host operations, called back by the VMM */
			_motherboard.bus_hostop.add  (this, receive_static<MessageHostOp>);
			_motherboard.bus_timer.add   (this, receive_static<MessageTimer>);
			_motherboard.bus_time.add    (this, receive_static<MessageTime>);
			_motherboard.bus_hwpcicfg.add(this, receive_static<MessageHwPciConfig>);
			_motherboard.bus_acpi.add    (this, receive_static<MessageAcpi>);
			_motherboard.bus_legacy.add  (this, receive_static<MessageLegacy>);
			_motherboard.bus_audio.add   (this, receive_static<MessageAudio>);
		}


		/**
		 * Exception type thrown on configuration errors
		 */
		class Config_error { };


		/**
		 * Configure virtual machine according to the provided config
		 *
		 * \param machine_node  config node containing device-model sub nodes
		 * \throw               Config_error
		 *
		 * Device models are instantiated in the order of appearance in the XML
		 * configuration.
		 */
		void setup_devices(auto const &config, auto const &machine)
		{
			using namespace Genode;

			bool const verbose = machine.attribute_value("verbose", false);

			machine.for_each_sub_node([&](auto const &node) {

				typedef String<32> Model_name;

				Model_name const name = node.type();

				if (verbose)
					Genode::log("device: ", name);

				if (name == "xhci") {
					if (_xhci.constructed()) {
						Logging::panic("xhci can only by instantiated once");
						throw Config_error();
					}

					_xhci.construct(_vmm.env, _vmm.heap, config, _motherboard);
				}

				Device_model_info *dmi = device_model_registry()->lookup(name.string());

				if (!dmi) {
					Genode::error("configuration error: device model '",
					              name, "' does not exist");
					throw Config_error();
				}

				/*
				 * Read device-model arguments into 'argv' array
				 */
				enum { MAX_ARGS = 8 };
				unsigned long argv[MAX_ARGS];

				for (int i = 0; i < MAX_ARGS; i++)
					argv[i] = ~0UL;

				for (int i = 0; dmi->arg_names[i] && (i < MAX_ARGS); i++) {
					if (node.has_attribute(dmi->arg_names[i])) {
						argv[i] = node.attribute_value(dmi->arg_names[i], ~0UL);
						if (verbose)
							Genode::log(" arg[", i, "]: ", Genode::Hex(argv[i]));
					}
				}

				/*
				 * Initialize new instance of device model
				 *
				 * We never pass any argument string to a device model because
				 * it is not examined by the existing device models.
				 */

				dmi->create(_motherboard, argv, "", 0);
			});
		}

		/**
		 * Reset the machine and unblock the VCPUs
		 */
		void boot(bool cpuid_native);
};

void Machine::boot(bool const cpuid_native)
{
	Genode::log("VM is starting with ", _vcpus_up, " vCPU",
	            _vcpus_up > 1 ? "s" : "");

	unsigned apic_id = _vcpus_up - 1;

	bool xsave = false;

	/* init vCPUs */
	for (VCpu *vcpu = _motherboard.last_vcpu; vcpu; vcpu = vcpu->get_last()) {

		enum   { EAX = 0, EBX = 1, ECX = 2, EDX = 3 };
		unsigned eax = 0, ebx = 0, ecx = 0, edx = 0;

		bool ok = false;

		/* init CPU strings */
		if (!cpuid_native) {
			char const * const short_name_char = "NOVA microHV";
			auto short_name = reinterpret_cast<unsigned const *>(short_name_char);

			ok = vcpu->set_cpuid(0, EBX, short_name[0]); assert(ok);
			ok = vcpu->set_cpuid(0, EDX, short_name[1]); assert(ok);
			ok = vcpu->set_cpuid(0, ECX, short_name[2]); assert(ok);

			Cpu::cpuid(0, ebx, ecx, edx);

			const char *long_name = "Seoul VMM proudly presents this VirtualCPU. ";
			for (unsigned i=0; i<12; i++) {
				ok = vcpu->set_cpuid(0x80000002 + (i / 4), i % 4, reinterpret_cast<const unsigned *>(long_name)[i]);
				assert(ok);
			}
		} else {

			xsave = true;

			eax = ebx = ecx = edx = 0;
			eax = Cpu::cpuid(0, ebx, ecx, edx);

			/* eax specifies max CPUID - up to 0xd relevant for AVX */
			ok = vcpu->set_cpuid(0, EAX, 0xd); assert(ok);
			ok = vcpu->set_cpuid(0, EBX, ebx); assert(ok);
			ok = vcpu->set_cpuid(0, EDX, edx); assert(ok);
			ok = vcpu->set_cpuid(0, ECX, ecx); assert(ok);

			for (unsigned id = 2; id < 4; id++) {
				eax = ebx = ecx = edx = 0;
				eax = Cpu::cpuid(id, ebx, ecx, edx);

				ok = vcpu->set_cpuid(id, EAX, eax); assert(ok);
				ok = vcpu->set_cpuid(id, EBX, ebx); assert(ok);
				ok = vcpu->set_cpuid(id, EDX, edx); assert(ok);
				ok = vcpu->set_cpuid(id, ECX, ecx); assert(ok);
			}

			/* Intel - Table 3-8. Information Returned by CPUID Instruction */
			for (unsigned subleaf = 0; subleaf < 3; subleaf++) {
				eax = ebx = edx = 0;
				ecx = subleaf;

				if (!xsave)
					break;

				eax = Cpu::cpuid(0x7, ebx, ecx, edx);

				/* AVX */
				auto avx_eax = (subleaf == 1)
				             ? ((1u <<  4) | (1u <<  5))
				             : 0u;
				auto avx_ebx = (subleaf == 0)
				             ? (1u <<  5) | /* AVX2 */
				               (1u << 16) | (1u << 17) | (1u << 21) | /* AVX 512 */
				               (1u << 26) | (1u << 27) | (1u << 28) |
				               (1u << 30) | (1u << 31)
				             : 0u;
				auto avx_ecx = (subleaf == 0)
				             ? (1u <<  1) | (1u <<  6) | (1u << 11) |
				               (1u << 12) | (1u << 14)
				             : 0u;
				auto avx_edx = (subleaf == 0)
                             ? (1u <<  2) | (1u <<  3) | (1u <<  8) |
				               (1u << 23)
				             : 0u;

				auto mov_eax = (subleaf == 1) ? (1u << 10) | (1u << 11) | (1u << 12) : 0u;

				auto fpu_ebx = (subleaf == 0) ? (1u <<  6) | (1u << 13) : 0u;

				auto mov_ebx = (subleaf == 0) ? (1u <<  9) : 0u;
				auto sha_ebx = (subleaf == 0) ? (1u << 29) : 0u;

				/* enable mask */
				auto mask_eax = avx_eax | mov_eax;
				auto mask_ebx = avx_ebx | mov_ebx | fpu_ebx | sha_ebx;
				auto mask_ecx = avx_ecx;
				auto mask_edx = avx_edx;

				ok = vcpu->set_cpuid(0x7, EAX + subleaf * 4, eax, mask_eax); assert(ok);
				ok = vcpu->set_cpuid(0x7, EBX + subleaf * 4, ebx, mask_ebx); assert(ok);
				ok = vcpu->set_cpuid(0x7, ECX + subleaf * 4, ecx, mask_ecx); assert(ok);
				ok = vcpu->set_cpuid(0x7, EDX + subleaf * 4, edx, mask_edx); assert(ok);
			}

			for (unsigned id = 0x80000002u; id < 0x80000002u + 3; id++) {
				eax = ebx = ecx = edx = 0;
				eax = Cpu::cpuid(id, ebx, ecx, edx);

				ok = vcpu->set_cpuid(id, EAX, eax); assert(ok);
				ok = vcpu->set_cpuid(id, EBX, ebx); assert(ok);
				ok = vcpu->set_cpuid(id, EDX, edx); assert(ok);
				ok = vcpu->set_cpuid(id, ECX, ecx); assert(ok);
			}
		}

		/* propagate feature flags from the host */
		eax = ebx = ecx = edx = 0;
		eax = Cpu::cpuid(1, ebx, ecx, edx);
		ok  = vcpu->set_cpuid(1, EAX, eax); assert(ok);

		/* clflush size, apic_id */
		ok = vcpu->set_cpuid(1, EBX, (apic_id << 24) | (ebx & 0xff00), 0xff00ff00u);
		assert(ok);

		ok = vcpu->set_cpuid(1, ECX, ecx, 1u <<  0 | /*  SSE3 */
		                                  1u <<  1 | /* PCLMULQDQ */
		                                  1u <<  2 | /* 64bit DS area */
		                                  1u <<  9 | /* SSSE3 */
		                                  1u << 10 | /* CNXT-ID */
		                                  1u << 12 | /* FMA - extension to YMM */
		                                  1u << 13 | /* CMPXCHG16b */
		                                  1u << 17 | /* PCID */
		                                  1u << 19 | /* SSE4.1 */
		                                  1u << 20 | /* SSE4.2 */
		                                  1u << 22 | /* MOVBE */
		                                  1u << 23 | /* POPCNT */
		                                  1u << 25 | /* AES */
		 (!xsave ? 0 :                    1u << 26) | /* XSAVE */
		 (!xsave ? 0 :                    1u << 27) | /* OSXSAVE */
		 (!xsave ? 0 :                    1u << 28) | /* AVX */
		                                  1u << 29 | /* F16C */
		                                  1u << 30); /* RDRAND */
		assert(ok);

		/* -APIC, -MTRR, -MCA, -PAT, -PSE36, -PSN, -DS, -ACPI, -HTT, -TM, -PBE */
		ok = vcpu->set_cpuid(1, EDX, edx, (1u <<  0) | /* FPU */
		                                  (1u <<  1) | /* VME */
		                                  (1u <<  2) | /*  DE */
		                                  (1u <<  3) | /* PSE */
		                                  (1u <<  4) | /* TSC */
		                                  (1u <<  5) | /* MSR */
		                                  (1u <<  6) | /* PAE */
		                                  (1u <<  7) | /* MCE */
		                                  (1u <<  8) | /* CX8 */
		                                  (1u << 11) | /* SEP */
		                                  (1u << 13) | /* PGE */
		                                  (1u << 15) | /* CMOV */
		                                  (1u << 19) | /* CLFSH */
		                                  (1u << 23) | /* MMX */
		                                  (1u << 24) | /* FXSR */
		                                  (1u << 25) | /* SSE */
		                                  (1u << 26) | /* SSE2 */
		                                  (1u << 27)); /* SS */
		assert(ok);

		/* +NX */
		eax = ebx = ecx = edx = 0;
		Cpu::cpuid(0x80000001, ebx, ecx, edx);
		ok = vcpu->set_cpuid(0x80000001, EDX, edx,
		                     (1u << 20) | /* NX */
		                     (1u << 29)   /* Long Mode */ );
		assert(ok);

		apic_id = apic_id ? apic_id - 1 : _vcpus_up - 1;
	}

	if (verbose_debug)
		Genode::log("RESET device state");

	MessageLegacy msg2(MessageLegacy::RESET, 0);
	_motherboard.bus_legacy.send_fifo(msg2);

	if (verbose_debug)
		Genode::log("INIT done");

	for (auto &vcpu : _vcpus) {
		if (vcpu)
			vcpu->start();
	}
}

void Component::construct(Genode::Env &env)
{
	static Vmm vmm(env);

	Genode::log("--- Seoul VMM starting ---");

	/* request max available memory */
	auto vm_size = env.pd().avail_ram().value;
	/* reserve some memory for the VMM */
	vm_size -= vmm.vmm_size;
	/* calculate max memory for the VM */
	vm_size = vm_size & ~((1UL << PAGE_SIZE_LOG2) - 1);

	Genode::log(" VMM memory ", Genode::Number_of_bytes(vmm.vmm_size));
	Genode::log(" using ", vmm.map_small ? "small": "large",
	            " memory attachments for guest VM.");
	if (vmm.rdtsc_exit)
		Genode::log(" enabling VM exit on RDTSC.");

	unsigned const width  = vmm.config.node().attribute_value("width", 1024U);
	unsigned const height = vmm.config.node().attribute_value("height", 768U);

	Genode::log(" framebuffer ", width, "x", height);

	/* setup guest memory */
	static Seoul::Guest_memory guest_memory(env, vmm.heap, vmm.vm_con, vm_size,
	                                        vmm.memory_verbose);

	/* diagnostic messages */
	guest_memory.dump_regions();

	if (!guest_memory.backing_store_local_base()) {
		Genode::error("Not enough space left for ",
		              guest_memory.backing_store_local_base() ? "framebuffer"
		                                                      : "VMM");
		env.parent().exit(-1);
		return;
	}

	/* register device models of Seoul, see device_model_registry.cc */
	env.exec_static_constructors();

	Genode::log("\n--- Setup VM ---");

	heap_init_env(&vmm.heap);

	/* create the PC machine based on the configuration given */
	static Machine machine(guest_memory, vmm);

	static Seoul::Console vcon(env, vmm.heap, machine.motherboard(),
	                           Gui::Area(width, height), guest_memory);

	static Seoul::Disk vdisk(env, machine.motherboard(),
	                         guest_memory.backing_store_local_base(),
	                         guest_memory.backing_store_size());

	vmm.config.node().with_sub_node("machine",
		[&] (auto const &sub_node) { machine.setup_devices(vmm.config.node(), sub_node); },
		[&] { Genode::warning("missing 'machine' config node"); });

	Genode::log("\n--- Booting VM ---");

	machine.boot(vmm.cpuid_native);
}
