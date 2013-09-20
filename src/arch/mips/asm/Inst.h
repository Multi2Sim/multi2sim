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

#ifndef MIPS_ASM_INST_H
#define MIPS_ASM_INST_H

#include <iostream>


/* Forward declarations */
class MIPSAsm;


/*
 * Class 'MIPSInst'
 */

enum MIPSInstOpcode
{
	MIPSInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3) \
	MIPS_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	MIPSInstOpcodeCount

};


struct MIPSInstInfo
{
	MIPSInstOpcode opcode;
	const char *name;
	const char *fmt_str;
	int size;
	int next_table_low;
	int next_table_high;
	MIPSInstInfo *next_table;
};


struct MIPSInstBytesStandard
{
	unsigned int function     :6;	/* [5:0] */
	unsigned int sa           :5;	/* [10:6] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};


struct MIPSInstBytesTarget
{
	unsigned int target       :26;	/* [25:0] */
	unsigned int opc          :6;	/* [31:26] */

};


struct MIPSInstBytesOffsetImm
{
	unsigned int offset       :16;	/* [15:0] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int base         :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};


struct MIPSInstBytesCC
{
	unsigned int offsetbr    :16;	/* [15:0] */
	unsigned int tf           :1;	/* [16] */
	unsigned int nd           :1;	/* [17] */
	unsigned int cc           :3;	/* [20:18] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};


struct MIPSInstBytesCode
{
	unsigned int function     :6;	/* [5:0] */
	unsigned int code         :10;	/* [15:6] */
	unsigned int rs_rt        :10;	/* [25:16] */
	unsigned int opc          :6;	/* [31:26] */

};


struct MIPSInstBytesSel
{
	unsigned int sel          :3;	/* [2:0] */
	unsigned int impl         :8;	/* [10:3] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};


union MIPSInstBytes
{
	unsigned int word;

	MIPSInstBytesStandard standard;
	MIPSInstBytesTarget target;
	MIPSInstBytesOffsetImm offset_imm;
	MIPSInstBytesCC cc;
	MIPSInstBytesCode code;
	MIPSInstBytesSel sel;

};


class MIPSInst
{
public:
	/* Disassembler */
	MIPSAsm *as;

	/* Virtual address of the instruction, as loaded from the ELF file */
	unsigned int addr;

	/* Instruction bytes */
	MIPSInstBytes bytes;

	/* Decoded instruction information */
	MIPSInstInfo *info;

	/* Target address printed by last call to 'MIPSInstDumpBuf' or
	 * 'MIPSInstDump', or 0 if no address was printed. */
	unsigned int target;
};


void MIPSInstCreate(MIPSInst *self, MIPSAsm *as);
void MIPSInstDestroy(MIPSInst *self);

/* Read an instruction from the buffer 'buf' into the 'bytes' field of the
 * instruction object, and decode it by populating the 'info' field. The value
 * in 'addr' gives the virtual address of the instruction, needed to print
 * branch addresses. */
void MIPSInstDecode(MIPSInst *self, unsigned int addr, void *buf);

/* Dump an instruction. This function sets the 'target' field of the
 * instruction offset to a value other than 0 if a target address has been
 * printed. */
void MIPSInstDumpBuf(MIPSInst *self, char *buf, int size);
void MIPSInstDump(MIPSInst *self, std::ostream &os);

/* Print address and hexadecimal characters for instruction */
void MIPSInstDumpHex(MIPSInst *self, std::ostream &os);


#endif

