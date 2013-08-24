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

#include <lib/class/class.h>


/*
 * Class 'KplInst'
 */

typedef enum
{
	KplInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, ...)        \
	KPL_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	KplInstOpcodeCount
} KplInstOpcode;


typedef struct
{
	KplInstOpcode opcode;
	char *name;
	char *fmt_str;
} KplInstInfo;


typedef struct _KplInstTableEntry
{
	/* Fields used when the table entry points to another table. */
	int next_table_low;
	int next_table_high;
	struct _KplInstTableEntry *next_table;

	/* Field used when the table entry points to a final instruction */
	KplInstInfo *info;

} KplInstTableEntry;


/* 1st level struct */
/* BFE, POPC, IADD, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD,
   FMNMX, FMUL, DADD, SEL, P2R, RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK */
typedef struct
{
	unsigned long long int op0 	: 2; 	/* 1:0 */
	unsigned long long int dst 	: 8; 	/* 9:2 */
	unsigned long long int mod0 	: 8; 	/* 17:10 */
	unsigned long long int pred 	: 4; 	/* 21:18 */
	unsigned long long int s 	: 1; 	/* 22 */
	unsigned long long int srcB 	: 9; 	/* 41:23 */
	unsigned long long int mod1 	: 12;	/* 53:42 */
	unsigned long long int op1 	: 9; 	/* 62:54 */
	unsigned long long int srcB_mod	: 1; 	/* 63 */
} KplInstBytesGeneral0;


/* BRA, JMX, JMP, JCAL, BRX, CAL, PRET, PLONGJMP, SSY, PBK */
typedef struct
{
	unsigned long long int op0 	: 2; 	/* 1:0 */
	unsigned long long int mod0 	: 16;	/* 17:2 */
	unsigned long long int pred 	: 4; 	/* 21:18 */
	unsigned long long int unused 	: 1; 	/* 22 */
	unsigned long long int srcB 	: 21; 	/* 43:23 */
	unsigned long long int mod1 	: 11;	/* 54:44 */
	unsigned long long int op1 	: 9; 	/* 63:55 */
} KplInstBytesGeneral1;


/* GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETLMEMBASE, EXIT, LONGJUMP, RET, KIL,
 * BRK, CONT, RTT, SAM, RAM  */
typedef struct
{
	unsigned long long int op0     	: 2; 	/* 1:0 */
	unsigned long long int mod     	: 8; 	/* 9:2 */
	unsigned long long int src     	: 8; 	/* 17:10 */
	unsigned long long int pred    	: 4; 	/* 21:18 */
	unsigned long long int unused   : 33; 	/* 54:22 */
	unsigned long long int op1     	: 9; 	/* 63:55 */
} KplInstBytesGeneral2;


/* MOV32I, FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I  */
typedef struct
{
	unsigned long long int op0      : 2; 	/* 1:0 */
	unsigned long long int dst      : 8; 	/* 9:2 */
	unsigned long long int mod0     : 8; 	/* 17:10 */
	unsigned long long int pred     : 4; 	/* 21:18 */
	unsigned long long int s        : 1; 	/* 22 */
	unsigned long long int srcB     : 9; 	/* 54:23 */
	unsigned long long int mod1    	: 6; 	/* 60:55 */
	unsigned long long int op1 	: 3; 	/* 63:61 */
} KplInstBytesImm;


/* 2nd level struct */

/* BFE, POPC, IADD, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD,
 * FMNMX, FMUL, DADD, SEL, P2R */
typedef struct
{
	unsigned long long int reserved0: 10; 	/* 9:0 */
	unsigned long long int srcA 	: 8; 	/* 17:10 */
	unsigned long long int reserved1: 46;	/* 63:18 */
} KplInstBytesGeneral0Mod0A;


/* RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK */
typedef struct
{
	unsigned long long int reserved0: 10;   /* 9:0 */
	unsigned long long int mod2     : 8;    /* 17:10 */
	unsigned long long int reserved1: 46;   /* 63:18 */
} KplInstBytesGeneral0Mod0B;


/* JCAL, CAL, PRET */
typedef struct
{
	unsigned long long int reserved0: 2; 	/* 1:0 */
	unsigned long long int unused0	: 5; 	/* 6:2 */
	unsigned long long int srcB_mod : 1; 	/* 7 */
	unsigned long long int noinc 	: 1; 	/* 8 */
	unsigned long long int unused1	: 9; 	/* 17:9 */
	unsigned long long int reserved1: 46; 	/* 63:18 */
} KplInstBytesGeneral1Mod0A;


/* BRA, JMX, JMP, BRX */
typedef struct
{
	unsigned long long int reserved0: 2;    /* 1:0 */
	unsigned long long int cc   	: 5;    /* 6:2 */
	unsigned long long int srcB_mod : 1;    /* 7 */
	unsigned long long int lmt    	: 1;    /* 8 */
	unsigned long long int u	: 1;	/* 9 */
	unsigned long long int srcB   	: 8;    /* 17:10 */
	unsigned long long int reserved1: 46;   /* 63:18 */
} KplInstBytesGeneral1Mod0B;


/* BRA, BRX, CAL, PRET */
typedef struct
{
	unsigned long long int reserved0: 44;  	/* 43:0 */
	unsigned long long int srcB     : 2;    /* 45:44 */
	unsigned long long int neg_srcB	: 1;    /* 46 */
	unsigned long long int unused  	: 8;    /* 54:47 */
	unsigned long long int reserved1: 9;   	/* 63:55 */
} KplInstBytesGeneral1Mod1A;


/* JMP, JCAL */
typedef struct
{
	unsigned long long int reserved0: 44;   /* 43:0 */
	unsigned long long int srcB   	: 11;    /* 54:44 */
	unsigned long long int reserved1: 9;    /* 63:55 */
} KplInstBytesGeneral1Mod1B;


/* JMX */
typedef struct
{
	unsigned long long int reserved0: 44;  	/* 43:0 */
	unsigned long long int srcB     : 10;  	/* 53:44 */
	unsigned long long int neg_srcB	: 1;	/* 54 */
	unsigned long long int reserved1: 9;   	/* 63:55 */
} KplInstBytesGeneral1Mod1C;


/* GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETMEMLBASE */
typedef struct
{
	unsigned long long int reserved0: 2;   	/* 41:0 */
	unsigned long long int dst     	: 8;    /* 9:2 */
	unsigned long long int reserved1: 54;  	/* 63:10 */
} KplInstBytesGeneral2ModA;


/* EXIT, LONGJUMP, RET, KIL, BRK, CONT, RTT, SAM, RAM */
typedef struct
{
	unsigned long long int reserved0: 2;  	/* 41:0 */
	unsigned long long int cc  	: 8;    /* 6:2 */
	unsigned long long int unused	: 3;	/* 9:7 */
	unsigned long long int reserved1: 54;  	/* 63:10 */
} KplInstBytesGeneral2ModB;


/* Need to figure out how to re-label constant once I know what this field represents */
/* MOV32I */
typedef struct
{
	unsigned long long int reserved0: 10;  	/* 9:0 */
	unsigned long long int unused  	: 4;    /* 13:10 */
	unsigned long long int constant	: 4;	/* 17:14 */     /* ????????????????? */
	unsigned long long int reserved1: 46;  	/* 63:18 */
} KplInstBytesImmMod0A;


/* FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I */
typedef struct
{
	unsigned long long int reserved0: 10;  	/* 9:0 */
	unsigned long long int src	: 8;	/* 17:10 */
	unsigned long long int reserved1: 46;  	/* 63:18 */
} KplInstBytesImmMod0B;


/* MOV32I */
typedef struct
{
	unsigned long long int reserved0: 55;	/* 54:0 */
	unsigned long long int op1	: 6;	/* 60:55 */
	unsigned long long int reserved1: 3;	/* 63:61 */
} KplInstBytesImmMod1A;


/* FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I */
typedef struct
{
	unsigned long long int reserved0: 55;	/* 54:0 */
	unsigned long long int cc	: 1;	/* 55 */
	unsigned long long int mod2	: 5;	/* 60:56 */
	unsigned long long int reserved1: 3;	/* 63:61 */
} KplInstBytesImmMod1B;


/* FMUL321, IMUL32I */
typedef struct
{
	unsigned long long int reserved0: 55;	/* 54:0 */
	unsigned long long int cc	: 1;	/* 55 */
	unsigned long long int mod2	: 3;	/* 58:56 */
	unsigned long long int op1	: 2;	/* 60:59 */
	unsigned long long int reserved1: 3;	/* 63:61 */
} KplInstBytesImmMod1C;


typedef union
{
	unsigned char as_uchar[8];
	unsigned int as_uint[2];
	unsigned long long as_dword;

	KplInstBytesGeneral0 general0;
	KplInstBytesGeneral1 general1;
	KplInstBytesGeneral2 general2;

} KplInstBytes;


CLASS_BEGIN(KplInst, Object)

	/* Disassembler */
	KplAsm *as;

	/* Instruction offset */
	unsigned int addr;

	/* Instruction bytes */
	KplInstBytes bytes;

	/* Decoded information */
	KplInstInfo *info;

CLASS_END(KplInst)


void KplInstCreate(KplInst *self, KplAsm *as);
void KplInstDestroy(KplInst *self);

/* Decode instruction from 'buf' and populate its fields. The address given in
 * 'addr' is used for branch decoding purposes. */
void KplInstDecode(KplInst *self, void *buf, unsigned int addr);

void KplInstDumpHex(KplInst *self, FILE *f);
void KplInstDumpBuf(KplInst *self, char *buf, int size);
void KplInstDump(KplInst *self, FILE *f);

#endif

