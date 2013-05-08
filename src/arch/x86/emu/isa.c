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

#include <stdarg.h>

#include <arch/common/arch.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <mem-system/memory.h>
#include <mem-system/spec-mem.h>

#include "context.h"
#include "emu.h"
#include "isa.h"
#include "loader.h"
#include "machine.h"
#include "regs.h"
#include "uinst.h"


/* Debug categories */
int x86_isa_call_debug_category;
int x86_isa_inst_debug_category;

/* Variables used to preserve host state before running assembly */
long x86_isa_host_flags;
unsigned char x86_isa_host_fpenv[28];


/* Table including references to functions in machine.c
 * that implement machine instructions. */
typedef void (*x86_isa_inst_func_t)(struct x86_ctx_t *ctx);
static x86_isa_inst_func_t x86_isa_inst_func[x86_opcode_count] =
{
	NULL /* for op_none */
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) , x86_isa_##name##_impl
#include <arch/x86/asm/asm.dat>
#undef DEFINST
};



/*
 * Memory access based on 'x86_isa_spec_mode'
 * IMPORTANT: This should be the first section in the file.
 *            Macros are defined after these two functions.
 */

void x86_isa_mem_read(struct x86_ctx_t *ctx, unsigned int addr, int size, void *buf)
{
	/* Speculative mode read */
	if (ctx->state & x86_ctx_spec_mode)
	{
		spec_mem_read(ctx->spec_mem, addr, size, buf);
		return;
	}

	/* Read in regular mode */
	mem_read(ctx->mem, addr, size, buf);
}


void x86_isa_mem_write(struct x86_ctx_t *ctx, unsigned int addr, int size, void *buf)
{
	/* Speculative mode write */
	if (ctx->state & x86_ctx_spec_mode)
	{
		spec_mem_write(ctx->spec_mem, addr, size, buf);
		return;
	}

	/* Write in regular mode */
	mem_write(ctx->mem, addr, size, buf);
}


void x86_isa_error(struct x86_ctx_t *ctx, char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	/* No error shown on speculative mode */
	if (ctx->state & x86_ctx_spec_mode)
		return;

	/* Error */
	fprintf(stderr, "fatal: x86 context %d at 0x%08x inst %lld: ",
		ctx->pid, ctx->curr_eip, arch_x86->inst_count);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	exit(1);
}


/* Macros defined to prevent accidental use of 'mem_<xx>' instructions.
 * Error messages should only be shown based on speculative execution. */
#define mem_access __COMPILATION_ERROR__
#define mem_read __COMPILATION_ERROR__
#define mem_write __COMPILATION_ERROR__
#define mem_zero __COMPILATION_ERROR__
#define mem_read_string __COMPILATION_ERROR__
#define mem_write_string __COMPILATION_ERROR__
#define mem_get_buffer __COMPILATION_ERROR__
#define fatal __COMPILATION_ERROR__
#define panic __COMPILATION_ERROR__
#define warning __COMPILATION_ERROR__
#ifdef assert
#undef assert
#endif
#define assert __COMPILATION_ERROR__




/*
 * Instruction statistics
 */

static long long x86_inst_freq[x86_opcode_count];


void x86_isa_inst_stat_dump(FILE *f)
{
	int i;
	for (i = 1; i < x86_opcode_count; i++)
	{
		if (!x86_inst_freq[i])
			continue;
		fprintf(f, "%s    %lld\n", x86_inst_name(i), x86_inst_freq[i]);
	}
}


void x86_isa_inst_stat_reset(void)
{
	int i;
	for (i = 1; i < x86_opcode_count; i++)
		x86_inst_freq[i] = 0;
}


/* Trace call debugging */
static void x86_isa_debug_call(struct x86_ctx_t *ctx)
{
	struct elf_symbol_t *from;
	struct elf_symbol_t *to;

	struct x86_loader_t *loader = ctx->loader;
	struct x86_regs_t *regs = ctx->regs;

	char *action;
	int i;

	/* Do nothing on speculative mode */
	if (ctx->state & x86_ctx_spec_mode)
		return;

	/* Call or return. Otherwise, exit */
	if (!strncmp(ctx->inst.format, "call", 4))
		action = "call";
	else if (!strncmp(ctx->inst.format, "ret", 3))
		action = "ret";
	else
		return;

	/* Debug it */
	for (i = 0; i < ctx->function_level; i++)
		x86_isa_call_debug("| ");
	from = elf_symbol_get_by_address(loader->elf_file, ctx->curr_eip, NULL);
	to = elf_symbol_get_by_address(loader->elf_file, regs->eip, NULL);
	if (from)
		x86_isa_call_debug("%s", from->name);
	else
		x86_isa_call_debug("0x%x", ctx->curr_eip);
	x86_isa_call_debug(" - %s to ", action);
	if (to)
		x86_isa_call_debug("%s", to->name);
	else
		x86_isa_call_debug("0x%x", regs->eip);
	x86_isa_call_debug("\n");

	/* Change current level */
	if (strncmp(ctx->inst.format, "call", 4))
		ctx->function_level--;
	else
		ctx->function_level++;
}




/*
 * Integer registers
 */


/* Shift and size inside the x86_regs_t structure. This table is indexed by the
 * op->data.reg.id field. */
static struct
{
	int shift;
	int size;
} x86_isa_reg_info[] =
{
	{ 0, 0 },
	{ 0, 4 },	/* 1. eax */
	{ 4, 4 },	/* 2. ecx */
	{ 8, 4 },	/* 3. edx */
	{ 12, 4 },	/* 4. ebx */
	{ 16, 4 },	/* 5. esp */
	{ 20, 4 },	/* 6. ebp */
	{ 24, 4 },	/* 7. esi */
	{ 28, 4 },	/* 8. edi */
	{ 0, 2 },	/* 9. ax */
	{ 4, 2 },	/* 10. cx */
	{ 8, 2 },	/* 11. dx */
	{ 12, 2 },	/* 12. bx */
	{ 16, 2 },	/* 13. sp */
	{ 20, 2 },	/* 14. bp */
	{ 24, 2 },	/* 15. si */
	{ 28, 2 },	/* 16. di */
	{ 0, 1 },	/* 17. al */
	{ 4, 1 },	/* 18. cl */
	{ 8, 1 },	/* 19. dl */
	{ 12, 1 },	/* 20. bl */
	{ 1, 1 },	/* 21. ah */
	{ 5, 1 },	/* 22. ch */
	{ 9, 1 },	/* 23. dh */
	{ 13, 1 },	/* 24. bh */
	{ 32, 2 },	/* 25. es */
	{ 34, 2 },	/* 26. cs */
	{ 36, 2 },	/* 27. ss */
	{ 38, 2 },	/* 28. ds */
	{ 40, 2 },	/* 29. fs */
	{ 42, 2 },	/* 30. gs */
};


void x86_isa_set_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag)
{
	struct x86_regs_t *regs = ctx->regs;

	regs->eflags = SETBIT32(regs->eflags, flag);
}


void x86_isa_clear_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag)
{
	struct x86_regs_t *regs = ctx->regs;

	regs->eflags = CLEARBIT32(regs->eflags, flag);
}


int x86_isa_get_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag)
{
	struct x86_regs_t *regs = ctx->regs;

	return GETBIT32(regs->eflags, flag) > 0;
}


/* Load/store the value of a register. If the register size is less than 32 bits,
 * it is zero-extended. These functions work for reg = reg_none, too. */

static unsigned int x86_isa_bit_mask[5] = { 0, 0xff, 0xffff, 0, 0xffffffff};


unsigned int x86_isa_load_reg(struct x86_ctx_t *ctx, enum x86_reg_t reg)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int mask;
	unsigned int *reg_ptr;

	mask = x86_isa_bit_mask[x86_isa_reg_info[reg].size];
	reg_ptr = (void *) regs + x86_isa_reg_info[reg].shift;
	return *reg_ptr & mask;
}


void x86_isa_store_reg(struct x86_ctx_t *ctx, enum x86_reg_t reg, unsigned int value)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int mask;
	unsigned int *reg_ptr;

	mask = x86_isa_bit_mask[x86_isa_reg_info[reg].size];
	reg_ptr = (void *) regs + x86_isa_reg_info[reg].shift;
	*reg_ptr = (*reg_ptr & ~mask) | (value & mask);
	x86_isa_inst_debug("  %s <- 0x%x", x86_reg_name[reg], value);
}




/*
 * Effective address computation
 */

/* Return the final address obtained from binding address 'addr' inside
 * the corresponding segment. The segment boundaries are checked. */
static unsigned int x86_isa_linear_address(struct x86_ctx_t *ctx, unsigned int offset)
{
	/* No segment override */
	if (!ctx->inst.segment)
		return offset;
	
	/* Segment override */
	if (ctx->inst.segment != x86_reg_gs)
	{
		x86_isa_error(ctx, "segment override not supported");
		return 0;
	}

	/* GLibc segment at TLS entry 6 */
	if (x86_isa_load_reg(ctx, x86_reg_gs) != 0x33)
	{
		x86_isa_error(ctx, "isa_linear_address: gs = 0x%x",
				x86_isa_load_reg(ctx, x86_reg_gs));
		return 0;
	}

	if (!ctx->glibc_segment_base)
	{
		x86_isa_error(ctx, "isa_linear_address: glibc segment not set");
		return 0;
	}

	/* Return address */
	return ctx->glibc_segment_base + offset;
}


/* Return the effective address obtained from the 'SIB' and 'disp' fields */
unsigned int x86_isa_effective_address(struct x86_ctx_t *ctx)
{
	unsigned int addr;

	/* Check 'modrm_mod' field */
	if (ctx->inst.modrm_mod == 3)
	{
		x86_isa_error(ctx, "%s: wrong value for 'modrm_mod'", __FUNCTION__);
		return 0;
	}

	/* Address */
	addr = x86_isa_load_reg(ctx, ctx->inst.ea_base) +
		x86_isa_load_reg(ctx, ctx->inst.ea_index) * ctx->inst.ea_scale +
		ctx->inst.disp;
	
	/* Add segment base */
	addr = x86_isa_linear_address(ctx, addr);

	/* Record effective address in context. This address is used later in the
	 * generation of micro-instructions. We need to record it to avoid calling this
	 * function again later, since the source register used to calculate the effective
	 * address can be overwritten after the instruction emulation. */
	ctx->effective_address = addr;

	return addr;
}


/* Return the effective address obtained from the
 * immediate field. */
unsigned int x86_isa_moffs_address(struct x86_ctx_t *ctx)
{
	unsigned int addr;

	/* Immediate value as effective address. */
	addr = ctx->inst.imm.d;

	/* Add segment base */
	addr = x86_isa_linear_address(ctx, addr);

	return addr;
}




/*
 * Register/Memory dependences
 */


unsigned char x86_isa_load_rm8(struct x86_ctx_t *ctx)
{
	unsigned char value;

	if (ctx->inst.modrm_mod == 0x03)
		return x86_isa_load_reg(ctx, ctx->inst.modrm_rm + x86_reg_al);

	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 1, &value);
	x86_isa_inst_debug("  [0x%x]=0x%x", x86_isa_effective_address(ctx), value);
	return value;
}


unsigned short x86_isa_load_rm16(struct x86_ctx_t *ctx)
{
	unsigned short value;

	if (ctx->inst.modrm_mod == 0x03)
		return x86_isa_load_reg(ctx, ctx->inst.modrm_rm + x86_reg_ax);

	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 2, &value);
	x86_isa_inst_debug("  [0x%x]=0x%x", x86_isa_effective_address(ctx), value);
	return value;
}


unsigned int x86_isa_load_rm32(struct x86_ctx_t *ctx)
{
	unsigned int value;

	if (ctx->inst.modrm_mod == 0x03)
		return x86_isa_load_reg(ctx, ctx->inst.modrm_rm + x86_reg_eax);

	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 4, &value);
	x86_isa_inst_debug("  [0x%x]=0x%x", x86_isa_effective_address(ctx), value);
	return value;
}


unsigned short x86_isa_load_r32m16(struct x86_ctx_t *ctx)
{
	unsigned short value;

	if (ctx->inst.modrm_mod == 0x03)
		return x86_isa_load_reg(ctx, ctx->inst.modrm_rm + x86_reg_eax);

	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 2, &value);
	x86_isa_inst_debug("  [0x%x]=0x%x", x86_isa_effective_address(ctx), value);
	return value;
}


unsigned long long x86_isa_load_m64(struct x86_ctx_t *ctx)
{
	unsigned long long value;

	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 8, &value);
	x86_isa_inst_debug("  [0x%x]=0x%llx", x86_isa_effective_address(ctx), value);
	return value;
}


void x86_isa_store_rm8(struct x86_ctx_t *ctx, unsigned char value)
{
	if (ctx->inst.modrm_mod == 0x03)
	{
		x86_isa_store_reg(ctx, ctx->inst.modrm_rm + x86_reg_al, value);
		return;
	}
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 1, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%x", x86_isa_effective_address(ctx), value);
}


void x86_isa_store_rm16(struct x86_ctx_t *ctx, unsigned short value)
{
	if (ctx->inst.modrm_mod == 0x03)
	{
		x86_isa_store_reg(ctx, ctx->inst.modrm_rm + x86_reg_ax, value);
		return;
	}
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 2, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%x", x86_isa_effective_address(ctx), value);
}


void x86_isa_store_rm32(struct x86_ctx_t *ctx, unsigned int value)
{
	if (ctx->inst.modrm_mod == 0x03)
	{
		x86_isa_store_reg(ctx, ctx->inst.modrm_rm + x86_reg_eax, value);
		return;
	}
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 4, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%x", x86_isa_effective_address(ctx), value);
}


void x86_isa_store_m64(struct x86_ctx_t *ctx, unsigned long long value)
{
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 8, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%llx", x86_isa_effective_address(ctx), value);
}




/*
 * Floating-point
 */


void x86_isa_load_fpu(struct x86_ctx_t *ctx, int index, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;
	int eff_index;

	if (index < 0 || index >= 8)
	{
		x86_isa_error(ctx, "%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	eff_index = (regs->fpu_top + index) % 8;
	if (!regs->fpu_stack[eff_index].valid)
	{
		x86_isa_error(ctx, "%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	memcpy(value, regs->fpu_stack[eff_index].value, 10);
	if (debug_status(x86_isa_inst_debug_category))
		x86_isa_inst_debug("  st(%d)=%g", index, x86_isa_extended_to_double(value));
}


void x86_isa_store_fpu(struct x86_ctx_t *ctx, int index, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	/* Check valid index */
	if (index < 0 || index >= 8)
	{
		x86_isa_error(ctx, "%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	/* Get index */
	index = (regs->fpu_top + index) % 8;
	if (!regs->fpu_stack[index].valid)
	{
		x86_isa_error(ctx, "%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	/* Store value */
	memcpy(regs->fpu_stack[index].value, value, 10);
	if (debug_status(x86_isa_inst_debug_category))
		x86_isa_inst_debug("  st(%d) <- %g", index, x86_isa_extended_to_double(value));
}


void x86_isa_push_fpu(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	/* Debug */
	if (debug_status(x86_isa_inst_debug_category))
		x86_isa_inst_debug("  st(0) <- %g (pushed)", x86_isa_extended_to_double(value));

	/* Get stack top */
	regs->fpu_top = (regs->fpu_top + 7) % 8;
	if (regs->fpu_stack[regs->fpu_top].valid)
	{
		x86_isa_error(ctx, "%s: unexpected valid entry", __FUNCTION__);
		return;
	}

	regs->fpu_stack[regs->fpu_top].valid = 1;
	memcpy(regs->fpu_stack[regs->fpu_top].value, value, 10);
}


void x86_isa_pop_fpu(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	/* Check valid entry */
	if (!regs->fpu_stack[regs->fpu_top].valid)
	{
		x86_isa_error(ctx, "%s: unexpected invalid entry", __FUNCTION__);
		return;
	}

	if (value)
	{
		memcpy(value, regs->fpu_stack[regs->fpu_top].value, 10);
		if (debug_status(x86_isa_inst_debug_category))
			x86_isa_inst_debug("  st(0) -> %g (popped)", x86_isa_extended_to_double(value));
	}
	regs->fpu_stack[regs->fpu_top].valid = 0;
	regs->fpu_top = (regs->fpu_top + 1) % 8;
}


double x86_isa_load_double(struct x86_ctx_t *ctx)
{
	double value;

	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 8, &value);
	x86_isa_inst_debug("  [0x%x]=%g", x86_isa_effective_address(ctx), value);
	return value;
}


void x86_isa_double_to_extended(double f, unsigned char *e)
{
	asm volatile ("fldl %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


double x86_isa_extended_to_double(unsigned char *e)
{
	double f;
	asm volatile ("fldt %1; fstpl %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}


void x86_isa_float_to_extended(float f, unsigned char *e)
{
	asm volatile ("flds %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


float x86_isa_extended_to_float(unsigned char *e)
{
	float f;
	asm volatile ("fldt %1; fstps %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}


void x86_isa_load_extended(struct x86_ctx_t *ctx, unsigned char *value)
{
	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 10, value);
}


void x86_isa_store_extended(struct x86_ctx_t *ctx, unsigned char *value)
{
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 10, value);
}


void x86_isa_store_double(struct x86_ctx_t *ctx, double value)
{
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 8, &value);
	x86_isa_inst_debug("  [0x%x] <- %g", x86_isa_effective_address(ctx), value);
}


float x86_isa_load_float(struct x86_ctx_t *ctx)
{
	float value;

	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 4, &value);
	x86_isa_inst_debug("  [0x%x]=%g", x86_isa_effective_address(ctx), (double) value);

	return value;
}


void x86_isa_store_float(struct x86_ctx_t *ctx, float value)
{
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 4, &value);
	x86_isa_inst_debug("  [0x%x] <- %g", x86_isa_effective_address(ctx), (double) value);
}


/* Store the code bits (14, 10, 9, and 8) of the FPU state word into
 * the 'code' register. */
void x86_isa_store_fpu_code(struct x86_ctx_t *ctx, unsigned short status)
{
	struct x86_regs_t *regs = ctx->regs;

	regs->fpu_code = 0;
	if (GETBIT32(status, 14))
		regs->fpu_code |= 0x8;

	regs->fpu_code |= (status >> 8) & 0x7;
}


/* Read the state register, by building it from the 'top' and
 * 'code' fields. */
unsigned short x86_isa_load_fpu_status(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned short status = 0;

	if (regs->fpu_top < 0 || regs->fpu_top >= 8)
	{
		x86_isa_error(ctx, "%s: wrong FPU stack top", __FUNCTION__);
		return 0;
	}

	status |= regs->fpu_top << 11;
	if (GETBIT32(regs->fpu_code, 3))
		status |= 0x4000;
	status |= (regs->fpu_code & 0x7) << 8;
	return status;
}




/*
 * XMM Registers
 */

void x86_isa_dump_xmm(struct x86_ctx_t *ctx, unsigned char *value, FILE *f)
{
	union x86_xmm_reg_t *xmm;
	char *comma;
	int i;

	xmm = (union x86_xmm_reg_t *) value;
	for (i = 0; i < 16; i++)
		fprintf(f, "%02x ", xmm->as_uchar[i]);

	comma = "(";
	for (i = 0; i < 4; i++)
	{
		fprintf(f, "%s%g", comma, xmm->as_float[i]);
		comma = ", ";
	}
	fprintf(f, ")");
}


void x86_isa_load_xmm(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	memcpy(value, &regs->xmm[ctx->inst.modrm_reg], 16);
}


void x86_isa_store_xmm(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	memcpy(&regs->xmm[ctx->inst.modrm_reg], value, 16);
}


/* Load a 32-bit value into the lower 32 bits of 'value' */
void x86_isa_load_xmmm32(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	if (ctx->inst.modrm_mod == 3)
	{
		memcpy(value, regs->xmm[ctx->inst.modrm_rm], 4);
		return;
	}
	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 4, value);
}


/* Store the low 32 bits of 'value' into an XMM register or memory */
void x86_isa_store_xmmm32(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	if (ctx->inst.modrm_mod == 3)
	{
		memcpy(&regs->xmm[ctx->inst.modrm_rm], value, 4);
		return;
	}
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 4, value);
}


/* Load a 64-bit value into the LSB of 'value'.
 * If 'value' is a 128-bit array, its upper 64 bits will not be initialized. */
void x86_isa_load_xmmm64(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	if (ctx->inst.modrm_mod == 0x03)
	{
		memcpy(value, &regs->xmm[ctx->inst.modrm_rm], 8);
		return;
	}
	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 8, value);
}


/* Store the low 64 bits of 'value' into an XMM register or memory */
void x86_isa_store_xmmm64(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	if (ctx->inst.modrm_mod == 0x03)
	{
		memcpy(&regs->xmm[ctx->inst.modrm_rm], value, 8);
		return;
	}
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 8, value);
}


/* Load a 128-bit value into XMM register */
void x86_isa_load_xmmm128(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	if (ctx->inst.modrm_mod == 3)
	{
		memcpy(value, regs->xmm[ctx->inst.modrm_rm], 16);
		return;
	}
	x86_isa_mem_read(ctx, x86_isa_effective_address(ctx), 16, value);
}


/* Store a 128-bit value into an XMM register of 128-bit memory location. */
void x86_isa_store_xmmm128(struct x86_ctx_t *ctx, unsigned char *value)
{
	struct x86_regs_t *regs = ctx->regs;

	if (ctx->inst.modrm_mod == 3)
	{
		memcpy(&regs->xmm[ctx->inst.modrm_rm], value, 16);
		return;
	}
	x86_isa_mem_write(ctx, x86_isa_effective_address(ctx), 16, value);
}




/*
 * Public Functions
 */


void x86_isa_init(void)
{
	x86_disasm_init();
	x86_uinst_init();
}


void x86_isa_done(void)
{
	x86_uinst_done();
	x86_disasm_done();
}


void x86_isa_execute_inst(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	/* Clear existing list of microinstructions, though the architectural
	 * simulator might have cleared it already.
	 * A new list will be generated for the next executed x86 instruction. */
	x86_uinst_clear();

	/* Set last, current, and target instruction addresses */
	ctx->last_eip = ctx->curr_eip;
	ctx->curr_eip = regs->eip;
	ctx->target_eip = 0;

	/* Reset effective address */
	ctx->effective_address = 0;

	/* Debug */
	if (debug_status(x86_isa_inst_debug_category))
	{
		x86_isa_inst_debug("%d %8lld %x: ", ctx->pid,
			arch_x86->inst_count, ctx->curr_eip);
		x86_inst_dump(&ctx->inst, debug_file(x86_isa_inst_debug_category));
		x86_isa_inst_debug("  (%d bytes)", ctx->inst.size);
	}

	/* Call instruction emulation function */
	regs->eip = regs->eip + ctx->inst.size;
	if (ctx->inst.opcode)
		x86_isa_inst_func[ctx->inst.opcode](ctx);
	
	/* Statistics */
	x86_inst_freq[ctx->inst.opcode]++;

	/* Debug */
	x86_isa_inst_debug("\n");
	if (debug_status(x86_isa_call_debug_category))
		x86_isa_debug_call(ctx);
}

