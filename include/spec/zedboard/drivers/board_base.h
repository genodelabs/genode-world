/*
 * \brief  Base driver for Zedboard
 * \author Mark Albers
 * \date   2015-09-29
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__ZEDBOARD__DRIVERS__BOARD_BASE_H_
#define _INCLUDE__ZEDBOARD__DRIVERS__BOARD_BASE_H_

#include <spec/zynq/drivers/board_base_support.h>

namespace Genode { struct Board_base; }

/**
 * Base driver for the Zedboard platform
 */
struct Genode::Board_base : Zynq::Board_base
{
	enum
	{
		PS_CLOCK = 33333333,
		ARM_PLL_CLOCK = 1333333*1000,
		DDR_PLL_CLOCK = 1066667*1000,
		IO_PLL_CLOCK = 1000*1000*1000,
		CPU_1X_CLOCK = 111111115,
		CPU_6X4X_CLOCK = 6*CPU_1X_CLOCK,
		CPU_3X2X_CLOCK = 3*CPU_1X_CLOCK,
		CPU_2X_CLOCK = 2*CPU_1X_CLOCK,

		CORTEX_A9_CLOCK = CPU_6X4X_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_CLK = CORTEX_A9_CLOCK,

		RAM_0_SIZE = 0x20000000, /* 512MB */
		DDR_CLOCK = 533333313,

		FCLK_CLK0 = 100*1000*1000, /* AXI */
		FCLK_CLK1 = 20250*1000, /* Cam */
		FCLK_CLK2 = 150*1000*1000, /* AXI HP */

		I2C0_MMIO_BASE = 0xE0004000,
		I2C1_MMIO_BASE = 0xE0005000,
		I2C_MMIO_SIZE = 0x1000,


		QSPI_CLOCK = 200*1000*1000,
		ETH_CLOCK = 125*1000*1000,
		SD_CLOCK = 50*1000*1000,

		GPIO_MMIO_SIZE = 0x1000,
		VDMA_MMIO_SIZE = 0x10000,
	};
};

#endif /* _INCLUDE__ZEDBOARD__DRIVERS__BOARD_BASE_H_ */
