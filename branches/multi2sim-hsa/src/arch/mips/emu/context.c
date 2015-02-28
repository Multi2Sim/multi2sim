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


#include <fcntl.h>
#include <unistd.h>

#include <arch/mips/asm/Wrapper.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>
#include <memory/memory.h>

#include "context.h"
#include "file.h"
#include "isa.h"
#include "regs.h"

int mips_loader_debug_category;
int mips_context_debug_category;

static struct str_map_t mips_ctx_status_map =
{
	16, {
		{ "running",      MIPSContextRunning },
		{ "specmode",     MIPSContextSpecMode },
		{ "suspended",    MIPSContextSuspended },
		{ "finished",     MIPSContextFinished },
		{ "exclusive",    MIPSContextExclusive },
		{ "locked",       MIPSContextLocked },
		{ "handler",      MIPSContextHandler },
		{ "sigsuspend",   MIPSContextSigsuspend },
		{ "nanosleep",    MIPSContextNanosleep },
		{ "poll",         MIPSContextPoll },
		{ "read",         MIPSContextRead },
		{ "write",        MIPSContextWrite },
		{ "waitpid",      MIPSContextWaitpid },
		{ "zombie",       MIPSContextZombie },
		{ "futex",        MIPSContextFutex },
		{ "alloc",        MIPSContextAlloc }
	}
};

/* Stack parameter definitions */
#define LD_STACK_BASE  0xc0000000
#define LD_MAX_ENVIRON  0x10000  /* 16KB for environment */
#define LD_STACK_SIZE  0x800000  /* 8MB stack size */

static struct str_map_t elf_section_flags_map =
{
	3, {
		{ "SHF_WRITE", 1 },
		{ "SHF_ALLOC", 2 },
		{ "SHF_EXECINSTR", 4 }
	}
};

static struct str_map_t elf_program_header_type_map = {
	8, {
		{ "PT_NULL",        0 },
		{ "PT_LOAD",        1 },
		{ "PT_DYNAMIC",     2 },
		{ "PT_INTERP",      3 },
		{ "PT_NOTE",        4 },
		{ "PT_SHLIB",       5 },
		{ "PT_PHDR",        6 },
		{ "PT_TLS",         7 }
	}
};



/*
 * Class 'MIPSContext'
 */

static void MIPSContextDoCreate(MIPSContext *self, MIPSEmu *emu)
{
	/* Initialize */
	self->emu = emu;
	self->pid = emu->current_pid++;

	/* Update status so that the context is inserted in the
	 * corresponding lists. The mips_ctx_running parameter has no
	 * effect, since it will be updated later. */
	MIPSContextSetState(self, MIPSContextRunning);

	assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, context, self)));
	DOUBLE_LINKED_LIST_INSERT_HEAD(emu, context, self);

	/* Structures */
	self->regs = mips_regs_create();
	self->inst = MIPSInstWrapCreate();
}


/* Load program headers table */
static void MIPSContextLoadProgramHeaders(MIPSContext *self)
{
	struct mem_t *mem = self->mem;

	struct elf_file_t *elf_file = self->elf_file;
	struct elf_program_header_t *program_header;

	unsigned int phdt_base;
	unsigned int phdt_size;
	unsigned int phdr_count;
	unsigned int phdr_size;

	char str[MAX_STRING_SIZE];
	int i;

	/* Load program header table from ELF */
	mips_loader_debug("\nLoading program headers\n");
	phdr_count = elf_file->header->e_phnum;
	phdr_size = elf_file->header->e_phentsize;
	phdt_size = phdr_count * phdr_size;
	assert(phdr_count == list_count(elf_file->program_header_list));

	/* Program header PT_PHDR, specifying location and size of the program header table itself. */
	/* Search for program header PT_PHDR, specifying location and size of the program header table.
	 * If none found, choose ld->bottom - phdt_size. */
	phdt_base = self->bottom - phdt_size;
	for (i = 0; i < list_count(elf_file->program_header_list); i++)
	{
		program_header = list_get(elf_file->program_header_list, i);
		if (program_header->header->p_type == PT_PHDR)
			phdt_base = program_header->header->p_vaddr;
	}
	mips_loader_debug("  virtual address for program header table: 0x%x\n", phdt_base);

	/* Load program headers */
	mem_map(mem, phdt_base, phdt_size, mem_access_init | mem_access_read);
	for (i = 0; i < list_count(elf_file->program_header_list); i++)
	{
		/* Load program header */
		program_header = list_get(elf_file->program_header_list, i);
		mem_access(mem, phdt_base + i * phdr_size, phdr_size,
			program_header->header, mem_access_init);

		/* Debug */
		str_map_value_buf(&elf_program_header_type_map, program_header->header->p_type,
			str, sizeof(str));
		mips_loader_debug("  header loaded at 0x%x\n", phdt_base + i * phdr_size);
		mips_loader_debug("    type=%s, offset=0x%x, vaddr=0x%x, paddr=0x%x\n",
			str, program_header->header->p_offset,
			program_header->header->p_vaddr,
			program_header->header->p_paddr);
		mips_loader_debug("    filesz=%d, memsz=%d, flags=%d, align=%d\n",
			program_header->header->p_filesz,
			program_header->header->p_memsz,
			program_header->header->p_flags,
			program_header->header->p_align);

		/* TODO: Program Interpreter to be added */
	}
	/* Free buffer and save pointers */
	self->phdt_base = phdt_base;
	self->phdr_count = phdr_count;
}


/* Load sections from an ELF file */
static void MIPSContextLoadSections(MIPSContext *self, struct elf_file_t *elf_file)
{
	struct mem_t *mem = self->mem;

	struct elf_section_t *section;
	int i;

	enum mem_access_t perm;
	char flags_str[200];

	mips_loader_debug("\nLoading ELF sections\n");
	self->bottom = 0xffffffff;
	for (i = 0; i < list_count(elf_file->section_list); i++)
	{
		section = list_get(elf_file->section_list, i);

		perm = mem_access_init | mem_access_read;
		str_map_flags(&elf_section_flags_map, section->header->sh_flags,
			flags_str, sizeof(flags_str));
		mips_loader_debug("  section %d: name='%s', offset=0x%x, addr=0x%x,"
			" size=%u, flags=%s\n",
			i, section->name, section->header->sh_offset,
			section->header->sh_addr, section->header->sh_size,
			flags_str);

		/* Process section */
		if (section->header->sh_flags & SHF_ALLOC)
		{
			/* Permissions */
			if (section->header->sh_flags & SHF_WRITE)
				perm |= mem_access_write;
			if (section->header->sh_flags & SHF_EXECINSTR)
				perm |= mem_access_exec;

			/* Load section */
			mem_map(mem, section->header->sh_addr, section->header->sh_size, perm);
			mem->heap_break = MAX(mem->heap_break, section->header->sh_addr
				+ section->header->sh_size);
			self->bottom = MIN(self->bottom, section->header->sh_addr);

			/* If section type is SHT_NOBITS (sh_type=8), initialize to 0.
			 * Otherwise, copy section contents from ELF file. */
			if (section->header->sh_type == 8)
			{
				void *ptr;

				ptr = xcalloc(1, section->header->sh_size);
				mem_access(mem, section->header->sh_addr,
					section->header->sh_size,
					ptr, mem_access_init);
				free(ptr);
			} else {
				mem_access(mem, section->header->sh_addr,
					section->header->sh_size,
					section->buffer.ptr, mem_access_init);
			}
		}
	}
}

/* Load auxiliary vector, and return its size in bytes. */

#define MIPS_LOADER_AV_ENTRY(t, v) \
{ \
	unsigned int a_type = t; \
	unsigned int a_value = v; \
	mem_write(mem, sp, 4, &a_type); \
	mem_write(mem, sp + 4, 4, &a_value); \
	sp += 8; \
}

static unsigned int MIPSContextLoadAuxiliaryVector(MIPSContext *self,
		unsigned int where)
{

	struct mem_t *mem = self->mem;
	unsigned int sp = where;

	mips_loader_debug("Loading auxiliary vector at 0x%x\n", where);

	/* Program headers */
	MIPS_LOADER_AV_ENTRY(3, self->phdt_base);  /* AT_PHDR */
	MIPS_LOADER_AV_ENTRY(4, 32);  /* AT_PHENT -> program header size of 32 bytes */
	MIPS_LOADER_AV_ENTRY(5, self->phdr_count);  /* AT_PHNUM */

	/* Other values */
	MIPS_LOADER_AV_ENTRY(6, MEM_PAGE_SIZE);  /* AT_PAGESZ */
	MIPS_LOADER_AV_ENTRY(7, 0);  /* AT_BASE */
	MIPS_LOADER_AV_ENTRY(8, 0);  /* AT_FLAGS */
	MIPS_LOADER_AV_ENTRY(9, self->prog_entry);  /* AT_ENTRY */
	MIPS_LOADER_AV_ENTRY(11, getuid());  /* AT_UID */
	MIPS_LOADER_AV_ENTRY(12, geteuid());  /* AT_EUID */
	MIPS_LOADER_AV_ENTRY(13, getgid());  /* AT_GID */
	MIPS_LOADER_AV_ENTRY(14, getegid());  /* AT_EGID */
	MIPS_LOADER_AV_ENTRY(17, 0x64);  /* AT_CLKTCK */
	MIPS_LOADER_AV_ENTRY(23, 0);  /* AT_SECURE */

	/* Random bytes */
	self->at_random_addr_holder = sp + 4;
	MIPS_LOADER_AV_ENTRY(25, 0);  /* AT_RANDOM */

	/*MIPS_LOADER_AV_ENTRY(32, 0xffffe400);
	MIPS_LOADER_AV_ENTRY(33, 0xffffe000);
	MIPS_LOADER_AV_ENTRY(16, 0xbfebfbff);*/

	/* ??? AT_HWCAP, AT_PLATFORM, 32 and 33 ???*/

	/* Finally, AT_NULL, and return size */
	MIPS_LOADER_AV_ENTRY(0, 0);
	return sp - where;
}
#undef MIPS_LOADER_AV_ENTRY


/* Load stack for the Mips implementation */
static void MIPSContextLoadStack(MIPSContext *self)
{
	/* FIXME: Implement the stack loading function for MIPS and the stack
	 * layout for the same. Current implementation matches that of i386.
	 * Investigation n MIPS stack is in progress
	 */

		struct mem_t *mem = self->mem;
		unsigned int sp, argc, argvp, envp;
		unsigned int zero = 0;
		char *str;
		int i;

		/* Allocate stack */
		self->stack_base = LD_STACK_BASE;
		self->stack_size = LD_STACK_SIZE;
		self->stack_top = LD_STACK_BASE - LD_STACK_SIZE;
		mem_map(mem, self->stack_top, self->stack_size, mem_access_read | mem_access_write);
		mips_loader_debug("mapping region for stack from 0x%x to 0x%x\n",
			self->stack_top, self->stack_base - 1);

		/* Load arguments and environment variables */
		self->environ_base = LD_STACK_BASE - LD_MAX_ENVIRON;
		sp = self->environ_base;
		argc = linked_list_count(self->args);
		mips_loader_debug("  saved 'argc=%d' at 0x%x\n", argc, sp);
		mem_write(mem, sp, 4, &argc);
		sp += 4;
		argvp = sp;
		sp = sp + (argc + 1) * 4;

		/* Save space for environ and null */
		envp = sp;
		sp += linked_list_count(self->env) * 4 + 4;

		/* Load here the auxiliary vector MIPS */
		sp += MIPSContextLoadAuxiliaryVector(self, sp);


		/* Write arguments into stack */
		mips_loader_debug("\nArguments:\n");
		for (i = 0; i < argc; i++)
		{
			linked_list_goto(self->args, i);
			str = linked_list_get(self->args);
			mem_write(mem, argvp + i * 4, 4, &sp);
			mem_write_string(mem, sp, str);
			mips_loader_debug("  argument %d at 0x%x: '%s'\n", i, sp, str);
			sp += strlen(str) + 1;
		}
		mem_write(mem, argvp + i * 4, 4, &zero);

		/* Write environment variables */
		mips_loader_debug("\nEnvironment variables:\n");
		mips_loader_debug("\nMax Environment variables are %d\n",
			linked_list_count(self->env)); /* FIXME: */
		for (i = 0; i < linked_list_count(self->env); i++)
		{
			linked_list_goto(self->env, i);
			str = linked_list_get(self->env);
			mem_write(mem, envp + i * 4, 4, &sp);
			mem_write_string(mem, sp, str);
			mips_loader_debug("  env var %d at 0x%x: '%s'\n", i, sp, str);
			sp += strlen(str) + 1;
		}
		mem_write(mem, envp + i * 4, 4, &zero);

		/* Random bytes */
		self->at_random_addr = sp;
		for (i = 0; i < 16; i++)
		{
			unsigned char c = random();
			mem_write(mem, sp, 1, &c);
			sp++;
		}

		mips_loader_debug("\n at_random_addr_holder = %x , at_random_addr = %x \n",
			self->at_random_addr_holder, self->at_random_addr);
		mem_write(mem, self->at_random_addr_holder, 4, &self->at_random_addr);

		/* Check that we didn't overflow */
		if (sp > LD_STACK_BASE)
			fatal("%s: initial stack overflow, increment LD_MAX_ENVIRON",
				__FUNCTION__);
}


void MIPSContextLoadExecutable(MIPSContext *self, char *path)
{

	struct mem_t *mem = self->mem;
	struct mips_file_desc_table_t *fdt = self->file_desc_table;

	char stdin_file_full_path[MAX_STRING_SIZE];
	char stdout_file_full_path[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];

	/* Alternative stdin */
	MIPSContextGetFullPath(self, self->stdin_file, stdin_file_full_path, MAX_STRING_SIZE);
	if (*stdin_file_full_path)
	{
		struct mips_file_desc_t *fd;
		fd = mips_file_desc_table_entry_get(fdt, 0);
		assert(fd);
		fd->host_fd = open(stdin_file_full_path, O_RDONLY);
		if (fd->host_fd < 0)
			fatal("%s: cannot open stdin", self->stdin_file);
		mips_loader_debug("%s: stdin redirected\n", stdin_file_full_path);
	}

	/* Alternative stdout/stderr */
	MIPSContextGetFullPath(self, self->stdout_file, stdout_file_full_path, MAX_STRING_SIZE);
	if (*stdout_file_full_path)
	{
		struct mips_file_desc_t *fd1, *fd2;
		fd1 = mips_file_desc_table_entry_get(fdt, 1);
		fd2 = mips_file_desc_table_entry_get(fdt, 2);
		assert(fd1 && fd2);
		fd1->host_fd = fd2->host_fd = open(stdout_file_full_path,
			O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, 0660);
		if (fd1->host_fd < 0)
			fatal("%s: cannot open stdout/stderr", self->stdout_file);
		mips_loader_debug("%s: stdout redirected\n", stdout_file_full_path);
	}


	/* Load program into memory */
	MIPSContextGetFullPath(self, path, exe_full_path, MAX_STRING_SIZE);
	self->elf_file = elf_file_create_from_path(exe_full_path);
	self->exe = xstrdup(exe_full_path);

	/* Read sections and program entry */
	MIPSContextLoadSections(self, self->elf_file);
	self->prog_entry = self->elf_file->header->e_entry;

	/* Set heap break to the highest written address rounded up to
	 * the memory page boundary. */
	mem->heap_break = ROUND_UP(mem->heap_break, MEM_PAGE_SIZE);

	/* Load program header table. If we found a PT_INTERP program header,
	 * we have to load the program interpreter. This means we are dealing with
	 * a dynamically linked application. */
	MIPSContextLoadProgramHeaders(self);

	/* Stack */
	MIPSContextLoadStack(self);

	/* Initialization */
	self->next_ip = self->prog_entry;
	self->n_next_ip = self->prog_entry + 4;
	self->regs->regs_R[29] = self->environ_base;

	mips_loader_debug("Program entry is 0x%x\n", self->next_ip);
	mips_loader_debug("Initial stack pointer is 0x%x\n", self->regs->regs_R[29]);
	mips_loader_debug("Heap start set to 0x%x\n", mem->heap_break);
}


void MIPSContextCreate(MIPSContext *self, MIPSEmu *emu)
{
	/* Effective creation */
	MIPSContextDoCreate(self, emu);

	/* Memory */
	self->mem = mem_create();

	/* Initialize Loader Sections*/
	self->args = linked_list_create();
	self->env = linked_list_create();

	self->file_desc_table = mips_file_desc_table_create();
}


void MIPSContextDestroy(MIPSContext *self)
{
	MIPSEmu *emu = self->emu;

	/* If context is not finished/zombie, finish it first.
	 * This removes all references to current freed context. */
	if (!MIPSContextGetState(self, MIPSContextFinished | MIPSContextZombie))
		MIPSContextFinish(self, 0);

	/* Remove context from finished contexts list. This should
	 * be the only list the context is in right now. */
	assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, running, self)));
	assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, suspended, self)));
	assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, zombie, self)));
	assert((DOUBLE_LINKED_LIST_MEMBER(emu, finished, self)));
	DOUBLE_LINKED_LIST_REMOVE(emu, finished, self);

	/* Free private structures */
	mips_regs_free(self->regs);

	/* Free ELF file  */
	if (self->elf_file)
		elf_file_free(self->elf_file);

	/* Free arguments */
	fflush(NULL);
	LINKED_LIST_FOR_EACH(self->args)
	free(linked_list_get(self->args));
	linked_list_free(self->args);

	/* Free environment variables */
	LINKED_LIST_FOR_EACH(self->env)
	free(linked_list_get(self->env));
	linked_list_free(self->env);

	/* Free loader */
	free(self->exe);
	free(self->cwd);
	free(self->stdin_file);
	free(self->stdout_file);

	/* Free Memory */
	mem_unlink(self->mem);
	mips_file_desc_table_unlink(self->file_desc_table);

	/* Free Call Stack */
	free(self->cstack);

	/* Remove context from contexts list and free */
	assert(DOUBLE_LINKED_LIST_MEMBER(emu, context, self));
	DOUBLE_LINKED_LIST_REMOVE(emu, context, self);
	MIPSContextDebug("context %d freed\n", self->pid);

	/* Instruction */
	MIPSInstWrapFree(self->inst);
}


void MIPSContextAddArgsVector(MIPSContext *self, int argc, char **argv)
{
	char *arg;
	int i;

	for (i = 0; i < argc; i++)
	{
		/* Add */
		arg = xstrdup(argv[i]);
		linked_list_add(self->args, arg);
	}
}


void MIPSContextAddEnviron(MIPSContext *self, char *env)
{
	extern char **environ;
	char *next;
	int i;

	/* Add variables from actual environment */
	for (i = 0; environ[i]; i++)
		linked_list_add(self->env, xstrdup(environ[i]));

	/* Add the environment vars provided in 'env' */
	while (env)
	{
		/* Skip spaces */
		while (*env == ' ')
			env++;
		if (!*env)
			break;

		/* Get new environment variable */
		switch (*env)
		{

		case '"':
		case '\'':
			if (!(next = strchr(env + 1, *env)))
				fatal("%s: wrong format", __FUNCTION__);
			*next = 0;
			linked_list_add(self->env, xstrdup(env + 1));
			env = next + 1;
			break;

		default:
			linked_list_add(self->env, xstrdup(env));
			env = NULL;
		}
	}
}


void MIPSContextExecute(MIPSContext *self)
{
	MIPSEmu *emu = self->emu;

	struct mips_regs_t *regs = self->regs;
	struct mem_t *mem = self->mem;

	unsigned int *buffer_ptr;
	int spec_mode;

	/* Memory permissions should not be checked if the context is executing in
	 * speculative mode. This will prevent guest segmentation faults to occur. */
	spec_mode = MIPSContextGetState(self, MIPSContextSpecMode);
	mem->safe = spec_mode ? 0 : mem_safe_mode;

	/* Read instruction from memory. Memory should be accessed here in unsafe mode
	 * (i.e., allowing segmentation faults) if executing speculatively. */
	self->regs->pc = self->next_ip;
	buffer_ptr = mem_get_buffer(mem, (regs->pc), 4, mem_access_exec);

	/* FIXME: Mips speculative mode execution to be added */
	if (!buffer_ptr)
		fatal("mem_get_buffer error\n");

	mem->safe = mem_safe_mode;

	/* Disassemble */
	MIPSInstWrapDecode(self->inst, regs->pc, buffer_ptr);
	if (MIPSInstWrapGetOpcode(self->inst) == MIPSInstOpcodeInvalid)
		fatal("0x%x: instruction not implemented\nOpcode: %x\n",
			regs->pc,
			MIPSInstWrapGetBytes(self->inst)->standard.opc);
	else
		mips_loader_debug("Instruction decoded:%8x - %s\n",
				MIPSInstWrapGetAddress(self->inst),
				MIPSInstWrapGetName(self->inst));

	/* Execute instruction */
	mips_isa_execute_inst(self);

	/* Statistics */
	asEmu(emu)->instructions++;
}


/* Finish a context group. This call does a subset of action of the 'mips_ctx_finish'
 * call, but for all parent and child contexts sharing a memory map. */
void MIPSContextFinishGroup(MIPSContext *self, int state)
{
	MIPSEmu *emu = self->emu;
	MIPSContext *aux;

	/* Get group parent */
	if (self->group_parent)
		self = self->group_parent;
	assert(!self->group_parent);  /* Only one level */

	/* Context already finished */
	if (MIPSContextGetState(self, MIPSContextFinished | MIPSContextZombie))
		return;

	/* Finish all contexts in the group */
	DOUBLE_LINKED_LIST_FOR_EACH(emu, context, aux)
	{
		if (aux->group_parent != self && aux != self)
			continue;

		/* TODO: Thread support to be added for Mips */
		/*
		if (mips_ctx_get_status(aux, mips_ctx_zombie))
			mips_ctx_set_status(aux, mips_ctx_finished);
		if (mips_ctx_get_status(aux, mips_ctx_handler))
			signal_handler_return(aux);
		mips_ctx_host_thread_suspend_cancel(aux);
		mips_ctx_host_thread_timer_cancel(aux);
		*/

		/* Child context of 'ctx' goes to state 'finished'.
		 * Context 'ctx' goes to state 'zombie' or 'finished' if it has a parent */
		if (aux == self)
			MIPSContextSetState(aux, aux->parent ? MIPSContextZombie : MIPSContextFinished);
		else
			MIPSContextSetState(aux, MIPSContextFinished);
		aux->exit_code = state;
	}

	/* Process events */
	MIPSEmuProcessEventsSchedule(emu);
}


/* Finish a context. If the context has no parent, its status will be set
 * to 'mips_ctx_finished'. If it has, its status is set to 'mips_ctx_zombie', waiting for
 * a call to 'waitpid'.
 * The children of the finished context will set their 'parent' attribute to NULL.
 * The zombie children will be finished. */
void MIPSContextFinish(MIPSContext *self, int state)
{
	MIPSEmu *emu = self->emu;

	/* TODO: Multi-thread support for Mips */
	/*MIPSContext *aux;*/

	/* Context already finished */
	if (MIPSContextGetState(self, MIPSContextFinished | MIPSContextZombie))
		return;

	/* TODO: Multi-thread support for Mips */
	/*
	 If context is waiting for host events, cancel spawned host threads.
	mips_ctx_host_thread_suspend_cancel(ctx);
	mips_ctx_host_thread_timer_cancel(ctx);

	 From now on, all children have lost their parent. If a child is
	 * already zombie, finish it, since its parent won't be able to waitpid it
	 * anymore.
	DOUBLE_LINKED_LIST_FOR_EACH(emu, context, aux)
	{
		if (aux->parent == ctx)
		{
			aux->parent = NULL;
			if (mips_ctx_get_status(aux, mips_ctx_zombie))
				mips_ctx_set_status(aux, mips_ctx_finished);
		}
	}

	 Send finish signal to parent
	if (ctx->exit_signal && ctx->parent)
	{
		mips_sys_debug("  sending signal %d to pid %d\n",
			ctx->exit_signal, ctx->parent->pid);
		sim_sigset_add(&ctx->parent->signal_mask_table->pending,
			ctx->exit_signal);
		MIPSEmuProcessEventsSchedule();
	}

	 If clear_child_tid was set, a futex() call must be performed on
	 * that pointer. Also wake up futexes in the robust list.
	if (ctx->clear_child_tid)
	{
		unsigned int zero = 0;
		mem_write(ctx->mem, ctx->clear_child_tid, 4, &zero);
		mips_ctx_futex_wake(ctx, ctx->clear_child_tid, 1, -1);
	}
	mips_ctx_exit_robust_list(ctx);

	 If we are in a signal handler, stop it.
	if (mips_ctx_get_status(ctx, mips_ctx_handler))
		signal_handler_return(ctx);
	*/

	/* Finish context */
	MIPSContextSetState(self, self->parent ? MIPSContextZombie : MIPSContextFinished);
	self->exit_code = state;
	MIPSEmuProcessEventsSchedule(emu);
}


static void MIPSContextUpdateState(MIPSContext *self, MIPSContextState state)
{
	MIPSEmu *emu = self->emu;
	MIPSContextState diff;

	/* Remove contexts from the following lists:
	 *   running, suspended, zombie */
	if ((DOUBLE_LINKED_LIST_MEMBER(emu, running, self)))
		DOUBLE_LINKED_LIST_REMOVE(emu, running, self);

	if ((DOUBLE_LINKED_LIST_MEMBER(emu, suspended, self)))
		DOUBLE_LINKED_LIST_REMOVE(emu, suspended, self);

	if ((DOUBLE_LINKED_LIST_MEMBER(emu, zombie, self)))
		DOUBLE_LINKED_LIST_REMOVE(emu, zombie, self);

	if ((DOUBLE_LINKED_LIST_MEMBER(emu, finished, self)))
		DOUBLE_LINKED_LIST_REMOVE(emu, finished, self);

	if ((DOUBLE_LINKED_LIST_MEMBER(emu, alloc, self)))
		DOUBLE_LINKED_LIST_REMOVE(emu, alloc, self);

	/* If the difference between the old and new status lies in other
	 * states other than 'mips_ctx_specmode', a reschedule is marked. */
	diff = self->status ^ state;
	if (diff & ~MIPSContextSpecMode)
		emu->context_reschedule = 1;

	/* Update status */
	self->status = state;
	if (self->status & MIPSContextFinished)
		self->status = MIPSContextFinished | (state & MIPSContextAlloc);
	if (self->status & MIPSContextZombie)
		self->status = MIPSContextZombie | (state & MIPSContextAlloc);
	if (!(self->status & MIPSContextSuspended) &&
		!(self->status & MIPSContextFinished) &&
		!(self->status & MIPSContextZombie) &&
		!(self->status & MIPSContextLocked))
		self->status |= MIPSContextRunning;
	else
		self->status &= ~MIPSContextRunning;

	/* Insert context into the corresponding lists. */
	if (self->status & MIPSContextRunning)
	{
		assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, running, self)));
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, running, self);
	}
	if (self->status & MIPSContextZombie)
	{
		assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, zombie, self)));
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, zombie, self);
	}
	if (self->status & MIPSContextFinished)
	{
		assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, finished, self)));
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, finished, self);
	}
	if (self->status & MIPSContextSuspended)
	{
		assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, suspended, self)));
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, suspended, self);
	}
	if (self->status & MIPSContextAlloc)
	{
		assert(!(DOUBLE_LINKED_LIST_MEMBER(emu, alloc, self)));
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, alloc, self);
	}

	/* Dump new status (ignore 'mips_ctx_specmode' status, it's too frequent) */
	if (debug_status(mips_context_debug_category) && (diff & ~MIPSContextSpecMode))
	{
		char sstatus[200];
		str_map_flags(&mips_ctx_status_map, self->status, sstatus, 200);
		MIPSContextDebug("ctx %d changed status to %s\n",
			self->pid, sstatus);
	}

	/* Start/stop mips timer depending on whether there are any contexts
	 * currently running. */
	if (emu->running_list_count)
		m2s_timer_start(asEmu(emu)->timer);
	else
		m2s_timer_stop(asEmu(emu)->timer);
}


void MIPSContextSetState(MIPSContext *self, MIPSContextState state)
{
	MIPSContextUpdateState(self, self->status | state);
}


void MIPSContextClearState(MIPSContext *self, MIPSContextState state)
{
	MIPSContextUpdateState(self, self->status & ~state);
}


int MIPSContextGetState(MIPSContext *self, MIPSContextState state)
{
	return (self->status & state) > 0;
}


void MIPSContextGetFullPath(MIPSContext *self, char *path,
		char *full_path, int size)
{
	/* Remove './' prefix from 'file_name' */
	while (path && !strncmp(path, "./", 2))
		path += 2;

	/* File name is NULL or empty */
	assert(full_path);
	if (!path || !*path)
	{
		snprintf(full_path, size, "%s", "");
		return;
	}

	/* File name is given as an absolute path */
	if (*path == '/')
	{
		if (size < strlen(path) + 1)
			fatal("%s: buffer too small", __FUNCTION__);
		snprintf(full_path, size, "%s", path);
		return;
	}

	/* Relative path */
	if (strlen(self->cwd) + strlen(path) + 2 > size)
		fatal("%s: buffer too small", __FUNCTION__);
	snprintf(full_path, size, "%s/%s", self->cwd, path);
}


/* Generate virtual file '/proc/self/maps' and return it in 'path'. */
void MIPSContextGenerateProcSelfMaps(MIPSContext *self, char *path)
{
	unsigned int start, end;
	enum mem_access_t perm, page_perm;
	struct mem_page_t *page;
	struct mem_t *mem = self->mem;
	int fd;
	FILE *f = NULL;

	/* Create temporary file */
	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "wt")) == NULL)
		fatal("ctx_gen_proc_self_maps: cannot create temporary file");

	/* Get the first page */
	end = 0;
	for (;;)
	{
		/* Get start of next range */
		page = mem_page_get_next(mem, end);
		if (!page)
			break;
		start = page->tag;
		end = page->tag;
		perm = page->perm & (mem_access_read | mem_access_write | mem_access_exec);

		/* Get end of range */
		for (;;)
		{
			page = mem_page_get(mem, end + MEM_PAGE_SIZE);
			if (!page)
				break;
			page_perm = page->perm & (mem_access_read | mem_access_write | mem_access_exec);
			if (page_perm != perm)
				break;
			end += MEM_PAGE_SIZE;
			perm = page_perm;
		}

		/* Dump range */
		fprintf(f, "%08x-%08x %c%c%c%c 00000000 00:00", start, end + MEM_PAGE_SIZE,
			perm & mem_access_read ? 'r' : '-',
			perm & mem_access_write ? 'w' : '-',
			perm & mem_access_exec ? 'x' : '-',
			'p');
		fprintf(f, "\n");
	}

	/* Close file */
	fclose(f);
}
