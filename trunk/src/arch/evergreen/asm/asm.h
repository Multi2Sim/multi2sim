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

#ifndef EVERGREEN_ASM_ASM_H
#define EVERGREEN_ASM_ASM_H

#include <stdio.h>

#include <arch/common/asm.h>
#include <lib/util/elf-format.h>

#include "inst.h"





/*
 * String maps
 */

/* Table containing information of all instructions */
extern EvgInstInfo evg_inst_info[EvgInstOpcodeCount];


typedef void (*evg_fmt_dump_func_t)(void *buf, FILE *);

void evg_disasm_init(void);
void evg_disasm_done(void);
void evg_disasm_buffer(struct elf_buffer_t *buffer, FILE *f);

void evg_inst_slot_dump_buf(EvgInst *inst, int count, int loop_idx, int slot, char *buf, int size);
void evg_inst_dump_buf(EvgInst *inst, int count, int loop_idx, char *buf, int size);

void evg_inst_dump_gpr(int gpr, int rel, int chan, int im, FILE *f);
void evg_inst_slot_dump(EvgInst *inst, int count, int loop_idx, int slot, FILE *f);
void evg_inst_dump(EvgInst *inst, int count, int loop_idx, FILE *f);
void evg_inst_dump_debug(EvgInst *inst, int count, int loop_idx, FILE *f);
void evg_alu_group_dump(EvgALUGroup *group, int shift, FILE *f);
void evg_alu_group_dump_buf(EvgALUGroup *alu_group, char *buf, int size);
void evg_alu_group_dump_debug(EvgALUGroup *alu_group, int count, int loop_idx, FILE *f);

/* Copy instruction */
void evg_inst_copy(EvgInst *dest, EvgInst *src);
void evg_alu_group_copy(EvgALUGroup *dest, EvgALUGroup *src);

/* Obtaining source operand fields for ALU instructions */
void evg_inst_get_op_src(EvgInst *inst, int src_idx,
	int *sel, int *rel, int *chan, int *neg, int *abs);

/* Decode */
void *evg_inst_decode_cf(void *buf, EvgInst *inst);
void *evg_inst_decode_alu(void *buf, EvgInst *inst);
void *evg_inst_decode_alu_group(void *buf, int group_id, EvgALUGroup *group);
void *evg_inst_decode_tc(void *buf, EvgInst *inst);



/*
 * Class 'EvgAsm'
 */

CLASS_BEGIN(EvgAsm, Asm)

CLASS_END(EvgAsm)

void EvgAsmCreate(EvgAsm *self);
void EvgAsmDestroy(EvgAsm *self);

#endif

