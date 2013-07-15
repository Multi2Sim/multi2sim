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
struct elf_file_t;



/*
 * Class 'X86Context'
 * Additional Functions
 */

void X86ContextAddArgsVector(X86Context *self, int argc, char **argv);
void X86ContextAddArgsString(X86Context *self, char *args);
void X86ContextAddEnv(X86Context *self, char *env);

void X86ContextLoadELFSections(X86Context *self, struct elf_file_t *elf_file);
void X86ContextLoadInterp(X86Context *self);
void X86ContextLoadProgramHeaders(X86Context *self);
unsigned int X86ContextLoadAV(X86Context *self, unsigned int where);
void X86ContextLoadStack(X86Context *self);

void X86ContextLoadExe(X86Context *self, char *exe);
void X86ContextGetFullPath(X86Context *self, char *file_name, char *full_path, int size);



/*
 * Object 'x86_loader_t'
 */

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


struct x86_loader_t *x86_loader_create(void);
void x86_loader_free(struct x86_loader_t *loader);

struct x86_loader_t *x86_loader_link(struct x86_loader_t *loader);
void x86_loader_unlink(struct x86_loader_t *loader);




/*
 * Public
 */

#define x86_loader_debug(...) debug(x86_loader_debug_category, __VA_ARGS__)
extern int x86_loader_debug_category;

extern char *x86_loader_help;


#endif

