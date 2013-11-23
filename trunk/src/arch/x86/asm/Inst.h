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

#ifndef ARCH_X86_ASM_INST_H
#define ARCH_X86_ASM_INST_H

#include <iostream>

#include <lib/cpp/String.h>


namespace x86
{

class Asm;



/// List of unique x86 instruction identifiers
enum InstOpcode
{
	InstOpcodeInvalid = 0,

#define DEFINST(name, op1, op2, op3, modrm, imm, prefixes) \
	INST_##name,
#include "Asm.dat"
#undef DEFINST

	// Max
	InstOpcodeCount
};


/// x86 instruction prefixes
enum InstPrefix
{
	InstPrefixNone = 0x00,
	InstPrefixRep = 0x01,
	InstPrefixRepz = 0x01,  // same value as rep
	InstPrefixRepnz = 0x02,
	InstPrefixLock = 0x04,
	InstPrefixAddr = 0x08,  // address-size override
	InstPrefixOp = 0x10  // operand-size override
};


// Registers
extern misc::StringMap inst_reg_map;
enum InstReg
{
	InstRegNone = 0,

	InstRegEax,
	InstRegEcx,
	InstRegEdx,
	InstRegEbx,
	InstRegEsp,
	InstRegEbp,
	InstRegEsi,
	InstRegEdi,

	InstRegAx,
	InstRegCx,
	InstRegDx,
	InstRegBx,
	InstRegSp,
	InstRegBp,
	InstRegSi,
	InstRegDi,

	InstRegAl,
	InstRegCl,
	InstRegDl,
	InstRegBl,
	InstRegAh,
	InstRegCh,
	InstRegDh,
	InstRegBh,

	InstRegEs,
	InstRegCs,
	InstRegSs,
	InstRegDs,
	InstRegFs,
	InstRegGs,

	InstRegCount
};


/// x86 flags in register \c eflags
enum InstFlag
{
	InstFlagCF = 0,
	InstFlagPF = 2,
	InstFlagAF = 4,
	InstFlagZF = 6,
	InstFlagSF = 7,
	InstFlagDF = 10,
	InstFlagOF = 11
};


/// This structure represents information for one x86 instruction
struct InstInfo
{
	InstOpcode opcode;

	unsigned int op1;
	unsigned int op2;
	unsigned int op3;
	unsigned int modrm;
	unsigned int imm;

	// Mask of prefixes of type InstPrefix
	int prefixes;

	// Format string
	const char *fmt;

	// Derived fields
	unsigned int match_mask;
	unsigned int match_result;

	unsigned int nomatch_mask;
	unsigned int nomatch_result;

	int opindex_shift;  // Pos to shift inst to obtain index of op1/op2 if any
	int impl_reg;  // Implied register in op1 (0-7)
	int opcode_size;  // Size of opcode (1 or 2), not counting the modrm part.
	int modrm_size;  // Size of modrm field (0 or 1)
	int imm_size;  // Immediate size (0, 1, 2, or 4)
};


// Containers for instruction infos. We need this because an info can belong to
// different lists when there are registers embedded in the opcodes.
struct InstDecodeInfo
{
	InstInfo *info;
	InstDecodeInfo *next;
};


/// x86 instruction
class Inst
{
	// X86 assembler used to decode instruction
	const Asm *as;

	// Flag indicating if instruction has been decoded
	bool decoded;

	unsigned int eip;  // position inside the code
	int size;  // number of instruction bytes
	InstOpcode opcode;
	const char *format;  // format of the instruction
	
	// Size of fields
	int prefix_size;
	int opcode_size;
	int modrm_size;
	int sib_size;
	int disp_size;
	int imm_size;

	// Index in the opcode. This is a value between 0 and 7, which
	// is present in some instructions at op1 or op2.
	int opindex;

	// Prefixes
	InstReg segment;  // Reg. used to override segment
	int prefixes;  // Mask of prefixes of type 'X86InstPrefix'
	int op_size;  // Operand size: 2 or 4, default 4
	int addr_size;  // Address size: 2 or 4, default 4
	
	// ModR/M Field
	unsigned char modrm;  // ModR/M field
	unsigned char modrm_mod;  // mod field of ModR/M
	unsigned char modrm_reg;  // reg field of ModR/M
	unsigned char modrm_rm;  // rm field of ModR/M

	// SIB Field
	unsigned char sib;  // SIB field
	unsigned char sib_scale;  // Scale field of SIB
	unsigned char sib_index;  // Index field of SIB
	unsigned char sib_base;  // Base field of SIB

	// Displacement and Immediate
	int disp;
	union {
		unsigned char b;
		unsigned short w;
		unsigned int d;
	} imm;

	// Effective address
	InstReg ea_base;
	InstReg ea_index;
	unsigned int ea_scale;

	// Register, same as modrm_reg
	int reg;

	// Dump functions
	void DumpMoffsAddr(std::ostream &os) const;
	void DumpAddr(std::ostream &os) const;

	// Clear content of instruction
	void Clear();

public:
	/// Constructor
	Inst(const Asm *as) : as(as) {
		Clear();
	}

	/// Read the bytes in the beginning of \a buffer and decode the x86
	/// instruction represented by them. The value given in \a address
	/// specifies the virtual address of the instruction, stored internally
	/// and used later for branch decoding purposes.
	void Decode(const char *buffer, unsigned eip);

	/// Return the instruction opcode, or \c InstOpcodeInvalid if the
	/// sequence of bytes failed to decode. This function must be invoked
	/// after a previous call to Decode().
	InstOpcode getOpcode() const {
		assert(decoded);
		return opcode;
	}

	/// Return the number of bytes of this instruction, or 0 if the sequence
	/// of bytes failed to decode. This function must be invoked after a
	/// previous call to Decode().
	int getSize() const {
		assert(decoded);
		return size;
	}

	/// Dump instruction into output stream
	void Dump(std::ostream &os = std::cout) const;

	/// Dump instruction
	friend std::ostream &operator<<(std::ostream &os, const Inst &inst) {
		inst.Dump(os);
		return os;
	}
};


}  // namespace x86

#endif

