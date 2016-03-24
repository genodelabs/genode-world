/*
 * \brief  Base driver for ZC702 Board
 * \author Johannes Schlatow
 * \date   2016-03-24
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__ZC702__DRIVERS__BOARD_BASE_H_
#define _INCLUDE__ZC702__DRIVERS__BOARD_BASE_H_

#include <spec/zynq/drivers/board_base_support.h>

namespace Genode { struct Board_base; }

/**
 * Base driver for the ZC702 platform
 */
struct Genode::Board_base : Zynq::Board_base
{
	enum
	{
		/* clocks (assuming 6:2:1 mode) */
		CPU_1X_CLOCK   = 111111100,
		CPU_6X4X_CLOCK = 6*CPU_1X_CLOCK,

		CORTEX_A9_CLOCK             = CPU_6X4X_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_CLK = CORTEX_A9_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_DIV = 100,

		SDHCI_BASE = MMIO_0_BASE + 0x100000,
		SDHCI_SIZE = 0x100,
		SDHCI_IRQ  = 56,

		UART_1_MMIO_BASE = MMIO_0_BASE + UART_SIZE,
	};
};

#endif /* _INCLUDE__ZC702__DRIVERS__BOARD_BASE_H_ */
