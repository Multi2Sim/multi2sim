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

#ifndef ARCH_X86_EMU_LOADER_H
#define ARCH_X86_EMU_LOADER_H

/* Forward type declarations */
struct config_t;


struct x86_loader_t
{
	/* Number of extra contexts using this loader */
	int num_links;

	/* Program data */
	struct elf_file_t *elf_file;
	struct linked_list_t *args;
	struct linked_list_t *env;
	char *interp;  /* Executable interpreter */
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
};


#define x86_loader_debug(...) debug(x86_loader_debug_category, __VA_ARGS__)
extern int x86_loader_debug_category;

extern char *x86_loader_help;

struct x86_loader_t *x86_loader_create(void);
void x86_loader_free(struct x86_loader_t *ld);

struct x86_loader_t *x86_loader_link(struct x86_loader_t *ld);
void x86_loader_unlink(struct x86_loader_t *ld);

void x86_loader_convert_filename(struct x86_loader_t *ld, char *file_name);
void x86_loader_get_full_path(struct x86_ctx_t *ctx, char *file_name, char *full_path, int size);

void x86_loader_add_args(struct x86_ctx_t *ctx, int argc, char **argv);
void x86_loader_add_cmdline(struct x86_ctx_t *ctx, char *cmdline);
void x86_loader_set_cwd(struct x86_ctx_t *ctx, char *cwd);
void x86_loader_set_redir(struct x86_ctx_t *ctx, char *stdin, char *stdout);
void x86_loader_load_exe(struct x86_ctx_t *ctx, char *exe);

void x86_loader_load_from_ctx_config(struct config_t *config, char *section);
void x86_loader_load_from_command_line(int argc, char **argv);


#endif

