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

#ifndef ARCH_EVERGREEN_EMU_ISA_H
#define ARCH_EVERGREEN_EMU_ISA_H

#include "wavefront.h"


enum evg_isa_write_task_kind_t
{
	EVG_ISA_WRITE_TASK_NONE = 0,
	EVG_ISA_WRITE_TASK_WRITE_LDS,
	EVG_ISA_WRITE_TASK_WRITE_DEST,
	EVG_ISA_WRITE_TASK_PUSH_BEFORE,
	EVG_ISA_WRITE_TASK_SET_PRED
};


struct evg_isa_write_task_t
{
	/* Work-item affected */
	struct evg_work_item_t *work_item;

	/* All */
	enum evg_isa_write_task_kind_t kind;
	struct evg_inst_t *inst;
	
	/* When 'kind' == EVG_ISA_WRITE_TASK_WRITE_DEST */
	int gpr, rel, chan, index_mode, write_mask;
	unsigned int value;

	/* When 'kind' == EVG_ISA_WRITE_TASK_WRITE_LDS */
	unsigned int lds_addr;
	unsigned int lds_value;
        int lds_value_size;

	/* When 'kind' == GPU_ISA_WRITE_TASK_PRED_SET */
	int cond;
};


/* Repository for 'struct evg_isa_write_task_t' objects */
extern struct repos_t *evg_isa_write_task_repos;


/* Functions to handle deferred tasks */
void evg_isa_enqueue_write_lds(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, unsigned int addr, unsigned int value,
	int value_size);
void evg_isa_enqueue_write_dest(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, unsigned int value);
void evg_isa_enqueue_write_dest_float(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, float value);
void evg_isa_enqueue_push_before(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst);
void evg_isa_enqueue_pred_set(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int cond);

void evg_isa_write_task_commit(struct evg_work_item_t *work_item);



/* Debugging */
#define evg_isa_debugging() debug_status(evg_isa_debug_category)
#define evg_isa_debug(...) debug(evg_isa_debug_category, __VA_ARGS__)
extern int evg_isa_debug_category;


/* Macros for unsupported parameters */
extern char *evg_err_isa_note;

#define EVG_ISA_ARG_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, inst->info->name, (p), evg_err_isa_note);
#define EVG_ISA_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, inst->info->name, (v), evg_err_isa_note); }
#define EVG_ISA_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, inst->info->name, (min), (max), evg_err_opencl_param_note); }


/* Macros for fast access of instruction words */
#define EVG_CF_WORD0			inst->words[0].cf_word0
#define EVG_CF_GWS_WORD0		inst->words[0].cf_gws_word0
#define EVG_CF_WORD1			inst->words[1].cf_word1

#define EVG_CF_ALU_WORD0		inst->words[0].cf_alu_word0
#define EVG_CF_ALU_WORD1		inst->words[1].cf_alu_word1
#define EVG_CF_ALU_WORD0_EXT		inst->words[0].cf_alu_word0_ext
#define EVG_CF_ALU_WORD1_EXT		inst->words[1].cf_alu_word1_ext

#define EVG_CF_ALLOC_EXPORT_WORD0	inst->words[0].cf_alloc_export_word0
#define EVG_CF_ALLOC_EXPORT_WORD0_RAT	inst->words[0].cf_alloc_export_word0_rat
#define EVG_CF_ALLOC_EXPORT_WORD1_BUF	inst->words[1].cf_alloc_export_word1_buf
#define EVG_CF_ALLOC_EXPORT_WORD1_SWIZ	inst->words[1].cf_alloc_export_word1_swiz

#define EVG_ALU_WORD0			inst->words[0].alu_word0
#define EVG_ALU_WORD1_OP2		inst->words[1].alu_word1_op2
#define EVG_ALU_WORD1_OP3		inst->words[1].alu_word1_op3

#define EVG_ALU_WORD0_LDS_IDX_OP	inst->words[0].alu_word0_lds_idx_op
#define EVG_ALU_WORD1_LDS_IDX_OP	inst->words[1].alu_word1_lds_idx_op

#define EVG_VTX_WORD0			inst->words[0].vtx_word0
#define EVG_VTX_WORD1_GPR		inst->words[1].vtx_word1_gpr
#define EVG_VTX_WORD1_SEM		inst->words[1].vtx_word1_sem
#define EVG_VTX_WORD2			inst->words[2].vtx_word2

#define EVG_TEX_WORD0			inst->words[0].tex_word0
#define EVG_TEX_WORD1			inst->words[1].tex_word1
#define EVG_TEX_WORD2			inst->words[2].tex_word2

#define EVG_MEM_RD_WORD0		inst->words[0].mem_rd_word0
#define EVG_MEM_RD_WORD1		inst->words[1].mem_rd_word1
#define EVG_MEM_RD_WORD2		inst->words[2].mem_rd_word2

#define EVG_MEM_GDS_WORD0		inst->words[0].mem_gds_word0
#define EVG_MEM_GDS_WORD1		inst->words[1].mem_gds_word1
#define EVG_MEM_GDS_WORD2		inst->words[2].mem_gds_word2


/* Table of functions implementing implementing the Evergreen ISA */
typedef void (*evg_isa_inst_func_t)(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst);
extern evg_isa_inst_func_t *evg_isa_inst_func;

/* Access to constant memory */
void evg_isa_const_mem_write(int bank, int vector, int elem, void *value_ptr);
void evg_isa_const_mem_read(int bank, int vector, int elem, void *value_ptr);

/* For ALU clauses */
void evg_isa_alu_clause_start(struct evg_wavefront_t *wavefront);
void evg_isa_alu_clause_end(struct evg_wavefront_t *wavefront);

/* For TC clauses */
void evg_isa_tc_clause_start(struct evg_wavefront_t *wavefront);
void evg_isa_tc_clause_end(struct evg_wavefront_t *wavefront);

/* Read from source register */
unsigned int evg_isa_read_gpr(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, int im);
float evg_isa_read_gpr_float(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, int im);

/* Write into destination register */
void evg_isa_write_gpr(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, unsigned int value);
void evg_isa_write_gpr_float(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, float value);

/* Read input operands */
unsigned int evg_isa_read_op_src_int(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int src_idx);
float evg_isa_read_op_src_float(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int src_idx);

struct evg_inst_t *evg_isa_get_alu_inst(struct evg_alu_group_t *alu_group,
	enum evg_alu_enum alu);

void evg_isa_init(void);
void evg_isa_done(void);


#endif

