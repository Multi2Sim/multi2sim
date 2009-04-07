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

#ifndef M2SKERNEL_H
#define M2SKERNEL_H

#include <mhandle.h>
#include <debug.h>
#include <config.h>
#include <buffer.h>
#include <lnlist.h>
#include <misc.h>
#include <elf.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <disasm.h>
#include <time.h>
#include <sys/time.h>


/* Declaration of the kernel context type. We will need it
 * before specifying all its fields. */
struct ctx_t;




/* Memory */

#define MEM_LOGPAGESIZE		12
#define MEM_PAGESIZE		(1<<MEM_LOGPAGESIZE)
#define MEM_PAGE_COUNT		1024

enum mem_access_enum {
	mem_access_read = 0x01,
	mem_access_write = 0x02,
	mem_access_exec = 0x04,
	mem_access_init = 0x08
};

/* A 4KB page of memory */
struct mem_page_t {
	uint32_t tag;
	enum mem_access_enum perm;  /* access permissions; combination of flags */
	struct mem_page_t *next;
	unsigned char data[MEM_PAGESIZE];
};

struct mem_t {
	struct mem_page_t *pages[MEM_PAGE_COUNT];
	int sharing;  /* Number of contexts sharing memory map */
	uint32_t last_address;  /* Address of last access */
	int safe;  /* Safe mode */
};


struct mem_t *mem_create(void);
void mem_free(struct mem_t *mem);

uint32_t mem_map_space(struct mem_t *mem, uint32_t addr, int size);
uint32_t mem_map_space_down(struct mem_t *mem, uint32_t addr, int size);

void mem_map(struct mem_t *mem, uint32_t addr, int size, enum mem_access_enum perm);
void mem_unmap(struct mem_t *mem, uint32_t addr, int size);
void mem_protect(struct mem_t *mem, uint32_t addr, int size, enum mem_access_enum perm);
void mem_copy(struct mem_t *mem, uint32_t dest, uint32_t src, int size);

#define mem_read(mem, addr, size, buf) mem_access(mem, addr, size, buf, mem_access_read)
#define mem_write(mem, addr, size, buf) mem_access(mem, addr, size, buf, mem_access_write)
void mem_access(struct mem_t *mem, uint32_t addr, int size, void *buf, enum mem_access_enum access);
void mem_zero(struct mem_t *mem, uint32_t addr, int size);
int mem_read_string(struct mem_t *mem, uint32_t addr, int size, char *str);
void mem_write_string(struct mem_t *mem, uint32_t addr, char *str);
void *mem_get_buffer(struct mem_t *mem, uint32_t addr, int size, enum mem_access_enum access);

void mem_dump(struct mem_t *mem, char *filename, uint32_t start, uint32_t end);
void mem_load(struct mem_t *mem, char *filename, uint32_t start);




/* Registers */

struct regs_t {
	
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
} __attribute__((packed));

struct regs_t *regs_create(void);
void regs_free(struct regs_t *regs);

void regs_copy(struct regs_t *dst, struct regs_t *src);
void regs_dump(struct regs_t *regs, FILE *f);
void regs_fpu_stack_dump(struct regs_t *regs, FILE *f);




/* ELF File Handling */

#define elf_debug(...) debug(elf_debug_category, __VA_ARGS__)
extern int elf_debug_category;

struct elf_file_t;

struct elf_file_t *elf_open(char *path);
void elf_close(struct elf_file_t *f);

int elf_section_count(struct elf_file_t *f);
int elf_section_info(struct elf_file_t *f, int section,
	char **pname, uint32_t *paddr, uint32_t *psize, uint32_t *pflags);
void *elf_section_read(struct elf_file_t *f, int section);
void elf_section_free(void *buf);

void *elf_phdt(struct elf_file_t *f);
uint32_t elf_phdt_base(struct elf_file_t *f);
uint32_t elf_phdr_count(struct elf_file_t *f);
uint32_t elf_phdr_size(struct elf_file_t *f);

char *elf_get_symbol(struct elf_file_t *f, uint32_t addr, uint32_t *poffs);
uint32_t elf_get_entry(struct elf_file_t *f);




/* Program loader */

struct loader_t {
	
	/* Program data */
	struct elf_file_t *elf;
	struct lnlist_t *args;
	struct lnlist_t *env;
	char *interp;  /* Executable interpreter */
	char *exe;  /* Executable file name */
	char *cwd;  /* Current working directory */
	char *stdin_file, *stdout_file;  /* File names for stdin and stdout */
	int stdin_fd, stdout_fd;  /* File descriptors for stdin/stdout */

	/* Code pointers */
	uint32_t stack_size, text_size;
	uint32_t environ_base, brk, bottom;
	uint32_t prog_entry;
	uint32_t phdt_base, phdr_count;
};


#define ld_debug(...) debug(ld_debug_category, __VA_ARGS__)
extern int ld_debug_category;

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
int ld_translate_fd(struct ctx_t *ctx, int fd);
void ld_get_full_path(struct ctx_t *ctx, char *filename, char *fullpath, int size);




/* Machine & ISA */

extern struct ctx_t *isa_ctx;
extern struct regs_t *isa_regs;
extern struct mem_t *isa_mem;
extern uint32_t isa_eip;
extern x86_inst_t isa_inst;
extern uint64_t isa_inst_count;
extern int isa_function_level;

#define isa_call_debug(...) debug(isa_call_debug_category, __VA_ARGS__)
#define isa_inst_debug(...) debug(isa_inst_debug_category, __VA_ARGS__)
extern int isa_call_debug_category;
extern int isa_inst_debug_category;

/* References to inst implementation functions */
#define DEFINST(name,op1,op2,op3,imm,pfx) void op_##name##_impl(void);
#include <machine.dat>
#undef DEFINST

void isa_dump_flags(FILE *f);
void isa_set_flag(x86_flag_t flag);
void isa_clear_flag(x86_flag_t flag);
int isa_get_flag(x86_flag_t flag);

uint32_t isa_load_reg(x86_register_t reg);
void isa_store_reg(x86_register_t reg, uint32_t value);

uint8_t isa_load_rm8(void);
uint16_t isa_load_rm16(void);
uint32_t isa_load_rm32(void);
void isa_store_rm8(uint8_t value);
void isa_store_rm16(uint16_t value);
void isa_store_rm32(uint32_t value);

#define isa_load_r8() isa_load_reg(isa_inst.reg + reg_al)
#define isa_load_r16() isa_load_reg(isa_inst.reg + reg_ax)
#define isa_load_r32() isa_load_reg(isa_inst.reg + reg_eax)
#define isa_load_sreg() isa_load_reg(isa_inst.reg + reg_es)
#define isa_store_r8(value) isa_store_reg(isa_inst.reg + reg_al, value)
#define isa_store_r16(value) isa_store_reg(isa_inst.reg + reg_ax, value)
#define isa_store_r32(value) isa_store_reg(isa_inst.reg + reg_eax, value)
#define isa_store_sreg(value) isa_store_reg(isa_inst.reg + reg_es, value)

#define isa_load_ir8() isa_load_reg(isa_inst.opindex + reg_al)
#define isa_load_ir16() isa_load_reg(isa_inst.opindex + reg_ax)
#define isa_load_ir32() isa_load_reg(isa_inst.opindex + reg_eax)
#define isa_store_ir8(value) isa_store_reg(isa_inst.opindex + reg_al, value)
#define isa_store_ir16(value) isa_store_reg(isa_inst.opindex + reg_ax, value)
#define isa_store_ir32(value) isa_store_reg(isa_inst.opindex + reg_eax, value)

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
void isa_execute_inst(void *buf);

void isa_trace_call_init(char *filename);
void isa_trace_call_done(void);

void isa_inst_stat_dump(FILE *f);
void isa_inst_stat_reset(void);




/* System calls */

#define syscall_debug(...) debug(syscall_debug_category, __VA_ARGS__)
extern int syscall_debug_category;

void syscall_do(void);
void syscall_summary(void);




/* System signals */

/* Every contexts (parent and children) has its own masks */
struct signal_masks_t {
	uint64_t pending;  /* mask of pending signals */
	uint64_t blocked;  /* mask of blocked signals */
	uint64_t backup;  /* backup of blocked signals while suspended */
	struct regs_t *regs;  /* backup of regs while executing handler */
};

struct signal_masks_t *signal_masks_create(void);
void signal_masks_free(struct signal_masks_t *signal_masks);

/* This structure is shared for parent and child contexts. A change
 * in the singal handler by any of them affects all of them. */
struct signal_handlers_t {
	struct sim_sigaction {
		uint32_t handler;
		uint32_t flags;
		uint32_t restorer;
		uint64_t mask;
	} sigaction[64];
};

struct signal_handlers_t *signal_handlers_create(void);
void signal_handlers_free(struct signal_handlers_t *signal_handlers);

void signal_process(struct ctx_t *ctx);
void signal_handler_run(struct ctx_t *ctx, int sig);
void signal_handler_return(struct ctx_t *ctx);

char *sim_signal_name(int signum);
void sim_sigaction_dump(struct sim_sigaction *sim_sigaction, FILE *f);
void sim_sigaction_flags_dump(uint32_t flags, FILE *f);
void sim_sigset_dump(uint64_t sim_sigset, FILE *f);
void sim_sigset_add(uint64_t *sim_sigset, int signal);
void sim_sigset_del(uint64_t *sim_sigset, int signal);
int sim_sigset_member(uint64_t *sim_sigset, int signal);




/* System pipes */

struct pipe_t {
	int fd[2];
	struct buffer_t *buffer;
	struct pipe_t *next;
};

struct pipemgr_t {
	struct pipe_t *pipe_list;  /* Linked list of existing pipes */
};

void pipemgr_init(void);
void pipemgr_done(void);

struct pipe_t *pipe_create(void);
void pipe_free(struct pipe_t *pipe);

void pipe_pipe(int fd[2]);
void pipe_close(int fd);
int pipe_is_pipe(int fd);

int pipe_write(int fd, void *buf, int size);
int pipe_read(int fd, void *buf, int size);
int pipe_count(int fd);




/* Context */

struct ctx_t {
	
	/* Context properties */
	int status;
	int pid;  /* Context id */
	int mid;  /* Memory id - the same for contexts sharing memory map */
	struct ctx_t *parent;
	int exit_signal;  /* Signal to send parent when finished */
	int exit_code;  /* For zombie processes */
	uint32_t backup_eip;  /* Saved eip when in specmode */
	uint32_t set_child_tid, clear_child_tid;
	uint32_t robust_list_head;  /* robust futex list */
	uint32_t initial_stack;  /* Value of esp when context is cloned */

	/* For segmented memory access in glibc */
	uint32_t glibc_segment_base;
	uint32_t glibc_segment_limit;

	/* Variables used for waking up suspended contexts. */
	uint64_t wakeup_time;  /* ke_timer time to wake up (poll/nanosleep) */
	int wakeup_fd;  /* File descriptor (read/write/poll) */
	int wakeup_events;  /* Events for wake up (poll) */
	int wakeup_pid;  /* Pid waiting for (waitpid) */
	uint32_t wakeup_futex;  /* Address of futex where context is suspended */
	uint64_t wakeup_futex_sleep;  /* Assignment from ke->futex_sleep_count */

	/* Links to contexts forming a linked list. */
	struct ctx_t *context_next, *context_prev;
	struct ctx_t *running_next, *running_prev;
	struct ctx_t *suspended_next, *suspended_prev;
	struct ctx_t *finished_next, *finished_prev;
	struct ctx_t *zombie_next, *zombie_prev;

	/* Substructures */
	struct loader_t *loader;
	struct mem_t *mem;
	struct regs_t *regs;
	struct signal_masks_t *signal_masks;
	struct signal_handlers_t *signal_handlers;
};

enum ctx_status_enum {
	ctx_running      = 0x0001,  /* it is able to run instructions */
	ctx_specmode     = 0x0002,  /* executing in speculative mode */
	ctx_suspended    = 0x0004,  /* suspended in a system call */
	ctx_finished     = 0x0008,  /* no more inst to execute */
	ctx_exclusive    = 0x0010,  /* executing in excl mode */
	ctx_locked       = 0x0020,  /* another context is running in excl mode */
	ctx_handler      = 0x0040,  /* executing a signal handler */
	ctx_sigsuspend   = 0x0080,  /* suspended after syscall 'sigsuspend' */
	ctx_nanosleep    = 0x0100,  /* suspended after syscall 'nanosleep' */
	ctx_poll         = 0x0200,  /* 'poll' system call */
	ctx_read         = 0x0400,  /* 'read' system call */
	ctx_write        = 0x0800,  /* 'write' system call */
	ctx_waitpid      = 0x1000,  /* 'waitpid' system call */
	ctx_zombie       = 0x2000,  /* zombie context */
	ctx_futex        = 0x4000,  /* suspended in a futex */
	ctx_none         = 0x0000
};

struct ctx_t *ctx_create(void);
struct ctx_t *ctx_clone(struct ctx_t *ctx);
void ctx_free(struct ctx_t *ctx);
void ctx_dump(struct ctx_t *ctx, FILE *f);

void ctx_finish(struct ctx_t *ctx, int status);
void ctx_execute_inst(struct ctx_t *ctx);

void ctx_set_eip(struct ctx_t *ctx, uint32_t eip);
void ctx_recover(struct ctx_t *ctx);

struct ctx_t *ctx_get(int pid);
struct ctx_t *ctx_get_zombie(struct ctx_t *parent, int pid);

int ctx_get_status(struct ctx_t *ctx, enum ctx_status_enum status);
void ctx_set_status(struct ctx_t *ctx, enum ctx_status_enum status);
void ctx_clear_status(struct ctx_t *ctx, enum ctx_status_enum status);

int ctx_futex_wake(struct ctx_t *ctx, uint32_t futex, uint32_t count);
void ctx_exit_robust_list(struct ctx_t *ctx);




/* Kernel */

struct kernel_t {

	/* pid & mid assignment */
	int current_pid;
	int current_mid;

	/* Time based on the value of ke_timer() for the next needed
	 * call to ke_event_timer, where it is checked whether suspended
	 * contexts must resume execution. */
	uint64_t event_timer_next;

	/* Counter of times that a context has been suspended in a
	 * futex. Used for FIFO wakeups. */
	uint64_t futex_sleep_count;
	
	/* Pipe manager */
	struct pipemgr_t *pipemgr;

	/* Lists of contexts */
	int context_max;  /* max number of allocated contexts so far */
	int context_count;
	int running_count;
	int suspended_count;
	int zombie_count;
	int finished_count;
	struct ctx_t *context_list;
	struct ctx_t *running_list;
	struct ctx_t *suspended_list;
	struct ctx_t *zombie_list;
	struct ctx_t *finished_list;
};

enum ke_list_enum {
	ke_list_context = 0,
	ke_list_running,
	ke_list_suspended,
	ke_list_zombie,
	ke_list_finished
};

void ke_list_insert(enum ke_list_enum list, struct ctx_t *ctx);
void ke_list_remove(enum ke_list_enum list, struct ctx_t *ctx);
int ke_list_member(enum ke_list_enum list, struct ctx_t *ctx);


/* Global Multi2Sim
 * Kernel Variable */
extern struct kernel_t *ke;

void ke_init(void);
void ke_done(void);
void ke_run(void);
void ke_dump(FILE *f);

uint64_t ke_timer(void);
void ke_event_timer(void);
void ke_event_read(void);
void ke_event_write(void);
void ke_event_signal(void);
void ke_event_finish(void);



#endif

