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

#include <assert.h>

#include <arch/southern-islands/asm/arg.h>
#include <arch/southern-islands/asm/bin-file.h>
#include <m2c/common/basic-block.h>
#include <m2c/common/ctree.h>
#include <m2c/common/cnode.h>
#include <m2c/si2bin/arg.h>
#include <m2c/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <llvm-c/Core.h>

#include "basic-block.h"
#include "function.h"
#include "symbol.h"
#include "symbol-table.h"


/*
 * Function Argument Object
 */

/* Return a Southern Islands argument type from an LLVM type. */
static enum si_arg_data_type_t llvm2si_function_arg_get_data_type(LLVMTypeRef lltype)
{
	LLVMTypeKind lltype_kind;
	int bit_width;

	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind == LLVMIntegerTypeKind)
	{
		bit_width = LLVMGetIntTypeWidth(lltype);
		switch (bit_width)
		{
		case 1: return si_arg_i1;
		case 8: return si_arg_i8;
		case 16: return si_arg_i16;
		case 32: return si_arg_i32;
		case 64: return si_arg_i64;

		default:
			fatal("%s: invalid argument bit width (%d)",
				__FUNCTION__, bit_width);
			return si_arg_data_type_invalid;
		}
	}
	else
	{
		fatal("%s: unsupported argument type kind (%d)",
				__FUNCTION__, lltype_kind);
		return si_arg_data_type_invalid;
	}
}


struct llvm2si_function_arg_t *llvm2si_function_arg_create(LLVMValueRef llarg)
{
	struct llvm2si_function_arg_t *arg;
	struct si_arg_t *si_arg;

	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	char *name;

	/* Get argument name */
	name = (char *) LLVMGetValueName(llarg);
	if (!*name)
		fatal("%s: anonymous arguments not allowed", __FUNCTION__);

	/* Initialize 'si_arg' object */
	lltype = LLVMTypeOf(llarg);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind == LLVMPointerTypeKind)
	{
		lltype = LLVMGetElementType(lltype);
		si_arg = si_arg_create(si_arg_pointer, name);
		si_arg->pointer.scope = si_arg_uav;
		si_arg->pointer.data_type = llvm2si_function_arg_get_data_type(lltype);
	}
	else
	{
		si_arg = si_arg_create(si_arg_value, name);
		si_arg->value.data_type = llvm2si_function_arg_get_data_type(lltype);
	}

	/* Initialize 'arg' object */
	arg = xcalloc(1, sizeof(struct llvm2si_function_arg_t));
	arg->name = xstrdup(name);
	arg->llarg = llarg;
	arg->si_arg = si_arg;

	/* Return */
	return arg;
}


void llvm2si_function_arg_free(struct llvm2si_function_arg_t *arg)
{
	assert(arg->name);
	assert(arg->si_arg);
	str_free(arg->name);
	si_arg_free(arg->si_arg);
	free(arg);
}


void llvm2si_function_arg_dump(struct llvm2si_function_arg_t *arg, FILE *f)
{
	struct si_arg_t *si_arg = arg->si_arg;

	switch (si_arg->type)
	{
	case si_arg_pointer:

		switch (si_arg->pointer.scope)
		{

		case si_arg_uav:
			
			/* Type, name, offset, UAV */
			fprintf(f, "\t%s* %s %d uav%d\n",
				str_map_value(&si_arg_data_type_map, si_arg->pointer.data_type),
				si_arg->name, arg->index * 16, arg->uav_index + 10);
			break;

		default:

			fatal("%s: pointer scope not supported (%d)",
				__FUNCTION__, si_arg->pointer.scope);
		}

		break;

	default:
		fatal("%s: argument type not recognized (%d)",
				__FUNCTION__, si_arg->type);
	}
}




/*
 * Function UAV Object
 */

struct llvm2si_function_uav_t *llvm2si_function_uav_create(void)
{
	struct llvm2si_function_uav_t *uav;

	/* Initialize */
	uav = xcalloc(1, sizeof(struct llvm2si_function_uav_t));

	/* Return */
	return uav;
}


void llvm2si_function_uav_free(struct llvm2si_function_uav_t *uav)
{
	free(uav);
}




/*
 * Function Object
 */

/* Add a UAV to the UAV list. This function allocates a series of 4 aligned
 * scalar registers to the UAV, populating its 'index' and 'sreg' fields.
 * The UAV object will be freed automatically after calling this function.
 * Emit the code needed to load UAV into 'function->basic_block_uavs' */
static void llvm2si_function_add_uav(struct llvm2si_function_t *function,
		struct llvm2si_function_uav_t *uav)
{
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;
	struct llvm2si_basic_block_t *basic_block;

	/* Associate UAV with function */
	assert(!uav->function);
	uav->function = function;

	/* Get basic block or create it */
	basic_block = LLVM2SI_BASIC_BLOCK(cnode_get_basic_block(
			function->uavs_node));
	if (!basic_block)
		basic_block = llvm2si_basic_block_create(function,
				function->uavs_node);

	/* Allocate 4 aligned scalar registers */
	uav->sreg = llvm2si_function_alloc_sreg(function, 4, 4);

	/* Insert to UAV list */
	uav->index = function->uav_list->count;
	list_add(function->uav_list, uav);

	/* Emit code to load UAV.
	 * s_load_dwordx4 s[uavX:uavX+3], s[uav_table:uav_table+1], x * 8
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			uav->sreg, uav->sreg + 3));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			function->sreg_uav_table, function->sreg_uav_table + 1));
	list_add(arg_list, si2bin_arg_create_literal((uav->index + 10) * 8));
	inst = si2bin_inst_create(SI_INST_S_LOAD_DWORDX4, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


/* Add argument 'arg' into the list of arguments of 'function', and emit code
 * to load it into 'function->basic_block_args'. */
static void llvm2si_function_add_arg(struct llvm2si_function_t *function,
		struct llvm2si_function_arg_t *arg)
{
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;
	struct llvm2si_symbol_t *symbol;
	struct llvm2si_basic_block_t *basic_block;
	struct llvm2si_function_uav_t *uav;

	/* Check that argument does not belong to a function yet */
	if (arg->function)
		panic("%s: argument already added", __FUNCTION__);

	/* Get basic block, or create it */
	basic_block = LLVM2SI_BASIC_BLOCK(cnode_get_basic_block(
			function->args_node));
	if (!basic_block)
		basic_block = llvm2si_basic_block_create(function,
				function->args_node);

	/* Add argument */
	list_add(function->arg_list, arg);
	arg->function = function;
	arg->index = function->arg_list->count - 1;

	/* Allocate 1 scalar and 1 vector register for the argument */
	arg->sreg = llvm2si_function_alloc_sreg(function, 1, 1);
	arg->vreg = llvm2si_function_alloc_vreg(function, 1, 1);

	/* Generate code to load argument into a scalar register.
	 * s_buffer_load_dword s[arg], s[cb1:cb1+3], idx*4
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register(arg->sreg));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(function->sreg_cb1,
			function->sreg_cb1 + 3));
	list_add(arg_list, si2bin_arg_create_literal(arg->index * 4));
	inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);

	/* Copy argument into a vector register. This vector register will be
	 * used for convenience during code emission, so that we don't have to
	 * worry at this point about different operand type encodings for
	 * instructions. Optimization passes will get rid later of redundant
	 * copies and scalar opportunities.
	 * v_mov_b32 v[arg], s[arg]
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(arg->vreg));
	list_add(arg_list, si2bin_arg_create_scalar_register(arg->sreg));
	inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);

	/* Insert argument name in symbol table, using its vector register. */
	symbol = llvm2si_symbol_create_vreg(arg->name, arg->vreg);
	llvm2si_symbol_table_add_symbol(function->symbol_table, symbol);

	/* If argument is an object in global memory, create a UAV
	 * associated with it. */
	if (arg->si_arg->type == si_arg_pointer &&
			arg->si_arg->pointer.scope == si_arg_uav)
	{
		/* New UAV */
		uav = llvm2si_function_uav_create();
		llvm2si_function_add_uav(function, uav);

		/* Store UAV index in argument and symbol */
		llvm2si_symbol_set_uav_index(symbol, uav->index);
		arg->uav_index = uav->index;
	}
}


static void llvm2si_function_dump_data(struct llvm2si_function_t *function,
		FILE *f)
{
	/* Section header */
	fprintf(f, ".data\n");

	/* User elements */
	fprintf(f, "\tuserElements[0] = PTR_UAV_TABLE, 0, s[%d:%d]\n",
			function->sreg_uav_table, function->sreg_uav_table + 1);
	fprintf(f, "\tuserElements[1] = IMM_CONST_BUFFER, 0, s[%d:%d]\n",
			function->sreg_cb0, function->sreg_cb0 + 3);
	fprintf(f, "\tuserElements[2] = IMM_CONST_BUFFER, 1, s[%d:%d]\n",
			function->sreg_cb1, function->sreg_cb1 + 3);
	fprintf(f, "\n");

	/* Floating-point mode */
	fprintf(f, "\tFloatMode = 192\n");
	fprintf(f, "\tIeeeMode = 0\n");
	fprintf(f, "\n");

	/* Program resources */
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:USER_SGPR = %d\n", function->sreg_wgid);
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:TGID_X_EN = %d\n", 1);
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:TGID_Y_EN = %d\n", 1);
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:TGID_Z_EN = %d\n", 1);
	fprintf(f, "\n");
}


struct llvm2si_function_t *llvm2si_function_create(LLVMValueRef llfunction)
{
	struct llvm2si_function_t *function;
	struct ctree_t *ctree;

	/* Allocate */
	function = xcalloc(1, sizeof(struct llvm2si_function_t));
	function->llfunction = llfunction;
	function->name = xstrdup(LLVMGetValueName(llfunction));
	function->arg_list = list_create();
	function->uav_list = list_create();
	function->symbol_table = llvm2si_symbol_table_create();
	function->ctree = ctree = ctree_create(function->name);

	/* Create pre-defined nodes in control tree */
	function->header_node = cnode_create_leaf("header");
	function->uavs_node = cnode_create_leaf("uavs");
	function->args_node = cnode_create_leaf("args");
	ctree_add_node(ctree, function->header_node);
	ctree_add_node(ctree, function->uavs_node);
	ctree_add_node(ctree, function->args_node);
	ctree->entry_node = function->header_node;

	/* Add all nodes from the LLVM control flow graph */
	function->body_node = ctree_add_llvm_cfg(ctree, llfunction);

	/* Connect nodes */
	cnode_connect(function->header_node, function->uavs_node);
	cnode_connect(function->uavs_node, function->args_node);
	cnode_connect(function->args_node, function->body_node);

	/* Return */
	return function;
}


void llvm2si_function_free(struct llvm2si_function_t *function)
{
	int index;

	/* Free list of arguments */
	LIST_FOR_EACH(function->arg_list, index)
		llvm2si_function_arg_free(list_get(function->arg_list, index));
	list_free(function->arg_list);

	/* Free list of UAVs */
	LIST_FOR_EACH(function->uav_list, index)
		llvm2si_function_uav_free(list_get(function->uav_list, index));
	list_free(function->uav_list);

	/* Free control tree */
	if (function->ctree)
		ctree_free(function->ctree);

	/* Rest */
	llvm2si_symbol_table_free(function->symbol_table);
	free(function->name);
	free(function);
}


void llvm2si_function_dump(struct llvm2si_function_t *function, FILE *f)
{
	struct llvm2si_basic_block_t *basic_block;
	struct llvm2si_function_arg_t *function_arg;

	struct linked_list_t *node_list;
	struct cnode_t *node;

	int index;

	/* Function name */
	fprintf(f, ".global %s\n\n", function->name);

	/* Arguments */
	fprintf(f, ".args\n");
	LIST_FOR_EACH(function->arg_list, index)
	{
		function_arg = list_get(function->arg_list, index);
		llvm2si_function_arg_dump(function_arg, f);
	}
	fprintf(f, "\n");

	/* Dump basic blocks */
	fprintf(f, ".text\n");
	node_list = linked_list_create();
	ctree_traverse(function->ctree, node_list, NULL);
	LINKED_LIST_FOR_EACH(node_list)
	{
		/* Skip abstract nodes */
		node = linked_list_get(node_list);
		if (node->kind != cnode_leaf)
			continue;

		/* Get node's basic block */
		basic_block = LLVM2SI_BASIC_BLOCK(node->leaf.basic_block);
		if (!basic_block)
			continue;

		/* Dump code of basic block */
		llvm2si_basic_block_dump(BASIC_BLOCK(basic_block), f);
	}
	linked_list_free(node_list);
	fprintf(f, "\n");

	/* Dump section '.data' */
	llvm2si_function_dump_data(function, f);
	fprintf(f, "\n");
}


void llvm2si_function_emit_header(struct llvm2si_function_t *function)
{
	struct llvm2si_basic_block_t *basic_block;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	char comment[MAX_STRING_SIZE];
	int index;

	/* Create header basic block */
	basic_block = llvm2si_basic_block_create(function,
			function->header_node);

	/* Function must be empty at this point */
	assert(!function->num_sregs);
	assert(!function->num_vregs);

	/* Allocate 3 vector registers (v[0:2]) for local ID */
	function->vreg_lid = llvm2si_function_alloc_vreg(function, 3, 1);
	if (function->vreg_lid)
		panic("%s: vreg_lid is expented to be 0", __FUNCTION__);

	/* Allocate 2 scalar registers for UAV table. The value for these
	 * registers is assigned by the runtime based on info found in the
	 * 'userElements' metadata of the binary.*/
	function->sreg_uav_table = llvm2si_function_alloc_sreg(function, 2, 1);

	/* Allocate 4 scalar registers for CB0, and 4 more for CB1. The
	 * values for these registers will be assigned by the runtime based
	 * on info present in the 'userElements' metadata. */
	function->sreg_cb0 = llvm2si_function_alloc_sreg(function, 4, 1);
	function->sreg_cb1 = llvm2si_function_alloc_sreg(function, 4, 1);

	/* Allocate 3 scalar registers for the work-group ID. The content of
	 * these register will be populated by the runtime based on info found
	 * in COMPUTE_PGM_RSRC2 metadata. */
	function->sreg_wgid = llvm2si_function_alloc_sreg(function, 3, 1);

	/* Obtain global size in s[gsize:gsize+2].
	 * s_buffer_load_dword s[gsize], s[cb0:cb0+3], 0x00
	 * s_buffer_load_dword s[gsize+1], s[cb0:cb0+3], 0x01
	 * s_buffer_load_dword s[gsize+2], s[cb0:cb0+3], 0x02
	 */
	llvm2si_basic_block_add_comment(basic_block, "Obtain global size");
	function->sreg_gsize = llvm2si_function_alloc_sreg(function, 3, 1);
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_gsize + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				function->sreg_cb0, function->sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);
	}

	/* Obtain local size in s[lsize:lsize+2].
	 *
	 * s_buffer_load_dword s[lsize], s[cb0:cb0+3], 0x04
	 * s_buffer_load_dword s[lsize+1], s[cb0:cb0+3], 0x05
	 * s_buffer_load_dword s[lsize+2], s[cb0:cb0+3], 0x06
	 */
	llvm2si_basic_block_add_comment(basic_block, "Obtain local size");
	function->sreg_lsize = llvm2si_function_alloc_sreg(function, 3, 1);
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_lsize + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				function->sreg_cb0, function->sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(4 + index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);
	}

	/* Obtain global offset in s[offs:offs+2].
	 *
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x18
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x19
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x1a
	 */
	llvm2si_basic_block_add_comment(basic_block, "Obtain global offset");
	function->sreg_offs = llvm2si_function_alloc_sreg(function, 3, 1);
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_offs + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				function->sreg_cb0, function->sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(0x18 + index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);
	}

	/* Calculate global ID in dimensions [0:2] and store it in v[3:5].
	 *
	 * v_mov_b32 v[gid+dim], s[lsize+dim]
	 * v_mul_i32_i24 v[gid+dim], s[wgid+dim], v[gid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], v[lid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], s[offs+dim]
	 */
	function->vreg_gid = llvm2si_function_alloc_vreg(function, 3, 1);
	for (index = 0; index < 3; index++)
	{
		/* Comment */
		snprintf(comment, sizeof comment, "Calculate global ID "
				"in dimension %d", index);
		llvm2si_basic_block_add_comment(basic_block, comment);

		/* v_mov_b32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_lsize + index));
		inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);

		/* v_mul_i32_i24 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_wgid + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_gid + index));
		inst = si2bin_inst_create(SI_INST_V_MUL_I32_I24, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);

		/* v_add_i32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_lid + index));
		inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);

		/* v_add_i32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_offs + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				function->vreg_gid + index));
		inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);
	}
}


void llvm2si_function_emit_args(struct llvm2si_function_t *function)
{
	LLVMValueRef llfunction;
	LLVMValueRef llarg;

	struct llvm2si_function_arg_t *arg;

	/* Emit code for each argument individually */
	llfunction = function->llfunction;
	for (llarg = LLVMGetFirstParam(llfunction); llarg;
			llarg = LLVMGetNextParam(llarg))
	{
		/* Create function argument and add it */
		arg = llvm2si_function_arg_create(llarg);

		/* Add the argument to the list. This call will cause the
		 * corresponding code to be emitted. */
		llvm2si_function_add_arg(function, arg);
	}
}


void llvm2si_function_emit_body(struct llvm2si_function_t *function)
{
	struct llvm2si_basic_block_t *basic_block;
	struct linked_list_t *node_list;

	struct ctree_t *ctree;
	struct cnode_t *node;

	/* Code for the function body must be emitted using a depth-first
	 * traversal of the control tree. For this, we need right here the
	 * structural analysis that produces the control tree from the
	 * control flow graph.
	 */
	ctree = function->ctree;
	assert(!ctree->structural_analysis_done);
	ctree_structural_analysis(ctree);

	/* Whether we use a pre- or a post-order traversal does not matter,
	 * since we are only considering the leaf nodes.
	 */
	node_list = linked_list_create();
	ctree_traverse(ctree, node_list, NULL);

	/* Emit code for basic blocks */
	LINKED_LIST_FOR_EACH(node_list)
	{
		/* Skip abstract nodes */
		node = linked_list_get(node_list);
		if (node->kind != cnode_leaf)
			continue;

		/* Skip nodes with no LLVM code to translate */
		if (!node->llbb)
			continue;

		/* Create basic block and emit the code */
		assert(!node->leaf.basic_block);
		basic_block = llvm2si_basic_block_create(function, node);
		llvm2si_basic_block_emit(basic_block, node->llbb);
	}

	/* Free structures */
	linked_list_free(node_list);
}


static void llvm2si_function_emit_if_then(
		struct llvm2si_function_t *function,
		struct cnode_t *node)
{
	struct cnode_t *if_node;
	struct cnode_t *then_node;

	struct llvm2si_basic_block_t *if_bb;
	struct llvm2si_basic_block_t *then_bb;

	struct llvm2si_symbol_t *cond_symbol;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	LLVMBasicBlockRef llbb;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;

	/* Identify the two nodes */
	assert(node->kind == cnode_abstract);
	assert(node->abstract.region == cnode_region_if_then);
	assert(node->abstract.child_list->count == 2);
	if_node = linked_list_goto(node->abstract.child_list, 0);
	then_node = linked_list_goto(node->abstract.child_list, 1);

	/* Make sure roles match */
	assert(if_node->role == cnode_role_if);
	assert(then_node->role == cnode_role_then);

	/* Get basic blocks. 'If' node should be a leaf. */
	then_node = cnode_get_last_leaf(then_node);
	assert(if_node->kind == cnode_leaf);
	assert(then_node->kind == cnode_leaf);
	if_bb = LLVM2SI_BASIC_BLOCK(if_node->leaf.basic_block);
	then_bb = LLVM2SI_BASIC_BLOCK(then_node->leaf.basic_block);


	/*** Code for 'If' block ***/

	/* Get 'If' basic block terminator */
	llbb = if_node->llbb;
	llinst = LLVMGetBasicBlockTerminator(llbb);
	assert(llinst);
	assert(LLVMGetInstructionOpcode(llinst) == LLVMBr);
	assert(LLVMGetNumOperands(llinst) == 3);

	/* Get symbol associated with condition variable */
	llcond = LLVMGetOperand(llinst, 0);
	cond_name = (char *) LLVMGetValueName(llcond);
	cond_symbol = llvm2si_symbol_table_lookup(function->symbol_table, cond_name);
	assert(cond_symbol);
	assert(cond_symbol->type == llvm2si_symbol_scalar_register);
	assert(cond_symbol->count == 2);
	cond_sreg = cond_symbol->reg;

	/* Allocate two scalar registers to push the active mask */
	tos_sreg = llvm2si_function_alloc_sreg(function, 2, 2);

	/* Emit active mask push and set at the end of the 'If' block.
	 * s_and_saveexec_b64 <tos_sreg> <cond_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(cond_sreg, cond_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_AND_SAVEEXEC_B64, arg_list);
	llvm2si_basic_block_add_inst(if_bb, inst);


	/*** Code for 'then' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	llvm2si_basic_block_add_inst(then_bb, inst);
}


static void llvm2si_function_emit_if_then_else(
		struct llvm2si_function_t *function,
		struct cnode_t *node)
{
	struct cnode_t *if_node;
	struct cnode_t *then_node;
	struct cnode_t *else_node;

	struct llvm2si_basic_block_t *if_bb;
	struct llvm2si_basic_block_t *then_bb;
	struct llvm2si_basic_block_t *else_bb;

	struct llvm2si_symbol_t *cond_symbol;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	LLVMBasicBlockRef llbb;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;

	/* Identify the three nodes */
	assert(node->kind == cnode_abstract);
	assert(node->abstract.region == cnode_region_if_then_else);
	assert(node->abstract.child_list->count == 3);
	if_node = linked_list_goto(node->abstract.child_list, 0);
	then_node = linked_list_goto(node->abstract.child_list, 1);
	else_node = linked_list_goto(node->abstract.child_list, 2);

	/* Make sure roles match */
	assert(if_node->role == cnode_role_if);
	assert(then_node->role == cnode_role_then);
	assert(else_node->role == cnode_role_else);

	/* Get basic blocks. 'If' node should be a leaf. */
	then_node = cnode_get_last_leaf(then_node);
	else_node = cnode_get_last_leaf(else_node);
	assert(if_node->kind == cnode_leaf);
	assert(then_node->kind == cnode_leaf);
	assert(else_node->kind == cnode_leaf);
	if_bb = LLVM2SI_BASIC_BLOCK(if_node->leaf.basic_block);
	then_bb = LLVM2SI_BASIC_BLOCK(then_node->leaf.basic_block);
	else_bb = LLVM2SI_BASIC_BLOCK(else_node->leaf.basic_block);


	/*** Code for 'If' block ***/

	/* Get 'If' basic block terminator */
	llbb = if_node->llbb;
	llinst = LLVMGetBasicBlockTerminator(llbb);
	assert(llinst);
	assert(LLVMGetInstructionOpcode(llinst) == LLVMBr);
	assert(LLVMGetNumOperands(llinst) == 3);

	/* Get symbol associated with condition variable */
	llcond = LLVMGetOperand(llinst, 0);
	cond_name = (char *) LLVMGetValueName(llcond);
	cond_symbol = llvm2si_symbol_table_lookup(function->symbol_table, cond_name);
	assert(cond_symbol);
	assert(cond_symbol->type == llvm2si_symbol_scalar_register);
	assert(cond_symbol->count == 2);
	cond_sreg = cond_symbol->reg;

	/* Allocate two scalar registers to push the active mask */
	tos_sreg = llvm2si_function_alloc_sreg(function, 2, 2);

	/* Emit active mask push and set at the end of the 'If' block.
	 * s_and_saveexec_b64 <tos_sreg> <cond_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(cond_sreg, cond_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_AND_SAVEEXEC_B64, arg_list);
	llvm2si_basic_block_add_inst(if_bb, inst);


	/*** Code for 'then' block ***/

	/* Invert active mask and-ing it with the top of the stack.
	 * s_andn2_b64 exec, <tos_sreg>, exec
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	inst = si2bin_inst_create(SI_INST_S_ANDN2_B64, arg_list);
	llvm2si_basic_block_add_inst(then_bb, inst);


	/*** Code for 'else' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	llvm2si_basic_block_add_inst(else_bb, inst);
}


static void llvm2si_function_emit_while_loop(
		struct llvm2si_function_t *function,
		struct cnode_t *node)
{
	struct cnode_t *head_node;
	struct cnode_t *tail_node;
	struct cnode_t *pre_node;
	struct cnode_t *exit_node;

	struct llvm2si_basic_block_t *head_bb;
	struct llvm2si_basic_block_t *tail_bb;
	struct llvm2si_basic_block_t *pre_bb;
	struct llvm2si_basic_block_t *exit_bb;

	struct llvm2si_symbol_t *cond_symbol;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	enum si_inst_opcode_t opcode;

	LLVMBasicBlockRef llbb;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;


	/* Identify the two nodes */
	assert(node->kind == cnode_abstract);
	assert(node->abstract.region == cnode_region_while_loop);
	assert(node->abstract.child_list->count == 4);
	pre_node = linked_list_goto(node->abstract.child_list, 0);
	head_node = linked_list_goto(node->abstract.child_list, 1);
	tail_node = linked_list_goto(node->abstract.child_list, 2);
	exit_node = linked_list_goto(node->abstract.child_list, 3);

	/* Make sure roles match */
	assert(pre_node->role == cnode_role_pre);
	assert(head_node->role == cnode_role_head);
	assert(tail_node->role == cnode_role_tail);
	assert(exit_node->role == cnode_role_exit);

	/* Get basic blocks. Pre-header/head/exit nodes should be a leaves.
	 * Tail node can be an abstract node, which we need to track down to
	 * its last leaf to introduce control flow at the end.
	 *
	 * Basic blocks must exist associated to the head and the tail blocks:
	 * they come from LLVM blocks already emitted. But pre-header and exit
	 * blocks have been inserted during the structural analysis, so they
	 * contain no basic block yet.
	 */
	tail_node = cnode_get_last_leaf(tail_node);
	assert(pre_node->kind == cnode_leaf);
	assert(head_node->kind == cnode_leaf);
	assert(tail_node->kind == cnode_leaf);
	assert(exit_node->kind == cnode_leaf);
	pre_bb = LLVM2SI_BASIC_BLOCK(pre_node->leaf.basic_block);
	head_bb = LLVM2SI_BASIC_BLOCK(head_node->leaf.basic_block);
	tail_bb = LLVM2SI_BASIC_BLOCK(tail_node->leaf.basic_block);
	exit_bb = LLVM2SI_BASIC_BLOCK(exit_node->leaf.basic_block);
	assert(!pre_bb);
	assert(head_bb);
	assert(tail_bb);
	assert(!exit_bb);

	/* Create pre-header and exit basic blocks */
	pre_bb = llvm2si_basic_block_create(function, pre_node);
	exit_bb = llvm2si_basic_block_create(function, exit_node);


	/*** Code for pre-header block ***/

	/* Allocate two scalar registers to push the active mask */
	tos_sreg = llvm2si_function_alloc_sreg(function, 2, 2);

	/* Push active mask.
	 * s_mov_b64 <tos_sreg>, exec
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	llvm2si_basic_block_add_inst(pre_bb, inst);


	/*** Code for exit block ***/

	/* Pop active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	llvm2si_basic_block_add_inst(exit_bb, inst);


	/*** Code for tail block ***/

	/* Jump to head block.
	 * s_branch <head_block>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_label(head_node->name));
	inst = si2bin_inst_create(SI_INST_S_BRANCH, arg_list);
	llvm2si_basic_block_add_inst(tail_bb, inst);


	/*** Code for head block ***/

	/* Get head block terminator */
	llbb = head_node->llbb;
	llinst = LLVMGetBasicBlockTerminator(llbb);
	assert(llinst);
	assert(LLVMGetInstructionOpcode(llinst) == LLVMBr);
	assert(LLVMGetNumOperands(llinst) == 3);

	/* Get symbol associated with condition variable */
	llcond = LLVMGetOperand(llinst, 0);
	cond_name = (char *) LLVMGetValueName(llcond);
	cond_symbol = llvm2si_symbol_table_lookup(function->symbol_table, cond_name);
	assert(cond_symbol);
	assert(cond_symbol->type == llvm2si_symbol_scalar_register);
	assert(cond_symbol->count == 2);
	cond_sreg = cond_symbol->reg;

	/* Obtain opcode depending on whether the loop exists when the head's
	 * condition is true or it does when it is false. */
	opcode = SI_INST_S_AND_B64;
	assert(head_node->exit_if_true ^ head_node->exit_if_false);
	if (head_node->exit_if_true)
		opcode = SI_INST_S_ANDN2_B64;

	/* Bitwise 'and' of active mask with condition.
	 * s_and(n2)_b64 exec, exec, <cond_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(cond_sreg, cond_sreg + 1));
	inst = si2bin_inst_create(opcode, arg_list);
	llvm2si_basic_block_add_inst(head_bb, inst);

	/* Exit loop if no more work-items are active.
	 * s_cbranch_execz <exit_node>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_label(exit_node->name));
	inst = si2bin_inst_create(SI_INST_S_CBRANCH_EXECZ, arg_list);
	llvm2si_basic_block_add_inst(head_bb, inst);
}


void llvm2si_function_emit_control_flow(struct llvm2si_function_t *function)
{
	struct linked_list_t *node_list;
	struct cnode_t *node;

	/* Emit control flow actions using a post-order traversal of the syntax
	 * tree (not control-flow graph), from inner to outer control flow
	 * structures. Which specific post-order traversal does not matter. */
	node_list = linked_list_create();
	ctree_traverse(function->ctree, NULL, node_list);

	/* Traverse nodes */
	LINKED_LIST_FOR_EACH(node_list)
	{
		/* Ignore leaf nodes */
		node = linked_list_get(node_list);
		if (node->kind == cnode_leaf)
			continue;

		/* Check control structure */
		switch (node->abstract.region)
		{

		case cnode_region_block:

			/* Ignore blocks */
			break;

		case cnode_region_if_then:

			llvm2si_function_emit_if_then(function, node);
			break;

		case cnode_region_if_then_else:

			llvm2si_function_emit_if_then_else(function, node);
			break;

		case cnode_region_while_loop:

			llvm2si_function_emit_while_loop(function, node);
			break;

		default:
			fatal("%s: region %s not supported", __FUNCTION__,
					str_map_value(&cnode_region_map,
					node->abstract.region));
		}
	}

	/* Free structures */
	linked_list_free(node_list);
}


static struct si2bin_arg_t *llvm2si_function_translate_const_value(
		struct llvm2si_function_t *function,
		LLVMValueRef llvalue)
{
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	lltype = LLVMTypeOf(llvalue);
	lltype_kind = LLVMGetTypeKind(lltype);
	
	/* Check constant type */
	switch (lltype_kind)
	{
	
	case LLVMIntegerTypeKind:
	{
		int bit_width;
		int value;

		/* Only 32-bit constants supported for now. We need to figure
		 * out what to do with the sign extension otherwise. */
		bit_width = LLVMGetIntTypeWidth(lltype);
		if (bit_width != 32)
			fatal("%s: only 32-bit integer constant supported "
				" (%d-bit found)", __FUNCTION__, bit_width);

		/* Create argument */
		value = LLVMConstIntGetZExtValue(llvalue);
		return si2bin_arg_create_literal(value);
	}

	default:
		
		fatal("%s: constant type not supported (%d)",
			__FUNCTION__, lltype_kind);
		return NULL;
	}
}


struct si2bin_arg_t *llvm2si_function_translate_value(
		struct llvm2si_function_t *function,
		LLVMValueRef llvalue,
		struct llvm2si_symbol_t **symbol_ptr)
{
	struct llvm2si_symbol_t *symbol;
	struct si2bin_arg_t *arg;

	char *name;

	/* Returned symbol is NULL by default */
	PTR_ASSIGN(symbol_ptr, NULL);

	/* Treat constants separately */
	if (LLVMIsConstant(llvalue))
		return llvm2si_function_translate_const_value(function, llvalue);

	/* Get name */
	name = (char *) LLVMGetValueName(llvalue);
	if (!name || !*name)
		fatal("%s: anonymous values not supported", __FUNCTION__);

	/* Look up symbol */
	symbol = llvm2si_symbol_table_lookup(function->symbol_table, name);
	if (!symbol)
		fatal("%s: %s: symbol not found", __FUNCTION__, name);

	/* Create argument based on symbol type */
	switch (symbol->type)
	{

	case llvm2si_symbol_vector_register:

		arg = si2bin_arg_create_vector_register(symbol->reg);
		break;

	case llvm2si_symbol_scalar_register:

		arg = si2bin_arg_create_scalar_register(symbol->reg);
		break;

	default:

		arg = NULL;
		fatal("%s: invalid symbol type (%d)", __FUNCTION__,
				symbol->type);
	}
	
	/* Return argument and symbol */
	PTR_ASSIGN(symbol_ptr, symbol);
	return arg;
}


struct si2bin_arg_t *llvm2si_function_const_to_vreg(
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block,
		struct si2bin_arg_t *arg)
{
	struct si2bin_arg_t *ret_arg;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;
	int vreg;

	/* If argument is not a literal of any kind, don't convert. */
	if (!SI2BIN_ARG_IS_CONSTANT(arg))
		return arg;

	/* Allocate vector register */
	vreg = llvm2si_function_alloc_vreg(function, 1, 1);
	ret_arg = si2bin_arg_create_vector_register(vreg);

	/* Copy constant to vector register.
	 * v_mov_b32 <vreg>, <const>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(vreg));
	list_add(arg_list, arg);
	inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);

	/* Return new argument */
	return ret_arg;
}


int llvm2si_function_alloc_sreg(struct llvm2si_function_t *function,
		int count, int align)
{
	function->num_sregs = (function->num_sregs + align - 1)
			/ align * align;
	function->num_sregs += count;
	return function->num_sregs - count;
}


int llvm2si_function_alloc_vreg(struct llvm2si_function_t *function,
		int count, int align)
{
	function->num_vregs = (function->num_vregs + align - 1)
			/ align * align;
	function->num_vregs += count;
	return function->num_vregs - count;
}


