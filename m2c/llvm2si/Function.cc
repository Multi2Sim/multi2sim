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

#include <llvm/Function.h>

#include "BasicBlock.h"
#include "Function.h"


using namespace si2bin;

namespace llvm2si
{


/*
 * Class 'FunctionArg'
 */

SI::ArgDataType FunctionArg::GetDataType(llvm::Type *llvm_type)
{
	/* Get vector element type */
	if (llvm_type->isVectorTy())
		llvm_type = llvm_type->getVectorElementType();

	/* Check types */
	if (llvm_type->isIntegerTy())
	{
		int bit_width = llvm_type->getIntegerBitWidth();
		switch (bit_width)
		{
		case 1: return SI::ArgDataTypeInt1;
		case 8: return SI::ArgDataTypeInt8;
		case 16: return SI::ArgDataTypeInt16;
		case 32: return SI::ArgDataTypeInt32;
		case 64: return SI::ArgDataTypeInt64;

		default:
			panic("%s: unsupported argument bit width (%d)",
				__FUNCTION__, bit_width);
		}
	}
	else if (llvm_type->isFloatTy())
	{
		return SI::ArgDataTypeFloat;
	}
	else
	{
		panic("%s: unsupported argument type kind (%d)",
				__FUNCTION__, llvm_type->getTypeID());
	}

	/* Unreachable */
	return SI::ArgDataTypeInvalid;
}


int FunctionArg::GetNumElements(llvm::Type *llvm_type)
{
	if (llvm_type->isVectorTy())
		return llvm_type->getVectorNumElements();
	else
		return 1;
}


FunctionArg::FunctionArg(llvm::Argument *llvm_arg)
{
	/* Get argument name */
	name = llvm_arg->getName();
	if (name.empty())
		fatal("%s: anonymous arguments not allowed", __FUNCTION__);

	/* Initialize */
	this->llvm_arg = llvm_arg;

	/* Initialize SI argument */
	llvm::Type *llvm_type = llvm_arg->getType();
	if (llvm_type->isPointerTy())
	{
		llvm_type = llvm_type->getPointerElementType();
		arg.reset(new SI::ArgPointer(name, GetDataType(llvm_type),
				GetNumElements(llvm_type), 0, 0, SI::ArgScopeUAV, 0, 0,
				SI::ArgAccessTypeReadWrite));
	}
	else
	{
		arg.reset(new SI::ArgValue(name, GetDataType(llvm_type),
				GetNumElements(llvm_type), 0, 0));
	}
}


void FunctionArg::Dump(std::ostream &os)
{
	switch (arg->GetType())
	{

	case SI::ArgTypePointer:
	{
		os << '\t' << *arg << ' ' << index * 16 << uav_index + 10 << '\n';
		break;
	}

	case SI::ArgTypeValue:
	{
		os << '\t' << *arg << ' ' << index * 16 << uav_index + 10 << '\n';
		break;
	}

	default:
		panic("%s: argument type not recognized (%d)",
				__FUNCTION__, arg->GetType());
	}
}


void Function::AddUAV(FunctionUAV *uav)
{
	/* Associate UAV with self */
	assert(!uav->function);
	uav->function = this;

	/* Get basic block or create it */
	BasicBlock *basic_block = dynamic_cast<BasicBlock *>
			(uavs_node->GetBasicBlock());
	if (!basic_block)
		basic_block = new llvm2si::BasicBlock(this, uavs_node);

	/* Allocate 4 aligned scalar registers */
	uav->sreg = AllocSReg(4, 4);

	/* Insert to UAV list */
	uav->index = uav_list.size();
	uav_list.emplace_back(uav);

	/* Emit code to load UAV.
	 * s_load_dwordx4 s[uavX:uavX+3], s[uav_table:uav_table+1], x * 8
	 */
	Inst *inst = new Inst(SI::INST_S_LOAD_DWORDX4,
			new ArgScalarRegisterSeries(uav->sreg, uav->sreg + 3),
			new ArgScalarRegisterSeries(sreg_uav_table, sreg_uav_table + 1),
			new ArgLiteral((uav->index + 10) * 8));
	basic_block->AddInst(inst);
}


void Function::AddArg(FunctionArg *arg, int num_elem)
{
	/* Check that argument does not belong to a self yet */
	if (arg->function)
		panic("%s: argument already added", __FUNCTION__);

	/* Get basic block, or create it */
	BasicBlock *basic_block = dynamic_cast<BasicBlock *>
			(args_node->GetBasicBlock());
	if (!basic_block)
		basic_block = new BasicBlock(this, args_node);

	/* Add argument */
	arg_list.emplace_back(arg);
	arg->function = this;
	arg->index = arg_list.size() - 1;

	/* Allocate 1 scalar and 1 vector register for the argument */
	arg->sreg = AllocSReg(num_elem, num_elem);
	arg->vreg = AllocVReg(num_elem, num_elem);

	/* Generate code to load argument into a scalar register.
	 * s_buffer_load_dword s[arg], s[cb1:cb1+3], idx*4
	 */
	Symbol *symbol = nullptr;
	switch (num_elem)
	{
	case 1:
	{
		Inst *inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORD,
				new ArgScalarRegister(arg->sreg),
				new ArgScalarRegisterSeries(sreg_cb1, sreg_cb1 + 3),
				new ArgLiteral(arg->index * 4));
		basic_block->AddInst(inst);

		/* Copy argument into a vector register. This vector register will be
		 * used for convenience during code emission, so that we don't have to
		 * worry at this point about different operand type encodings for
		 * instructions. Optimization passes will get rid later of redundant
		 * copies and scalar opportunities.
		 * v_mov_b32 v[arg], s[arg]
		 */
		inst = new Inst(SI::INST_V_MOV_B32,
				new ArgVectorRegister(arg->vreg),
				new ArgVectorRegister(arg->sreg));
		basic_block->AddInst(inst);

		/* Insert argument name in symbol table, using its vector register. */
		symbol = new Symbol(arg->name, SymbolVectorRegister, arg->vreg);
		AddSymbol(symbol);
		break;
	}
	
	case 4:
	{
		Inst *inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORDX4,
				new ArgScalarRegisterSeries(arg->sreg, arg->sreg + 3),
				new ArgScalarRegisterSeries(sreg_cb1, sreg_cb1 + 3),
				new ArgLiteral(arg->index * 4));
		basic_block->AddInst(inst);

		/* Insert argument name in symbol table, using its scalar register. */
		symbol = new Symbol(arg->name, SymbolScalarRegister, arg->sreg);
		AddSymbol(symbol);
		
		break;
	}
	default:
		panic("%s: not supported number of elements", __FUNCTION__);
	}
	
	
	/* If argument is an object in global memory, create a UAV
	 * associated with it. */
	SI::ArgPointer *pointer = dynamic_cast<SI::ArgPointer *>
			(arg->arg.get());
	if (pointer && pointer->GetScope() == SI::ArgScopeUAV)
	{
		/* New UAV */
		FunctionUAV *uav = new FunctionUAV();
		AddUAV(uav);

		/* Store UAV index in argument and symbol */
		assert(symbol);
		symbol->SetUAVIndex(uav->index);
		arg->uav_index = uav->index;
	}
}


void Function::DumpData(std::ostream &os)
{
	/* Section header */
	os << ".metadata\n";

	/* User elements */
	os << "\tuserElements[0] = PTR_UAV_TABLE, 0, s["
			<< sreg_uav_table << ':' << sreg_uav_table + 1 << "]\n";
	os << "\tuserElements[1] = IMM_CONST_BUFFER, 0, s["
			<< sreg_cb0 << ':' << sreg_cb0 + 3 << "\n";
	os << "\tuserElements[2] = IMM_CONST_BUFFER, 1, s["
			<< sreg_cb1 << ':' << sreg_cb1 + 3 << "]\n";
	os << '\n';

	/* Floating-point mode */
	os << "\tFloatMode = 192\n";
	os << "\tIeeeMode = 0\n";
	os << '\n';

	/* Program resources */
	os << "\tCOMPUTE_PGM_RSRC2:USER_SGPR = " << sreg_wgid << '\n';
	os << "\tCOMPUTE_PGM_RSRC2:TGID_X_EN = 1\n";
	os << "\tCOMPUTE_PGM_RSRC2:TGID_Y_EN = 1\n";
	os << "\tCOMPUTE_PGM_RSRC2:TGID_Z_EN = 1\n";
	os << '\n';
}


Function::Function(llvm::Function *llvm_function)
		: name(llvm_function->getName()),
		  llvm_function(llvm_function),
		  tree(name)
{
	/* Create pre-defined nodes in control tree */
	header_node = new Common::LeafNode("header");
	uavs_node = new Common::LeafNode("uavs");
	args_node = new Common::LeafNode("args");
	tree.AddNode(header_node);
	tree.AddNode(uavs_node);
	tree.AddNode(args_node);
	tree.SetEntryNode(header_node);

	/* Add all nodes from the LLVM control flow graph */
	body_node = tree.AddLlvmCFG(llvm_function);

	/* Connect nodes */
	header_node->Connect(uavs_node);
	uavs_node->Connect(args_node);
	args_node->Connect(body_node);
}


void Function::Dump(std::ostream &os)
{
	/* Function name */
	os << ".global " << name << "\n\n";

	/* Arguments */
	os << ".args\n";
	for (auto &arg : arg_list)
		os << *arg;
	os << '\n';

	/* Dump basic blocks */
	os << ".text\n";
	std::list<Common::Node *> node_list;
	tree.PreorderTraversal(node_list);
	for (auto &node : node_list)
	{
		/* Skip abstract nodes */
		Common::LeafNode *leaf_node = dynamic_cast
				<Common::LeafNode *>(node);
		if (!leaf_node)
			continue;

		/* Get node's basic block */
		BasicBlock *basic_block = dynamic_cast<BasicBlock *>
				(leaf_node->GetBasicBlock());
		if (!basic_block)
			continue;

		/* Dump code of basic block */
		os << *basic_block;
	}
	os << '\n';

	/* Dump section '.data' */
	DumpData(os);
	os << '\n';
}


void Function::EmitHeader()
{
	/* Create header basic block */
	BasicBlock *basic_block = new BasicBlock(this, header_node);

	/* Function must be empty at this point */
	assert(!num_sregs);
	assert(!num_vregs);

	/* Allocate 3 vector registers (v[0:2]) for local ID */
	vreg_lid = AllocVReg(3);
	if (vreg_lid)
		panic("%s: vreg_lid is expented to be 0", __FUNCTION__);

	/* Allocate 2 scalar registers for UAV table. The value for these
	 * registers is assigned by the runtime based on info found in the
	 * 'userElements' metadata of the binary.*/
	sreg_uav_table = AllocSReg(2);

	/* Allocate 4 scalar registers for CB0, and 4 more for CB1. The
	 * values for these registers will be assigned by the runtime based
	 * on info present in the 'userElements' metadata. */
	sreg_cb0 = AllocSReg(4);
	sreg_cb1 = AllocSReg(4);

	/* Allocate 3 scalar registers for the work-group ID. The content of
	 * these register will be populated by the runtime based on info found
	 * in COMPUTE_PGM_RSRC2 metadata. */
	sreg_wgid = AllocSReg(3);

	/* Obtain global size in s[gsize:gsize+2].
	 * s_buffer_load_dword s[gsize], s[cb0:cb0+3], 0x00
	 * s_buffer_load_dword s[gsize+1], s[cb0:cb0+3], 0x01
	 * s_buffer_load_dword s[gsize+2], s[cb0:cb0+3], 0x02
	 */
	basic_block->AddComment("Obtain global size");
	sreg_gsize = AllocSReg(3);
	for (int index = 0; index < 3; index++)
	{
		Inst *inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORD,
				new ArgScalarRegister(sreg_gsize + index),
				new ArgScalarRegisterSeries(sreg_cb0, sreg_cb0 + 3),
				new ArgLiteral(index));
		basic_block->AddInst(inst);
	}

	/* Obtain local size in s[lsize:lsize+2].
	 *
	 * s_buffer_load_dword s[lsize], s[cb0:cb0+3], 0x04
	 * s_buffer_load_dword s[lsize+1], s[cb0:cb0+3], 0x05
	 * s_buffer_load_dword s[lsize+2], s[cb0:cb0+3], 0x06
	 */
	basic_block->AddComment("Obtain local size");
	sreg_lsize = AllocSReg(3);
	for (int index = 0; index < 3; index++)
	{
		Inst *inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORD,
				new ArgScalarRegister(sreg_lsize + index),
				new ArgScalarRegisterSeries(sreg_cb0, sreg_cb0 + 3),
				new ArgLiteral(4 + index));
		basic_block->AddInst(inst);
	}

	/* Obtain global offset in s[offs:offs+2].
	 *
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x18
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x19
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x1a
	 */
	basic_block->AddComment("Obtain global offset");
	sreg_offs = AllocSReg(3);
	for (int index = 0; index < 3; index++)
	{
		Inst *inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORD,
				new ArgScalarRegister(sreg_offs + index),
				new ArgScalarRegisterSeries(sreg_cb0, sreg_cb0 + 3),
				new ArgLiteral(0x18 + index));
		basic_block->AddInst(inst);
	}

	/* Calculate global ID in dimensions [0:2] and store it in v[3:5].
	 *
	 * v_mov_b32 v[gid+dim], s[lsize+dim]
	 * v_mul_i32_i24 v[gid+dim], s[wgid+dim], v[gid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], v[lid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], s[offs+dim]
	 */
	vreg_gid = AllocVReg(3);
	for (int index = 0; index < 3; index++)
	{
		/* Comment */
		basic_block->AddComment(StringFormat("Calculate global ID "
				"in dimension %d", index));

		/* v_mov_b32 */
		Inst *inst = new Inst(SI::INST_V_MOV_B32,
				new ArgVectorRegister(vreg_gid + index),
				new ArgScalarRegister(sreg_lsize + index));
		basic_block->AddInst(inst);

		/* v_mul_i32_i24 */
		inst = new Inst(SI::INST_V_MUL_I32_I24,
				new ArgVectorRegister(vreg_gid + index),
				new ArgScalarRegister(sreg_wgid + index),
				new ArgVectorRegister(vreg_gid + index));
		basic_block->AddInst(inst);

		/* v_add_i32 */
		inst = new Inst(SI::INST_V_ADD_I32,
				new ArgVectorRegister(vreg_gid + index),
				new ArgSpecialRegister(SI::InstSpecialRegVcc),
				new ArgVectorRegister(vreg_gid + index),
				new ArgVectorRegister(vreg_lid + index));
		basic_block->AddInst(inst);

		/* v_add_i32 */
		inst = new Inst(SI::INST_V_ADD_I32,
				new ArgVectorRegister(vreg_gid + index),
				new ArgSpecialRegister(SI::InstSpecialRegVcc),
				new ArgScalarRegister(sreg_offs + index),
				new ArgVectorRegister(vreg_gid + index));
		basic_block->AddInst(inst);
	}
}


void Function::EmitArgs()
{
	/* Emit code for each argument individually */
	for (auto &llvm_arg : llvm_function->getArgumentList())
	{
		/* Create function argument and add it */
		FunctionArg *arg = new FunctionArg(&llvm_arg);
		llvm::Type *llvm_type = llvm_arg.getType();
		int num_elem = llvm_type->isVectorTy() ?
				llvm_type->getVectorNumElements() : 1;

		/* Add the argument to the list. This call will cause the
		 * corresponding code to be emitted. */
		AddArg(arg, num_elem);
	}
}


void Function::EmitBody()
{
	/* Code for the function body must be emitted using a depth-first
	 * traversal of the control tree. For this, we need right here the
	 * structural analysis that produces the control tree from the
	 * control flow graph.
	 */
	assert(!tree.IsStructuralAnalysisDone());
	tree.StructuralAnalysis();

	/* Whether we use a pre- or a post-order traversal does not matter,
	 * since we are only considering the leaf nodes.
	 */
	std::list<Common::Node *> node_list;
	tree.PreorderTraversal(node_list);

	/* Emit code for basic blocks */
	for (auto node : node_list)
	{
		/* Skip abstract nodes */
		Common::LeafNode *leaf_node = dynamic_cast
				<Common::LeafNode *>(node);
		if (!leaf_node)
			continue;

		/* Skip nodes with no LLVM code to translate */
		if (!leaf_node->GetLlvmBasicBlock())
			continue;

		/* Create basic block and emit the code */
		assert(!leaf_node->GetBasicBlock());
		BasicBlock *basic_block = new BasicBlock(this, leaf_node);
		basic_block->Emit(leaf_node->GetLlvmBasicBlock());
	}
}


void Function::EmitPhi()
{
	while (phi_list.size())
	{
		/* Extract element from list */
		Phi *phi = phi_list.front().get();

		/* Get basic block */
		Common::LeafNode *node = phi->GetSrcNode();
		BasicBlock *basic_block = cast<BasicBlock *>(node->GetBasicBlock());

		/* Get source value */
		Arg *src_value = TranslateValue(phi->GetSrcValue());

		/* Copy source value to destination value.
		 * s_mov_b32 <dest_value>, <src_value>
		 */
		Inst *inst = new Inst(SI::INST_V_MOV_B32,
				phi->GetDestValue(),
				src_value);
		basic_block->AddInst(inst);

		/* Free phi object */
		phi_list.pop_front();
	}
}


#if 0
static void Llvm2siFunctionEmitIfThen(Llvm2siFunction *self, Node *node)
{
	Node *if_node;
	Node *then_node;

	Llvm2siBasicBlock *if_bb;
	Llvm2siBasicBlock *then_bb;

	Llvm2siSymbol *cond_symbol;
	List *arg_list;
	Si2binInst *inst;

	LLVMBasicBlockRef llvm_basic_block;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;

	/* Identify the two nodes */
	assert(isAbstractNode(node));
	assert(asAbstractNode(node)->region == AbstractNodeIfThen);
	assert(asAbstractNode(node)->child_list->count == 2);
	if_node = asNode(ListGoto(asAbstractNode(node)->child_list, 0));
	then_node = asNode(ListGoto(asAbstractNode(node)->child_list, 1));

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
	llvm_basic_block = asLeafNode(if_node)->llvm_basic_block;
	llinst = LLVMGetBasicBlockTerminator(llvm_basic_block);
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
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, tos_sreg, tos_sreg + 1)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, cond_sreg, cond_sreg + 1)));
	inst = new(Si2binInst, SI_INST_S_AND_SAVEEXEC_B64, arg_list);
	Llvm2siBasicBlockAddInst(if_bb, inst);


	/*** Code for 'then' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, tos_sreg, tos_sreg + 1)));
	inst = new(Si2binInst, SI_INST_S_MOV_B64, arg_list);
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
	List *arg_list;
	Si2binInst *inst;

	LLVMBasicBlockRef llvm_basic_block;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;

	/* Identify the three nodes */
	assert(isAbstractNode(node));
	assert(asAbstractNode(node)->region == AbstractNodeIfThenElse);
	assert(asAbstractNode(node)->child_list->count == 3);
	if_node = asNode(ListGoto(asAbstractNode(node)->child_list, 0));
	then_node = asNode(ListGoto(asAbstractNode(node)->child_list, 1));
	else_node = asNode(ListGoto(asAbstractNode(node)->child_list, 2));

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
	llvm_basic_block = asLeafNode(if_node)->llvm_basic_block;
	llinst = LLVMGetBasicBlockTerminator(llvm_basic_block);
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
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, tos_sreg, tos_sreg + 1)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, cond_sreg, cond_sreg + 1)));
	inst = new(Si2binInst, SI_INST_S_AND_SAVEEXEC_B64, arg_list);
	Llvm2siBasicBlockAddInst(if_bb, inst);


	/*** Code for 'then' block ***/

	/* Invert active mask and-ing it with the top of the stack.
	 * s_andn2_b64 exec, <tos_sreg>, exec
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, tos_sreg, tos_sreg + 1)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	inst = new(Si2binInst, SI_INST_S_ANDN2_B64, arg_list);
	Llvm2siBasicBlockAddInst(then_bb, inst);


	/*** Code for 'else' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, tos_sreg, tos_sreg + 1)));
	inst = new(Si2binInst, SI_INST_S_MOV_B64, arg_list);
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
	List *arg_list;
	Si2binInst *inst;

	SIInstOpcode opcode;

	LLVMBasicBlockRef llvm_basic_block;
	LLVMValueRef llinst;
	LLVMValueRef llcond;

	int cond_sreg;
	int tos_sreg;

	char *cond_name;


	/* Identify the two nodes */
	assert(isAbstractNode(node));
	assert(asAbstractNode(node)->region == AbstractNodeWhileLoop);
	assert(asAbstractNode(node)->child_list->count == 4);
	pre_node = asNode(ListGoto(asAbstractNode(node)->child_list, 0));
	head_node = asNode(ListGoto(asAbstractNode(node)->child_list, 1));
	tail_node = asNode(ListGoto(asAbstractNode(node)->child_list, 2));
	exit_node = asNode(ListGoto(asAbstractNode(node)->child_list, 3));

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
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, tos_sreg, tos_sreg + 1)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	inst = new(Si2binInst, SI_INST_S_MOV_B64, arg_list);
	Llvm2siBasicBlockAddInst(pre_bb, inst);


	/*** Code for exit block ***/

	/* Pop active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, tos_sreg, tos_sreg + 1)));
	inst = new(Si2binInst, SI_INST_S_MOV_B64, arg_list);
	Llvm2siBasicBlockAddInst(exit_bb, inst);


	/*** Code for tail block ***/

	/* Jump to head block.
	 * s_branch <head_block>
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateLabel, head_node->name)));
	inst = new(Si2binInst, SI_INST_S_BRANCH, arg_list);
	Llvm2siBasicBlockAddInst(tail_bb, inst);


	/*** Code for head block ***/

	/* Get head block terminator */
	llvm_basic_block = asLeafNode(head_node)->llvm_basic_block;
	llinst = LLVMGetBasicBlockTerminator(llvm_basic_block);
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
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegExec)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, cond_sreg, cond_sreg + 1)));
	inst = new(Si2binInst, opcode, arg_list);
	Llvm2siBasicBlockAddInst(head_bb, inst);

	/* Exit loop if no more work-items are active.
	 * s_cbranch_execz <exit_node>
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateLabel, exit_node->name)));
	inst = new(Si2binInst, SI_INST_S_CBRANCH_EXECZ, arg_list);
	Llvm2siBasicBlockAddInst(head_bb, inst);
}


void Llvm2siFunctionEmitControlFlow(Llvm2siFunction *self)
{
	List *node_list;
	Node *node;

	/* Emit control flow actions using a post-order traversal of the syntax
	 * tree (not control-flow graph), from inner to outer control flow
	 * structures. Which specific post-order traversal does not matter. */
	node_list = new(List);
	CTreeTraverse(self->ctree, NULL, node_list);

	/* Traverse nodes */
	ListForEach(node_list, node, Node)
	{
		/* Ignore leaf nodes */
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

	/* Free */
	delete(node_list);
}


static Si2binArg *Llvm2siFunctionTranslateConstValue(
		Llvm2siFunction *self, LLVMValueRef llvm_value)
{
	LLVMTypeRef llvm_type;
	LLVMTypeKind lltype_kind;

	llvm_type = LLVMTypeOf(llvm_value);
	lltype_kind = LLVMGetTypeKind(llvm_type);
	
	/* Check constant type */
	switch (lltype_kind)
	{
	
	case LLVMIntegerTypeKind:
	{
		int bit_width;
		int value;

		/* Only 32-bit constants supported for now. We need to figure
		 * out what to do with the sign extension otherwise. */
		bit_width = LLVMGetIntTypeWidth(llvm_type);
		if (bit_width != 32)
			fatal("%s: only 32-bit integer constant supported "
				" (%d-bit found)", __FUNCTION__, bit_width);

		/* Create argument */
		value = LLVMConstIntGetZExtValue(llvm_value);
		return new_ctor(Si2binArg, CreateLiteral, value);
	}

	default:
		
		fatal("%s: constant type not supported (%d)",
			__FUNCTION__, lltype_kind);
		return NULL;
	}
}


Si2binArg *Llvm2siFunctionTranslateValue(Llvm2siFunction *self,
		LLVMValueRef llvm_value, Llvm2siSymbol **symbol_ptr)
{
	Llvm2siSymbol *symbol;
	Si2binArg *arg;

	char *name;

	/* Returned symbol is NULL by default */
	PTR_ASSIGN(symbol_ptr, NULL);

	/* Treat constants separately */
	if (LLVMIsConstant(llvm_value))
		return Llvm2siFunctionTranslateConstValue(self, llvm_value);

	/* Get name */
	name = (char *) LLVMGetValueName(llvm_value);
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

		arg = new_ctor(Si2binArg, CreateVectorRegister, symbol->reg);
		break;

	case llvm2si_symbol_scalar_register:

		arg = new_ctor(Si2binArg, CreateScalarRegister, symbol->reg);
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


Si2binArg *Llvm2siFunctionConstToVReg(Llvm2siFunction *self,
		Llvm2siBasicBlock *basic_block, Si2binArg *arg)
{
	Si2binArg *ret_arg;
	List *arg_list;
	Si2binInst *inst;
	int vreg;

	/* If argument is not a literal of any kind, don't convert. */
	if (!Si2binArgIsConstant(arg))
		return arg;

	/* Allocate vector register */
	vreg = Llvm2siFunctionAllocVReg(self, 1, 1);
	ret_arg = new_ctor(Si2binArg, CreateVectorRegister, vreg);

	/* Copy constant to vector register.
	 * v_mov_b32 <vreg>, <const>
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, vreg)));
	ListAdd(arg_list, asObject(arg));
	inst = new(Si2binInst, SI_INST_V_MOV_B32, arg_list);
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
	//return 100;
}
#endif


}  /* namespace llvm2si */
