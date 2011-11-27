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

#ifndef CPUKERNEL_H
#define CPUKERNEL_H

#include <mhandle.h>
#include <debug.h>
#include <config.h>
#include <buffer.h>
#include <list.h>
#include <lnlist.h>
#include <misc.h>
#include <elf-format.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <cpudisasm.h>
#include <time.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>
#include <gpukernel.h>
#include <sys/time.h>


/*
 * Global variables
 */

extern uint64_t ke_max_cycles;
extern uint64_t ke_max_inst;
extern uint64_t ke_max_time;

extern enum cpu_sim_kind_t {
	cpu_sim_kind_functional,
	cpu_sim_kind_detailed
} cpu_sim_kind;


/* Reason for simulation end */
extern struct string_map_t ke_sim_finish_map;

extern enum ke_sim_finish_t
{
	ke_sim_finish_none,  /* Simulation not finished */
	ke_sim_finish_ctx,  /* Contexts finished */
	ke_sim_finish_max_cpu_inst,  /* Maximum instruction count reached in CPU */
	ke_sim_finish_max_cpu_cycles,  /* Maximum cycle count reached in CPU */
	ke_sim_finish_max_gpu_inst,  /* Maximum instruction count reached in GPU */
	ke_sim_finish_max_gpu_cycles,  /* Maximum cycle count reached in GPU */
	ke_sim_finish_max_gpu_kernels,  /* Maximum number of GPU kernels */
	ke_sim_finish_max_time,  /* Maximum simulation time reached */
	ke_sim_finish_signal,  /* Signal received */
	ke_sim_finish_gpu_no_faults  /* GPU-REL: no fault in '--gpu-stack-faults' caused error */
} ke_sim_finish;



/* Some forward declarations */
struct ctx_t;
struct fd_t;




/*
 * Memory
 */

#define MEM_LOGPAGESIZE    12
#define MEM_PAGESHIFT      MEM_LOGPAGESIZE
#define MEM_PAGESIZE       (1 << MEM_LOGPAGESIZE)
#define MEM_PAGEMASK       (~(MEM_PAGESIZE - 1))
#define MEM_PAGE_COUNT     1024

enum mem_access_t
{
	mem_access_none   = 0x00,
	mem_access_read   = 0x01,
	mem_access_write  = 0x02,
	mem_access_exec   = 0x04,
	mem_access_init   = 0x08,
	mem_access_modif  = 0x10
};

/* Safe mode */
extern int mem_safe_mode;

/* Host mapping: mappings performed with file descriptors other than -1 */
struct mem_host_mapping_t
{
	void *host_ptr;  /* Pointer to the host memory space */
	uint32_t addr;  /* Guest range base */
	uint32_t size;  /* Host mapping size */
	int pages;  /* Number of allocated pages left */
	char path[MAX_PATH_SIZE];  /* Path of the mapped file */
	struct mem_host_mapping_t *next;  /* Linked list */
};

/* A 4KB page of memory */
struct mem_page_t
{
	uint32_t tag;
	enum mem_access_t perm;  /* Access permissions; combination of flags */
	struct mem_page_t *next;
	unsigned char *data;
	struct mem_host_mapping_t *host_mapping;  /* If other than null, page is host mapping */
};

struct mem_t
{
	struct mem_page_t *pages[MEM_PAGE_COUNT];
	int sharing;  /* Number of contexts sharing memory map */
	uint32_t last_address;  /* Address of last access */
	int safe;  /* Safe mode */
	struct mem_host_mapping_t *host_mapping_list;  /* List of host mappings */
};

extern unsigned long mem_mapped_space;
extern unsigned long mem_max_mapped_space;

struct mem_t *mem_create(void);
void mem_free(struct mem_t *mem);

struct mem_page_t *mem_page_get(struct mem_t *mem, uint32_t addr);
struct mem_page_t *mem_page_get_next(struct mem_t *mem, uint32_t addr);

uint32_t mem_map_space(struct mem_t *mem, uint32_t addr, int size);
uint32_t mem_map_space_down(struct mem_t *mem, uint32_t addr, int size);

void mem_map(struct mem_t *mem, uint32_t addr, int size, enum mem_access_t perm);
void mem_unmap(struct mem_t *mem, uint32_t addr, int size);

void mem_map_host(struct mem_t *mem, struct fd_t *fd, uint32_t addr,
	int size, enum mem_access_t perm, void *data);
void mem_unmap_host(struct mem_t *mem, uint32_t addr);

void mem_protect(struct mem_t *mem, uint32_t addr, int size, enum mem_access_t perm);
void mem_copy(struct mem_t *mem, uint32_t dest, uint32_t src, int size);

void mem_access(struct mem_t *mem, uint32_t addr, int size, void *buf, enum mem_access_t access);
void mem_read(struct mem_t *mem, uint32_t addr, int size, void *buf);
void mem_write(struct mem_t *mem, uint32_t addr, int size, void *buf);

void mem_zero(struct mem_t *mem, uint32_t addr, int size);
int mem_read_string(struct mem_t *mem, uint32_t addr, int size, char *str);
void mem_write_string(struct mem_t *mem, uint32_t addr, char *str);
void *mem_get_buffer(struct mem_t *mem, uint32_t addr, int size, enum mem_access_t access);

void mem_dump(struct mem_t *mem, char *filename, uint32_t start, uint32_t end);
void mem_load(struct mem_t *mem, char *filename, uint32_t start);




/*
 * X86 Registers
 */

struct regs_t
{
	/* Integer registers */
	uint32_t eax, ecx, edx, ebx;
	uint32_t esp, ebp, esi, edi;
	uint16_t es, cs, ss, ds, fs, gs;
	uint32_t eip;
	uint32_t eflags;

	/* Floating-point unit */
	struct {
		unsigned char value[10];
		int valid;
	} fpu_stack[8];
	int fpu_top;  /* top of stack (field 'top' of status register) */
	int fpu_code;  /* field 'code' of status register (C3-C2-C1-C0) */
	uint16_t fpu_ctrl;  /* fpu control word */

	/* XMM registers (8 128-bit regs) */
	unsigned char xmm[8][16];

} __attribute__((packed));

struct regs_t *regs_create(void);
void regs_free(struct regs_t *regs);

void regs_copy(struct regs_t *dst, struct regs_t *src);
void regs_dump(struct regs_t *regs, FILE *f);
void regs_fpu_stack_dump(struct regs_t *regs, FILE *f);




/*
 * Program loader
 */

struct loader_t
{
	/* Program data */
	struct elf_file_t *elf_file;
	struct lnlist_t *args;
	struct lnlist_t *env;
	char *interp;  /* Executable interpreter */
	char *exe;  /* Executable file name */
	char *cwd;  /* Current working directory */
	char *stdin_file, *stdout_file;  /* File names for stdin and stdout */

	/* Code pointers */
	uint32_t stack_base, stack_top, stack_size;
	uint32_t text_size;
	uint32_t environ_base, brk, bottom;
	uint32_t prog_entry, interp_prog_entry;
	uint32_t phdt_base, phdr_count;
};


#define ld_debug(...) debug(ld_debug_category, __VA_ARGS__)
extern int ld_debug_category;

extern char *ld_help_ctxconfig;

void ld_init(struct ctx_t *ctx);
void ld_done(struct ctx_t *ctx);

void ld_add_args(struct ctx_t *ctx, int argc, char **argv);
void ld_add_cmdline(struct ctx_t *ctx, char *cmdline);
void ld_set_cwd(struct ctx_t *ctx, char *cwd);
void ld_set_redir(struct ctx_t *ctx, char *stdin, char *stdout);
void ld_load_exe(struct ctx_t *ctx, char *exe);

void ld_load_prog_from_ctxconfig(char *ctxconfig);
void ld_load_prog_from_cmdline(int argc, char **argv);

void ld_convert_filename(struct ctx_t *ctx, char *filename);
void ld_get_full_path(struct ctx_t *ctx, char *filename, char *fullpath, int size);





/*
 * Microinstructions
 */


/* Micro-instruction dependences.
 * WARNING: update 'x86_uinst_dep_name' if modified (uinst.c).
 * WARNING: also update 'x86_uinst_dep_map' if modified (uinst.c). */
enum x86_dep_t {

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
	X86_UINST_INT		= 0x001,
	X86_UINST_LOGIC		= 0x002,
	X86_UINST_FP		= 0x004,
	X86_UINST_MEM		= 0x008,
	X86_UINST_CTRL		= 0x010,
	X86_UINST_XMM		= 0x020
};


/* Micro-instruction opcodes.
 * WARNING: update 'x86_uinst_info' if modified (uinst.c).
 * WARNING: also update 'fu_class_table' if modified (src/libcpuarch/fu.c). */
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
	x86_uinst_fp_simple,
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

	x86_uinst_xmm_move,
	x86_uinst_xmm_conv,

	x86_uinst_load,
	x86_uinst_store,

	x86_uinst_call,
	x86_uinst_ret,
	x86_uinst_jump,
	x86_uinst_branch,

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
	uint32_t address;
	int size;
};


extern struct list_t *x86_uinst_list;

void x86_uinst_init(void);
void x86_uinst_done(void);

struct x86_uinst_t *x86_uinst_create(void);
void x86_uinst_free(struct x86_uinst_t *uinst);

/* To prevent performance degradation in functional simulation, do the check before the actual
 * function call. Notice that 'x86_uinst_new' calls are done for every x86 instruction emulation. */
#define x86_uinst_new(opcode, idep0, idep1, idep2, odep0, odep1, odep2, odep3) \
	{ if (cpu_sim_kind == cpu_sim_kind_detailed) \
	__x86_uinst_new(opcode, idep0, idep1, idep2, odep0, odep1, odep2, odep3); }
#define x86_uinst_new_mem(opcode, addr, size, idep0, idep1, idep2, odep0, odep1, odep2, odep3) \
	{ if (cpu_sim_kind == cpu_sim_kind_detailed) \
	__x86_uinst_new_mem(opcode, addr, size, idep0, idep1, idep2, odep0, odep1, odep2, odep3); }
#define x86_uinst_new_move(idep, odep) \
	{ if (cpu_sim_kind == cpu_sim_kind_detailed) \
	__x86_uinst_new_move(idep, odep); }

void __x86_uinst_new(enum x86_uinst_opcode_t opcode,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3);
void __x86_uinst_new_mem(enum x86_uinst_opcode_t opcode, uint32_t addr, int size,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3);
void __x86_uinst_new_move(enum x86_dep_t idep, enum x86_dep_t odep);
void x86_uinst_clear(void);

void x86_uinst_dump_buf(struct x86_uinst_t *uinst, char *buf, int size);
void x86_uinst_dump(struct x86_uinst_t *uinst, FILE *f);
void x86_uinst_list_dump(FILE *f);




/*
 * Machine & ISA
 */

extern struct ctx_t *isa_ctx;
extern struct regs_t *isa_regs;
extern struct mem_t *isa_mem;
extern int isa_spec_mode;
extern uint32_t isa_eip;
extern uint32_t isa_target;
extern struct x86_inst_t isa_inst;
extern uint64_t isa_inst_count;
extern int isa_function_level;

#define isa_call_debug(...) debug(isa_call_debug_category, __VA_ARGS__)
#define isa_inst_debug(...) debug(isa_inst_debug_category, __VA_ARGS__)
extern int isa_call_debug_category;
extern int isa_inst_debug_category;

extern long isa_host_flags;
#define __ISA_ASM_START__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	: "=m" (isa_host_flags));
#define __ISA_ASM_END__ asm volatile ( \
	"push %0\n\t" "popf\n\t" \
	: "=m" (isa_host_flags));

extern uint16_t isa_host_fpcw;
extern uint16_t isa_guest_fpcw;
#define __ISA_FP_ASM_START__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	"fstcw %1\n\t" \
	"fldcw %2\n\t" \
	: "=m" (isa_host_flags), "=m" (isa_host_fpcw) \
	: "m" (isa_guest_fpcw));
#define __ISA_FP_ASM_END__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	"fnclex\n\t" \
	"fldcw %1\n\t" \
	: "=m" (isa_host_flags) \
	: "m" (isa_host_fpcw));

/* References to functions emulating x86 instructions */
#define DEFINST(name,op1,op2,op3,modrm,imm,pfx) void op_##name##_impl(void);
#include <machine.dat>
#undef DEFINST

void isa_error(char *fmt, ...);

void isa_mem_read(struct mem_t *mem, uint32_t addr, int size, void *buf);
void isa_mem_write(struct mem_t *mem, uint32_t addr, int size, void *buf);

void isa_dump_flags(FILE *f);
void isa_set_flag(enum x86_flag_t flag);
void isa_clear_flag(enum x86_flag_t flag);
int isa_get_flag(enum x86_flag_t flag);

uint32_t isa_load_reg(enum x86_reg_t reg);
void isa_store_reg(enum x86_reg_t reg, uint32_t value);

uint8_t isa_load_rm8(void);
uint16_t isa_load_rm16(void);
uint32_t isa_load_rm32(void);
uint64_t isa_load_m64(void);
void isa_store_rm8(uint8_t value);
void isa_store_rm16(uint16_t value);
void isa_store_rm32(uint32_t value);
void isa_store_m64(uint64_t value);

#define isa_load_r8() isa_load_reg(isa_inst.reg + x86_reg_al)
#define isa_load_r16() isa_load_reg(isa_inst.reg + x86_reg_ax)
#define isa_load_r32() isa_load_reg(isa_inst.reg + x86_reg_eax)
#define isa_load_sreg() isa_load_reg(isa_inst.reg + x86_reg_es)
#define isa_store_r8(value) isa_store_reg(isa_inst.reg + x86_reg_al, value)
#define isa_store_r16(value) isa_store_reg(isa_inst.reg + x86_reg_ax, value)
#define isa_store_r32(value) isa_store_reg(isa_inst.reg + x86_reg_eax, value)
#define isa_store_sreg(value) isa_store_reg(isa_inst.reg + x86_reg_es, value)

#define isa_load_ir8() isa_load_reg(isa_inst.opindex + x86_reg_al)
#define isa_load_ir16() isa_load_reg(isa_inst.opindex + x86_reg_ax)
#define isa_load_ir32() isa_load_reg(isa_inst.opindex + x86_reg_eax)
#define isa_store_ir8(value) isa_store_reg(isa_inst.opindex + x86_reg_al, value)
#define isa_store_ir16(value) isa_store_reg(isa_inst.opindex + x86_reg_ax, value)
#define isa_store_ir32(value) isa_store_reg(isa_inst.opindex + x86_reg_eax, value)

void isa_load_fpu(int index, uint8_t *value);
void isa_store_fpu(int index, uint8_t *value);
void isa_pop_fpu(uint8_t *value);
void isa_push_fpu(uint8_t *value);

float isa_load_float(void);
double isa_load_double(void);
void isa_load_extended(uint8_t *value);
void isa_store_float(float value);
void isa_store_double(double value);
void isa_store_extended(uint8_t *value);

void isa_dump_xmm(uint8_t *value, FILE *f);
void isa_load_xmm(uint8_t *value);
void isa_store_xmm(uint8_t *value);
void isa_load_xmmm32(uint8_t *value);
void isa_store_xmmm32(uint8_t *value);
void isa_load_xmmm64(uint8_t *value);
void isa_store_xmmm64(uint8_t *value);

void isa_double_to_extended(double f, uint8_t *e);
double isa_extended_to_double(uint8_t *e);
void isa_float_to_extended(float f, uint8_t *e);
float isa_extended_to_float(uint8_t *e);

void isa_store_fpu_code(uint16_t status);
uint16_t isa_load_fpu_status(void);

uint32_t isa_effective_address(void);
uint32_t isa_moffs_address(void);

void isa_init(void);
void isa_done(void);
void isa_dump(FILE *f);

void isa_execute_inst(void);

void isa_trace_call_init(char *filename);
void isa_trace_call_done(void);

void isa_inst_stat_dump(FILE *f);
void isa_inst_stat_reset(void);




/*
 * System calls
 */

#define syscall_debug(...) debug(syscall_debug_category, __VA_ARGS__)
extern int syscall_debug_category;

void syscall_do(void);
void syscall_summary(void);




/*
 * System signals
 */


/* Every contexts (parent and children) has its own masks */
struct signal_masks_t
{
	uint64_t pending;  /* mask of pending signals */
	uint64_t blocked;  /* mask of blocked signals */
	uint64_t backup;  /* backup of blocked signals while suspended */
	struct regs_t *regs;  /* backup of regs while executing handler */
	uint32_t pretcode;  /* base address of a memory page allocated for retcode execution */
};

struct signal_masks_t *signal_masks_create(void);
void signal_masks_free(struct signal_masks_t *signal_masks);


/* This structure is shared for parent and child contexts. A change
 * in the singal handler by any of them affects all of them. */
struct signal_handlers_t
{
	struct sim_sigaction {
		uint32_t handler;
		uint32_t flags;
		uint32_t restorer;
		uint64_t mask;
	} sigaction[64];
};

struct signal_handlers_t *signal_handlers_create(void);
void signal_handlers_free(struct signal_handlers_t *signal_handlers);

void signal_handler_run(struct ctx_t *ctx, int sig);
void signal_handler_return(struct ctx_t *ctx);
void signal_handler_check(struct ctx_t *ctx);
void signal_handler_check_intr(struct ctx_t *ctx);

char *sim_signal_name(int signum);
void sim_sigaction_dump(struct sim_sigaction *sim_sigaction, FILE *f);
void sim_sigaction_flags_dump(uint32_t flags, FILE *f);
void sim_sigset_dump(uint64_t sim_sigset, FILE *f);
void sim_sigset_add(uint64_t *sim_sigset, int signal);
void sim_sigset_del(uint64_t *sim_sigset, int signal);
int sim_sigset_member(uint64_t *sim_sigset, int signal);




/*
 * File management
 */


enum fd_kind_t
{
	fd_kind_regular = 0,  /* Regular file */
	fd_kind_std,  /* Standard input or output */
	fd_kind_pipe,  /* A pipe */
	fd_kind_virtual,  /* A virtual file with artificial contents */
	fd_kind_gpu,  /* GPU device */
	fd_kind_socket  /* Network socket */
};


/* File descriptor */
struct fd_t
{
	enum fd_kind_t kind;  /* File type */
	int guest_fd;  /* Guest file descriptor id */
	int host_fd;  /* Equivalent open host file */
	char path[MAX_PATH_SIZE];  /* Equivalent path if applicable */
	int flags;  /* O_xxx flags */
};


/* File descriptor table */
struct fdt_t
{
	struct list_t *fd_list;  /* List of file descriptors (fd_t elements) */
};


struct fdt_t *fdt_create(void);
void fdt_free(struct fdt_t *fdt);
void fdt_dump(struct fdt_t *fdt, FILE *f);

struct fd_t *fdt_entry_get(struct fdt_t *fdt, int index);
struct fd_t *fdt_entry_new(struct fdt_t *fdt, enum fd_kind_t kind, int host_fd, char *path, int flags);
void fdt_entry_free(struct fdt_t *fdt, int index);
void fdt_entry_dump(struct fdt_t *fdt, int index, FILE *f);

int fdt_get_host_fd(struct fdt_t *fdt, int guest_fd);
int fdt_get_guest_fd(struct fdt_t *fdt, int host_fd);




/*
 * Context
 */

#define ctx_debug(...) debug(ctx_debug_category, __VA_ARGS__)
extern int ctx_debug_category;

struct ctx_t
{
	/* Context properties */
	int status;
	int pid;  /* Context id */
	int mid;  /* Memory id - the same for contexts sharing memory map */
	struct ctx_t *parent;
	int exit_signal;  /* Signal to send parent when finished */
	int exit_code;  /* For zombie processes */
	uint32_t set_child_tid, clear_child_tid;
	uint32_t robust_list_head;  /* robust futex list */
	uint32_t initial_stack;  /* Value of esp when context is cloned */

	/* Allocation to hardware threads */
	uint64_t alloc_when;  /* esim_cycle of allocation */
	uint64_t dealloc_when;  /* esim_cycle of deallocation */
	int alloc_core, alloc_thread;  /* core/thread id of last allocation */
	int dealloc_signal;  /* signal to deallocate context */

	/* For segmented memory access in glibc */
	uint32_t glibc_segment_base;
	uint32_t glibc_segment_limit;

	/* For the OpenCL library access */
	int libopencl_open_attempt;

	/* Host thread that suspends and then schedules call to 'ke_process_events'. */
	/* The 'host_thread_suspend_active' flag is set when a 'host_thread_suspend' thread
	 * is launched for this context (by caller).
	 * It is clear when the context finished (by the host thread).
	 * It should be accessed safely by locking global mutex 'ke->process_events_mutex'. */
	pthread_t host_thread_suspend;  /* Thread */
	int host_thread_suspend_active;  /* Thread-spawned flag */

	/* Host thread that lets time elapse and schedules call to 'ke_process_events'. */
	pthread_t host_thread_timer;  /* Thread */
	int host_thread_timer_active;  /* Thread-spawned flag */
	uint64_t host_thread_timer_wakeup;  /* Time when the thread will wake up */

	/* Three timers used by 'setitimer' system call - real, virtual, and prof. */
	uint64_t itimer_value[3];  /* Time when current occurrence of timer expires (0=inactive) */
	uint64_t itimer_interval[3];  /* Interval (in usec) of repetition (0=inactive) */

	/* Variables used to wake up suspended contexts. */
	uint64_t wakeup_time;  /* ke_timer time to wake up (poll/nanosleep) */
	int wakeup_fd;  /* File descriptor (read/write/poll) */
	int wakeup_events;  /* Events for wake up (poll) */
	int wakeup_pid;  /* Pid waiting for (waitpid) */
	uint32_t wakeup_futex;  /* Address of futex where context is suspended */
	uint32_t wakeup_futex_bitset;  /* Bit mask for selective futex wakeup */
	uint64_t wakeup_futex_sleep;  /* Assignment from ke->futex_sleep_count */

	/* Links to contexts forming a linked list. */
	struct ctx_t *context_next, *context_prev;
	struct ctx_t *running_next, *running_prev;
	struct ctx_t *suspended_next, *suspended_prev;
	struct ctx_t *finished_next, *finished_prev;
	struct ctx_t *zombie_next, *zombie_prev;
	struct ctx_t *alloc_next, *alloc_prev;

	/* Substructures */
	struct loader_t *loader;
	struct mem_t *mem;  /* Virtual memory image */
	struct fdt_t *fdt;  /* File descriptor table */
	struct regs_t *regs;  /* Logical register file */
	struct regs_t *backup_regs;  /* Backup when entering in speculative mode */
	struct signal_masks_t *signal_masks;
	struct signal_handlers_t *signal_handlers;
};

enum ctx_status_t
{
	ctx_running      = 0x00001,  /* it is able to run instructions */
	ctx_specmode     = 0x00002,  /* executing in speculative mode */
	ctx_suspended    = 0x00004,  /* suspended in a system call */
	ctx_finished     = 0x00008,  /* no more inst to execute */
	ctx_exclusive    = 0x00010,  /* executing in excl mode */
	ctx_locked       = 0x00020,  /* another context is running in excl mode */
	ctx_handler      = 0x00040,  /* executing a signal handler */
	ctx_sigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	ctx_nanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	ctx_poll         = 0x00200,  /* 'poll' system call */
	ctx_read         = 0x00400,  /* 'read' system call */
	ctx_write        = 0x00800,  /* 'write' system call */
	ctx_waitpid      = 0x01000,  /* 'waitpid' system call */
	ctx_zombie       = 0x02000,  /* zombie context */
	ctx_futex        = 0x04000,  /* suspended in a futex */
	ctx_alloc        = 0x08000,  /* allocated to a core/thread */
	ctx_gpu          = 0x10000,  /* running a GPU kernel */
	ctx_none         = 0x00000
};

struct ctx_t *ctx_create(void);
struct ctx_t *ctx_clone(struct ctx_t *ctx);
void ctx_free(struct ctx_t *ctx);
void ctx_dump(struct ctx_t *ctx, FILE *f);

/* Thread safe/unsafe versions */
void __ctx_host_thread_suspend_cancel(struct ctx_t *ctx);
void ctx_host_thread_suspend_cancel(struct ctx_t *ctx);
void __ctx_host_thread_timer_cancel(struct ctx_t *ctx);
void ctx_host_thread_timer_cancel(struct ctx_t *ctx);

void ctx_finish(struct ctx_t *ctx, int status);
void ctx_finish_group(struct ctx_t *ctx, int status);
void ctx_execute_inst(struct ctx_t *ctx);

void ctx_set_eip(struct ctx_t *ctx, uint32_t eip);
void ctx_recover(struct ctx_t *ctx);

struct ctx_t *ctx_get(int pid);
struct ctx_t *ctx_get_zombie(struct ctx_t *parent, int pid);

int ctx_get_status(struct ctx_t *ctx, enum ctx_status_t status);
void ctx_set_status(struct ctx_t *ctx, enum ctx_status_t status);
void ctx_clear_status(struct ctx_t *ctx, enum ctx_status_t status);

int ctx_futex_wake(struct ctx_t *ctx, uint32_t futex, uint32_t count, uint32_t bitset);
void ctx_exit_robust_list(struct ctx_t *ctx);

void ctx_gen_proc_self_maps(struct ctx_t *ctx, char *path);




/*
 * CPU Emulation Kernel
 */

struct kernel_t
{
	/* pid & mid assignment */
	int current_pid;
	int current_mid;

	/* Schedule next call to 'ke_process_events()'.
	 * The call will only be effective if 'process_events_force' is set.
	 * This flag should be accessed thread-safely locking 'process_events_mutex'. */
	pthread_mutex_t process_events_mutex;
	int process_events_force;

	/* Counter of times that a context has been suspended in a
	 * futex. Used for FIFO wakeups. */
	uint64_t futex_sleep_count;
	
	/* Flag set when any context changes any status other than 'specmode' */
	int context_reschedule;

	/* Lists of contexts */
	int context_count, context_max;
	int running_count, running_max;
	int suspended_count, suspended_max;
	int zombie_count, zombie_max;
	int finished_count, finished_max;
	int alloc_count, alloc_max;
	struct ctx_t *context_list_head, *context_list_tail;
	struct ctx_t *running_list_head, *running_list_tail;
	struct ctx_t *suspended_list_head, *suspended_list_tail;
	struct ctx_t *zombie_list_head, *zombie_list_tail;
	struct ctx_t *finished_list_head, *finished_list_tail;
	struct ctx_t *alloc_list_head, *alloc_list_tail;

	/* Stats */
	uint64_t inst_count;  /* Number of emulated instructions */
};

enum ke_list_kind_t
{
	ke_list_context = 0,
	ke_list_running,
	ke_list_suspended,
	ke_list_zombie,
	ke_list_finished,
	ke_list_alloc
};

void ke_list_insert_head(enum ke_list_kind_t list, struct ctx_t *ctx);
void ke_list_insert_tail(enum ke_list_kind_t list, struct ctx_t *ctx);
void ke_list_remove(enum ke_list_kind_t list, struct ctx_t *ctx);
int ke_list_member(enum ke_list_kind_t list, struct ctx_t *ctx);


/* Global CPU kernel variable */
extern struct kernel_t *ke;

void ke_init(void);
void ke_done(void);
void ke_run(void);
void ke_disasm(char *file_name);

void ke_dump(FILE *f);

uint64_t ke_timer(void);
void ke_process_events(void);
void ke_process_events_schedule(void);



#endif

