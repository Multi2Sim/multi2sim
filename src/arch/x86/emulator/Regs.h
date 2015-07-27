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

#include <arch/x86/disassembler/Instruction.h>
#include <lib/cpp/Misc.h>

#include "Extended.h"
#include "XmmValue.h"


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
	unsigned eip;
	unsigned eflags;

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
	XmmValue xmm[8];

	// Register info table, used to efficiently access a register given its
	// identifier (\c Inst::RegXXX constant).
	static const struct Info
	{
		int offset;
		int size;
	} info[Instruction::RegCount];

	// Table indexed by a number of bytes, returning a mask that can be
	// applied on a value to filter that number of least significant bytes
	static const unsigned mask[5];

public:

	/// Constructor
	Regs();

	/// Read one of the main x86 registers (registers \a eax through \a gs),
	/// identified with an \c Inst::RegXXX constant. If the requested register
	/// is less than 32-bit wide, the read value is zero-extended. The type
	/// of argument \a reg is \c int in order to avoid conversion warnings
	/// when using arithmetic to compute it.
	unsigned Read(int reg) const;

	/// Write one of the main x86 registers (register \a eax through \a gs),
	/// identified with an \c Inst::RegXXX constant. Argument \a reg has type
	/// \a int to avoid cast warnings when using integer arithmetic to
	/// compute it.
	void Write(int reg, unsigned value);

	/// Set the value of a flag, given as an \c Inst::FlagXXX identifier.
	void setFlag(Instruction::Flag flag) {
		eflags = misc::setBit32(eflags, flag);
	}

	/// Clear the value of a flag, given as an \c Inst::FlagXXX identifier.
	void clearFlag(Instruction::Flag flag) {
		eflags = misc::clearBit32(eflags, flag);
	}

	/// Get the value of a flag, given as an \c Inst::FlagXXX constant.
	bool getFlag(Instruction::Flag flag) {
		return misc::getBit32(eflags, flag);
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
	XmmValue &getXMM(int index)
	{
		assert(misc::inRange(index, 0, 7));
		return xmm[index];
	}

	/// Get a constant reference to XMM register \a index (0 to 7)
	const XmmValue &getXMM(int index) const
	{
		assert(misc::inRange(index, 0, 7));
		return xmm[index];
	}

	/// Get value of register \c eax
	unsigned getEax() const { return eax; }

	/// Get value of register \c ax
	unsigned short getAx() const { return eax; }

	/// Get value of register \c al
	unsigned char getAl() const { return eax; }

	/// Get value of register \c ah
	unsigned char getAh() const { return eax >> 8; }

	/// Get value of register \c ebx
	unsigned getEbx() const { return ebx; }

	/// Get value of register \c bx
	unsigned short getBx() const { return ebx; }

	/// Get value of register \c bl
	unsigned char getBl() const { return ebx; }

	/// Get value of register \c bh
	unsigned char getBh() const { return ebx >> 8; }

	/// Get value of register \c ecx
	unsigned getEcx() const { return ecx; }

	/// Get value of register \c cx
	unsigned short getCx() const { return ecx; }

	/// Get value of register \c cl
	unsigned char getCl() const { return ecx; }

	/// Get value of register \c ch
	unsigned char getCh() const { return ecx >> 8; }

	/// Get value of register \c edx
	unsigned getEdx() const { return edx; }

	/// Get value of register \c dx
	unsigned short getDx() const { return edx; }

	/// Get value of register \c dl
	unsigned char getDl() const { return edx; }

	/// Get value of register \c dh
	unsigned char getDh() const { return edx >> 8; }

	/// Get value of register \c ebp
	unsigned getEbp() const { return ebp; }

	/// Get value of register \c bp
	unsigned short getBp() const { return ebp; }

	/// Get value of register \c esp
	unsigned getEsp() const { return esp; }

	/// Get value of register \c sp
	unsigned short getSp() const { return esp; }

	/// Get value of register \c esi
	unsigned getEsi() const { return esi; }

	/// Get value of register \c si
	unsigned short getSi() const { return esi; }

	/// Get value of register \c edi
	unsigned getEdi() const { return edi; }

	/// Get value of register \c di
	unsigned short getDi() const { return edi; }

	/// Get value of register \c eip
	unsigned getEip() const { return eip; }

	/// Get value of register \c eflags
	unsigned getEflags() const { return eflags; }

	/// Get value of register \c es
	unsigned short getEs() const { return es; }

	/// Get value of register \c cs
	unsigned short getCs() const { return cs; }

	/// Get value of register \c ss
	unsigned short getSs() const { return ss; }

	/// Get value of register \c ds
	unsigned short getDs() const { return ds; }

	/// Get value of register \c fs
	unsigned short getFs() const { return fs; }

	/// Get value of register \c gs
	unsigned short getGs() const { return gs; }

	/// Set value of register \c eax
	void setEax(unsigned value) { eax = value; }

	/// Set value of register \c ax
	void setAx(unsigned short value) { Write(Instruction::RegAx, value); }

	/// Set value of register \c al
	void setAl(unsigned char value) { Write(Instruction::RegAl, value); }

	/// Set value of register \c ah
	void setAh(unsigned char value) { Write(Instruction::RegAh, value); }

	/// Set value of register \c ebx
	void setEbx(unsigned value) { ebx = value; }

	/// Set value of register \c bx
	void setBx(unsigned short value) { Write(Instruction::RegBx, value); }

	/// Set value of register \c bl
	void setBl(unsigned char value) { Write(Instruction::RegBl, value); }

	/// Set value of register \c bh
	void setBh(unsigned char value) { Write(Instruction::RegBh, value); }

	/// Set value of register \c ecx
	void setEcx(unsigned value) { ecx = value; }

	/// Increment value of register \c ecs by \a value, or by 1 if no value
	/// is specified. Used by string operations.
	void incEcx(int value = 1) { ecx += value; }

	/// Increment value of register \c ecs by \a value, or by 1 if no value
	/// is specified. Used by string operations.
	void decEcx(int value = 1) { ecx -= value; }

	/// Set value of register \c cx
	void setCx(unsigned short value) { Write(Instruction::RegCx, value); }

	/// Set value of register \c cl
	void setCl(unsigned char value) { Write(Instruction::RegCl, value); }

	/// Set value of register \c ch
	void setCh(unsigned char value) { Write(Instruction::RegCh, value); }

	/// Set value of register \c edx
	void setEdx(unsigned value) { edx = value; }

	/// Set value of register \c dx
	void setDx(unsigned short value) { Write(Instruction::RegDx, value); }

	/// Set value of register \c dl
	void setDl(unsigned char value) { Write(Instruction::RegDl, value); }

	/// Set value of register \c dh
	void setDh(unsigned char value) { Write(Instruction::RegDh, value); }

	/// Set value of register \c esp
	void setEsp(unsigned value) { esp = value; }

	/// Increment stack pointer by \a value bytes. If argument \a value is
	/// omitted, the stack pointer is incremented by one word (4 bytes).
	void incEsp(int value = 4) { esp += value; }

	/// Decrement stack pointer by \a value bytes. If argument \a value is
	/// omitted, the stack pointer is decremented by one word (4 bytes).
	void decEsp(int value = 4) { esp -= value; }

	/// Set value of register \c sp
	void setSp(unsigned value) { Write(Instruction::RegSp, value); }

	/// Set value of register \c ebp
	void setEbp(unsigned value) { ebp = value; }

	/// Set value of register \c bp
	void setBp(unsigned value) { Write(Instruction::RegBp, value); }

	/// Set value of register \c esi
	void setEsi(unsigned value) { esi = value; }

	/// Increment value of register \a esi by \a value bytes, or by 1 if
	/// no value is specified. Used in string operations.
	void incEsi(int value = 1) { esi += value; }

	/// Set value of register \c si
	void setSi(unsigned value) { Write(Instruction::RegSi, value); }

	/// Set value of register \c edi
	void setEdi(unsigned value) { edi = value; }

	/// Increment value of register \a edi by \a value bytes, or by 1 if
	/// no value is specified. Used in string operations.
	void incEdi(int value = 1) { edi += value; }

	/// Set value of register \c di
	void setDi(unsigned value) { Write(Instruction::RegDi, value); }

	/// Set value of register \c es
	void setEs(unsigned short value) { es = value; }

	/// Set value of register \c cs
	void setCs(unsigned short value) { cs = value; }

	/// Set value of register \c ss
	void setSs(unsigned short value) { ss = value; }

	/// Set value of register \c ds
	void setDs(unsigned short value) { ds = value; }

	/// Set value of register \c fs
	void setFs(unsigned short value) { fs = value; }

	/// Set value of register \c gs
	void setGs(unsigned short value) { gs = value; }

	/// Set value of register \c eip
	void setEip(unsigned value) { eip = value; }

	/// Increment value of register \c eip by \a value bytes.
	void incEip(int value) { eip += value; }

	/// Decrement value of register \c eip by \a value bytes.
	void decEip(int value) { eip -= value; }

	/// Set value of register \c eflags
	void setEflags(unsigned value) { eflags = value; }

	/// Get the top of the FP stack
	int getFpuTop() const { return fpu_top; }

	/// Set the top of the FP stack
	void setFpuTop(int index) {
		assert(misc::inRange(index, 0, 7));
		fpu_top = index;
	}

	/// Increment the top of the FP stack in a circular manner
	void incFpuTop() { fpu_top = (fpu_top + 1) % 8; }

	/// Decrement the top of the FP stack in a circular manner
	void decFpuTop() { fpu_top = (fpu_top + 7) % 8; }

	/// Get field \a code of the FP status register (bits 3..0)
	int getFpuCode() const { return fpu_code; }

	/// Set field \a code of the FP status register
	void setFpuCode(int fpu_code) { this->fpu_code = fpu_code; }

	/// Get the FP control word (16 bits)
	unsigned short getFpuCtrl() const { return fpu_ctrl; }

	/// Set the FP control word (16 bits)
	void setFpuCtrl(unsigned short fpu_ctrl) { this->fpu_ctrl = fpu_ctrl; }

	/// Return whether a position in the FP stack is valid
	bool isFpuValid(int index) {
		assert(misc::inRange(index, 0, 7));
		return fpu_stack[index].valid;
	}

	/// Set the validity of the FP stack at position \a index
	void setFpuValid(int index, bool valid = true) {
		assert(misc::inRange(index, 0, 7));
		fpu_stack[index].valid = valid;
	}

	/// Return the extended value at FP stack position \a index
	unsigned char *getFpuValue(int index) {
		assert(misc::inRange(index, 0, 7));
		return fpu_stack[index].value.getValue();
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

