/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_INST_H
#define ARCH_SOUTHERN_ISLANDS_ASM_INST_H

#include <lib/cpp/String.h>


namespace SI
{

// Forward declarations
class Asm;


// Microcode Formats
extern misc::StringMap inst_format_map;
enum InstFormat
{
	InstFormatInvalid = 0,

	// Scalar ALU and Control Formats
	InstFormatSOP2,
	InstFormatSOPK,
	InstFormatSOP1,
	InstFormatSOPC,
	InstFormatSOPP,

	// Scalar Memory Format
	InstFormatSMRD,

	// Vector ALU Formats
	InstFormatVOP2,
	InstFormatVOP1,
	InstFormatVOPC,
	InstFormatVOP3a,
	InstFormatVOP3b,

	// Vector Parameter Interpolation Format
	InstFormatVINTRP,

	// LDS/GDS Format
	InstFormatDS,

	// Vector Memory Buffer Formats
	InstFormatMUBUF,
	InstFormatMTBUF,
	
	// Vector Memory Image Format
	InstFormatMIMG,

	// Export Formats
	InstFormatEXP,

	// Max
	InstFormatCount
};


enum InstCategory
{
	InstCategoryInvalid = 0,

	// Scalar ALU and Control Instructions
	InstCategorySOP2,
	InstCategorySOPK,
	InstCategorySOP1,
	InstCategorySOPC,
	InstCategorySOPP,

	// Scalar Memory Instruction
	InstCategorySMRD,

	// Vector ALU Instructions
	InstCategoryVOP2,
	InstCategoryVOP1,
	InstCategoryVOPC,
	InstCategoryVOP3a,
	InstCategoryVOP3b,

	// Vector Parameter Interpolation Instruction
	InstCategoryVINTRP,

	// LDS/GDS Instruction
	InstCategoryDS,

	// Vector Memory Buffer Instructions
	InstCategoryMUBUF,
	InstCategoryMTBUF,

	// Vector Memory Image Instruction
	InstCategoryMIMG,

	InstCategoryCount
};


extern misc::StringMap inst_special_reg_map;
enum InstSpecialReg
{
	InstSpecialRegInvalid = 0,
	InstSpecialRegVcc,
	InstSpecialRegScc,
	InstSpecialRegExec,
	InstSpecialRegTma
};


extern misc::StringMap inst_buf_data_format_map;
enum InstBufDataFormat
{
	InstBufDataFormatInvalid = 0,
	InstBufDataFormat8 = 1,
	InstBufDataFormat16 = 2,
	InstBufDataFormat8_8 = 3,
	InstBufDataFormat32 = 4,
	InstBufDataFormat16_16 = 5,
	InstBufDataFormat10_11_11 = 6,
	InstBufDataFormat11_10_10 = 7,
	InstBufDataFormat10_10_10_2 = 8,
	InstBufDataFormat2_10_10_10 = 9,
	InstBufDataFormat8_8_8_8 = 10,
	InstBufDataFormat32_32 = 11,
	InstBufDataFormat16_16_16_16 = 12,
	InstBufDataFormat32_32_32 = 13,
	InstBufDataFormat32_32_32_32 = 14,
	InstBufDataFormatReserved = 15
};


extern misc::StringMap inst_buf_num_format_map;
enum InstBufNumFormat
{
	InstBufNumFormatUnorm = 0,
	InstBufNumFormatSnorm = 1,
	InstBufNumFormatUnscaled = 2,
	InstBufNumFormatSscaled = 3,
	InstBufNumFormatUint = 4,
	InstBufNumFormatSint = 5,
	InstBufNumFormatSnormNz = 6,
	InstBufNumFormatFloat = 7,
	InstBufNumFormatReserved = 8,
	InstBufNumFormatSrgb = 9,
	InstBufNumFormatUbnorm = 10,
	InstBufNumFormatUbnormNz = 11,
	InstBufNumFormatUbint = 12,
	InstBufNumFormatUbscaled = 13
};


enum InstFlag
{
	InstFlagNone = 0x0000,
	InstFlagOp8 = 0x0001,  // Opcode represents 8 comparison instructions
	InstFlagOp16 = 0x0002  // Opcode represents 16 comparison instructions
};


enum InstOpcode
{
	InstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	INST_##_name,

#include "Inst.def"
#undef DEFINST

	// Max
	InstOpcodeCount
};


struct InstBytesSOP2
{
        unsigned int ssrc0    : 8;  //   [7:0]
        unsigned int ssrc1    : 8;  //  [15:8]
        unsigned int sdst     : 7;  // [22:16]
        unsigned int op       : 7;  // [29:23]
        unsigned int enc      : 2;  // [31:30]
        unsigned int lit_cnst : 32; // [63:32]
};


struct InstBytesSOPK
{
        unsigned int simm16 : 16;  //  [15:0]
        unsigned int sdst   : 7;   // [22:16]
        unsigned int op     : 5;   // [27:23]
        unsigned int enc    : 4;   // [31:28]
};


struct InstBytesSOP1
{
        unsigned int ssrc0    : 8;  //   [7:0]
        unsigned int op       : 8;  //  [15:8]
        unsigned int sdst     : 7;  // [22:16]
        unsigned int enc      : 9;  // [31:23]
        unsigned int lit_cnst : 32; // [63:32]
};


struct InstBytesSOPC
{
        unsigned int ssrc0    : 8;  //   [7:0]
        unsigned int ssrc1    : 8;  //  [15:8]
        unsigned int op       : 7;  // [22:16]
        unsigned int enc      : 9;  // [31:23]
        unsigned int lit_cnst : 32; // [63:32]
};


struct InstBytesSOPP
{
        unsigned int simm16 : 16;  //  [15:0]
        unsigned int op     : 7;   // [22:16]
        unsigned int enc    : 9;   // [31:23]
};


struct InstBytesSMRD
{
        unsigned int offset : 8;   //   [7:0]
        unsigned int imm    : 1;   //      8 
        unsigned int sbase  : 6;   //  [14:9]
        unsigned int sdst   : 7;   // [21:15]
        unsigned int op     : 5;   // [26:22]
        unsigned int enc    : 5;   // [31:27]
};


struct InstBytesVOP2
{
        unsigned int src0     : 9;   //   [8:0]
        unsigned int vsrc1    : 8;   //  [16:9]
        unsigned int vdst     : 8;   // [24:17]
        unsigned int op       : 6;   // [30:25]
        unsigned int enc      : 1;   //     31 
        unsigned int lit_cnst : 32;  // [63:32]
};


struct InstBytesVOP1
{
        unsigned int src0     : 9;   //   [8:0]
        unsigned int op       : 8;   //  [16:9]
        unsigned int vdst     : 8;   // [24:17]
        unsigned int enc      : 7;   // [31:25]
        unsigned int lit_cnst : 32;  // [63:32]
};


struct InstBytesVOPC
{
        unsigned int src0     : 9;   //   [8:0]
        unsigned int vsrc1    : 8;   //  [16:9]
        unsigned int op       : 8;   // [24:17]
        unsigned int enc      : 7;   // [31:25]
        unsigned int lit_cnst : 32;  // [63:32]
};


struct InstBytesVOP3A
{
        unsigned int vdst     : 8;   //   [7:0]
        unsigned int abs      : 3;   //  [10:8]
        unsigned int clamp    : 1;   //     11 
        unsigned int reserved : 5;   // [16:12]
        unsigned int op       : 9;   // [25:17]
        unsigned int enc      : 6;   // [31:26]
        unsigned int src0     : 9;   // [40:32]
        unsigned int src1     : 9;   // [49:41]
        unsigned int src2     : 9;   // [57:49]
        unsigned int omod     : 2;   // [59:58]
        unsigned int neg      : 3;   // [63:60]
};


struct InstBytesVOP3B
{
        unsigned int vdst     : 8;   //   [7:0]
        unsigned int sdst     : 7;   //  [14:8]
        unsigned int reserved : 2;   // [16:15]
        unsigned int op       : 9;   // [25:17]
        unsigned int enc      : 6;   // [31:26]
        unsigned int src0     : 9;   // [40:32]
        unsigned int src1     : 9;   // [49:41]
        unsigned int src2     : 9;   // [57:49]
        unsigned int omod     : 2;   // [59:58]
        unsigned int neg      : 3;   // [63:60]
};


struct InstBytesVINTRP
{
        unsigned int vsrc     : 8;   //   [7:0]
        unsigned int attrchan : 2;   //   [9:8]
        unsigned int attr     : 6;   // [15:10]
        unsigned int op       : 2;   // [17:16]
        unsigned int vdst     : 8;   // [25:18]
        unsigned int enc      : 6;   // [31:26]
};


struct InstBytesDS
{
        unsigned int offset0  : 8;   //   [7:0]
        unsigned int offset1  : 8;   //  [15:8]
        unsigned int reserved : 1;   //     16 
        unsigned int gds      : 1;   //     17 
        unsigned int op       : 8;   // [25:18]
        unsigned int enc      : 6;   // [31:26]
        unsigned int addr     : 8;   // [39:32]
        unsigned int data0    : 8;   // [47:40]
        unsigned int data1    : 8;   // [55:48]
        unsigned int vdst     : 8;   // [63:56]
};


struct InstBytesMTBUF
{
        unsigned int offset   : 12;  //  [11:0]
        unsigned int offen    : 1;   //     12 
        unsigned int idxen    : 1;   //     13 
        unsigned int glc      : 1;   //     14 
        unsigned int addr64   : 1;   //     15 
        unsigned int op       : 3;   // [18:16]
        unsigned int dfmt     : 4;   // [22:19]
        unsigned int nfmt     : 3;   // [25:23]
        unsigned int enc      : 6;   // [31:26]
        unsigned int vaddr    : 8;   // [39:32]
        unsigned int vdata    : 8;   // [47:40]
        unsigned int srsrc    : 5;   // [52:48]
        unsigned int reserved : 1;   //     53 
        unsigned int slc      : 1;   //     54 
        unsigned int tfe      : 1;   //     55 
        unsigned int soffset  : 8;   // [63:56]
};


struct InstBytesMUBUF
{
        unsigned int offset    : 12;  //  [11:0]
        unsigned int offen     : 1;   //     12 
        unsigned int idxen     : 1;   //     13 
        unsigned int glc       : 1;   //     14 
        unsigned int addr64    : 1;   //     15 
        unsigned int lds       : 1;   //     16 
        unsigned int reserved0 : 1;   //     17 
        unsigned int op        : 7;   // [24:18]
        unsigned int reserved1 : 1;   //     25 
        unsigned int enc       : 6;   // [31:26]
        unsigned int vaddr     : 8;   // [39:32]
        unsigned int vdata     : 8;   // [47:40]
        unsigned int srsrc     : 5;   // [52:48]
        unsigned int reserved2 : 1;   //     53 
        unsigned int slc       : 1;   //     54 
        unsigned int tfe       : 1;   //     55 
        unsigned int soffset   : 8;   // [63:56]
};


struct InstBytesMIMG
{
        unsigned int reserved0 : 8;   //   [7:0]
        unsigned int dmask     : 4;   //  [11:8]
        unsigned int unorm     : 1;   //     12 
        unsigned int glc       : 1;   //     13 
        unsigned int da        : 1;   //     14 
        unsigned int r128      : 1;   //     15 
        unsigned int tfe       : 1;   //     16 
        unsigned int lwe       : 1;   //     17 
        unsigned int op        : 7;   // [24:18]
        unsigned int slc       : 1;   //     25 
        unsigned int enc       : 6;   // [31:26]
        unsigned int vaddr     : 8;   // [39:32]
        unsigned int vdata     : 8;   // [47:40]
        unsigned int srsrc     : 5;   // [52:48]
        unsigned int ssamp     : 5;   // [57:53]
        unsigned int reserved1  : 6;   // [63:58]
};


struct InstBytesEXP
{
        unsigned int en       : 4;   //   [3:0]
        unsigned int tgt      : 6;   //   [9:4]
        unsigned int compr    : 1;   //     10 
        unsigned int done     : 1;   //     11 
        unsigned int vm       : 1;   //     12 
        unsigned int reserved : 13;  // [25:13]
        unsigned int enc      : 6;   // [31:26]
        unsigned int vsrc0    : 8;   // [39:32]
        unsigned int vsrc1    : 8;   // [47:40]
        unsigned int vsrc2    : 8;   // [55:48]
        unsigned int vsrc3    : 8;   // [63:56]
};


union InstBytes
{
	unsigned char byte[8];
	unsigned int word[2];
	unsigned long long dword;

	InstBytesSOP2 sop2;
	InstBytesSOPK sopk;
	InstBytesSOP1 sop1;
	InstBytesSOPC sopc;
	InstBytesSOPP sopp;
	InstBytesSMRD smrd;
	InstBytesVOP2 vop2;
	InstBytesVOP1 vop1;
	InstBytesVOPC vopc;
	InstBytesVOP3A vop3a;
	InstBytesVOP3B vop3b;
	InstBytesVINTRP vintrp;
	InstBytesDS ds;
	InstBytesMUBUF mubuf;
	InstBytesMTBUF mtbuf;
	InstBytesMIMG mimg;
	InstBytesEXP exp;
};


union InstReg
{
	int as_int;
	unsigned int as_uint;

	short int as_short[2];
	unsigned short int as_ushort[2];

	char as_byte[4];
	unsigned char as_ubyte[4];

	float as_float;
};


struct InstInfo
{
	InstOpcode opcode;
	InstCategory category;

	const char *name;
	const char *fmt_str;
	InstFormat fmt;  // Word formats
	int op;  // Opcode bits
	InstFlag flags;  // Flag bitmap

	/* Size of the micro-code format in bytes, not counting a possible
	 * additional literal added by a particular instance. */
	int size;
};


class Inst
{
	// Disassembler
	Asm *as;

	// Instruction identifier with all information
	InstInfo *info;

	// Instruction bytes
	InstBytes bytes;

	// Instruction size in bytes, including the literal constant
	// if present.
	int size;

	// Instruction virtual address, stored when decoding
	int address;

	// Dump functions
	static void DumpOperand(std::ostream &os, int operand);
	static void DumpOperandSeries(std::ostream &os, int start, int end);
	static void DumpScalar(std::ostream &os, int operand);
	static void DumpScalarSeries(std::ostream &os, int start, int end);
	static void DumpVector(std::ostream &os, int operand);
	static void DumpVectorSeries(std::ostream &os, int start, int end);
	static void DumpOperandExp(std::ostream &os, int operand);
	static void DumpSeriesVdata(std::ostream &os, unsigned int vdata, int op);
	void DumpSsrc(std::ostream &os, unsigned int ssrc) const;
	void Dump64Ssrc(std::ostream &os, unsigned int ssrc) const;
	void DumpVop3Src(std::ostream &os, unsigned int src, int neg) const;
	void DumpVop364Src(std::ostream &os, unsigned int src, int neg) const;
	void DumpMaddr(std::ostream &os) const;
	void DumpDug(std::ostream &os) const;

public:

	/// Constructor
	Inst(Asm *as);

	/// Decode instruction from buffer
	void Decode(const char *buffer, unsigned int offset);

	/// Print instruction
	void Dump(std::ostream &os) const;

	/// Print instruction (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os, const Inst &inst) {
		inst.Dump(os);
		return os;
	}

	// Reset instruction content
	void Clear();

	int getOp() { return info ? info->op : 0; }
	InstOpcode getOpcode() { return info ? info->opcode : InstOpcodeInvalid; }
	InstFormat getFormat() { return info ? info->fmt : InstFormatInvalid; }
	const char *getName() { return info ? info->name : "<unknown>"; }
	InstBytes *getBytes() { return &bytes; }
	int getSize() { return size; }
};


}  // namespace SI



#endif
