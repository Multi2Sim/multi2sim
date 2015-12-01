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

#ifndef ARCH_KEPLER_DISASSEMBLER_INST_H
#define ARCH_KEPLER_DISASSEMBLER_INST_H

#include <iostream>

namespace Kepler
{

// Forward declarations
class Disassembler;


class Instruction
{
public:

	/// Unique instruction opcodes
	enum Opcode
	{
		OpcodeInvalid = 0,

	#define DEFINST(_name, _fmt_str, ...)        \
		INST_##_name,
	#include "Instruction.def"
	#undef DEFINST

		// Max
		OpcodeCount
	};

	/// Opcode information
	struct Info
	{
		Opcode opcode;
		const char *name;
		const char *fmt_str;
	};

	/// Decode information
	struct DecodeInfo
	{
		// Fields used when the table entry points to another table.
		int next_table_low;
		int next_table_high;
		DecodeInfo *next_table;

		// Field used when the table entry points to a final
		// instruction
		Info *info;

		// Constructor that initializes all fields to 0
		DecodeInfo();
	};

	/// 1st level struct
	/// BFE, POPC, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD, LDS,
	/// STS FMNMX, FMUL, DADD, SEL, P2R, RRO, MOV, F2F, F2I, I2F, I2I, FLO,
	/// DCHK, FCHK S2R, IMAD, ISETP, ISCADD
	struct BytesGeneral0
	{
		unsigned long long int op0 	: 2; 	// 1:0
		unsigned long long int dst 	: 8; 	// 9:2
		unsigned long long int mod0 	: 8; 	// 17:10
		unsigned long long int pred 	: 4; 	// 21:18
		unsigned long long int s 	: 1; 	// 22
		unsigned long long int srcB 	: 19; 	// 41:23
		unsigned long long int mod1 	: 12;	// 53:42
		unsigned long long int op1 	: 9; 	// 62:54
		unsigned long long int srcB_mod	: 1; 	// 63
	};

	/// BRA, JMX, JMP, JCAL, BRX, CAL, PRET, PLONGJMP, SSY, PBK
	struct BytesGeneral1
	{
		unsigned long long int op0 	: 2; 	// 1:0
		unsigned long long int mod0 	: 16;	// 17:2
		unsigned long long int pred 	: 4; 	// 21:18
		unsigned long long int unused 	: 1; 	// 22
		unsigned long long int srcB 	: 21; 	// 43:23
		unsigned long long int mod1 	: 11;	// 54:44
		unsigned long long int op1 	: 9; 	// 63:55
	};

	/// GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETLMEMBASE, EXIT, LONGJUMP, RET,
	/// KIL,BRK, CONT, RTT, SAM, RAM
	struct BytesGeneral2
	{
		unsigned long long int op0     	: 2; 	// 1:0
		unsigned long long int mod     	: 8; 	// 9:2
		unsigned long long int src     	: 8; 	// 17:10
		unsigned long long int pred    	: 4; 	// 21:18
		unsigned long long int unused   : 33; 	// 54:22
		unsigned long long int op1     	: 9; 	// 63:55
	};

	/// MOV32I, FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I
	struct BytesImm
	{
		unsigned long long int op0      : 2; 	// 1:0
		unsigned long long int dst      : 8; 	// 9:2
		unsigned long long int mod0     : 8; 	// 17:10
		unsigned long long int pred     : 4; 	// 21:18
		unsigned long long int s        : 1; 	// 22
		unsigned long long int imm32    :32; 	// 54:23
		unsigned long long int mod1    	: 6; 	// 60:55
		unsigned long long int op1 		: 3; 	// 63:61
	};

	/// 2nd level struct
	/// BFE, POPC, IADD, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD,
	/// FMNMX, FMUL, DADD, SEL, P2R
	struct BytesGeneral0Mod0A
	{
		unsigned long long int reserved0: 10; 	// 9:0
		unsigned long long int srcA 	: 8; 	// 17:10
		unsigned long long int reserved1: 46;	// 63:18
	};


	/// RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK
	struct BytesGeneral0Mod0B
	{
		unsigned long long int reserved0: 10;   // 9:0
		unsigned long long int mod2     : 8;    // 17:10
		unsigned long long int reserved1: 46;   // 63:18
	};

	/// JCAL, CAL, PRET
	struct BytesGeneral1Mod0A
	{
		unsigned long long int reserved0: 2; 	// 1:0
		unsigned long long int unused0	: 5; 	// 6:2
		unsigned long long int srcB_mod : 1; 	// 7
		unsigned long long int noinc 	: 1; 	// 8
		unsigned long long int unused1	: 9; 	// 17:9
		unsigned long long int reserved1: 46; 	// 63:18
	};

	/// BRA, JMX, JMP, BRX
	struct BytesGeneral1Mod0B
	{
		unsigned long long int reserved0: 2;    // 1:0
		unsigned long long int cc   	: 5;    // 6:2
		unsigned long long int srcB_mod : 1;    // 7
		unsigned long long int lmt    	: 1;    // 8
		unsigned long long int u	: 1;	// 9
		unsigned long long int srcB   	: 8;    // 17:10
		unsigned long long int reserved1: 46;   // 63:18
	};

	/// BRA, BRX, CAL, PRET
	struct BytesGeneral1Mod1A
	{
		unsigned long long int reserved0: 44;  	// 43:0
		unsigned long long int srcB     : 2;    // 45:44
		unsigned long long int neg_srcB	: 1;    // 46
		unsigned long long int unused  	: 8;    // 54:47
		unsigned long long int reserved1: 9;   	// 63:55
	};

	/// JMP, JCAL
	struct BytesGeneral1Mod1B
	{
		unsigned long long int reserved0: 44;   // 43:0
		unsigned long long int srcB   	: 11;    // 54:44
		unsigned long long int reserved1: 9;    // 63:55
	};

	/// JMX
	struct BytesGeneral1Mod1C
	{
		unsigned long long int reserved0: 44;  	// 43:0
		unsigned long long int srcB     : 10;  	// 53:44
		unsigned long long int neg_srcB	: 1;	// 54
		unsigned long long int reserved1: 9;   	// 63:55
	};

	/// GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETMEMLBASE
	struct BytesGeneral2ModA
	{
		unsigned long long int reserved0: 2;   	// 41:0
		unsigned long long int dst     	: 8;    // 9:2
		unsigned long long int reserved1: 54;  	// 63:10
	};

	/// EXIT, LONGJUMP, RET, KIL, BRK, CONT, RTT, SAM, RAM
	struct BytesGeneral2ModB
	{
		unsigned long long int reserved0: 2;  	// 41:0
		unsigned long long int cc  	: 8;    // 6:2
		unsigned long long int unused	: 3;	// 9:7
		unsigned long long int reserved1: 54;  	// 63:10
	};

	/// Need to figure out how to re-label constant once I know what this field represents
	/// MOV32I
	struct BytesImmMod0A
	{
		unsigned long long int reserved0: 10;  	// 9:0
		unsigned long long int unused  	: 4;    // 13:10
		unsigned long long int constant	: 4;	// 17:14 */     /* ??????
		unsigned long long int reserved1: 46;  	// 63:18
	};

	/// FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I
	struct BytesImmMod0B
	{
		unsigned long long int reserved0: 10;  	// 9:0
		unsigned long long int src	: 8;	// 17:10
		unsigned long long int reserved1: 46;  	// 63:18
	};

	/// MOV32I
	struct BytesImmMod1A
	{
		unsigned long long int reserved0: 55;	// 54:0
		unsigned long long int op1	: 6;	// 60:55
		unsigned long long int reserved1: 3;	// 63:61
	};

	/// FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I
	struct BytesImmMod1B
	{
		unsigned long long int reserved0: 55;	// 54:0
		unsigned long long int cc	: 1;	// 55
		unsigned long long int mod2	: 5;	// 60:56
		unsigned long long int reserved1: 3;	// 63:61
	};

	/// FMUL321, IMUL32I
	struct BytesImmMod1C
	{
		unsigned long long int reserved0: 55;	// 54:0
		unsigned long long int cc	: 1;	// 55
		unsigned long long int mod2	: 3;	// 58:56
		unsigned long long int op1	: 2;	// 60:59
		unsigned long long int reserved1: 3;	// 63:61
	};

	/// LDC
	struct BytesLDC
	{
		unsigned long long int op0		: 2;		//1:0
		unsigned long long int dst		: 8;		//9:2
		unsigned long long int src1     : 8;        //17:10
		unsigned long long int pred		: 4;		//21:18
		unsigned long long int s		: 1;		//22
		unsigned long long int src2_2  : 16;		//38:23
		unsigned long long int src2_1	: 5;		//43:39
		unsigned long long int unknown1 : 3;		//46:44
		unsigned long long int is		: 2;		//48:47
	 	unsigned long long int unknown2 : 2;		//50:49
		unsigned long long int u_or_s	: 3;		//53:51
		unsigned long long int op1		: 10;		//63:54
	};

	/// PSETP
	struct BytesPSETP
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int pred1:			3; // 4:2
		unsigned long long int pred0:			3; // 7:5
		unsigned long long int unknown5:		6; // 13:8
		unsigned long long int pred2:			4; // 17:14
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int unknown4:		4; // 26:23
		unsigned long long int bool_op1:		2; // 28:27
		unsigned long long int unknown3:		3; // 31:29
		unsigned long long int pred3:			4; // 35:32
		unsigned long long int unknown2:		6; // 41:36
		unsigned long long int pred4:			4; // 45:42
		unsigned long long int unknown1:		2; // 47:46
		unsigned long long int bool_op0:		2; // 49:48
		unsigned long long int unknown0:		4; // 53:50
		unsigned long long int op1:			10; // 63:54
	};

	/// BRA
	struct BytesBRA
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int cc:			5; // 6:2
		unsigned long long int src_mod:			1; // 7
		unsigned long long int lmt:			1; // 8
		unsigned long long int uni:			1; // 9
		unsigned long long int unknown2:		8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int unknown1:		1; // 22
		unsigned long long int offset:			24; // 46:23
		unsigned long long int unknown0:		8; // 54:47
		unsigned long long int op1:			9; // 63:55
	};

	/// SSY
	struct BytesSSY
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int unknown2:		5; // 6:2
		unsigned long long int isconstmem:		1; // 7
		unsigned long long int unknown1:		15; // 22:8
		unsigned long long int offset:			24; // 46:23
		unsigned long long int unknown0:		8; // 54:47
		unsigned long long int op1:			9; // 63:55
	};

	/// NOP
	struct BytesNOP
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int unknown2:		8; // 9:2
		unsigned long long int cc:			5; // 14:10
		unsigned long long int trig:			1; // 15
		unsigned long long int unknown1:		2; // 17:16
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int address:			16; // 38:23
		unsigned long long int unknown0:		15; // 53:39
		unsigned long long int op1:			10; // 63:54
	};

	/// SHL
	struct BytesSHL
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int mode:			1; // 42
		unsigned long long int unknown2:		3; // 45:43
		unsigned long long int x:			1; // 46
		unsigned long long int unknown1:		3; // 49:47
		unsigned long long int cc:			1; // 50
		unsigned long long int unknown0:		3; // 53:51
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// PBK
	struct BytesPBK
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int unknown0:		5; // 6:2
		unsigned long long int constant:		1; // 7
		unsigned long long int unknown1:		15; // 22:8
		unsigned long long int offset:			24; // 46:23
		unsigned long long int unknown2:		8; // 54:47
		unsigned long long int op1:			9; // 63:55
	};

	/// PCNT
	/// constant unverified
	struct BytesPCNT
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int unknown0:		5; // 6:2
		unsigned long long int constant:		1; // 7
		unsigned long long int unknown1:		15; // 22:8
		unsigned long long int offset:			24; // 46:23
		unsigned long long int unknown2:		8; // 54:47
		unsigned long long int op1:			9; // 63:55
	};

	/// BRK
	/// cc not verified
	struct BytesBRK
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int cc:			5; // 6:2
		unsigned long long int unknown0:		11; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int unknown1:		33; // 54:22
		unsigned long long int op1:			9; // 63:55
	};

	/// CONT
	/// cc not verified
	struct BytesCONT
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int cc:			5; // 6:2
		unsigned long long int unknown0:		11; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int unknown1:		33; // 54:22
		unsigned long long int op1:			9; // 63:55
	};

	/// CAL
	/// constant unverified
	struct BytesCAL
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int unknown0:		16; // 17:2
		unsigned long long int pred:			4; // 21:18
		unsigned long long int unknown1:		1; // 22:22
		unsigned long long int offset:			24; // 46:23
		unsigned long long int unknown2:		8; // 54:47
		unsigned long long int op1:			9; // 63:55
	};

	/// IADD
	struct BytesIADD
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int unknown0:		4; // 45:42
		unsigned long long int x:			1; // 46
		unsigned long long int unknown1:		3; // 49:47
		unsigned long long int cc:			1; // 50
		unsigned long long int po:			2; // 52:51
		unsigned long long int sat:			1; // 53
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// IADD32I
	struct BytesIADD32I
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int imm32:			32; // 54:23
		unsigned long long int cc:			1; // 55
		unsigned long long int x:			1; // 56
		unsigned long long int sat:			1; // 57
		unsigned long long int po:			2; // 59:58
		unsigned long long int op1:			4; // 63:60
	};

	/// IMUL
	struct BytesIMUL
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int hi:			1; // 42
		unsigned long long int u_s:			2; // 44:43
		unsigned long long int unknown1:		5; // 49:45
		unsigned long long int cc:			1; // 50
		unsigned long long int unknown2:		2; // 53:51
		unsigned long long int op1:			9; // 62:54
		unsigned long long int op2:			1; // 63
	};

	/// ISCADD
	struct BytesISCADD
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int shamt:			5; // 46:42
		unsigned long long int unused:			3; // 49:47
		unsigned long long int cc:			1; // 50
		unsigned long long int po:			2; // 52:51
		unsigned long long int unused1:			1; // 53
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// ISAD
	struct BytesISAD
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int src3:			8; // 49:42
		unsigned long long int cc:			1; // 50
		unsigned long long int u_s:			1; // 51
		unsigned long long int unused:			2; // 53:52
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// I2F
	struct BytesI2F
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int d_fmt:			2; // 11:10
		unsigned long long int s_fmt:			2; // 13:12
		unsigned long long int unused1:			1; // 14
		unsigned long long int src_signed:		1; // 15
		unsigned long long int unused2:			2; // 17:16
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src:			19; // 41:23
		unsigned long long int round:			2; // 43:42
		unsigned long long int extract:			2; // 45:44
		unsigned long long int unused3:			2; // 47:46
		unsigned long long int src_negate:		1; // 48
		unsigned long long int unused4:			1; // 49
		unsigned long long int cc:			1; // 50
		unsigned long long int unused5:			1; // 51
		unsigned long long int src_abs:			1; // 52
		unsigned long long int unused6:			1; // 53
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// I2I
	struct BytesI2I
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int d_fmt:			2; // 11:10
		unsigned long long int s_fmt:			2; // 13:12
		unsigned long long int dst_signed:		1; // 14
		unsigned long long int src_signed:		1; // 15
		unsigned long long int unused1:			2; // 17:16
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src:			19; // 41:23
		unsigned long long int unused2:			2; // 43:42
		unsigned long long int extract:			2; // 45:44
		unsigned long long int unused3:			2; // 47:46
		unsigned long long int src_negate:		1; // 48
		unsigned long long int unused4:			1; // 49
		unsigned long long int cc:			1; // 50
		unsigned long long int unused5:			1; // 51
		unsigned long long int src_abs:			1; // 52
		unsigned long long int sat:			1; // 53
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// F2I
	struct BytesF2I
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int d_fmt:			2; // 11:10
		unsigned long long int s_fmt:			2; // 13:12
		unsigned long long int dst_singed:		1; // 14
		unsigned long long int unused1:			3; // 17:15
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src:			19; // 41:23
		unsigned long long int round:			2; // 43:42
		unsigned long long int extract:			1; // 44
		unsigned long long int unused3:			2; // 46:45
		unsigned long long int ftz:			1; // 47
		unsigned long long int src_negate:		1; // 48
		unsigned long long int unused4:			1; // 49
		unsigned long long int cc:			1; // 50
		unsigned long long int unused5:			1; // 51
		unsigned long long int src_abs:			1; // 52
		unsigned long long int unused6:			1; // 53
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// F2F
	struct BytesF2F
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int d_fmt:			2; // 11:10
		unsigned long long int s_fmt:			2; // 13:12
		unsigned long long int unused1:			4; // 17:14
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src:			19; // 41:23
		unsigned long long int round:			2; // 43:42
		unsigned long long int extract:			1; // 44
		unsigned long long int pass:			1; // 45
		unsigned long long int unused2:			1; // 46
		unsigned long long int ftz:			1; // 47
		unsigned long long int src_negate:		1; // 48
		unsigned long long int unused3:			1; // 49
		unsigned long long int cc:			1; // 50
		unsigned long long int unused4:			1; // 51
		unsigned long long int src_abs:			1; // 52
		unsigned long long int sat:			1; // 53
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// MUFU
	struct BytesMUFU
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int mufu_op:			4; // 26:23
		unsigned long long int unused1:			22; // 48:27
		unsigned long long int src_abs:			1; // 49
		unsigned long long int unused2:			1; // 50
		unsigned long long int src_negate:		1; // 51
		unsigned long long int unused3:			1; // 52
		unsigned long long int sat:			1; // 53
		unsigned long long int op1:			5; // 58:54
		unsigned long long int op2:			5; // 63:59
	};

	/// SEL
	struct BytesSEL
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int pred_src:		4; // 45:42
		unsigned long long int unused1:			8; // 53:46
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// LOP
	struct BytesLOP
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int src1_negate:		1; // 42
		unsigned long long int src2_negate:		1; // 43
		unsigned long long int lop:			2; // 45:44
		unsigned long long int x:			1; // 46
		unsigned long long int unused1:			3; // 49:47
		unsigned long long int po:			1; // 50
		unsigned long long int unused2:			3; // 53:51
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// FFMA
	struct BytesFFMA
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int src3:			8; // 49:42
		unsigned long long int cc:			1; // 50
		unsigned long long int negate_ab:		1; // 51
		unsigned long long int negate_c:		1; // 52
		unsigned long long int sat:			1; // 53
		unsigned long long int round:			2; // 55:54
		unsigned long long int fmz:			2; // 57:56
		unsigned long long int op1:			4; // 61:58
		unsigned long long int op2:			2; // 63:62
	};

	/// SHR
	struct BytesSHR
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int shift_mode:		1; // 42
		unsigned long long int bit_reverse:		1; // 43
		unsigned long long int unused1:			2; // 45:44
		unsigned long long int x_precision:		2; // 47:46
		unsigned long long int unused2:			2; // 49:48
		unsigned long long int cc:			1; // 50
		unsigned long long int u_s:			1; // 51
		unsigned long long int unused3:			2; // 53:52
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// BFI
	struct BytesBFI
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s: 			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int src3:			8; // 49:42
		unsigned long long int cc:			1; // 50
		unsigned long long int unused:			3; // 53:51
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// BFE
	struct BytesBFE
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int unused1:			1; // 42
		unsigned long long int bit_reverse:		1; // 43
		unsigned long long int unused2:			6; // 49:44
		unsigned long long int cc:			1; // 50
		unsigned long long int u_s:			1; // 51
		unsigned long long int unused3:			2; // 53:52
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};

	/// LOP32I
	struct BytesLOP32I
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			32; // 54:23
		unsigned long long int cc:			1; // 55
		unsigned long long int lop:			2; // 57:56
		unsigned long long int src1_negate:		1; // 58
		unsigned long long int src2_negate:		1; // 59
		unsigned long long int x:			1; // 60
		unsigned long long int op1:			3; // 63:61
	};

	/// ISET
	struct BytesISET
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int pred_src:		4; // 45:42
		unsigned long long int x:			1; // 46
		unsigned long long int bval:			1; // 47
		unsigned long long int bop:			2; // 49:48
		unsigned long long int cc:			1; // 50
		unsigned long long int u_s:			1; // 51
		unsigned long long int comp:			3; // 54:52
		unsigned long long int op1:			7; // 61:55
		unsigned long long int op2:			2; // 63:62
	};

	/// FSET
	struct BytesFSET
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int pred_src:		4; // 45:42
		unsigned long long int src1_negate:		1; // 46
		unsigned long long int src2_abs:		1; // 47
		unsigned long long int bop:			2; // 49:48
		unsigned long long int cc:			1; // 50
		unsigned long long int comp:			4; // 54:51
		unsigned long long int bval:			1; // 55
		unsigned long long int src2_negate:		1; // 56
		unsigned long long int src1_abs:		1; // 57
		unsigned long long int ftz:			1; // 58
		unsigned long long int op1:			3; // 61:59
		unsigned long long int op2:			2; // 63:62
	};

	/// ICMP
	struct BytesICMP
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s: 			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int src3:			8; // 49:42
		unsigned long long int unused:			1; // 50
		unsigned long long int u_s:			1; // 51
		unsigned long long int comp:			3; // 54:52
		unsigned long long int op1:			7; // 61:55
		unsigned long long int op2:			2; // 63:62
	};

	/// FADD
	struct BytesFADD
	{
		unsigned long long int op0:			2; // 1:0
		unsigned long long int dst:			8; // 9:2
		unsigned long long int src1:			8; // 17:10
		unsigned long long int pred:			4; // 21:18
		unsigned long long int s:			1; // 22
		unsigned long long int src2:			19; // 41:23
		unsigned long long int round:			2; // 43:42
		unsigned long long int unused:			3; // 46:44
		unsigned long long int ftz:			1; // 47
		unsigned long long int src2_negate:		1; // 48
		unsigned long long int src1_abs:		1; // 49
		unsigned long long int cc:			1; // 50
		unsigned long long int src1_negate:		1; // 51
		unsigned long long int src2_abs:		1; // 52
		unsigned long long int sat:			1; // 53
		unsigned long long int op1:			8; // 61:54
		unsigned long long int op2:			2; // 63:62
	};
	/// Instruction bytes
	union Bytes
	{
		unsigned char as_uchar[8];
		unsigned int as_uint[2];
		unsigned long long as_dword;

		BytesGeneral0 general0;
		BytesGeneral1 general1;
		BytesGeneral2 general2;
		BytesImm      immediate;
		BytesLDC      ldc;
		BytesPSETP    psetp;
		BytesSSY      ssy;
		BytesBRA      bra;
		BytesNOP      nop;
		BytesSHL	  shl;
		BytesPBK	  pbk;
		BytesCONT	  cont;
		BytesBRK	  brk;
		BytesPCNT	  pcnt;
		BytesCAL      cal;
		BytesIADD     iadd;
		BytesIADD32I  iadd32i;
		BytesIMUL     imul;
		BytesISAD     isad;
		BytesISCADD   iscadd;
		BytesI2F      i2f;
		BytesI2I      i2i;
		BytesF2F	  f2f;
		BytesF2I	  f2i;
		BytesMUFU     mufu;
		BytesSEL      sel;
		BytesLOP      lop;
		BytesFFMA     ffma;
		BytesSHR      shr;
		BytesBFI      bfi;
		BytesBFE      bfe;
		BytesLOP32I	  lop32i;
		BytesISET     iset;
		BytesFSET     fset;
		BytesICMP     icmp;
		BytesFADD     fadd;
	};

private:

	// Disassembler
	const Disassembler *disassembler;

	// Instruction offset within .text section
	unsigned int address;

	// Instruction bytes
	Bytes bytes;

	// Decoded information
	Info *info;

	void DumpPredShort(std::ostream &os, int high, int low) const;
	void DumpPredNoat(std::ostream &os, int high, int low) const;
	void DumpPred(std::ostream &os, int high, int low) const;
	void DumpReg(std::ostream &os, int high, int low) const;
	void DumpSpecReg(std::ostream &os, int high, int low) const;
	void DumpS(std::ostream &os, int high, int low) const;
	void DumpF(std::ostream &os, int high, int low) const;
	void DumpAnd(std::ostream &os, int high, int low) const;
	void DumpU8(std::ostream &os, int high, int low) const;
	void DumpX(std::ostream &os, int high, int low) const;
	void DumpU32(std::ostream &os, int high, int low) const;
	void DumpHi(std::ostream &os, int high, int low) const;
	void DumpSat(std::ostream &os, int high, int low) const;
	void DumpPo(std::ostream &os, int high, int low) const;
	void DumpUs(std::ostream &os, int high0, int low0, int high1,
			int low1) const;
	void DumpCc(std::ostream &os, int high, int low) const;
	void DumpE(std::ostream &os, int high, int low) const;
	void DumpCv(std::ostream &os, int high, int low) const;
	void DumpLmt(std::ostream &os, int high, int low) const;
	void DumpU(std::ostream &os, int high, int low) const;
	void DumpRm(std::ostream &os, int high, int low) const;
	void DumpKeepRefCount(std::ostream &os, int high, int low) const;
	void DumpCc2(std::ostream &os, int high, int low) const;
	void DumpSRCB(std::ostream &os, int high0, int low0, int high1,
			int low1, int high2, int low2, int high3,
			int low3) const;
	void DumpEndConst(std::ostream &os, int high, int low) const;
	void DumpOffset(std::ostream &os, int high, int low) const;
	void DumpTarget(std::ostream &os, int high0, int low0, int high1,
			int low1) const;

public:

	/// Constructor
	Instruction();

	/// Decode the instruction in \a buffer at the offset specified by \a
	/// address.
	void Decode(const char *buffer, unsigned int address);

	/// Get opcode
	unsigned getOpcode() { return info ? (unsigned) info->opcode : OpcodeInvalid; }

	/// Get instruction name
	const char *getName() { return info ? info->name : "<UnknownInsturction>"; }

	/// Get Inst Bytes
	Bytes getInstBytes() const { return bytes; }

	/// Dump instruction as a sequence of hexadecimal digits
	void DumpHex(std::ostream &os) const;

	/// Dump instruction name into output stream.
	void Dump(std::ostream &os) const;
};


}  // namespace Kepler

#endif

