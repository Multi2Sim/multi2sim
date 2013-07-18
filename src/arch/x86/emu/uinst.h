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

#ifndef ARCH_X86_EMU_UINST_H
#define ARCH_X86_EMU_UINST_H

#include <lib/util/class.h>


/* Micro-instruction dependences.
 * WARNING: update 'x86_uinst_dep_name' if modified (uinst.c).
 * WARNING: also update 'x86_uinst_dep_map' if modified (uinst.c). */
enum x86_dep_t
{
	x86_dep_none = 0,

	/* Integer dependences */

	x86_dep_eax = 1,
	x86_dep_ecx = 2,
	x86_dep_edx = 3,
	x86_dep_ebx = 4,
	x86_dep_esp = 5,
	x86_dep_ebp = 6,
	x86_dep_esi = 7,
	x86_dep_edi = 8,

	x86_dep_es = 9,
	x86_dep_cs = 10,
	x86_dep_ss = 11,
	x86_dep_ds = 12,
	x86_dep_fs = 13,
	x86_dep_gs = 14,

	x86_dep_zps = 15,
	x86_dep_of = 16,
	x86_dep_cf = 17,
	x86_dep_df = 18,

	x86_dep_aux = 19,  /* Intermediate results for uops */
	x86_dep_aux2 = 20,
	x86_dep_ea = 21,  /* Internal - Effective address */
	x86_dep_data = 22,  /* Internal - Data for load/store */

	x86_dep_int_first = x86_dep_eax,
	x86_dep_int_last = x86_dep_data,
	x86_dep_int_count = x86_dep_int_last - x86_dep_int_first + 1,

	x86_dep_flag_first = x86_dep_zps,
	x86_dep_flag_last = x86_dep_df,
	x86_dep_flag_count = x86_dep_flag_last - x86_dep_flag_first + 1,


	/* Floating-point dependences */

	x86_dep_st0 = 23,  /* FP registers */
	x86_dep_st1 = 24,
	x86_dep_st2 = 25,
	x86_dep_st3 = 26,
	x86_dep_st4 = 27,
	x86_dep_st5 = 28,
	x86_dep_st6 = 29,
	x86_dep_st7 = 30,
	x86_dep_fpst = 31,  /* FP status word */
	x86_dep_fpcw = 32,  /* FP control word */
	x86_dep_fpaux = 33,  /* Auxiliary FP reg */

	x86_dep_fp_first = x86_dep_st0,
	x86_dep_fp_last = x86_dep_fpaux,
	x86_dep_fp_count = x86_dep_fp_last - x86_dep_fp_first + 1,

	x86_dep_fp_stack_first = x86_dep_st0,
	x86_dep_fp_stack_last  = x86_dep_st7,
	x86_dep_fp_stack_count = x86_dep_fp_stack_last - x86_dep_fp_stack_first + 1,


	/* XMM dependences */

	x86_dep_xmm0 = 34,
	x86_dep_xmm1 = 35,
	x86_dep_xmm2 = 36,
	x86_dep_xmm3 = 37,
	x86_dep_xmm4 = 38,
	x86_dep_xmm5 = 39,
	x86_dep_xmm6 = 40,
	x86_dep_xmm7 = 41,
	x86_dep_xmm_data = 42,

	x86_dep_xmm_first = x86_dep_xmm0,
	x86_dep_xmm_last = x86_dep_xmm_data,
	x86_dep_xmm_count = x86_dep_xmm_last - x86_dep_xmm_first + 1,


	/* Special dependences */

	x86_dep_rm8 = 0x100,
	x86_dep_rm16 = 0x101,
	x86_dep_rm32 = 0x102,

	x86_dep_ir8 = 0x200,
	x86_dep_ir16 = 0x201,
	x86_dep_ir32 = 0x202,

	x86_dep_r8 = 0x300,
	x86_dep_r16 = 0x301,
	x86_dep_r32 = 0x302,
	x86_dep_sreg = 0x400,

	x86_dep_mem8 = 0x500,
	x86_dep_mem16 = 0x501,
	x86_dep_mem32 = 0x502,
	x86_dep_mem64 = 0x503,
	x86_dep_mem80 = 0x504,
	x86_dep_mem128 = 0x505,

	x86_dep_easeg = 0x601,  /* Effective address - segment */
	x86_dep_eabas = 0x602,  /* Effective address - base */
	x86_dep_eaidx = 0x603,  /* Effective address - index */

	x86_dep_sti = 0x700,  /* FP - ToS+Index */

	x86_dep_xmmm32 = 0x800,
	x86_dep_xmmm64 = 0x801,
	x86_dep_xmmm128 = 0x802,
	
	x86_dep_xmm = 0x900
};

#define X86_DEP_IS_INT_REG(dep) ((dep) >= x86_dep_int_first && (dep) <= x86_dep_int_last)
#define X86_DEP_IS_FP_REG(dep) ((dep) >= x86_dep_fp_first && (dep) <= x86_dep_fp_last)
#define X86_DEP_IS_XMM_REG(dep)  ((dep) >= x86_dep_xmm_first && (dep) <= x86_dep_xmm_last)
#define X86_DEP_IS_FLAG(dep) ((dep) >= x86_dep_flag_first && (dep) <= x86_dep_flag_last)
#define X86_DEP_IS_VALID(dep) (X86_DEP_IS_INT_REG(dep) || X86_DEP_IS_FP_REG(dep) || X86_DEP_IS_XMM_REG(dep))


enum x86_uinst_flag_t
{
	X86_UINST_INT		= 0x001,  /* Arithmetic integer instruction */
	X86_UINST_LOGIC		= 0x002,  /* Logic computation */
	X86_UINST_FP		= 0x004,  /* Floating-point micro-instruction */
	X86_UINST_MEM		= 0x008,  /* Memory micro-instructions */
	X86_UINST_CTRL		= 0x010,  /* Micro-instruction affecting control flow */
	X86_UINST_COND		= 0x020,  /* Conditional branch */
	X86_UINST_UNCOND	= 0x040,  /* Unconditional jump */
	X86_UINST_XMM		= 0x080   /* XMM micro-instruction */
};


/* Micro-instruction opcodes.
 * WARNING: when the set of micro-instructions is modified, also update:
 *   - Variable 'x86_uinst_info' (src/arch/x86/emu/uinst.c).
 *   - Variable 'fu_class_table' (src/arch/x86/timing/fu.c).
 *   - M2S Guide (CISC instruction decoding) */
enum x86_uinst_opcode_t
{
	x86_uinst_nop = 0,

	x86_uinst_move,
	x86_uinst_add,
	x86_uinst_sub,
	x86_uinst_mult,
	x86_uinst_div,
	x86_uinst_effaddr,

	x86_uinst_and,
	x86_uinst_or,
	x86_uinst_xor,
	x86_uinst_not,
	x86_uinst_shift,
	x86_uinst_sign,

	x86_uinst_fp_move,
	x86_uinst_fp_sign,
	x86_uinst_fp_round,

	x86_uinst_fp_add,
	x86_uinst_fp_sub,
	x86_uinst_fp_comp,
	x86_uinst_fp_mult,
	x86_uinst_fp_div,

	x86_uinst_fp_exp,
	x86_uinst_fp_log,
	x86_uinst_fp_sin,
	x86_uinst_fp_cos,
	x86_uinst_fp_sincos,
	x86_uinst_fp_tan,
	x86_uinst_fp_atan,
	x86_uinst_fp_sqrt,

	x86_uinst_fp_push,
	x86_uinst_fp_pop,

	x86_uinst_xmm_and,
	x86_uinst_xmm_or,
	x86_uinst_xmm_xor,
	x86_uinst_xmm_not,
	x86_uinst_xmm_nand,
	x86_uinst_xmm_shift,
	x86_uinst_xmm_sign,

	x86_uinst_xmm_add,
	x86_uinst_xmm_sub,
	x86_uinst_xmm_comp,
	x86_uinst_xmm_mult,
	x86_uinst_xmm_div,

	x86_uinst_xmm_fp_add,
	x86_uinst_xmm_fp_sub,
	x86_uinst_xmm_fp_comp,
	x86_uinst_xmm_fp_mult,
	x86_uinst_xmm_fp_div,

	x86_uinst_xmm_fp_sqrt,

	x86_uinst_xmm_move,
	x86_uinst_xmm_shuf,
	x86_uinst_xmm_conv,

	x86_uinst_load,
	x86_uinst_store,
	x86_uinst_prefetch,

	x86_uinst_call,
	x86_uinst_ret,
	x86_uinst_jump,
	x86_uinst_branch,
	x86_uinst_ibranch,

	x86_uinst_syscall,

	x86_uinst_opcode_count
};


extern struct x86_uinst_info_t
{
	char *name;
	enum x86_uinst_flag_t flags;
} x86_uinst_info[x86_uinst_opcode_count];


#define X86_UINST_MAX_IDEPS 3
#define X86_UINST_MAX_ODEPS 4
#define X86_UINST_MAX_DEPS  (X86_UINST_MAX_IDEPS + X86_UINST_MAX_ODEPS)

struct x86_uinst_t
{
	/* Operation */
	enum x86_uinst_opcode_t opcode;

	/* Dependences */
	enum x86_dep_t dep[X86_UINST_MAX_IDEPS + X86_UINST_MAX_ODEPS];
	enum x86_dep_t *idep;  /* First 'X86_UINST_MAX_IDEPS' elements of 'dep' */
	enum x86_dep_t *odep;  /* Last 'X86_UINST_MAX_ODEPS' elements of 'dep' */

	/* Memory access */
	unsigned int address;
	int size;
};


/* True if generation of micro-instruction is active. This is true when there is
 * an x86 detailed simulation. For efficiency purposes, the condition is cached
 * in this global variable. */
extern int x86_uinst_active;

extern struct list_t *x86_uinst_list;

void x86_uinst_init(void);
void x86_uinst_done(void);

struct x86_uinst_t *x86_uinst_create(void);
void x86_uinst_free(struct x86_uinst_t *uinst);

/* To prevent performance degradation in functional simulation, do the check before the actual
 * function call. Notice that 'x86_uinst_new' calls are done for every x86 instruction emulation. */
#define x86_uinst_new(ctx, opcode, idep0, idep1, idep2, odep0, odep1, odep2, odep3) \
	{ if (x86_uinst_active) \
	__x86_uinst_new(ctx, opcode, idep0, idep1, idep2, odep0, odep1, odep2, odep3); }
#define x86_uinst_new_mem(ctx, opcode, addr, size, idep0, idep1, idep2, odep0, odep1, odep2, odep3) \
	{ if (x86_uinst_active) \
	__x86_uinst_new_mem(ctx, opcode, addr, size, idep0, idep1, idep2, odep0, odep1, odep2, odep3); }

void __x86_uinst_new(X86Context *ctx, enum x86_uinst_opcode_t opcode,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3);
void __x86_uinst_new_mem(X86Context *ctx,
	enum x86_uinst_opcode_t opcode, unsigned int addr, int size,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3);
void x86_uinst_clear(void);

void x86_uinst_dump_buf(struct x86_uinst_t *uinst, char *buf, int size);
void x86_uinst_dump(struct x86_uinst_t *uinst, FILE *f);
void x86_uinst_list_dump(FILE *f);


#endif

