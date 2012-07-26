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

#ifndef ARM_EMU_H
#define ARM_EMU_H


#include <stdio.h>
#include <string.h>
#include <list.h>
#include <debug.h>
#include <misc.h>
#include <elf-format.h>
#include <mhandle.h>
#include <debug.h>
#include <config.h>
#include <buffer.h>
#include <list.h>
#include <linked-list.h>
#include <elf-format.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <timer.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>
#include <esim.h>
#include <sys/time.h>
#include <math.h>
#include <arm-asm.h>

#include <southern-islands-emu.h>
#include <evergreen-emu.h>


/* Some forward declarations */
struct arm_ctx_t;
struct file_desc_t;

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
 * System signals
 */


/* Every contexts (parent and children) has its own masks */
struct arm_signal_mask_table_t
{
	unsigned long long pending;  /* Mask of pending signals */
	unsigned long long blocked;  /* Mask of blocked signals */
	unsigned long long backup;  /* Backup of blocked signals while suspended */
	struct x86_regs_t *regs;  /* Backup of regs while executing handler */
	unsigned int pretcode;  /* Base address of a memory page allocated for retcode execution */
};

struct arm_signal_handler_table_t
{
	/* Number of extra contexts sharing the table */
	int num_links;

	/* Signal handlers */
	struct arm_sim_sigaction
	{
		unsigned int handler;
		unsigned int flags;
		unsigned int restorer;
		unsigned long long mask;
	} sigaction[64];
};




/*
 * File management
 */

enum arm_file_desc_kind_t
{
	arm_file_desc_invalid = 0,
	arm_file_desc_regular,  /* Regular arm_file */
	arm_file_desc_std,  /* Standard input or output */
	arm_file_desc_pipe,  /* A pipe */
	arm_file_desc_virtual,  /* A virtual arm_file with artificial contents */
	arm_file_desc_gpu,  /* GPU device */
	arm_file_desc_socket  /* Network socket */
};


/* File descriptor */
struct arm_file_desc_t
{
	enum arm_file_desc_kind_t kind;  /* File type */
	int guest_fd;  /* Guest arm_file descriptor id */
	int host_fd;  /* Equivalent open host arm_file */
	int flags;  /* O_xxx flags */
	char *path;  /* Associated path if applicable */
};


/* File descriptor table */
struct arm_file_desc_table_t
{
	/* Number of extra contexts sharing table */
	int num_links;

	/* List of descriptors */
	struct list_t *arm_file_desc_list;
};


struct arm_file_desc_table_t *arm_file_desc_table_create(void);
void arm_file_desc_table_free(struct arm_file_desc_table_t *table);

struct arm_file_desc_table_t *arm_file_desc_table_link(struct arm_file_desc_table_t *table);
void arm_file_desc_table_unlink(struct arm_file_desc_table_t *table);

void arm_file_desc_table_dump(struct arm_file_desc_table_t *table, FILE *f);

struct arm_file_desc_t *arm_file_desc_table_entry_get(struct arm_file_desc_table_t *table, int index);
struct arm_file_desc_t *arm_file_desc_table_entry_new(struct arm_file_desc_table_t *table,
	enum arm_file_desc_kind_t kind, int host_fd, char *path, int flags);
struct arm_file_desc_t *arm_file_desc_table_entry_new_guest_fd(struct arm_file_desc_table_t *table,
        enum arm_file_desc_kind_t kind, int guest_fd, int host_fd, char *path, int flags);
void arm_file_desc_table_entry_free(struct arm_file_desc_table_t *table, int index);
void arm_file_desc_table_entry_dump(struct arm_file_desc_table_t *table, int index, FILE *f);

int arm_file_desc_table_get_host_fd(struct arm_file_desc_table_t *table, int guest_fd);
int arm_file_desc_table_get_guest_fd(struct arm_file_desc_table_t *table, int host_fd);




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
#include <arm-asm.dat>
#undef DEFINST

enum arm_isa_op2_cat_t
{
	immd = 0,
	reg
};

int arm_isa_op2_get(struct arm_ctx_t *ctx, unsigned int op2 , enum arm_isa_op2_cat_t cat);
unsigned int arm_isa_get_addr_amode2(struct arm_ctx_t *ctx);
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
void arm_isa_subtract(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_subtract_rev(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_add(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_multiply(struct arm_ctx_t *ctx);
int arm_isa_op2_carry(struct arm_ctx_t *ctx,  unsigned int op2 , enum arm_isa_op2_cat_t cat);

void arm_isa_syscall(struct arm_ctx_t *ctx);




/*
 * System calls
 */

#define ARM_set_tls 0xF0005

#define arm_sys_debug(...) debug(arm_sys_debug_category, __VA_ARGS__)
#define arm_sys_debug_buffer(...) debug_buffer(arm_sys_debug_category, __VA_ARGS__)
extern int arm_sys_debug_category;

void arm_sys_init(void);
void arm_sys_done(void);

void arm_sys_call(struct arm_ctx_t *ctx);


/*
 * ARM Context
 */

#define arm_ctx_debug(...) debug(arm_ctx_debug_category, __VA_ARGS__)
extern int arm_ctx_debug_category;

/* Event scheduled periodically to dump IPC statistics for a context */
extern int EV_ARM_CTX_IPC_REPORT;

struct arm_ctx_t
{
	/* Number of extra contexts using this loader */
	int num_links;

	/* Parent context */
	struct arm_ctx_t *parent;

	/* Context properties */
	int status;
	int pid;  /* Context ID */
	int address_space_index;  /* Virtual memory address space index */

	int exit_signal;  /* Signal to send parent when finished */
	int exit_code;  /* For zombie contexts */

	/* IPC report (for detailed simulation) */
	FILE *ipc_report_file;
	int ipc_report_interval;

	/* Program data */
	struct elf_file_t *elf_file;
	struct linked_list_t *args;
	struct linked_list_t *env;
	char *exe;  /* Executable file name */
	char *cwd;  /* Current working directory */
	char *stdin_file;  /* File name for stdin */
	char *stdout_file;  /* File name for stdout */

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

	/* Instruction pointers */
	unsigned int last_ip;  /* Address of last emulated instruction */
	unsigned int curr_ip;  /* Address of currently emulated instruction */
	unsigned int target_ip;  /* Target address for branch, even if not taken */

	/* Currently emulated instruction */
	struct arm_inst_t inst;

	/* Links to contexts forming a linked list. */
	struct arm_ctx_t *context_list_next, *context_list_prev;
	struct arm_ctx_t *running_list_next, *running_list_prev;
	struct arm_ctx_t *suspended_list_next, *suspended_list_prev;
	struct arm_ctx_t *finished_list_next, *finished_list_prev;
	struct arm_ctx_t *zombie_list_next, *zombie_list_prev;
	struct arm_ctx_t *alloc_list_next, *alloc_list_prev;

	/* Substructures */
	struct mem_t *mem; /* Virtual Memory image */
	struct arm_regs_t *regs; /* Logical register file */
	struct arm_file_desc_table_t *file_desc_table;  /* File descriptor table */
	struct signal_mask_table_t *signal_mask_table;
	struct signal_handler_table_t *signal_handler_table;



	/* Statistics */

	/* Number of non-speculate instructions.
	 * Updated by the architectural simulator at the commit stage. */
	long long inst_count;
};

enum arm_ctx_status_t
{
	arm_ctx_running      = 0x00001,  /* it is able to run instructions */
	arm_ctx_spec_mode    = 0x00002,  /* executing in speculative mode */
	arm_ctx_suspended    = 0x00004,  /* suspended in a system call */
	arm_ctx_finished     = 0x00008,  /* no more inst to execute */
	arm_ctx_exclusive    = 0x00010,  /* executing in excl mode */
	arm_ctx_locked       = 0x00020,  /* another context is running in excl mode */
	arm_ctx_handler      = 0x00040,  /* executing a signal handler */
	arm_ctx_sigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	arm_ctx_nanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	arm_ctx_poll         = 0x00200,  /* 'poll' system call */
	arm_ctx_read         = 0x00400,  /* 'read' system call */
	arm_ctx_write        = 0x00800,  /* 'write' system call */
	arm_ctx_waitpid      = 0x01000,  /* 'waitpid' system call */
	arm_ctx_zombie       = 0x02000,  /* zombie context */
	arm_ctx_futex        = 0x04000,  /* suspended in a futex */
	arm_ctx_alloc        = 0x08000,  /* allocated to a core/thread */
	arm_ctx_callback     = 0x10000,  /* suspended after syscall with callback */
	arm_ctx_none         = 0x00000
};

struct arm_ctx_t *arm_ctx_create();

#define arm_loader_debug(...) debug(arm_loader_debug_category, __VA_ARGS__)
extern int arm_loader_debug_category;

int arm_ctx_get_status(struct arm_ctx_t *ctx, enum arm_ctx_status_t status);
void arm_ctx_set_status(struct arm_ctx_t *ctx, enum arm_ctx_status_t status);

void arm_ctx_execute(struct arm_ctx_t *ctx);
void arm_ctx_free(struct arm_ctx_t *ctx);
void arm_ctx_loader_get_full_path(struct arm_ctx_t *ctx, char *file_name, char *full_path, int size);
void arm_ctx_finish(struct arm_ctx_t *ctx, int status);

void arm_ctx_load_from_command_line(int argc, char **argv);
void arm_ctx_load_from_ctx_config(struct config_t *config, char *section);

void arm_ctx_ipc_report_handler(int event, void *data);


enum arm_gpu_emulator_kind_t
{
	arm_gpu_emulator_evg = 0,
	arm_gpu_emulator_si
};




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

	/* Determines which GPU emulator will be called */
	enum arm_gpu_emulator_kind_t arm_gpu_emulator;
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

/* Global CPU emulator variable */
extern struct arm_emu_t *arm_emu;

extern long long arm_emu_max_cycles;
extern long long arm_emu_max_inst;
extern long long arm_emu_max_time;
extern char * arm_emu_last_inst_bytes;

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
