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

#define ARCH_KEPLER_ASM_ASM_H
#define ARCH_KEPLER_ASM_ASM_H

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * Kepler Disassembler
 */

#define kpl_reg_count 255

void kpl_asm_init();
void kpl_asm_done();
void kpl_emu_disasm(char *path);

/* 
 * Structure of Instruction Format
 */

/* 1st level struct */

struct kpl_fmt_general0_t 		/* BFE, POPC, IADD, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD,
					FMNMX, FMUL, DADD, SEL, P2R, RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK */  
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
};


struct kpl_fmt_general1_t 		/* BRA, JMX, JMP, JCAL, BRX, CAL, PRET, PLONGJMP, SSY, PBK */
{
	unsigned long long int op0 	: 2; 	/* 1:0 */
	unsigned long long int mod0 	: 16;	/* 17:2 */
	unsigned long long int pred 	: 4; 	/* 21:18 */
	unsigned long long int unused 	: 1; 	/* 22 */
	unsigned long long int srcB 	: 21; 	/* 43:23 */
	unsigned long long int mod1 	: 11;	/* 54:44 */
	unsigned long long int op1 	: 9; 	/* 63:55 */
};

struct kpl_fmt_general2_t 		/* GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETLMEMBASE, EXIT, LONGJUMP, RET, KIL, 
					BRK, CONT, RTT, SAM, RAM  */
{    
	unsigned long long int op0     	: 2; 	/* 1:0 */
       	unsigned long long int mod     	: 8; 	/* 9:2 */
       	unsigned long long int src     	: 8; 	/* 17:10 */
       	unsigned long long int pred    	: 4; 	/* 21:18 */
       	unsigned long long int unused   : 33; 	/* 54:22 */
       	unsigned long long int op1     	: 9; 	/* 63:55 */
};

struct kpl_fmt_imm_t 			/* MOV32I, FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I  */
{
        unsigned long long int op0      : 2; 	/* 1:0 */
        unsigned long long int dst      : 8; 	/* 9:2 */
        unsigned long long int mod0     : 8; 	/* 17:10 */
        unsigned long long int pred     : 4; 	/* 21:18 */
        unsigned long long int s        : 1; 	/* 22 */
        unsigned long long int srcB     : 9; 	/* 54:23 */
        unsigned long long int mod1    	: 6; 	/* 60:55 */
        unsigned long long int op1 	: 3; 	/* 63:61 */
};


/* 2nd level struct */

struct kpl_fmt_general0_mod0_A_t 	/* BFE, POPC, IADD, IASCADD, IMNMX, SHR, IMUL, LOP, SHL, DMUL, DMNMX, FADD,
                                	FMNMX, FMUL, DADD, SEL, P2R */
{
	unsigned long long int reserved0: 10; 	/* 9:0 */
	unsigned long long int srcA 	: 8; 	/* 17:10 */
	unsigned long long int reserved1: 46;	/* 63:18 */
};

struct kpl_fmt_general0_mod0_B_t 	/* RRO, MOV, F2F, F2I, I2F, I2I, FLO, DCHK, FCHK */
{
        unsigned long long int reserved0: 10;   /* 9:0 */
        unsigned long long int mod2     : 8;    /* 17:10 */
        unsigned long long int reserved1: 46;   /* 63:18 */
};

struct kpl_fmt_general1_mod0_A_t 	/* JCAL, CAL, PRET */
{
	unsigned long long int reserved0: 2; 	/* 1:0 */
	unsigned long long int unused0	: 5; 	/* 6:2 */
	unsigned long long int srcB_mod : 1; 	/* 7 */
	unsigned long long int noinc 	: 1; 	/* 8 */
	unsigned long long int unused1	: 9; 	/* 17:9 */
	unsigned long long int reserved1: 46; 	/* 63:18 */
};

struct kpl_fmt_general1_mod0_B_t        /* BRA, JMX, JMP, BRX */
{
        unsigned long long int reserved0: 2;    /* 1:0 */
        unsigned long long int cc   	: 5;    /* 6:2 */
        unsigned long long int srcB_mod : 1;    /* 7 */
        unsigned long long int lmt    	: 1;    /* 8 */
	unsigned long long int u	: 1;	/* 9 */
        unsigned long long int srcB   	: 8;    /* 17:10 */
        unsigned long long int reserved1: 46;   /* 63:18 */
};

struct kpl_fmt_general1_mod1_A_t        /* BRA, BRX, CAL, PRET */
{
        unsigned long long int reserved0: 44;  	/* 43:0 */
        unsigned long long int srcB     : 2;    /* 45:44 */
        unsigned long long int neg_srcB	: 1;    /* 46 */
        unsigned long long int unused  	: 8;    /* 54:47 */
        unsigned long long int reserved1: 9;   	/* 63:55 */
};

struct kpl_fmt_general1_mod1_B_t        /* JMP, JCAL */
{
        unsigned long long int reserved0: 44;   /* 43:0 */
        unsigned long long int srcB   	: 11;    /* 54:44 */
        unsigned long long int reserved1: 9;    /* 63:55 */
};

struct kpl_fmt_general1_mod1_C_t        /* JMX */
{
        unsigned long long int reserved0: 44;  	/* 43:0 */
        unsigned long long int srcB     : 10;  	/* 53:44 */
	unsigned long long int neg_srcB	: 1;	/* 54 */
        unsigned long long int reserved1: 9;   	/* 63:55 */
};

struct kpl_fmt_general2_mod_A_t        	/* GETCRSPTR, GETLMEMBASE, SETCRSPTR, SETMEMLBASE */
{
        unsigned long long int reserved0: 2;   	/* 41:0 */
        unsigned long long int dst     	: 8;    /* 9:2 */
        unsigned long long int reserved1: 54;  	/* 63:10 */
};

struct kpl_fmt_general2_mod_B_t        	/* EXIT, LONGJUMP, RET, KIL, BRK, CONT, RTT, SAM, RAM */
{
        unsigned long long int reserved0: 2;  	/* 41:0 */
        unsigned long long int cc  	: 8;    /* 6:2 */
	unsigned long long int unused	: 3;	/* 9:7 */
        unsigned long long int reserved1: 54;  	/* 63:10 */
};

/* Need to figure out how to re-label constant once I know what this field represents */
struct kpl_fmt_imm_mod0_A_t        	/* MOV32I */
{
        unsigned long long int reserved0: 10;  	/* 9:0 */
        unsigned long long int unused  	: 4;    /* 13:10 */
	unsigned long long int constant	: 4;	/* 17:14 */     /* ????????????????? */
        unsigned long long int reserved1: 46;  	/* 63:18 */
};

struct kpl_fmt_imm_mod0_B_t        	/* FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I, FMUL32I, IMUL32I */
{
        unsigned long long int reserved0: 10;  	/* 9:0 */
	unsigned long long int src	: 8;	/* 17:10 */  
        unsigned long long int reserved1: 46;  	/* 63:18 */
};

struct kpl_fmt_imm_mod1_A_t		/* MOV32I */
{
	unsigned long long int reserved0: 55;	/* 54:0 */
	unsigned long long int op1	: 6;	/* 60:55 */
	unsigned long long int reserved1: 3;	/* 63:61 */
};


struct kpl_fmt_imm_mod1_B_t		/* FADD32I, LOP32I, FFMA32I, IMAD32I, ISCADD32I */
{
	unsigned long long int reserved0: 55;	/* 54:0 */
	unsigned long long int cc	: 1;	/* 55 */
	unsigned long long int mod2	: 5;	/* 60:56 */
	unsigned long long int reserved1: 3;	/* 63:61 */
};

struct kpl_fmt_imm_mod1_C_t		/* FMUL321, IMUL32I */
{
	unsigned long long int reserved0: 55;	/* 54:0 */
	unsigned long long int cc	: 1;	/* 55 */
	unsigned long long int mod2	: 3;	/* 58:56 */
	unsigned long long int op1	: 2;	/* 60:59 */
	unsigned long long int reserved1: 3;	/* 63:61 */
};




union kpl_inst_dword_t
{
        unsigned char as_uchar[8];
	unsigned int as_uint[2];
	unsigned long long as_dword;
};


enum kpl_inst_opcode_t
{
        KPL_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, ...)        \
        KPL_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
        KPL_INST_COUNT
};


struct kpl_inst_info_t
{
        enum kpl_inst_opcode_t opcode;
        char *name;
        char *fmt_str;
};


struct kpl_inst_table_entry_t
{
	/* Fields used when the table entry points to another table. */
        int next_table_low;
        int next_table_high;
        struct kpl_inst_table_entry_t *next_table;

	/* Field used when the table entry points to a final instruction */
	struct kpl_inst_info_t *info;
};


struct kpl_inst_t
{
        unsigned int addr;
        union kpl_inst_dword_t dword;
        struct kpl_inst_info_t *info;

};



/*
 * Public Functions
 */

void kpl_disasm_init(void);
void kpl_disasm_done(void);

/* Read the content of '.cubin' file in 'filename' and disassemble its sections
 * containing Kepler ISA. */
void kpl_disasm(char *path);

/* Disassemble the Kepler ISA contained in 'buf' of 'size' bytes. */
void kpl_disasm_buffer(void *buf, int size);

/* Decode the instruction in buffer 'ptr'. The instruction information is
 * populated in inst->info. If the bytes represent an invalid instruction,
 * inst->info is set to NULL. */
void kpl_inst_decode(struct kpl_inst_t *inst, void *ptr);


/* Print the address of the instruction and the instruction in hex */
void kpl_inst_hex_dump(FILE *f, void *inst_ptr, unsigned int inst_addr);

/* Instruction dump */
//void kpl_inst_dump(FILE *f, struct kpl_inst_t *inst);


void kpl_inst_dump(FILE *f, char *str, int inst_str_size, void *inst_ptr,
 	unsigned int inst_index, unsigned int inst_addr,
 	unsigned int *print_symbol_address);




