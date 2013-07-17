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
#include <mem-system/memory.h>

#include "context.h"
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



/*
 * Class 'X86Context'
 * Additional functions
 */

static struct str_map_t elf_section_flags_map =
{
	3, {
		{ "SHF_WRITE", 1 },
		{ "SHF_ALLOC", 2 },
		{ "SHF_EXECINSTR", 4 }
	}
};


void X86ContextAddArgsVector(X86Context *self, int argc, char **argv)
{
	struct x86_loader_t *loader = self->loader;

	char *arg;
	int i;

	for (i = 0; i < argc; i++)
	{
		arg = str_set(NULL, argv[i]);
		linked_list_add(loader->args, arg);
	}
}


void X86ContextAddArgsString(X86Context *self, char *args)
{
	struct x86_loader_t *loader = self->loader;

	char *delim = " ";
	char *arg;
	
	/* Duplicate argument string */
	args = str_set(NULL, args);

	/* Tokens */
	for (arg = strtok(args, delim); arg; arg = strtok(NULL, delim))
	{
		arg = str_set(NULL, arg);
		linked_list_add(loader->args, arg);
	}

	/* Free argument string */
	str_free(args);
}


/* Add environment variables from the actual environment plus
 * the list attached in the argument 'env'. */
void X86ContextAddEnv(X86Context *self, char *env)
{
	struct x86_loader_t *loader = self->loader;
	extern char **environ;

	char *next;
	char *str;

	int i;

	/* Add variables from actual environment */
	for (i = 0; environ[i]; i++)
	{
		str = str_set(NULL, environ[i]);
		linked_list_add(loader->env, str);
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
			linked_list_add(loader->env, str);
			env = next + 1;
			break;

		default:
			str = str_set(NULL, env);
			linked_list_add(loader->env, str);
			env = NULL;
		}
	}
}


#define X86_LOADER_STACK_BASE  0xc0000000
#define X86_LOADER_MAX_ENVIRON  0x10000  /* 16KB for environment */
#define X86_LOADER_STACK_SIZE  0x800000  /* 8MB stack size */


/* Load sections from an ELF file */
void X86ContextLoadELFSections(X86Context *self, struct elf_file_t *elf_file)
{
	struct mem_t *mem = self->mem;
	struct x86_loader_t *loader = self->loader;

	struct elf_section_t *section;
	int i;

	enum mem_access_t perm;
	char flags_str[200];

	x86_loader_debug("\nLoading ELF sections\n");
	loader->bottom = 0xffffffff;
	for (i = 0; i < list_count(elf_file->section_list); i++)
	{
		section = list_get(elf_file->section_list, i);

		perm = mem_access_init | mem_access_read;
		str_map_flags(&elf_section_flags_map, section->header->sh_flags,
				flags_str, sizeof(flags_str));
		x86_loader_debug("  section %d: name='%s', offset=0x%x, "
				"addr=0x%x, size=%u, flags=%s\n",
				i, section->name, section->header->sh_offset,
				section->header->sh_addr, section->header->sh_size, flags_str);

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
			loader->bottom = MIN(loader->bottom, section->header->sh_addr);

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


void X86ContextLoadInterp(X86Context *self)
{
	struct x86_loader_t *loader = self->loader;
	struct elf_file_t *elf_file;

	/* Open dynamic loader */
	x86_loader_debug("\nLoading program interpreter '%s'\n", loader->interp);
	elf_file = elf_file_create_from_path(loader->interp);
	
	/* Load section from program interpreter */
	X86ContextLoadELFSections(self, elf_file);

	/* Change program entry to the one specified by the interpreter */
	loader->interp_prog_entry = elf_file->header->e_entry;
	x86_loader_debug("  program interpreter entry: 0x%x\n\n", loader->interp_prog_entry);
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
void X86ContextLoadProgramHeaders(X86Context *self)
{
	struct x86_loader_t *loader = self->loader;
	struct mem_t *mem = self->mem;

	struct elf_file_t *elf_file = loader->elf_file;
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
	 * If none found, choose loader->bottom - phdt_size. */
	phdt_base = loader->bottom - phdt_size;
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
			loader->interp = str_set(NULL, str);
		}
	}

	/* Free buffer and save pointers */
	loader->phdt_base = phdt_base;
	loader->phdr_count = phdr_count;
}


/* Load auxiliary vector, and return its size in bytes. */

#define X86_LOADER_AV_ENTRY(t, v) \
{ \
	unsigned int a_type = t; \
	unsigned int a_value = v; \
	mem_write(mem, sp, 4, &a_type); \
	mem_write(mem, sp + 4, 4, &a_value); \
	sp += 8; \
}

unsigned int X86ContextLoadAV(X86Context *self, unsigned int where)
{
	struct x86_loader_t *loader = self->loader;
	struct mem_t *mem = self->mem;
	unsigned int sp = where;

	x86_loader_debug("Loading auxiliary vector at 0x%x\n", where);

	/* Program headers */
	X86_LOADER_AV_ENTRY(3, loader->phdt_base);  /* AT_PHDR */
	X86_LOADER_AV_ENTRY(4, 32);  /* AT_PHENT -> program header size of 32 bytes */
	X86_LOADER_AV_ENTRY(5, loader->phdr_count);  /* AT_PHNUM */

	/* Other values */
	X86_LOADER_AV_ENTRY(6, MEM_PAGE_SIZE);  /* AT_PAGESZ */
	X86_LOADER_AV_ENTRY(7, 0);  /* AT_BASE */
	X86_LOADER_AV_ENTRY(8, 0);  /* AT_FLAGS */
	X86_LOADER_AV_ENTRY(9, loader->prog_entry);  /* AT_ENTRY */
	X86_LOADER_AV_ENTRY(11, getuid());  /* AT_UID */
	X86_LOADER_AV_ENTRY(12, geteuid());  /* AT_EUID */
	X86_LOADER_AV_ENTRY(13, getgid());  /* AT_GID */
	X86_LOADER_AV_ENTRY(14, getegid());  /* AT_EGID */
	X86_LOADER_AV_ENTRY(17, 0x64);  /* AT_CLKTCK */
	X86_LOADER_AV_ENTRY(23, 0);  /* AT_SECURE */

	/* Random bytes */
	loader->at_random_addr_holder = sp + 4;
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

void X86ContextLoadStack(X86Context *self)
{
	struct x86_loader_t *loader = self->loader;
	struct mem_t *mem = self->mem;
	unsigned int sp, argc, argvp, envp;
	unsigned int zero = 0;
	char *str;
	int i;

	/* Allocate stack */
	loader->stack_base = X86_LOADER_STACK_BASE;
	loader->stack_size = X86_LOADER_STACK_SIZE;
	loader->stack_top = X86_LOADER_STACK_BASE - X86_LOADER_STACK_SIZE;
	mem_map(mem, loader->stack_top, loader->stack_size, mem_access_read | mem_access_write);
	x86_loader_debug("mapping region for stack from 0x%x to 0x%x\n",
		loader->stack_top, loader->stack_base - 1);
	
	/* Load arguments and environment variables */
	loader->environ_base = X86_LOADER_STACK_BASE - X86_LOADER_MAX_ENVIRON;
	sp = loader->environ_base;
	argc = linked_list_count(loader->args);
	x86_loader_debug("  saved 'argc=%d' at 0x%x\n", argc, sp);
	mem_write(mem, sp, 4, &argc);
	sp += 4;
	argvp = sp;
	sp = sp + (argc + 1) * 4;

	/* Save space for environ and null */
	envp = sp;
	sp += linked_list_count(loader->env) * 4 + 4;

	/* Load here the auxiliary vector */
	sp += X86ContextLoadAV(self, sp);

	/* Write arguments into stack */
	x86_loader_debug("\nArguments:\n");
	for (i = 0; i < argc; i++)
	{
		linked_list_goto(loader->args, i);
		str = linked_list_get(loader->args);
		mem_write(mem, argvp + i * 4, 4, &sp);
		mem_write_string(mem, sp, str);
		x86_loader_debug("  argument %d at 0x%x: '%s'\n", i, sp, str);
		sp += strlen(str) + 1;
	}
	mem_write(mem, argvp + i * 4, 4, &zero);

	/* Write environment variables */
	x86_loader_debug("\nEnvironment variables:\n");
	for (i = 0; i < linked_list_count(loader->env); i++)
	{
		linked_list_goto(loader->env, i);
		str = linked_list_get(loader->env);
		mem_write(mem, envp + i * 4, 4, &sp);
		mem_write_string(mem, sp, str);
		x86_loader_debug("  env var %d at 0x%x: '%s'\n", i, sp, str);
		sp += strlen(str) + 1;
	}
	mem_write(mem, envp + i * 4, 4, &zero);

	/* Random bytes */
	loader->at_random_addr = sp;
	for (i = 0; i < 16; i++)
	{
		unsigned char c = random();
		mem_write(mem, sp, 1, &c);
		sp++;
	}
	mem_write(mem, loader->at_random_addr_holder, 4, &loader->at_random_addr);

	/* Check that we didn't overflow */
	if (sp > X86_LOADER_STACK_BASE)
		fatal("%s: initial stack overflow, increment LD_MAX_ENVIRON",
			__FUNCTION__);
}


void X86ContextLoadExe(X86Context *self, char *exe)
{
	struct x86_loader_t *loader = self->loader;
	struct mem_t *mem = self->mem;
	struct x86_file_desc_table_t *fdt = self->file_desc_table;

	char stdin_file_full_path[MAX_STRING_SIZE];
	char stdout_file_full_path[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];

	/* Alternative stdin */
	X86ContextGetFullPath(self, loader->stdin_file, stdin_file_full_path, MAX_STRING_SIZE);
	if (*stdin_file_full_path)
	{
		struct x86_file_desc_t *fd;
		fd = x86_file_desc_table_entry_get(fdt, 0);
		assert(fd);
		fd->host_fd = open(stdin_file_full_path, O_RDONLY);
		if (fd->host_fd < 0)
			fatal("%s: cannot open stdin", loader->stdin_file);
		x86_loader_debug("%s: stdin redirected\n", stdin_file_full_path);
	}

	/* Alternative stdout/stderr */
	X86ContextGetFullPath(self, loader->stdout_file, stdout_file_full_path, MAX_STRING_SIZE);
	if (*stdout_file_full_path)
	{
		struct x86_file_desc_t *fd1, *fd2;
		fd1 = x86_file_desc_table_entry_get(fdt, 1);
		fd2 = x86_file_desc_table_entry_get(fdt, 2);
		assert(fd1 && fd2);
		fd1->host_fd = fd2->host_fd = open(stdout_file_full_path,
			O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, 0660);
		if (fd1->host_fd < 0)
			fatal("%s: cannot open stdout/stderr", loader->stdout_file);
		x86_loader_debug("%s: stdout redirected\n", stdout_file_full_path);
	}
	
	
	/* Load program into memory */
	X86ContextGetFullPath(self, exe, exe_full_path, MAX_STRING_SIZE);
	loader->elf_file = elf_file_create_from_path(exe_full_path);
	loader->exe = str_set(NULL, exe_full_path);

	/* Read sections and program entry */
	X86ContextLoadELFSections(self, loader->elf_file);
	loader->prog_entry = loader->elf_file->header->e_entry;

	/* Set heap break to the highest written address rounded up to
	 * the memory page boundary. */
	mem->heap_break = ROUND_UP(mem->heap_break, MEM_PAGE_SIZE);

	/* Load program header table. If we found a PT_INTERP program header,
	 * we have to load the program interpreter. This means we are dealing with
	 * a dynamically linked application. */
	X86ContextLoadProgramHeaders(self);
	if (loader->interp)
		X86ContextLoadInterp(self);

	/* Stack */
	X86ContextLoadStack(self);

	/* Register initialization */
	self->regs->eip = loader->interp ? loader->interp_prog_entry : loader->prog_entry;
	self->regs->esp = loader->environ_base;

	x86_loader_debug("Program entry is 0x%x\n", self->regs->eip);
	x86_loader_debug("Initial stack pointer is 0x%x\n", self->regs->esp);
	x86_loader_debug("Heap start set to 0x%x\n", mem->heap_break);
}


void X86ContextGetFullPath(X86Context *ctx, char *file_name, char *full_path, int size)
{
	struct x86_loader_t *loader = ctx->loader;

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
	if (strlen(loader->cwd) + strlen(file_name) + 2 > size)
		fatal("%s: buffer too small", __FUNCTION__);
	snprintf(full_path, size, "%s/%s", loader->cwd, file_name);
}




/*
 * Object 'x86_loader_t'
 */


struct x86_loader_t *x86_loader_create(void)
{
	struct x86_loader_t *loader;

	/* Initialize */
	loader = xcalloc(1, sizeof(struct x86_loader_t));
	loader->args = linked_list_create();
	loader->env = linked_list_create();

	/* Return */
	return loader;
}


void x86_loader_free(struct x86_loader_t *loader)
{
	/* Check no more links */
	assert(!loader->num_links);

	/* Free ELF file  */
	if (loader->elf_file)
		elf_file_free(loader->elf_file);

	/* Free arguments */
	LINKED_LIST_FOR_EACH(loader->args)
		str_free(linked_list_get(loader->args));
	linked_list_free(loader->args);

	/* Free environment variables */
	LINKED_LIST_FOR_EACH(loader->env)
		str_free(linked_list_get(loader->env));
	linked_list_free(loader->env);

	/* Free loader */
	str_free(loader->interp);
	str_free(loader->exe);
	str_free(loader->cwd);
	str_free(loader->stdin_file);
	str_free(loader->stdout_file);
	free(loader);
}


struct x86_loader_t *x86_loader_link(struct x86_loader_t *loader)
{
	loader->num_links++;
	return loader;
}


void x86_loader_unlink(struct x86_loader_t *loader)
{
	assert(loader->num_links >= 0);
	if (loader->num_links)
		loader->num_links--;
	else
		x86_loader_free(loader);
}
