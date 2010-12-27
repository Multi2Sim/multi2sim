/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include "m2skernel.h"


/* Variables to perform instruction simulation */
struct ctx_t *isa_ctx;
struct regs_t *isa_regs;
struct mem_t *isa_mem;
uint32_t isa_eip;
uint32_t isa_addr;  /* Address of last memory access */
uint32_t isa_target;  /* Target address of branch/jmp/call/ret inst, even if it's not taken */
x86_inst_t isa_inst;
uint64_t isa_inst_count;
int isa_function_level;

int isa_call_debug_category;
int isa_inst_debug_category;


/* Table including references to functions in machine.c
 * that implement machine instructions. */
typedef void (*inst_impl_fn_t)(void);
static inst_impl_fn_t inst_impl_table[x86_opcode_count] = {
	NULL /* for op_none */
#define DEFINST(name,op1,op2,op3,imm,pfx) ,op_##name##_impl
#include <machine.dat>
#undef DEFINST
};




/* Instruction stats */

static uint64_t inst_freq[x86_opcode_count];

void isa_inst_stat_dump(FILE *f)
{
	int i;
	for (i = 1; i < x86_opcode_count; i++) {
		if (!inst_freq[i])
			continue;
		fprintf(f, "%s    %lld\n", x86_inst_name(i),
			(long long) inst_freq[i]);
	}
}

void isa_inst_stat_reset(void)
{
	int i;
	for (i = 1; i < x86_opcode_count; i++)
		inst_freq[i] = 0;
}




void isa_error(char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "fatal: context %d at 0x%08x inst %lld: ",
		isa_ctx->pid, isa_eip, (long long) isa_inst_count);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	abort();
}


#ifdef ISA_CHECK_NAN
#define BIT(x) ((value[9-(x)/8]&(1<<(7-(x%8))))>0)
int isa_is_nan(uint8_t *value)
{
	int exp_0s = 0, sig_0s = 0, i;
	for (i = 1; i <= 15; i++)
		if (!BIT(i))
			exp_0s++;
	for (i = 16; i <= 79; i++)
		if (!BIT(i))
			sig_0s++;
	return !exp_0s && sig_0s < 64;
}
#undef BIT
#endif



/* Shift and size inside the regs_t structure. This table is indexed by the
 * op->data.reg.id field. */
static struct {
	int shift;
	int size;
} isa_reg_info[] = {
	{ 0, 0 },
	{ 0, 4 }, /* 1. eax */
	{ 4, 4 }, /* 2. ecx */
	{ 8, 4 }, /* 3. edx */
	{ 12, 4 }, /* 4. ebx */
	{ 16, 4 }, /* 5. esp */
	{ 20, 4 }, /* 6. ebp */
	{ 24, 4 }, /* 7. esi */
	{ 28, 4 }, /* 8. edi */
	{ 0, 2 }, /* 9. ax */
	{ 4, 2 }, /* 10. cx */
	{ 8, 2 }, /* 11. dx */
	{ 12, 2 }, /* 12. bx */
	{ 16, 2 }, /* 13. sp */
	{ 20, 2 }, /* 14. bp */
	{ 24, 2 }, /* 15. si */
	{ 28, 2 }, /* 16. di */
	{ 0, 1 }, /* 17. al */
	{ 4, 1 }, /* 18. cl */
	{ 8, 1 }, /* 19. dl */
	{ 12, 1 }, /* 20. bl */
	{ 1, 1 }, /* 21. ah */
	{ 5, 1 }, /* 22. ch */
	{ 9, 1 }, /* 23. dh */
	{ 13, 1 }, /* 24. bh */
	{ 32, 2 }, /* 25. es */
	{ 34, 2 }, /* 26. cs */
	{ 36, 2 }, /* 27. ss */
	{ 38, 2 }, /* 28. ds */
	{ 40, 2 }, /* 29. fs */
	{ 42, 2 }, /* 30. gs */
};


void isa_set_flag(x86_flag_t flag)
{
	isa_regs->eflags = SETBIT32(isa_regs->eflags, flag);
}


void isa_clear_flag(x86_flag_t flag)
{
	isa_regs->eflags = CLEARBIT32(isa_regs->eflags, flag);
}


int isa_get_flag(x86_flag_t flag)
{
	int ret;
	ret = GETBIT32(isa_regs->eflags, flag) > 0;
	assert(ret == 0 || ret == 1);
	return ret;
}


/* Load/store the value of a register. If the register size
 * is less than 32 bits, it is zero-extended. These
 * functions work for reg = reg_none, too. 
 */
static uint32_t isa_bit_mask[5] = { 0, 0xff, 0xffff, 0, 0xffffffff};
uint32_t isa_load_reg(x86_register_t reg)
{
	uint32_t mask, *preg;
	mask = isa_bit_mask[isa_reg_info[reg].size];
	preg = (void *) isa_regs + isa_reg_info[reg].shift;
	return *preg & mask;
}


void isa_store_reg(x86_register_t reg, uint32_t value)
{
	uint32_t mask, *preg;
	mask = isa_bit_mask[isa_reg_info[reg].size];
	preg = (void *) isa_regs + isa_reg_info[reg].shift;
	*preg = (*preg & ~mask) | (value & mask);
	isa_inst_debug("  %s <- 0x%x", x86_register_name[reg], value);
}


/* Return the final address obtained from binding address 'addr' inside
 * the corresponding segment. The segment boundaries are checked. */
uint32_t isa_linear_address(uint32_t offset)
{
	/* No segment override */
	if (!isa_inst.segment) {
		isa_addr = offset;
		return isa_addr;
	}
	
	/* Segment override */
	if (isa_inst.segment != reg_gs)
		fatal("segment override not supported for other register than gs");
	if (isa_load_reg(reg_gs) != 0x33)  /* glibc segment at TLS entry 6 */
		fatal("isa_linear_address: gs = 0x%x", isa_load_reg(reg_gs));
	if (!isa_ctx->glibc_segment_base)
		fatal("isa_linear_address: glibc segment not set");

	/* Return address */
	isa_addr = isa_ctx->glibc_segment_base + offset;
	return isa_addr;
}


/* Return the effective address obtained from the SIB and disp fields */
uint32_t isa_effective_address()
{
	uint32_t addr;
	assert(isa_inst.modrm_mod != 0x03);

	/* Address */
	addr = isa_load_reg(isa_inst.ea_base) +
		isa_load_reg(isa_inst.ea_index) * isa_inst.ea_scale +
		isa_inst.disp;
	
	/* Add segment base */
	addr = isa_linear_address(addr);

	return addr;
}


/* Return the effective address obtained from the
 * immediate field. */
uint32_t isa_moffs_address()
{
	uint32_t addr;

	/* Immediate value as effective address. */
	addr = isa_inst.imm.d;

	/* Add segment base */
	addr = isa_linear_address(addr);

	return addr;
}


uint8_t isa_load_rm8(void)
{
	uint8_t value;
	if (isa_inst.modrm_mod == 0x03)
		return isa_load_reg(isa_inst.modrm_rm + reg_al);
	mem_read(isa_mem, isa_effective_address(), 1, &value);
	isa_inst_debug("  [0x%x]=0x%x", isa_effective_address(), value);
	return value;
}


uint16_t isa_load_rm16(void)
{
	uint16_t value;
	if (isa_inst.modrm_mod == 0x03)
		return isa_load_reg(isa_inst.modrm_rm + reg_ax);
	mem_read(isa_mem, isa_effective_address(), 2, &value);
	isa_inst_debug("  [0x%x]=0x%x", isa_effective_address(), value);
	return value;
}


uint32_t isa_load_rm32(void)
{
	uint32_t value;
	if (isa_inst.modrm_mod == 0x03)
		return isa_load_reg(isa_inst.modrm_rm + reg_eax);
	mem_read(isa_mem, isa_effective_address(), 4, &value);
	isa_inst_debug("  [0x%x]=0x%x", isa_effective_address(), value);
	return value;
}


void isa_store_rm8(uint8_t value)
{
	if (isa_inst.modrm_mod == 0x03) {
		isa_store_reg(isa_inst.modrm_rm + reg_al, value);
		return;
	}
	mem_write(isa_mem, isa_effective_address(), 1, &value);
	isa_inst_debug("  [0x%x] <- 0x%x", isa_effective_address(), value);
}


void isa_store_rm16(uint16_t value)
{
	if (isa_inst.modrm_mod == 0x03) {
		isa_store_reg(isa_inst.modrm_rm + reg_ax, value);
		return;
	}
	mem_write(isa_mem, isa_effective_address(), 2, &value);
	isa_inst_debug("  [0x%x] <- 0x%x", isa_effective_address(), value);
}


void isa_store_rm32(uint32_t value)
{
	if (isa_inst.modrm_mod == 0x03) {
		isa_store_reg(isa_inst.modrm_rm + reg_eax, value);
		return;
	}
	mem_write(isa_mem, isa_effective_address(), 4, &value);
	isa_inst_debug("  [0x%x] <- 0x%x", isa_effective_address(), value);
}


void isa_load_fpu(int index, uint8_t *value)
{
	int eff_index = (isa_regs->fpu_top + index) % 8;
	assert(index >= 0 && index < 8);
	assert(isa_regs->fpu_stack[eff_index].valid);
	memcpy(value, isa_regs->fpu_stack[eff_index].value, 10);
	if (debug_status(isa_inst_debug_category))
		isa_inst_debug("  st(%d)=%g", index, isa_extended_to_double(value));
}


void isa_store_fpu(int index, uint8_t *value)
{
	assert(index >= 0 && index < 8);
	if (debug_status(isa_inst_debug_category))
		isa_inst_debug("  st(%d) <- %g", index, isa_extended_to_double(value));
#ifdef ISA_CHECK_NAN
	if (isa_is_nan(value))
		fatal("NaN floating point result");
#endif
	index = (isa_regs->fpu_top + index) % 8;
	assert(isa_regs->fpu_stack[index].valid);
	memcpy(isa_regs->fpu_stack[index].value, value, 10);
}


void isa_push_fpu(uint8_t *value)
{
	if (debug_status(isa_inst_debug_category))
		isa_inst_debug("  st(0) <- %g (pushed)", isa_extended_to_double(value));
#ifdef ISA_CHECK_NAN
	if (isa_is_nan(value))
		fatal("NaN floating point result");
#endif
	isa_regs->fpu_top = (isa_regs->fpu_top + 7) % 8;
	assert(!isa_regs->fpu_stack[isa_regs->fpu_top].valid);
	isa_regs->fpu_stack[isa_regs->fpu_top].valid = 1;
	memcpy(isa_regs->fpu_stack[isa_regs->fpu_top].value, value, 10);
}


void isa_pop_fpu(uint8_t *value)
{
	assert(isa_regs->fpu_stack[isa_regs->fpu_top].valid);
	if (value) {
		memcpy(value, isa_regs->fpu_stack[isa_regs->fpu_top].value, 10);
		if (debug_status(isa_inst_debug_category))
			isa_inst_debug("  st(0) -> %g (popped)", isa_extended_to_double(value));
	}
	isa_regs->fpu_stack[isa_regs->fpu_top].valid = 0;
	isa_regs->fpu_top = (isa_regs->fpu_top + 1) % 8;
}


double isa_load_double()
{
	double value;
	mem_read(isa_mem, isa_effective_address(), 8, &value);
	isa_inst_debug("  [0x%x]=%g", isa_effective_address(), value);
	return value;
}


void isa_double_to_extended(double f, uint8_t *e)
{
	asm volatile ("fldl %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


double isa_extended_to_double(uint8_t *e)
{
	double f;
	asm volatile ("fldt %1; fstpl %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}


void isa_float_to_extended(float f, uint8_t *e)
{
	asm volatile ("fld %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


float isa_extended_to_float(uint8_t *e)
{
	float f;
	asm volatile ("fldt %1; fstp %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}


void isa_load_extended(uint8_t *value)
{
	mem_read(isa_mem, isa_effective_address(), 10, value);
}


void isa_store_extended(uint8_t *value)
{
	mem_write(isa_mem, isa_effective_address(), 10, value);
}


void isa_store_double(double value)
{
	mem_write(isa_mem, isa_effective_address(), 8, &value);
	isa_inst_debug("  [0x%x] <- %g", isa_effective_address(), value);
}


float isa_load_float()
{
	float value;
	mem_read(isa_mem, isa_effective_address(), 4, &value);
	isa_inst_debug("  [0x%x]=%g", isa_effective_address(), (double) value);
	return value;
}


void isa_store_float(float value)
{
	mem_write(isa_mem, isa_effective_address(), 4, &value);
	isa_inst_debug("  [0x%x] <- %g", isa_effective_address(), (double) value);
}


/* Store the code bits (14, 10, 9, and 8) of the FPU status word into
 * the 'code' register. */
void isa_store_fpu_code(uint16_t status)
{
	isa_regs->fpu_code = 0;
	if (GETBIT32(status, 14))
		isa_regs->fpu_code |= 0x8;
	isa_regs->fpu_code |= (status >> 8) & 0x7;
}


/* Read the status register, by building it from the 'top' and
 * 'code' fields. */
uint16_t isa_load_fpu_status()
{
	uint16_t status = 0;
	assert(isa_regs->fpu_top < 8);
	status |= isa_regs->fpu_top << 11;
	if (GETBIT32(isa_regs->fpu_code, 3))
		status |= 0x4000;
	status |= (isa_regs->fpu_code & 0x7) << 8;
	return status;
}




/* Trace call debugging */
static void isa_debug_call()
{
	int i;
	struct elf_symbol_t *from, *to;
	char *action;

	/* Call or return. Otherwise, exit */
	if (!strncmp(isa_inst.format, "call", 4))
		action = "call";
	else if (!strncmp(isa_inst.format, "ret", 3))
		action = "ret";
	else
		return;
	
	/* Debug it */
	for (i = 0; i < isa_function_level; i++)
		isa_call_debug("| ");
	from = elf_get_symbol_by_address(isa_ctx->loader->elf, isa_eip, NULL);
	to = elf_get_symbol_by_address(isa_ctx->loader->elf, isa_regs->eip, NULL);
	if (from)
		isa_call_debug("%s", from->name);
	else
		isa_call_debug("0x%x", isa_eip);
	isa_call_debug(" - %s to ", action);
	if (to)
		isa_call_debug("%s", to->name);
	else
		isa_call_debug("0x%x", isa_regs->eip);
	isa_call_debug("\n");

	/* Change current level */
	strncmp(isa_inst.format, "call", 4) ? isa_function_level-- : isa_function_level++;
}




/* Public functions */


void isa_init()
{
	disasm_init();
}


void isa_done()
{
	disasm_done();
}


void isa_dump(FILE *f)
{
	if (!isa_ctx)
		return;
	fprintf(f, "isa_ctx: pid=%d\n", isa_ctx->pid);
	fprintf(f, "isa_inst_count: %lld\n", (long long) isa_inst_count);
	fprintf(f, "isa_inst:\n");
	fprintf(f, "  eip=0x%x\n", isa_inst.eip);
	fprintf(f, "  ");
	x86_inst_dump(&isa_inst, f);
	fprintf(f, "  (%d bytes)\n", isa_inst.size);
	fprintf(f, "  rep=%d\n", isa_inst.rep);
	fprintf(f, "isa_regs:\n");
	regs_dump(isa_regs, f);
}


void isa_execute_inst(void *buf)
{
	if (isa_inst.opcode == op_none) {
		unsigned char b1, b2, b3, b4;
		mem_read(isa_mem, isa_eip, 1, &b1);
		mem_read(isa_mem, isa_eip + 1, 1, &b2);
		mem_read(isa_mem, isa_eip + 2, 1, &b3);
		mem_read(isa_mem, isa_eip + 3, 1, &b4);
		isa_error("not implemented (%02x %02x %02x %02x...)",
			b1, b2, b3, b4);
	}
	
	/* Debug instruction */
	if (debug_status(isa_inst_debug_category)) {
		isa_inst_debug("%d %8lld %x: ", isa_ctx->pid,
			(long long) isa_inst_count, isa_eip);
		x86_inst_dump(&isa_inst, debug_file(isa_inst_debug_category));
		isa_inst_debug("  (%d bytes)", isa_inst.size);
	}

	/* Execute */
	isa_target = 0;
	isa_regs->eip = isa_regs->eip + isa_inst.size;
	inst_impl_table[isa_inst.opcode]();
	inst_freq[isa_inst.opcode]++;

	/* Debug */
	isa_inst_debug("\n");
	if (debug_status(isa_call_debug_category))
		isa_debug_call();
}

