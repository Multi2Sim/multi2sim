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

#ifndef MIPS_ASM_H
#define MIPS_ASM_H

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* 
 * Mips Disassembler
 */

#define mips_reg_count 32

void mips_asm_init();
void mips_asm_done();
void mips_emu_disasm(char *path);

/* 
 * Structure of Instruction Format
 */

struct mips_fmt_standard_t
{
	unsigned int function     :6;	/* [5:0] */
	unsigned int sa           :5;	/* [10:6] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};

struct mips_fmt_target_t
{
	unsigned int target       :26;	/* [25:0] */
	unsigned int opc          :6;	/* [31:26] */

};

struct mips_fmt_offset_imm_t
{
	unsigned int offset       :16;	/* [15:0] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int base         :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};

struct mips_fmt_cc_t
{
	unsigned int offsetbr    :16;	/* [15:0] */
	unsigned int tf           :1;	/* [16] */
	unsigned int nd           :1;	/* [17] */
	unsigned int cc           :3;	/* [20:18] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};
struct mips_fmt_code_t
{
	unsigned int function     :6;	/* [5:0] */
	unsigned int code         :10;	/* [15:6] */
	unsigned int rs_rt        :10;	/* [25:16] */
	unsigned int opc          :6;	/* [31:26] */

};
struct mips_fmt_sel_t
{
	unsigned int sel          :3;	/* [2:0] */
	unsigned int impl         :8;	/* [10:3] */
	unsigned int rd           :5;	/* [15:11] */
	unsigned int rt           :5;	/* [20:16] */
	unsigned int rs           :5;	/* [25:21] */
	unsigned int opc          :6;	/* [31:26] */

};

union mips_inst_dword_t
{
	unsigned int bytes;

	struct mips_fmt_standard_t standard;
	struct mips_fmt_target_t target;
	struct mips_fmt_offset_imm_t offset_imm;
	struct mips_fmt_cc_t cc;
	struct mips_fmt_code_t code;
	struct mips_fmt_sel_t sel;

};

enum mips_inst_enum
{
	MIPS_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3)	\
	MIPS_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	MIPS_INST_COUNT
};

struct mips_inst_info_t
{
	enum mips_inst_enum inst;
	char *name;
	char *fmt_str;
	unsigned int opcode;
	int size;
	int next_table_low;
	int next_table_high;
	struct mips_inst_info_t *next_table;

};

struct mips_inst_t
{
	unsigned int addr;
	union mips_inst_dword_t dword;
	struct mips_inst_info_t *info;

};

void mips_inst_decode(struct mips_inst_t *inst);
void mips_disasm(unsigned int buf, unsigned int ip, volatile struct mips_inst_t *inst);
void mips_inst_debug_dump(struct mips_inst_t *inst, FILE *f);
void mips_inst_hex_dump(FILE *f, void *inst_ptr, unsigned int inst_addr);

void mips_inst_dump(FILE *f, char *str, int inst_str_size, void *inst_ptr,
	unsigned int inst_index, unsigned int inst_addr,
	unsigned int *print_symbol_address);

void mips_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_RS(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_RT(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_SA(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_TARGET(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst, unsigned int *print_symbol_address);

void mips_inst_dump_OFFSET(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);
void mips_inst_dump_OFFSETBR(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst, unsigned int inst_addr,
	unsigned int *print_symbol_address);
void mips_inst_dump_IMM(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_IMMHEX(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_BASE(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_SEL(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_CC(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_POS(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_SIZE(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_FS(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_FT(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_FD(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);

void mips_inst_dump_CODE(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst);


#endif /* MIPS_ASM_H */
