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

#ifndef ARCH_KEPLER_ASM_INST_H
#define ARCH_KEPLER_ASM_INST_H

#include <iostream>

namespace Kepler
{

// Forward declarations
class Asm;


enum InstOpcode
{
	InstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, ...)        \
	INST_##_name,
#include "Inst.def"
#undef DEFINST

	// Max
	InstOpcodeCount
};


struct InstInfo
{
	InstOpcode opcode;
	const char *name;
	const char *fmt_str;
};


struct InstDecodeInfo
{
	// Fields used when the table entry points to another table.
	int next_table_low;
	int next_table_high;
	InstDecodeInfo *next_table;

	// Field used when the table entry points to a final
	// instruction
	InstInfo *info;

	// Constructor that initializes all fields to 0
	InstDecodeInfo();
};




// 1st level struct
/* BFE, POPC, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD, LDS, STS
   FMNMX, FMUL, DADD, SEL, P2R, RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK */	//S2R, IMAD, ISETP, ISCADD
struct InstBytesGeneral0
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


// BRA, JMX, JMP, JCAL, BRX, CAL, PRET, PLONGJMP, SSY, PBK
struct InstBytesGeneral1
{
	unsigned long long int op0 	: 2; 	// 1:0
	unsigned long long int mod0 	: 16;	// 17:2
	unsigned long long int pred 	: 4; 	// 21:18
	unsigned long long int unused 	: 1; 	// 22
	unsigned long long int srcB 	: 21; 	// 43:23
	unsigned long long int mod1 	: 11;	// 54:44
	unsigned long long int op1 	: 9; 	// 63:55
};


/* GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETLMEMBASE, EXIT, LONGJUMP, RET, KIL,
 * BRK, CONT, RTT, SAM, RAM  */
struct InstBytesGeneral2
{
	unsigned long long int op0     	: 2; 	// 1:0
	unsigned long long int mod     	: 8; 	// 9:2
	unsigned long long int src     	: 8; 	// 17:10
	unsigned long long int pred    	: 4; 	// 21:18
	unsigned long long int unused   : 33; 	// 54:22
	unsigned long long int op1     	: 9; 	// 63:55
};

//LDC
struct InstBytesLDC
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

// MOV32I, FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I 
struct InstBytesImm
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


// 2nd level struct

/* BFE, POPC, IADD, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD,
 * FMNMX, FMUL, DADD, SEL, P2R */
struct InstBytesGeneral0Mod0A
{
	unsigned long long int reserved0: 10; 	// 9:0
	unsigned long long int srcA 	: 8; 	// 17:10
	unsigned long long int reserved1: 46;	// 63:18
};


// RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK
struct InstBytesGeneral0Mod0B
{
	unsigned long long int reserved0: 10;   // 9:0
	unsigned long long int mod2     : 8;    // 17:10
	unsigned long long int reserved1: 46;   // 63:18
};


// JCAL, CAL, PRET
struct InstBytesGeneral1Mod0A
{
	unsigned long long int reserved0: 2; 	// 1:0
	unsigned long long int unused0	: 5; 	// 6:2
	unsigned long long int srcB_mod : 1; 	// 7
	unsigned long long int noinc 	: 1; 	// 8
	unsigned long long int unused1	: 9; 	// 17:9
	unsigned long long int reserved1: 46; 	// 63:18
};


// BRA, JMX, JMP, BRX
struct InstBytesGeneral1Mod0B
{
	unsigned long long int reserved0: 2;    // 1:0
	unsigned long long int cc   	: 5;    // 6:2
	unsigned long long int srcB_mod : 1;    // 7
	unsigned long long int lmt    	: 1;    // 8
	unsigned long long int u	: 1;	// 9
	unsigned long long int srcB   	: 8;    // 17:10
	unsigned long long int reserved1: 46;   // 63:18
};


// BRA, BRX, CAL, PRET
struct InstBytesGeneral1Mod1A
{
	unsigned long long int reserved0: 44;  	// 43:0
	unsigned long long int srcB     : 2;    // 45:44
	unsigned long long int neg_srcB	: 1;    // 46
	unsigned long long int unused  	: 8;    // 54:47
	unsigned long long int reserved1: 9;   	// 63:55
};


// JMP, JCAL
struct InstBytesGeneral1Mod1B
{
	unsigned long long int reserved0: 44;   // 43:0
	unsigned long long int srcB   	: 11;    // 54:44
	unsigned long long int reserved1: 9;    // 63:55
};


// JMX
struct InstBytesGeneral1Mod1C
{
	unsigned long long int reserved0: 44;  	// 43:0
	unsigned long long int srcB     : 10;  	// 53:44
	unsigned long long int neg_srcB	: 1;	// 54
	unsigned long long int reserved1: 9;   	// 63:55
};


// GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETMEMLBASE
struct InstBytesGeneral2ModA
{
	unsigned long long int reserved0: 2;   	// 41:0
	unsigned long long int dst     	: 8;    // 9:2
	unsigned long long int reserved1: 54;  	// 63:10
};


// EXIT, LONGJUMP, RET, KIL, BRK, CONT, RTT, SAM, RAM
struct InstBytesGeneral2ModB
{
	unsigned long long int reserved0: 2;  	// 41:0
	unsigned long long int cc  	: 8;    // 6:2
	unsigned long long int unused	: 3;	// 9:7
	unsigned long long int reserved1: 54;  	// 63:10
};


// Need to figure out how to re-label constant once I know what this field represents
// MOV32I
struct InstBytesImmMod0A
{
	unsigned long long int reserved0: 10;  	// 9:0
	unsigned long long int unused  	: 4;    // 13:10
	unsigned long long int constant	: 4;	// 17:14 */     /* ?????????????????
	unsigned long long int reserved1: 46;  	// 63:18
};


// FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I
struct InstBytesImmMod0B
{
	unsigned long long int reserved0: 10;  	// 9:0
	unsigned long long int src	: 8;	// 17:10
	unsigned long long int reserved1: 46;  	// 63:18
};


// MOV32I
struct InstBytesImmMod1A
{
	unsigned long long int reserved0: 55;	// 54:0
	unsigned long long int op1	: 6;	// 60:55
	unsigned long long int reserved1: 3;	// 63:61
};


// FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I
struct InstBytesImmMod1B
{
	unsigned long long int reserved0: 55;	// 54:0
	unsigned long long int cc	: 1;	// 55
	unsigned long long int mod2	: 5;	// 60:56
	unsigned long long int reserved1: 3;	// 63:61
};


// FMUL321, IMUL32I
struct InstBytesImmMod1C
{
	unsigned long long int reserved0: 55;	// 54:0
	unsigned long long int cc	: 1;	// 55
	unsigned long long int mod2	: 3;	// 58:56
	unsigned long long int op1	: 2;	// 60:59
	unsigned long long int reserved1: 3;	// 63:61
};


// PSETP
struct InstBytesPSETP
{
	unsigned long long int op0:			 2; // 1:0
	unsigned long long int pred1:		 3; // 4:2
	unsigned long long int pred0:		 3; // 7:5
	unsigned long long int unknown5:	 6; // 13:8
	unsigned long long int pred2:		 4; // 17:14
	unsigned long long int pred:		 4; // 21:18
	unsigned long long int s:			 1; // 22
	unsigned long long int unknown4:	 4; // 26:23
	unsigned long long int bool_op1:	 2; // 28:27
	unsigned long long int unknown3:	 3; // 31:29
	unsigned long long int pred3:		 4; // 35:32
	unsigned long long int unknown2:	 6; // 41:36
	unsigned long long int pred4:		 4; // 45:42
	unsigned long long int unknown1:	 2; // 47:46
	unsigned long long int bool_op0:	 2; // 49:48
	unsigned long long int unknown0:	 4; // 53:50
	unsigned long long int op1:			10; // 63:54
};


// BRA
struct InstBytesBRA
{
	unsigned long long int op0:          2; // 1:0
	unsigned long long int cc:           5; // 6:2
	unsigned long long int src_mod:      1; // 7
	unsigned long long int lmt:          1; // 8
	unsigned long long int uni:          1; // 9
	unsigned long long int unknown2:	 8; // 17:10
	unsigned long long int pred:         4; // 21:18
	unsigned long long int unknown1:     1; // 22
	unsigned long long int offset:	    24; // 46:23
	unsigned long long int unknown0:	 8; // 54:47
	unsigned long long int op1:			 9; // 63:55
};


// SSY
struct InstBytesSSY
{
	unsigned long long int op0:          2; // 1:0
	unsigned long long int unknown2:     5; // 6:2
	unsigned long long int isconstmem:	 1; // 7
	unsigned long long int unknown1:    15; // 22:8
	unsigned long long int offset:	    24; // 46:23
	unsigned long long int unknown0:	 8; // 54:47
	unsigned long long int op1:			 9; // 63:55
};


// NOP
struct InstBytesNOP
{
	unsigned long long int op0:           2; // 1:0
	unsigned long long int unknown2:	  8; // 9:2
	unsigned long long int cc:			  5; // 14:10
	unsigned long long int trig:      	  1; // 15
	unsigned long long int unknown1:	  2; // 17:16
	unsigned long long int pred:	  	  4; // 21:18
	unsigned long long int s:    		  1; // 22
	unsigned long long int address:	     16; // 38:23
	unsigned long long int unknown0:	 15; // 53:39
	unsigned long long int op1:			 10; // 63:54
};


// SHL
struct InstBytesSHL
{
	unsigned long long int op0:			  2; // 1:0
	unsigned long long int dst:			  8; // 9:2
	unsigned long long int src1:		  8; // 17:10
	unsigned long long int pred:		  4; // 21:18
	unsigned long long int s:             1; // 22
	unsigned long long int src2:	  	 19; // 41:23
	unsigned long long int w:			  1; // 42
	unsigned long long int unknown2:      3; // 45:43
	unsigned long long int x:	  		  1; // 46
	unsigned long long int unknown1:	  3; // 49:47
	unsigned long long int cc:    		  1; // 50
	unsigned long long int unknown0:	  3; // 53:51
	unsigned long long int op1:	 	      9; // 62:54
	unsigned long long int op2:			  1; // 63
};


// PBK
struct InstBytesPBK
{
	unsigned long long int op0:			  2; // 1:0
	unsigned long long int unknown0:	  5; // 6:2
	unsigned long long int constant:	  1; // 7
	unsigned long long int unknown1:	 15; // 22:8
	unsigned long long int offset:       24; // 46:23
	unsigned long long int unknown2:	  8; // 54:47
	unsigned long long int op1:			  9; // 63:55
};


// PCNT
// constant unverified
struct InstBytesPCNT
{
	unsigned long long int op0:			  2; // 1:0
	unsigned long long int unknown0:	  5; // 6:2
	unsigned long long int constant:	  1; // 7
	unsigned long long int unknown1:	 15; // 22:8
	unsigned long long int offset:       24; // 46:23
	unsigned long long int unknown2:	  8; // 54:47
	unsigned long long int op1:			  9; // 63:55
};


// BRK
// cc not verified
struct InstBytesBRK
{
	unsigned long long int op0:			  2; // 1:0
	unsigned long long int cc:   	      5; // 6:2
	unsigned long long int unknown0:	 11; // 17:10
	unsigned long long int pred:	      4; // 21:18
	unsigned long long int unknown1:     33; // 54:22
	unsigned long long int op1:			  9; // 63:55
};


// CONT
// cc not verified
struct InstBytesCONT
{
	unsigned long long int op0:			  2; // 1:0
	unsigned long long int cc:   	      5; // 6:2
	unsigned long long int unknown0:	 11; // 17:10
	unsigned long long int pred:	      4; // 21:18
	unsigned long long int unknown1:     33; // 54:22
	unsigned long long int op1:			  9; // 63:55
};


// CAL
// constant unverified
struct InstBytesCAL
{
	unsigned long long int op0:			  2; // 1:0
	unsigned long long int unknown0:	 16; // 17:2
	unsigned long long int pred:	      4; // 21:18
	unsigned long long int unknown1:	  1; // 22:22
	unsigned long long int offset:       24; // 46:23
	unsigned long long int unknown2:	  8; // 54:47
	unsigned long long int op1:			  9; // 63:55
};


// IADD
struct InstBytesIADD
{
	unsigned long long int op0: 		  2; // 1:0
	unsigned long long int dst: 		  8; // 9:2
	unsigned long long int src1: 		  8; // 17:10
	unsigned long long int pred: 		  4; // 21:18
	unsigned long long int s: 			  1; // 22
	unsigned long long int src2: 	 	 19; // 41:23
	unsigned long long int unknown0: 	  4; // 45:42
	unsigned long long int x: 			  1; // 46
	unsigned long long int unknown1: 	  3; // 49:47
	unsigned long long int cc: 			  1; // 50
	unsigned long long int po: 			  2; // 52:51
	unsigned long long int sat:			  1; // 53
	unsigned long long int op1: 		  9; // 62:54
	unsigned long long int op2:			  1; // 63
};

union InstBytes
{
	unsigned char as_uchar[8];
	unsigned int as_uint[2];
	unsigned long long as_dword;

	InstBytesGeneral0 general0;
	InstBytesGeneral1 general1;
	InstBytesGeneral2 general2;
	InstBytesImm      immediate;
	InstBytesLDC      ldc;
	InstBytesPSETP    psetp;
	InstBytesSSY      ssy;
	InstBytesBRA      bra;
	InstBytesNOP      nop;
	InstBytesSHL	  shl;
	InstBytesPBK	  pbk;
	InstBytesCONT	  cont;
	InstBytesBRK	  brk;
	InstBytesPCNT	  pcnt;
	InstBytesCAL      cal;
	InstBytesIADD     iadd;
};


class Inst
{
	// Disassembler
	const Asm *as;

	// Instruction offset within .text section
	unsigned int address;

	// Instruction bytes
	InstBytes bytes;

	// Decoded information
	InstInfo *info;

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
	Inst();

	/// Decode the instruction in \a buffer at the offset specified by \a
	/// address.
	void Decode(const char *buffer, unsigned int address);

	/// Get opcode
	unsigned getOpcode() { return info ? (unsigned) info->opcode : InstOpcodeInvalid; }

	/// Get instruction name
	const char *getName() { return info ? info->name : "<UnknownInsturction>"; }

	///Get Inst Bytes
	InstBytes getInstBytes() const { return bytes; }

	/// Dump instruction as a sequence of hexadecimal digits
	void DumpHex(std::ostream &os) const;

	/// Dump instruction name into output stream.
	void Dump(std::ostream &os) const;
};


}  // namespace Kepler

#endif

