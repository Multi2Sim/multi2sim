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

#ifndef FERMI_ASM_INST_H
#define FERMI_ASM_INST_H

#include <iostream>

#include <lib/cpp/String.h>


namespace Fermi
{


// Forward declarations
class Asm;


struct FmtReg
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int src2 : 20; // 45:26
	unsigned long long int s2mod : 2; // 47:46
	unsigned long long int dmod : 1; // 48
	unsigned long long int fmod1_srco : 10; // 58:49
	unsigned long long int func : 5; // 63:59
};

struct FmtImm
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int imm32 : 32; // 57:26
	unsigned long long int dmod : 1; // 58
	unsigned long long int func : 5; // 63:59
};

struct FmtOther
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int fmod1_src1 : 6; // 25:20
	unsigned long long int src2 : 20; // 45:26
	unsigned long long int s2mod : 2; // 47:46
	unsigned long long int dmod : 1; // 48
	unsigned long long int fmod2_srco : 9; // 57:49
	unsigned long long int func : 6; // 63:58
};

struct FmtLdSt
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int fmod1_srco : 32; // 57:26
	unsigned long long int func : 6; // 63:58
};

struct FmtCtrl
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int mmod : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int imm32 : 32; // 57:26
	unsigned long long int x : 1; // 58
	unsigned long long int func : 5; // 63:59
};

// Instruction categories
typedef enum
{
	InstCategorySpFp,  // Single-precision floating point
	InstCategoryDpFp,  // Double-precision floating point
	InstCategoryImm,  // Immediate
	InstCategoryInt,  // Integer
	InstCategoryOther,  // Other
	InstCategoryLdSt,  // Load/Store from/to read/write memory
	InstCategoryLdRO,  // Load from read-only memory
	InstCategoryCtrl,  // Control

	InstCategoryCount
} InstCategory;

// Instruction ID
typedef enum
{
	InstIdInvalid = 0,

#define DEFINST(_op, _fmt_str, _opcode) \
		InstId##_op,
#include "asm.dat"
#undef DEFINST

	InstIdCount
} InstId;

// Instruction operations
typedef enum
{
#define DEFINST(_op, _fmt_str, _opcode) \
		InstOp##_op = _opcode,
#include "asm.dat"
#undef DEFINST
} InstOp;

typedef union
{
	unsigned char bytes[8];
	unsigned int word[2];
	unsigned long long int dword;

	FmtReg fmt_reg;
	FmtImm fmt_imm;
	FmtOther fmt_other;
	FmtLdSt fmt_ldst;
	FmtCtrl fmt_ctrl;
} InstBytes;

struct InstInfo
{
	InstId id;
	InstOp op;
	InstCategory category;
	std::string name;
	std::string fmt_str;
};

class Inst
{
	// Disassembler
	Asm *as;

	// Instruction bytes
	InstBytes bytes;

	// Decoded information
	InstInfo *info;

	// Virtual address
	unsigned addr;

	// Assembly
	std::string str;

public:
	/// Constructor
	Inst();

	/// Dump functions
	void DumpPC(std::ostream &os);
	void DumpToBufWithFmtReg(void);
	void DumpToBufWithFmtImm(void);
	void DumpToBufWithFmtOther(void);
	void DumpToBufWithFmtLdSt(void);
	void DumpToBufWithFmtCtrl(void);
	void DumpToBuf(void);
	void Dump(std::ostream &os, unsigned width);
	void DumpHex(std::ostream &os);

	/// Decode instruction pointed by ptr. Set its virtual address to addr.
	void Decode(unsigned addr, const char *ptr);

	// Getters
	InstBytes *getBytes() { return &bytes; }
	InstInfo *getInfo() { return info; }
	std::string getStr() { return str; }

	/// Get ID
	InstId getId() { return info ? info->id : InstIdInvalid; }

	/// Get operation
	InstOp getOp() { assert(info); return info->op; }

	/// Get opcode
	unsigned getOpcode() { return info ? (unsigned) info->op : 0; }

	/// Get category
	InstCategory getCategory() { assert(info); return info->category; }

	/// Get name
	std::string getName() { assert(info); return info->name; }
};


}  // namespace Fermi

#endif
