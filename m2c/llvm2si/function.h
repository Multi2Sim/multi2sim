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

#ifndef M2C_LLVM2SI_FUNCTION_H
#define M2C_LLVM2SI_FUNCTION_H

#include <arch/southern-islands/asm/arg.h>
#include <llvm-c/Core.h>

#include <stdio.h>


/* Forward declarations */
CLASS_FORWARD_DECLARATION(Node);
CLASS_FORWARD_DECLARATION(Llvm2siFunction);
CLASS_FORWARD_DECLARATION(Llvm2siBasicBlock);
struct linked_list_t;
struct llvm2si_symbol_t;
struct llvm2si_node_t;
struct si2bin_arg_t;



/*
 * Function Argument
 */

struct llvm2si_function_arg_t
{
	char *name;
	LLVMValueRef llarg;  /* Associated LLVM argument */
	struct si_arg_t *si_arg;  /* Inherited from 'si_arg_t'. */


	/* The fields below are populated when the argument is inserted into
	 * a function with a call to 'llvm2si_function_add_arg()'. */

	Llvm2siFunction *function;  /* Function it belongs to */
	int index;  /* Index occupied in function argument list */
	int uav_index;  /* For arguments of type 'si_arg_pointer', and scope 'si_arg_uav' */
	int sreg;  /* Scalar register identifier containing the argument */
	int vreg;  /* Vector register where argument was copied */
};


struct llvm2si_function_arg_t *llvm2si_function_arg_create(LLVMValueRef llarg);
void llvm2si_function_arg_free(struct llvm2si_function_arg_t *arg);
void llvm2si_function_arg_dump(struct llvm2si_function_arg_t *function_arg, FILE *f);




/*
 * Function UAV Object
 */

struct llvm2si_function_uav_t
{
	/* Function where it belongs */
	Llvm2siFunction *function;

	/* UAV index in 'function->uav_list'. Uav10 has an index 0, uav11 has
	 * index 1, etc. */
	int index;

	/* Base scalar register of a group of 4 assigned to the UAV. This
	 * register identifier is a multiple of 4. */
	int sreg;
};

struct llvm2si_function_uav_t *llvm2si_function_uav_create(void);
void llvm2si_function_uav_free(struct llvm2si_function_uav_t *uav);




/*
 * Class 'Llvm2siFunction'
 */

CLASS_BEGIN(Llvm2siFunction, Object)
	
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
	int sreg_gsize;  /* Global size (3 register) */
	int sreg_lsize;  /* Local size (3 registers) */
	int sreg_offs;  /* Global offset (3 registers) */

	int vreg_lid;  /* Local ID (3 registers) */
	int vreg_gid;  /* Global ID (4 registers) */

	/* List of arguments. Each element is of type
	 * 'struct llvm2si_function_arg_t' */
	struct list_t *arg_list;

	/* List of UAVs, starting at uav10. Each UAV is associated with one
	 * function argument using a buffer in global memory. */
	struct list_t *uav_list;

	/* Predefined nodes */
	Node *header_node;
	Node *uavs_node;
	Node *args_node;
	Node *body_node;

	/* Symbol table associated with the function, storing LLVM variables */
	struct llvm2si_symbol_table_t *symbol_table;

	/* Control tree */
	CTree *ctree;

	/* List of elements found in LLVM 'phi' instructions during emission of
	 * the function body. */
	struct linked_list_t *phi_list;

	/* While code is generated, this variable keeps track of the total
	 * amount of bytes pushed into the stack for this function. */
	unsigned int stack_size;

CLASS_END(Llvm2siFunction)


void Llvm2siFunctionCreate(Llvm2siFunction *self, LLVMValueRef llfunction);
void Llvm2siFunctionDestroy(Llvm2siFunction *self);

/* Virtual function from class Object */
void Llvm2siFunctionDump(Object *self, FILE *f);

/* Add a basic block to the function */
/* Add a basic block to the function after basic block 'after'. If the value in
 * 'after' is NULL, the basic block is added after the last basic block added
 * using this function. */
void llvm2si_function_add_basic_block(Llvm2siFunction *function,
		Llvm2siBasicBlock *basic_block);

/* Add a basic block to the function before basic block 'before'. The value
 * in 'before' can only be NULL if the function is empty. */
void llvm2si_function_add_basic_block_before(
		Llvm2siFunction *function,
		Llvm2siBasicBlock *basic_block,
		Llvm2siBasicBlock *before);

/* Generate initialization code for the function in basic block
 * 'function->basic_block_header'. */
void llvm2si_function_emit_header(Llvm2siFunction *function);

/* Emit code to load arguments into registers. The code will be emitted in
 * 'function->basic_block_args'. UAVs will be created and loaded in
 * 'function->basic_block_uavs', as they are needed by new arguments. */
void llvm2si_function_emit_args(Llvm2siFunction *function);

/* Emit code for the function body. The first basic block of the function will
 * be added at the end of 'basic_block', which should be already part of the
 * function. As the code emission progresses, new basic blocks will be created. */
void llvm2si_function_emit_body(Llvm2siFunction *function);

/* Emit code for the phi elements that were encountered during the emission of
 * the function body, comming from LLVM phi nodes. */
void llvm2si_function_emit_phi(Llvm2siFunction *function);

/* Emit additional instructions managing active masks and active mask stacks
 * related with the function control flow. */
void llvm2si_function_emit_control_flow(Llvm2siFunction *function);

/* Create a Southern Islands instruction argument from an LLVM value. The type
 * of argument created depends on the LLVM value as follows:
 *   - If the LLVM value is an integer constant, the Southern Islands argument
 *     will be of type integer literal.
 *   - If the LLVM value is an LLVM identifier, the Southern Islands argument
 *     will be the vector register associated with that symbol. In this case,
 *     the symbol is returned in the 'symbol_ptr' argument.
 */
struct si2bin_arg_t *llvm2si_function_translate_value(
		Llvm2siFunction *function,
		LLVMValueRef llvalue,
		struct llvm2si_symbol_t **symbol_ptr);

/* Convert an argument of type literal (any variant) into a vector register by
 * emitting a 'v_mob_b32' instruction. The original argument is consumed and
 * make part of the new instruction, while a new argument instance is returned
 * containing the new vector register. If the original argument was not a
 * literal, it will be returned directly, and no instruction is emitted. */
struct si2bin_arg_t *llvm2si_function_const_to_vreg(
		Llvm2siFunction *function,
		Llvm2siBasicBlock *basic_block,
		struct si2bin_arg_t *arg);

/* Allocate 'count' scalar/vector registers where the first register
 * identifier is a multiple of 'align'. */
int llvm2si_function_alloc_sreg(Llvm2siFunction *function,
		int count, int align);
int llvm2si_function_alloc_vreg(Llvm2siFunction *function,
		int count, int align);

#endif

