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
#include <arm-emu.h>

int arm_loader_debug_category;

/* Stack parameter definitions */
#define LD_STACK_BASE  0xc0000000
#define LD_MAX_ENVIRON  0x10000  /* 16KB for environment */
#define LD_STACK_SIZE  0x800000  /* 8MB stack size */

static struct string_map_t elf_section_flags_map =
{
	3, {
		{ "SHF_WRITE", 1 },
		{ "SHF_ALLOC", 2 },
		{ "SHF_EXECINSTR", 4 }
	}
};

static struct string_map_t elf_program_header_type_map = {
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

static struct arm_ctx_t *arm_ctx_do_create()
{
	struct arm_ctx_t *ctx;

	/* Create context and set its value */
	ctx = calloc(1, sizeof(struct arm_ctx_t));
	if(!ctx)
		fatal("%s: out of memory", __FUNCTION__);

	/* Structures */
	ctx->regs = arm_regs_create();

	/* Return context */
	return ctx;
}

static void arm_ctx_loader_add_args_vector(struct arm_ctx_t *ctx, int argc, char **argv)
{
	char *arg;
	int i;

	for (i = 0; i < argc; i++)
	{
		/* Allocate */
		arg = strdup(argv[i]);
		if (!arg)
			fatal("%s: out of memory", __FUNCTION__);

		/* Add */
		linked_list_add(ctx->args, arg);
	}
}

/* Add environment variables from the actual environment plus
 * the list attached in the argument 'env'. */
static void arm_ctx_loader_add_environ(struct arm_ctx_t *ctx, char *env)
{
	extern char **environ;
	char *next;
	int i;

	/* Add variables from actual environment */
	for (i = 0; environ[i]; i++)
		linked_list_add(ctx->env, strdup(environ[i]));

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
			linked_list_add(ctx->env, strdup(env + 1));
			env = next + 1;
			break;

		default:
			linked_list_add(ctx->env, strdup(env));
			env = NULL;
		}
	}
}

/* Load program headers table */
static void arm_ctx_loader_load_program_headers(struct arm_ctx_t *ctx)
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
	arm_loader_debug("\nLoading program headers\n");
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
	arm_loader_debug("  virtual address for program header table: 0x%x\n", phdt_base);

	/* Load program headers */
	mem_map(mem, phdt_base, phdt_size, mem_access_init | mem_access_read);
	for (i = 0; i < list_count(elf_file->program_header_list); i++)
	{
		/* Load program header */
		program_header = list_get(elf_file->program_header_list, i);
		mem_access(mem, phdt_base + i * phdr_size, phdr_size,
			program_header->header, mem_access_init);

		/* Debug */
		map_value_string(&elf_program_header_type_map, program_header->header->p_type,
			str, sizeof(str));
		arm_loader_debug("  header loaded at 0x%x\n", phdt_base + i * phdr_size);
		arm_loader_debug("    type=%s, offset=0x%x, vaddr=0x%x, paddr=0x%x\n",
			str, program_header->header->p_offset,
			program_header->header->p_vaddr,
			program_header->header->p_paddr);
		arm_loader_debug("    filesz=%d, memsz=%d, flags=%d, align=%d\n",
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
static void arm_ctx_loader_load_sections(struct arm_ctx_t *ctx, struct elf_file_t *elf_file)
{
	struct mem_t *mem = ctx->mem;

	struct elf_section_t *section;
	int i;

	enum mem_access_t perm;
	char flags_str[200];

	arm_loader_debug("\nLoading ELF sections\n");
	ctx->bottom = 0xffffffff;
	for (i = 0; i < list_count(elf_file->section_list); i++)
	{
		section = list_get(elf_file->section_list, i);

		perm = mem_access_init | mem_access_read;
		map_flags(&elf_section_flags_map, section->header->sh_flags,
			flags_str, sizeof(flags_str));
		arm_loader_debug("  section %d: name='%s', offset=0x%x, addr=0x%x,"
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

				ptr = calloc(1, section->header->sh_size);
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

static void arm_ctx_loader_load_stack(struct arm_ctx_t *ctx)
{
	/* FIXME: Implement the stack loading function for ARM and the stack
	 * layout for the same. Current implementation matches that of i386.
	 * Investigation n ARM stack is in progress
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
		arm_loader_debug("mapping region for stack from 0x%x to 0x%x\n",
			ctx->stack_top, ctx->stack_base - 1);

		/* Load arguments and environment variables */
		ctx->environ_base = LD_STACK_BASE - LD_MAX_ENVIRON;
		sp = ctx->environ_base;
		argc = linked_list_count(ctx->args);
		arm_loader_debug("  saved 'argc=%d' at 0x%x\n", argc, sp);
		mem_write(mem, sp, 4, &argc);
		sp += 4;
		argvp = sp;
		sp = sp + (argc + 1) * 4;

		/* Save space for environ and null */
		envp = sp;
		sp += linked_list_count(ctx->env) * 4 + 4;

		/* FIXME: Load here the auxiliary vector ARM */


		/* Write arguments into stack */
		arm_loader_debug("\nArguments:\n");
		for (i = 0; i < argc; i++)
		{
			linked_list_goto(ctx->args, i);
			str = linked_list_get(ctx->args);
			mem_write(mem, argvp + i * 4, 4, &sp);
			mem_write_string(mem, sp, str);
			arm_loader_debug("  argument %d at 0x%x: '%s'\n", i, sp, str);
			sp += strlen(str) + 1;
		}
		mem_write(mem, argvp + i * 4, 4, &zero);

		/* Write environment variables */
		arm_loader_debug("\nEnvironment variables:\n");
		for (i = 0; i < linked_list_count(ctx->env); i++)
		{
			linked_list_goto(ctx->env, i);
			str = linked_list_get(ctx->env);
			mem_write(mem, envp + i * 4, 4, &sp);
			mem_write_string(mem, sp, str);
			arm_loader_debug("  env var %d at 0x%x: '%s'\n", i, sp, str);
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
		mem_write(mem, ctx->at_random_addr_holder, 4, &ctx->at_random_addr);

		/* Check that we didn't overflow */
		if (sp > LD_STACK_BASE)
			fatal("%s: initial stack overflow, increment LD_MAX_ENVIRON",
				__FUNCTION__);
}

void arm_ctx_loader_load_exe(struct arm_ctx_t *ctx, char *exe)
{

	struct mem_t *mem = ctx->mem;
	struct arm_file_desc_table_t *fdt = ctx->file_desc_table;

	char stdin_file_full_path[MAX_STRING_SIZE];
	char stdout_file_full_path[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];

	/* Alternative stdin */
	arm_ctx_loader_get_full_path(ctx, ctx->stdin_file, stdin_file_full_path, MAX_STRING_SIZE);
	if (*stdin_file_full_path)
	{
		struct arm_file_desc_t *fd;
		fd = arm_file_desc_table_entry_get(fdt, 0);
		assert(fd);
		fd->host_fd = open(stdin_file_full_path, O_RDONLY);
		if (fd->host_fd < 0)
			fatal("%s: cannot open stdin", ctx->stdin_file);
		arm_loader_debug("%s: stdin redirected\n", stdin_file_full_path);
	}

	/* Alternative stdout/stderr */
	arm_ctx_loader_get_full_path(ctx, ctx->stdout_file, stdout_file_full_path, MAX_STRING_SIZE);
	if (*stdout_file_full_path)
	{
		struct arm_file_desc_t *fd1, *fd2;
		fd1 = arm_file_desc_table_entry_get(fdt, 1);
		fd2 = arm_file_desc_table_entry_get(fdt, 2);
		assert(fd1 && fd2);
		fd1->host_fd = fd2->host_fd = open(stdout_file_full_path,
			O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, 0660);
		if (fd1->host_fd < 0)
			fatal("%s: cannot open stdout/stderr", ctx->stdout_file);
		arm_loader_debug("%s: stdout redirected\n", stdout_file_full_path);
	}


	/* Load program into memory */
	arm_ctx_loader_get_full_path(ctx, exe, exe_full_path, MAX_STRING_SIZE);
	ctx->elf_file = elf_file_create_from_path(exe_full_path);
	ctx->exe = strdup(exe_full_path);
	if (!ctx->exe)
		fatal("%s: out of memory", __FUNCTION__);

	/* Read sections and program entry */
	arm_ctx_loader_load_sections(ctx, ctx->elf_file);
	ctx->prog_entry = ctx->elf_file->header->e_entry;

	/* Set heap break to the highest written address rounded up to
	 * the memory page boundary. */
	mem->heap_break = ROUND_UP(mem->heap_break, MEM_PAGE_SIZE);

	/* Load program header table. If we found a PT_INTERP program header,
	 * we have to load the program interpreter. This means we are dealing with
	 * a dynamically linked application. */
	arm_ctx_loader_load_program_headers(ctx);

	/* Stack */
	arm_ctx_loader_load_stack(ctx);

	/* Register initialization */
	ctx->regs->ip = ctx->prog_entry;
	ctx->regs->sp = ctx->environ_base;

	arm_loader_debug("Program entry is 0x%x\n", ctx->regs->ip);
	arm_loader_debug("Initial stack pointer is 0x%x\n", ctx->regs->sp);
	arm_loader_debug("Heap start set to 0x%x\n", mem->heap_break);
}

struct arm_ctx_t *arm_ctx_create(void)
{
	struct arm_ctx_t *ctx;

	ctx = arm_ctx_do_create();

	/* Memory */
	ctx->mem = mem_create();

	/* Initialize Loader Sections*/
	ctx->args = linked_list_create();
	ctx->env = linked_list_create();


	return ctx;
}

void arm_ctx_free(struct arm_ctx_t *ctx)
{

	/* Free private structures */
	arm_regs_free(ctx->regs);

	/* Check no more links */
	assert(!ctx->num_links);

	/* Free ELF file  */
	if (ctx->elf_file)
		elf_file_free(ctx->elf_file);

	/* Free arguments */
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

	free(ctx);
}

void arm_ctx_loader_load_prog_from_cmdline(int argc, char **argv)
{
	struct arm_ctx_t *ctx;

	char buf[MAX_STRING_SIZE];

	/* Create context */
	ctx = arm_ctx_create();

	/* Arguments and environment */
	arm_ctx_loader_add_args_vector(ctx, argc, argv);
	arm_ctx_loader_add_environ(ctx, "");


	/* Get current directory */
	ctx->cwd = getcwd(buf, sizeof buf);
	if (!ctx->cwd)
		panic("%s: buffer too small", __FUNCTION__);

	/* Duplicate */
	ctx->cwd = strdup(ctx->cwd);
	if (!ctx->cwd)
		fatal("%s: out of memory", __FUNCTION__);

	/* Redirections */
	ctx->stdin_file = strdup("");
	ctx->stdout_file = strdup("");
	if (!ctx->stdin_file || !ctx->stdout_file)
		fatal("%s: out of memory", __FUNCTION__);

	/* Load executable */
	arm_ctx_loader_load_exe(ctx, argv[0]);
}

void arm_ctx_loader_get_full_path(struct arm_ctx_t *ctx, char *file_name, char *full_path, int size)
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


void arm_ctx_load_from_command_line(int argc, char **argv)
{
}


void arm_ctx_load_from_ctx_file(struct config_t *config, char *section)
{
}
