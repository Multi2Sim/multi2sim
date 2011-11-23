/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <cpukernel.h>

int ld_debug_category;

char *ld_help_ctxconfig =
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
	"\n"
	"See the Multi2Sim Guide (www.multi2sim.org) for further details and\n"
	"examples on how to use the context configuration file.\n"
	"\n";


static struct string_map_t elf_section_flags_map = {
	3, {
		{ "SHF_WRITE", 1 },
		{ "SHF_ALLOC", 2 },
		{ "SHF_EXECINSTR", 4 }
	}
};


void ld_init(struct ctx_t *ctx)
{
	ctx->loader = calloc(1, sizeof(struct loader_t));
	ctx->loader->args = lnlist_create();
	ctx->loader->env = lnlist_create();
}


void ld_done(struct ctx_t *ctx)
{
	struct loader_t *ld = ctx->loader;

	/* Free ELF file  */
	elf_file_free(ld->elf_file);
	
	/* Free arguments and environment variables */
	for (lnlist_head(ld->args); !lnlist_eol(ld->args); lnlist_next(ld->args))
		free(lnlist_get(ld->args));
	for (lnlist_head(ld->env); !lnlist_eol(ld->env); lnlist_next(ld->env))
		free(lnlist_get(ld->env));
	lnlist_free(ld->args);
	lnlist_free(ld->env);

	/* Free loader */
	if (ld->interp)
		free(ld->interp);
	free(ld->exe);
	free(ld->cwd);
	free(ld->stdin_file);
	free(ld->stdout_file);
	free(ld);
}


void ld_get_full_path(struct ctx_t *ctx, char *filename, char *fullpath, int size)
{
	if (*filename == '/' || !*filename) {
		strcpy(fullpath, filename);
		return;
	}
	if (strlen(ctx->loader->cwd) + strlen(filename) + 2 > size)
		fatal("ld_get_full_path: buffer too small");
	strcpy(fullpath, ctx->loader->cwd);
	strcat(fullpath, "/");
	strcat(fullpath, filename);
}


void ld_add_args_vector(struct ctx_t *ctx, int argc, char **argv)
{
	struct loader_t *ld = ctx->loader;
	int i;
	for (i = 0; i < argc; i++)
		lnlist_add(ld->args, strdup(argv[i]));
}


void ld_add_args_string(struct ctx_t *ctx, char *args)
{
	struct loader_t *ld = ctx->loader;
	char *arg;
	int wordlen = 0;
	
	while (*args) {
		if (args[wordlen] != 32 && args[wordlen]) {
			wordlen++;
			continue;
		}
		if (!wordlen) {
			args++;
			continue;
		}
		
		/* Retrieve new argument in 'arg' */
		arg = calloc(1, wordlen + 1);
		memcpy(arg, args, wordlen);
		lnlist_add(ld->args, arg);
		args += wordlen;
		wordlen = 0;
	}
}


/* Add environment variables from the actual environment plus
 * the list attached in the argument 'env'. */
void ld_add_environ(struct ctx_t *ctx, char *env)
{
	struct loader_t *ld = ctx->loader;
	extern char **environ;
	char *next;
	int i;

	/* Add variables from actual environment */
	for (i = 0; environ[i]; i++)
		lnlist_add(ld->env, strdup(environ[i]));
	
	/* Add the environment vars provided in 'env' */
	while (env) {
		
		/* Skip spaces */
		while (*env == ' ') env++;
		if (!*env) break;

		/* Get new environment variable */
		switch (*env) {

		case '"':
		case '\'':
			if (!(next = strchr(env + 1, *env)))
				fatal("ld_add_environ: wrong format");
			*next = 0;
			lnlist_add(ld->env, strdup(env + 1));
			env = next + 1;
			break;

		default:
			lnlist_add(ld->env, strdup(env));
			env = NULL;
		}
	}
}


#define LD_STACK_BASE  0xffff0000
#define LD_MAX_ENVIRON  0x10000  /* 16KB for environment */
#define LD_STACK_SIZE  0x800000  /* 8MB stack size */


/* Load sections from an ELF file */
static void ld_load_sections(struct ctx_t *ctx, struct elf_file_t *elf_file)
{
	struct mem_t *mem = ctx->mem;
	struct loader_t *ld = ctx->loader;

	struct elf_section_t *section;
	int i;

	enum mem_access_t perm;
	char flags_str[200];

	ld_debug("\nLoading ELF sections\n");
	ld->bottom = 0xffffffff;
	for (i = 0; i < list_count(elf_file->section_list); i++)
	{
		section = list_get(elf_file->section_list, i);

		perm = mem_access_init | mem_access_read;
		map_flags(&elf_section_flags_map, section->header->sh_flags, flags_str, sizeof(flags_str));
		ld_debug("  section %d: name='%s', offset=0x%x, addr=0x%x, size=%u, flags=%s\n",
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
			ld->brk = MAX(ld->brk, section->header->sh_addr + section->header->sh_size);
			ld->bottom = MIN(ld->bottom, section->header->sh_addr);

			/* If section type is SHT_NOBITS (sh_type=8), initialize to 0.
			 * Otherwise, copy section contents from ELF file. */
			if (section->header->sh_type == 8)
			{
				void *ptr;

				ptr = calloc(1, section->header->sh_size);
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


static void ld_load_interp(struct ctx_t *ctx)
{
	struct loader_t *ld = ctx->loader;
	struct elf_file_t *elf_file;

	/* Open dynamic loader */
	ld_debug("\nLoading program interpreter '%s'\n", ld->interp);
	elf_file = elf_file_create_from_path(ld->interp);
	
	/* Load section from program interpreter */
	ld_load_sections(ctx, elf_file);

	/* Change program entry to the one specified by the interpreter */
	ld->interp_prog_entry = elf_file->header->e_entry;
	ld_debug("  program interpreter entry: 0x%x\n\n", ld->interp_prog_entry);
	elf_file_free(elf_file);
}


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


/* Load program headers table */
static void ld_load_program_headers(struct ctx_t *ctx)
{
	struct loader_t *ld = ctx->loader;
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
	ld_debug("\nLoading program headers\n");
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
	ld_debug("  virtual address for program header table: 0x%x\n", phdt_base);

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
		ld_debug("  header loaded at 0x%x\n", phdt_base + i * phdr_size);
		ld_debug("    type=%s, offset=0x%x, vaddr=0x%x, paddr=0x%x\n",
			str, program_header->header->p_offset,
			program_header->header->p_vaddr,
			program_header->header->p_paddr);
		ld_debug("    filesz=%d, memsz=%d, flags=%d, align=%d\n",
			program_header->header->p_filesz,
			program_header->header->p_memsz,
			program_header->header->p_flags,
			program_header->header->p_align);

		/* Program interpreter */
		if (program_header->header->p_type == 3) {
			mem_read_string(mem, program_header->header->p_vaddr, sizeof(str), str);
			ld->interp = strdup(str);
		}
	}

	/* Free buffer and save pointers */
	ld->phdt_base = phdt_base;
	ld->phdr_count = phdr_count;
}


/* Load auxiliary vector, and return its size in bytes. */
#define LD_AV_ENTRY(t, v) { uint32_t a_type = t, a_value = v; \
	mem_write(mem, sp, 4, &a_type); mem_write(mem, sp + 4, 4, &a_value); sp += 8; }
static uint32_t ld_load_av(struct ctx_t *ctx, uint32_t where)
{
	struct loader_t *ld = ctx->loader;
	struct mem_t *mem = ctx->mem;
	uint32_t sp = where;

	ld_debug("Loading auxiliary vector at 0x%x\n", where);

	/* Program headers */
	LD_AV_ENTRY(3, ld->phdt_base);  /* AT_PHDR */
	LD_AV_ENTRY(4, 32);  /* AT_PHENT -> program header size of 32 bytes */
	LD_AV_ENTRY(5, ld->phdr_count);  /* AT_PHNUM */

	/* Other values */
	LD_AV_ENTRY(6, MEM_PAGESIZE);  /* AT_PAGESZ */
	LD_AV_ENTRY(7, 0);  /* AT_BASE */
	LD_AV_ENTRY(8, 0);  /* AT_FLAGS */
	LD_AV_ENTRY(9, ld->prog_entry);  /* AT_ENTRY */
	LD_AV_ENTRY(11, getuid());  /* AT_UID */
	LD_AV_ENTRY(12, geteuid());  /* AT_EUID */
	LD_AV_ENTRY(13, getgid());  /* AT_GID */
	LD_AV_ENTRY(14, getegid());  /* AT_EGID */
	LD_AV_ENTRY(17, 0x64);  /* AT_CLKTCK */
	LD_AV_ENTRY(23, 0);  /* AT_SECURE */

	/*LD_AV_ENTRY(32, 0xffffe400);
	LD_AV_ENTRY(33, 0xffffe000);
	LD_AV_ENTRY(16, 0xbfebfbff);*/

	/* FIXME: AT_HWCAP, AT_PLATFORM, 32 and 33 */

	/* Finally, AT_NULL, and return size */
	LD_AV_ENTRY(0, 0);
	return sp - where;
}
#undef LD_AV_ENTRY


static void ld_load_stack(struct ctx_t *ctx)
{
	struct loader_t *ld = ctx->loader;
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
	ld_debug("mapping region for stack from 0x%x to 0x%x\n",
		ld->stack_top, ld->stack_base - 1);
	
	/* Load arguments and environment vars */
	ld->environ_base = LD_STACK_BASE - LD_MAX_ENVIRON;
	sp = ld->environ_base;
	argc = lnlist_count(ld->args);
	ld_debug("  saved 'argc=%d' at 0x%x\n", argc, sp);
	mem_write(mem, sp, 4, &argc);
	sp += 4;
	argvp = sp;
	sp = sp + (argc + 1) * 4;

	/* Save space for environ and null */
	envp = sp;
	sp += lnlist_count(ld->env) * 4 + 4;

	/* Load here the auxiliary vector */
	sp += ld_load_av(ctx, sp);

	/* Write arguments into stack */
	ld_debug("\nArguments:\n");
	for (i = 0; i < argc; i++) {
		lnlist_goto(ld->args, i);
		str = lnlist_get(ld->args);
		mem_write(mem, argvp + i * 4, 4, &sp);
		mem_write_string(mem, sp, str);
		ld_debug("  argument %d at 0x%x: '%s'\n", i, sp, str);
		sp += strlen(str) + 1;
	}
	mem_write(mem, argvp + i * 4, 4, &zero);

	/* Write environment variables */
	ld_debug("\nEnvironment variables:\n");
	for (i = 0; i < lnlist_count(ld->env); i++) {
		lnlist_goto(ld->env, i);
		str = lnlist_get(ld->env);
		mem_write(mem, envp + i * 4, 4, &sp);
		mem_write_string(mem, sp, str);
		ld_debug("  env var %d at 0x%x: '%s'\n", i, sp, str);
		sp += strlen(str) + 1;
	}
	mem_write(mem, envp + i * 4, 4, &zero);

	/* Check that we didn't overflow */
	if (sp > LD_STACK_BASE)
		fatal("'environ' overflow, increment LD_MAX_ENVIRON");
}


void ld_load_exe(struct ctx_t *ctx, char *exe)
{
	struct loader_t *ld = ctx->loader;
	struct fdt_t *fdt = ctx->fdt;
	char stdin_file_fullpath[MAX_STRING_SIZE];
	char stdout_file_fullpath[MAX_STRING_SIZE];
	char exe_fullpath[MAX_STRING_SIZE];

	/* Alternative stdin */
	ld_get_full_path(ctx, ld->stdin_file, stdin_file_fullpath, MAX_STRING_SIZE);
	if (*stdin_file_fullpath) {
		struct fd_t *fd;
		fd = fdt_entry_get(fdt, 0);
		assert(fd);
		fd->host_fd = open(stdin_file_fullpath, O_RDONLY);
		if (fd->host_fd < 0)
			fatal("%s: cannot open stdin", ld->stdin_file);
		ld_debug("%s: stdin redirected\n", stdin_file_fullpath);
	}

	/* Alternative stdout/stderr */
	ld_get_full_path(ctx, ld->stdout_file, stdout_file_fullpath, MAX_STRING_SIZE);
	if (*stdout_file_fullpath) {
		struct fd_t *fd1, *fd2;
		fd1 = fdt_entry_get(fdt, 1);
		fd2 = fdt_entry_get(fdt, 2);
		assert(fd1 && fd2);
		fd1->host_fd = fd2->host_fd = open(stdout_file_fullpath,
			O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, 0660);
		if (fd1->host_fd < 0)
			fatal("%s: cannot open stdout/stderr", ld->stdout_file);
		ld_debug("%s: stdout redirected\n", stdout_file_fullpath);
	}
	
	
	/* Load program into memory */
	ld_get_full_path(ctx, exe, exe_fullpath, MAX_STRING_SIZE);
	ld->exe = strdup(exe_fullpath);
	ld->elf_file = elf_file_create_from_path(exe_fullpath);

	/* Read sections and program entry */
	ld_load_sections(ctx, ld->elf_file);
	ld->prog_entry = ld->elf_file->header->e_entry;
	ld->brk = ROUND_UP(ld->brk, MEM_PAGESIZE);

	/* Load program header table. If we found a PT_INTERP program header,
	 * we have to load the program interpreter. This means we are dealing with
	 * a dynamically linked application. */
	ld_load_program_headers(ctx);
	if (ld->interp)
		ld_load_interp(ctx);

	/* Stack */
	ld_load_stack(ctx);

	/* Register initialization */
	ctx->regs->eip = ld->interp ? ld->interp_prog_entry : ld->prog_entry;
	ctx->regs->esp = ld->environ_base;

	ld_debug("Program entry is 0x%x\n", ctx->regs->eip);
	ld_debug("Initial stack pointer is 0x%x\n", ctx->regs->esp);
	ld_debug("Heap start set to 0x%x\n", ld->brk);
}


void ld_load_prog_from_ctxconfig(char *ctxconfig)
{
	struct config_t *config;
	struct ctx_t *ctx;
	struct loader_t *ld;
	int ctxnum;
	char *exe, *cwd;
	char *in, *out;
	char section[MAX_STRING_SIZE];
	
	/* Open context config file */
	config = config_create(ctxconfig);
	if (!config_load(config))
		fatal("%s: cannot open context configuration file",
			ctxconfig);
	
	/* Create contexts */
	for (ctxnum = 0; ; ctxnum++) {
	
		/* Create new context */
		sprintf(section, "Context %d", ctxnum);
		if (!config_section_exists(config, section))
			break;
		ctx = ctx_create();
		ld = ctx->loader;
		
		/* Arguments and environment variables */
		exe = config_read_string(config, section, "exe", "");
		lnlist_add(ld->args, strdup(exe));
		ld_add_args_string(ctx,
			config_read_string(config, section, "args", ""));
		ld_add_environ(ctx,
			config_read_string(config, section, "env", ""));
			
		/* Current working directory */
		cwd = config_read_string(config, section, "cwd", NULL);
		if (cwd)
			ld->cwd = strdup(cwd);
		else {
			ld->cwd = calloc(1, MAX_STRING_SIZE);
			ld->cwd = getcwd(ld->cwd, MAX_STRING_SIZE);
			if (!ld->cwd)
				fatal("loader: cannot retrieve current directory; increase MAX_STRING_SIZE");
		}
		
		/* Standard input and output */
		in = config_read_string(config, section, "stdin", "");
		out = config_read_string(config, section, "stdout", "");
		ld->stdin_file = strdup(in);
		ld->stdout_file = strdup(out);

		/* Load executable */
		ld_load_exe(ctx, exe);
	}
	config_free(config);
}


void ld_load_prog_from_cmdline(int argc, char **argv)
{
	struct ctx_t *ctx;
	struct loader_t *ld;
	
	/* Create context */
	ctx = ctx_create();
	ld = ctx->loader;

	/* Arguments and environment */
	ld_add_args_vector(ctx, argc, argv);
	ld_add_environ(ctx, "");

	/* Current working directory */
	ld->cwd = calloc(1, MAX_STRING_SIZE);
	ld->cwd = getcwd(ld->cwd, MAX_STRING_SIZE);
	if (!ld->cwd)
		fatal("loader: cannot retrieve current directory; increase MAX_STRING_SIZE");

	/* Redirections */
	ld->stdin_file = strdup("");
	ld->stdout_file = strdup("");

	/* Load executable */
	ld_load_exe(ctx, argv[0]);
}

