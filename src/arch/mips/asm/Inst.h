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


#ifdef __cplusplus


/*
 * C++ Code
 */

#include <iostream>

namespace MIPS
{

/* Forward declarations */
class Asm;


enum InstOpcode
{
	InstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3) \
	INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
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
	unsigned int function     :6;	/* [5:0] */
	unsigned int sa           :5;	/* [10:6] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};


struct InstBytesTarget
{
	unsigned int target       :26;	/* [25:0] */
	unsigned int opc          :6;	/* [31:26] */

};


struct InstBytesOffsetImm
{
	unsigned int offset       :16;	/* [15:0] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int base         :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};


struct InstBytesCC
{
	unsigned int offsetbr    :16;	/* [15:0] */
	unsigned int tf           :1;	/* [16] */
	unsigned int nd           :1;	/* [17] */
	unsigned int cc           :3;	/* [20:18] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};


struct InstBytesCode
{
	unsigned int function     :6;	/* [5:0] */
	unsigned int code         :10;	/* [15:6] */
	unsigned int rs_rt        :10;	/* [25:16] */
	unsigned int opc          :6;	/* [31:26] */

};


struct InstBytesSel
{
	unsigned int sel          :3;	/* [2:0] */
	unsigned int impl         :8;	/* [10:3] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

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
	/* Disassembler */
	Asm *as;

	/* Decoded instruction information */
	InstInfo *info;

	/* Virtual address of the instruction, as loaded from the ELF file */
	unsigned int addr;

	/* Instruction bytes */
	InstBytes bytes;

	void DumpBufSa(char **buf_ptr, int *size_ptr);
	void DumpBufRd(char **buf_ptr, int *size_ptr);
	void DumpBufRt(char **buf_ptr, int *size_ptr);
	void DumpBufRs(char **buf_ptr, int *size_ptr);
	void DumpBufTarget(char **buf_ptr, int *size_ptr);
	void DumpBufOffset(char **buf_ptr, int *size_ptr);
	void DumpBufOffsetbr(char **buf_ptr, int *size_ptr);
	void DumpBufImm(char **buf_ptr, int *size_ptr);
	void DumpBufImmhex(char **buf_ptr, int *size_ptr);
	void DumpBufBase(char **buf_ptr, int *size_ptr);
	void DumpBufSel(char **buf_ptr, int *size_ptr);
	void DumpBufCc(char **buf_ptr, int *size_ptr);
	void DumpBufPos(char **buf_ptr, int *size_ptr);
	void DumpBufFs(char **buf_ptr, int *size_ptr);
	void DumpBufSize(char **buf_ptr, int *size_ptr);
	void DumpBufFt(char **buf_ptr, int *size_ptr);
	void DumpBufFd(char **buf_ptr, int *size_ptr);
	void DumpBufCode(char **buf_ptr, int *size_ptr);

public:
	/* Target address used for instruction dump */
	unsigned int target;

	/* Constructor */
	Inst(Asm *as);

	/* Read an instruction from the buffer 'buf' into the 'bytes' field of the
	 * instruction object, and decode it by populating the 'info' field. The value
	 * in 'addr' gives the virtual address of the instruction, needed to print
	 * branch addresses. */
	void Decode(unsigned int addr, const char *buf);

	/* Dump an instruction. This function sets the 'target' field of the
	 * instruction offset to a value other than 0 if a target address has been
	 * printed. */
	void DumpBuf(char *buf, int size);
	void Dump(std::ostream &os);
	
	/* Print address and hexadecimal characters for instruction */
	void DumpHex(std::ostream &os);

	/* Getters */
	std::string GetName() { return info ? info->name : "<unknwon>"; }
	InstOpcode GetOpcode() { return info ? info->opcode :
			InstOpcodeInvalid; }
	unsigned int GetAddress() { return addr; }
	InstBytes *GetBytes() { return &bytes; }
};




} /* namespace MIPS */

#endif  /* __cplusplus */


/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

struct MIPSAsmWrap;
struct MIPSInstWrap;

typedef enum
{
	MIPSInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3) \
	MIPS_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	MIPSInstOpcodeCount

} MIPSInstOpcode;

typedef struct
{
	unsigned int function     :6;	/* [5:0] */
	unsigned int sa           :5;	/* [10:6] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

} MIPSInstBytesStandard;


typedef struct
{
	unsigned int target       :26;	/* [25:0] */
	unsigned int opc          :6;	/* [31:26] */

} MIPSInstBytesTarget;


typedef struct
{
	unsigned int offset       :16;	/* [15:0] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int base         :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

} MIPSInstBytesOffsetImm;


typedef struct
{
	unsigned int offsetbr    :16;	/* [15:0] */
	unsigned int tf           :1;	/* [16] */
	unsigned int nd           :1;	/* [17] */
	unsigned int cc           :3;	/* [20:18] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

} MIPSInstBytesCC;


typedef struct
{
	unsigned int function     :6;	/* [5:0] */
	unsigned int code         :10;	/* [15:6] */
	unsigned int rs_rt        :10;	/* [25:16] */
	unsigned int opc          :6;	/* [31:26] */

} MIPSInstBytesCode;


typedef struct
{
	unsigned int sel          :3;	/* [2:0] */
	unsigned int impl         :8;	/* [10:3] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

} MIPSInstBytesSel;


typedef union
{
	unsigned int word;

	MIPSInstBytesStandard standard;
	MIPSInstBytesTarget target;
	MIPSInstBytesOffsetImm offset_imm;
	MIPSInstBytesCC cc;
	MIPSInstBytesCode code;
	MIPSInstBytesSel sel;

} MIPSInstBytes;

struct MIPSInstWrap *MIPSInstWrapCreate(struct MIPSAsmWrap *as);
void MIPSInstWrapFree(struct MIPSInstWrap *self);
void MIPSInstWrapCopy(struct MIPSInstWrap *left, struct MIPSInstWrap *right);

void MIPSInstWrapDecode(struct MIPSInstWrap *self, unsigned int addr, void *buf);
void MIPSInstWrapDump(struct MIPSInstWrap *self, FILE *f);

MIPSInstOpcode MIPSInstWrapGetOpcode(struct MIPSInstWrap *self);
const char *MIPSInstWrapGetName(struct MIPSInstWrap *self);
unsigned int MIPSInstWrapGetAddress(struct MIPSInstWrap *self);
MIPSInstBytes *MIPSInstWrapGetBytes(struct MIPSInstWrap *self);

#ifdef __cplusplus
}
#endif


#endif  /* MIPS_ASM_INST_H */

