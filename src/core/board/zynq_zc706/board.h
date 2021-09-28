/*
 * \brief  Board driver for core on Zynq
 * \author Johannes Schlatow
 * \author Stefan Kalkowski
 * \author Martin Stein
 * \date   2014-06-02
 */

/*
 * Copyright (C) 2014-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__BOARD__ZYNQ_ZC706__BOARD_H_
#define _CORE__BOARD__ZYNQ_ZC706__BOARD_H_

/* core includes */
#include <drivers/defs/zynq_zc706.h>
#include <drivers/uart/xilinx.h>

/* base-hw internal includes */
#include <hw/spec/arm/boot_info.h>
#include <hw/spec/arm/cortex_a9.h>
#include <hw/spec/arm/gicv2.h>
#include <hw/spec/arm/pl310.h>

/* base-hw Core includes */
#include <spec/arm/cortex_a9_private_timer.h>
#include <spec/cortex_a9/cpu.h>

namespace Board {
	using namespace Zynq_zc706;
	using Cpu_mmio = Hw::Cortex_a9_mmio<CORTEX_A9_PRIVATE_MEM_BASE>;
	using L2_cache = Hw::Pl310;
	using Serial   = Genode::Xilinx_uart;

	class Global_interrupt_controller { };
	class Pic : public Hw::Gicv2 { public: Pic(Global_interrupt_controller &) { } };

	enum {
		UART_BASE  = UART_1_MMIO_BASE,
	};

	L2_cache & l2_cache();
}

#endif /* _CORE__BOARD__ZYNQ_ZC706__BOARD_H_ */
