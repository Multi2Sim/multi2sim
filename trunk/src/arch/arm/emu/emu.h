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

#ifndef ARCH_ARM_EMU_EMU_H
#define ARCH_ARM_EMU_EMU_H


#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include <arch/arm/asm/asm.h>
#include <arch/southern-islands/emu/emu.h>
#include <arch/evergreen/emu/emu.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/misc/misc.h>
#include <lib/struct/buffer.h>
#include <lib/struct/config.h>
#include <lib/struct/debug.h>
#include <lib/struct/elf-format.h>
#include <lib/struct/list.h>
#include <lib/struct/linked-list.h>
#include <lib/struct/timer.h>


/* Some forward declarations */
struct arm_ctx_t;
struct x86_file_desc_t;

/* Arm Modes of Operation */
#define ARM_MODE_USER 0x10
#define ARM_MODE_UNDEF 0x1b
#define ARM_MODE_SYS 0x1f
#define ARM_MODE_SVC 0x13
#define ARM_MODE_IRQ 0x12
#define ARM_MODE_FIQ 0x11
#define ARM_MODE_ABT 0x17




/*
 * ARM Registers
 */

struct arm_regs_t
{
	/* Integer registers */
	unsigned int r0, r1, r2, r3;
	unsigned int r4,r5, r6, r7, r8, r9;
	unsigned int sl, fp, ip, sp, lr, pc;
	struct arm_cpsr_reg_t
	{
		unsigned int mode	: 5; /* [4:0] */
		unsigned int thumb	: 1; /* [5] */
		unsigned int fiq  	: 1; /* [6] */
		unsigned int irq	: 1; /* [7] */
		unsigned int _reserved0	:16; /* [23:8] */
		unsigned int jaz	: 1; /* [24] */
		unsigned int _reserved1	: 2; /* [26:25] */
		unsigned int q		: 1; /* [27] */
		unsigned int v		: 1; /* [28] */
		unsigned int C		: 1; /* [29] */
		unsigned int z		: 1; /* [30] */
		unsigned int n		: 1; /* [31] */
	}cpsr;

	struct arm_cpsr_reg_t spsr;

	/* TODO: Arrange all the coproc in structures */
	/* System control coprocessor (cp15) */
	struct arm_coproc_regs_t
	{
		unsigned int c0_cpuid;
		unsigned int c0_cachetype;
		unsigned int c0_ccsid[16]; /* Cache size.  */
		unsigned int c0_clid; /* Cache level.  */
		unsigned int c0_cssel; /* Cache size selection.  */
		unsigned int c0_c1[8]; /* Feature registers.  */
		unsigned int c0_c2[8]; /* Instruction set registers.  */
		unsigned int c1_sys; /* System control register.  */
		unsigned int c1_coproc; /* Coprocessor access register.  */
		unsigned int c1_xscaleauxcr; /* XScale auxiliary control register.  */
		unsigned int c1_scr; /* secure config register.  */
		unsigned int c2_base0; /* MMU translation table base 0.  */
		unsigned int c2_base1; /* MMU translation table base 1.  */
		unsigned int c2_control; /* MMU translation table base control.  */
		unsigned int c2_mask; /* MMU translation table base selection mask.  */
		unsigned int c2_base_mask; /* MMU translation table base 0 mask. */
		unsigned int c2_data; /* MPU data cachable bits.  */
		unsigned int c2_insn; /* MPU instruction cachable bits.  */
		unsigned int c3; /* MMU domain access control register MPU write buffer control.  */
		unsigned int c5_insn; /* Fault status registers.  */
		unsigned int c5_data;
		unsigned int c6_region[8]; /* MPU base/size registers.  */
		unsigned int c6_insn; /* Fault address registers.  */
		unsigned int c6_data;
		unsigned int c7_par; /* Translation result. */
		unsigned int c9_insn; /* Cache lockdown registers.  */
		unsigned int c9_data;
		unsigned int c9_pmcr; /* performance monitor control register */
		unsigned int c9_pmcnten; /* perf monitor counter enables */
		unsigned int c9_pmovsr; /* perf monitor overflow status */
		unsigned int c9_pmxevtyper; /* perf monitor event type */
		unsigned int c9_pmuserenr; /* perf monitor user enable */
		unsigned int c9_pminten; /* perf monitor interrupt enables */
		unsigned int c13_fcse; /* FCSE PID.  */
		unsigned int c13_context; /* Context ID.  */
		unsigned int c13_tls1; /* User RW Thread register.  */
		unsigned int c13_tls2; /* User RO Thread register.  */
		unsigned int c13_tls3; /* Privileged Thread register.  */
		unsigned int c15_cpar; /* XScale Coprocessor Access Register */
		unsigned int c15_ticonfig; /* TI925T configuration byte.  */
		unsigned int c15_i_max; /* Maximum D-cache dirty line index.  */
		unsigned int c15_i_min; /* Minimum D-cache dirty line index.  */
		unsigned int c15_threadid; /* TI debugger thread-ID.  */
		unsigned int c15_config_base_address; /* SCU base address.  */
		unsigned int c15_diagnostic; /* diagnostic register */
		unsigned int c15_power_diagnostic;
		unsigned int c15_power_control; /* power control */
	}cp15;


} __attribute__((packed));
struct arm_regs_t *arm_regs_create();
void arm_regs_free(struct arm_regs_t *regs);
void arm_regs_copy(struct arm_regs_t *dst, struct arm_regs_t *src);




/*
 * Machine & ISA
 */

extern char *arm_isa_inst_bytes;

#define arm_isa_call_debug(...) debug(arm_isa_call_debug_category, __VA_ARGS__)
#define arm_isa_inst_debug(...) debug(arm_isa_inst_debug_category, __VA_ARGS__)

extern int arm_isa_call_debug_category;
extern int arm_isa_inst_debug_category;

void arm_isa_execute_inst(struct arm_ctx_t *ctx);

/* Table of functions implementing implementing the Evergreen ISA */
typedef void (*arm_isa_inst_func_t)(struct arm_ctx_t *ctx);

/* Declarations of function prototypes implementing Evergreen ISA */
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	extern void arm_isa_##_name##_impl(struct arm_ctx_t *ctx);
#include <arch/arm/asm/asm.dat>
#undef DEFINST

enum arm_isa_op2_cat_t
{
	immd = 0,
	reg
};


int arm_isa_op2_get(struct arm_ctx_t *ctx, unsigned int op2 , enum arm_isa_op2_cat_t cat);
unsigned int arm_isa_get_addr_amode2(struct arm_ctx_t *ctx);
int arm_isa_get_addr_amode3_imm(struct arm_ctx_t *ctx);
void arm_isa_reg_store(struct arm_ctx_t *ctx, unsigned int reg_no,
	int value);
void arm_isa_reg_store_safe(struct arm_ctx_t *ctx, unsigned int reg_no,
	unsigned int value);
void arm_isa_reg_load(struct arm_ctx_t *ctx, unsigned int reg_no,
	 int *value);
void arm_isa_branch(struct arm_ctx_t *ctx);
int arm_isa_check_cond(struct arm_ctx_t *ctx);
void arm_isa_amode4s_str(struct arm_ctx_t *ctx);
void arm_isa_amode4s_ld(struct arm_ctx_t *ctx);
void arm_isa_cpsr_print(struct arm_ctx_t *ctx);
unsigned int arm_isa_ret_cpsr_val(struct arm_ctx_t *ctx);
void arm_isa_set_cpsr_val(struct arm_ctx_t *ctx, unsigned int op2);
void arm_isa_subtract(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_subtract_rev(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_add(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_multiply(struct arm_ctx_t *ctx);
int arm_isa_op2_carry(struct arm_ctx_t *ctx,  unsigned int op2 , enum arm_isa_op2_cat_t cat);

void arm_isa_syscall(struct arm_ctx_t *ctx);
unsigned int arm_isa_invalid_addr_str(unsigned int addr, int value, struct arm_ctx_t *ctx);
unsigned int arm_isa_invalid_addr_ldr(unsigned int addr, unsigned int* value, struct arm_ctx_t *ctx);


#define CALL_STACK_SIZE 10000

struct arm_isa_cstack_t
{
	char *sym_name[CALL_STACK_SIZE];
	unsigned int top;
};
struct arm_isa_cstack_t* arm_isa_cstack_create(struct arm_ctx_t *ctx);
void arm_isa_cstack_push(struct arm_ctx_t *ctx, char *str);
char* arm_isa_cstack_pop(struct arm_ctx_t *ctx);


/*
 * System calls
 */

#define ARM_set_tls 0xF0005
#define ARM_exit_group 248

#define arm_sys_debug(...) debug(arm_sys_debug_category, __VA_ARGS__)
#define arm_sys_debug_buffer(...) debug_buffer(arm_sys_debug_category, __VA_ARGS__)
extern int arm_sys_debug_category;

void arm_sys_init(void);
void arm_sys_done(void);

void arm_sys_call(struct arm_ctx_t *ctx);





/*
 * ARM CPU Emulator
 */

struct arm_emu_t
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
	struct arm_ctx_t *context_list_head;
	struct arm_ctx_t *context_list_tail;
	int context_list_count;
	int context_list_max;

	/* List of running contexts */
	struct arm_ctx_t *running_list_head;
	struct arm_ctx_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of suspended contexts */
	struct arm_ctx_t *suspended_list_head;
	struct arm_ctx_t *suspended_list_tail;
	int suspended_list_count;
	int suspended_list_max;

	/* List of zombie contexts */
	struct arm_ctx_t *zombie_list_head;
	struct arm_ctx_t *zombie_list_tail;
	int zombie_list_count;
	int zombie_list_max;

	/* List of finished contexts */
	struct arm_ctx_t *finished_list_head;
	struct arm_ctx_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;

	/* List of allocated contexts */
	struct arm_ctx_t *alloc_list_head;
	struct arm_ctx_t *alloc_list_tail;
	int alloc_list_count;
	int alloc_list_max;

	/* Stats */
	long long inst_count;  /* Number of emulated instructions */
};

enum arm_emu_list_kind_t
{
	arm_emu_list_context = 0,
	arm_emu_list_running,
	arm_emu_list_suspended,
	arm_emu_list_zombie,
	arm_emu_list_finished,
	arm_emu_list_alloc
};

int arm_emu_list_member(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);
void arm_emu_list_remove(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);
void arm_emu_list_insert_tail(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);
void arm_emu_list_insert_head(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);
void arm_emu_dump_summary(FILE *f);

/* Global CPU emulator variable */
extern struct arm_emu_t *arm_emu;

extern long long arm_emu_max_cycles;
extern long long arm_emu_max_inst;
extern long long arm_emu_max_time;

extern enum arm_emu_kind_t
{
	arm_emu_kind_functional,
	arm_emu_kind_detailed
} arm_emu_kind;

void arm_emu_init(void);
void arm_emu_done(void);

int arm_emu_run(void);

void arm_emu_process_events_schedule();

#endif
