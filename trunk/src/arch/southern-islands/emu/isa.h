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

#ifndef SOUTHERN_ISLANDS_ISA_H
#define SOUTHERN_ISLANDS_ISA_H

#include <arch/southern-islands/asm/asm.h>
#include <arch/southern-islands/emu/emu.h>

/* Macros for special registers */
#define SI_M0 124
#define SI_VCC 106
#define SI_VCCZ 251
#define SI_EXEC 126
#define SI_EXECZ 252
#define SI_SCC 253


/* Debugging */
#define si_isa_debugging() debug_status(si_isa_debug_category)
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
#define SI_INST_SMRD		inst->micro_inst.smrd
#define SI_INST_SOPP		inst->micro_inst.sopp
#define SI_INST_SOPK		inst->micro_inst.sopk
#define SI_INST_SOPC		inst->micro_inst.sopc
#define SI_INST_SOP1		inst->micro_inst.sop1
#define SI_INST_SOP2		inst->micro_inst.sop2
#define SI_INST_VOP1		inst->micro_inst.vop1
#define SI_INST_VOP2		inst->micro_inst.vop2
#define SI_INST_VOPC		inst->micro_inst.vopc
#define SI_INST_VOP3b		inst->micro_inst.vop3b
#define SI_INST_VOP3a		inst->micro_inst.vop3a
#define SI_INST_DS			inst->micro_inst.ds
#define SI_INST_MTBUF		inst->micro_inst.mtbuf
/* FIXME Finish filling these in */


/* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
typedef void (*si_isa_inst_func_t)(struct si_work_item_t *work_item, struct si_inst_t *inst);
extern si_isa_inst_func_t *si_isa_inst_func;

union si_reg_t si_isa_read_sreg(struct si_work_item_t *work_item, int sreg);
void si_isa_write_sreg(struct si_work_item_t *work_item, int sreg, union si_reg_t value);
union si_reg_t si_isa_read_vreg(struct si_work_item_t *work_item, int vreg);
void si_isa_write_vreg(struct si_work_item_t *work_item, int vreg, union si_reg_t value);
union si_reg_t si_isa_read_reg(struct si_work_item_t *work_item, int reg);
void si_isa_bitmask_sreg(struct si_work_item_t *work_item, int sreg, union si_reg_t value);
int si_isa_read_bitmask_sreg(struct si_work_item_t *work_item, int sreg);

void si_isa_read_buf_res(struct si_work_item_t *work_item, struct si_buffer_resource_t *buf_desc, int sreg);
void si_isa_read_mem_ptr(struct si_work_item_t *work_item, struct si_mem_ptr_t *mem_ptr, int sreg);
int si_isa_get_num_elems(int data_format);
int si_isa_get_elem_size(int data_format);

#endif
