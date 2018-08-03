/*
 * \brief  Base driver for ZC706 Board
 * \author Johannes Schlatow
 * \date   2016-03-24
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__ZC706__DRIVERS__BOARD_BASE_H_
#define _INCLUDE__ZC706__DRIVERS__BOARD_BASE_H_

#include <drivers/defs/zynq.h>

namespace Zynq_zc706 {

	using namespace Zynq;

	enum {
		/* clocks (assuming 6:2:1 mode) with
		 *   - 33.33333Mhz PS_CLK
		 *   - PLL = 40 * PS_CLK
		 *   - CPU_6x4x = PLL / 2
		 */
		CPU_1X_CLOCK   = 111111100,
		CPU_3X2X_CLOCK = 3*CPU_1X_CLOCK,
		CPU_6X4X_CLOCK = 6*CPU_1X_CLOCK,

		CORTEX_A9_CLOCK             = 2*CPU_6X4X_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_CLK = CPU_3X2X_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_DIV = 100,

		SDHCI_BASE = MMIO_0_BASE + 0x100000,
		SDHCI_SIZE = 0x100,
		SDHCI_IRQ  = 56,

		UART_1_MMIO_BASE = MMIO_0_BASE + UART_SIZE,
	};
};

#endif /* _INCLUDE__ZC706__DRIVERS__BOARD_BASE_H_ */
