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

#ifndef KEPLER_EMU_ISA_H
#define KEPLER_EMU_ISA_H


/* Global variables referring to the instruction that is currently being emulated.
 * There variables are set before calling the instruction emulation function in
 * 'machine.c' to avoid passing pointers. */
extern struct kpl_grid_t *kpl_isa_grid;
extern struct kpl_thread_block_t *kpl_isa_thread_block;
extern struct kpl_warp_t *kpl_isa_warp;
extern struct kpl_thread_t *kpl_isa_thread;
extern struct kpl_inst_t *kpl_isa_inst;

/* Macros for quick access */
#define KPL_GPR_ELEM(_gpr, _elem)  (kpl_isa_thread->gpr[(_gpr)].elem[(_elem)])
#define KPL_GPR_X(_gpr)  KPL_GPR_ELEM((_gpr), 0)
#define KPL_GPR_Y(_gpr)  KPL_GPR_ELEM((_gpr), 1)
#define KPL_GPR_Z(_gpr)  KPL_GPR_ELEM((_gpr), 2)
#define KPL_GPR_W(_gpr)  KPL_GPR_ELEM((_gpr), 3)
#define KPL_GPR_T(_gpr)  KPL_GPR_ELEM((_gpr), 4)

#define KPL_GPR_FLOAT_ELEM(_gpr, _elem)  (* (float *) &kpl_isa_thread->gpr[(_gpr)].elem[(_elem)])
#define KPL_GPR_FLOAT_X(_gpr)  KPL_GPR_FLOAT_ELEM((_gpr), 0)
#define KPL_GPR_FLOAT_Y(_gpr)  KPL_GPR_FLOAT_ELEM((_gpr), 1)
#define KPL_GPR_FLOAT_Z(_gpr)  KPL_GPR_FLOAT_ELEM((_gpr), 2)
#define KPL_GPR_FLOAT_W(_gpr)  KPL_GPR_FLOAT_ELEM((_gpr), 3)
#define KPL_GPR_FLOAT_T(_gpr)  KPL_GPR_FLOAT_ELEM((_gpr), 4)


/* Debugging */
#define kpl_isa_debugging() debug_status(kpl_isa_debug_category)
#define kpl_isa_debug(...) debug(kpl_isa_debug_category, __VA_ARGS__)
extern int kpl_isa_debug_category;


/* Macros for unsupported parameters */
extern char *err_kpl_isa_note;
#define KPL_ISA_ARG_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, kpl_isa_inst->info->name, (p), err_kpl_isa_note);
#define KPL_ISA_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, kpl_isa_inst->info->name, (v), err_kpl_isa_note); }
#define KPL_ISA_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, kpl_isa_inst->info->name, (min), (max), err_kpl_cuda_param_note); }



#endif

