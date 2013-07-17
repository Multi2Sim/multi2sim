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

#ifndef ARCH_KEPLER_ASM_ASM_H
#define ARCH_KEPLER_ASM_ASM_H

#include <stdio.h>

/* 1st level struct */


#if 0
struct kpl_fmt_general0_t // IADD, IMUL, DADD  
{
	unsigned long long int scr0 : 5; /* 4:0 */
	unsigned long long int src0 : 5; /* 9:5 */
	unsigned long long int mod0 : 12; /* 21:10 */
	unsigned long long int op0 : 9; /* 30:22 */
	unsigned long long int src0_mod : 1; /* 31 */
	unsigned long long int op1 : 2; /* 33:32 */
	unsigned long long int dst : 8; /* 41:34 */
	unsigned long long int src1 : 8; /* 49:42 */
	unsigned long long int pred : 4; /* 53:50 */
	unsigned long long int mod1 : 1; /* 54 */
	unsigned long long int src1 : 9; /* 63:55 */	
};


struct kpl_fmt_general1_t // LD, ST
{
	unsigned long long int offset0 : 22; /* 21:0 */
	unsigned long long int mod0 : 7; /* 28:22 */
	unsigned long long int op : 5; /* 33:29 */
	unsigned long long int src : 8; /* 41:34 */
	unsigned long long int dst : 8; /* 49:42 */	
	unsigned long long int pred : 4; /* 53:50 */
	unsigned long long int mod1 : 1; /* 54 */
	unsigned long long int offset1 : 9; /* 63:55 */
};

struct kpl_fmt_general2_t /* EXIT */
{    
	unsigned long long int reserved0 : 23; /* 22:0 */
	unsigned long long int offset0 : 11; /* 33:23 */
	unsigned long long int mod0 : 5; /* 38:34 */
	unsigned long long int mod1 : 1; /* 39 */
	unsigned long long int pred : 4; /* 53:50 */
	unsigned long long int reserved1 : 10; /* 63:54 */
};

struct kpl_fmt_general3_t /* S2R */
{
	unsigned long long int reserved0 : 22; /* 21:0 */
	unsigned long long int op : 12; /* 33:22 */
	unsigned long long int dst : 8; /* 41:34 */
	unsigned long long int reserved1 : 8; /* 49:42 */
	unsigned long long int pred : 4; /* 53:50 */
	unsigned long long int mod : 1; /* 54 */
	unsigned long long int src : 8; /* 62:55 */
	unsigned long long int reserved2 : 1; /* 63 */
};

struct kpl_fmt_general4_t /* ISETP */
{
	unsigned long long int src : 10; /* 9:0 */
	unsigned long long int pred3 : 4; /* 13:10 */
	unsigned long long int mod : 9; /* 22:14 */
	unsigned long long int op : 11; /* 33:23 */
	unsigned long long int pred2 : 3; /* 36:34 */
	unsigned long long int pred1 : 3; /* 39:37 */
	unsigned long long int reserved : 2; /* 41:40 */
	unsigned long long int dst : 8; /* 49:42 */
	unsigned long long int pred0 : 4; /* 53:50 */
	unsigned long long int mod : 1; /* 54 */
	unsigned long long int src : 9; /* 63:55 */
};

struct kpl_fmt_general5_t /* BRA */
{
	unsigned long long int tgt : 14; /* 13:0 */
	unsigned long long int mod : 1; /* 14 */
	unsigned long long int reserved : 8; /* 22:15 */
	unsigned long long int op : 11; /* 33:23 */
	unsigned long long int mod : 5; /* 38:34 */
	unsigned long long int tgt_mod : 1; /* 39 */
	unsigned long long int mod : 2; /* 41:40 */
	unsigned long long int reserved : 8; /* 49:42 */
	unsigned long long int pred0 : 4; /* 53:50 */
	unsigned long long int reserved : 1; /* 54 */
	unsigned long long int tgt : 9; /* 63:55 */
};

/* 2nd level struct */

struct kpl_fmt_general0_mod1_A_t /* IADD */
{
	unsigned long long int _reserved0 : 10; /* 9:0 */
	unsigned long long int reserved2 : 4; /* 13:10 */
	unsigned long long int x : 1; /* 14 */
	unsigned long long int reserved3 : 3; /* 17:15 */
	unsigned long long int cc : 1; /* 18 */
	unsigned long long int mod : 2; /* 20:19 */
	unsigned long long int sat : 1; /* 21 */
	unsigned long long int _reserved1 : 42; /* 63:22 */
};

struct kpl_fmt_general0_mod1_B_t /* IMUL */
{
	unsigned long long int _reserved0 : 10; /* 9:0 */
	unsigned long long int HI : 1; /* 10 */
	unsigned long long int mod : 2; /* 12:11 */
	unsigned long long int reserved2 : 5; /* 17:13 */
	unsigned long long int cc : 1; /* 18 */
	unsigned long long int reserved3: 3; /* 21:19 */
	unsigned long long int _reserved1 : 42; /* 63:22 */
};

struct kpl_fmt_general0_mod1_C_t /* DADD */
{
	unsigned long long int _reserved0 : 10; /* 9:0 */
	unsigned long long int mod0 : 2; /* 11:10 */
	unsigned long long int reserved2 : 4; /* 15:12 */
	unsigned long long int mod1 : 2; /* 17:16 */
	unsigned long long int cc : 1; /* 18 */
	unsigned long long int mod2 : 2; /* 20:19 */
	unsigned long long int reserved3 : 1; /* 21 */
	unsigned long long int _reserved1 : 42; /* 63:22 */
};


struct kpl_fmt_general1_mod1_A_t /* LD, ST */
{
	unsigned long long int _reserved0 : 22; /* 21:0 */
	unsigned long long int mod0 : 1; /* 22 */
	unsigned long long int E : 1; /* 23 */
	unsigned long long int mod1 : 3; /* 26:24 */
	unsigned long long int mod2 : 2; /* 28:27 */
	unsigned long long int _reserved1 : 35; /* 63:29 */

};

#endif



union kpl_inst_dword_t
{
        unsigned int bytes;


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
void kpl_disasm(char *file_name);

/* Disassemble the Kepler ISA contained in 'buf' of 'size' bytes. */
void kpl_disasm_buffer(void *buf, int size);

/* Decode the instruction in inst->dword.byte. The instruction information is
 * populated in inst->info. If the bytes represent an invalid instruction,
 * inst->info is set to NULL. */
void kpl_inst_decode(struct kpl_inst_t *inst);


#endif

