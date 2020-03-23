/*
 * \brief   Pandaboard specific definitions
 * \author  Stefan Kalkowski
 * \date    2019-05-16
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__INCLUDE__HW__SPEC__ARM__PANDA_BOARD_H_
#define _SRC__INCLUDE__HW__SPEC__ARM__PANDA_BOARD_H_

#include <base/stdint.h>
#include <hw/spec/arm/boot_info.h>
#include <hw/spec/arm/cortex_a9.h>
#include <hw/spec/arm/pl310.h>
#include <hw/uart/tl16c750.h>

namespace Hw::Panda_board {

	enum {
		/* device IO memory */
		MMIO_0_BASE = 0x48000000,

		/* normal RAM */
		RAM_0_BASE = 0x80000000,
		RAM_0_SIZE = 0x40000000,

		/* L2 cache */
		PL310_MMIO_BASE = 0x48242000,
		PL310_MMIO_SIZE = 0x00001000,

		/* CPU */
		CORTEX_A9_PRIVATE_MEM_BASE  = 0x48240000,
		CORTEX_A9_PRIVATE_MEM_SIZE  = 0x00002000,
		CORTEX_A9_PRIVATE_TIMER_CLK = 400000000,
		CORTEX_A9_PRIVATE_TIMER_DIV = 200,
		CORTEX_A9_WUGEN_MMIO_BASE   = 0x48281000,
		CORTEX_A9_SCU_MMIO_BASE     = 0x48240000,

		TL16C750_3_MMIO_BASE = MMIO_0_BASE + 0x20000,
		TL16C750_MMIO_SIZE = 0x2000,
		TL16C750_CLOCK = 48*1000*1000,
	};

	using Cpu_mmio = Hw::Cortex_a9_mmio<CORTEX_A9_PRIVATE_MEM_BASE>;
	using Serial   = Genode::Tl16c750_uart;

	enum {
		UART_BASE  = TL16C750_3_MMIO_BASE,
		UART_CLOCK = TL16C750_CLOCK,
	};

	enum Panda_firmware_opcodes {
		CPU_ACTLR_SMP_BIT_RAISE =  0x25,
		L2_CACHE_SET_DEBUG_REG  = 0x100,
		L2_CACHE_ENABLE_REG     = 0x102,
		L2_CACHE_AUX_REG        = 0x109,
	};

	static inline void call_panda_firmware(Genode::addr_t func,
	                                       Genode::addr_t val)
	{
		register Genode::addr_t _func asm("r12") = func;
		register Genode::addr_t _val  asm("r0")  = val;
		asm volatile("dsb           \n"
		             "push {r1-r11} \n"
		             "smc  #0       \n"
		             "pop  {r1-r11} \n"
		             :: "r" (_func), "r" (_val) : "memory", "cc");
	}
}

#endif /* _SRC__INCLUDE__HW__SPEC__ARM__PANDA_BOARD_H_ */
