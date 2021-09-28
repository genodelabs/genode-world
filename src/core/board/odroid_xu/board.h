/*
 * \brief  Board driver for core
 * \author Stefan Kalkowski
 * \date   2017-04-27
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__ODROID_XU__BOARD_H_
#define _CORE__SPEC__ODROID_XU__BOARD_H_

/* base-hw internal includes */
#include <hw/spec/arm/gicv2.h>
#include <hw/spec/arm/odroid_xu_board.h>

/* base-hw Core includes */
#include <spec/arm/exynos_mct.h>
#include <spec/cortex_a15/cpu.h>

namespace Board {
	using namespace Hw::Odroid_xu_board;

	class Global_interrupt_controller { };
	class Pic : public Hw::Gicv2 { public: Pic(Global_interrupt_controller &) { } };
}

#endif /* _CORE__SPEC__ODROID_XU__BOARD_H_ */
