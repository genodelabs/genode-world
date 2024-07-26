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
 * under the terms of the GNU General Public License version 2.
 */

#include <base/log.h>

#include "state.h"

void Seoul::write_vcpu_state(CpuState &seoul, unsigned mtr, Genode::Vcpu_state &state)
{
	state.discharge(); /* reset */

	if (mtr & MTD_GPR_ACDB) {
		state.ax.charge(seoul.rax);
		state.cx.charge(seoul.rcx);
		state.dx.charge(seoul.rdx);
		state.bx.charge(seoul.rbx);
		mtr &= ~MTD_GPR_ACDB;
	}

	if (mtr & MTD_GPR_BSD) {
		state.di.charge(seoul.rdi);
		state.si.charge(seoul.rsi);
		state.bp.charge(seoul.rbp);
		mtr &= ~MTD_GPR_BSD;
	}

	if (mtr & MTD_RIP_LEN) {
		state.ip.charge(seoul.rip);
		state.ip_len.charge(seoul.inst_len);
		mtr &= ~MTD_RIP_LEN;
	}

	if (mtr & MTD_RSP) {
		state.sp.charge(seoul.rsp);
		mtr &= ~MTD_RSP;
	}

	if (mtr & MTD_RFLAGS) {
		state.flags.charge(seoul.rfl);
		mtr &= ~MTD_RFLAGS;
	}

	if (mtr & MTD_DR) {
		state.dr7.charge(seoul.dr7);
		mtr &= ~MTD_DR;
	}

	if (mtr & MTD_CR) {
		state.cr0.charge(seoul.cr0);
		state.cr2.charge(seoul.cr2);
		state.cr3.charge(seoul.cr3);
		state.cr4.charge(seoul.cr4);
		mtr &= ~MTD_CR;
	}

	typedef Genode::Vcpu_state::Segment Segment;
	typedef Genode::Vcpu_state::Range   Range;

	if (mtr & MTD_CS_SS) {
		state.cs.charge(Segment { .sel   = seoul.cs.sel,
		                          .ar    = seoul.cs.ar,
		                          .limit = seoul.cs.limit,
		                          .base  = seoul.cs.base });
		state.ss.charge(Segment { .sel   = seoul.ss.sel,
		                          .ar    = seoul.ss.ar,
		                          .limit = seoul.ss.limit,
		                          .base  = seoul.ss.base });
		mtr &= ~MTD_CS_SS;
	}

	if (mtr & MTD_DS_ES) {
		state.es.charge(Segment { .sel   = seoul.es.sel,
		                          .ar    = seoul.es.ar,
		                          .limit = seoul.es.limit,
		                          .base  = seoul.es.base });
		state.ds.charge(Segment { .sel   = seoul.ds.sel,
		                          .ar    = seoul.ds.ar,
		                          .limit = seoul.ds.limit,
		                          .base  = seoul.ds.base });
		mtr &= ~MTD_DS_ES;
	}

	if (mtr & MTD_FS_GS) {
		state.fs.charge(Segment { .sel   = seoul.fs.sel,
		                          .ar    = seoul.fs.ar,
		                          .limit = seoul.fs.limit,
		                          .base  = seoul.fs.base });
		state.gs.charge(Segment { .sel   = seoul.gs.sel,
		                          .ar    = seoul.gs.ar,
		                          .limit = seoul.gs.limit,
		                          .base  = seoul.gs.base });
		mtr &= ~MTD_FS_GS;
	}

	if (mtr & MTD_TR) {
		state.tr.charge(Segment { .sel   = seoul.tr.sel,
		                          .ar    = seoul.tr.ar,
		                          .limit = seoul.tr.limit,
		                          .base  = seoul.tr.base });
		mtr &= ~MTD_TR;
	}

	if (mtr & MTD_LDTR) {
		state.ldtr.charge(Segment { .sel   = seoul.ld.sel,
		                            .ar    = seoul.ld.ar,
		                            .limit = seoul.ld.limit,
		                            .base  = seoul.ld.base });
		mtr &= ~MTD_LDTR;
	}

	if (mtr & MTD_GDTR) {
		state.gdtr.charge(Range { .limit = seoul.gd.limit,
		                          .base  = seoul.gd.base });
		mtr &= ~MTD_GDTR;
	}

	if (mtr & MTD_IDTR) {
		state.idtr.charge(Range{ .limit = seoul.id.limit,
		                         .base  = seoul.id.base });
		mtr &= ~MTD_IDTR;
	}

	if (mtr & MTD_SYSENTER) {
		state.sysenter_cs.charge(seoul.sysenter_cs);
		state.sysenter_sp.charge(seoul.sysenter_esp);
		state.sysenter_ip.charge(seoul.sysenter_eip);
		mtr &= ~MTD_SYSENTER;
	}

	if (mtr & MTD_QUAL) {
		state.qual_primary.charge(seoul.qual[0]);
		state.qual_secondary.charge(seoul.qual[1]);
		/* not read by any kernel */
		mtr &= ~MTD_QUAL;
	}

	if (mtr & MTD_CTRL) {
		state.ctrl_primary.charge(seoul.ctrl[0]);
		state.ctrl_secondary.charge(seoul.ctrl[1]);
		mtr &= ~MTD_CTRL;
	}

	if (mtr & MTD_INJ) {
		state.inj_info.charge(seoul.inj_info);
		state.inj_error.charge(seoul.inj_error);
		mtr &= ~MTD_INJ;
	}

	if (mtr & MTD_STATE) {
		state.intr_state.charge(seoul.intr_state);
		state.actv_state.charge(seoul.actv_state);
		mtr &= ~MTD_STATE;
	}

	if (mtr & MTD_TSC) {
		state.tsc.charge(seoul.tsc_value);
		state.tsc_offset.charge(seoul.tsc_off);
		mtr &= ~MTD_TSC;
	}

	if (mtr & MTD_XSAVE) {
		state.xcr0.charge(seoul.xcr0);
		state.xss.charge(seoul.xss);
		mtr &= ~MTD_XSAVE;
	}

#ifdef __x86_64__
	if (mtr & MTD_EFER) {
		state.efer.charge(seoul.efer);
		mtr &= ~MTD_EFER;
	}

	if (mtr & MTD_R8_R15) {
		state. r8.charge(seoul.gpr[ 8]);
		state. r9.charge(seoul.gpr[ 9]);
		state.r10.charge(seoul.gpr[10]);
		state.r11.charge(seoul.gpr[11]);
		state.r12.charge(seoul.gpr[12]);
		state.r13.charge(seoul.gpr[13]);
		state.r14.charge(seoul.gpr[14]);
		state.r15.charge(seoul.gpr[15]);
		mtr &= ~MTD_R8_R15;
	}

	if (mtr & MTD_SYSCALL_SWAPGS) {
		state. star.charge(seoul.star);
		state.cstar.charge(seoul.cstar);
		state.lstar.charge(seoul.lstar);
		state.fmask.charge(seoul.fmask);
		state.kernel_gs_base.charge(seoul.kernel_gs);
		mtr &= ~MTD_SYSCALL_SWAPGS;
	}

	if (mtr & MTD_PDPTE) {
		state.pdpte_0.charge(0);
		state.pdpte_1.charge(0);
		state.pdpte_2.charge(0);
		state.pdpte_3.charge(0);
		mtr &= ~MTD_PDPTE;
	}
#endif

	if (mtr)
		Genode::error("state transfer incomplete ", Genode::Hex(mtr));
}

unsigned Seoul::read_vcpu_state(Genode::Vcpu_state &state, CpuState &seoul)
{
	unsigned mtr = 0;

	if (state.ax.charged() || state.cx.charged() ||
	    state.dx.charged() || state.bx.charged()) {

		if (!state.ax.charged() || !state.cx.charged() ||
		    !state.dx.charged() || !state.bx.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_GPR_ACDB;

		seoul.rax   = state.ax.value();
		seoul.rcx   = state.cx.value();
		seoul.rdx   = state.dx.value();
		seoul.rbx   = state.bx.value();
	}

	if (state.bp.charged() || state.di.charged() || state.si.charged()) {

		if (!state.bp.charged() || !state.di.charged() || !state.si.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_GPR_BSD;
		seoul.rdi = state.di.value();
		seoul.rsi = state.si.value();
		seoul.rbp = state.bp.value();
	}

	if (state.flags.charged()) {
		mtr |= MTD_RFLAGS;
		seoul.rfl = state.flags.value();
	}

	if (state.sp.charged()) {
		mtr |= MTD_RSP;
		seoul.rsp = state.sp.value();
	}

	if (state.ip.charged() || state.ip_len.charged()) {
		if (!state.ip.charged() || !state.ip_len.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_RIP_LEN;
		seoul.rip      = state.ip.value();
		seoul.inst_len = state.ip_len.value();
	}

	if (state.dr7.charged()) {
		mtr |= MTD_DR;
		seoul.dr7 = state.dr7.value();
	}

	if (state.cr0.charged() || state.cr2.charged() ||
	    state.cr3.charged() || state.cr4.charged()) {

		mtr |= MTD_CR;

		seoul.cr0 = state.cr0.value();
		seoul.cr2 = state.cr2.value();
		seoul.cr3 = state.cr3.value();
		seoul.cr4 = state.cr4.value();
	}

	if (state.cs.charged() || state.ss.charged()) {
		if (!state.cs.charged() || !state.ss.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_CS_SS;

		seoul.cs.sel   = state.cs.value().sel;
		seoul.cs.ar    = state.cs.value().ar;
		seoul.cs.limit = state.cs.value().limit;
		seoul.cs.base  = state.cs.value().base;

		seoul.ss.sel   = state.ss.value().sel;
		seoul.ss.ar    = state.ss.value().ar;
		seoul.ss.limit = state.ss.value().limit;
		seoul.ss.base  = state.ss.value().base;
	}

	if (state.es.charged() || state.ds.charged()) {
		if (!state.es.charged() || !state.ds.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_DS_ES;

		seoul.es.sel   = state.es.value().sel;
		seoul.es.ar    = state.es.value().ar;
		seoul.es.limit = state.es.value().limit;
		seoul.es.base  = state.es.value().base;

		seoul.ds.sel   = state.ds.value().sel;
		seoul.ds.ar    = state.ds.value().ar;
		seoul.ds.limit = state.ds.value().limit;
		seoul.ds.base  = state.ds.value().base;
	}

	if (state.fs.charged() || state.gs.charged()) {
		if (!state.fs.charged() || !state.gs.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_FS_GS;

		seoul.fs.sel   = state.fs.value().sel;
		seoul.fs.ar    = state.fs.value().ar;
		seoul.fs.limit = state.fs.value().limit;
		seoul.fs.base  = state.fs.value().base;

		seoul.gs.sel   = state.gs.value().sel;
		seoul.gs.ar    = state.gs.value().ar;
		seoul.gs.limit = state.gs.value().limit;
		seoul.gs.base  = state.gs.value().base;
	}

	if (state.tr.charged()) {
		mtr |= MTD_TR;
		seoul.tr.sel   = state.tr.value().sel;
		seoul.tr.ar    = state.tr.value().ar;
		seoul.tr.limit = state.tr.value().limit;
		seoul.tr.base  = state.tr.value().base;
	}

	if (state.ldtr.charged()) {
		mtr |= MTD_LDTR;
		seoul.ld.sel   = state.ldtr.value().sel;
		seoul.ld.ar    = state.ldtr.value().ar;
		seoul.ld.limit = state.ldtr.value().limit;
		seoul.ld.base  = state.ldtr.value().base;
	}

	if (state.gdtr.charged()) {
		mtr |= MTD_GDTR;
		seoul.gd.limit = state.gdtr.value().limit;
		seoul.gd.base  = state.gdtr.value().base;
	}

	if (state.idtr.charged()) {
		mtr |= MTD_IDTR;
		seoul.id.limit = state.idtr.value().limit;
		seoul.id.base  = state.idtr.value().base;
	}

	if (state.sysenter_cs.charged() || state.sysenter_sp.charged() ||
	    state.sysenter_ip.charged()) {

		if (!state.sysenter_cs.charged() || !state.sysenter_sp.charged() ||
		    !state.sysenter_ip.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_SYSENTER;

		seoul.sysenter_cs = state.sysenter_cs.value();
		seoul.sysenter_esp = state.sysenter_sp.value();
		seoul.sysenter_eip = state.sysenter_ip.value();
	}

	if (state.ctrl_primary.charged() || state.ctrl_secondary.charged()) {
		if (!state.ctrl_primary.charged() || !state.ctrl_secondary.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_CTRL;

		seoul.ctrl[0] = state.ctrl_primary.value();
		seoul.ctrl[1] = state.ctrl_secondary.value();
	}

	if (state.inj_info.charged() || state.inj_error.charged()) {
		if (!state.inj_info.charged() || !state.inj_error.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_INJ;

		seoul.inj_info  = state.inj_info.value();
		seoul.inj_error = state.inj_error.value();
	}

	if (state.intr_state.charged() || state.actv_state.charged()) {
		if (!state.intr_state.charged() || !state.actv_state.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_STATE;

		seoul.intr_state = state.intr_state.value();
		seoul.actv_state = state.actv_state.value();
	}

	if (state.tsc.charged() || state.tsc_offset.charged()) {
		if (!state.tsc.charged() || !state.tsc_offset.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_TSC;

		seoul.tsc_value = state.tsc.value();
		seoul.tsc_off   = state.tsc_offset.value();
	}

	if (state.xcr0.charged() || state.xss.charged()) {
		if (!state.xcr0.charged() || !state.xss.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_XSAVE;

		seoul.xcr0 = state.xcr0.value();
		seoul.xss  = state.xss.value();
	}

	if (state.qual_primary.charged() || state.qual_secondary.charged()) {
		if (!state.qual_primary.charged() || !state.qual_secondary.charged())
			Genode::warning("missing state ", __LINE__);

		mtr |= MTD_QUAL;

		seoul.qual[0] = state.qual_primary.value();
		seoul.qual[1] = state.qual_secondary.value();
	}

#ifdef __x86_64__
	if (state.efer.charged()) {
		mtr |= MTD_EFER;
		seoul.efer = state.efer.value();
	}

	if (state.r8.charged()) {
		mtr |= MTD_R8_R15;
		seoul.gpr[ 8] = state. r8.value();
		seoul.gpr[ 9] = state. r9.value();
		seoul.gpr[10] = state.r10.value();
		seoul.gpr[11] = state.r11.value();
		seoul.gpr[12] = state.r12.value();
		seoul.gpr[13] = state.r13.value();
		seoul.gpr[14] = state.r14.value();
		seoul.gpr[15] = state.r15.value();
	}

	if (state.star.charged() || state.lstar.charged() || state.cstar.charged() ||
	    state.fmask.charged() || state.kernel_gs_base.charged()) {

		mtr |= MTD_SYSCALL_SWAPGS;

		seoul.star  = state. star.value();
		seoul.cstar = state.cstar.value();
		seoul.lstar = state.lstar.value();
		seoul.fmask = state.fmask.value();
		seoul.kernel_gs = state.kernel_gs_base.value();
	}
#endif

#if 0
	if (state.pdpte_0.charged() || state.pdpte_1.charged() ||
	    state.pdpte_2.charged() || state.pdpte_3.charged()) {

		Genode::warning("pdpte not supported by Seoul ",
		                Genode::Hex(state.pdpte_0.value()), " ",
		                Genode::Hex(state.pdpte_1.value()), " ",
		                Genode::Hex(state.pdpte_2.value()), " ",
		                Genode::Hex(state.pdpte_3.value()));
	}

	if (state.tpr.charged() || state.tpr_threshold.charged()) {
		Genode::warning("tpr not supported by Seoul");
	}
#endif

	return mtr;
}

void Seoul::dump(unsigned mtr, Genode::Vcpu_state const &state, CpuState const &seoul)
{
	using namespace Genode;

	auto output_reg = [&](auto const &string, auto const &value, auto const &value2) {
		auto const diff_xor = value ^ value2;

		if (diff_xor)
			log(string, " ", Hex(value , Hex::PREFIX, Hex::PAD),
			            " ", Hex(value2, Hex::PREFIX, Hex::PAD),
			            " ^:", Hex(diff_xor));
	};

	if (mtr & MTD_GPR_ACDB) {
		output_reg("rax", state.ax.value(), seoul.rax);
		output_reg("rcx", state.cx.value(), seoul.rcx);
		output_reg("rdx", state.dx.value(), seoul.rdx);
		output_reg("rbx", state.bx.value(), seoul.rbx);
		mtr &= ~MTD_GPR_ACDB;
	}

	if (mtr & MTD_GPR_BSD) {
		output_reg("rdi", state.di.value(), seoul.rdi);
		output_reg("rsi", state.si.value(), seoul.rsi);
		output_reg("rbp", state.bp.value(), seoul.rbp);
		mtr &= ~MTD_GPR_BSD;
	}

	if (mtr & MTD_RIP_LEN) {
		output_reg("rip"   , state.ip.value(),     seoul.rip);
		output_reg("ip_len", state.ip_len.value(), seoul.inst_len);
		mtr &= ~MTD_RIP_LEN;
	}

	if (mtr & MTD_RSP) {
		output_reg("rsp", state.sp.value(), seoul.rsp);
		mtr &= ~MTD_RSP;
	}

	if (mtr & MTD_RFLAGS) {
		output_reg("rfl", state.flags.value(), seoul.rfl);
		mtr &= ~MTD_RFLAGS;
	}

	if (mtr & MTD_DR) {
		output_reg("dr7", state.dr7.value(), seoul.dr7);
		mtr &= ~MTD_DR;
	}

	if (mtr & MTD_CR) {
		output_reg("cr0", state.cr0.value(), seoul.cr0);
		output_reg("cr2", state.cr2.value(), seoul.cr2);
		output_reg("cr3", state.cr3.value(), seoul.cr3);
		output_reg("cr4", state.cr4.value(), seoul.cr4);
		mtr &= ~MTD_CR;
	}

	if (mtr & MTD_CS_SS) {
		output_reg("cs   sel", state.cs.value().sel,   seoul.cs.sel);
		output_reg("cs    ar", state.cs.value().ar,    seoul.cs.ar);
		output_reg("cs limit", state.cs.value().limit, seoul.cs.limit);
		output_reg("cs base" , state.cs.value().base,  seoul.cs.base);

		output_reg("ss   sel", state.ss.value().sel,   seoul.ss.sel);
		output_reg("ss    ar", state.ss.value().ar,    seoul.ss.ar);
		output_reg("ss limit", state.ss.value().limit, seoul.ss.limit);
		output_reg("ss base" , state.ss.value().base,  seoul.ss.base);
		mtr &= ~MTD_CS_SS;
	}

	if (mtr & MTD_DS_ES) {
		output_reg("ds   sel", state.ds.value().sel,   seoul.ds.sel);
		output_reg("ds    ar", state.ds.value().ar,    seoul.ds.ar);
		output_reg("ds limit", state.ds.value().limit, seoul.ds.limit);
		output_reg("ds base" , state.ds.value().base,  seoul.ds.base);

		output_reg("es   sel", state.es.value().sel,   seoul.es.sel);
		output_reg("es    ar", state.es.value().ar,    seoul.es.ar);
		output_reg("es limit", state.es.value().limit, seoul.es.limit);
		output_reg("es base" , state.es.value().base,  seoul.es.base);
		mtr &= ~MTD_DS_ES;
	}

	if (mtr & MTD_FS_GS) {
		output_reg("fs   sel", state.fs.value().sel,   seoul.fs.sel);
		output_reg("fs    ar", state.fs.value().ar,    seoul.fs.ar);
		output_reg("fs limit", state.fs.value().limit, seoul.fs.limit);
		output_reg("fs base" , state.fs.value().base,  seoul.fs.base);

		output_reg("gs   sel", state.gs.value().sel,   seoul.gs.sel);
		output_reg("gs    ar", state.gs.value().ar,    seoul.gs.ar);
		output_reg("gs limit", state.gs.value().limit, seoul.gs.limit);
		output_reg("gs base" , state.gs.value().base,  seoul.gs.base);
		mtr &= ~MTD_FS_GS;
	}

	if (mtr & MTD_TR) {
		output_reg("tr   sel", state.tr.value().sel,   seoul.tr.sel);
		output_reg("tr    ar", state.tr.value().ar,    seoul.tr.ar);
		output_reg("tr limit", state.tr.value().limit, seoul.tr.limit);
		output_reg("tr base" , state.tr.value().base,  seoul.tr.base);
		mtr &= ~MTD_TR;
	}

	if (mtr & MTD_LDTR) {
		output_reg("ldtr   sel", state.ldtr.value().sel,   seoul.ld.sel);
		output_reg("ldtr    ar", state.ldtr.value().ar,    seoul.ld.ar);
		output_reg("ldtr limit", state.ldtr.value().limit, seoul.ld.limit);
		output_reg("ldtr base" , state.ldtr.value().base,  seoul.ld.base);
		mtr &= ~MTD_LDTR;
	}

	if (mtr & MTD_GDTR) {
		output_reg("gdtr limit", state.gdtr.value().limit, seoul.gd.limit);
		output_reg("gdtr base" , state.gdtr.value().base,  seoul.gd.base);
		mtr &= ~MTD_GDTR;
	}

	if (mtr & MTD_IDTR) {
		output_reg("idtr limit", state.idtr.value().limit, seoul.id.limit);
		output_reg("idtr base" , state.idtr.value().base,  seoul.id.base);
		mtr &= ~MTD_IDTR;
	}

	if (mtr & MTD_SYSENTER) {
		output_reg("sysenter_cs", state.sysenter_cs.value(), seoul.sysenter_cs);
		output_reg("sysenter_sp", state.sysenter_sp.value(), seoul.sysenter_esp);
		output_reg("sysenter_ip", state.sysenter_ip.value(), seoul.sysenter_eip);
		mtr &= ~MTD_SYSENTER;
	}

	if (mtr & MTD_QUAL) {
		output_reg("qual_primary",   state.qual_primary.value(),   seoul.qual[0]);
		output_reg("qual_secondary", state.qual_secondary.value(), seoul.qual[1]);
		mtr &= ~MTD_QUAL;
	}

	if (mtr & MTD_CTRL) {
		output_reg("ctrl_primary",   state.ctrl_primary.value(),   seoul.ctrl[0]);
		output_reg("ctrl_secondary", state.ctrl_secondary.value(), seoul.ctrl[1]);
		mtr &= ~MTD_CTRL;
	}

	if (mtr & MTD_INJ) {
		output_reg("inj_info",  state.inj_info.value(),  seoul.inj_info);
		output_reg("inj_error", state.inj_error.value(), seoul.inj_error);
		mtr &= ~MTD_INJ;
	}

	if (mtr & MTD_STATE) {
		output_reg("intr_state", state.intr_state.value(), seoul.intr_state);
		output_reg("actv_state", state.actv_state.value(), seoul.actv_state);
		mtr &= ~MTD_STATE;
	}

	if (mtr & MTD_TSC) {
		output_reg("tsc    ", state.tsc.value(),        seoul.tsc_value);
		output_reg("tsc_off", state.tsc_offset.value(), seoul.tsc_off);
		mtr &= ~MTD_TSC;
	}

	if (mtr & MTD_XSAVE) {
		output_reg("xcr0", state.xcr0.value(), seoul.xcr0);
		output_reg("xss", state.xss.value(), seoul.xss);
		mtr &= ~MTD_XSAVE;
	}

#ifdef __x86_64__
	if (mtr & MTD_EFER) {
		output_reg("efer", state.efer.value(), seoul.efer);
		mtr &= ~MTD_EFER;
	}

	if (mtr & MTD_R8_R15) {
		output_reg("r8 ", state.r8.value() , seoul.r8);
		output_reg("r9 ", state.r9.value() , seoul.r9);
		output_reg("r10", state.r10.value(), seoul.r10);
		output_reg("r11", state.r11.value(), seoul.r11);
		output_reg("r12", state.r12.value(), seoul.r12);
		output_reg("r13", state.r13.value(), seoul.r13);
		output_reg("r14", state.r14.value(), seoul.r14);
		output_reg("r15", state.r15.value(), seoul.r15);
		mtr &= ~MTD_R8_R15;
	}

	if (mtr & MTD_SYSCALL_SWAPGS) {
		output_reg("star ", state.star.value() , seoul.star);
		output_reg("cstar", state.cstar.value(), seoul.cstar);
		output_reg("lstar", state.lstar.value(), seoul.lstar);
		output_reg("fmask", state.fmask.value(), seoul.fmask);
		output_reg("kernel_gs_base", state.kernel_gs_base.value(), seoul.kernel_gs);
		mtr &= ~MTD_SYSCALL_SWAPGS;
	}

	if (mtr & MTD_PDPTE) {
		output_reg("pdpte_0", state.pdpte_0.value() , seoul.pdpte[0]);
		output_reg("pdpte_1", state.pdpte_1.value() , seoul.pdpte[1]);
		output_reg("pdpte_2", state.pdpte_2.value() , seoul.pdpte[2]);
		output_reg("pdpte_3", state.pdpte_3.value() , seoul.pdpte[3]);
		mtr &= ~MTD_PDPTE;
	}
#endif

	if (mtr)
		error("unknown state to dump ", Hex(mtr));
}
