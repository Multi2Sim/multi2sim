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

#include <arm-asm.h>


/* Some forward declarations */
struct arm_ctx_t;
struct file_desc_t;


/*
 * ARM Registers
 */
struct arm_regs_t
{
	/* Integer registers */
	uint32_t r0, r1, r2, r3;
	uint32_t r4,r5, r6, r7, r8, r9;
	uint32_t sl, fp, ip, sp, lr, pc;

} __attribute__((packed));
struct arm_regs_t *arm_regs_create();
void arm_regs_free(struct arm_regs_t *regs);
void arm_regs_copy(struct arm_regs_t *dst, struct arm_regs_t *src);




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
 * ARM Context
 */

struct arm_ctx_t
{
	/* Number of extra contexts using this loader */
	int num_links;

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

	/* Substructures */
	struct mem_t *mem; /* Virtual Memory image */
	struct arm_regs_t *regs; /* Logical register file */
	struct arm_file_desc_table_t *file_desc_table;  /* File descriptor table */

};

#define arm_loader_debug(...) debug(arm_loader_debug_category, __VA_ARGS__)
extern int arm_loader_debug_category;

struct arm_ctx_t *arm_ctx_create(void);
void arm_ctx_free(struct arm_ctx_t *ctx);

int arm_ctx_load_from_command_line(int argc, char **argv);
int arm_ctx_load_from_ctx_file(struct config_t *config, int index);

void arm_ctx_loader_get_full_path(struct arm_ctx_t *ctx, char *file_name, char *full_path, int size);



/*
 * ARM disassembler
 */

void arm_emu_disasm(char *path);


#endif
