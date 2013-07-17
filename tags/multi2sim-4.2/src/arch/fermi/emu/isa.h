/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FERMI_EMU_ISA_H
#define FERMI_EMU_ISA_H


/* Global variables referring to the instruction that is currently being emulated.
 * There variables are set before calling the instruction emulation function in
 * 'machine.c' to avoid passing pointers. */
extern struct frm_grid_t *frm_isa_grid;
extern struct frm_thread_block_t *frm_isa_thread_block;
extern struct frm_warp_t *frm_isa_warp;
extern struct frm_thread_t *frm_isa_thread;
extern struct frm_inst_t *frm_isa_inst;

/* Macros for quick access */
#define FRM_GPR_ELEM(_gpr, _elem)  (frm_isa_thread->gpr[(_gpr)].elem[(_elem)])
#define FRM_GPR_X(_gpr)  FRM_GPR_ELEM((_gpr), 0)
#define FRM_GPR_Y(_gpr)  FRM_GPR_ELEM((_gpr), 1)
#define FRM_GPR_Z(_gpr)  FRM_GPR_ELEM((_gpr), 2)
#define FRM_GPR_W(_gpr)  FRM_GPR_ELEM((_gpr), 3)
#define FRM_GPR_T(_gpr)  FRM_GPR_ELEM((_gpr), 4)

#define FRM_GPR_FLOAT_ELEM(_gpr, _elem)  (* (float *) &frm_isa_thread->gpr[(_gpr)].elem[(_elem)])
#define FRM_GPR_FLOAT_X(_gpr)  FRM_GPR_FLOAT_ELEM((_gpr), 0)
#define FRM_GPR_FLOAT_Y(_gpr)  FRM_GPR_FLOAT_ELEM((_gpr), 1)
#define FRM_GPR_FLOAT_Z(_gpr)  FRM_GPR_FLOAT_ELEM((_gpr), 2)
#define FRM_GPR_FLOAT_W(_gpr)  FRM_GPR_FLOAT_ELEM((_gpr), 3)
#define FRM_GPR_FLOAT_T(_gpr)  FRM_GPR_FLOAT_ELEM((_gpr), 4)


/* Debugging */
#define frm_isa_debugging() debug_status(frm_isa_debug_category)
#define frm_isa_debug(...) debug(frm_isa_debug_category, __VA_ARGS__)
extern int frm_isa_debug_category;


/* Macros for unsupported parameters */
extern char *err_frm_isa_note;
#define FRM_ISA_ARG_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, frm_isa_inst->info->name, (p), err_frm_isa_note);
#define FRM_ISA_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, frm_isa_inst->info->name, (v), err_frm_isa_note); }
#define FRM_ISA_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, frm_isa_inst->info->name, (min), (max), err_frm_opencl_param_note); }


/* Access to global memory */
void frm_isa_global_mem_write(unsigned int addr, void *pvalue);
void frm_isa_global_mem_read(unsigned int addr, void *pvalue);

/* Access to constant memory */
void frm_isa_const_mem_write(unsigned int addr, void *pvalue);
void frm_isa_const_mem_read(unsigned int addr, void *pvalue);

/* For ALU clauses */
void frm_isa_alu_clause_start(void);
void frm_isa_alu_clause_end(void);

/* For TC clauses */
void frm_isa_tc_clause_start(void);
void frm_isa_tc_clause_end(void);

/* For functional simulation */
unsigned int frm_isa_read_gpr_int(int id);
float frm_isa_read_gpr_float(int id);
unsigned int frm_isa_read_sgpr(int id);
void frm_isa_write_gpr_int(int id, unsigned int value);
void frm_isa_write_gpr_float(int id, float value);

unsigned int frm_isa_read_src_int(int src_idx);
float frm_isa_read_src_float(int src_idx);

void frm_isa_init(void);
void frm_isa_done(void);


#endif

