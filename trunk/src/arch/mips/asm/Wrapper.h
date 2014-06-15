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

#ifndef MIPS_ASM_WRAPPER_H
#define MIPS_ASM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Inst
////////////////////////////////////////////////////////////////////////////////

struct MIPSAsmWrap;
struct MIPSInstWrap;

typedef enum
{
	MIPSInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3) \
	MIPS_INST_##_name,
#include "Inst.def"
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

struct MIPSInstWrap *MIPSInstWrapCreate();
void MIPSInstWrapFree(struct MIPSInstWrap *self);
void MIPSInstWrapCopy(struct MIPSInstWrap *left, struct MIPSInstWrap *right);

void MIPSInstWrapDecode(struct MIPSInstWrap *self, unsigned int addr, void *buf);
void MIPSInstWrapDump(struct MIPSInstWrap *self, FILE *f);

MIPSInstOpcode MIPSInstWrapGetOpcode(struct MIPSInstWrap *self);
const char *MIPSInstWrapGetName(struct MIPSInstWrap *self);
unsigned int MIPSInstWrapGetAddress(struct MIPSInstWrap *self);
MIPSInstBytes *MIPSInstWrapGetBytes(struct MIPSInstWrap *self);


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////

struct MIPSAsmWrap;

struct MIPSAsmWrap *MIPSAsmWrapCreate(void);
void MIPSAsmWrapFree(struct MIPSAsmWrap *self);

void MIPSAsmWrapDisassembleBinary(struct MIPSAsmWrap *self, char *path);





#ifdef __cplusplus
}
#endif


#endif  // MIPS_ASM_WRAPPER_H

