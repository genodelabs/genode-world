/*
 * \brief   Zynq specific board definitions
 * \author  Stefan Kalkowski
 * \date    2017-02-20
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__BOOTSTRAP__BOARD__ZYNQ_ZEDBOARD__BOARD_H_
#define _SRC__BOOTSTRAP__BOARD__ZYNQ_ZEDBOARD__BOARD_H_

#include <drivers/defs/zynq_zedboard.h>
#include <drivers/uart/xilinx.h>
#include <hw/spec/arm/boot_info.h>
#include <hw/spec/arm/cortex_a9.h>
#include <hw/spec/arm/gicv2.h>
#include <hw/spec/arm/pl310.h>

#include <spec/arm/cortex_a9_actlr.h>
#include <spec/arm/cortex_a9_page_table.h>
#include <spec/arm/cpu.h>

namespace Board {
	struct L2_cache;

	using namespace Zynq_zedboard;
	using Cpu_mmio = Hw::Cortex_a9_mmio<CORTEX_A9_PRIVATE_MEM_BASE>;
	using Serial   = Genode::Xilinx_uart;
	using Pic = Hw::Gicv2;
	static constexpr bool NON_SECURE = false;

	enum {
		UART_BASE  = UART_1_MMIO_BASE,
	};
}

struct Board::L2_cache : Hw::Pl310
{
	L2_cache(Genode::addr_t mmio) : Hw::Pl310(mmio)
	{
		Aux::access_t aux = 0;
		Aux::Full_line_of_zero::set(aux, true);
		Aux::Associativity::set(aux, Aux::Associativity::WAY_8);
		Aux::Way_size::set(aux, Aux::Way_size::KB_64);
		Aux::Share_override::set(aux, true);
		Aux::Replacement_policy::set(aux, Aux::Replacement_policy::PRAND);
		Aux::Ns_lockdown::set(aux, true);
		Aux::Data_prefetch::set(aux, true);
		Aux::Inst_prefetch::set(aux, true);
		Aux::Early_bresp::set(aux, true);
		write<Aux>(aux);
	}

	using Hw::Pl310::invalidate;

	void enable()
	{
		Pl310::mask_interrupts();
		write<Control::Enable>(1);
	}

	void disable() {
		write<Control::Enable>(0);
	}
};

#endif /* _SRC__BOOTSTRAP__BOARD__ZYNQ_ZEDBOARD__BOARD_H_ */
