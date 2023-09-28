/*
 * \brief  Transform state between Genode VM session interface and Seoul
 * \author Alexander Boettcher
 * \author Benjamin Lamowski
 * \date   2018-08-27
 */

/*
 * Copyright (C) 2018-2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _STATE_H_
#define _STATE_H_

#include <cpu/vcpu_state.h>

#include <nul/vcpu.h>

namespace Seoul {
	void write_vcpu_state(CpuState &, unsigned mtr, Genode::Vcpu_state &);
	unsigned read_vcpu_state(Genode::Vcpu_state &, CpuState &);
}

#endif /* _STATE_H_ */
