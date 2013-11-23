/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_X86_EMU_REGS_H
#define ARCH_X86_EMU_REGS_H

#include <iostream>

#include "Extended.h"
#include "XMMValue.h"


namespace x86
{


class Regs
{
	// General-purpose registers
	unsigned eax;
	unsigned ecx;
	unsigned edx;
	unsigned ebx;

	// Special-purpose registers
	unsigned esp;
	unsigned ebp;
	unsigned esi;
	unsigned edi;

	// Segment registers
	unsigned short es;
	unsigned short cs;
	unsigned short ss;
	unsigned short ds;
	unsigned short fs;
	unsigned short gs;

	unsigned int eip;
	unsigned int eflags;

	// Floating-point stack
	struct
	{
		Extended value;
		bool valid;
	} fpu_stack[8];

	// Top of the stack (field 'top' of status register)
	int fpu_top;

	// Field 'code' of status register (C3-C2-C1-C0)
	int fpu_code;

	// FPU control word
	unsigned short fpu_ctrl;

	// XMM registers (8 128-bit values)
	XMMValue xmm[8];

public:

	/// Constructor
	Regs();

	/// Dump register in a human-readable fashion into an output stream (or
	/// standard output if argument \a os is omitted.
	void Dump(std::ostream &os = std::cout);

	/// Dump the floating-point stack in a human-readable fashion into an
	/// output stream (or standard output if argument \a os is omitted).
	void DumpFPUStack(std::ostream &os = std::cout);

	/// Operator \c << overloaded, invoking function Dump()
	friend std::ostream &operator<<(std::ostream &os, const Regs &regs);
};


}  // namespace x86

#endif

