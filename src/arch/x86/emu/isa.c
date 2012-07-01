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

#include <mem-system.h>
#include <x86-emu.h>


/* Variables to perform instruction simulation */
struct x86_ctx_t *x86_isa_ctx;
struct x86_regs_t *x86_isa_regs;
struct mem_t *x86_isa_mem;
struct x86_inst_t x86_isa_inst;
int x86_isa_spec_mode;  /* If true, instructions will not modify memory */
unsigned int x86_isa_eip;
char * x86_isa_inst_bytes;
unsigned int isa_addr;  /* Address of last memory access */
unsigned int x86_isa_target;  /* Target address of branch/jmp/call/ret inst, even if it's not taken */
long long x86_isa_inst_count;
int x86_isa_function_level;

int x86_isa_call_debug_category;
int x86_isa_inst_debug_category;

/* Variables used to preserve host state before running assembly */
long x86_isa_host_flags;
uint16_t x86_isa_guest_fpcw;
uint8_t x86_isa_host_fpenv[28];


/* Table including references to functions in machine.c
 * that implement machine instructions. */
typedef void (*x86_isa_inst_func_t)(void);
static x86_isa_inst_func_t x86_isa_inst_func[x86_opcode_count] =
{
	NULL /* for op_none */
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) , x86_isa_##name##_impl
#include <x86-asm.dat>
#undef DEFINST
};



/*
 * Memory access based on 'x86_isa_spec_mode'
 * IMPORTANT: This should be the first section in the file.
 *            Macros are defined after these two functions.
 */

void x86_isa_mem_read(struct mem_t *mem, uint32_t addr, int size, void *buf)
{
	/* Speculative mode read */
	if (x86_isa_spec_mode)
	{
		spec_mem_read(x86_isa_ctx->spec_mem, addr, size, buf);
		return;
	}

	/* Read in regular mode */
	mem_read(mem, addr, size, buf);
}


void x86_isa_mem_write(struct mem_t *mem, uint32_t addr, int size, void *buf)
{
	/* Speculative mode write */
	if (x86_isa_spec_mode)
	{
		spec_mem_write(x86_isa_ctx->spec_mem, addr, size, buf);
		return;
	}

	/* Write in regular mode */
	mem_write(mem, addr, size, buf);
}


void x86_isa_error(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	/* No error shown on speculative mode */
	if (x86_isa_spec_mode)
		return;

	/* Error */
	fprintf(stderr, "fatal: context %d at 0x%08x inst %lld: ",
		x86_isa_ctx->pid, x86_isa_eip, x86_isa_inst_count);
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
static void x86_isa_debug_call()
{
	int i;
	struct elf_symbol_t *from, *to;
	char *action;

	/* Do nothing on speculative mode */
	if (x86_isa_spec_mode)
		return;

	/* Call or return. Otherwise, exit */
	if (!strncmp(x86_isa_inst.format, "call", 4))
		action = "call";
	else if (!strncmp(x86_isa_inst.format, "ret", 3))
		action = "ret";
	else
		return;

	/* Debug it */
	for (i = 0; i < x86_isa_function_level; i++)
		x86_isa_call_debug("| ");
	from = elf_symbol_get_by_address(x86_isa_ctx->loader->elf_file, x86_isa_eip, NULL);
	to = elf_symbol_get_by_address(x86_isa_ctx->loader->elf_file, x86_isa_regs->eip, NULL);
	if (from)
		x86_isa_call_debug("%s", from->name);
	else
		x86_isa_call_debug("0x%x", x86_isa_eip);
	x86_isa_call_debug(" - %s to ", action);
	if (to)
		x86_isa_call_debug("%s", to->name);
	else
		x86_isa_call_debug("0x%x", x86_isa_regs->eip);
	x86_isa_call_debug("\n");

	/* Change current level */
	strncmp(x86_isa_inst.format, "call", 4) ? x86_isa_function_level-- : x86_isa_function_level++;
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


void x86_isa_set_flag(enum x86_flag_t flag)
{
	x86_isa_regs->eflags = SETBIT32(x86_isa_regs->eflags, flag);
}


void x86_isa_clear_flag(enum x86_flag_t flag)
{
	x86_isa_regs->eflags = CLEARBIT32(x86_isa_regs->eflags, flag);
}


int x86_isa_get_flag(enum x86_flag_t flag)
{
	int ret;
	ret = GETBIT32(x86_isa_regs->eflags, flag) > 0;
	return ret;
}


/* Load/store the value of a register. If the register size is less than 32 bits,
 * it is zero-extended. These functions work for reg = reg_none, too. */

static uint32_t x86_isa_bit_mask[5] = { 0, 0xff, 0xffff, 0, 0xffffffff};


uint32_t x86_isa_load_reg(enum x86_reg_t reg)
{
	uint32_t mask, *preg;
	mask = x86_isa_bit_mask[x86_isa_reg_info[reg].size];
	preg = (void *) x86_isa_regs + x86_isa_reg_info[reg].shift;
	return *preg & mask;
}


void x86_isa_store_reg(enum x86_reg_t reg, uint32_t value)
{
	uint32_t mask, *preg;
	mask = x86_isa_bit_mask[x86_isa_reg_info[reg].size];
	preg = (void *) x86_isa_regs + x86_isa_reg_info[reg].shift;
	*preg = (*preg & ~mask) | (value & mask);
	x86_isa_inst_debug("  %s <- 0x%x", x86_reg_name[reg], value);
}




/*
 * Effective address computation
 */

/* Return the final address obtained from binding address 'addr' inside
 * the corresponding segment. The segment boundaries are checked. */
static uint32_t x86_isa_linear_address(uint32_t offset)
{
	/* No segment override */
	if (!x86_isa_inst.segment)
	{
		isa_addr = offset;
		return isa_addr;
	}
	
	/* Segment override */
	if (x86_isa_inst.segment != x86_reg_gs)
	{
		x86_isa_error("segment override not supported for other register than gs");
		return 0;
	}

	/* GLibc segment at TLS entry 6 */
	if (x86_isa_load_reg(x86_reg_gs) != 0x33)
	{
		x86_isa_error("isa_linear_address: gs = 0x%x", x86_isa_load_reg(x86_reg_gs));
		return 0;
	}

	if (!x86_isa_ctx->glibc_segment_base)
	{
		x86_isa_error("isa_linear_address: glibc segment not set");
		return 0;
	}

	/* Return address */
	isa_addr = x86_isa_ctx->glibc_segment_base + offset;
	return isa_addr;
}


/* Return the effective address obtained from the 'SIB' and 'disp' fields */
uint32_t x86_isa_effective_address()
{
	uint32_t addr;

	/* Check 'modrm_mod' field */
	if (x86_isa_inst.modrm_mod == 3)
	{
		x86_isa_error("%s: wrong value for 'modrm_mod'", __FUNCTION__);
		return 0;
	}

	/* Address */
	addr = x86_isa_load_reg(x86_isa_inst.ea_base) +
		x86_isa_load_reg(x86_isa_inst.ea_index) * x86_isa_inst.ea_scale +
		x86_isa_inst.disp;
	
	/* Add segment base */
	addr = x86_isa_linear_address(addr);

	return addr;
}


/* Return the effective address obtained from the
 * immediate field. */
uint32_t x86_isa_moffs_address()
{
	uint32_t addr;

	/* Immediate value as effective address. */
	addr = x86_isa_inst.imm.d;

	/* Add segment base */
	addr = x86_isa_linear_address(addr);

	return addr;
}




/*
 * Register/Memory dependences
 */


uint8_t x86_isa_load_rm8(void)
{
	uint8_t value;
	if (x86_isa_inst.modrm_mod == 0x03)
		return x86_isa_load_reg(x86_isa_inst.modrm_rm + x86_reg_al);
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 1, &value);
	x86_isa_inst_debug("  [0x%x]=0x%x", x86_isa_effective_address(), value);
	return value;
}


uint16_t x86_isa_load_rm16(void)
{
	uint16_t value;
	if (x86_isa_inst.modrm_mod == 0x03)
		return x86_isa_load_reg(x86_isa_inst.modrm_rm + x86_reg_ax);
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 2, &value);
	x86_isa_inst_debug("  [0x%x]=0x%x", x86_isa_effective_address(), value);
	return value;
}


uint32_t x86_isa_load_rm32(void)
{
	uint32_t value;
	if (x86_isa_inst.modrm_mod == 0x03)
		return x86_isa_load_reg(x86_isa_inst.modrm_rm + x86_reg_eax);
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 4, &value);
	x86_isa_inst_debug("  [0x%x]=0x%x", x86_isa_effective_address(), value);
	return value;
}


uint64_t x86_isa_load_m64(void)
{
	uint64_t value;
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 8, &value);
	x86_isa_inst_debug("  [0x%x]=0x%llx", x86_isa_effective_address(), (unsigned long long) value);
	return value;
}


void x86_isa_store_rm8(uint8_t value)
{
	if (x86_isa_inst.modrm_mod == 0x03) {
		x86_isa_store_reg(x86_isa_inst.modrm_rm + x86_reg_al, value);
		return;
	}
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 1, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%x", x86_isa_effective_address(), value);
}


void x86_isa_store_rm16(uint16_t value)
{
	if (x86_isa_inst.modrm_mod == 0x03) {
		x86_isa_store_reg(x86_isa_inst.modrm_rm + x86_reg_ax, value);
		return;
	}
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 2, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%x", x86_isa_effective_address(), value);
}


void x86_isa_store_rm32(uint32_t value)
{
	if (x86_isa_inst.modrm_mod == 0x03) {
		x86_isa_store_reg(x86_isa_inst.modrm_rm + x86_reg_eax, value);
		return;
	}
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 4, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%x", x86_isa_effective_address(), value);
}


void x86_isa_store_m64(uint64_t value)
{
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 8, &value);
	x86_isa_inst_debug("  [0x%x] <- 0x%llx", x86_isa_effective_address(), (unsigned long long) value);
}




/*
 * Floating-point
 */


void x86_isa_load_fpu(int index, uint8_t *value)
{
	int eff_index = (x86_isa_regs->fpu_top + index) % 8;

	if (index < 0 || index >= 8) {
		x86_isa_error("%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	if (!x86_isa_regs->fpu_stack[eff_index].valid) {
		x86_isa_error("%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	memcpy(value, x86_isa_regs->fpu_stack[eff_index].value, 10);
	if (debug_status(x86_isa_inst_debug_category))
		x86_isa_inst_debug("  st(%d)=%g", index, x86_isa_extended_to_double(value));
}


void x86_isa_store_fpu(int index, uint8_t *value)
{
	if (index < 0 || index >= 8) {
		x86_isa_error("%s: invalid value for 'index'", __FUNCTION__);
		return;
	}

	/* Get index */
	index = (x86_isa_regs->fpu_top + index) % 8;
	if (!x86_isa_regs->fpu_stack[index].valid) {
		x86_isa_error("%s: invalid FPU stack entry", __FUNCTION__);
		return;
	}

	/* Store value */
	memcpy(x86_isa_regs->fpu_stack[index].value, value, 10);
	if (debug_status(x86_isa_inst_debug_category))
		x86_isa_inst_debug("  st(%d) <- %g", index, x86_isa_extended_to_double(value));
}


void x86_isa_push_fpu(uint8_t *value)
{
	if (debug_status(x86_isa_inst_debug_category))
		x86_isa_inst_debug("  st(0) <- %g (pushed)", x86_isa_extended_to_double(value));

	/* Get stack top */
	x86_isa_regs->fpu_top = (x86_isa_regs->fpu_top + 7) % 8;
	if (x86_isa_regs->fpu_stack[x86_isa_regs->fpu_top].valid) {
		x86_isa_error("%s: unexpected valid entry", __FUNCTION__);
		return;
	}

	x86_isa_regs->fpu_stack[x86_isa_regs->fpu_top].valid = 1;
	memcpy(x86_isa_regs->fpu_stack[x86_isa_regs->fpu_top].value, value, 10);
}


void x86_isa_pop_fpu(uint8_t *value)
{
	if (!x86_isa_regs->fpu_stack[x86_isa_regs->fpu_top].valid) {
		x86_isa_error("%s: unexpected invalid entry", __FUNCTION__);
		return;
	}

	if (value) {
		memcpy(value, x86_isa_regs->fpu_stack[x86_isa_regs->fpu_top].value, 10);
		if (debug_status(x86_isa_inst_debug_category))
			x86_isa_inst_debug("  st(0) -> %g (popped)", x86_isa_extended_to_double(value));
	}
	x86_isa_regs->fpu_stack[x86_isa_regs->fpu_top].valid = 0;
	x86_isa_regs->fpu_top = (x86_isa_regs->fpu_top + 1) % 8;
}


double x86_isa_load_double()
{
	double value;
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 8, &value);
	x86_isa_inst_debug("  [0x%x]=%g", x86_isa_effective_address(), value);
	return value;
}


void x86_isa_double_to_extended(double f, uint8_t *e)
{
	asm volatile ("fldl %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


double x86_isa_extended_to_double(uint8_t *e)
{
	double f;
	asm volatile ("fldt %1; fstpl %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}


void x86_isa_float_to_extended(float f, uint8_t *e)
{
	asm volatile ("fld %1; fstpt %0\n\t"
			: "=m" (*e) : "m" (f));
}


float x86_isa_extended_to_float(uint8_t *e)
{
	float f;
	asm volatile ("fldt %1; fstp %0\n\t"
			: "=m" (f) : "m" (*e));
	return f;
}


void x86_isa_load_extended(uint8_t *value)
{
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 10, value);
}


void x86_isa_store_extended(uint8_t *value)
{
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 10, value);
}


void x86_isa_store_double(double value)
{
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 8, &value);
	x86_isa_inst_debug("  [0x%x] <- %g", x86_isa_effective_address(), value);
}


float x86_isa_load_float()
{
	float value;
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 4, &value);
	x86_isa_inst_debug("  [0x%x]=%g", x86_isa_effective_address(), (double) value);
	return value;
}


void x86_isa_store_float(float value)
{
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 4, &value);
	x86_isa_inst_debug("  [0x%x] <- %g", x86_isa_effective_address(), (double) value);
}


/* Store the code bits (14, 10, 9, and 8) of the FPU status word into
 * the 'code' register. */
void x86_isa_store_fpu_code(uint16_t status)
{
	x86_isa_regs->fpu_code = 0;
	if (GETBIT32(status, 14))
		x86_isa_regs->fpu_code |= 0x8;
	x86_isa_regs->fpu_code |= (status >> 8) & 0x7;
}


/* Read the status register, by building it from the 'top' and
 * 'code' fields. */
uint16_t x86_isa_load_fpu_status()
{
	uint16_t status = 0;

	if (x86_isa_regs->fpu_top < 0 || x86_isa_regs->fpu_top >= 8)
	{
		x86_isa_error("%s: wrong FPU stack top", __FUNCTION__);
		return 0;
	}

	status |= x86_isa_regs->fpu_top << 11;
	if (GETBIT32(x86_isa_regs->fpu_code, 3))
		status |= 0x4000;
	status |= (x86_isa_regs->fpu_code & 0x7) << 8;
	return status;
}




/*
 * XMM Registers
 */

void x86_isa_dump_xmm(unsigned char *value, FILE *f)
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


void x86_isa_load_xmm(unsigned char *value)
{
	memcpy(value, &x86_isa_regs->xmm[x86_isa_inst.modrm_reg], 16);
}


void x86_isa_store_xmm(unsigned char *value)
{
	memcpy(&x86_isa_regs->xmm[x86_isa_inst.modrm_reg], value, 16);
}


/* Load a 32-bit value into the lower 32 bits of 'value' */
void x86_isa_load_xmmm32(unsigned char *value)
{
	if (x86_isa_inst.modrm_mod == 3)
	{
		memcpy(value, x86_isa_regs->xmm[x86_isa_inst.modrm_rm], 4);
		return;
	}
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 4, value);
}


/* Store the low 32 bits of 'value' into an XMM register or memory */
void x86_isa_store_xmmm32(unsigned char *value)
{
	if (x86_isa_inst.modrm_mod == 3)
	{
		memcpy(&x86_isa_regs->xmm[x86_isa_inst.modrm_rm], value, 4);
		return;
	}
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 4, value);
}


/* Load a 64-bit value into the LSB of 'value'.
 * If 'value' is a 128-bit array, its upper 64 bits will not be initialized. */
void x86_isa_load_xmmm64(unsigned char *value)
{
	if (x86_isa_inst.modrm_mod == 0x03)
	{
		memcpy(value, &x86_isa_regs->xmm[x86_isa_inst.modrm_rm], 8);
		return;
	}
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 8, value);
}


/* Store the low 64 bits of 'value' into an XMM register or memory */
void x86_isa_store_xmmm64(unsigned char *value)
{
	if (x86_isa_inst.modrm_mod == 0x03)
	{
		memcpy(&x86_isa_regs->xmm[x86_isa_inst.modrm_rm], value, 8);
		return;
	}
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 8, value);
}


/* Load a 128-bit value into XMM register */
void x86_isa_load_xmmm128(unsigned char *value)
{
	if (x86_isa_inst.modrm_mod == 3)
	{
		memcpy(value, x86_isa_regs->xmm[x86_isa_inst.modrm_rm], 16);
		return;
	}
	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 16, value);
}


/* Store a 128-bit value into an XMM register of 128-bit memory location. */
void x86_isa_store_xmmm128(unsigned char *value)
{
	if (x86_isa_inst.modrm_mod == 3)
	{
		memcpy(&x86_isa_regs->xmm[x86_isa_inst.modrm_rm], value, 16);
		return;
	}
	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 16, value);
}




/*
 * Public Functions
 */


void x86_isa_init()
{
	x86_disasm_init();
	x86_uinst_init();

	/* Initialize default floating-point control word (FPCW) */
	asm volatile (
		"fstcw %0\n\t"
		: "=m" (x86_isa_guest_fpcw)
	);
	x86_isa_guest_fpcw |= 0x3f;
}


void x86_isa_done()
{
	x86_uinst_done();
	x86_disasm_done();
}


void x86_isa_dump(FILE *f)
{
	if (!x86_isa_ctx)
		return;
	fprintf(f, "isa_ctx: pid=%d\n", x86_isa_ctx->pid);
	fprintf(f, "isa_inst_count: %lld\n", x86_isa_inst_count);
	fprintf(f, "isa_inst:\n");
	fprintf(f, "  eip=0x%x\n", x86_isa_inst.eip);
	fprintf(f, "  ");
	x86_inst_dump(&x86_isa_inst, f);
	fprintf(f, "  (%d bytes)\n", x86_isa_inst.size);
	fprintf(f, "isa_regs:\n");
	x86_regs_dump(x86_isa_regs, f);
}


void x86_isa_execute_inst(void)
{
	/* Debug instruction */
	if (debug_status(x86_isa_inst_debug_category))
	{
		x86_isa_inst_debug("%d %8lld %x: ", x86_isa_ctx->pid,
			x86_isa_inst_count, x86_isa_eip);
		x86_inst_dump(&x86_isa_inst, debug_file(x86_isa_inst_debug_category));
		x86_isa_inst_debug("  (%d bytes)", x86_isa_inst.size);
	}

	/* Clear existing list of microinstructions, though the architectural
	 * simulator might have cleared it already.
	 * A new list will be generated for the next executed x86 instruction. */
	x86_uinst_clear();

	/* Execute */
	x86_isa_target = 0;
	x86_isa_regs->eip = x86_isa_regs->eip + x86_isa_inst.size;
	if (x86_isa_inst.opcode)
		x86_isa_inst_func[x86_isa_inst.opcode]();
	x86_isa_ctx->last_eip = x86_isa_eip;
	
	/* Stats */
	x86_inst_freq[x86_isa_inst.opcode]++;

	/* Debug */
	x86_isa_inst_debug("\n");
	if (debug_status(x86_isa_call_debug_category))
		x86_isa_debug_call();
}

