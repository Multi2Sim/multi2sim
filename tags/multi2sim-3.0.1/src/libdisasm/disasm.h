/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#ifndef DISASM_H
#define DISASM_H

#include <stdio.h>
#include <stdint.h>
#include <mhandle.h>


/* List of opcodes */
typedef enum {
	op_none = 0,
#define DEFINST(name,op1,op2,op3,imm,prefixes) op_##name,
#include "machine.dat"
#undef DEFINST
	x86_opcode_count
} x86_opcode_t;


/* Prefixes */
typedef enum x86_prefix_t {
	prefix_none = 0x00,
	prefix_rep = 0x01,
	prefix_repz = 0x01,  /* same value as rep */
	prefix_repnz = 0x02,
	prefix_lock = 0x04,
	prefix_addr = 0x08,  /* address-size override */
	prefix_op = 0x10  /* operand-size override */
} x86_prefix_t;


/* Register identifiers
 * and names. */
typedef enum {
	reg_none = 0,
	reg_eax, reg_ecx, reg_edx, reg_ebx, reg_esp, reg_ebp, reg_esi, reg_edi,
	reg_ax, reg_cx, reg_dx, reg_bx, reg_sp, reg_bp, reg_si, reg_di,
	reg_al, reg_cl, reg_dl, reg_bl, reg_ah, reg_ch, reg_dh, reg_bh,
	reg_es, reg_cs, reg_ss, reg_ds, reg_fs, reg_gs,
	x86_register_count
} x86_register_t;
extern char *x86_register_name[x86_register_count];


/* Names of flags */
typedef enum {
	flag_cf = 0,
	flag_pf = 2,
	flag_af = 4,
	flag_zf = 6,
	flag_sf = 7,
	flag_df = 10,
	flag_of = 11
} x86_flag_t;


/* x86 Instruction */
typedef struct {
	uint32_t eip;  /* position inside the code */
	int size;  /* number of instruction bytes */
	x86_opcode_t opcode;
	char *format;  /* format of the instruction */
	
	/* Size of fields */
	int prefix_size;
	int opcode_size;
	int modrm_size;
	int sib_size;
	int disp_size;
	int imm_size;

	/* Index in the opcode. This is a value between 0 and 7, which
	 * is present in some instructions at op1 or op2. */
	int opindex;

	/* Prefixes */
	x86_register_t segment;  /* Reg. used to override segment */
	x86_prefix_t prefixes;  /* Prefixes ORed */
	int op_size;  /* Operand size: 2 or 4, default 4 */
	int addr_size;  /* Address size: 2 or 4, default 4 */
	int rep;  /* Number of iteration for string inst (updated by user) */
	
	/* ModR/M Field */
	unsigned char modrm;  /* ModR/M field */
	unsigned char modrm_mod;  /* mod field of ModR/M */
	unsigned char modrm_reg;  /* reg field of ModR/M */
	unsigned char modrm_rm;  /* rm field of ModR/M */

	/* SIB Field */
	unsigned char sib;  /* SIB field */
	unsigned char sib_scale;  /* scale field of SIB */
	unsigned char sib_index;  /* index field of SIB */
	unsigned char sib_base;  /* base field of SIB */

	/* Displacement and Immediate */
	int32_t disp;
	union {
		uint8_t b;
		uint16_t w;
		uint32_t d;
	} imm;

	/* Effective address */
	x86_register_t ea_base;
	x86_register_t ea_index;
	uint32_t ea_scale;

	/* Register */
	int reg;  /* same as modrm_reg */
} x86_inst_t;


/* Initialization and finalization routines */
void disasm_init(void);
void disasm_done(void);


/* Disassemble and dump */
void x86_disasm(void *buf, uint32_t eip, volatile x86_inst_t *inst);
void x86_inst_dump_buf(x86_inst_t *inst, char *buf, int size);
void x86_inst_dump(x86_inst_t *inst, FILE *f);
char *x86_inst_name(x86_opcode_t opcode);


#endif

