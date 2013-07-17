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
#include <string.h>

#include <lib/util/misc.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>
#include <mem-system/spec-mem.h>

#include "context.h"
#include "emu.h"
#include "isa.h"
#include "loader.h"
#include "machine.h"
#include "regs.h"
#include "uinst.h"



/*
 * Non-Class Stuff
 */


/* Table including references to functions in machine.c
 * that implement machine instructions. */
typedef void (*X86ContextInstFunc)(X86Context *ctx);
static X86ContextInstFunc x86_context_inst_func[x86_inst_opcode_count] =
{
	NULL /* for op_none */
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) , x86_isa_##name##_impl
#include <arch/x86/asm/asm.dat>
#undef DEFINST
};

/* Debug categories */
int x86_context_call_debug_category;
int x86_context_isa_debug_category;

/* Variables used to preserve host state before running assembly */
long x86_context_host_flags;
unsigned char x86_context_host_fpenv[28];

static long long x86_inst_freq[x86_inst_opcode_count];


void x86_isa_inst_stat_dump(FILE *f)
{
	int i;
	for (i = 1; i < x86_inst_opcode_count; i++)
	{
		if (!x86_inst_freq[i])
			continue;
		fprintf(f, "%s    %lld\n", x86_inst_get_name(i), x86_inst_freq[i]);
	}
}


void x86_isa_inst_stat_reset(void)
{
	int i;
	for (i = 1; i < x86_inst_opcode_count; i++)
		x86_inst_freq[i] = 0;
}


void X86ContextDoubleToExtended(double f, unsigned char *e)
{
	asm volatile ("fldl %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


double X86ContextExtendedToDouble(unsigned char *e)
{
	double f;
	asm volatile ("fldt %1; fstpl %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}


void X86ContextFloatToExtended(float f, unsigned char *e)
{
	asm volatile ("flds %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


float X86ContextExtendedToFloat(unsigned char *e)
{
	float f;
	asm volatile ("fldt %1; fstps %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}





/*
 * Class 'X86Context'
 * Additional functions.
 */

/*
 * Memory access based on 'x86_isa_spec_mode'
 * IMPORTANT: This should be the first section in the file.
 *            Macros are defined after these two functions.
 */

void X86ContextMemRead(X86Context *self, unsigned int addr, int size, void *buf)
{
	/* Speculative mode read */
	if (self->state & X86ContextSpecMode)
	{
		spec_mem_read(self->spec_mem, addr, size, buf);
		return;
	}

	/* Read in regular mode */
	mem_read(self->mem, addr, size, buf);
}


void X86ContextMemWrite(X86Context *self, unsigned int addr, int size, void *buf)
{
	/* Speculative mode write */
	if (self->state & X86ContextSpecMode)
	{
		spec_mem_write(self->spec_mem, addr, size, buf);
		return;
	}

	/* Write in regular mode */
	mem_write(self->mem, addr, size, buf);
}


void X86ContextError(X86Context *self, char *fmt, ...)
{
	X86Emu *emu = self->emu;

	va_list va;
	va_start(va, fmt);

	/* No error shown on speculative mode */
	if (self->state & X86ContextSpecMode)
		return;

	/* Error */
	fprintf(stderr, "fatal: x86 context %d at 0x%08x inst %lld: ",
		self->pid, self->curr_eip, asEmu(emu)->instructions);
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


static void X86ContextDebugCallInst(X86Context *self)
{
	struct elf_symbol_t *from;
	struct elf_symbol_t *to;

	struct x86_loader_t *loader = self->loader;
	struct x86_regs_t *regs = self->regs;

	char *action;
	int i;

	/* Do nothing on speculative mode */
	if (self->state & X86ContextSpecMode)
		return;

	/* Call or return. Otherwise, exit */
	if (!strncmp(self->inst.format, "call", 4))
		action = "call";
	else if (!strncmp(self->inst.format, "ret", 3))
		action = "ret";
	else
		return;

	/* Debug it */
	for (i = 0; i < self->function_level; i++)
		X86ContextDebugCall("| ");
	from = elf_symbol_get_by_address(loader->elf_file, self->curr_eip, NULL);
	to = elf_symbol_get_by_address(loader->elf_file, regs->eip, NULL);
	if (from)
		X86ContextDebugCall("%s", from->name);
	else
		X86ContextDebugCall("0x%x", self->curr_eip);
	X86ContextDebugCall(" - %s to ", action);
	if (to)
		X86ContextDebugCall("%s", to->name);
	else
		X86ContextDebugCall("0x%x", regs->eip);
	X86ContextDebugCall("\n");

	/* Change current level */
	if (strncmp(self->inst.format, "call", 4))
		self->function_level--;
	else
		self->function_level++;
}


/* Shift and size inside the x86_regs_t structure. This table is indexed by the
 * op->data.reg.id field. */
static struct
{
	int shift;
	int size;
} x86_context_regs_info[] =
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


void X86ContextSetFlag(X86Context *self, enum x86_inst_flag_t flag)
{
	struct x86_regs_t *regs = self->regs;

	regs->eflags = SETBIT32(regs->eflags, flag);
}


void X86ContextClearFlag(X86Context *self, enum x86_inst_flag_t flag)
{
	struct x86_regs_t *regs = self->regs;

	regs->eflags = CLEARBIT32(regs->eflags, flag);
}


int X86ContextGetFlag(X86Context *self, enum x86_inst_flag_t flag)
{
	struct x86_regs_t *regs = self->regs;

	return GETBIT32(regs->eflags, flag) > 0;
}


/* Load/store the value of a register. If the register size is less than 32 bits,
 * it is zero-extended. These functions work for reg = reg_none, too. */

static unsigned int x86_context_bit_mask[5] = { 0, 0xff, 0xffff, 0, 0xffffffff};


unsigned int X86ContextLoadReg(X86Context *self, enum x86_inst_reg_t reg)
{
	struct x86_regs_t *regs = self->regs;

	unsigned int mask;
	unsigned int *reg_ptr;

	mask = x86_context_bit_mask[x86_context_regs_info[reg].size];
	reg_ptr = (void *) regs + x86_context_regs_info[reg].shift;
	return *reg_ptr & mask;
}


void X86ContextStoreReg(X86Context *self, enum x86_inst_reg_t reg, unsigned int value)
{
	struct x86_regs_t *regs = self->regs;

	unsigned int mask;
	unsigned int *reg_ptr;

	mask = x86_context_bit_mask[x86_context_regs_info[reg].size];
	reg_ptr = (void *) regs + x86_context_regs_info[reg].shift;
	*reg_ptr = (*reg_ptr & ~mask) | (value & mask);
	X86ContextDebugISA("  %s <- 0x%x", str_map_value(&x86_inst_reg_map, reg), value);
}


/* Return the final address obtained from binding address 'addr' inside
 * the corresponding segment. The segment boundaries are checked. */
static unsigned int X86ContextLinearAddress(X86Context *self, unsigned int offset)
{
	/* No segment override */
	if (!self->inst.segment)
		return offset;
	
	/* Segment override */
	if (self->inst.segment != x86_inst_reg_gs)
	{
		X86ContextError(self, "segment override not supported");
		return 0;
	}

	/* GLibc segment at TLS entry 6 */
	if (X86ContextLoadReg(self, x86_inst_reg_gs) != 0x33)
	{
		X86ContextError(self, "isa_linear_address: gs = 0x%x",
				X86ContextLoadReg(self, x86_inst_reg_gs));
		return 0;
	}

	if (!self->glibc_segment_base)
	{
		X86ContextError(self, "isa_linear_address: glibc segment not set");
		return 0;
	}

	/* Return address */
	return self->glibc_segment_base + offset;
}


/* Return the effective address obtained from the 'SIB' and 'disp' fields */
unsigned int X86ContextEffectiveAddress(X86Context *self)
{
	unsigned int addr;

	/* Check 'modrm_mod' field */
	if (self->inst.modrm_mod == 3)
	{
		X86ContextError(self, "%s: wrong value for 'modrm_mod'", __FUNCTION__);
		return 0;
	}

	/* Address */
	addr = X86ContextLoadReg(self, self->inst.ea_base) +
		X86ContextLoadReg(self, self->inst.ea_index) * self->inst.ea_scale +
		self->inst.disp;
	
	/* Add segment base */
	addr = X86ContextLinearAddress(self, addr);

	/* Record effective address in context. This address is used later in the
	 * generation of micro-instructions. We need to record it to avoid calling this
	 * function again later, since the source register used to calculate the effective
	 * address can be overwritten after the instruction emulation. */
	self->effective_address = addr;

	return addr;
}


unsigned int X86ContextMoffsAddress(X86Context *self)
{
	unsigned int addr;

	/* Immediate value as effective address. */
	addr = self->inst.imm.d;

	/* Add segment base */
	addr = X86ContextLinearAddress(self, addr);

	return addr;
}


unsigned char X86ContextLoadRm8(X86Context *self)
{
	unsigned char value;

	if (self->inst.modrm_mod == 0x03)
		return X86ContextLoadReg(self, self->inst.modrm_rm + x86_inst_reg_al);

	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 1, &value);
	X86ContextDebugISA("  [0x%x]=0x%x", X86ContextEffectiveAddress(self), value);
	return value;
}


unsigned short X86ContextLoadRm16(X86Context *self)
{
	unsigned short value;

	if (self->inst.modrm_mod == 0x03)
		return X86ContextLoadReg(self, self->inst.modrm_rm + x86_inst_reg_ax);

	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 2, &value);
	X86ContextDebugISA("  [0x%x]=0x%x", X86ContextEffectiveAddress(self), value);
	return value;
}


unsigned int X86ContextLoadRm32(X86Context *self)
{
	unsigned int value;

	if (self->inst.modrm_mod == 0x03)
		return X86ContextLoadReg(self, self->inst.modrm_rm + x86_inst_reg_eax);

	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 4, &value);
	X86ContextDebugISA("  [0x%x]=0x%x", X86ContextEffectiveAddress(self), value);
	return value;
}


unsigned short X86ContextLoadR32M16(X86Context *self)
{
	unsigned short value;

	if (self->inst.modrm_mod == 0x03)
		return X86ContextLoadReg(self, self->inst.modrm_rm + x86_inst_reg_eax);

	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 2, &value);
	X86ContextDebugISA("  [0x%x]=0x%x", X86ContextEffectiveAddress(self), value);
	return value;
}


unsigned long long X86ContextLoadM64(X86Context *self)
{
	unsigned long long value;

	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 8, &value);
	X86ContextDebugISA("  [0x%x]=0x%llx", X86ContextEffectiveAddress(self), value);
	return value;
}


void X86ContextStoreRm8(X86Context *self, unsigned char value)
{
	if (self->inst.modrm_mod == 0x03)
	{
		X86ContextStoreReg(self, self->inst.modrm_rm + x86_inst_reg_al, value);
		return;
	}
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 1, &value);
	X86ContextDebugISA("  [0x%x] <- 0x%x", X86ContextEffectiveAddress(self), value);
}


void X86ContextStoreRm16(X86Context *self, unsigned short value)
{
	if (self->inst.modrm_mod == 0x03)
	{
		X86ContextStoreReg(self, self->inst.modrm_rm + x86_inst_reg_ax, value);
		return;
	}
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 2, &value);
	X86ContextDebugISA("  [0x%x] <- 0x%x", X86ContextEffectiveAddress(self), value);
}


void X86ContextStoreRm32(X86Context *self, unsigned int value)
{
	if (self->inst.modrm_mod == 0x03)
	{
		X86ContextStoreReg(self, self->inst.modrm_rm + x86_inst_reg_eax, value);
		return;
	}
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 4, &value);
	X86ContextDebugISA("  [0x%x] <- 0x%x", X86ContextEffectiveAddress(self), value);
}


void X86ContextStoreM64(X86Context *self, unsigned long long value)
{
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 8, &value);
	X86ContextDebugISA("  [0x%x] <- 0x%llx", X86ContextEffectiveAddress(self), value);
}


void X86ContextLoadFpu(X86Context *self, int index, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;
	int eff_index;

	if (index < 0 || index >= 8)
	{
		X86ContextError(self, "%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	eff_index = (regs->fpu_top + index) % 8;
	if (!regs->fpu_stack[eff_index].valid)
	{
		X86ContextError(self, "%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	memcpy(value, regs->fpu_stack[eff_index].value, 10);
	if (debug_status(x86_context_isa_debug_category))
		X86ContextDebugISA("  st(%d)=%g", index, X86ContextExtendedToDouble(value));
}


void X86ContextStoreFpu(X86Context *self, int index, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	/* Check valid index */
	if (index < 0 || index >= 8)
	{
		X86ContextError(self, "%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	/* Get index */
	index = (regs->fpu_top + index) % 8;
	if (!regs->fpu_stack[index].valid)
	{
		X86ContextError(self, "%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	/* Store value */
	memcpy(regs->fpu_stack[index].value, value, 10);
	if (debug_status(x86_context_isa_debug_category))
		X86ContextDebugISA("  st(%d) <- %g", index, X86ContextExtendedToDouble(value));
}


void X86ContextPushFpu(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	/* Debug */
	if (debug_status(x86_context_isa_debug_category))
		X86ContextDebugISA("  st(0) <- %g (pushed)", X86ContextExtendedToDouble(value));

	/* Get stack top */
	regs->fpu_top = (regs->fpu_top + 7) % 8;
	if (regs->fpu_stack[regs->fpu_top].valid)
	{
		X86ContextError(self, "%s: unexpected valid entry", __FUNCTION__);
		return;
	}

	regs->fpu_stack[regs->fpu_top].valid = 1;
	memcpy(regs->fpu_stack[regs->fpu_top].value, value, 10);
}


void X86ContextPopFpu(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	/* Check valid entry */
	if (!regs->fpu_stack[regs->fpu_top].valid)
	{
		X86ContextError(self, "%s: unexpected invalid entry", __FUNCTION__);
		return;
	}

	if (value)
	{
		memcpy(value, regs->fpu_stack[regs->fpu_top].value, 10);
		if (debug_status(x86_context_isa_debug_category))
			X86ContextDebugISA("  st(0) -> %g (popped)", X86ContextExtendedToDouble(value));
	}
	regs->fpu_stack[regs->fpu_top].valid = 0;
	regs->fpu_top = (regs->fpu_top + 1) % 8;
}


double X86ContextLoadDouble(X86Context *self)
{
	double value;

	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 8, &value);
	X86ContextDebugISA("  [0x%x]=%g", X86ContextEffectiveAddress(self), value);
	return value;
}


void X86ContextLoadExtended(X86Context *self, unsigned char *value)
{
	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 10, value);
}


void X86ContextStoreExtended(X86Context *self, unsigned char *value)
{
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 10, value);
}


void X86ContextStoreDouble(X86Context *self, double value)
{
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 8, &value);
	X86ContextDebugISA("  [0x%x] <- %g", X86ContextEffectiveAddress(self), value);
}


float X86ContextLoadFloat(X86Context *self)
{
	float value;

	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 4, &value);
	X86ContextDebugISA("  [0x%x]=%g", X86ContextEffectiveAddress(self), (double) value);

	return value;
}


void X86ContextStoreFloat(X86Context *self, float value)
{
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 4, &value);
	X86ContextDebugISA("  [0x%x] <- %g", X86ContextEffectiveAddress(self), (double) value);
}


/* Store the code bits (14, 10, 9, and 8) of the FPU state word into
 * the 'code' register. */
void X86ContextStoreFpuCode(X86Context *self, unsigned short status)
{
	struct x86_regs_t *regs = self->regs;

	regs->fpu_code = 0;
	if (GETBIT32(status, 14))
		regs->fpu_code |= 0x8;

	regs->fpu_code |= (status >> 8) & 0x7;
}


/* Read the state register, by building it from the 'top' and
 * 'code' fields. */
unsigned short X86ContextLoadFpuStatus(X86Context *self)
{
	struct x86_regs_t *regs = self->regs;
	unsigned short status = 0;

	if (regs->fpu_top < 0 || regs->fpu_top >= 8)
	{
		X86ContextError(self, "%s: wrong FPU stack top", __FUNCTION__);
		return 0;
	}

	status |= regs->fpu_top << 11;
	if (GETBIT32(regs->fpu_code, 3))
		status |= 0x4000;
	status |= (regs->fpu_code & 0x7) << 8;
	return status;
}


void X86ContextDumpXMM(X86Context *self, unsigned char *value, FILE *f)
{
	union x86_inst_xmm_reg_t *xmm;
	char *comma;
	int i;

	xmm = (union x86_inst_xmm_reg_t *) value;
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


void X86ContextLoadXMM(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	memcpy(value, &regs->xmm[self->inst.modrm_reg], 16);
}


void X86ContextStoreXMM(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	memcpy(&regs->xmm[self->inst.modrm_reg], value, 16);
}


/* Load a 32-bit value into the lower 32 bits of 'value' */
void X86ContextLoadXMMM32(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	if (self->inst.modrm_mod == 3)
	{
		memcpy(value, regs->xmm[self->inst.modrm_rm], 4);
		return;
	}
	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 4, value);
}


/* Store the low 32 bits of 'value' into an XMM register or memory */
void X86ContextStoreXMMM32(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	if (self->inst.modrm_mod == 3)
	{
		memcpy(&regs->xmm[self->inst.modrm_rm], value, 4);
		return;
	}
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 4, value);
}


/* Load a 64-bit value into the LSB of 'value'.
 * If 'value' is a 128-bit array, its upper 64 bits will not be initialized. */
void X86ContextLoadXMMM64(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	if (self->inst.modrm_mod == 0x03)
	{
		memcpy(value, &regs->xmm[self->inst.modrm_rm], 8);
		return;
	}
	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 8, value);
}


/* Store the low 64 bits of 'value' into an XMM register or memory */
void X86ContextStoreXMMM64(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	if (self->inst.modrm_mod == 0x03)
	{
		memcpy(&regs->xmm[self->inst.modrm_rm], value, 8);
		return;
	}
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 8, value);
}


/* Load a 128-bit value into XMM register */
void X86ContextLoadXMMM128(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	if (self->inst.modrm_mod == 3)
	{
		memcpy(value, regs->xmm[self->inst.modrm_rm], 16);
		return;
	}
	X86ContextMemRead(self, X86ContextEffectiveAddress(self), 16, value);
}


/* Store a 128-bit value into an XMM register of 128-bit memory location. */
void X86ContextStoreXMMM128(X86Context *self, unsigned char *value)
{
	struct x86_regs_t *regs = self->regs;

	if (self->inst.modrm_mod == 3)
	{
		memcpy(&regs->xmm[self->inst.modrm_rm], value, 16);
		return;
	}
	X86ContextMemWrite(self, X86ContextEffectiveAddress(self), 16, value);
}


void X86ContextExecuteInst(X86Context *self)
{
	X86Emu *emu = self->emu;
	struct x86_regs_t *regs = self->regs;

	/* Clear existing list of microinstructions, though the architectural
	 * simulator might have cleared it already.
	 * A new list will be generated for the next executed x86 instruction. */
	x86_uinst_clear();

	/* Set last, current, and target instruction addresses */
	self->last_eip = self->curr_eip;
	self->curr_eip = regs->eip;
	self->target_eip = 0;

	/* Reset effective address */
	self->effective_address = 0;

	/* Debug */
	if (debug_status(x86_context_isa_debug_category))
	{
		X86ContextDebugISA("%d %8lld %x: ", self->pid,
			asEmu(emu)->instructions, self->curr_eip);
		x86_inst_dump(&self->inst, debug_file(x86_context_isa_debug_category));
		X86ContextDebugISA("  (%d bytes)", self->inst.size);
	}

	/* Call instruction emulation function */
	regs->eip = regs->eip + self->inst.size;
	if (self->inst.opcode)
		x86_context_inst_func[self->inst.opcode](self);
	
	/* Statistics */
	x86_inst_freq[self->inst.opcode]++;

	/* Debug */
	X86ContextDebugISA("\n");
	if (debug_status(x86_context_call_debug_category))
		X86ContextDebugCallInst(self);
}
