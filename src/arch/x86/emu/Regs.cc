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

const Regs::Info Regs::info[InstRegCount] =
{
	{ 0, 0 },
	{ 0, 4 },	// 1. eax
	{ 4, 4 },	// 2. ecx
	{ 8, 4 },	// 3. edx
	{ 12, 4 },	// 4. ebx
	{ 16, 4 },	// 5. esp
	{ 20, 4 },	// 6. ebp
	{ 24, 4 },	// 7. esi
	{ 28, 4 },	// 8. edi
	{ 0, 2 },	// 9. ax
	{ 4, 2 },	// 10. cx
	{ 8, 2 },	// 11. dx
	{ 12, 2 },	// 12. bx
	{ 16, 2 },	// 13. sp
	{ 20, 2 },	// 14. bp
	{ 24, 2 },	// 15. si
	{ 28, 2 },	// 16. di
	{ 0, 1 },	// 17. al
	{ 4, 1 },	// 18. cl
	{ 8, 1 },	// 19. dl
	{ 12, 1 },	// 20. bl
	{ 1, 1 },	// 21. ah
	{ 5, 1 },	// 22. ch
	{ 9, 1 },	// 23. dh
	{ 13, 1 },	// 24. bh
	{ 32, 2 },	// 25. es
	{ 34, 2 },	// 26. cs
	{ 36, 2 },	// 27. ss
	{ 38, 2 },	// 28. ds
	{ 40, 2 },	// 29. fs
	{ 42, 2 },	// 30. gs
};


const unsigned Regs::mask[5] =
{
	0,		// 0-bit
	0xff,		// 8-bit
	0xffff,		// 16-bit
	0,		// unused
	0xffffffff	// 32-bit
};


Regs::Regs()
{
	// Make sure that the structure containing the main registers (eax to
	// gs) is packed. This is critical for correct access of registers using
	// the register info table.
	assert((char *) &gs - (char *) &eax == 42);

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


unsigned Regs::Read(InstReg reg) const
{
	assert(inRange(reg, InstRegNone, InstRegCount - 1));
	unsigned *value_ptr = (unsigned *) ((char *) &eax + info[reg].offset);
	return *value_ptr & mask[info[reg].size];
}


void Regs::Write(InstReg reg, unsigned value)
{
	assert(inRange(reg, InstRegNone, InstRegCount - 1));
	unsigned mask = this->mask[info[reg].size];
	unsigned *value_ptr = (unsigned *) ((char *) &eax + info[reg].offset);
	*value_ptr = (*value_ptr & ~mask) | (value & mask);
}


Extended Regs::ReadFpu(int index) const
{
	// Invalid index
	if (!inRange(index, 0, 7))
		return 0.0;

	// Calculate effective index
	index = (fpu_top + index) % 8;
	if (!fpu_stack[index].valid)
		return 0.0;

	// Return
	return fpu_stack[index].value;
}


void Regs::WriteFpu(int index, const Extended &value)
{
	// Invalid index
	if (!inRange(index, 0, 7))
		return;

	// Calculate effective index
	index = (fpu_top + index) % 8;
	if (!fpu_stack[index].valid)
		return;

	// Write value
	fpu_stack[index].value = value;
}
	
	
void Regs::PushFpu(const Extended &value)
{
	fpu_top = (fpu_top + 7) % 8;
	fpu_stack[fpu_top].valid = true;
	fpu_stack[fpu_top].value = value;
}


Extended Regs::PopFpu()
{
	Extended &value = fpu_stack[fpu_top].value;
	fpu_stack[fpu_top].valid = false;
	fpu_top = (fpu_top + 1) % 8;
	return value;
}


void Regs::DumpFpuStack(std::ostream &os) const
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


void Regs::Dump(std::ostream &os) const
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
	DumpFpuStack(os);

	// Floating point code (part from status register)
	os << StringFmt("  fpu_code (C3-C2-C1-C0): %d-%d-%d-%d\n",
		getBit32(fpu_code, 3) > 0, getBit32(fpu_code, 2) > 0,
		getBit32(fpu_code, 1) > 0, getBit32(fpu_code, 0) > 0);
	os << StringFmt("  fpu_ctrl=%04x\n", fpu_ctrl);

	// XMM registers
	for (int i = 0; i < 8; i++)
		os << "  xmm" << i << " = " << xmm[i] << '\n';
}


}  // namespace x86

