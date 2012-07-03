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

#ifndef X86_EMU_H
#define X86_EMU_H

#include <mhandle.h>
#include <debug.h>
#include <config.h>
#include <buffer.h>
#include <list.h>
#include <linked-list.h>
#include <misc.h>
#include <elf-format.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <x86-asm.h>
#include <time.h>
#include <timer.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>
#include <esim.h>
#include <sys/time.h>

#include <southern-islands-emu.h>
#include <evergreen-emu.h>
#include <fermi-emu.h>


/* Some forward declarations */
struct x86_ctx_t;
struct file_desc_t;




/*
 * X86 Registers
 */

struct x86_regs_t
{
	/* Integer registers */
	uint32_t eax, ecx, edx, ebx;
	uint32_t esp, ebp, esi, edi;
	uint16_t es, cs, ss, ds, fs, gs;
	uint32_t eip;
	uint32_t eflags;

	/* Floating-point unit */
	struct
	{
		unsigned char value[10];
		int valid;
	} fpu_stack[8];
	int fpu_top;  /* top of stack (field 'top' of status register) */
	int fpu_code;  /* field 'code' of status register (C3-C2-C1-C0) */
	uint16_t fpu_ctrl;  /* fpu control word */

	/* XMM registers (8 128-bit regs) */
	unsigned char xmm[8][16];

} __attribute__((packed));

struct x86_regs_t *x86_regs_create(void);
void x86_regs_free(struct x86_regs_t *regs);

void x86_regs_copy(struct x86_regs_t *dst, struct x86_regs_t *src);
void x86_regs_dump(struct x86_regs_t *regs, FILE *f);
void x86_regs_fpu_stack_dump(struct x86_regs_t *regs, FILE *f);




/*
 * Program loader
 */

struct x86_loader_t
{
	/* Number of extra contexts using this loader */
	int num_links;

	/* Program data */
	struct elf_file_t *elf_file;
	struct linked_list_t *args;
	struct linked_list_t *env;
	char *interp;  /* Executable interpreter */
	char *exe;  /* Executable file name */
	char *cwd;  /* Current working directory */
	char *stdin_file;  /* File name for stdin */
	char *stdout_file;  /* File name for stdout */

	/* IPC report (for detailed simulation) */
	FILE *ipc_report_file;
	int ipc_report_interval;

	/* Stack */
	unsigned int stack_base;
	unsigned int stack_top;
	unsigned int stack_size;
	unsigned int environ_base;

	/* Lowest address initialized */
	unsigned int bottom;

	/* Program entries */
	unsigned int prog_entry;
	unsigned int interp_prog_entry;

	/* Program headers */
	unsigned int phdt_base;
	unsigned int phdr_count;

	/* Random bytes */
	unsigned int at_random_addr;
	unsigned int at_random_addr_holder;
};


#define x86_loader_debug(...) debug(x86_loader_debug_category, __VA_ARGS__)
extern int x86_loader_debug_category;

extern char *x86_loader_help_ctxconfig;

struct x86_loader_t *x86_loader_create(void);
void x86_loader_free(struct x86_loader_t *ld);

struct x86_loader_t *x86_loader_link(struct x86_loader_t *ld);
void x86_loader_unlink(struct x86_loader_t *ld);

void x86_loader_convert_filename(struct x86_loader_t *ld, char *file_name);
void x86_loader_get_full_path(struct x86_ctx_t *ctx, char *file_name, char *full_path, int size);

void x86_loader_add_args(struct x86_ctx_t *ctx, int argc, char **argv);
void x86_loader_add_cmdline(struct x86_ctx_t *ctx, char *cmdline);
void x86_loader_set_cwd(struct x86_ctx_t *ctx, char *cwd);
void x86_loader_set_redir(struct x86_ctx_t *ctx, char *stdin, char *stdout);
void x86_loader_load_exe(struct x86_ctx_t *ctx, char *exe);

void x86_loader_load_prog_from_ctxconfig(char *ctxconfig);
void x86_loader_load_prog_from_cmdline(int argc, char **argv);

/* Architectural state checkpoints */
void x86_checkpoint_load(char *path);
void x86_checkpoint_save(char *path);

/*
 * Microinstructions
 */


/* Micro-instruction dependences.
 * WARNING: update 'x86_uinst_dep_name' if modified (uinst.c).
 * WARNING: also update 'x86_uinst_dep_map' if modified (uinst.c). */
enum x86_dep_t
{
	x86_dep_none = 0,

	/** Integer dependences **/

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


	/** Floating-point dependences **/

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


	/** XMM dependences */

	x86_dep_xmm0 = 34,
	x86_dep_xmm1 = 35,
	x86_dep_xmm2 = 36,
	x86_dep_xmm3 = 37,
	x86_dep_xmm4 = 38,
	x86_dep_xmm5 = 39,
	x86_dep_xmm6 = 40,
	x86_dep_xmm7 = 41,

	x86_dep_xmm_first = x86_dep_xmm0,
	x86_dep_xmm_last = x86_dep_xmm7,
	x86_dep_xmm_count = x86_dep_xmm_last - x86_dep_xmm_first + 1,


	/** Special dependences **/

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


extern struct list_t *x86_uinst_list;

void x86_uinst_init(void);
void x86_uinst_done(void);

struct x86_uinst_t *x86_uinst_create(void);
void x86_uinst_free(struct x86_uinst_t *uinst);

/* To prevent performance degradation in functional simulation, do the check before the actual
 * function call. Notice that 'x86_uinst_new' calls are done for every x86 instruction emulation. */
#define x86_uinst_new(ctx, opcode, idep0, idep1, idep2, odep0, odep1, odep2, odep3) \
	{ if (x86_emu_kind == x86_emu_kind_detailed) \
	__x86_uinst_new(ctx, opcode, idep0, idep1, idep2, odep0, odep1, odep2, odep3); }
#define x86_uinst_new_mem(ctx, opcode, addr, size, idep0, idep1, idep2, odep0, odep1, odep2, odep3) \
	{ if (x86_emu_kind == x86_emu_kind_detailed) \
	__x86_uinst_new_mem(ctx, opcode, addr, size, idep0, idep1, idep2, odep0, odep1, odep2, odep3); }

void __x86_uinst_new(struct x86_ctx_t *ctx, enum x86_uinst_opcode_t opcode,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3);
void __x86_uinst_new_mem(struct x86_ctx_t *ctx,
	enum x86_uinst_opcode_t opcode, uint32_t addr, int size,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3);
void x86_uinst_clear(void);

void x86_uinst_dump_buf(struct x86_uinst_t *uinst, char *buf, int size);
void x86_uinst_dump(struct x86_uinst_t *uinst, FILE *f);
void x86_uinst_list_dump(FILE *f);




/*
 * Machine & ISA
 */

extern struct x86_ctx_t *x86_isa_ctx;
extern struct x86_regs_t *x86_isa_regs;
extern struct mem_t *x86_isa_mem;
extern int x86_isa_spec_mode;
extern unsigned int x86_isa_eip;
extern char * x86_isa_inst_bytes;
extern unsigned int x86_isa_target;
extern struct x86_inst_t x86_isa_inst;
extern long long x86_isa_inst_count;
extern int x86_isa_function_level;

#define x86_isa_call_debug(...) debug(x86_isa_call_debug_category, __VA_ARGS__)
#define x86_isa_inst_debug(...) debug(x86_isa_inst_debug_category, __VA_ARGS__)

extern int x86_isa_call_debug_category;
extern int x86_isa_inst_debug_category;


extern long x86_isa_host_flags;

#define __X86_ISA_ASM_START__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	: "=m" (x86_isa_host_flags));

#define __X86_ISA_ASM_END__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	: "=m" (x86_isa_host_flags));


extern unsigned char x86_isa_host_fpenv[28];
extern unsigned short x86_isa_guest_fpcw;

#define __X86_ISA_FP_ASM_START__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	"fnstenv %1\n\t" /* store host FPU environment */ \
	"fnclex\n\t" /* clear host FP exceptions */ \
	"fldcw %2\n\t" \
	: "=m" (x86_isa_host_flags), "=m" (*x86_isa_host_fpenv) \
	: "m" (x86_isa_guest_fpcw));

#define __X86_ISA_FP_ASM_END__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	"fnstcw %1\n\t" \
	"fldenv %2\n\t" /* restore host FPU environment */ \
	: "=m" (x86_isa_host_flags), "=m" (x86_isa_guest_fpcw) \
	: "m" (*x86_isa_host_fpenv));


/* References to functions emulating x86 instructions */
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) \
		void x86_isa_##name##_impl(struct x86_ctx_t *ctx);
#include <x86-asm.dat>
#undef DEFINST

void x86_isa_error(struct x86_ctx_t *ctx, char *fmt, ...);

void x86_isa_mem_read(struct x86_ctx_t *ctx, unsigned int addr, int size, void *buf);
void x86_isa_mem_write(struct x86_ctx_t *ctx, unsigned int addr, int size, void *buf);

void x86_isa_dump_flags(struct x86_ctx_t *ctx, FILE *f);
void x86_isa_set_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag);
void x86_isa_clear_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag);
int x86_isa_get_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag);

unsigned int x86_isa_load_reg(struct x86_ctx_t *ctx, enum x86_reg_t reg);
void x86_isa_store_reg(struct x86_ctx_t *ctx, enum x86_reg_t reg, unsigned int value);

unsigned char x86_isa_load_rm8(struct x86_ctx_t *ctx);
unsigned short x86_isa_load_rm16(struct x86_ctx_t *ctx);
unsigned int x86_isa_load_rm32(struct x86_ctx_t *ctx);
unsigned long long x86_isa_load_m64(struct x86_ctx_t *ctx);
void x86_isa_store_rm8(struct x86_ctx_t *ctx, unsigned char value);
void x86_isa_store_rm16(struct x86_ctx_t *ctx, unsigned short value);
void x86_isa_store_rm32(struct x86_ctx_t *ctx, unsigned int value);
void x86_isa_store_m64(struct x86_ctx_t *ctx, unsigned long long value);

#define x86_isa_load_r8(ctx) x86_isa_load_reg(ctx, x86_isa_inst.reg + x86_reg_al)
#define x86_isa_load_r16(ctx) x86_isa_load_reg(ctx, x86_isa_inst.reg + x86_reg_ax)
#define x86_isa_load_r32(ctx) x86_isa_load_reg(ctx, x86_isa_inst.reg + x86_reg_eax)
#define x86_isa_load_sreg(ctx) x86_isa_load_reg(ctx, x86_isa_inst.reg + x86_reg_es)
#define x86_isa_store_r8(ctx, value) x86_isa_store_reg(ctx, x86_isa_inst.reg + x86_reg_al, value)
#define x86_isa_store_r16(ctx, value) x86_isa_store_reg(ctx, x86_isa_inst.reg + x86_reg_ax, value)
#define x86_isa_store_r32(ctx, value) x86_isa_store_reg(ctx, x86_isa_inst.reg + x86_reg_eax, value)
#define x86_isa_store_sreg(ctx, value) x86_isa_store_reg(ctx, x86_isa_inst.reg + x86_reg_es, value)

#define x86_isa_load_ir8(ctx) x86_isa_load_reg(ctx, x86_isa_inst.opindex + x86_reg_al)
#define x86_isa_load_ir16(ctx) x86_isa_load_reg(ctx, x86_isa_inst.opindex + x86_reg_ax)
#define x86_isa_load_ir32(ctx) x86_isa_load_reg(ctx, x86_isa_inst.opindex + x86_reg_eax)
#define x86_isa_store_ir8(ctx, value) x86_isa_store_reg(ctx, x86_isa_inst.opindex + x86_reg_al, value)
#define x86_isa_store_ir16(ctx, value) x86_isa_store_reg(ctx, x86_isa_inst.opindex + x86_reg_ax, value)
#define x86_isa_store_ir32(ctx, value) x86_isa_store_reg(ctx, x86_isa_inst.opindex + x86_reg_eax, value)

void x86_isa_load_fpu(struct x86_ctx_t *ctx, int index, unsigned char *value);
void x86_isa_store_fpu(struct x86_ctx_t *ctx, int index, unsigned char *value);
void x86_isa_pop_fpu(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_push_fpu(struct x86_ctx_t *ctx, unsigned char *value);

float x86_isa_load_float(struct x86_ctx_t *ctx);
double x86_isa_load_double(struct x86_ctx_t *ctx);
void x86_isa_load_extended(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_float(struct x86_ctx_t *ctx, float value);
void x86_isa_store_double(struct x86_ctx_t *ctx, double value);
void x86_isa_store_extended(struct x86_ctx_t *ctx, unsigned char *value);

void x86_isa_dump_xmm(struct x86_ctx_t *ctx, unsigned char *value, FILE *f);
void x86_isa_load_xmm(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmm(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_load_xmmm32(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmmm32(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_load_xmmm64(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmmm64(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_load_xmmm128(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmmm128(struct x86_ctx_t *ctx, unsigned char *value);

void x86_isa_double_to_extended(double f, unsigned char *e);
double x86_isa_extended_to_double(unsigned char *e);
void x86_isa_float_to_extended(float f, unsigned char *e);
float x86_isa_extended_to_float(unsigned char *e);

void x86_isa_store_fpu_code(struct x86_ctx_t *ctx, unsigned short status);
unsigned short x86_isa_load_fpu_status(struct x86_ctx_t *ctx);

unsigned int x86_isa_effective_address(struct x86_ctx_t *ctx);
unsigned int x86_isa_moffs_address(struct x86_ctx_t *ctx);

void x86_isa_init(void);
void x86_isa_done(void);

void x86_isa_execute_inst(struct x86_ctx_t *ctx);

void x86_isa_trace_call_init(char *filename);
void x86_isa_trace_call_done(void);

void x86_isa_inst_stat_dump(FILE *f);
void x86_isa_inst_stat_reset(void);




/*
 * System calls
 */

#define x86_sys_debug(...) debug(x86_sys_debug_category, __VA_ARGS__)
#define x86_sys_debug_buffer(...) debug_buffer(x86_sys_debug_category, __VA_ARGS__)
extern int x86_sys_debug_category;

void x86_sys_init(void);
void x86_sys_done(void);
void x86_sys_dump(FILE *f);

void x86_sys_call(void);
 



/*
 * GLUT system call interface
 *
 * NOTE: for every new function or external variable added here, its
 * implementation should be added in the regular 'glut-xx.c' files and also in
 * 'glut-missing.c' to allow for correct compilation when the GLUT library is
 * missing in the user's system.
 */

#define x86_glut_debug(...) debug(x86_glut_debug_category, __VA_ARGS__)
extern int x86_glut_debug_category;

extern pthread_mutex_t x86_glut_mutex;

void x86_glut_init(void);
void x86_glut_done(void);

int x86_glut_call(void);


void x86_glut_frame_buffer_init(void);
void x86_glut_frame_buffer_done(void);

void x86_glut_frame_buffer_clear(void);
void x86_glut_frame_buffer_pixel(int x, int y, int color);

void x86_glut_frame_buffer_resize(int width, int height);
void x86_glut_frame_buffer_get_size(int *width, int *height);

void x86_glut_frame_buffer_flush_request(void);
void x86_glut_frame_buffer_flush_if_requested(void);




/*
 * OPENGL system call interface
 *
 * NOTE: any additional function added to this interface should be implemented
 * both in 'opengl.c' and 'opengl-missing.c' to allow for correct compilation on
 * systems lacking the OpenGL and GLUT libraries.
 */

#define x86_opengl_debug(...) debug(x86_opengl_debug_category, __VA_ARGS__)
extern int x86_opengl_debug_category;

void x86_opengl_init(void);
void x86_opengl_done(void);

int x86_opengl_call(void);




/*
 * OpenCL Runtime Interface
 */

#define x86_clrt_debug(...) debug(x86_clrt_debug_category, __VA_ARGS__)
extern int x86_clrt_debug_category;

void x86_clrt_init(void);
void x86_clrt_done(void);

int x86_clrt_call(void);




/*
 * CUDA system call interface
 */

int frm_cuda_call(void);




/*
 * System signals
 */


/* Every contexts (parent and children) has its own masks */
struct signal_mask_table_t
{
	unsigned long long pending;  /* Mask of pending signals */
	unsigned long long blocked;  /* Mask of blocked signals */
	unsigned long long backup;  /* Backup of blocked signals while suspended */
	struct x86_regs_t *regs;  /* Backup of regs while executing handler */
	unsigned int pretcode;  /* Base address of a memory page allocated for retcode execution */
};

struct signal_mask_table_t *signal_mask_table_create(void);
void signal_mask_table_free(struct signal_mask_table_t *table);


struct signal_handler_table_t
{
	/* Number of extra contexts sharing the table */
	int num_links;

	/* Signal handlers */
	struct sim_sigaction
	{
		unsigned int handler;
		unsigned int flags;
		unsigned int restorer;
		unsigned long long mask;
	} sigaction[64];
};

struct signal_handler_table_t *signal_handler_table_create(void);
void signal_handler_table_free(struct signal_handler_table_t *table);

struct signal_handler_table_t *signal_handler_table_link(struct signal_handler_table_t *table);
void signal_handler_table_unlink(struct signal_handler_table_t *table);

void signal_handler_run(struct x86_ctx_t *ctx, int sig);
void signal_handler_return(struct x86_ctx_t *ctx);
void signal_handler_check(struct x86_ctx_t *ctx);
void signal_handler_check_intr(struct x86_ctx_t *ctx);

char *sim_signal_name(int signum);
void sim_sigaction_dump(struct sim_sigaction *sim_sigaction, FILE *f);
void sim_sigaction_flags_dump(unsigned int flags, FILE *f);
void sim_sigset_dump(unsigned long long sim_sigset, FILE *f);
void sim_sigset_add(unsigned long long *sim_sigset, int signal);
void sim_sigset_del(unsigned long long *sim_sigset, int signal);
int sim_sigset_member(unsigned long long *sim_sigset, int signal);




/*
 * File management
 */


enum file_desc_kind_t
{
	file_desc_invalid = 0,
	file_desc_regular,  /* Regular file */
	file_desc_std,  /* Standard input or output */
	file_desc_pipe,  /* A pipe */
	file_desc_virtual,  /* A virtual file with artificial contents */
	file_desc_gpu,  /* GPU device */
	file_desc_socket  /* Network socket */
};


/* File descriptor */
struct file_desc_t
{
	enum file_desc_kind_t kind;  /* File type */
	int guest_fd;  /* Guest file descriptor id */
	int host_fd;  /* Equivalent open host file */
	int flags;  /* O_xxx flags */
	char *path;  /* Associated path if applicable */
};


/* File descriptor table */
struct file_desc_table_t
{
	/* Number of extra contexts sharing table */
	int num_links;

	/* List of descriptors */
	struct list_t *file_desc_list;
};


struct file_desc_table_t *file_desc_table_create(void);
void file_desc_table_free(struct file_desc_table_t *table);

struct file_desc_table_t *file_desc_table_link(struct file_desc_table_t *table);
void file_desc_table_unlink(struct file_desc_table_t *table);

void file_desc_table_dump(struct file_desc_table_t *table, FILE *f);

struct file_desc_t *file_desc_table_entry_get(struct file_desc_table_t *table, int index);
struct file_desc_t *file_desc_table_entry_new(struct file_desc_table_t *table,
	enum file_desc_kind_t kind, int host_fd, char *path, int flags);
struct file_desc_t *file_desc_table_entry_new_guest_fd(struct file_desc_table_t *table,
        enum file_desc_kind_t kind, int guest_fd, int host_fd, char *path, int flags);
void file_desc_table_entry_free(struct file_desc_table_t *table, int index);
void file_desc_table_entry_dump(struct file_desc_table_t *table, int index, FILE *f);

int file_desc_table_get_host_fd(struct file_desc_table_t *table, int guest_fd);
int file_desc_table_get_guest_fd(struct file_desc_table_t *table, int host_fd);




/*
 * x86 Context
 */

#define x86_ctx_debug(...) debug(x86_ctx_debug_category, __VA_ARGS__)
extern int x86_ctx_debug_category;

/* Event scheduled periodically to dump IPC statistics for a context */
extern int EV_X86_CTX_IPC_REPORT;

typedef int (*x86_ctx_can_wakeup_callback_func_t)(struct x86_ctx_t *ctx, void *data);
typedef void (*x86_ctx_wakeup_callback_func_t)(struct x86_ctx_t *ctx, void *data);

struct x86_ctx_t
{
	/* Context properties */
	int status;
	int pid;  /* Context ID */
	int address_space_index;  /* Virtual memory address space index */

	/* Parent context */
	struct x86_ctx_t *parent;

	/* Context group initiator. There is only one group parent (if not NULL)
	 * with many group children, no tree organization. */
	struct x86_ctx_t *group_parent;

	int exit_signal;  /* Signal to send parent when finished */
	int exit_code;  /* For zombie contexts */

	unsigned int clear_child_tid;
	unsigned int robust_list_head;  /* robust futex list */

	/* For emulation of string operations */
	unsigned int last_eip;  /* Eip of last emulated instruction */
	unsigned int str_op_esi;  /* Initial value for register 'esi' in string operation */
	unsigned int str_op_edi;  /* Initial value for register 'edi' in string operation */
	int str_op_dir;  /* Direction: 1 = forward, -1 = backward */
	int str_op_count;  /* Number of iterations in string operation */

	/* Allocation to hardware threads */
	long long alloc_when;  /* esim_cycle of allocation */
	long long dealloc_when;  /* esim_cycle of deallocation */
	int alloc_core, alloc_thread;  /* core/thread id of last allocation */
	int dealloc_signal;  /* signal to deallocate context */

	/* For segmented memory access in glibc */
	unsigned int glibc_segment_base;
	unsigned int glibc_segment_limit;

	/* For the OpenCL library access */
	int libopencl_open_attempt;

	/* Host thread that suspends and then schedules call to 'x86_emu_process_events'. */
	/* The 'host_thread_suspend_active' flag is set when a 'host_thread_suspend' thread
	 * is launched for this context (by caller).
	 * It is clear when the context finished (by the host thread).
	 * It should be accessed safely by locking global mutex 'x86_emu->process_events_mutex'. */
	pthread_t host_thread_suspend;  /* Thread */
	int host_thread_suspend_active;  /* Thread-spawned flag */

	/* Host thread that lets time elapse and schedules call to 'x86_emu_process_events'. */
	pthread_t host_thread_timer;  /* Thread */
	int host_thread_timer_active;  /* Thread-spawned flag */
	long long host_thread_timer_wakeup;  /* Time when the thread will wake up */

	/* Three timers used by 'setitimer' system call - real, virtual, and prof. */
	long long itimer_value[3];  /* Time when current occurrence of timer expires (0=inactive) */
	long long itimer_interval[3];  /* Interval (in usec) of repetition (0=inactive) */

	/* Variables used to wake up suspended contexts. */
	long long wakeup_time;  /* x86_emu_timer time to wake up (poll/nanosleep) */
	int wakeup_fd;  /* File descriptor (read/write/poll) */
	int wakeup_events;  /* Events for wake up (poll) */
	int wakeup_pid;  /* Pid waiting for (waitpid) */
	unsigned int wakeup_futex;  /* Address of futex where context is suspended */
	unsigned int wakeup_futex_bitset;  /* Bit mask for selective futex wakeup */
	long long wakeup_futex_sleep;  /* Assignment from x86_emu->futex_sleep_count */

	/* Generic callback function (and data to pass to it) to call when a
	 * context gets suspended in a system call to check whether it should be
	 * waken up, and once it is waken up, respectively */
	x86_ctx_can_wakeup_callback_func_t can_wakeup_callback_func;
	x86_ctx_wakeup_callback_func_t wakeup_callback_func;
	void *can_wakeup_callback_data;
	void *wakeup_callback_data;

	/* Links to contexts forming a linked list. */
	struct x86_ctx_t *context_list_next, *context_list_prev;
	struct x86_ctx_t *running_list_next, *running_list_prev;
	struct x86_ctx_t *suspended_list_next, *suspended_list_prev;
	struct x86_ctx_t *finished_list_next, *finished_list_prev;
	struct x86_ctx_t *zombie_list_next, *zombie_list_prev;
	struct x86_ctx_t *alloc_list_next, *alloc_list_prev;

	/* Substructures */
	struct x86_loader_t *loader;
	struct mem_t *mem;  /* Virtual memory image */
	struct spec_mem_t *spec_mem;  /* Speculative memory */
	struct x86_regs_t *regs;  /* Logical register file */
	struct x86_regs_t *backup_regs;  /* Backup when entering in speculative mode */
	struct file_desc_table_t *file_desc_table;  /* File descriptor table */
	struct signal_mask_table_t *signal_mask_table;
	struct signal_handler_table_t *signal_handler_table;


	/* Statistics */

	/* Number of non-speculate micro-instructions.
	 * Updated by the architectural simulator at the commit stage. */
	long long inst_count;
};

enum x86_ctx_status_t
{
	x86_ctx_running      = 0x00001,  /* it is able to run instructions */
	x86_ctx_spec_mode     = 0x00002,  /* executing in speculative mode */
	x86_ctx_suspended    = 0x00004,  /* suspended in a system call */
	x86_ctx_finished     = 0x00008,  /* no more inst to execute */
	x86_ctx_exclusive    = 0x00010,  /* executing in excl mode */
	x86_ctx_locked       = 0x00020,  /* another context is running in excl mode */
	x86_ctx_handler      = 0x00040,  /* executing a signal handler */
	x86_ctx_sigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	x86_ctx_nanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	x86_ctx_poll         = 0x00200,  /* 'poll' system call */
	x86_ctx_read         = 0x00400,  /* 'read' system call */
	x86_ctx_write        = 0x00800,  /* 'write' system call */
	x86_ctx_waitpid      = 0x01000,  /* 'waitpid' system call */
	x86_ctx_zombie       = 0x02000,  /* zombie context */
	x86_ctx_futex        = 0x04000,  /* suspended in a futex */
	x86_ctx_alloc        = 0x08000,  /* allocated to a core/thread */
	x86_ctx_callback     = 0x10000,  /* suspended after syscall with callback */
	x86_ctx_none         = 0x00000
};

struct x86_ctx_t *x86_ctx_create(void);
void x86_ctx_free(struct x86_ctx_t *ctx);

void x86_ctx_dump(struct x86_ctx_t *ctx, FILE *f);

struct x86_ctx_t *x86_ctx_clone(struct x86_ctx_t *ctx);
struct x86_ctx_t *x86_ctx_fork(struct x86_ctx_t *ctx);

/* Thread safe/unsafe versions */
void __x86_ctx_host_thread_suspend_cancel(struct x86_ctx_t *ctx);
void x86_ctx_host_thread_suspend_cancel(struct x86_ctx_t *ctx);
void __x86_ctx_host_thread_timer_cancel(struct x86_ctx_t *ctx);
void x86_ctx_host_thread_timer_cancel(struct x86_ctx_t *ctx);

void x86_ctx_suspend(struct x86_ctx_t *ctx, x86_ctx_can_wakeup_callback_func_t can_wakeup_callback_func,
	void *can_wakeup_callback_data, x86_ctx_wakeup_callback_func_t wakeup_callback_func,
	void *wakeup_callback_data);

void x86_ctx_finish(struct x86_ctx_t *ctx, int status);
void x86_ctx_finish_group(struct x86_ctx_t *ctx, int status);
void x86_ctx_execute_inst(struct x86_ctx_t *ctx);

void x86_ctx_set_eip(struct x86_ctx_t *ctx, uint32_t eip);
void x86_ctx_recover(struct x86_ctx_t *ctx);

struct x86_ctx_t *x86_ctx_get(int pid);
struct x86_ctx_t *x86_ctx_get_zombie(struct x86_ctx_t *parent, int pid);

int x86_ctx_get_status(struct x86_ctx_t *ctx, enum x86_ctx_status_t status);
void x86_ctx_set_status(struct x86_ctx_t *ctx, enum x86_ctx_status_t status);
void x86_ctx_clear_status(struct x86_ctx_t *ctx, enum x86_ctx_status_t status);

int x86_ctx_futex_wake(struct x86_ctx_t *ctx, uint32_t futex, uint32_t count, uint32_t bitset);
void x86_ctx_exit_robust_list(struct x86_ctx_t *ctx);

void x86_ctx_gen_proc_self_maps(struct x86_ctx_t *ctx, char *path);

void x86_ctx_ipc_report_schedule(struct x86_ctx_t *ctx);
void x86_ctx_ipc_report_handler(int event, void *data);


enum gpu_emulator_kind_t 
{
	gpu_emulator_evg = 0,
	gpu_emulator_si
};


/*
 * x86 CPU Emulator
 */

struct x86_emu_t
{
	/* pid & address_space_index assignment */
	int current_pid;

	/* Timer for emulator activity */
	struct m2s_timer_t *timer;

	/* Schedule next call to 'x86_emu_process_events()'.
	 * The call will only be effective if 'process_events_force' is set.
	 * This flag should be accessed thread-safely locking 'process_events_mutex'. */
	pthread_mutex_t process_events_mutex;
	int process_events_force;

	/* Counter of times that a context has been suspended in a
	 * futex. Used for FIFO wakeups. */
	long long futex_sleep_count;
	
	/* Flag set when any context changes any status other than 'specmode' */
	int context_reschedule;

	/* List of contexts */
	struct x86_ctx_t *context_list_head;
	struct x86_ctx_t *context_list_tail;
	int context_list_count;
	int context_list_max;

	/* List of running contexts */
	struct x86_ctx_t *running_list_head;
	struct x86_ctx_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of suspended contexts */
	struct x86_ctx_t *suspended_list_head;
	struct x86_ctx_t *suspended_list_tail;
	int suspended_list_count;
	int suspended_list_max;

	/* List of zombie contexts */
	struct x86_ctx_t *zombie_list_head;
	struct x86_ctx_t *zombie_list_tail;
	int zombie_list_count;
	int zombie_list_max;

	/* List of finished contexts */
	struct x86_ctx_t *finished_list_head;
	struct x86_ctx_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;

	/* List of allocated contexts */
	struct x86_ctx_t *alloc_list_head;
	struct x86_ctx_t *alloc_list_tail;
	int alloc_list_count;
	int alloc_list_max;

	/* Stats */
	long long inst_count;  /* Number of emulated instructions */

	/* Determines which GPU emulator will be called */
	enum gpu_emulator_kind_t gpu_emulator;
};

enum x86_emu_list_kind_t
{
	x86_emu_list_context = 0,
	x86_emu_list_running,
	x86_emu_list_suspended,
	x86_emu_list_zombie,
	x86_emu_list_finished,
	x86_emu_list_alloc
};

void x86_emu_list_insert_head(enum x86_emu_list_kind_t list, struct x86_ctx_t *ctx);
void x86_emu_list_insert_tail(enum x86_emu_list_kind_t list, struct x86_ctx_t *ctx);
void x86_emu_list_remove(enum x86_emu_list_kind_t list, struct x86_ctx_t *ctx);
int x86_emu_list_member(enum x86_emu_list_kind_t list, struct x86_ctx_t *ctx);


/* Reason for simulation end */
extern struct string_map_t x86_emu_finish_map;

extern volatile enum x86_emu_finish_t
{
	x86_emu_finish_none,  /* Simulation not finished */
	x86_emu_finish_ctx,  /* Contexts finished */
	x86_emu_finish_last_cpu_inst_bytes, /* Last CPU instruction reached */
	x86_emu_finish_max_cpu_inst,  /* Maximum instruction count reached in CPU */
	x86_emu_finish_max_cpu_cycles,  /* Maximum cycle count reached in CPU */
	x86_emu_finish_max_gpu_inst,  /* Maximum instruction count reached in GPU */
	x86_emu_finish_max_gpu_cycles,  /* Maximum cycle count reached in GPU */
	x86_emu_finish_max_gpu_kernels,  /* Maximum number of GPU kernels */
	x86_emu_finish_max_time,  /* Maximum simulation time reached */
	x86_emu_finish_signal,  /* Signal received */
	x86_emu_finish_stall,  /* Simulation stalled */
	x86_emu_finish_gpu_no_faults  /* GPU-REL: no fault in '--evg_gpu-stack-faults' caused error */
} x86_emu_finish;


/* Global CPU emulator variable */
extern struct x86_emu_t *x86_emu;

extern long long x86_emu_max_cycles;
extern long long x86_emu_max_inst;
extern long long x86_emu_max_time;
extern char * x86_emu_last_inst_bytes;

extern enum x86_emu_kind_t
{
	x86_emu_kind_functional,
	x86_emu_kind_detailed
} x86_emu_kind;


void x86_emu_init(void);
void x86_emu_done(void);
void x86_emu_run(void);
void x86_emu_disasm(char *file_name);

void x86_emu_dump(FILE *f);

void x86_emu_process_events(void);
void x86_emu_process_events_schedule(void);



#endif

