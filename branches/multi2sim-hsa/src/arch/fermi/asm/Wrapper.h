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

#ifndef FERMI_ASM_WRAPPER_H
#define FERMI_ASM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif


struct FrmInstWrap;
struct FrmAsmWrap;


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Inst
////////////////////////////////////////////////////////////////////////////////

typedef struct
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
} FrmFmtReg;

typedef struct
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int imm32 : 32; // 57:26
	unsigned long long int dmod : 1; // 58
	unsigned long long int func : 5; // 63:59
} FrmFmtImm;

typedef struct
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
} FrmFmtOther;

typedef struct
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int dst : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int fmod1_srco : 32; // 57:26
	unsigned long long int func : 6; // 63:58
} FrmFmtLdSt;

typedef struct
{
	unsigned long long int cat : 4; // 3:0
	unsigned long long int fmod0 : 6; // 9:4
	unsigned long long int pred : 4; // 13:10
	unsigned long long int mmod : 6; // 19:14
	unsigned long long int src1 : 6; // 25:20
	unsigned long long int imm32 : 32; // 57:26
	unsigned long long int x : 1; // 58
	unsigned long long int func : 5; // 63:59
} FrmFmtCtrl;

typedef enum
{
	FrmInstCategorySpFp,  // Single-precision floating point
	FrmInstCategoryDpFp,  // Double-precision floating point
	FrmInstCategoryImm,  // Immediate
	FrmInstCategoryInt,  // Integer
	FrmInstCategoryOther,  // Other
	FrmInstCategoryLdSt,  // Load/Store from/to read/write memory
	FrmInstCategoryLdRO,  // Load from read-only memory
	FrmInstCategoryCtrl,  // Control

	FrmInstCategoryCount
} FrmInstCategory;

typedef enum
{
	FrmInstIdInvalid = 0,

#define DEFINST(_op, _fmt_str, _opcode) \
		FrmInstId##_op,
#include "asm.dat"
#undef DEFINST

	FrmInstIdCount
} FrmInstId;

typedef enum
{
	FrmInstOpInvalid = 0,

#define DEFINST(_op, _fmt_str, _opcode) \
		FrmInstOp##_op = _opcode,
#include "asm.dat"
#undef DEFINST
} FrmInstOp;

typedef union
{
	unsigned char bytes[8];
	unsigned int word[2];
	unsigned long long int dword;

	FrmFmtReg fmt_reg;
	FrmFmtImm fmt_imm;
	FrmFmtOther fmt_other;
	FrmFmtLdSt fmt_ldst;
	FrmFmtCtrl fmt_ctrl;
} FrmInstBytes;

typedef struct
{
	FrmInstOp op;
	FrmInstCategory category;
	char *name;
	char *fmt_str;
} FrmInstInfo;

struct FrmInstWrap *FrmInstWrapCreate();
void FrmInstWrapFree(struct FrmInstWrap *self);
void FrmInstWrapCopy(struct FrmInstWrap *left, struct FrmInstWrap *right);

void FrmInstWrapDecode(struct FrmInstWrap *self, unsigned int addr, void *ptr);
FrmInstBytes *FrmInstWrapGetBytes(struct FrmInstWrap *self);
FrmInstId FrmInstWrapGetId(struct FrmInstWrap *self);
FrmInstOp FrmInstWrapGetOpcode(struct FrmInstWrap *self);
FrmInstCategory FrmInstWrapGetCategory(struct FrmInstWrap *self);
char *FrmInstWrapGetName(struct FrmInstWrap *self);


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////

struct FrmAsmWrap *FrmAsmWrapCreate(void);
void FrmAsmWrapFree(struct FrmAsmWrap *self);

void FrmAsmWrapDisassembleBinary(struct FrmAsmWrap *self, char *path);
void FrmAsmWrapDisassembleBuffer(struct FrmAsmWrap *self, char *buffer,
		unsigned size);

FrmInstInfo *FrmAsmWrapGetInstInfo(struct FrmAsmWrap *self, unsigned cat,
		unsigned op);


#ifdef __cplusplus
}
#endif

#endif  // FERMI_ASM_WRAPPER_H
