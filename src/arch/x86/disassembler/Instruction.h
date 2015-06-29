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

#ifndef ARCH_X86_DISASSEMBLER_INST_H
#define ARCH_X86_DISASSEMBLER_INST_H

#include <iostream>

#include <lib/cpp/String.h>


namespace x86
{

class Disassembler;


/// x86 instruction
class Instruction
{
public:

	/// List of unique x86 instruction identifiers
	enum Opcode
	{
		OpcodeInvalid = 0,

#define DEFINST(name, op1, op2, op3, modrm, imm, prefixes) \
	Opcode_##name,
#include "Instruction.def"
#undef DEFINST

		// Max
		OpcodeCount
	};


	/// x86 instruction prefixes
	enum Prefix
	{
		PrefixNone = 0x00,
		PrefixRep = 0x01,
		PrefixRepz = 0x01,  // same value as rep
		PrefixRepnz = 0x02,
		PrefixLock = 0x04,
		PrefixAddr = 0x08,  // address-size override
		PrefixOp = 0x10  // operand-size override
	};


	// Registers
	enum Reg
	{
		RegNone = 0,

		RegEax,
		RegEcx,
		RegEdx,
		RegEbx,
		RegEsp,
		RegEbp,
		RegEsi,
		RegEdi,

		RegAx,
		RegCx,
		RegDx,
		RegBx,
		RegSp,
		RegBp,
		RegSi,
		RegDi,

		RegAl,
		RegCl,
		RegDl,
		RegBl,
		RegAh,
		RegCh,
		RegDh,
		RegBh,

		RegEs,
		RegCs,
		RegSs,
		RegDs,
		RegFs,
		RegGs,

		RegCount
	};

	/// String map used for values of type Register
	static const misc::StringMap reg_map;

	/// x86 flags in register \c eflags
	enum Flag
	{
		FlagCF = 0,
		FlagPF = 2,
		FlagAF = 4,
		FlagZF = 6,
		FlagSF = 7,
		FlagDF = 10,
		FlagOF = 11
	};

	/// This structure represents information for one x86 instruction
	struct Info
	{
		Opcode opcode;

		unsigned int op1;
		unsigned int op2;
		unsigned int op3;
		unsigned int modrm;
		unsigned int imm;

		// Mask of prefixes of type Prefix
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
	struct DecodeInfo
	{
		Info *info;
		DecodeInfo *next;
	};


private:

	// X86 assembler used to decode instruction
	const Disassembler *disassembler;

	// Flag indicating if instruction has been decoded
	bool decoded;

	unsigned int eip;  // position inside the code
	int size;  // number of instruction bytes
	Opcode opcode;
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
	Reg segment;  // Reg. used to override segment
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
	Reg ea_base;
	Reg ea_index;
	unsigned int ea_scale;

	// Register, same as modrm_reg
	int reg;

	// Data structure for 'modrm_table'
	struct ModRMTableEntry
	{
		Instruction::Reg ea_base;
		int disp_size;
		int sib_size;
	};

	// Table indexed by pairs ModRM.mod and ModRM.rm, containing
	// information about what will come next and effective address
	// computation.
	static const ModRMTableEntry modrm_table[32];

	// Dump functions
	void DumpMoffsAddr(std::ostream &os) const;
	void DumpAddr(std::ostream &os) const;

	// Clear content of instruction
	void Clear();

public:

	/// Constructor
	Instruction();

	/// Read the bytes in the beginning of \a buffer and decode the x86
	/// instruction represented by them. The value given in \a address
	/// specifies the virtual address of the instruction, stored internally
	/// and used later for branch decoding purposes.
	void Decode(const char *buffer, unsigned eip);

	/// Return the instruction opcode, or \c OpcodeInvalid if the
	/// sequence of bytes failed to decode. This function must be invoked
	/// after a previous call to Decode().
	Opcode getOpcode() const
	{
		assert(decoded);
		return opcode;
	}

	/// Return the number of bytes of this instruction, or 0 if the sequence
	/// of bytes failed to decode. This function must be invoked after a
	/// previous call to Decode().
	int getSize() const
	{
		assert(decoded);
		return size;
	}

	/// Return instruction address
	unsigned getEip() const { return eip; }

	/// Dump instruction into output stream
	void Dump(std::ostream &os = std::cout) const;

	/// Dump instruction
	friend std::ostream &operator<<(std::ostream &os, const Instruction &inst)
	{
		inst.Dump(os);
		return os;
	}

	/// Return the opcode index (value between 0 and 7)
	int getOpIndex() const { return opindex; }

	/// Return segment register
	Reg getSegment() const { return segment; }

	/// Return the base register for the effective address computation
	Reg getEaBase() const { return ea_base; }

	/// Return the index register for the effective address computation
	Reg getEaIndex() const { return ea_index; }

	/// Return the scale factor for the effective address computation
	unsigned getEaScale() const { return ea_scale; }

	/// Return the decoded \i ModR/M field
	unsigned char getModRm() const { return modrm; }

	/// Return field \a mod extracted from the decoded \i ModR/M field
	unsigned char getModRmMod() const { return modrm_mod; }

	/// Return field \a reg extracted from the decoded \i ModR/M field
	unsigned char getModRmReg() const { return modrm_reg; }

	/// Return field \a rm extracted from the decoded \i ModR/M field
	unsigned char getModRmRm() const { return modrm_rm; }
	
	/// Return field \a SIB from the decoded instruction
	unsigned char getSib() const { return sib; }

	/// Return sub-field \a scale extracted from field \a SIB
	unsigned char getSibScale() const { return sib_scale; }

	/// Return sub-field \a index extracted from field \a SIB
	unsigned char getSibIndex() const { return sib_index; }

	/// Return sub-field \a base extracted from field \a SIB
	unsigned char getSibBase() const { return sib_base; }
	
	/// Return decoded displacement field
	int getDisp() const { return disp; }

	/// Return immediate value interpreted as a byte
	unsigned char getImmByte() const { return imm.b; }

	/// Return immediate value interpreted as a word (2 bytes)
	unsigned short getImmWord() const { return imm.w; }

	/// Return immediate value interpreted as a double word (4 bytes)
	unsigned getImmDWord() const { return imm.d; }
};


}  // namespace x86

#endif

