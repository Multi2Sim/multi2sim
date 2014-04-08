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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "context.h"
#include "file.h"
#include "isa.h"
#include "regs.h"

int mips_loader_debug_category;
int mips_ctx_debug_category;

static struct str_map_t mips_ctx_status_map =
{
	16, {
		{ "running",      mips_ctx_running },
		{ "specmode",     mips_ctx_spec_mode },
		{ "suspended",    mips_ctx_suspended },
		{ "finished",     mips_ctx_finished },
		{ "exclusive",    mips_ctx_exclusive },
		{ "locked",       mips_ctx_locked },
		{ "handler",      mips_ctx_handler },
		{ "sigsuspend",   mips_ctx_sigsuspend },
		{ "nanosleep",    mips_ctx_nanosleep },
		{ "poll",         mips_ctx_poll },
		{ "read",         mips_ctx_read },
		{ "write",        mips_ctx_write },
		{ "waitpid",      mips_ctx_waitpid },
		{ "zombie",       mips_ctx_zombie },
		{ "futex",        mips_ctx_futex },
		{ "alloc",        mips_ctx_alloc }
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

static struct mips_ctx_t *mips_ctx_do_create()
{
	struct mips_ctx_t *ctx;

	/* Initialize */
	ctx = xcalloc(1, sizeof(struct mips_ctx_t));
	ctx->pid = mips_emu->current_pid++;

	/* Update status so that the context is inserted in the
	 * corresponding lists. The mips_ctx_running parameter has no
	 * effect, since it will be updated later. */
	mips_ctx_set_status(ctx, mips_ctx_running);
	MIPSEmuListInsertHead(mips_emu, mips_emu_list_context, ctx);

	/* Structures */
	ctx->regs = mips_regs_create();

	/* Return context */
	return ctx;
}

static void mips_ctx_loader_add_args_vector(struct mips_ctx_t *ctx, int argc, char **argv)
{
	char *arg;
	int i;

	for (i = 0; i < argc; i++)
	{
		/* Add */
		arg = xstrdup(argv[i]);
		linked_list_add(ctx->args, arg);
	}
}

/* Add environment variables from the actual environment plus
 * the list attached in the argument 'env'. */
static void mips_ctx_loader_add_environ(struct mips_ctx_t *ctx, char *env)
{
	extern char **environ;
	char *next;
	int i;

	/* Add variables from actual environment */
	for (i = 0; environ[i]; i++)
		linked_list_add(ctx->env, xstrdup(environ[i]));

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
			linked_list_add(ctx->env, xstrdup(env + 1));
			env = next + 1;
			break;

		default:
			linked_list_add(ctx->env, xstrdup(env));
			env = NULL;
		}
	}
}

/* Load program headers table */
static void mips_ctx_loader_load_program_headers(struct mips_ctx_t *ctx)
{
	struct mem_t *mem = ctx->mem;

	struct elf_file_t *elf_file = ctx->elf_file;
	struct elf_program_header_t *program_header;

	uint32_t phdt_base;
	uint32_t phdt_size;
	uint32_t phdr_count;
	uint32_t phdr_size;

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
	phdt_base = ctx->bottom - phdt_size;
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
	ctx->phdt_base = phdt_base;
	ctx->phdr_count = phdr_count;
}

/* Load sections from an ELF file */
static void mips_ctx_loader_load_sections(struct mips_ctx_t *ctx, struct elf_file_t *elf_file)
{
	struct mem_t *mem = ctx->mem;

	struct elf_section_t *section;
	int i;

	enum mem_access_t perm;
	char flags_str[200];

	mips_loader_debug("\nLoading ELF sections\n");
	ctx->bottom = 0xffffffff;
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
			ctx->bottom = MIN(ctx->bottom, section->header->sh_addr);

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
	uint32_t a_type = t; \
	uint32_t a_value = v; \
	mem_write(mem, sp, 4, &a_type); \
	mem_write(mem, sp + 4, 4, &a_value); \
	sp += 8; \
}

static uint32_t mips_load_av(struct mips_ctx_t *ctx, uint32_t where)
{

	struct mem_t *mem = ctx->mem;
	uint32_t sp = where;

	mips_loader_debug("Loading auxiliary vector at 0x%x\n", where);

	/* Program headers */
	MIPS_LOADER_AV_ENTRY(3, ctx->phdt_base);  /* AT_PHDR */
	MIPS_LOADER_AV_ENTRY(4, 32);  /* AT_PHENT -> program header size of 32 bytes */
	MIPS_LOADER_AV_ENTRY(5, ctx->phdr_count);  /* AT_PHNUM */

	/* Other values */
	MIPS_LOADER_AV_ENTRY(6, MEM_PAGE_SIZE);  /* AT_PAGESZ */
	MIPS_LOADER_AV_ENTRY(7, 0);  /* AT_BASE */
	MIPS_LOADER_AV_ENTRY(8, 0);  /* AT_FLAGS */
	MIPS_LOADER_AV_ENTRY(9, ctx->prog_entry);  /* AT_ENTRY */
	MIPS_LOADER_AV_ENTRY(11, getuid());  /* AT_UID */
	MIPS_LOADER_AV_ENTRY(12, geteuid());  /* AT_EUID */
	MIPS_LOADER_AV_ENTRY(13, getgid());  /* AT_GID */
	MIPS_LOADER_AV_ENTRY(14, getegid());  /* AT_EGID */
	MIPS_LOADER_AV_ENTRY(17, 0x64);  /* AT_CLKTCK */
	MIPS_LOADER_AV_ENTRY(23, 0);  /* AT_SECURE */

	/* Random bytes */
	ctx->at_random_addr_holder = sp + 4;
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
static void mips_ctx_loader_load_stack(struct mips_ctx_t *ctx)
{
	/* FIXME: Implement the stack loading function for MIPS and the stack
	 * layout for the same. Current implementation matches that of i386.
	 * Investigation n MIPS stack is in progress
	 */

		struct mem_t *mem = ctx->mem;
		uint32_t sp, argc, argvp, envp;
		uint32_t zero = 0;
		char *str;
		int i;

		/* Allocate stack */
		ctx->stack_base = LD_STACK_BASE;
		ctx->stack_size = LD_STACK_SIZE;
		ctx->stack_top = LD_STACK_BASE - LD_STACK_SIZE;
		mem_map(mem, ctx->stack_top, ctx->stack_size, mem_access_read | mem_access_write);
		mips_loader_debug("mapping region for stack from 0x%x to 0x%x\n",
			ctx->stack_top, ctx->stack_base - 1);

		/* Load arguments and environment variables */
		ctx->environ_base = LD_STACK_BASE - LD_MAX_ENVIRON;
		sp = ctx->environ_base;
		argc = linked_list_count(ctx->args);
		mips_loader_debug("  saved 'argc=%d' at 0x%x\n", argc, sp);
		mem_write(mem, sp, 4, &argc);
		sp += 4;
		argvp = sp;
		sp = sp + (argc + 1) * 4;

		/* Save space for environ and null */
		envp = sp;
		sp += linked_list_count(ctx->env) * 4 + 4;

		/* Load here the auxiliary vector MIPS */
		sp += mips_load_av(ctx, sp);


		/* Write arguments into stack */
		mips_loader_debug("\nArguments:\n");
		for (i = 0; i < argc; i++)
		{
			linked_list_goto(ctx->args, i);
			str = linked_list_get(ctx->args);
			mem_write(mem, argvp + i * 4, 4, &sp);
			mem_write_string(mem, sp, str);
			mips_loader_debug("  argument %d at 0x%x: '%s'\n", i, sp, str);
			sp += strlen(str) + 1;
		}
		mem_write(mem, argvp + i * 4, 4, &zero);

		/* Write environment variables */
		mips_loader_debug("\nEnvironment variables:\n");
		mips_loader_debug("\nMax Environment variables are %d\n",
			linked_list_count(ctx->env)); /* FIXME: */
		for (i = 0; i < linked_list_count(ctx->env); i++)
		{
			linked_list_goto(ctx->env, i);
			str = linked_list_get(ctx->env);
			mem_write(mem, envp + i * 4, 4, &sp);
			mem_write_string(mem, sp, str);
			mips_loader_debug("  env var %d at 0x%x: '%s'\n", i, sp, str);
			sp += strlen(str) + 1;
		}
		mem_write(mem, envp + i * 4, 4, &zero);

		/* Random bytes */
		ctx->at_random_addr = sp;
		for (i = 0; i < 16; i++)
		{
			unsigned char c = random();
			mem_write(mem, sp, 1, &c);
			sp++;
		}

		mips_loader_debug("\n at_random_addr_holder = %x , at_random_addr = %x \n",
			ctx->at_random_addr_holder, ctx->at_random_addr);
		mem_write(mem, ctx->at_random_addr_holder, 4, &ctx->at_random_addr);

		/* Check that we didn't overflow */
		if (sp > LD_STACK_BASE)
			fatal("%s: initial stack overflow, increment LD_MAX_ENVIRON",
				__FUNCTION__);
}
void mips_ctx_loader_load_exe(struct mips_ctx_t *ctx, char *exe)
{

	struct mem_t *mem = ctx->mem;
	struct mips_file_desc_table_t *fdt = ctx->file_desc_table;

	char stdin_file_full_path[MAX_STRING_SIZE];
	char stdout_file_full_path[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];

	/* Alternative stdin */
	mips_ctx_loader_get_full_path(ctx, ctx->stdin_file, stdin_file_full_path, MAX_STRING_SIZE);
	if (*stdin_file_full_path)
	{
		struct mips_file_desc_t *fd;
		fd = mips_file_desc_table_entry_get(fdt, 0);
		assert(fd);
		fd->host_fd = open(stdin_file_full_path, O_RDONLY);
		if (fd->host_fd < 0)
			fatal("%s: cannot open stdin", ctx->stdin_file);
		mips_loader_debug("%s: stdin redirected\n", stdin_file_full_path);
	}

	/* Alternative stdout/stderr */
	mips_ctx_loader_get_full_path(ctx, ctx->stdout_file, stdout_file_full_path, MAX_STRING_SIZE);
	if (*stdout_file_full_path)
	{
		struct mips_file_desc_t *fd1, *fd2;
		fd1 = mips_file_desc_table_entry_get(fdt, 1);
		fd2 = mips_file_desc_table_entry_get(fdt, 2);
		assert(fd1 && fd2);
		fd1->host_fd = fd2->host_fd = open(stdout_file_full_path,
			O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, 0660);
		if (fd1->host_fd < 0)
			fatal("%s: cannot open stdout/stderr", ctx->stdout_file);
		mips_loader_debug("%s: stdout redirected\n", stdout_file_full_path);
	}


	/* Load program into memory */
	mips_ctx_loader_get_full_path(ctx, exe, exe_full_path, MAX_STRING_SIZE);
	ctx->elf_file = elf_file_create_from_path(exe_full_path);
	ctx->exe = xstrdup(exe_full_path);

	/* Read sections and program entry */
	mips_ctx_loader_load_sections(ctx, ctx->elf_file);
	ctx->prog_entry = ctx->elf_file->header->e_entry;

	/* Set heap break to the highest written address rounded up to
	 * the memory page boundary. */
	mem->heap_break = ROUND_UP(mem->heap_break, MEM_PAGE_SIZE);

	/* Load program header table. If we found a PT_INTERP program header,
	 * we have to load the program interpreter. This means we are dealing with
	 * a dynamically linked application. */
	mips_ctx_loader_load_program_headers(ctx);

	/* Stack */
	mips_ctx_loader_load_stack(ctx);

	/* Initialization */
	ctx->next_ip = ctx->prog_entry;
	ctx->n_next_ip = ctx->prog_entry + 4;
	ctx->regs->regs_R[29] = ctx->environ_base;

	mips_loader_debug("Program entry is 0x%x\n", ctx->next_ip);
	mips_loader_debug("Initial stack pointer is 0x%x\n", ctx->regs->regs_R[29]);
	mips_loader_debug("Heap start set to 0x%x\n", mem->heap_break);
}

struct mips_ctx_t *mips_ctx_create(void)
{
	struct mips_ctx_t *ctx;

	mips_ctx_debug("Calling mips_ctx_do_create\n");
	ctx = mips_ctx_do_create();

	/* Memory */
	ctx->mem = mem_create();

	/* Initialize Loader Sections*/
	ctx->args = linked_list_create();
	ctx->env = linked_list_create();

	ctx->file_desc_table = mips_file_desc_table_create();

	/* Call Stack initialize */
//	ctx->cstack = mips_isa_cstack_create(ctx);

	return ctx;
}

void mips_ctx_free(struct mips_ctx_t *ctx)
{

	/* If context is not finished/zombie, finish it first.
	 * This removes all references to current freed context. */
	if (!mips_ctx_get_status(ctx, mips_ctx_finished | mips_ctx_zombie))
		mips_ctx_finish(ctx, 0);

	/* Remove context from finished contexts list. This should
	 * be the only list the context is in right now. */
	assert(!MIPSEmuListMember(mips_emu, mips_emu_list_running, ctx));
	assert(!MIPSEmuListMember(mips_emu, mips_emu_list_suspended, ctx));
	assert(!MIPSEmuListMember(mips_emu, mips_emu_list_zombie, ctx));
	assert(MIPSEmuListMember(mips_emu, mips_emu_list_finished, ctx));
	MIPSEmuListRemove(mips_emu, mips_emu_list_finished, ctx);

	/* Free private structures */
	mips_regs_free(ctx->regs);

	/* Check no more links */

	assert(ctx->num_links >= 0);
		if (ctx->num_links)
			ctx->num_links--;
	/*assert(!ctx->num_links);*/

	/* Free ELF file  */
	if (ctx->elf_file)
		elf_file_free(ctx->elf_file);

	/* Free arguments */
	fflush(NULL);
	LINKED_LIST_FOR_EACH(ctx->args)
	free(linked_list_get(ctx->args));
	linked_list_free(ctx->args);

	/* Free environment variables */
	LINKED_LIST_FOR_EACH(ctx->env)
	free(linked_list_get(ctx->env));
	linked_list_free(ctx->env);

	/* Free loader */
	free(ctx->exe);
	free(ctx->cwd);
	free(ctx->stdin_file);
	free(ctx->stdout_file);

	/* Free Memory */
	mem_unlink(ctx->mem);
	mips_file_desc_table_unlink(ctx->file_desc_table);

	/* Free Call Stack */
	free(ctx->cstack);

	/* Remove context from contexts list and free */
	MIPSEmuListRemove(mips_emu, mips_emu_list_context, ctx);
	mips_ctx_debug("context %d freed\n", ctx->pid);

	free(ctx);
}

//unsigned int mips_ctx_check_fault(struct mips_ctx_t *ctx)
//{
//	struct mips_regs_t *regs = ctx->regs;
//	unsigned int ret_val;
//
//	ret_val = 0;
//	switch (regs->pc)
//	{
//	case (0xffff0fe0 + 4):
//		mips_isa_inst_debug(
//			"  Fault handled\n Fault location : 0x%x\n pc restored at : 0x%x\n\n", regs->pc, regs->lr);
//		ret_val = 0xffff0fe0;
//		break;
//
//	case (0xffff0fc0 + 4):
//		mips_isa_inst_debug(
//			"  Fault handled\n Fault location : 0x%x\n pc restored at : 0x%x\n\n", regs->pc, regs->lr);
//		ret_val = 0xffff0fc0;
//		break;
//
//	case (0xffff0fa0 + 4):
//		mips_isa_inst_debug(
//			"  Fault handled\n Fault location : 0x%x\n pc restored at : 0x%x\n\n", regs->pc, regs->lr);
//		ret_val = 0xffff0fa0;
//		break;
//
//	default:
//		ret_val = 0;
//		break;
//	}
//
//	return (ret_val);
//}

void mips_ctx_execute(struct mips_ctx_t *ctx)
{
	struct mips_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int *buffer_ptr;
//	unsigned int fault_id;
	int spec_mode;

	/* Memory permissions should not be checked if the context is executing in
	 * speculative mode. This will prevent guest segmentation faults to occur. */
	spec_mode = mips_ctx_get_status(ctx, mips_ctx_spec_mode);
	mem->safe = spec_mode ? 0 : mem_safe_mode;

	/* Check for fault code execution in Mips */
//	fault_id = mips_ctx_check_fault(ctx);
//	if (fault_id)
//	{
//		ctx->regs->pc = ctx->regs->lr + 4;
//
//		/* Hard-coded structure for fault handle */
//		if(fault_id == 0xffff0fe0)
//		{
//			ctx->regs->r0 = 0x5bd4c0;
//		}
//		else if(fault_id == 0xffff0fc0)
//		{
//			ctx->regs->r3 = 0;
//		}
//		else if(fault_id == 0xffff0fa0)
//		{
//
//		}
//
//	}

	/* Read instruction from memory. Memory should be accessed here in unsafe mode
	 * (i.e., allowing segmentation faults) if executing speculatively. */
	ctx->regs->pc = ctx->next_ip;
	buffer_ptr = mem_get_buffer(mem, (regs->pc), 4, mem_access_exec);

	/* FIXME: Mips speculative mode execution to be added */
	if (!buffer_ptr)
		fatal("mem_get_buffer error\n");
	/*{
		 Disable safe mode. If a part of the 20 read bytes does not belong to the
		 * actual instruction, and they lie on a page with no permissions, this would
		 * generate an undesired protection fault.
		mem->safe = 0;
		buffer_ptr = buffer;
		mem_access(mem, regs->ip, 4, buffer_ptr, mem_access_exec);
	}
	*/

	mem->safe = mem_safe_mode;

	/* Disassemble */
	mips_disasm(*buffer_ptr, (regs->pc), &ctx->inst);
	if (ctx->inst.info->opcode == MIPS_INST_NONE)/*&& !spec_mode)*/
		fatal("0x%x: not supported mips instruction\nOpcode: %x\n",
			(regs->pc), ctx->inst.dword.standard.opc);
	else
		mips_loader_debug("Instruction decoded:%8x - %s\n", ctx->inst.addr, ctx->inst.info->name);

	/* Execute instruction */
	mips_isa_execute_inst(ctx);

	/* Statistics */
	asEmu(mips_emu)->instructions++;
}

/* Finish a context group. This call does a subset of action of the 'mips_ctx_finish'
 * call, but for all parent and child contexts sharing a memory map. */
void mips_ctx_finish_group(struct mips_ctx_t *ctx, int status)
{
	struct mips_ctx_t *aux;

	/* Get group parent */
	if (ctx->group_parent)
		ctx = ctx->group_parent;
	assert(!ctx->group_parent);  /* Only one level */

	/* Context already finished */
	if (mips_ctx_get_status(ctx, mips_ctx_finished | mips_ctx_zombie))
		return;

	/* Finish all contexts in the group */
	DOUBLE_LINKED_LIST_FOR_EACH(mips_emu, context, aux)
	{
		if (aux->group_parent != ctx && aux != ctx)
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
		if (aux == ctx)
			mips_ctx_set_status(aux, aux->parent ? mips_ctx_zombie : mips_ctx_finished);
		else
			mips_ctx_set_status(aux, mips_ctx_finished);
		aux->exit_code = status;
	}

	/* Process events */
	MIPSEmuProcessEventsSchedule(mips_emu);
}


/* Finish a context. If the context has no parent, its status will be set
 * to 'mips_ctx_finished'. If it has, its status is set to 'mips_ctx_zombie', waiting for
 * a call to 'waitpid'.
 * The children of the finished context will set their 'parent' attribute to NULL.
 * The zombie children will be finished. */
void mips_ctx_finish(struct mips_ctx_t *ctx, int status)
{
	/* TODO: Multi-thread support for Mips */
	/*struct mips_ctx_t *aux;*/

	/* Context already finished */
	if (mips_ctx_get_status(ctx, mips_ctx_finished | mips_ctx_zombie))
		return;

	/* TODO: Multi-thread support for Mips */
	/*
	 If context is waiting for host events, cancel spawned host threads.
	mips_ctx_host_thread_suspend_cancel(ctx);
	mips_ctx_host_thread_timer_cancel(ctx);

	 From now on, all children have lost their parent. If a child is
	 * already zombie, finish it, since its parent won't be able to waitpid it
	 * anymore.
	DOUBLE_LINKED_LIST_FOR_EACH(mips_emu, context, aux)
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
		uint32_t zero = 0;
		mem_write(ctx->mem, ctx->clear_child_tid, 4, &zero);
		mips_ctx_futex_wake(ctx, ctx->clear_child_tid, 1, -1);
	}
	mips_ctx_exit_robust_list(ctx);

	 If we are in a signal handler, stop it.
	if (mips_ctx_get_status(ctx, mips_ctx_handler))
		signal_handler_return(ctx);
	*/

	/* Finish context */
	mips_ctx_set_status(ctx, ctx->parent ? mips_ctx_zombie : mips_ctx_finished);
	ctx->exit_code = status;
	MIPSEmuProcessEventsSchedule(mips_emu);
}

static void mips_ctx_update_status(struct mips_ctx_t *ctx, enum mips_ctx_status_t status)
{
	enum mips_ctx_status_t status_diff;

	/* Remove contexts from the following lists:
	 *   running, suspended, zombie */
	if (MIPSEmuListMember(mips_emu, mips_emu_list_running, ctx))
		MIPSEmuListRemove(mips_emu, mips_emu_list_running, ctx);
	if (MIPSEmuListMember(mips_emu, mips_emu_list_suspended, ctx))
		MIPSEmuListRemove(mips_emu, mips_emu_list_suspended, ctx);
	if (MIPSEmuListMember(mips_emu, mips_emu_list_zombie, ctx))
		MIPSEmuListRemove(mips_emu, mips_emu_list_zombie, ctx);
	if (MIPSEmuListMember(mips_emu, mips_emu_list_finished, ctx))
		MIPSEmuListRemove(mips_emu, mips_emu_list_finished, ctx);
	if (MIPSEmuListMember(mips_emu, mips_emu_list_alloc, ctx))
		MIPSEmuListRemove(mips_emu, mips_emu_list_alloc, ctx);

	/* If the difference between the old and new status lies in other
	 * states other than 'mips_ctx_specmode', a reschedule is marked. */
	status_diff = ctx->status ^ status;
	if (status_diff & ~mips_ctx_spec_mode)
		mips_emu->context_reschedule = 1;

	/* Update status */
	ctx->status = status;
	if (ctx->status & mips_ctx_finished)
		ctx->status = mips_ctx_finished | (status & mips_ctx_alloc);
	if (ctx->status & mips_ctx_zombie)
		ctx->status = mips_ctx_zombie | (status & mips_ctx_alloc);
	if (!(ctx->status & mips_ctx_suspended) &&
		!(ctx->status & mips_ctx_finished) &&
		!(ctx->status & mips_ctx_zombie) &&
		!(ctx->status & mips_ctx_locked))
		ctx->status |= mips_ctx_running;
	else
		ctx->status &= ~mips_ctx_running;

	/* Insert context into the corresponding lists. */
	if (ctx->status & mips_ctx_running)
		MIPSEmuListInsertHead(mips_emu, mips_emu_list_running, ctx);
	if (ctx->status & mips_ctx_zombie)
		MIPSEmuListInsertHead(mips_emu, mips_emu_list_zombie, ctx);
	if (ctx->status & mips_ctx_finished)
		MIPSEmuListInsertHead(mips_emu, mips_emu_list_finished, ctx);
	if (ctx->status & mips_ctx_suspended)
		MIPSEmuListInsertHead(mips_emu, mips_emu_list_suspended, ctx);
	if (ctx->status & mips_ctx_alloc)
		MIPSEmuListInsertHead(mips_emu, mips_emu_list_alloc, ctx);

	/* Dump new status (ignore 'mips_ctx_specmode' status, it's too frequent) */
	if (debug_status(mips_ctx_debug_category) && (status_diff & ~mips_ctx_spec_mode))
	{
		char sstatus[200];
		str_map_flags(&mips_ctx_status_map, ctx->status, sstatus, 200);
		mips_ctx_debug("ctx %d changed status to %s\n",
			ctx->pid, sstatus);
	}

	/* Start/stop mips timer depending on whether there are any contexts
	 * currently running. */
	if (mips_emu->running_list_count)
		m2s_timer_start(asEmu(mips_emu)->timer);
	else
		m2s_timer_stop(asEmu(mips_emu)->timer);
}

void mips_ctx_set_status(struct mips_ctx_t *ctx, enum mips_ctx_status_t status)
{
	mips_ctx_update_status(ctx, ctx->status | status);
}

void mips_ctx_clear_status(struct mips_ctx_t *ctx, enum mips_ctx_status_t status)
{
	mips_ctx_update_status(ctx, ctx->status & ~status);
}

int mips_ctx_get_status(struct mips_ctx_t *ctx, enum mips_ctx_status_t status)
{
	return (ctx->status & status) > 0;
}

/* Look for a context matching pid in the list of existing
 * contexts of the kernel. */
struct mips_ctx_t *mips_ctx_get(int pid)
{
	struct mips_ctx_t *ctx;

	ctx = mips_emu->context_list_head;
	while (ctx && ctx->pid != pid)
		ctx = ctx->context_list_next;
	return ctx;
}

void mips_ctx_load_from_command_line(int argc, char **argv)
{
	struct mips_ctx_t *ctx;

	char buf[MAX_STRING_SIZE];

	/* Create context */
	mips_ctx_debug("Creating context\n");
	ctx = mips_ctx_create();
	mips_ctx_debug("Context Created\n");
	/* Arguments and environment */
	mips_ctx_loader_add_args_vector(ctx, argc, argv);
	mips_ctx_loader_add_environ(ctx, "");

	/* Get current directory */
	ctx->cwd = getcwd(buf, sizeof buf);
	if (!ctx->cwd)
		panic("%s: buffer too small", __FUNCTION__);

	/* Redirections */
	ctx->cwd = xstrdup(ctx->cwd);
	ctx->stdin_file = xstrdup("");
	ctx->stdout_file = xstrdup("");

	/* Load executable */
	mips_loader_debug("Loading executable\n");
	mips_ctx_loader_load_exe(ctx, argv[0]);
}

void mips_ctx_loader_get_full_path(struct mips_ctx_t *ctx, char *file_name, char *full_path, int size)
{
	/* Remove './' prefix from 'file_name' */
	while (file_name && !strncmp(file_name, "./", 2))
		file_name += 2;

	/* File name is NULL or empty */
	assert(full_path);
	if (!file_name || !*file_name)
	{
		snprintf(full_path, size, "%s", "");
		return;
	}

	/* File name is given as an absolute path */
	if (*file_name == '/')
	{
		if (size < strlen(file_name) + 1)
			fatal("%s: buffer too small", __FUNCTION__);
		snprintf(full_path, size, "%s", file_name);
		return;
	}

	/* Relative path */
	if (strlen(ctx->cwd) + strlen(file_name) + 2 > size)
		fatal("%s: buffer too small", __FUNCTION__);
	snprintf(full_path, size, "%s/%s", ctx->cwd, file_name);
}

void mips_ctx_load_from_ctx_config(struct config_t *config, char *section)
{
}


/* Generate virtual file '/proc/self/maps' and return it in 'path'. */
void mips_ctx_gen_proc_self_maps(struct mips_ctx_t *ctx, char *path)
{
	uint32_t start, end;
	enum mem_access_t perm, page_perm;
	struct mem_page_t *page;
	struct mem_t *mem = ctx->mem;
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
