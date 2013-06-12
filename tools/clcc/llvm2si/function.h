/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef TOOLS_CLCC_LLVM2SI_FUNCTION_H
#define TOOLS_CLCC_LLVM2SI_FUNCTION_H

#include <arch/southern-islands/asm/arg.h>
#include <llvm-c/Core.h>

#include <stdio.h>


/* Forward declarations */
struct linked_list_t;
struct llvm2si_basic_block_t;
struct llvm2si_function_t;



/*
 * Function Argument
 */

struct llvm2si_function_arg_t
{
	/* Associated LLVM argument */
	LLVMValueRef llarg;

	/* Index that the argument occupies in the function argument list. */
	int index;


	/* The fields below are populated when the argument is inserted into
	 * a function with a call to 'llvm2si_function_add_arg()'. */

	/* Scalar register identifier containing the argument */
	int sreg;

	/* Function that the argument belongs to */
	struct llvm2si_function_t *function;

	/* All fields inherited from 'si_arg_t'. */
	struct si_arg_t *si_arg;

};

/* Create an argument. Value in 'index' indicates the argument index within the
 * function. */
struct llvm2si_function_arg_t *llvm2si_function_arg_create(LLVMValueRef llarg,
		int index);
void llvm2si_function_arg_free(struct llvm2si_function_arg_t *arg);

void llvm2si_function_arg_dump(struct llvm2si_function_arg_t *function_arg, FILE *f);




/*
 * Function Object
 */

struct llvm2si_function_t
{
	/* Function name */
	char *name;

	/* LLVM function */
	LLVMValueRef llfunction;

	/* Number of used registers */
	int num_sregs;  /* Scalar */
	int num_vregs;  /* Vector */

	int sreg_uav_table;  /* UAV table (2 registers) */
	int sreg_cb0;  /* CB0 (4 registers) */
	int sreg_cb1;  /* CB1 (4 registers) */
	int sreg_wgid;  /* Work-group ID (3 registers) */
	int sreg_lsize;  /* Local size (3 registers) */
	int sreg_offs;  /* Global offset (3 registers) */
	int sreg_uav10;  /* UAV10 (4 registers) */
	int sreg_uav11;  /* UAV11 (4 registers) */
	int sreg_arg;  /* Arguments (variable number of registers) */

	int vreg_lid;  /* Local ID (3 registers) */
	int vreg_gid;  /* Global ID (4 registers) */
	int vreg_sp;  /* Stack pointer (1 register) */

	/* List of arguments. Each element is of type
	 * 'struct llvm2si_function_arg_t' */
	struct list_t *arg_list;

	/* List of basic blocks. Each element is of type
	 * 'struct llvm2si_basic_block_t' */
	struct linked_list_t *basic_block_list;

	/* Symbol table associated with the function. */
	struct llvm2si_symbol_table_t *symbol_table;

	/* While code is generated, this variable keeps track of the total
	 * amount of bytes pushed into the stack for this function. */
	unsigned int stack_size;
};


struct llvm2si_function_t *llvm2si_function_create(LLVMValueRef llfunction);
void llvm2si_function_free(struct llvm2si_function_t *function);
void llvm2si_function_dump(struct llvm2si_function_t *function, FILE *f);

/* Add a basic block to the function. */
void llvm2si_function_add_basic_block(struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block);

/* Add argument 'arg' into the list of arguments of 'function', and generate
 * code to load it into 'basic_block'. */
void llvm2si_function_add_arg(struct llvm2si_function_t *function,
		struct llvm2si_function_arg_t *arg,
		struct llvm2si_basic_block_t *basic_block);
	
/* Generate initialization code for the function. The code will be dumped in
 * 'basic_block', which must have been previously added to the function with a
 * call to 'llvm2si_function_add_basic_block'. */
void llvm2si_function_emit_header(struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block);

/* Emit code to load arguments into scalar register. The 'basic_block' must
 * have been added to the 'function' before. The function will internally
 * create its list of arguments. */
void llvm2si_function_emit_args(struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block);

#endif

