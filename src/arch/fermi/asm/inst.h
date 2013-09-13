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


#include <lib/class/class.h>



/*
 * Class 'FrmInst'
 */

/* Special Registers */
typedef enum
{
	FrmInstSRegLaneld = 0,
	FrmInstSRegVirtCfg = 2,
	FrmInstSRegVirtId,
	FrmInstSRegPM0,
	FrmInstSRegPM1,
	FrmInstSRegPM2,
	FrmInstSRegPM3,
	FrmInstSRegPM4,
	FrmInstSRegPM5,
	FrmInstSRegPM6,
	FrmInstSRegPM7,
	FrmInstSRegPrimType = 16,
	FrmInstSRegInvocationID,
	FrmInstSRegYDirection,
	FrmInstSRegMachineID0 = 24,
	FrmInstSRegMachineID1,
	FrmInstSRegMachineID2,
	FrmInstSRegMachineID3,
	FrmInstSRegAffinity,
	FrmInstSRegTid = 32,
	FrmInstSRegTidX,
	FrmInstSRegTidY,
	FrmInstSRegTidZ,
	FrmInstSRegCTAParam,
	FrmInstSRegCTAidX,
	FrmInstSRegCTAidY,
	FrmInstSRegCTAidZ,
	FrmInstSRegNTid,
	FrmInstSRegNTidX,
	FrmInstSRegNTidY,
	FrmInstSRegNTidZ,
	FrmInstSRegGridParam,
	FrmInstSRegNCTAidX,
	FrmInstSRegNCTAidY,
	FrmInstSRegNCTAidZ,
	FrmInstSRegSWinLo,
	FrmInstSRegSWINSZ,
	FrmInstSRegSMemSz,
	FrmInstSRegSMemBanks,
	FrmInstSRegLWinLo,
	FrmInstSRegLWINSZ,
	FrmInstSRegLMemLoSz,
	FrmInstSRegLMemHiOff,
	FrmInstSRegEqMask,
	FrmInstSRegLtMask,
	FrmInstSRegLeMask,
	FrmInstSRegGtMask,
	FrmInstSRegGeMask,
	FrmInstSRegClockLo = 80,
	FrmInstSRegClockHi,

	/* Max */
	FrmInstSRegCount
} FrmInstSReg;



/* 1st level struct */

typedef struct
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int src2 : 20; /* 45:26 */
	unsigned long long int src2_mod : 2; /* 47:46 */
	unsigned long long int dst_cc : 1; /* 48 */
	unsigned long long int mod1 : 9; /* 57:49 */
	unsigned long long int op1 : 6; /* 63:58 */
} FrmInstBytesGeneral0;


typedef struct
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int src2 : 20; /* 45:26 */
	unsigned long long int src2_mod : 2; /* 47:46 */
	unsigned long long int dst_cc : 1; /* 48 */
	unsigned long long int R : 3; /* 51:49 */
	unsigned long long int _reserved0: 1; /* 52 */
	unsigned long long int logic : 2; /* 54:53 */
	unsigned long long int cmp : 4; /* 58:55 */
	unsigned long long int op1 : 5; /* 63:59 */
} FrmInstBytesGeneral1;


typedef struct
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int imm32 : 32; /* 57:26 */
	unsigned long long int op1 : 6; /* 63:58 */
} FrmInstBytesImm;


typedef struct
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int offset : 16; /* 41:26 */
	unsigned long long int mod1 : 16; /* 57:42 */
	unsigned long long int op1 : 6; /* 63:58 */
} FrmInstBytesOffs;


typedef struct
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int tgt_mod : 1; /* 14 */
	unsigned long long int u : 1; /* 15 */
	unsigned long long int noinc : 1; /* 16 */
	unsigned long long int _const0 : 9; /* 25:17 */
	unsigned long long int target : 24; /* 49:26 */
	unsigned long long int _reserved0 : 8; /* 57:50 */
	unsigned long long int op1 : 6; /* 63:58 */
} FrmInstBytesTgt;

/* 2nd level struct */

typedef struct
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int R : 4; /* 52:49 */
	unsigned long long int  _const0: 5; /* 57:53 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
} FrmInstBytesGeneral0Mod1A;


typedef struct
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int src3 : 6; /* 54:49 */
	unsigned long long int rnd: 2; /* 56:55 */
	unsigned long long int _const0 : 1; /* 57 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
} FrmInstBytesGeneral0Mod1B;


typedef struct
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int rnd : 2; /* 50:49 */
	unsigned long long int word0 : 7; /* 57:51 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
} FrmInstBytesGeneral0Mod1C;


typedef struct
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int src3 : 6; /* 54:49 */
	unsigned long long int cmp : 3; /* 57:55 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
} FrmInstBytesGeneral0Mod1D;


typedef struct
{
	unsigned long long int _reserved0 : 42; /* 41:0 */
	unsigned long long int _const0 : 8; /* 49:42 */
	unsigned long long int trig : 1; /* 50 */
	unsigned long long int op : 4; /* 54:51 */
	unsigned long long int _const1 : 3; /* 57:55 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
} FrmInstBytesOffsMod1A;


typedef struct
{
	unsigned long long int _reserved0 : 42; /* 41:0 */
	unsigned long long int _const0 : 8; /* 49:42 */
	unsigned long long int R : 3; /* 52:50 */
	unsigned long long int _reserved1 : 5; /* 57:53 */
	unsigned long long int _reserved2 : 6; /* 63:58 */
} FrmInstBytesOffsMod1B;


typedef struct
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int satftz : 1; /* 5 */
	unsigned long long int abs_src2 : 1; /* 6 */
	unsigned long long int abs_src1 : 1; /* 7 */
	unsigned long long int neg_src2 : 1; /* 8 */
	unsigned long long int neg_src1 : 1; /* 9 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
} FrmInstBytesMod0A;


typedef struct
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int type : 3; /* 7:5 */
	unsigned long long int cop : 2; /* 9:8 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
} FrmInstBytesMod0B;


typedef struct
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int shamt : 5; /* 9:5 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
} FrmInstBytesMod0C;


typedef struct
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int sat : 1; /* 5 */
	unsigned long long int ftzfmz : 2; /* 7:6 */
	unsigned long long int fma_mod : 2; /* 9:8 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
} FrmInstBytesMod0D;


typedef enum
{
	FrmInstCategoryInvalid = 0,

	FrmInstCategoryFp,  /* Floating point instructions */
	FrmInstCategoryInt,  /* Integer instructions */
	FrmInstCategoryConv,  /* Conversion instructions */
	FrmInstCategoryMov,  /* Movement instructions */
	FrmInstCategoryPred,  /* Predicate/CC instructions */
	FrmInstCategoryTex,  /* Texture instructions */
	FrmInstCategoryLdSt, /* Compute load/store instructions */
	FrmInstCategorySurf, /* Surface memory instructions */
	FrmInstCategoryCtrl, /* Control instructions */
	FrmInstCategoryMisc, /* Miscellaneous instructions */

	FrmInstCategoryCount
} FrmInstCategory;


typedef enum
{
	FrmInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _category, _opcode) \
	FRM_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	FrmInstOpcodeCount
} FrmInstOpcode;


typedef union
{
	unsigned char bytes[8];
	unsigned int word[2];
	unsigned long long int dword;

	FrmInstBytesGeneral0 general0;
	FrmInstBytesGeneral1 general1;
	FrmInstBytesImm imm;
	FrmInstBytesOffs offs;
	FrmInstBytesTgt tgt;
	FrmInstBytesGeneral0Mod1A general0_mod1_A;
	FrmInstBytesGeneral0Mod1B general0_mod1_B;
	FrmInstBytesGeneral0Mod1C general0_mod1_C;
	FrmInstBytesGeneral0Mod1D general0_mod1_D;
	FrmInstBytesOffsMod1A offs_mod1_A;
	FrmInstBytesOffsMod1B offs_mod1_B;
	FrmInstBytesMod0A mod0_A;
	FrmInstBytesMod0B mod0_B;
	FrmInstBytesMod0C mod0_C;
	FrmInstBytesMod0D mod0_D;
} FrmInstBytes;


typedef struct
{
	FrmInstOpcode opcode;
	FrmInstCategory category;

	char *name;
	char *fmt_str;

	/* Field of the instruction containing the instruction opcode
	 * identifier. */
	unsigned int op;

	int size;

} FrmInstInfo;


CLASS_BEGIN(FrmInst, Object)

	/* Disassembler */
	FrmAsm *as;

	/* Instruction virtual address, for branch decoding purposes */
	unsigned int addr;

	/* Instruction bytes */
	FrmInstBytes bytes;

	/* Decoded information */
	FrmInstInfo *info;

CLASS_END(FrmInst)


void FrmInstCreate(FrmInst *self, FrmAsm *as);
void FrmInstDestroy(FrmInst *self);

void FrmInstDumpHex(FrmInst *self, FILE *f);
void FrmInstDumpBuf(FrmInst *self, char *str, int size);
void FrmInstDump(FrmInst *self, FILE *f);

/* From the buffer given in 'ptr', decode an instruction and populate fields
 * 'bytes' and 'info' of the instruction object. Argument 'addr' passes the
 * virtual address of the instruction for branch decoding purposes, and it is
 * copied to the 'addr' field of the instruction. */
void FrmInstDecode(FrmInst *self, unsigned int addr, void *ptr);


#endif

