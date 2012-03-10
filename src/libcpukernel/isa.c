/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cpukernel.h>


/* Variables to perform instruction simulation */
struct ctx_t *isa_ctx;
struct regs_t *isa_regs;
struct mem_t *isa_mem;
struct x86_inst_t isa_inst;
int isa_spec_mode;  /* If true, instructions will not modify memory */
uint32_t isa_eip;
uint32_t isa_addr;  /* Address of last memory access */
uint32_t isa_target;  /* Target address of branch/jmp/call/ret inst, even if it's not taken */
uint64_t isa_inst_count;
int isa_function_level;

int isa_call_debug_category;
int isa_inst_debug_category;

/* Variables used to preserve host state before running assembly */
long isa_host_flags;
uint16_t isa_guest_fpcw;
uint8_t isa_host_fpenv[28];


/* Table including references to functions in machine.c
 * that implement machine instructions. */
typedef void (*inst_impl_fn_t)(void);
static inst_impl_fn_t inst_impl_table[x86_opcode_count] =
{
	NULL /* for op_none */
#define DEFINST(name,op1,op2,op3,modrm,imm,pfx) ,op_##name##_impl
#include <machine.dat>
#undef DEFINST
};



/*
 * Memory access based on 'isa_spec_mode'
 * IMPORTANT: This should be the first section in the file.
 *            Macros are defined after these two functions.
 */

void isa_mem_read(struct mem_t *mem, uint32_t addr, int size, void *buf)
{
	/* Speculative mode read */
	if (isa_spec_mode)
	{
		spec_mem_read(isa_ctx->spec_mem, addr, size, buf);
		return;
	}

	/* Read in regular mode */
	mem_read(mem, addr, size, buf);
}


void isa_mem_write(struct mem_t *mem, uint32_t addr, int size, void *buf)
{
	/* Speculative mode write */
	if (isa_spec_mode)
	{
		spec_mem_write(isa_ctx->spec_mem, addr, size, buf);
		return;
	}

	/* Write in regular mode */
	mem_write(mem, addr, size, buf);
}


void isa_error(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	/* No error shown on speculative mode */
	if (isa_spec_mode)
		return;

	/* Error */
	fprintf(stderr, "fatal: context %d at 0x%08x inst %lld: ",
		isa_ctx->pid, isa_eip, (long long) isa_inst_count);
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
	from = elf_symbol_get_by_address(isa_ctx->loader->elf_file, isa_eip, NULL);
	to = elf_symbol_get_by_address(isa_ctx->loader->elf_file, isa_regs->eip, NULL);
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




/*
 * Integer registers
 */


/* Shift and size inside the regs_t structure. This table is indexed by the
 * op->data.reg.id field. */
static struct {
	int shift;
	int size;
} isa_reg_info[] = {
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


void isa_set_flag(enum x86_flag_t flag)
{
	isa_regs->eflags = SETBIT32(isa_regs->eflags, flag);
}


void isa_clear_flag(enum x86_flag_t flag)
{
	isa_regs->eflags = CLEARBIT32(isa_regs->eflags, flag);
}


int isa_get_flag(enum x86_flag_t flag)
{
	int ret;
	ret = GETBIT32(isa_regs->eflags, flag) > 0;
	return ret;
}


/* Load/store the value of a register. If the register size is less than 32 bits,
 * it is zero-extended. These functions work for reg = reg_none, too. */

static uint32_t isa_bit_mask[5] = { 0, 0xff, 0xffff, 0, 0xffffffff};


uint32_t isa_load_reg(enum x86_reg_t reg)
{
	uint32_t mask, *preg;
	mask = isa_bit_mask[isa_reg_info[reg].size];
	preg = (void *) isa_regs + isa_reg_info[reg].shift;
	return *preg & mask;
}


void isa_store_reg(enum x86_reg_t reg, uint32_t value)
{
	uint32_t mask, *preg;
	mask = isa_bit_mask[isa_reg_info[reg].size];
	preg = (void *) isa_regs + isa_reg_info[reg].shift;
	*preg = (*preg & ~mask) | (value & mask);
	isa_inst_debug("  %s <- 0x%x", x86_reg_name[reg], value);
}




/*
 * Effective address computation
 */

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
	if (isa_inst.segment != x86_reg_gs) {
		isa_error("segment override not supported for other register than gs");
		return 0;
	}

	/* GLibc segment at TLS entry 6 */
	if (isa_load_reg(x86_reg_gs) != 0x33)  {
		isa_error("isa_linear_address: gs = 0x%x", isa_load_reg(x86_reg_gs));
		return 0;
	}

	if (!isa_ctx->glibc_segment_base) {
		isa_error("isa_linear_address: glibc segment not set");
		return 0;
	}

	/* Return address */
	isa_addr = isa_ctx->glibc_segment_base + offset;
	return isa_addr;
}


/* Return the effective address obtained from the 'SIB' and 'disp' fields */
uint32_t isa_effective_address()
{
	uint32_t addr;

	/* Check 'modrm_mod' field */
	if (isa_inst.modrm_mod == 3) {
		isa_error("%s: wrong value for 'modrm_mod'", __FUNCTION__);
		return 0;
	}

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




/*
 * Register/Memory dependences
 */


uint8_t isa_load_rm8(void)
{
	uint8_t value;
	if (isa_inst.modrm_mod == 0x03)
		return isa_load_reg(isa_inst.modrm_rm + x86_reg_al);
	isa_mem_read(isa_mem, isa_effective_address(), 1, &value);
	isa_inst_debug("  [0x%x]=0x%x", isa_effective_address(), value);
	return value;
}


uint16_t isa_load_rm16(void)
{
	uint16_t value;
	if (isa_inst.modrm_mod == 0x03)
		return isa_load_reg(isa_inst.modrm_rm + x86_reg_ax);
	isa_mem_read(isa_mem, isa_effective_address(), 2, &value);
	isa_inst_debug("  [0x%x]=0x%x", isa_effective_address(), value);
	return value;
}


uint32_t isa_load_rm32(void)
{
	uint32_t value;
	if (isa_inst.modrm_mod == 0x03)
		return isa_load_reg(isa_inst.modrm_rm + x86_reg_eax);
	isa_mem_read(isa_mem, isa_effective_address(), 4, &value);
	isa_inst_debug("  [0x%x]=0x%x", isa_effective_address(), value);
	return value;
}


uint64_t isa_load_m64(void)
{
	uint64_t value;
	isa_mem_read(isa_mem, isa_effective_address(), 8, &value);
	isa_inst_debug("  [0x%x]=0x%llx", isa_effective_address(), (unsigned long long) value);
	return value;
}


void isa_store_rm8(uint8_t value)
{
	if (isa_inst.modrm_mod == 0x03) {
		isa_store_reg(isa_inst.modrm_rm + x86_reg_al, value);
		return;
	}
	isa_mem_write(isa_mem, isa_effective_address(), 1, &value);
	isa_inst_debug("  [0x%x] <- 0x%x", isa_effective_address(), value);
}


void isa_store_rm16(uint16_t value)
{
	if (isa_inst.modrm_mod == 0x03) {
		isa_store_reg(isa_inst.modrm_rm + x86_reg_ax, value);
		return;
	}
	isa_mem_write(isa_mem, isa_effective_address(), 2, &value);
	isa_inst_debug("  [0x%x] <- 0x%x", isa_effective_address(), value);
}


void isa_store_rm32(uint32_t value)
{
	if (isa_inst.modrm_mod == 0x03) {
		isa_store_reg(isa_inst.modrm_rm + x86_reg_eax, value);
		return;
	}
	isa_mem_write(isa_mem, isa_effective_address(), 4, &value);
	isa_inst_debug("  [0x%x] <- 0x%x", isa_effective_address(), value);
}


void isa_store_m64(uint64_t value)
{
	isa_mem_write(isa_mem, isa_effective_address(), 8, &value);
	isa_inst_debug("  [0x%x] <- 0x%llx", isa_effective_address(), (unsigned long long) value);
}




/*
 * Floating-point
 */


void isa_load_fpu(int index, uint8_t *value)
{
	int eff_index = (isa_regs->fpu_top + index) % 8;

	if (index < 0 || index >= 8) {
		isa_error("%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	if (!isa_regs->fpu_stack[eff_index].valid) {
		isa_error("%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	memcpy(value, isa_regs->fpu_stack[eff_index].value, 10);
	if (debug_status(isa_inst_debug_category))
		isa_inst_debug("  st(%d)=%g", index, isa_extended_to_double(value));
}


void isa_store_fpu(int index, uint8_t *value)
{
	if (index < 0 || index >= 8) {
		isa_error("%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	/* Get index */
	index = (isa_regs->fpu_top + index) % 8;
	if (!isa_regs->fpu_stack[index].valid) {
		isa_error("%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	/* Store value */
	memcpy(isa_regs->fpu_stack[index].value, value, 10);
	if (debug_status(isa_inst_debug_category))
		isa_inst_debug("  st(%d) <- %g", index, isa_extended_to_double(value));
}


void isa_push_fpu(uint8_t *value)
{
	if (debug_status(isa_inst_debug_category))
		isa_inst_debug("  st(0) <- %g (pushed)", isa_extended_to_double(value));

	/* Get stack top */
	isa_regs->fpu_top = (isa_regs->fpu_top + 7) % 8;
	if (isa_regs->fpu_stack[isa_regs->fpu_top].valid) {
		isa_error("%s: unexpected valid entry", __FUNCTION__);
		return;
	}

	isa_regs->fpu_stack[isa_regs->fpu_top].valid = 1;
	memcpy(isa_regs->fpu_stack[isa_regs->fpu_top].value, value, 10);
}


void isa_pop_fpu(uint8_t *value)
{
	if (!isa_regs->fpu_stack[isa_regs->fpu_top].valid) {
		isa_error("%s: unexpected invalid entry", __FUNCTION__);
		return;
	}

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
	isa_mem_read(isa_mem, isa_effective_address(), 8, &value);
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
	isa_mem_read(isa_mem, isa_effective_address(), 10, value);
}


void isa_store_extended(uint8_t *value)
{
	isa_mem_write(isa_mem, isa_effective_address(), 10, value);
}


void isa_store_double(double value)
{
	isa_mem_write(isa_mem, isa_effective_address(), 8, &value);
	isa_inst_debug("  [0x%x] <- %g", isa_effective_address(), value);
}


float isa_load_float()
{
	float value;
	isa_mem_read(isa_mem, isa_effective_address(), 4, &value);
	isa_inst_debug("  [0x%x]=%g", isa_effective_address(), (double) value);
	return value;
}


void isa_store_float(float value)
{
	isa_mem_write(isa_mem, isa_effective_address(), 4, &value);
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

	if (isa_regs->fpu_top < 0 || isa_regs->fpu_top >= 8) {
		isa_error("%s: wrong FPU stack top", __FUNCTION__);
		return 0;
	}

	status |= isa_regs->fpu_top << 11;
	if (GETBIT32(isa_regs->fpu_code, 3))
		status |= 0x4000;
	status |= (isa_regs->fpu_code & 0x7) << 8;
	return status;
}




/*
 * XMM Registers
 */

void isa_dump_xmm(unsigned char *value, FILE *f)
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


void isa_load_xmm(unsigned char *value)
{
	memcpy(value, &isa_regs->xmm[isa_inst.modrm_reg], 16);
}


void isa_store_xmm(unsigned char *value)
{
	memcpy(&isa_regs->xmm[isa_inst.modrm_reg], value, 16);
}


/* Load a 32-bit value into the lower 32 bits of 'value' */
void isa_load_xmmm32(unsigned char *value)
{
	if (isa_inst.modrm_mod == 3)
	{
		memcpy(value, isa_regs->xmm[isa_inst.modrm_rm], 4);
		return;
	}
	isa_mem_read(isa_mem, isa_effective_address(), 4, value);
}


/* Store the low 32 bits of 'value' into an XMM register or memory */
void isa_store_xmmm32(unsigned char *value)
{
	if (isa_inst.modrm_mod == 3)
	{
		memcpy(&isa_regs->xmm[isa_inst.modrm_rm], value, 4);
		return;
	}
	isa_mem_write(isa_mem, isa_effective_address(), 4, value);
}


/* Load a 64-bit value into the LSB of 'value'.
 * If 'value' is a 128-bit array, its upper 64 bits will not be initialized. */
void isa_load_xmmm64(unsigned char *value)
{
	if (isa_inst.modrm_mod == 0x03)
	{
		memcpy(value, &isa_regs->xmm[isa_inst.modrm_rm], 8);
		return;
	}
	isa_mem_read(isa_mem, isa_effective_address(), 8, value);
}


/* Store the low 64 bits of 'value' into an XMM register or memory */
void isa_store_xmmm64(unsigned char *value)
{
	if (isa_inst.modrm_mod == 0x03)
	{
		memcpy(&isa_regs->xmm[isa_inst.modrm_rm], value, 8);
		return;
	}
	isa_mem_write(isa_mem, isa_effective_address(), 8, value);
}


/* Load a 128-bit value into XMM register */
void isa_load_xmmm128(unsigned char *value)
{
	if (isa_inst.modrm_mod == 3)
	{
		memcpy(value, isa_regs->xmm[isa_inst.modrm_rm], 16);
		return;
	}
	isa_mem_read(isa_mem, isa_effective_address(), 16, value);
}


/* Store a 128-bit value into an XMM register of 128-bit memory location. */
void isa_store_xmmm128(unsigned char *value)
{
	if (isa_inst.modrm_mod == 3)
	{
		memcpy(&isa_regs->xmm[isa_inst.modrm_rm], value, 16);
		return;
	}
	isa_mem_write(isa_mem, isa_effective_address(), 16, value);
}




/*
 * Public Functions
 */


void isa_init()
{
	x86_disasm_init();
	x86_uinst_init();

	/* Initialize default floating-point control word (FPCW) */
	asm volatile (
		"fstcw %0\n\t"
		: "=m" (isa_guest_fpcw)
	);
	isa_guest_fpcw |= 0x3f;
}


void isa_done()
{
	x86_uinst_done();
	x86_disasm_done();
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
	fprintf(f, "isa_regs:\n");
	regs_dump(isa_regs, f);
}


void isa_execute_inst(void)
{
	/* Debug instruction */
	if (debug_status(isa_inst_debug_category))
	{
		isa_inst_debug("%d %8lld %x: ", isa_ctx->pid,
			(long long) isa_inst_count, isa_eip);
		x86_inst_dump(&isa_inst, debug_file(isa_inst_debug_category));
		isa_inst_debug("  (%d bytes)", isa_inst.size);
	}

	/* Clear existing list of microinstructions, though the architectural
	 * simulator might have cleared it already.
	 * A new list will be generated for the next executed x86 instruction. */
	x86_uinst_clear();

	/* Execute */
	isa_target = 0;
	isa_regs->eip = isa_regs->eip + isa_inst.size;
	if (isa_inst.opcode)
		inst_impl_table[isa_inst.opcode]();
	isa_ctx->last_eip = isa_eip;
	
	/* Stats */
	inst_freq[isa_inst.opcode]++;

	/* Debug */
	isa_inst_debug("\n");
	if (debug_status(isa_call_debug_category))
		isa_debug_call();
}

