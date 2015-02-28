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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_ISA_H
#define ARCH_SOUTHERN_ISLANDS_EMU_ISA_H

#include <arch/southern-islands/asm/Wrapper.h>
#include <lib/class/class.h>


enum si_isa_write_task_kind_t
{
	SI_ISA_WRITE_TASK_NONE = 0,
	SI_ISA_WRITE_TASK_WRITE_LDS,
	SI_ISA_WRITE_TASK_WRITE_DEST,
	SI_ISA_WRITE_TASK_PUSH_BEFORE,
	SI_ISA_WRITE_TASK_SET_PRED
};


struct si_isa_write_task_t
{
	/* All */
	enum si_isa_write_task_kind_t kind;
	SIInst *inst;

	/* When 'kind' == SI_ISA_WRITE_TASK_WRITE_DEST */
	int gpr, rel, chan, index_mode, write_mask;
	unsigned int value;

	/* When 'kind' == SI_ISA_WRITE_TASK_WRITE_LDS */
	unsigned int lds_addr;
	unsigned int lds_value;
	size_t   lds_value_size;

	/* When 'kind' == GPU_ISA_WRITE_TASK_PRED_SET */
	int cond;
};

/* Functions to handle deferred tasks */
void si_isa_enqueue_write_lds(unsigned int addr, unsigned int value, size_t value_size);
void si_isa_enqueue_write_dest(unsigned int value);
void si_isa_enqueue_write_dest_float(float value);
void si_isa_enqueue_push_before(void);
void si_isa_enqueue_pred_set(int cond);
void si_isa_write_task_commit(void);

/* Macros for special registers */
#define SI_M0 124
#define SI_VCC 106
#define SI_VCCZ 251
#define SI_EXEC 126
#define SI_EXECZ 252
#define SI_SCC 253


/* Debugging */
#define si_isa_debug(...) debug(si_isa_debug_category, __VA_ARGS__)
extern int si_isa_debug_category;


/* Macros for unsupported parameters */
extern char *err_si_isa_note;

#define SI_ISA_ARG_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, si_isa_inst->info->name, (p), err_si_isa_note);
#define SI_ISA_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, si_isa_inst->info->name, (v), err_si_isa_note); }
#define SI_ISA_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, si_isa_inst->info->name, (min), (max), err_si_opencl_param_note); }


/* Macros for fast access of instruction words */
#define SI_INST_SMRD		SIInstWrapGetBytes(inst)->smrd
#define SI_INST_SOPP		SIInstWrapGetBytes(inst)->sopp
#define SI_INST_SOPK		SIInstWrapGetBytes(inst)->sopk
#define SI_INST_SOPC		SIInstWrapGetBytes(inst)->sopc
#define SI_INST_SOP1		SIInstWrapGetBytes(inst)->sop1
#define SI_INST_SOP2		SIInstWrapGetBytes(inst)->sop2
#define SI_INST_VOP1		SIInstWrapGetBytes(inst)->vop1
#define SI_INST_VOP2		SIInstWrapGetBytes(inst)->vop2
#define SI_INST_VOPC		SIInstWrapGetBytes(inst)->vopc
#define SI_INST_VOP3b		SIInstWrapGetBytes(inst)->vop3b
#define SI_INST_VOP3a		SIInstWrapGetBytes(inst)->vop3a
#define SI_INST_VINTRP		SIInstWrapGetBytes(inst)->vintrp
#define SI_INST_DS		SIInstWrapGetBytes(inst)->ds
#define SI_INST_MTBUF		SIInstWrapGetBytes(inst)->mtbuf
#define SI_INST_MUBUF		SIInstWrapGetBytes(inst)->mubuf
#define SI_INST_EXP		SIInstWrapGetBytes(inst)->exp


/* Forward declaration */
union SIInstReg;

/* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
typedef void (*si_isa_inst_func_t)(SIWorkItem *work_item, struct SIInstWrap *inst);
extern si_isa_inst_func_t si_isa_inst_func[];

int si_isa_get_num_elems(int data_format);
int si_isa_get_elem_size(int data_format);

#endif
