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

extern misc::StringMap inst_ftzfmz_map;
extern misc::StringMap inst_rnd_map;
extern misc::StringMap inst_rnd1_map;
extern misc::StringMap inst_cmp_map;
extern misc::StringMap inst_round_map;
extern misc::StringMap inst_sat_map;
extern misc::StringMap inst_logic_map;
extern misc::StringMap inst_op_map;
extern misc::StringMap inst_op56_map;
extern misc::StringMap inst_op67_map;
extern misc::StringMap inst_dtype_n_map;
extern misc::StringMap inst_dtype_map;
extern misc::StringMap inst_stype_n_map;
extern misc::StringMap inst_stype_map;
extern misc::StringMap inst_type_map;
extern misc::StringMap inst_cop_map;
extern misc::StringMap inst_NOP_op_map;
extern misc::StringMap inst_ccop_map;
extern misc::StringMap inst_sreg_map;


// Forward declarations
class Asm;

// Special Registers
//enum InstSReg
//{
//	InstSRegLaneld = 0,
//	InstSRegVirtCfg = 2,
//	InstSRegVirtId,
//	InstSRegPM0,
//	InstSRegPM1,
//	InstSRegPM2,
//	InstSRegPM3,
//	InstSRegPM4,
//	InstSRegPM5,
//	InstSRegPM6,
//	InstSRegPM7,
//	InstSRegPrimType = 16,
//	InstSRegInvocationID,
//	InstSRegYDirection,
//	InstSRegMachineID0 = 24,
//	InstSRegMachineID1,
//	InstSRegMachineID2,
//	InstSRegMachineID3,
//	InstSRegAffinity,
//	InstSRegTid = 32,
//	InstSRegTidX,
//	InstSRegTidY,
//	InstSRegTidZ,
//	InstSRegCTAParam,
//	InstSRegCTAidX,
//	InstSRegCTAidY,
//	InstSRegCTAidZ,
//	InstSRegNTid,
//	InstSRegNTidX,
//	InstSRegNTidY,
//	InstSRegNTidZ,
//	InstSRegGridParam,
//	InstSRegNCTAidX,
//	InstSRegNCTAidY,
//	InstSRegNCTAidZ,
//	InstSRegSWinLo,
//	InstSRegSWINSZ,
//	InstSRegSMemSz,
//	InstSRegSMemBanks,
//	InstSRegLWinLo,
//	InstSRegLWINSZ,
//	InstSRegLMemLoSz,
//	InstSRegLMemHiOff,
//	InstSRegEqMask,
//	InstSRegLtMask,
//	InstSRegLeMask,
//	InstSRegGtMask,
//	InstSRegGeMask,
//	InstSRegClockLo = 80,
//	InstSRegClockHi,
//
//	// Max
//	InstSRegCount
//};


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


// 1st level struct

struct InstBytesGeneral0
{
	unsigned long long int op0 : 4; // 3:0
	unsigned long long int mod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int src2 : 20; // 45:26
	unsigned long long int src2_mod : 2; // 47:46
	unsigned long long int dst_cc : 1; // 48
	unsigned long long int mod1 : 9; // 57:49
	unsigned long long int op1 : 6; // 63:58
};


struct InstBytesGeneral1
{
	unsigned long long int op0 : 4; // 3:0
	unsigned long long int mod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int src2 : 20; // 45:26
	unsigned long long int src2_mod : 2; // 47:46
	unsigned long long int dst_cc : 1; // 48
	unsigned long long int R : 3; // 51:49
	unsigned long long int _reserved0: 1; // 52
	unsigned long long int logic : 2; // 54:53
	unsigned long long int cmp : 4; // 58:55
	unsigned long long int op1 : 5; // 63:59
};


struct InstBytesImm
{
	unsigned long long int op0 : 4; // 3:0
	unsigned long long int mod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int imm32 : 32; // 57:26
	unsigned long long int op1 : 6; // 63:58
};


struct InstBytesOffs
{
	unsigned long long int op0 : 4; // 3:0
	unsigned long long int mod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int offset : 16; // 41:26
	unsigned long long int mod1 : 16; // 57:42
	unsigned long long int op1 : 6; // 63:58
};


struct InstBytesTgt
{
	unsigned long long int op0 : 4; // 3:0
	unsigned long long int mod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int tgt_mod : 1; // 14
	unsigned long long int u : 1; // 15
	unsigned long long int noinc : 1; // 16
	unsigned long long int _const0 : 9; // 25:17
	unsigned long long int target : 20; // 45:26
	unsigned long long int _reserved0 : 12; // 57:46
	unsigned long long int op1 : 6; // 63:58
};

// 2nd level struct

struct InstBytesGeneral0Mod1A
{
	unsigned long long int _reserved0 : 49; // 48:0
	unsigned long long int R : 4; // 52:49
	unsigned long long int  _const0: 5; // 57:53
	unsigned long long int _reserved1 : 6; // 63:58
};


struct InstBytesGeneral0Mod1B
{
	unsigned long long int _reserved0 : 49; // 48:0
	unsigned long long int src3 : 6; // 54:49
	unsigned long long int rnd: 2; // 56:55
	unsigned long long int _const0 : 1; // 57
	unsigned long long int _reserved1 : 6; // 63:58
};


struct InstBytesGeneral0Mod1C
{
	unsigned long long int _reserved0 : 49; // 48:0
	unsigned long long int rnd : 2; // 50:49
	unsigned long long int word0 : 7; // 57:51
	unsigned long long int _reserved1 : 6; // 63:58
};


struct InstBytesGeneral0Mod1D
{
	unsigned long long int _reserved0 : 49; // 48:0
	unsigned long long int src3 : 6; // 54:49
	unsigned long long int cmp : 3; // 57:55
	unsigned long long int _reserved1 : 6; // 63:58
};


struct InstBytesOffsMod1A
{
	unsigned long long int _reserved0 : 42; // 41:0
	unsigned long long int _const0 : 8; // 49:42
	unsigned long long int trig : 1; // 50
	unsigned long long int op : 4; // 54:51
	unsigned long long int _const1 : 3; // 57:55
	unsigned long long int _reserved1 : 6; // 63:58
};


struct InstBytesOffsMod1B
{
	unsigned long long int _reserved0 : 42; // 41:0
	unsigned long long int _const0 : 8; // 49:42
	unsigned long long int R : 3; // 52:50
	unsigned long long int _reserved1 : 5; // 57:53
	unsigned long long int _reserved2 : 6; // 63:58
};


struct InstBytesMod0A
{
	unsigned long long int _reserved0 : 4; // 3:0
	unsigned long long int s : 1; // 4
	unsigned long long int satftz : 1; // 5
	unsigned long long int abs_src2 : 1; // 6
	unsigned long long int abs_src1 : 1; // 7
	unsigned long long int neg_src2 : 1; // 8
	unsigned long long int neg_src1 : 1; // 9
	unsigned long long int _reserved1 : 54; // 63:10
};


struct InstBytesMod0B
{
	unsigned long long int _reserved0 : 4; // 3:0
	unsigned long long int s : 1; // 4
	unsigned long long int type : 3; // 7:5
	unsigned long long int cop : 2; // 9:8
	unsigned long long int _reserved1 : 54; // 63:10
};


struct InstBytesMod0C
{
	unsigned long long int _reserved0 : 4; // 3:0
	unsigned long long int s : 1; // 4
	unsigned long long int shamt : 5; // 9:5
	unsigned long long int _reserved1 : 54; // 63:10
};


struct InstBytesMod0D
{
	unsigned long long int _reserved0 : 4; // 3:0
	unsigned long long int s : 1; // 4
	unsigned long long int sat : 1; // 5
	unsigned long long int ftzfmz : 2; // 7:6
	unsigned long long int fma_mod : 2; // 9:8
	unsigned long long int _reserved1 : 54; // 63:10
};


enum InstCategory
{
	InstCategorySpFp,  // Single-precision floating point
	InstCategoryDpFp,  // Double-precision floating point
	InstCategoryImm,  // Immediate
	InstCategoryInt,  // Integer
	InstCategoryOther,  // Conversion/Movement/Predicate/CC/Miscellaneous
	InstCategoryLdStRW,  // Load/Store read/write
	InstCategoryLdStRO, // Load/Store read-only
	InstCategoryCtrl // Control
};


enum InstOpcode
{
	InstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _opcode) \
	INST_##_name,
#include "asm.dat"
#undef DEFINST

	// Max
	InstOpcodeCount
};


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
	InstOpcode opcode;
	InstCategory category;

	std::string name;
	std::string fmt_str;

	std::string str;

	/* Field of the instruction containing the instruction opcode
	 * identifier. */
	unsigned int op;

	int size;

	unsigned int cat;
	unsigned int func;
};


class Inst
{
	// Disassembler
	Asm *as;

	// Instruction virtual address, for branch decoding purposes
	unsigned int addr;

	// Instruction bytes
	InstBytes bytes;

	// Decoded information
	InstInfo *info;

	std::string str;

public:

	// Constructor
	Inst(Asm *as);

	// Dump functions
	void DumpPC(std::ostream &os);
	void DumpToBufWithFmtReg(void);
	void DumpToBufWithFmtImm(void);
	void DumpToBufWithFmtOther(void);
	void DumpToBufWithFmtLdSt(void);
	void DumpToBufWithFmtCtrl(void);
	void DumpToBuf(void);
	void Dump(std::ostream &os, unsigned int max_inst_len);
	void DumpHex(std::ostream &os);

	/* From the buffer given in 'ptr', decode an instruction and populate fields
	 * 'bytes' and 'info' of the instruction object. Argument 'addr' passes the
	 * virtual address of the instruction for branch decoding purposes, and it is
	 * copied to the 'addr' field of the instruction. */
	void Decode(unsigned int addr, const char *ptr);

	// Getters
	std::string GetName() { return info ? info->name : "<unknown>"; }
	InstOpcode GetOpcode() { return info ? info->opcode : InstOpcodeInvalid; }
	InstCategory GetCategory() { assert(info); return info->category; }
	InstBytes *GetBytes() { return &bytes; }
	std::string GetString() { return str; }
};



}  // namespace Fermi

#endif
