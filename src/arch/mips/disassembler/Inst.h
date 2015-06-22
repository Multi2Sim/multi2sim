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

#ifndef ARCH_MIPS_DISASSEMBLER_INST_H
#define ARCH_MIPS_DISASSEMBLER_INST_H

#include <iostream>

namespace MIPS
{

// Forward declarations
class Disassembler;


enum InstOpcode
{
	InstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3) \
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
	int size;
	int next_table_low;
	int next_table_high;
	InstInfo *next_table;
};


struct InstBytesStandard
{
	unsigned int function     :6;	// [5:0]
	unsigned int sa           :5;	// [10:6]
	unsigned int rd           :5;	// [15:11]
	unsigned int rt           :5;	// [20:16]
	unsigned int rs           :5;	// [25:21]
	unsigned int opc          :6;	// [31:26]

};


struct InstBytesTarget
{
	unsigned int target       :26;	// [25:0]
	unsigned int opc          :6;	// [31:26]

};


struct InstBytesOffsetImm
{
	unsigned int offset       :16;	// [15:0]
	unsigned int rt           :5;	// [20:16]
	unsigned int base         :5;	// [25:21]
	unsigned int opc          :6;	// [31:26]

};


struct InstBytesCC
{
	unsigned int offsetbr    :16;	// [15:0]
	unsigned int tf           :1;	// [16]
	unsigned int nd           :1;	// [17]
	unsigned int cc           :3;	// [20:18]
	unsigned int rs           :5;	// [25:21]
	unsigned int opc          :6;	// [31:26]

};


struct InstBytesCode
{
	unsigned int function     :6;	// [5:0]
	unsigned int code         :10;	// [15:6]
	unsigned int rs_rt        :10;	// [25:16]
	unsigned int opc          :6;	// [31:26]

};


struct InstBytesSel
{
	unsigned int sel          :3;	// [2:0]
	unsigned int impl         :8;	// [10:3]
	unsigned int rd           :5;	// [15:11]
	unsigned int rt           :5;	// [20:16]
	unsigned int rs           :5;	// [25:21]
	unsigned int opc          :6;	// [31:26]

};


union InstBytes
{
	unsigned int word;

	InstBytesStandard standard;
	InstBytesTarget target;
	InstBytesOffsetImm offset_imm;
	InstBytesCC cc;
	InstBytesCode code;
	InstBytesSel sel;

};


class Inst
{
	// Disassembler
	Disassembler *disassembler;

	// Decoded instruction information
	InstInfo *info;

	// Virtual address of the instruction, as loaded from the ELF file
	unsigned int addr;

	// Instruction bytes
	InstBytes bytes;

	// Target address used for instruction dump
	unsigned int target;

	void DumpSa(std::ostream &os) const;
	void DumpRd(std::ostream &os) const;
	void DumpRt(std::ostream &os) const;
	void DumpRs(std::ostream &os) const;
	void DumpTarget(std::ostream &os);
	void DumpOffset(std::ostream &os) const;
	void DumpOffsetbr(std::ostream &os);
	void DumpImm(std::ostream &os) const;
	void DumpImmhex(std::ostream &os) const;
	void DumpBase(std::ostream &os) const;
	void DumpSel(std::ostream &os) const;
	void DumpCc(std::ostream &os) const;
	void DumpPos(std::ostream &os) const;
	void DumpFs(std::ostream &os) const;
	void DumpSize(std::ostream &os) const;
	void DumpFt(std::ostream &os) const;
	void DumpFd(std::ostream &os) const;
	void DumpCode(std::ostream &os) const;
	void DumpHint(std::ostream &os) const;

public:

	/// Constructor
	Inst();

	/// Read an instruction from the buffer 'buf' into the 'bytes' field of the
	/// instruction object, and decode it by populating the 'info' field. The value
	/// in 'addr' gives the virtual address of the instruction, needed to print
	/// branch addresses.
	void Decode(unsigned int addr, const char *buf);

	/// Dump an instruction. This function sets the 'target' field of the
	/// instruction offset to a value other than 0 if a target address has been
	/// printed.
	// FIXME - Fritz, the behavior of this function should be modified to
	/// not alter the 'target' field. We want to be able to declare this
	/// function as 'const'. Instead, it should be the 'Decode()' function
	/// the one setting this field.
	void Dump(std::ostream &os);
	
	/// Print address and hexadecimal characters for instruction
	void DumpHex(std::ostream &os);

	/// Getters
	std::string getName() { return info ? info->name : "<unknwon>"; }
	InstOpcode getOpcode() { return info ? info->opcode : InstOpcodeInvalid; }
	unsigned int getAddress() { return addr; }
	InstBytes *getBytes() { return &bytes; }
	unsigned int getTarget() {return target; }
};


} // namespace MIPS

#endif

