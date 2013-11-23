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

#include <cstring>

#include <arch/x86/asm/Inst.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Regs.h"


using namespace misc;

namespace x86
{

Regs::Regs()
{
	// General-purpose registers
	eax = 0;
	ecx = 0;
	edx = 0;
	ebx = 0;

	// Special-purpose registers
	esp = 0;
	ebp = 0;
	esi = 0;
	edi = 0;

	// Segment registers
	es = 0;
	cs = 0;
	ss = 0;
	ds = 0;
	fs = 0;
	gs = 0;

	// Special registers
	eip = 0;
	eflags = 0;

	// Initialize floating-point stack
	for (int i = 0; i < 8; i++)
		fpu_stack[i].valid = false;

	// FPU special registers
	fpu_top = 0;
	fpu_code = 0;
	fpu_ctrl = 0x37f;
}


void Regs::DumpFPUStack(std::ostream &os)
{
	os << '{';
	int index = (fpu_top + 7) % 8;
	std::string comma;
	for (int i = 0; i < 8; i++)
	{
		if (fpu_stack[index].valid)
		{
			os << comma << fpu_stack[index].value;
			comma = ", ";
		}
		index = (index + 7) % 8;
	}
	os << "}\n";
}


void Regs::Dump(std::ostream &os)
{
	// Integer registers
	os << StringFmt("  eax=%08x  ecx=%08x  edx=%08x  ebx=%08x\n",
		eax, ecx, edx, ebx);
	os << StringFmt("  esp=%08x  ebp=%08x  esi=%08x  edi=%08x\n",
		esp, ebp, esi, edi);
	os << StringFmt("  es=%x, cs=%x, ss=%x, ds=%x, fs=%x, gs=%x\n",
		es, cs, ss, ds, fs, gs);
	os << StringFmt("  eip=%08x\n", eip);
	os << StringFmt("  flags=%04x (cf=%d  pf=%d  af=%d  zf=%d  sf=%d  df=%d  of=%d)\n",
		eflags,
		(eflags & (1 << InstFlagCF)) > 0,
		(eflags & (1 << InstFlagPF)) > 0,
		(eflags & (1 << InstFlagAF)) > 0,
		(eflags & (1 << InstFlagZF)) > 0,
		(eflags & (1 << InstFlagSF)) > 0,
		(eflags & (1 << InstFlagDF)) > 0,
		(eflags & (1 << InstFlagOF)) > 0);
	
	// Floating-point stack
	os << "  fpu_stack (last=top): ";
	DumpFPUStack(os);

	// Floating point code (part from status register)
	os << StringFmt("  fpu_code (C3-C2-C1-C0): %d-%d-%d-%d\n",
		GetBit32(fpu_code, 3) > 0, GetBit32(fpu_code, 2) > 0,
		GetBit32(fpu_code, 1) > 0, GetBit32(fpu_code, 0) > 0);
	os << StringFmt("  fpu_ctrl=%04x\n", fpu_ctrl);

	// XMM registers
	for (int i = 0; i < 8; i++)
		os << "  xmm" << i << " = " << xmm[i] << '\n';
}


}  // namespace x86

