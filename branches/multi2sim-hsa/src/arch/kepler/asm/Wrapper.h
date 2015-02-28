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

#ifndef ARCH_KEPLER_ASM_WRAPPER_H
#define ARCH_KEPLER_ASM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

struct KplInstWrap;
struct KplAsm;


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////


// 1st level struct
/* BFE, POPC, IADD, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD,
   FMNMX, FMUL, DADD, SEL, P2R, RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK */
typedef struct
{
	unsigned long long int op0 	: 2; 	// 1:0
	unsigned long long int dst 	: 8; 	// 9:2
	unsigned long long int mod0 	: 8; 	// 17:10
	unsigned long long int pred 	: 4; 	// 21:18
	unsigned long long int s 	: 1; 	// 22
	unsigned long long int srcB 	: 9; 	// 41:23
	unsigned long long int mod1 	: 12;	// 53:42
	unsigned long long int op1 	: 9; 	// 62:54
	unsigned long long int srcB_mod	: 1; 	// 63
} KplInstBytesGeneral0;


// BRA, JMX, JMP, JCAL, BRX, CAL, PRET, PLONGJMP, SSY, PBK
typedef struct
{
	unsigned long long int op0 	: 2; 	// 1:0
	unsigned long long int mod0 	: 16;	// 17:2
	unsigned long long int pred 	: 4; 	// 21:18
	unsigned long long int unused 	: 1; 	// 22
	unsigned long long int srcB 	: 21; 	// 43:23
	unsigned long long int mod1 	: 11;	// 54:44
	unsigned long long int op1 	: 9; 	// 63:55
} KplInstBytesGeneral1;


/* GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETLMEMBASE, EXIT, LONGJUMP, RET, KIL,
 * BRK, CONT, RTT, SAM, RAM  */
typedef struct
{
	unsigned long long int op0     	: 2; 	// 1:0
	unsigned long long int mod     	: 8; 	// 9:2
	unsigned long long int src     	: 8; 	// 17:10
	unsigned long long int pred    	: 4; 	// 21:18
	unsigned long long int unused   : 33; 	// 54:22
	unsigned long long int op1     	: 9; 	// 63:55
} KplInstBytesGeneral2;


typedef union
{
	unsigned char as_uchar[8];
	unsigned int as_uint[2];
	unsigned long long as_dword;

	KplInstBytesGeneral0 general0;
	KplInstBytesGeneral1 general1;
	KplInstBytesGeneral2 general2;

} KplInstBytes;


typedef enum
{
	KplInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, ...)        \
	INST_##_name,
#include "Inst.def"
#undef DEFINST

	// Max
	KplInstOpcodeCount
} KplInstOpcode;


typedef struct
{
	KplInstOpcode opcode;
	const char *name;
	const char *fmt_str;
} KplInstInfo;


struct KplInstWrap *KplInstWrapCreate();
void KplInstWrapFree(struct KplInstWrap *self);
void KplInstWrapDecode(struct KplInstWrap *self, unsigned int addr, void *ptr);
//KplInstId KplInstWrapGetId(struct KplInstWrap *self);
KplInstOpcode KplInstWrapGetOpcode(struct KplInstWrap *self);


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////


struct KplAsm *KplAsmCreate();
void KplAsmFree(struct KplAsm *as);
void KplAsmDisassembleBinary(struct KplAsm *as, const char *path);




#ifdef __cplusplus
}
#endif


#endif  // ARCH_KEPLER_ASM_WRAPPER_H

