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

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <arch/common/arch.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/file.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "file-desc.h"
#include "loader.h"
#include "regs.h"


int x86_loader_debug_category;

char *x86_loader_help =
	"A context configuration file contains a list of executable programs and\n"
	"their parameters that will be simulated by Multi2Sim. The context\n"
	"configuration file is a plain text file in the IniFile format, containing\n"
	"as many sections as x86 programs simulated. Each program is denoted with\n"
	"a section called '[ Context <num> ]', where <num> is an integer number\n"
	"starting from 0.\n"
	"\n"
	"Variables in section '[ Context <num> ]':\n"
	"\n"
	"  Exe = <path> (Required)\n"
	"      Path for the x86 executable file that will be simulated.\n"
	"  Args = <arg_list>\n"
	"      List of command-line arguments for the simulated program.\n"
	"  Env = <env_list>\n"
	"      List of environment variables enumerated using single or double\n"
	"      quotes. These variables will be added to the current set of\n"
	"      active environment variables.\n"
	"      E.g.: Env = 'ENV_VAR1=100' \"ENV_VAR2=200\"\n"
	"  Cwd = <path>\n"
	"      Current working directory for simulated program. If not specified,\n"
	"      the current working directory for the simulator will be also used\n"
	"      for the simulated program.\n"
	"  StdIn = <file>\n"
	"      File to use as standard input for the simulated program. If none\n"
	"      specified, the simulator standard input is selected.\n"
	"  StdOut = <file>\n"
	"      File to use as standard output and standard error output for the\n"
	"      simulated program. If none specified, the standard output for the\n"
	"      simulator is used in both cases.\n"
	"  IPCReport = <file>\n"
	"      File to dump a report of the context performance. At specific\n"
	"      intervals, the context IPC (instructions-per-cycle) value will be\n"
	"      dumped in this file. This option must be specified together with\n"
	"      command-line option '--x86-sim detailed'.\n"
	"  IPCReportInterval = <cycles>\n"
	"      Interval in number of cycles that a new record will be added into\n"
	"      the IPC report file.\n"
	"\n"
	"See the Multi2Sim Guide (www.multi2sim.org) for further details and\n"
	"examples on how to use the context configuration file.\n"
	"\n";

static struct str_map_t elf_section_flags_map =
{
	3, {
		{ "SHF_WRITE", 1 },
		{ "SHF_ALLOC", 2 },
		{ "SHF_EXECINSTR", 4 }
	}
};


static void x86_loader_add_args_vector(struct x86_ctx_t *ctx, int argc, char **argv)
{
	struct x86_loader_t *ld = ctx->loader;

	char *arg;
	int i;

	for (i = 0; i < argc; i++)
	{
		arg = str_set(NULL, argv[i]);
		linked_list_add(ld->args, arg);
	}
}


static void x86_loader_add_args_string(struct x86_ctx_t *ctx, char *args)
{
	struct x86_loader_t *ld = ctx->loader;

	char *delim = " ";
	char *arg;
	
	/* Duplicate argument string */
	args = str_set(NULL, args);

	/* Tokens */
	for (arg = strtok(args, delim); arg; arg = strtok(NULL, delim))
	{
		arg = str_set(NULL, arg);
		linked_list_add(ld->args, arg);
	}

	/* Free argument string */
	str_free(args);
}


/* Add environment variables from the actual environment plus
 * the list attached in the argument 'env'. */
static void x86_loader_add_environ(struct x86_ctx_t *ctx, char *env)
{
	struct x86_loader_t *ld = ctx->loader;
	extern char **environ;

	char *next;
	char *str;

	int i;

	/* Add variables from actual environment */
	for (i = 0; environ[i]; i++)
	{
		str = str_set(NULL, environ[i]);
		linked_list_add(ld->env, str);
	}
	
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
			str = str_set(NULL, env + 1);
			linked_list_add(ld->env, str);
			env = next + 1;
			break;

		default:
			str = str_set(NULL, env);
			linked_list_add(ld->env, str);
			env = NULL;
		}
	}
}


#define LD_STACK_BASE  0xc0000000
#define LD_MAX_ENVIRON  0x10000  /* 16KB for environment */
#define LD_STACK_SIZE  0x800000  /* 8MB stack size */


/* Load sections from an ELF file */
static void x86_loader_load_sections(struct x86_ctx_t *ctx, struct elf_file_t *elf_file)
{
	struct mem_t *mem = ctx->mem;
	struct x86_loader_t *ld = ctx->loader;

	struct elf_section_t *section;
	int i;

	enum mem_access_t perm;
	char flags_str[200];

	x86_loader_debug("\nLoading ELF sections\n");
	ld->bottom = 0xffffffff;
	for (i = 0; i < list_count(elf_file->section_list); i++)
	{
		section = list_get(elf_file->section_list, i);

		perm = mem_access_init | mem_access_read;
		str_map_flags(&elf_section_flags_map, section->header->sh_flags, flags_str, sizeof(flags_str));
		x86_loader_debug("  section %d: name='%s', offset=0x%x, addr=0x%x, size=%u, flags=%s\n",
			i, section->name, section->header->sh_offset, section->header->sh_addr, section->header->sh_size, flags_str);

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
			ld->bottom = MIN(ld->bottom, section->header->sh_addr);

			/* If section type is SHT_NOBITS (sh_type=8), initialize to 0.
			 * Otherwise, copy section contents from ELF file. */
			if (section->header->sh_type == 8)
			{
				void *ptr;

				ptr = xcalloc(1, section->header->sh_size);
				mem_access(mem, section->header->sh_addr, section->header->sh_size,
					ptr, mem_access_init);
				free(ptr);
			} else {
				mem_access(mem, section->header->sh_addr, section->header->sh_size,
					section->buffer.ptr, mem_access_init);
			}
		}
	}
}


static void x86_loader_load_interp(struct x86_ctx_t *ctx)
{
	struct x86_loader_t *ld = ctx->loader;
	struct elf_file_t *elf_file;

	/* Open dynamic loader */
	x86_loader_debug("\nLoading program interpreter '%s'\n", ld->interp);
	elf_file = elf_file_create_from_path(ld->interp);
	
	/* Load section from program interpreter */
	x86_loader_load_sections(ctx, elf_file);

	/* Change program entry to the one specified by the interpreter */
	ld->interp_prog_entry = elf_file->header->e_entry;
	x86_loader_debug("  program interpreter entry: 0x%x\n\n", ld->interp_prog_entry);
	elf_file_free(elf_file);
}


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


/* Load program headers table */
static void x86_loader_load_program_headers(struct x86_ctx_t *ctx)
{
	struct x86_loader_t *ld = ctx->loader;
	struct mem_t *mem = ctx->mem;

	struct elf_file_t *elf_file = ld->elf_file;
	struct elf_program_header_t *program_header;

	uint32_t phdt_base;
	uint32_t phdt_size;
	uint32_t phdr_count;
	uint32_t phdr_size;

	char str[MAX_STRING_SIZE];
	int i;

	/* Load program header table from ELF */
	x86_loader_debug("\nLoading program headers\n");
	phdr_count = elf_file->header->e_phnum;
	phdr_size = elf_file->header->e_phentsize;
	phdt_size = phdr_count * phdr_size;
	assert(phdr_count == list_count(elf_file->program_header_list));
	
	/* Program header PT_PHDR, specifying location and size of the program header table itself. */
	/* Search for program header PT_PHDR, specifying location and size of the program header table.
	 * If none found, choose ld->bottom - phdt_size. */
	phdt_base = ld->bottom - phdt_size;
	for (i = 0; i < list_count(elf_file->program_header_list); i++)
	{
		program_header = list_get(elf_file->program_header_list, i);
		if (program_header->header->p_type == PT_PHDR)
			phdt_base = program_header->header->p_vaddr;
	}
	x86_loader_debug("  virtual address for program header table: 0x%x\n", phdt_base);

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
		x86_loader_debug("  header loaded at 0x%x\n", phdt_base + i * phdr_size);
		x86_loader_debug("    type=%s, offset=0x%x, vaddr=0x%x, paddr=0x%x\n",
			str, program_header->header->p_offset,
			program_header->header->p_vaddr,
			program_header->header->p_paddr);
		x86_loader_debug("    filesz=%d, memsz=%d, flags=%d, align=%d\n",
			program_header->header->p_filesz,
			program_header->header->p_memsz,
			program_header->header->p_flags,
			program_header->header->p_align);

		/* Program interpreter */
		if (program_header->header->p_type == 3)
		{
			mem_read_string(mem, program_header->header->p_vaddr, sizeof(str), str);
			ld->interp = str_set(NULL, str);
		}
	}

	/* Free buffer and save pointers */
	ld->phdt_base = phdt_base;
	ld->phdr_count = phdr_count;
}


/* Load auxiliary vector, and return its size in bytes. */

#define X86_LOADER_AV_ENTRY(t, v) \
{ \
	uint32_t a_type = t; \
	uint32_t a_value = v; \
	mem_write(mem, sp, 4, &a_type); \
	mem_write(mem, sp + 4, 4, &a_value); \
	sp += 8; \
}

static uint32_t x86_loader_load_av(struct x86_ctx_t *ctx, uint32_t where)
{
	struct x86_loader_t *ld = ctx->loader;
	struct mem_t *mem = ctx->mem;
	uint32_t sp = where;

	x86_loader_debug("Loading auxiliary vector at 0x%x\n", where);

	/* Program headers */
	X86_LOADER_AV_ENTRY(3, ld->phdt_base);  /* AT_PHDR */
	X86_LOADER_AV_ENTRY(4, 32);  /* AT_PHENT -> program header size of 32 bytes */
	X86_LOADER_AV_ENTRY(5, ld->phdr_count);  /* AT_PHNUM */

	/* Other values */
	X86_LOADER_AV_ENTRY(6, MEM_PAGE_SIZE);  /* AT_PAGESZ */
	X86_LOADER_AV_ENTRY(7, 0);  /* AT_BASE */
	X86_LOADER_AV_ENTRY(8, 0);  /* AT_FLAGS */
	X86_LOADER_AV_ENTRY(9, ld->prog_entry);  /* AT_ENTRY */
	X86_LOADER_AV_ENTRY(11, getuid());  /* AT_UID */
	X86_LOADER_AV_ENTRY(12, geteuid());  /* AT_EUID */
	X86_LOADER_AV_ENTRY(13, getgid());  /* AT_GID */
	X86_LOADER_AV_ENTRY(14, getegid());  /* AT_EGID */
	X86_LOADER_AV_ENTRY(17, 0x64);  /* AT_CLKTCK */
	X86_LOADER_AV_ENTRY(23, 0);  /* AT_SECURE */

	/* Random bytes */
	ld->at_random_addr_holder = sp + 4;
	X86_LOADER_AV_ENTRY(25, 0);  /* AT_RANDOM */

	/*X86_LOADER_AV_ENTRY(32, 0xffffe400);
	X86_LOADER_AV_ENTRY(33, 0xffffe000);
	X86_LOADER_AV_ENTRY(16, 0xbfebfbff);*/

	/* ??? AT_HWCAP, AT_PLATFORM, 32 and 33 ???*/

	/* Finally, AT_NULL, and return size */
	X86_LOADER_AV_ENTRY(0, 0);
	return sp - where;
}
#undef X86_LOADER_AV_ENTRY


/* Load stack with the following layout.
 *
 * Address		Description			Size	Value
 * ------------------------------------------------------------------------------
 * (0xc0000000)		< bottom of stack >		0	(virtual)
 * (0xbffffffc)		[ end marker ]			4	(= NULL)
 *
 * 			[ environment ASCIIZ strings ]	>= 0
 * 			[ argument ASCIIZ strings ]	>= 0
 * 			[ padding ]			0 - 16
 *
 * 			[ auxv[term] (Elf32_auxv_t) ]	8	(= AT_NULL vector)
 * 			[ auxv[...] (Elf32_auxv_t) ]
 * 			[ auxv[1] (Elf32_auxv_t) ]	8
 * 			[ auxv[0] (Elf32_auxv_t) ]	8
 *
 * 			[ envp[term] (pointer) ]	4	(= NULL)
 * 			[ envp[...] (pointer) ]
 * 			[ envp[1] (pointer) ]		4
 * 			[ envp[0] (pointer) ]		4
 *
 * 			[ argv[argc] (pointer) ]	4	(= NULL)
 * 			[ argv[argc - 1] (pointer) ]	4
 * 			[ argv[...] (pointer) ]
 * 			[ argv[1] (pointer) ]		4
 * 			[ argv[0] (pointer) ]		4	(program name)
 * stack pointer ->	[ argc ]			4	(number of arguments)
 */

static void x86_loader_load_stack(struct x86_ctx_t *ctx)
{
	struct x86_loader_t *ld = ctx->loader;
	struct mem_t *mem = ctx->mem;
	uint32_t sp, argc, argvp, envp;
	uint32_t zero = 0;
	char *str;
	int i;

	/* Allocate stack */
	ld->stack_base = LD_STACK_BASE;
	ld->stack_size = LD_STACK_SIZE;
	ld->stack_top = LD_STACK_BASE - LD_STACK_SIZE;
	mem_map(mem, ld->stack_top, ld->stack_size, mem_access_read | mem_access_write);
	x86_loader_debug("mapping region for stack from 0x%x to 0x%x\n",
		ld->stack_top, ld->stack_base - 1);
	
	/* Load arguments and environment variables */
	ld->environ_base = LD_STACK_BASE - LD_MAX_ENVIRON;
	sp = ld->environ_base;
	argc = linked_list_count(ld->args);
	x86_loader_debug("  saved 'argc=%d' at 0x%x\n", argc, sp);
	mem_write(mem, sp, 4, &argc);
	sp += 4;
	argvp = sp;
	sp = sp + (argc + 1) * 4;

	/* Save space for environ and null */
	envp = sp;
	sp += linked_list_count(ld->env) * 4 + 4;

	/* Load here the auxiliary vector */
	sp += x86_loader_load_av(ctx, sp);

	/* Write arguments into stack */
	x86_loader_debug("\nArguments:\n");
	for (i = 0; i < argc; i++)
	{
		linked_list_goto(ld->args, i);
		str = linked_list_get(ld->args);
		mem_write(mem, argvp + i * 4, 4, &sp);
		mem_write_string(mem, sp, str);
		x86_loader_debug("  argument %d at 0x%x: '%s'\n", i, sp, str);
		sp += strlen(str) + 1;
	}
	mem_write(mem, argvp + i * 4, 4, &zero);

	/* Write environment variables */
	x86_loader_debug("\nEnvironment variables:\n");
	for (i = 0; i < linked_list_count(ld->env); i++)
	{
		linked_list_goto(ld->env, i);
		str = linked_list_get(ld->env);
		mem_write(mem, envp + i * 4, 4, &sp);
		mem_write_string(mem, sp, str);
		x86_loader_debug("  env var %d at 0x%x: '%s'\n", i, sp, str);
		sp += strlen(str) + 1;
	}
	mem_write(mem, envp + i * 4, 4, &zero);

	/* Random bytes */
	ld->at_random_addr = sp;
	for (i = 0; i < 16; i++)
	{
		unsigned char c = random();
		mem_write(mem, sp, 1, &c);
		sp++;
	}
	mem_write(mem, ld->at_random_addr_holder, 4, &ld->at_random_addr);

	/* Check that we didn't overflow */
	if (sp > LD_STACK_BASE)
		fatal("%s: initial stack overflow, increment LD_MAX_ENVIRON",
			__FUNCTION__);
}


void x86_loader_load_exe(struct x86_ctx_t *ctx, char *exe)
{
	struct x86_loader_t *ld = ctx->loader;
	struct mem_t *mem = ctx->mem;
	struct x86_file_desc_table_t *fdt = ctx->file_desc_table;

	char stdin_file_full_path[MAX_STRING_SIZE];
	char stdout_file_full_path[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];

	/* Alternative stdin */
	x86_loader_get_full_path(ctx, ld->stdin_file, stdin_file_full_path, MAX_STRING_SIZE);
	if (*stdin_file_full_path)
	{
		struct x86_file_desc_t *fd;
		fd = x86_file_desc_table_entry_get(fdt, 0);
		assert(fd);
		fd->host_fd = open(stdin_file_full_path, O_RDONLY);
		if (fd->host_fd < 0)
			fatal("%s: cannot open stdin", ld->stdin_file);
		x86_loader_debug("%s: stdin redirected\n", stdin_file_full_path);
	}

	/* Alternative stdout/stderr */
	x86_loader_get_full_path(ctx, ld->stdout_file, stdout_file_full_path, MAX_STRING_SIZE);
	if (*stdout_file_full_path)
	{
		struct x86_file_desc_t *fd1, *fd2;
		fd1 = x86_file_desc_table_entry_get(fdt, 1);
		fd2 = x86_file_desc_table_entry_get(fdt, 2);
		assert(fd1 && fd2);
		fd1->host_fd = fd2->host_fd = open(stdout_file_full_path,
			O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, 0660);
		if (fd1->host_fd < 0)
			fatal("%s: cannot open stdout/stderr", ld->stdout_file);
		x86_loader_debug("%s: stdout redirected\n", stdout_file_full_path);
	}
	
	
	/* Load program into memory */
	x86_loader_get_full_path(ctx, exe, exe_full_path, MAX_STRING_SIZE);
	ld->elf_file = elf_file_create_from_path(exe_full_path);
	ld->exe = str_set(NULL, exe_full_path);

	/* Read sections and program entry */
	x86_loader_load_sections(ctx, ld->elf_file);
	ld->prog_entry = ld->elf_file->header->e_entry;

	/* Set heap break to the highest written address rounded up to
	 * the memory page boundary. */
	mem->heap_break = ROUND_UP(mem->heap_break, MEM_PAGE_SIZE);

	/* Load program header table. If we found a PT_INTERP program header,
	 * we have to load the program interpreter. This means we are dealing with
	 * a dynamically linked application. */
	x86_loader_load_program_headers(ctx);
	if (ld->interp)
		x86_loader_load_interp(ctx);

	/* Stack */
	x86_loader_load_stack(ctx);

	/* Register initialization */
	ctx->regs->eip = ld->interp ? ld->interp_prog_entry : ld->prog_entry;
	ctx->regs->esp = ld->environ_base;

	x86_loader_debug("Program entry is 0x%x\n", ctx->regs->eip);
	x86_loader_debug("Initial stack pointer is 0x%x\n", ctx->regs->esp);
	x86_loader_debug("Heap start set to 0x%x\n", mem->heap_break);
}




/*
 * Public Functions
 */


struct x86_loader_t *x86_loader_create(void)
{
	struct x86_loader_t *ld;

	/* Initialize */
	ld = xcalloc(1, sizeof(struct x86_loader_t));
	ld->args = linked_list_create();
	ld->env = linked_list_create();

	/* Return */
	return ld;
}


void x86_loader_free(struct x86_loader_t *ld)
{
	/* Check no more links */
	assert(!ld->num_links);

	/* Free ELF file  */
	if (ld->elf_file)
		elf_file_free(ld->elf_file);

	/* Free arguments */
	LINKED_LIST_FOR_EACH(ld->args)
		str_free(linked_list_get(ld->args));
	linked_list_free(ld->args);

	/* Free environment variables */
	LINKED_LIST_FOR_EACH(ld->env)
		str_free(linked_list_get(ld->env));
	linked_list_free(ld->env);

	/* Free loader */
	str_free(ld->interp);
	str_free(ld->exe);
	str_free(ld->cwd);
	str_free(ld->stdin_file);
	str_free(ld->stdout_file);
	free(ld);
}


struct x86_loader_t *x86_loader_link(struct x86_loader_t *ld)
{
	ld->num_links++;
	return ld;
}


void x86_loader_unlink(struct x86_loader_t *ld)
{
	assert(ld->num_links >= 0);
	if (ld->num_links)
		ld->num_links--;
	else
		x86_loader_free(ld);
}


void x86_loader_get_full_path(struct x86_ctx_t *ctx, char *file_name, char *full_path, int size)
{
	struct x86_loader_t *ld = ctx->loader;

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
	if (strlen(ld->cwd) + strlen(file_name) + 2 > size)
		fatal("%s: buffer too small", __FUNCTION__);
	snprintf(full_path, size, "%s/%s", ld->cwd, file_name);
}


void x86_loader_load_from_ctx_config(struct config_t *config, char *section)
{
	struct x86_ctx_t *ctx;
	struct x86_loader_t *ld;

	char buf[MAX_STRING_SIZE];

	char *exe;
	char *cwd;
	char *args;
	char *env;

	char *in;
	char *out;

	char *config_file_name;

	/* Get configuration file name for errors */
	config_file_name = config_get_file_name(config);

	/* Create new context */
	ctx = x86_ctx_create();
	ld = ctx->loader;
		
	/* Executable */
	exe = config_read_string(config, section, "Exe", "");
	exe = str_set(NULL, exe);
	if (!*exe)
		fatal("%s: [%s]: invalid executable", config_file_name,
			section);

	/* Arguments */
	args = config_read_string(config, section, "Args", "");
	linked_list_add(ld->args, exe);
	x86_loader_add_args_string(ctx, args);

	/* Environment variables */
	env = config_read_string(config, section, "Env", "");
	x86_loader_add_environ(ctx, env);

	/* Current working directory */
	cwd = config_read_string(config, section, "Cwd", "");
	if (*cwd)
		ld->cwd = str_set(NULL, cwd);
	else
	{
		/* Get current directory */
		ld->cwd = getcwd(buf, sizeof buf);
		if (!ld->cwd)
			panic("%s: buffer too small", __FUNCTION__);

		/* Duplicate string */
		ld->cwd = str_set(NULL, ld->cwd);
	}

	/* Standard input */
	in = config_read_string(config, section, "Stdin", "");
	ld->stdin_file = str_set(NULL, in);

	/* Standard output */
	out = config_read_string(config, section, "Stdout", "");
	ld->stdout_file = str_set(NULL, out);

	/* Load executable */
	x86_loader_load_exe(ctx, exe);
}


void x86_loader_load_from_command_line(int argc, char **argv)
{
	struct x86_ctx_t *ctx;
	struct x86_loader_t *ld;
	
	char buf[MAX_STRING_SIZE];

	/* Create context */
	ctx = x86_ctx_create();
	ld = ctx->loader;

	/* Arguments and environment */
	x86_loader_add_args_vector(ctx, argc, argv);
	x86_loader_add_environ(ctx, "");

	/* Get current directory */
	ld->cwd = getcwd(buf, sizeof buf);
	if (!ld->cwd)
		panic("%s: buffer too small", __FUNCTION__);
	ld->cwd = str_set(NULL, ld->cwd);

	/* Redirections */
	ld->stdin_file = str_set(NULL, "");
	ld->stdout_file = str_set(NULL, "");

	/* Load executable */
	x86_loader_load_exe(ctx, argv[0]);
}

