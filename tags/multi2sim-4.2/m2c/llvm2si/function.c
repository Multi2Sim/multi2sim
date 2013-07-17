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


#include <m2c/common/ctree.h>
#include <m2c/si2bin/arg.h>
#include <m2c/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "basic-block.h"
#include "function.h"
#include "phi.h"
#include "symbol.h"
#include "symbol-table.h"


/*
 * Class 'Llvm2siFunctionArg'
 */

CLASS_IMPLEMENTATION(Llvm2siFunctionArg);

/* Return a Southern Islands argument type from an LLVM type. */
static enum si_arg_data_type_t Llvm2siFunctionArgGetDataType(LLVMTypeRef lltype)
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


void Llvm2siFunctionArgCreate(Llvm2siFunctionArg *self, LLVMValueRef llarg)
{
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
		si_arg->pointer.data_type = Llvm2siFunctionArgGetDataType(lltype);
	}
	else
	{
		si_arg = si_arg_create(si_arg_value, name);
		si_arg->value.data_type = Llvm2siFunctionArgGetDataType(lltype);
	}

	/* Initialize */
	self->name = str_set(self->name, name);
	self->llarg = llarg;
	self->si_arg = si_arg;
}


void Llvm2siFunctionArgDestroy(Llvm2siFunctionArg *self)
{
	assert(self->name);
	assert(self->si_arg);
	self->name = str_free(self->name);
	si_arg_free(self->si_arg);
}



void Llvm2siFunctionArgDump(Object *self, FILE *f)
{
	Llvm2siFunctionArg *arg = asLlvm2siFunctionArg(self);
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
 * Class 'Llvm2siFunctionUAV'
 */

CLASS_IMPLEMENTATION(Llvm2siFunctionUAV);


void Llvm2siFunctionUAVCreate(Llvm2siFunctionUAV *self)
{
}


void Llvm2siFunctionUAVDestroy(Llvm2siFunctionUAV *self)
{
}




/*
 * Private Functions
 */

/* Add a UAV to the UAV list. This function allocates a series of 4 aligned
 * scalar registers to the UAV, populating its 'index' and 'sreg' fields.
 * The UAV object will be freed automatically after calling this function.
 * Emit the code needed to load UAV into 'function->basic_block_uavs' */
static void Llvm2siFunctionAddUAV(Llvm2siFunction *self,
		Llvm2siFunctionUAV *uav)
{
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;
	Llvm2siBasicBlock *basic_block;

	/* Associate UAV with self */
	assert(!uav->function);
	uav->function = self;

	/* Get basic block or create it */
	basic_block = asLlvm2siBasicBlock(self->uavs_node->basic_block);
	if (!basic_block)
		basic_block = new(Llvm2siBasicBlock, self,
				self->uavs_node);

	/* Allocate 4 aligned scalar registers */
	uav->sreg = Llvm2siFunctionAllocSReg(self, 4, 4);

	/* Insert to UAV list */
	uav->index = self->uav_list->count;
	list_add(self->uav_list, uav);

	/* Emit code to load UAV.
	 * s_load_dwordx4 s[uavX:uavX+3], s[uav_table:uav_table+1], x * 8
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			uav->sreg, uav->sreg + 3));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			self->sreg_uav_table, self->sreg_uav_table + 1));
	list_add(arg_list, si2bin_arg_create_literal((uav->index + 10) * 8));
	inst = si2bin_inst_create(SI_INST_S_LOAD_DWORDX4, arg_list);
	Llvm2siBasicBlockAddInst(basic_block, inst);
}


/* Add argument 'arg' into the list of arguments of 'function', and emit code
 * to load it into 'function->basic_block_args'. */
static void Llvm2siFunctionAddArg(Llvm2siFunction *self,
		Llvm2siFunctionArg *arg)
{
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;
	Llvm2siSymbol *symbol;
	Llvm2siBasicBlock *basic_block;
	Llvm2siFunctionUAV *uav;

	/* Check that argument does not belong to a self yet */
	if (arg->function)
		panic("%s: argument already added", __FUNCTION__);

	/* Get basic block, or create it */
	basic_block = asLlvm2siBasicBlock(self->args_node->basic_block);
	if (!basic_block)
		basic_block = new(Llvm2siBasicBlock, self, self->args_node);

	/* Add argument */
	list_add(self->arg_list, arg);
	arg->function = self;
	arg->index = self->arg_list->count - 1;

	/* Allocate 1 scalar and 1 vector register for the argument */
	arg->sreg = Llvm2siFunctionAllocSReg(self, 1, 1);
	arg->vreg = Llvm2siFunctionAllocVReg(self, 1, 1);

	/* Generate code to load argument into a scalar register.
	 * s_buffer_load_dword s[arg], s[cb1:cb1+3], idx*4
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register(arg->sreg));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(self->sreg_cb1,
			self->sreg_cb1 + 3));
	list_add(arg_list, si2bin_arg_create_literal(arg->index * 4));
	inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
	Llvm2siBasicBlockAddInst(basic_block, inst);

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
	Llvm2siBasicBlockAddInst(basic_block, inst);

	/* Insert argument name in symbol table, using its vector register. */
	symbol = new_ctor(Llvm2siSymbol, CreateVReg, arg->name, arg->vreg);
	Llvm2siSymbolTableAddSymbol(self->symbol_table, symbol);

	/* If argument is an object in global memory, create a UAV
	 * associated with it. */
	if (arg->si_arg->type == si_arg_pointer &&
			arg->si_arg->pointer.scope == si_arg_uav)
	{
		/* New UAV */
		uav = new(Llvm2siFunctionUAV);
		Llvm2siFunctionAddUAV(self, uav);

		/* Store UAV index in argument and symbol */
		Llvm2siSymbolSetUAVIndex(symbol, uav->index);
		arg->uav_index = uav->index;
	}
}


static void Llvm2siFunctionDumpData(Llvm2siFunction *self, FILE *f)
{
	/* Section header */
	fprintf(f, ".metadata\n");

	/* User elements */
	fprintf(f, "\tuserElements[0] = PTR_UAV_TABLE, 0, s[%d:%d]\n",
			self->sreg_uav_table, self->sreg_uav_table + 1);
	fprintf(f, "\tuserElements[1] = IMM_CONST_BUFFER, 0, s[%d:%d]\n",
			self->sreg_cb0, self->sreg_cb0 + 3);
	fprintf(f, "\tuserElements[2] = IMM_CONST_BUFFER, 1, s[%d:%d]\n",
			self->sreg_cb1, self->sreg_cb1 + 3);
	fprintf(f, "\n");

	/* Floating-point mode */
	fprintf(f, "\tFloatMode = 192\n");
	fprintf(f, "\tIeeeMode = 0\n");
	fprintf(f, "\n");

	/* Program resources */
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:USER_SGPR = %d\n", self->sreg_wgid);
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:TGID_X_EN = %d\n", 1);
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:TGID_Y_EN = %d\n", 1);
	fprintf(f, "\tCOMPUTE_PGM_RSRC2:TGID_Z_EN = %d\n", 1);
	fprintf(f, "\n");
}




/*
 * Public Functions
 */

CLASS_IMPLEMENTATION(Llvm2siFunction);

void Llvm2siFunctionCreate(Llvm2siFunction *self, LLVMValueRef llfunction)
{
	CTree *ctree;

	/* Initialize */
	self->llfunction = llfunction;
	self->name = str_set(self->name, (char *) LLVMGetValueName(llfunction));
	self->arg_list = list_create();
	self->uav_list = list_create();
	self->symbol_table = new(Llvm2siSymbolTable);
	self->ctree = ctree = new(CTree, self->name);
	self->phi_list = linked_list_create();

	/* Create pre-defined nodes in control tree */
	self->header_node = new(LeafNode, "header");
	self->uavs_node = new(LeafNode, "uavs");
	self->args_node = new(LeafNode, "args");
	CTreeAddNode(ctree, asNode(self->header_node));
	CTreeAddNode(ctree, asNode(self->uavs_node));
	CTreeAddNode(ctree, asNode(self->args_node));
	ctree->entry_node = asNode(self->header_node);

	/* Add all nodes from the LLVM control flow graph */
	self->body_node = CTreeAddLlvmCFG(ctree, llfunction);

	/* Connect nodes */
	NodeConnect(asNode(self->header_node), asNode(self->uavs_node));
	NodeConnect(asNode(self->uavs_node), asNode(self->args_node));
	NodeConnect(asNode(self->args_node), asNode(self->body_node));
}


void Llvm2siFunctionDestroy(Llvm2siFunction *self)
{
	int index;

	/* Free list of arguments */
	LIST_FOR_EACH(self->arg_list, index)
		delete(asLlvm2siFunctionArg(list_get(self->arg_list, index)));
	list_free(self->arg_list);

	/* Free list of UAVs */
	LIST_FOR_EACH(self->uav_list, index)
		delete(asLlvm2siFunctionUAV(list_get(self->uav_list, index)));
	list_free(self->uav_list);

	/* Free control tree */
	if (self->ctree)
		delete(self->ctree);

	/* Free list of 'phi' entries */
	assert(!self->phi_list->count);
	linked_list_free(self->phi_list);

	/* Rest */
	delete(self->symbol_table);
	self->name = str_free(self->name);
}


void Llvm2siFunctionDump(Object *self, FILE *f)
{
	Llvm2siBasicBlock *basic_block;
	Llvm2siFunction *function;
	Node *node;

	Llvm2siFunctionArg *function_arg;
	struct linked_list_t *node_list;

	int index;

	/* Function name */
	function = asLlvm2siFunction(self);
	fprintf(f, ".global %s\n\n", function->name);

	/* Arguments */
	fprintf(f, ".args\n");
	LIST_FOR_EACH(function->arg_list, index)
	{
		function_arg = list_get(function->arg_list, index);
		Llvm2siFunctionArgDump(asObject(function_arg), f);
	}
	fprintf(f, "\n");

	/* Dump basic blocks */
	fprintf(f, ".text\n");
	node_list = linked_list_create();
	CTreeTraverse(function->ctree, node_list, NULL);
	LINKED_LIST_FOR_EACH(node_list)
	{
		/* Skip abstract nodes */
		node = linked_list_get(node_list);
		if (!isLeafNode(node))
			continue;

		/* Get node's basic block */
		basic_block = asLlvm2siBasicBlock(asLeafNode(node)->basic_block);
		if (!basic_block)
			continue;

		/* Dump code of basic block */
		Llvm2siBasicBlockDump(asObject(basic_block), f);
	}
	linked_list_free(node_list);
	fprintf(f, "\n");

	/* Dump section '.data' */
	Llvm2siFunctionDumpData(function, f);
	fprintf(f, "\n");
}


void Llvm2siFunctionEmitHeader(Llvm2siFunction *self)
{
	Llvm2siBasicBlock *basic_block;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	char comment[MAX_STRING_SIZE];
	int index;

	/* Create header basic block */
	basic_block = new(Llvm2siBasicBlock, self, self->header_node);

	/* Function must be empty at this point */
	assert(!self->num_sregs);
	assert(!self->num_vregs);

	/* Allocate 3 vector registers (v[0:2]) for local ID */
	self->vreg_lid = Llvm2siFunctionAllocVReg(self, 3, 1);
	if (self->vreg_lid)
		panic("%s: vreg_lid is expented to be 0", __FUNCTION__);

	/* Allocate 2 scalar registers for UAV table. The value for these
	 * registers is assigned by the runtime based on info found in the
	 * 'userElements' metadata of the binary.*/
	self->sreg_uav_table = Llvm2siFunctionAllocSReg(self, 2, 1);

	/* Allocate 4 scalar registers for CB0, and 4 more for CB1. The
	 * values for these registers will be assigned by the runtime based
	 * on info present in the 'userElements' metadata. */
	self->sreg_cb0 = Llvm2siFunctionAllocSReg(self, 4, 1);
	self->sreg_cb1 = Llvm2siFunctionAllocSReg(self, 4, 1);

	/* Allocate 3 scalar registers for the work-group ID. The content of
	 * these register will be populated by the runtime based on info found
	 * in COMPUTE_PGM_RSRC2 metadata. */
	self->sreg_wgid = Llvm2siFunctionAllocSReg(self, 3, 1);

	/* Obtain global size in s[gsize:gsize+2].
	 * s_buffer_load_dword s[gsize], s[cb0:cb0+3], 0x00
	 * s_buffer_load_dword s[gsize+1], s[cb0:cb0+3], 0x01
	 * s_buffer_load_dword s[gsize+2], s[cb0:cb0+3], 0x02
	 */
	Llvm2siBasicBlockAddComment(basic_block, "Obtain global size");
	self->sreg_gsize = Llvm2siFunctionAllocSReg(self, 3, 1);
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				self->sreg_gsize + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				self->sreg_cb0, self->sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);
	}

	/* Obtain local size in s[lsize:lsize+2].
	 *
	 * s_buffer_load_dword s[lsize], s[cb0:cb0+3], 0x04
	 * s_buffer_load_dword s[lsize+1], s[cb0:cb0+3], 0x05
	 * s_buffer_load_dword s[lsize+2], s[cb0:cb0+3], 0x06
	 */
	Llvm2siBasicBlockAddComment(basic_block, "Obtain local size");
	self->sreg_lsize = Llvm2siFunctionAllocSReg(self, 3, 1);
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				self->sreg_lsize + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				self->sreg_cb0, self->sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(4 + index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);
	}

	/* Obtain global offset in s[offs:offs+2].
	 *
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x18
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x19
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x1a
	 */
	Llvm2siBasicBlockAddComment(basic_block, "Obtain global offset");
	self->sreg_offs = Llvm2siFunctionAllocSReg(self, 3, 1);
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				self->sreg_offs + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				self->sreg_cb0, self->sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(0x18 + index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);
	}

	/* Calculate global ID in dimensions [0:2] and store it in v[3:5].
	 *
	 * v_mov_b32 v[gid+dim], s[lsize+dim]
	 * v_mul_i32_i24 v[gid+dim], s[wgid+dim], v[gid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], v[lid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], s[offs+dim]
	 */
	self->vreg_gid = Llvm2siFunctionAllocVReg(self, 3, 1);
	for (index = 0; index < 3; index++)
	{
		/* Comment */
		snprintf(comment, sizeof comment, "Calculate global ID "
				"in dimension %d", index);
		Llvm2siBasicBlockAddComment(basic_block, comment);

		/* v_mov_b32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				self->sreg_lsize + index));
		inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);

		/* v_mul_i32_i24 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				self->sreg_wgid + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_gid + index));
		inst = si2bin_inst_create(SI_INST_V_MUL_I32_I24, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);

		/* v_add_i32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_lid + index));
		inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);

		/* v_add_i32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				self->sreg_offs + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				self->vreg_gid + index));
		inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);
	}
}


void Llvm2siFunctionEmitArgs(Llvm2siFunction *self)
{
	LLVMValueRef llfunction;
	LLVMValueRef llarg;

	Llvm2siFunctionArg *arg;

	/* Emit code for each argument individually */
	llfunction = self->llfunction;
	for (llarg = LLVMGetFirstParam(llfunction); llarg;
			llarg = LLVMGetNextParam(llarg))
	{
		/* Create function argument and add it */
		arg = new(Llvm2siFunctionArg, llarg);

		/* Add the argument to the list. This call will cause the
		 * corresponding code to be emitted. */
		Llvm2siFunctionAddArg(self, arg);
	}
}


void Llvm2siFunctionEmitBody(Llvm2siFunction *self)
{
	Llvm2siBasicBlock *basic_block;
	struct linked_list_t *node_list;

	CTree *ctree;
	Node *node;

	/* Code for the function body must be emitted using a depth-first
	 * traversal of the control tree. For this, we need right here the
	 * structural analysis that produces the control tree from the
	 * control flow graph.
	 */
	ctree = self->ctree;
	assert(!ctree->structural_analysis_done);
	CTreeStructuralAnalysis(ctree);

	/* Whether we use a pre- or a post-order traversal does not matter,
	 * since we are only considering the leaf nodes.
	 */
	node_list = linked_list_create();
	CTreeTraverse(ctree, node_list, NULL);

	/* Emit code for basic blocks */
	LINKED_LIST_FOR_EACH(node_list)
	{
		/* Skip abstract nodes */
		node = linked_list_get(node_list);
		if (!isLeafNode(node))
			continue;

		/* Skip nodes with no LLVM code to translate */
		if (!asLeafNode(node)->llbb)
			continue;

		/* Create basic block and emit the code */
		assert(!asLeafNode(node)->basic_block);
		basic_block = new(Llvm2siBasicBlock, self, asLeafNode(node));
		Llvm2siBasicBlockEmit(basic_block, asLeafNode(node)->llbb);
	}

	/* Free structures */
	linked_list_free(node_list);
}


void Llvm2siFunctionEmitPhi(Llvm2siFunction *self)
{
	LeafNode *node;
	Llvm2siPhi *phi;
	Llvm2siBasicBlock *basic_block;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *src_value;

	while (self->phi_list->count)
	{
		/* Extract element from list */
		linked_list_head(self->phi_list);
		phi = linked_list_remove(self->phi_list);

		/* Get basic block */
		node = asLeafNode(phi->src_node);
		basic_block = asLlvm2siBasicBlock(node->basic_block);
		assert(basic_block);

		/* Get source value */
		src_value = Llvm2siFunctionTranslateValue(self,
				phi->src_value, NULL);

		/* Copy source value to destination value.
		 * s_mov_b32 <dest_value>, <src_value>
		 */
		arg_list = list_create();
		list_add(arg_list, phi->dest_value);
		list_add(arg_list, src_value);
		inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
		Llvm2siBasicBlockAddInst(basic_block, inst);

		/* Free phi object */
		delete(phi);
	}
}


static void Llvm2siFunctionEmitIfThen(Llvm2siFunction *self, Node *node)
{
	Node *if_node;
	Node *then_node;

	Llvm2siBasicBlock *if_bb;
	Llvm2siBasicBlock *then_bb;

	Llvm2siSymbol *cond_symbol;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	LLVMBasicBlockRef llbb;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;

	/* Identify the two nodes */
	assert(isAbstractNode(node));
	assert(asAbstractNode(node)->region == AbstractNodeIfThen);
	assert(asAbstractNode(node)->child_list->count == 2);
	if_node = linked_list_goto(asAbstractNode(node)->child_list, 0);
	then_node = linked_list_goto(asAbstractNode(node)->child_list, 1);

	/* Make sure roles match */
	assert(if_node->role == node_role_if);
	assert(then_node->role == node_role_then);

	/* Get basic blocks. 'If' node should be a leaf. */
	then_node = NodeGetLastLeaf(then_node);
	assert(isLeafNode(if_node));
	assert(isLeafNode(then_node));
	if_bb = asLlvm2siBasicBlock(asLeafNode(if_node)->basic_block);
	then_bb = asLlvm2siBasicBlock(asLeafNode(then_node)->basic_block);


	/*** Code for 'If' block ***/

	/* Get 'If' basic block terminator */
	llbb = asLeafNode(if_node)->llbb;
	llinst = LLVMGetBasicBlockTerminator(llbb);
	assert(llinst);
	assert(LLVMGetInstructionOpcode(llinst) == LLVMBr);
	assert(LLVMGetNumOperands(llinst) == 3);

	/* Get symbol associated with condition variable */
	llcond = LLVMGetOperand(llinst, 0);
	cond_name = (char *) LLVMGetValueName(llcond);
	cond_symbol = Llvm2siSymbolTableLookup(self->symbol_table, cond_name);
	assert(cond_symbol);
	assert(cond_symbol->type == llvm2si_symbol_scalar_register);
	assert(cond_symbol->count == 2);
	cond_sreg = cond_symbol->reg;

	/* Allocate two scalar registers to push the active mask */
	tos_sreg = Llvm2siFunctionAllocSReg(self, 2, 2);

	/* Emit active mask push and set at the end of the 'If' block.
	 * s_and_saveexec_b64 <tos_sreg> <cond_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(cond_sreg, cond_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_AND_SAVEEXEC_B64, arg_list);
	Llvm2siBasicBlockAddInst(if_bb, inst);


	/*** Code for 'then' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	Llvm2siBasicBlockAddInst(then_bb, inst);
}


static void Llvm2siFunctionEmitIfThenElse(Llvm2siFunction *self, Node *node)
{
	Node *if_node;
	Node *then_node;
	Node *else_node;

	Llvm2siBasicBlock *if_bb;
	Llvm2siBasicBlock *then_bb;
	Llvm2siBasicBlock *else_bb;

	Llvm2siSymbol *cond_symbol;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	LLVMBasicBlockRef llbb;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;

	/* Identify the three nodes */
	assert(isAbstractNode(node));
	assert(asAbstractNode(node)->region == AbstractNodeIfThenElse);
	assert(asAbstractNode(node)->child_list->count == 3);
	if_node = linked_list_goto(asAbstractNode(node)->child_list, 0);
	then_node = linked_list_goto(asAbstractNode(node)->child_list, 1);
	else_node = linked_list_goto(asAbstractNode(node)->child_list, 2);

	/* Make sure roles match */
	assert(if_node->role == node_role_if);
	assert(then_node->role == node_role_then);
	assert(else_node->role == node_role_else);

	/* Get basic blocks. 'If' node should be a leaf. */
	then_node = NodeGetLastLeaf(then_node);
	else_node = NodeGetLastLeaf(else_node);
	assert(isLeafNode(if_node));
	assert(isLeafNode(then_node));
	assert(isLeafNode(else_node));
	if_bb = asLlvm2siBasicBlock(asLeafNode(if_node)->basic_block);
	then_bb = asLlvm2siBasicBlock(asLeafNode(then_node)->basic_block);
	else_bb = asLlvm2siBasicBlock(asLeafNode(else_node)->basic_block);


	/*** Code for 'If' block ***/

	/* Get 'If' basic block terminator */
	llbb = asLeafNode(if_node)->llbb;
	llinst = LLVMGetBasicBlockTerminator(llbb);
	assert(llinst);
	assert(LLVMGetInstructionOpcode(llinst) == LLVMBr);
	assert(LLVMGetNumOperands(llinst) == 3);

	/* Get symbol associated with condition variable */
	llcond = LLVMGetOperand(llinst, 0);
	cond_name = (char *) LLVMGetValueName(llcond);
	cond_symbol = Llvm2siSymbolTableLookup(self->symbol_table, cond_name);
	assert(cond_symbol);
	assert(cond_symbol->type == llvm2si_symbol_scalar_register);
	assert(cond_symbol->count == 2);
	cond_sreg = cond_symbol->reg;

	/* Allocate two scalar registers to push the active mask */
	tos_sreg = Llvm2siFunctionAllocSReg(self, 2, 2);

	/* Emit active mask push and set at the end of the 'If' block.
	 * s_and_saveexec_b64 <tos_sreg> <cond_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(cond_sreg, cond_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_AND_SAVEEXEC_B64, arg_list);
	Llvm2siBasicBlockAddInst(if_bb, inst);


	/*** Code for 'then' block ***/

	/* Invert active mask and-ing it with the top of the stack.
	 * s_andn2_b64 exec, <tos_sreg>, exec
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	inst = si2bin_inst_create(SI_INST_S_ANDN2_B64, arg_list);
	Llvm2siBasicBlockAddInst(then_bb, inst);


	/*** Code for 'else' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	Llvm2siBasicBlockAddInst(else_bb, inst);
}


static void Llvm2siFunctionEmitWhileLoop(Llvm2siFunction *self, Node *node)
{
	Node *head_node;
	Node *tail_node;
	Node *pre_node;
	Node *exit_node;

	Llvm2siBasicBlock *head_bb;
	Llvm2siBasicBlock *tail_bb;
	Llvm2siBasicBlock *pre_bb;
	Llvm2siBasicBlock *exit_bb;

	Llvm2siSymbol *cond_symbol;
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
	assert(isAbstractNode(node));
	assert(asAbstractNode(node)->region == AbstractNodeWhileLoop);
	assert(asAbstractNode(node)->child_list->count == 4);
	pre_node = linked_list_goto(asAbstractNode(node)->child_list, 0);
	head_node = linked_list_goto(asAbstractNode(node)->child_list, 1);
	tail_node = linked_list_goto(asAbstractNode(node)->child_list, 2);
	exit_node = linked_list_goto(asAbstractNode(node)->child_list, 3);

	/* Make sure roles match */
	assert(pre_node->role == node_role_pre);
	assert(head_node->role == node_role_head);
	assert(tail_node->role == node_role_tail);
	assert(exit_node->role == node_role_exit);

	/* Get basic blocks. Pre-header/head/exit nodes should be a leaves.
	 * Tail node can be an abstract node, which we need to track down to
	 * its last leaf to introduce control flow at the end.
	 *
	 * Basic blocks must exist associated to the head and the tail blocks:
	 * they come from LLVM blocks already emitted. But pre-header and exit
	 * blocks have been inserted during the structural analysis, so they
	 * contain no basic block yet.
	 */
	tail_node = NodeGetLastLeaf(tail_node);
	assert(isLeafNode(pre_node));
	assert(isLeafNode(head_node));
	assert(isLeafNode(tail_node));
	assert(isLeafNode(exit_node));
	pre_bb = asLlvm2siBasicBlock(asLeafNode(pre_node)->basic_block);
	head_bb = asLlvm2siBasicBlock(asLeafNode(head_node)->basic_block);
	tail_bb = asLlvm2siBasicBlock(asLeafNode(tail_node)->basic_block);
	exit_bb = asLlvm2siBasicBlock(asLeafNode(exit_node)->basic_block);
	assert(!pre_bb);
	assert(head_bb);
	assert(tail_bb);
	assert(!exit_bb);

	/* Create pre-header and exit basic blocks */
	pre_bb = new(Llvm2siBasicBlock, self, asLeafNode(pre_node));
	exit_bb = new(Llvm2siBasicBlock, self, asLeafNode(exit_node));


	/*** Code for pre-header block ***/

	/* Allocate two scalar registers to push the active mask */
	tos_sreg = Llvm2siFunctionAllocSReg(self, 2, 2);

	/* Push active mask.
	 * s_mov_b64 <tos_sreg>, exec
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	Llvm2siBasicBlockAddInst(pre_bb, inst);


	/*** Code for exit block ***/

	/* Pop active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_exec));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(tos_sreg, tos_sreg + 1));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	Llvm2siBasicBlockAddInst(exit_bb, inst);


	/*** Code for tail block ***/

	/* Jump to head block.
	 * s_branch <head_block>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_label(head_node->name));
	inst = si2bin_inst_create(SI_INST_S_BRANCH, arg_list);
	Llvm2siBasicBlockAddInst(tail_bb, inst);


	/*** Code for head block ***/

	/* Get head block terminator */
	llbb = asLeafNode(head_node)->llbb;
	llinst = LLVMGetBasicBlockTerminator(llbb);
	assert(llinst);
	assert(LLVMGetInstructionOpcode(llinst) == LLVMBr);
	assert(LLVMGetNumOperands(llinst) == 3);

	/* Get symbol associated with condition variable */
	llcond = LLVMGetOperand(llinst, 0);
	cond_name = (char *) LLVMGetValueName(llcond);
	cond_symbol = Llvm2siSymbolTableLookup(self->symbol_table, cond_name);
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
	Llvm2siBasicBlockAddInst(head_bb, inst);

	/* Exit loop if no more work-items are active.
	 * s_cbranch_execz <exit_node>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_label(exit_node->name));
	inst = si2bin_inst_create(SI_INST_S_CBRANCH_EXECZ, arg_list);
	Llvm2siBasicBlockAddInst(head_bb, inst);
}


void Llvm2siFunctionEmitControlFlow(Llvm2siFunction *self)
{
	struct linked_list_t *node_list;
	Node *node;

	/* Emit control flow actions using a post-order traversal of the syntax
	 * tree (not control-flow graph), from inner to outer control flow
	 * structures. Which specific post-order traversal does not matter. */
	node_list = linked_list_create();
	CTreeTraverse(self->ctree, NULL, node_list);

	/* Traverse nodes */
	LINKED_LIST_FOR_EACH(node_list)
	{
		/* Ignore leaf nodes */
		node = linked_list_get(node_list);
		if (isLeafNode(node))
			continue;

		/* Check control structure */
		switch (asAbstractNode(node)->region)
		{

		case AbstractNodeBlock:

			/* Ignore blocks */
			break;

		case AbstractNodeIfThen:

			Llvm2siFunctionEmitIfThen(self, node);
			break;

		case AbstractNodeIfThenElse:

			Llvm2siFunctionEmitIfThenElse(self, node);
			break;

		case AbstractNodeWhileLoop:

			Llvm2siFunctionEmitWhileLoop(self, node);
			break;

		default:
			fatal("%s: region %s not supported", __FUNCTION__,
					str_map_value(&abstract_node_region_map,
					asAbstractNode(node)->region));
		}
	}

	/* Free structures */
	linked_list_free(node_list);
}


static struct si2bin_arg_t *Llvm2siFunctionTranslateConstValue(
		Llvm2siFunction *self, LLVMValueRef llvalue)
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


struct si2bin_arg_t *Llvm2siFunctionTranslateValue(Llvm2siFunction *self,
		LLVMValueRef llvalue, Llvm2siSymbol **symbol_ptr)
{
	Llvm2siSymbol *symbol;
	struct si2bin_arg_t *arg;

	char *name;

	/* Returned symbol is NULL by default */
	PTR_ASSIGN(symbol_ptr, NULL);

	/* Treat constants separately */
	if (LLVMIsConstant(llvalue))
		return Llvm2siFunctionTranslateConstValue(self, llvalue);

	/* Get name */
	name = (char *) LLVMGetValueName(llvalue);
	if (!name || !*name)
		fatal("%s: anonymous values not supported", __FUNCTION__);

	/* Look up symbol */
	symbol = Llvm2siSymbolTableLookup(self->symbol_table, name);
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


struct si2bin_arg_t *Llvm2siFunctionConstToVReg(Llvm2siFunction *self,
		Llvm2siBasicBlock *basic_block, struct si2bin_arg_t *arg)
{
	struct si2bin_arg_t *ret_arg;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;
	int vreg;

	/* If argument is not a literal of any kind, don't convert. */
	if (!SI2BIN_ARG_IS_CONSTANT(arg))
		return arg;

	/* Allocate vector register */
	vreg = Llvm2siFunctionAllocVReg(self, 1, 1);
	ret_arg = si2bin_arg_create_vector_register(vreg);

	/* Copy constant to vector register.
	 * v_mov_b32 <vreg>, <const>
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(vreg));
	list_add(arg_list, arg);
	inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
	Llvm2siBasicBlockAddInst(basic_block, inst);

	/* Return new argument */
	return ret_arg;
}


int Llvm2siFunctionAllocSReg(Llvm2siFunction *self, int count, int align)
{
	self->num_sregs = (self->num_sregs + align - 1)
			/ align * align;
	self->num_sregs += count;
	return self->num_sregs - count;
}


int Llvm2siFunctionAllocVReg(Llvm2siFunction *self, int count, int align)
{
	self->num_vregs = (self->num_vregs + align - 1)
			/ align * align;
	self->num_vregs += count;
	return self->num_vregs - count;
}
