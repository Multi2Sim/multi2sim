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

#include <lib/cpp/Misc.h>

#include "Extended.h"
#include "XMMValue.h"


namespace x86
{


/// Class representing the state of the x86 architected register file.
class Regs
{
	union
	{
		// View of the main set of registers as defined in the register
		// file specification of x86.
		struct
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

		} __attribute__ ((packed));

		// View of the register values as a sequence of bytes, useful
		// for efficient access using the register info table, given a
		// register identifier.
		char bytes[44];
	};

	// Program counter and flags
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

	// Register info table, used to efficiently access a register given its
	// identifier (\c InstRegXXX constant).
	static const struct Info
	{
		int offset;
		int size;
	} info[InstRegCount];

	// Table indexed by a number of bytes, returning a mask that can be
	// applied on a value to filter that number of least significant bytes
	static const unsigned mask[5];

public:

	/// Constructor
	Regs();

	/// Read one of the main x86 registers (registers \a eax through \a gs),
	/// identified with an \c InstRegXXX constant. If the requested register
	/// is less than 32-bit wide, the read value is zero-extended.
	unsigned Read(InstReg reg) const;

	/// Write one of the main x86 registers (register \a eax through \a gs),
	/// identified with an \c InstRegXXX constant.
	void Write(InstReg reg, unsigned value);

	/// Set the value of a flag, given as an \c InstFlagXXX identifier.
	void setFlag(InstFlag flag) {
		eflags = misc::SetBit32(eflags, flag);
	}

	/// Clear the value of a flag, given as an \c InstFlagXXX identifier.
	void clearFlag(InstFlag flag) {
		eflags = misc::ClearBit32(eflags, flag);
	}

	/// Get the value of a flag, given as an \c InstFlagXXX constant.
	bool getFlag(InstFlag flag) {
		return misc::GetBit32(eflags, flag);
	}

	/// Read a 10-byte extended value from the FPU stack at \a index, given
	/// as a relative position to the top of the stack.
	Extended ReadFpu(int index) const;

	/// Write a 10-byte extended value to the FPU stack at \a index, given
	/// as a relative position to the top of the stack.
	void WriteFpu(int index, const Extended &value);

	/// Push a 10-byte extended value into the FPU stack.
	void PushFpu(const Extended &value);

	/// Pop a 10-byte extended value from the FPU stack.
	Extended PopFpu();

	/// Get a reference to XMM register \a index (0 to 7)
	XMMValue &getXMM(int index) {
		assert(misc::InRange(index, 0, 7));
		return xmm[index];
	}

	/// Get a constant reference to XMM register \a index (0 to 7)
	const XMMValue &getXMM(int index) const {
		assert(misc::InRange(index, 0, 7));
		return xmm[index];
	}

	/// Dump register in a human-readable fashion into an output stream (or
	/// standard output if argument \a os is omitted.
	void Dump(std::ostream &os = std::cout) const;

	/// Dump the floating-point stack in a human-readable fashion into an
	/// output stream (or standard output if argument \a os is omitted).
	void DumpFpuStack(std::ostream &os = std::cout) const;

	/// Operator \c << overloaded, invoking function Dump()
	friend std::ostream &operator<<(std::ostream &os, const Regs &regs) {
		regs.Dump(os);
		return os;
	}
};


}  // namespace x86

#endif

