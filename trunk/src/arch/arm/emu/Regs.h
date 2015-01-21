/*
 *  Multi2Sim
 *  Copyright (C) 2012  Sida Gu (gu.sid@husky.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_ARM_EMU_REGS_H
#define ARCH_ARM_EMU_REGS_H

#include <lib/cpp/Misc.h>

namespace ARM
{

// Arm Modes of Operation 
#define ARM_MODE_USER 0x10
#define ARM_MODE_UNDEF 0x1b
#define ARM_MODE_SYS 0x1f
#define ARM_MODE_SVC 0x13
#define ARM_MODE_IRQ 0x12
#define ARM_MODE_FIQ 0x11
#define ARM_MODE_ABT 0x17


struct regsPsr
{
	unsigned int mode	: 5; // [4:0]
	unsigned int thumb	: 1; // [5]
	unsigned int fiq  	: 1; // [6]
	unsigned int irq	: 1; // [7]
	unsigned int _reserved0	:16; // [23:8]
	unsigned int jaz	: 1; // [24]
	unsigned int _reserved1	: 2; // [26:25]
	unsigned int q		: 1; // [27]
	unsigned int v		: 1; // [28]
	unsigned int C		: 1; // [29]
	unsigned int z		: 1; // [30]
	unsigned int n		: 1; // [31]
};


struct regsCoproc
{
	unsigned int c0_cpuid;
	unsigned int c0_cachetype;
	unsigned int c0_ccsid[16]; // Cache size.
	unsigned int c0_clid; // Cache level.
	unsigned int c0_cssel; // Cache size selection.
	unsigned int c0_c1[8]; // Feature registers.
	unsigned int c0_c2[8]; // Instruction set registers.
	unsigned int c1_sys; // System control register.
	unsigned int c1_coproc; // Coprocessor access register.
	unsigned int c1_xscaleauxcr; // XScale auxiliary control register.
	unsigned int c1_scr; // secure config register.
	unsigned int c2_base0; // MMU translation table base 0.
	unsigned int c2_base1; // MMU translation table base 1.
	unsigned int c2_control; // MMU translation table base control.
	unsigned int c2_mask; // MMU translation table base selection mask.
	unsigned int c2_base_mask; // MMU translation table base 0 mask.
	unsigned int c2_data; // MPU data cachable bits.
	unsigned int c2_insn; // MPU instruction cachable bits.
	unsigned int c3; // MMU domain access control register MPU write buffer control.
	unsigned int c5_insn; // Fault status registers.
	unsigned int c5_data;
	unsigned int c6_region[8]; // MPU base/size registers.
	unsigned int c6_insn; // Fault address registers.
	unsigned int c6_data;
	unsigned int c7_par; // Translation result.
	unsigned int c9_insn; // Cache lockdown registers.
	unsigned int c9_data;
	unsigned int c9_pmcr; // performance monitor control register
	unsigned int c9_pmcnten; // perf monitor counter enables
	unsigned int c9_pmovsr; // perf monitor overflow status
	unsigned int c9_pmxevtyper; // perf monitor event type
	unsigned int c9_pmuserenr; // perf monitor user enable
	unsigned int c9_pminten; // perf monitor interrupt enables
	unsigned int c13_fcse; // FCSE PID.
	unsigned int c13_context; // Context ID.
	unsigned int c13_tls1; // User RW Thread register.
	unsigned int c13_tls2; // User RO Thread register.
	unsigned int c13_tls3; // Privileged Thread register.
	unsigned int c15_cpar; // XScale Coprocessor Access Register
	unsigned int c15_ticonfig; // TI925T configuration byte.
	unsigned int c15_i_max; // Maximum D-cache dirty line index.
	unsigned int c15_i_min; // Minimum D-cache dirty line index.
	unsigned int c15_threadid; // TI debugger thread-ID.
	unsigned int c15_config_base_address; // SCU base address.
	unsigned int c15_diagnostic; // diagnostic register
	unsigned int c15_power_diagnostic;
	unsigned int c15_power_control; // power control
};


class Regs
{
	// Integer registers 
	unsigned int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
	unsigned int sl, fp, ip, sp, lr, pc;

	regsPsr cpsr;
	regsPsr spsr;

	// TODO: Arrange all the coproc in structures 
	// System control coprocessor (cp15) 
	regsCoproc cp15;

public:
	/// Constructor Destructor
	Regs();
	~Regs();

	/// Read one word from GPR, n is the register number If n is great than 9,
	/// exit program
	unsigned int getRegister(int n);

	/// Set the value of a general purpose register.
	///
	/// \param n
	///      Register index. Must be a value between 0 and 9.
	///
	/// \param value
	///      New value for the register.
	void setRegister(int n, unsigned int value);

	/// Write pc with value
	void setPC(unsigned int value) { pc = value; }

	/// Increment value of register \c pc by \a value bytes.
	void incPC(int value) { pc += value; }

	/// Decrement value of register \c pc by \a value bytes
	void decPC(unsigned int value) { pc -= value; }

	/// Read from Program Counter
	unsigned int getPC(){ return pc; }

	/// Write lr with value
	void setLR(unsigned int value) { lr = value; }

	/// Read from Link Register
	unsigned int getLR(){ return lr; }

	/// Read from Current Program Status Register
	regsPsr& getCPSR(){ return cpsr; }

	/// Write SP with value
	void setSP(unsigned int value) { sp = value; }

	/// Read from Stack Pointer
	unsigned int getSP(){ return sp; }

	/// Write IP with value
	void setIP(unsigned int value) { ip = value; }

	/// Read from Instruction Pointer
	unsigned int getIP(){ return ip; }

	/// Write FP with value
	void setFP(unsigned int value) { fp = value; }

	/// Read from Frame Pointer
	unsigned int getFP(){ return fp; }

	/// Write SL with value
	void setSL(unsigned int value) { sl = value; }

	/// Read from Stack Limit
	unsigned int getSL(){ return sl; }

	/// Read from Co-processor Register
	regsCoproc& getCP15(){ return cp15; }

};

}

#endif // ARCH_ARM_EMU_REGS_H
