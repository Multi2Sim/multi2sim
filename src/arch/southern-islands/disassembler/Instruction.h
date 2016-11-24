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

#ifndef ARCH_SOUTHERN_ISLANDS_DISASSEMBLER_INSTRUCTION_H
#define ARCH_SOUTHERN_ISLANDS_DISASSEMBLER_INSTRUCTION_H

#include <lib/cpp/String.h>


namespace SI
{

// Forward declarations
class Disassembler;


class Instruction
{
public:

	/// Microcode Formats
	enum Format
	{
		FormatInvalid = 0,

		// Scalar ALU and Control Formats
		FormatSOP2,
		FormatSOPK,
		FormatSOP1,
		FormatSOPC,
		FormatSOPP,

		// Scalar Memory Format
		FormatSMRD,

		// Vector ALU Formats
		FormatVOP2,
		FormatVOP1,
		FormatVOPC,
		FormatVOP3a,
		FormatVOP3b,

		// Vector Parameter Interpolation Format
		FormatVINTRP,

		// LDS/GDS Format
		FormatDS,

		// Vector Memory Buffer Formats
		FormatMUBUF,
		FormatMTBUF,
		
		// Vector Memory Image Format
		FormatMIMG,

		// Export Formats
		FormatEXP,

		// Max
		FormatCount
	};
	
	/// String map to convert values of type Format
	static const misc::StringMap format_map;

	/// Instruction category enumeration
	enum Category
	{
		CategoryInvalid = 0,

		// Scalar ALU and Control Instructions
		CategorySOP2,
		CategorySOPK,
		CategorySOP1,
		CategorySOPC,
		CategorySOPP,

		// Scalar Memory Instruction
		CategorySMRD,

		// Vector ALU Instructions
		CategoryVOP2,
		CategoryVOP1,
		CategoryVOPC,
		CategoryVOP3a,
		CategoryVOP3b,

		// Vector Parameter Interpolation Instruction
		CategoryVINTRP,

		// LDS/GDS Instruction
		CategoryDS,

		// Vector Memory Buffer Instructions
		CategoryMUBUF,
		CategoryMTBUF,

		// Vector Memory Image Instruction
		CategoryMIMG,

		CategoryCount
	};


	/// Special register enumeration
	enum SpecialReg
	{
		SpecialRegInvalid = 0,
		SpecialRegVcc,
		SpecialRegScc,
		SpecialRegExec,
		SpecialRegTma,
		SpecialRegM0
	};

	/// String map to translate values of type SpecialRegister
	static const misc::StringMap special_reg_map;

	/// Buffer data format
	enum BufDataFormat
	{
		BufDataFormatInvalid = 0,
		BufDataFormat8 = 1,
		BufDataFormat16 = 2,
		BufDataFormat8_8 = 3,
		BufDataFormat32 = 4,
		BufDataFormat16_16 = 5,
		BufDataFormat10_11_11 = 6,
		BufDataFormat11_10_10 = 7,
		BufDataFormat10_10_10_2 = 8,
		BufDataFormat2_10_10_10 = 9,
		BufDataFormat8_8_8_8 = 10,
		BufDataFormat32_32 = 11,
		BufDataFormat16_16_16_16 = 12,
		BufDataFormat32_32_32 = 13,
		BufDataFormat32_32_32_32 = 14,
		BufDataFormatReserved = 15
	};

	/// String map to translate values of type BufDataFormat
	static const misc::StringMap buf_data_format_map;

	/// Buffer number format
	enum BufNumFormat
	{
		BufNumFormatUnorm = 0,
		BufNumFormatSnorm = 1,
		BufNumFormatUnscaled = 2,
		BufNumFormatSscaled = 3,
		BufNumFormatUint = 4,
		BufNumFormatSint = 5,
		BufNumFormatSnormNz = 6,
		BufNumFormatFloat = 7,
		BufNumFormatReserved = 8,
		BufNumFormatSrgb = 9,
		BufNumFormatUbnorm = 10,
		BufNumFormatUbnormNz = 11,
		BufNumFormatUbint = 12,
		BufNumFormatUbscaled = 13
	};
	
	/// String map to translate values of type BufNumFormat
	static const misc::StringMap buf_num_format_map;

	/// Instruction flags
	enum Flag
	{
		FlagNone = 0x0000,
		FlagOp8 = 0x0001,  // Opcode represents 8 comparison instructions
		FlagOp16 = 0x0002  // Opcode represents 16 comparison instructions
	};

	/// Unique instruction opcodes
	enum Opcode
	{
		OpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	Opcode_##_name,

#include "Instruction.def"
#undef DEFINST

		// Max
		OpcodeCount
	};

	/// SOP2 format
	struct BytesSOP2
	{
		unsigned int ssrc0    : 8;  //   [7:0]
		unsigned int ssrc1    : 8;  //  [15:8]
		unsigned int sdst     : 7;  // [22:16]
		unsigned int op       : 7;  // [29:23]
		unsigned int enc      : 2;  // [31:30]
		unsigned int lit_cnst : 32; // [63:32]
	};

	/// SOPK format
	struct BytesSOPK
	{
		unsigned int simm16 : 16;  //  [15:0]
		unsigned int sdst   : 7;   // [22:16]
		unsigned int op     : 5;   // [27:23]
		unsigned int enc    : 4;   // [31:28]
	};

	/// SOP1 format
	struct BytesSOP1
	{
		unsigned int ssrc0    : 8;  //   [7:0]
		unsigned int op       : 8;  //  [15:8]
		unsigned int sdst     : 7;  // [22:16]
		unsigned int enc      : 9;  // [31:23]
		unsigned int lit_cnst : 32; // [63:32]
	};

	/// SOPC format
	struct BytesSOPC
	{
		unsigned int ssrc0    : 8;  //   [7:0]
		unsigned int ssrc1    : 8;  //  [15:8]
		unsigned int op       : 7;  // [22:16]
		unsigned int enc      : 9;  // [31:23]
		unsigned int lit_cnst : 32; // [63:32]
	};

	/// SOPP format
	struct BytesSOPP
	{
		unsigned int simm16 : 16;  //  [15:0]
		unsigned int op     : 7;   // [22:16]
		unsigned int enc    : 9;   // [31:23]
	};

	/// SMRD format
	struct BytesSMRD
	{
		unsigned int offset : 8;   //   [7:0]
		unsigned int imm    : 1;   //      8 
		unsigned int sbase  : 6;   //  [14:9]
		unsigned int sdst   : 7;   // [21:15]
		unsigned int op     : 5;   // [26:22]
		unsigned int enc    : 5;   // [31:27]
	};

	/// VOP2 format
	struct BytesVOP2
	{
		unsigned int src0     : 9;   //   [8:0]
		unsigned int vsrc1    : 8;   //  [16:9]
		unsigned int vdst     : 8;   // [24:17]
		unsigned int op       : 6;   // [30:25]
		unsigned int enc      : 1;   //     31 
		unsigned int lit_cnst : 32;  // [63:32]
	};

	/// VOP1 format
	struct BytesVOP1
	{
		unsigned int src0     : 9;   //   [8:0]
		unsigned int op       : 8;   //  [16:9]
		unsigned int vdst     : 8;   // [24:17]
		unsigned int enc      : 7;   // [31:25]
		unsigned int lit_cnst : 32;  // [63:32]
	};

	/// VOPC format
	struct BytesVOPC
	{
		unsigned int src0     : 9;   //   [8:0]
		unsigned int vsrc1    : 8;   //  [16:9]
		unsigned int op       : 8;   // [24:17]
		unsigned int enc      : 7;   // [31:25]
		unsigned int lit_cnst : 32;  // [63:32]
	};

	/// VOP3A format
	struct BytesVOP3A
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

	/// VOP3B format
	struct BytesVOP3B
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

	/// VINTRP format
	struct BytesVINTRP
	{
		unsigned int vsrc     : 8;   //   [7:0]
		unsigned int attrchan : 2;   //   [9:8]
		unsigned int attr     : 6;   // [15:10]
		unsigned int op       : 2;   // [17:16]
		unsigned int vdst     : 8;   // [25:18]
		unsigned int enc      : 6;   // [31:26]
	};

	/// DS format
	struct BytesDS
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

	/// MTBUF format
	struct BytesMTBUF
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

	/// MUBUF format
	struct BytesMUBUF
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

	/// MIMG format
	struct BytesMIMG
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

	/// EXP format
	struct BytesEXP
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

	/// Instruction bytes
	union Bytes
	{
		unsigned char byte[8];
		unsigned int word[2];
		unsigned long long dword;

		BytesSOP2 sop2;
		BytesSOPK sopk;
		BytesSOP1 sop1;
		BytesSOPC sopc;
		BytesSOPP sopp;
		BytesSMRD smrd;
		BytesVOP2 vop2;
		BytesVOP1 vop1;
		BytesVOPC vopc;
		BytesVOP3A vop3a;
		BytesVOP3B vop3b;
		BytesVINTRP vintrp;
		BytesDS ds;
		BytesMUBUF mubuf;
		BytesMTBUF mtbuf;
		BytesMIMG mimg;
		BytesEXP exp;
	};

	/// 4-byte register
	union Register
	{
		int as_int;
		unsigned int as_uint;

		short int as_short[2];
		unsigned short int as_ushort[2];

		char as_byte[4];
		unsigned char as_ubyte[4];

		float as_float;
	};

	/// Entry in the instruction information table
	struct Info
	{
		/// Operation code
		Opcode opcode;

		/// Instruction category
		Category category;

		/// Instruction name
		const char *name;
		
		/// Format string
		const char *fmt_str;

		/// Word formats
		Format fmt;

		/// Opcode bits
		int op;

		/// Bitmap of flags
		Flag flags;

		/// Size of the micro-code format in bytes, not counting a
		/// possible additional literal added by a particular instance.
		int size;
	};

private:

	// Disassembler
	Disassembler *disassembler;

	// Instruction identifier with all information
	Info *info;

	// Instruction bytes
	Bytes bytes;

	// Instruction size in bytes, including the literal constant
	// if present.
	int size;

	// Instruction virtual address, stored when decoding
	int address;

	// String map for values in field 'sdst'
	static const misc::StringMap sdst_map;

	// String map for values in field 'ssrc'
	static const misc::StringMap ssrc_map;

	// String map for suffixes of instructions using 'op8'
	static const misc::StringMap op8_map;

	// String map for suffixes of instructions using 'op16'
	static const misc::StringMap op16_map;

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
	Instruction();

	/// Constants for special registers
	static const unsigned RegisterM0 = 124;
	static const unsigned RegisterVcc = 106;
	static const unsigned RegisterVccz = 251;
	static const unsigned RegisterExec = 126;
	static const unsigned RegisterExecz = 252;
	static const unsigned RegisterScc = 253;
	
	/// Decode instruction from buffer
	void Decode(const char *buffer, unsigned int offset);

	/// Print instruction
	void Dump(std::ostream &os = std::cout) const;

	/// Print relative address
	void DumpAddress(std::ostream &os = std::cout) const;

	/// Print instruction (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os, const Instruction &inst)
	{
		inst.Dump(os);
		return os;
	}

	// Reset instruction content
	void Clear();

	int getOp() { return info ? info->op : 0; }
	Opcode getOpcode() { return info ? info->opcode : OpcodeInvalid; }
	Format getFormat() { return info ? info->fmt : FormatInvalid; }
	const char *getName() { return info ? info->name : "<unknown>"; }
	Bytes *getBytes() { return &bytes; }
	int getSize() { return size; }
};


}  // namespace SI



#endif
