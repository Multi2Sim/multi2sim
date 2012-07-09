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

#include <arm-asm.h>


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
	struct mem_t *mem;
	struct arm_regs_t *regs;
};

struct arm_ctx_t *arm_ctx_create();

/*
 * ARM disassembler
 */
void arm_emu_disasm(char *path);

#endif
