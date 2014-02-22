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

#include <fstream>
#include <lib/cpp/Misc.h>
#include <llvm/Constants.h>
#include <llvm/Function.h>

#include "BasicBlock.h"
#include "Function.h"
#include "InterferenceGraph.h"

using namespace si2bin;

namespace llvm2si
{


/*
 * Class 'FunctionArg'
 */

FunctionArg::FunctionArg(llvm::Argument *llvm_arg) :
		llvm_arg(llvm_arg)
{
	/* Initialize */
	function = nullptr;
	index = -1;
	uav_index = -1;
	sreg = 0;
	vreg = 0;

	/* Get argument name */
	name = llvm_arg->getName();
	if (name.empty())
		misc::fatal("%s: anonymous arguments not allowed", __FUNCTION__);

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
			misc::panic("%s: unsupported argument bit width (%d)",
				__FUNCTION__, bit_width);
		}
	}
	else if (llvm_type->isFloatTy())
	{
		return SI::ArgDataTypeFloat;
	}
	else
	{
		misc::panic("%s: unsupported argument type kind (%d)",
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


void FunctionArg::Dump(std::ostream &os)
{
	switch (arg->getType())
	{

	case SI::ArgTypePointer:
	{
		os << '\t' << *arg << ' ' << index * 16
				<< " uav" << uav_index + 10 << '\n';
		break;
	}

	case SI::ArgTypeValue:
	{
		os << '\t' << *arg << ' ' << index * 16 << '\n';
		break;
	}

	default:
		misc::panic("%s: argument type not recognized (%d)",
				__FUNCTION__, arg->getType());
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


int Function::AddArg(FunctionArg *arg, int num_elem, int offset)
{
	/* Check that argument does not belong to a self yet */
	if (arg->function)
		misc::panic("%s: argument already added", __FUNCTION__);

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
				new ArgLiteral(arg->index * 4 + offset));
		basic_block->AddInst(inst);

		inst = new Inst(SI::INST_S_WAITCNT,
				new ArgWaitCnt(WaitCntTypeLgkmCnt));
		basic_block->AddInst(inst);

		/* Copy argument into a vector register. This vector register will be
		 * used for convenience during code emission, so that we don't have to
		 * worry at this point about different operand type encodings for
		 * instructions. Optimization passes will get rid later of redundant
		 * copies and scalar opportunities.
		 * v_mov_b32 v[arg], s[arg]
		 */
		/*inst = new Inst(SI::INST_V_MOV_B32,
				new ArgVectorRegister(arg->vreg),
				new ArgScalarRegister(arg->sreg));
		basic_block->AddInst(inst);*/

		/* Insert argument name in symbol table, using its vector register. */
		symbol = new Symbol(arg->name, SymbolScalarRegister, arg->sreg);
		AddSymbol(symbol);
		break;
	}

	case 4:
	{
		Inst *inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORDX4,
				new ArgScalarRegisterSeries(arg->sreg, arg->sreg + 3),
				new ArgScalarRegisterSeries(sreg_cb1, sreg_cb1 + 3),
				new ArgLiteral(arg->index * 4 + offset));
		basic_block->AddInst(inst);

		inst = new Inst(SI::INST_S_WAITCNT,
				new ArgWaitCnt(WaitCntTypeLgkmCnt));
		basic_block->AddInst(inst);

		/* Insert argument name in symbol table, using its scalar register. */
		symbol = new Symbol(arg->name, SymbolScalarRegister, arg->sreg);
		AddSymbol(symbol);

		break;
	}
	case 8:
	{
		Inst *inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORDX4,
				new ArgScalarRegisterSeries(arg->sreg, arg->sreg + 3),
				new ArgScalarRegisterSeries(sreg_cb1, sreg_cb1 + 3),
				new ArgLiteral(arg->index * 4 + offset));
		basic_block->AddInst(inst);
		
		inst = new Inst(SI::INST_S_BUFFER_LOAD_DWORDX4,
				new ArgScalarRegisterSeries(arg->sreg + 4, arg->sreg + 7),
				new ArgScalarRegisterSeries(sreg_cb1, sreg_cb1 + 3),
				new ArgLiteral(arg->index * 4 + 4 + offset));

		offset += 4;
		basic_block->AddInst(inst);
		
		inst = new Inst(SI::INST_S_WAITCNT,
				new ArgWaitCnt(WaitCntTypeLgkmCnt));
		basic_block->AddInst(inst);

		/* Insert argument name in symbol table, using its scalar register. */
		symbol = new Symbol(arg->name, SymbolScalarRegister, arg->sreg);
		AddSymbol(symbol);
		
		break;
	}
	default:
		misc::panic("%s: not supported number of elements", __FUNCTION__);
	}


	/* If argument is an object in global memory, create a UAV
	 * associated with it. */
	SI::ArgPointer *pointer = dynamic_cast<SI::ArgPointer *>
			(arg->arg.get());
	if (pointer && pointer->getScope() == SI::ArgScopeUAV)
	{
		/* New UAV */
		FunctionUAV *uav = new FunctionUAV();
		AddUAV(uav);

		/* Store UAV index in argument and symbol */
		assert(symbol);
		symbol->SetUAVIndex(uav->index);
		arg->uav_index = uav->index;
	}

	return offset;
}


void Function::DumpData(std::ostream &os)
{
	/* Section header */
	os << ".metadata\n";

	/* User elements */
	os << "\tuserElements[0] = PTR_UAV_TABLE, 0, s["
			<< sreg_uav_table << ':' << sreg_uav_table + 1 << "]\n";
	os << "\tuserElements[1] = IMM_CONST_BUFFER, 0, s["
			<< sreg_cb0 << ':' << sreg_cb0 + 3 << "]\n";
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
		  num_sregs(0),
		  tree(name),
		  num_vregs(0)
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
		misc::panic("%s: vreg_lid is expected to be 0", __FUNCTION__);

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
		basic_block->AddComment(misc::fmt("Calculate global ID "
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

	int offset = 0;
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
		offset = AddArg(arg, num_elem, offset);
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
		BasicBlock *basic_block = misc::cast<BasicBlock *>(node->GetBasicBlock());

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


void Function::EmitIfThen(Common::AbstractNode *node)
{
	/* Identify the two nodes */
	assert(node);
	assert(node->GetRegion() == Common::AbstractNodeIfThen);
	assert(node->GetChildList().size() == 2);
	Common::Node *if_node = node->GetChildList().front();
	Common::Node *then_node = node->GetChildList().back();

	/* Make sure roles match */
	assert(if_node->getRole() == Common::NodeRoleIf);
	assert(then_node->getRole() == Common::NodeRoleThen);

	/* Get basic blocks. 'If' node should be a leaf. */
	then_node = then_node->GetLastLeaf();
	assert(if_node->getKind() == Common::NodeKindLeaf);
	assert(then_node->getKind() == Common::NodeKindLeaf);
	Common::LeafNode *if_leaf_node = misc::cast<Common::LeafNode *>(if_node);
	Common::LeafNode *then_leaf_node = misc::cast<Common::LeafNode *>(then_node);
	BasicBlock *if_basic_block = misc::cast<BasicBlock *>(if_leaf_node->GetBasicBlock());
	BasicBlock *then_basic_block = misc::cast<BasicBlock *>(then_leaf_node->GetBasicBlock());


	/*** Code for 'If' block ***/

	/* Get 'If' basic block terminator */
	llvm::BasicBlock *llvm_basic_block = if_leaf_node->GetLlvmBasicBlock();
	llvm::TerminatorInst *llvm_inst = llvm_basic_block->getTerminator();
	assert(llvm_inst);
	assert(llvm_inst->getOpcode() == llvm::Instruction::Br);
	assert(llvm_inst->getNumOperands() == 3);

	/* Get symbol associated with condition variable */
	llvm::Value *llvm_cond = llvm_inst->getOperand(0);
	std::string cond_name = llvm_cond->getName();
	Symbol *cond_symbol = symbol_table.Lookup(cond_name);
	assert(cond_symbol);
	assert(cond_symbol->getType() == SymbolScalarRegister);
	assert(cond_symbol->getNumRegs() == 2);
	int cond_sreg = cond_symbol->getReg();

	/* Allocate two scalar registers to push the active mask */
	int tos_sreg = AllocSReg(2, 2);

	/* Emit active mask push and set at the end of the 'If' block.
	 * s_and_saveexec_b64 <tos_sreg> <cond_sreg>
	 */
	Inst *inst = new Inst(SI::INST_S_AND_SAVEEXEC_B64,
			new ArgScalarRegisterSeries(tos_sreg, tos_sreg + 1),
			new ArgScalarRegisterSeries(cond_sreg, cond_sreg + 1));
	if_basic_block->AddInst(inst);


	/*** Code for 'then' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	inst = new Inst(SI::INST_S_MOV_B64,
			new ArgSpecialRegister(SI::InstSpecialRegExec),
			new ArgScalarRegisterSeries(tos_sreg, tos_sreg + 1));
	then_basic_block->AddInst(inst);
}


void Function::EmitIfThenElse(Common::AbstractNode *node)
{
	/* Identify the three nodes */
	assert(node);
	assert(node->GetRegion() == Common::AbstractNodeIfThenElse);
	assert(node->GetChildList().size() == 3);
	auto it = node->GetChildList().begin();
	Common::Node *if_node = *(it++);
	Common::Node *then_node = *(it++);
	Common::Node *else_node = *(it++);
	assert(it == node->GetChildList().end());

	/* Make sure roles match */
	assert(if_node->getRole() == Common::NodeRoleIf);
	assert(then_node->getRole() == Common::NodeRoleThen);
	assert(else_node->getRole() == Common::NodeRoleElse);

	/* Get basic blocks. 'If' node should be a leaf. */
	then_node = then_node->GetLastLeaf();
	else_node = else_node->GetLastLeaf();
	assert(if_node->getKind() == Common::NodeKindLeaf);
	assert(then_node->getKind() == Common::NodeKindLeaf);
	assert(else_node->getKind() == Common::NodeKindLeaf);
	Common::LeafNode *if_leaf_node = dynamic_cast<Common::LeafNode *>(if_node);
	Common::LeafNode *then_leaf_node = dynamic_cast<Common::LeafNode *>(then_node);
	Common::LeafNode *else_leaf_node = dynamic_cast<Common::LeafNode *>(else_node);
	BasicBlock *if_basic_block = dynamic_cast<BasicBlock *>(if_leaf_node->GetBasicBlock());
	BasicBlock *then_basic_block = dynamic_cast<BasicBlock *>(then_leaf_node->GetBasicBlock());
	BasicBlock *else_basic_block = dynamic_cast<BasicBlock *>(else_leaf_node->GetBasicBlock());


	/*** Code for 'If' block ***/

	/* Get 'If' basic block terminator */
	llvm::BasicBlock *llvm_basic_block = if_leaf_node->GetLlvmBasicBlock();
	llvm::TerminatorInst *llvm_inst = llvm_basic_block->getTerminator();
	assert(llvm_inst);
	assert(llvm_inst->getOpcode() == llvm::Instruction::Br);
	assert(llvm_inst->getNumOperands() == 3);

	/* Get symbol associated with condition variable */
	llvm::Value *llvm_cond = llvm_inst->getOperand(0);
	std::string cond_name = llvm_cond->getName();
	Symbol *cond_symbol = symbol_table.Lookup(cond_name);
	assert(cond_symbol);
	assert(cond_symbol->getType() == SymbolScalarRegister);
	assert(cond_symbol->getNumRegs() == 2);
	int cond_sreg = cond_symbol->getReg();

	/* Allocate two scalar registers to push the active mask */
	int tos_sreg = AllocSReg(2, 2);

	/* Emit active mask push and set at the end of the 'If' block.
	 * s_and_saveexec_b64 <tos_sreg> <cond_sreg>
	 */
	Inst *inst = new Inst(SI::INST_S_AND_SAVEEXEC_B64,
			new ArgScalarRegisterSeries(tos_sreg, tos_sreg + 1),
			new ArgScalarRegisterSeries(cond_sreg, cond_sreg + 1));
	if_basic_block->AddInst(inst);


	/*** Code for 'then' block ***/

	/* Invert active mask and-ing it with the top of the stack.
	 * s_andn2_b64 exec, <tos_sreg>, exec
	 */
	inst = new Inst(SI::INST_S_ANDN2_B64,
			new ArgSpecialRegister(SI::InstSpecialRegExec),
			new ArgScalarRegisterSeries(tos_sreg, tos_sreg + 1),
			new ArgSpecialRegister(SI::InstSpecialRegExec));
	then_basic_block->AddInst(inst);


	/*** Code for 'else' block ***/

	/* Pop the active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	inst = new Inst(SI::INST_S_MOV_B64,
			new ArgSpecialRegister(SI::InstSpecialRegExec),
			new ArgScalarRegisterSeries(tos_sreg, tos_sreg + 1));
	else_basic_block->AddInst(inst);
}


void Function::EmitWhileLoop(Common::AbstractNode *node)
{
	/* Identify the two nodes */
	assert(node);
	assert(node->GetRegion() == Common::AbstractNodeWhileLoop);
	assert(node->GetChildList().size() == 4);
	auto it = node->GetChildList().begin();
	Common::Node *pre_node = *(it++);
	Common::Node *head_node = *(it++);
	Common::Node *tail_node = *(it++);
	Common::Node *exit_node = *(it++);
	assert(it == node->GetChildList().end());

	/* Make sure roles match */
	assert(pre_node->getRole() == Common::NodeRolePre);
	assert(head_node->getRole() == Common::NodeRoleHead);
	assert(tail_node->getRole() == Common::NodeRoleTail);
	assert(exit_node->getRole() == Common::NodeRoleExit);

	/* Get basic blocks. Pre-header/head/exit nodes should be a leaves.
	 * Tail node can be an abstract node, which we need to track down to
	 * its last leaf to introduce control flow at the end.
	 *
	 * Basic blocks must exist associated to the head and the tail blocks:
	 * they come from LLVM blocks already emitted. But pre-header and exit
	 * blocks have been inserted during the structural analysis, so they
	 * contain no basic block yet.
	 */
	tail_node = tail_node->GetLastLeaf();
	assert(pre_node->getKind() == Common::NodeKindLeaf);
	assert(head_node->getKind() == Common::NodeKindLeaf);
	assert(tail_node->getKind() == Common::NodeKindLeaf);
	assert(exit_node->getKind() == Common::NodeKindLeaf);
	Common::LeafNode *pre_leaf_node = misc::cast<Common::LeafNode *>(pre_node);
	Common::LeafNode *head_leaf_node = misc::cast<Common::LeafNode *>(head_node);
	Common::LeafNode *tail_leaf_node = misc::cast<Common::LeafNode *>(tail_node);
	Common::LeafNode *exit_leaf_node = misc::cast<Common::LeafNode *>(exit_node);
	assert(!pre_leaf_node->GetBasicBlock());
	BasicBlock *head_basic_block = misc::cast<BasicBlock *>(head_leaf_node->GetBasicBlock());
	BasicBlock *tail_basic_block = misc::cast<BasicBlock *>(tail_leaf_node->GetBasicBlock());
	assert(!exit_leaf_node->GetBasicBlock());

	/* Create pre-header and exit basic blocks */
	BasicBlock *pre_basic_block = new BasicBlock(this, pre_leaf_node);
	BasicBlock *exit_basic_block = new BasicBlock(this, exit_leaf_node);


	/*** Code for pre-header block ***/

	/* Allocate two scalar registers to push the active mask */
	int tos_sreg = AllocSReg(2, 2);

	/* Push active mask.
	 * s_mov_b64 <tos_sreg>, exec
	 */
	Inst *inst = new Inst(SI::INST_S_MOV_B64,
			new ArgScalarRegisterSeries(tos_sreg, tos_sreg + 1),
			new ArgSpecialRegister(SI::InstSpecialRegExec));
	pre_basic_block->AddInst(inst);


	/*** Code for exit block ***/

	/* Pop active mask.
	 * s_mov_b64 exec, <tos_sreg>
	 */
	inst = new Inst(SI::INST_S_MOV_B64,
			new ArgSpecialRegister(SI::InstSpecialRegExec),
			new ArgScalarRegisterSeries(tos_sreg, tos_sreg + 1));
	exit_basic_block->AddInst(inst);


	/*** Code for tail block ***/

	/* Jump to head block.
	 * s_branch <head_block>
	 */
	inst = new Inst(SI::INST_S_BRANCH,
			new ArgLabel(head_leaf_node->getName()));
	tail_basic_block->AddInst(inst);


	/*** Code for head block ***/

	/* Get head block terminator */
	llvm::BasicBlock *llvm_basic_block = head_leaf_node->GetLlvmBasicBlock();
	llvm::TerminatorInst *llvm_inst = llvm_basic_block->getTerminator();
	assert(llvm_inst);
	assert(llvm_inst->getOpcode() == llvm::Instruction::Br);
	assert(llvm_inst->getNumOperands() == 3);

	/* Get symbol associated with condition variable */
	llvm::Value *llvm_cond = llvm_inst->getOperand(0);
	std::string cond_name = llvm_cond->getName();
	Symbol *cond_symbol = symbol_table.Lookup(cond_name);
	assert(cond_symbol);
	assert(cond_symbol->getType() == SymbolScalarRegister);
	assert(cond_symbol->getNumRegs() == 2);
	int cond_sreg = cond_symbol->getReg();

	/* Obtain opcode depending on whether the loop exists when the head's
	 * condition is true or it does when it is false. */
	SI::InstOpcode opcode = SI::INST_S_AND_B64;
	assert(head_node->getExitIfTrue() ^ head_node->getExitIfFalse());
	if (head_node->getExitIfTrue())
		opcode = SI::INST_S_ANDN2_B64;

	/* Bitwise 'and' of active mask with condition.
	 * s_and(n2)_b64 exec, exec, <cond_sreg>
	 */
	inst = new Inst(opcode,
			new ArgSpecialRegister(SI::InstSpecialRegExec),
			new ArgSpecialRegister(SI::InstSpecialRegExec),
			new ArgScalarRegisterSeries(cond_sreg, cond_sreg + 1));
	head_basic_block->AddInst(inst);

	/* Exit loop if no more work-items are active.
	 * s_cbranch_execz <exit_node>
	 */
	inst = new Inst(SI::INST_S_CBRANCH_EXECZ,
			new ArgLabel(exit_node->getName()));
	head_basic_block->AddInst(inst);
}


void Function::EmitControlFlow()
{
	/* Emit control flow actions using a post-order traversal of the syntax
	 * tree (not control-flow graph), from inner to outer control flow
	 * structures. Which specific post-order traversal does not matter. */
	std::list<Common::Node *> node_list;
	tree.PostorderTraversal(node_list);

	/* Traverse nodes */
	for (auto &node : node_list)
	{
		/* Ignore leaf nodes */
		Common::AbstractNode *abs_node = dynamic_cast
				<Common::AbstractNode *>(node);
		if (!abs_node)
			continue;

		/* Check control structure */
		switch (abs_node->GetRegion())
		{

		case Common::AbstractNodeBlock:

			/* Ignore blocks */
			break;

		case Common::AbstractNodeIfThen:

			EmitIfThen(abs_node);
			break;

		case Common::AbstractNodeIfThenElse:

			EmitIfThenElse(abs_node);
			break;

		case Common::AbstractNodeWhileLoop:

			EmitWhileLoop(abs_node);
			break;

		default:
			misc::panic("%s: region %s not supported", __FUNCTION__,
					Common::abstract_node_region_map.MapValue(
					abs_node->GetRegion()));
		}
	}
}


void Function::LiveRegisterAnalysis() {
	llvm2si::BasicBlock *basic_block;
	//Common::Node *node;

	std::list<llvm2si::BasicBlock *> worklist;

	std::list<Common::Node *> node_list;
	tree.PostorderTraversal(node_list);
	for(auto &node : node_list)
	{
		/* Skip abstract nodes */
		if (node->getKind() != Common::NodeKindLeaf)
			continue;

		/* Get node's basic block */
		if (!dynamic_cast<Common::LeafNode*>(node))
			continue;

		basic_block = dynamic_cast<llvm2si::BasicBlock*>((dynamic_cast<Common::LeafNode*>(node)->GetBasicBlock()));
		if (!basic_block)
			continue;

		/* Assigns blank bitmaps of size num_sregs to each of the
		 * basic block's bitmap fields
		 */
		basic_block->def = new misc::Bitmap(this->num_vregs);
		basic_block->use = new misc::Bitmap(this->num_vregs);
		basic_block->in = new misc::Bitmap(this->num_vregs);
		basic_block->out = new misc::Bitmap(this->num_vregs);

		/* Read each line of basic block */
		for (auto &inst : basic_block->getInstList())
		{
			/* Get each argument in the line */
			for (auto &arg : inst->getArgs())
			{
				/* Currently only deals with scalar registers */
				if (arg->getType() == si2bin::ArgTypeVectorRegister) {

					si2bin::ArgVectorRegister *argReg = dynamic_cast<si2bin::ArgVectorRegister*>(arg.get());
					if (!argReg)
						continue;

					if (arg->getToken()->GetDirection() == si2bin::TokenDirectionDst)
					{
						basic_block->def->Set(argReg->getId(), true);
					}
					else if (arg->getToken()->GetDirection() == si2bin::TokenDirectionSrc)
					{
						/* If register wasn't defined in the same basic block */
						if (basic_block->def->Test(argReg->getId()) != true)
						{
							basic_block->use->Set(argReg->getId(), true);
						}
					}
				}
			}
		}

		/* Adds basic block into worklist if it is a exit node */
		llvm::BasicBlock *llvm_basic_block = (dynamic_cast<Common::LeafNode*>(node))->GetLlvmBasicBlock();
		llvm::TerminatorInst *llvm_inst = llvm_basic_block->getTerminator();
		assert(llvm_inst);
		if(llvm_inst->getOpcode() == llvm::Instruction::Ret)
			worklist.push_back(basic_block);
	}

	/* Iterates through worklist until no basic blocks remain */
	while(worklist.size() != 0) {
		basic_block = worklist.front();

		/* Clones out into in so that it can be used to perform calculations */
		*basic_block->in = (*basic_block->out - *basic_block->def) & *basic_block->use;

		// Get predecessors
		// TODO is this pred list correct??? I dont think it contains headers
		for (auto &node : basic_block->GetNode()->pred_list)
		{
			llvm2si::BasicBlock *pred_basic_block;

			/* Skip abstract nodes */
			if (node->getKind() != Common::NodeKindLeaf)
				continue;

			/* Get node's basic block */
			if (!dynamic_cast<Common::LeafNode*>(node))
				continue;
			//
			pred_basic_block = dynamic_cast<llvm2si::BasicBlock*>((dynamic_cast<Common::LeafNode*>(node)->GetBasicBlock()));
			if (!basic_block)
				continue;

			// Check old n
			misc::Bitmap oldOut = *pred_basic_block->out;

			// Or old out with in of successor basic block
			*pred_basic_block->out = *(pred_basic_block->out) | *(basic_block->in);

			/* If compare returns one, it changed */
			if (*pred_basic_block->out == oldOut) {
				if(std::find(worklist.begin(), worklist.end(), pred_basic_block) == worklist.end());
					worklist.push_back(pred_basic_block);
			}

		}

		worklist.erase(worklist.begin());

	}

	/* Instruction bitmap population */
	for (auto &node : node_list)
	{
		/* Skip abstract nodes */
		if (node->getKind() != Common::NodeKindLeaf)
			continue;

		/* Get node's basic block */
		if (!dynamic_cast<Common::LeafNode*>(node))
			continue;

		basic_block = dynamic_cast<llvm2si::BasicBlock*>((dynamic_cast<Common::LeafNode*>(node)->GetBasicBlock()));
		if (!basic_block)
			continue;

		basic_block->LiveRegisterAnalysis();

	}

	LiveRegisterAnalysisBitmapDump();
	LiveRegisterAllocation();

	/* Free structures */
	for (auto &node : node_list)
	{
		/* Skip abstract nodes */
		if (node->getKind() != Common::NodeKindLeaf)
			continue;

		/* Get node's basic block */
		if (!dynamic_cast<Common::LeafNode*>(node))
			continue;

		basic_block = dynamic_cast<llvm2si::BasicBlock*>((dynamic_cast<Common::LeafNode*>(node)->GetBasicBlock()));
		if (!basic_block)
			continue;

		/* Iterate through list of instructions to delete bitmaps */
		for(auto &inst : basic_block->getInstList())
		{
			delete(inst->def);
			delete(inst->use);

			delete(inst->in);
			delete(inst->out);
		}

		/* Deletes allocated memory*/
		delete(basic_block->def);
		delete(basic_block->use);
		delete(basic_block->in);
		delete(basic_block->out);

	}

}


void Function::LiveRegisterAllocation() {
	llvm2si::InterferenceGraph interferenceGraph(this->num_vregs);

	llvm2si::BasicBlock *basic_block;
	//Common::Node *node;

	std::list<Common::Node *> node_list;
	tree.PostorderTraversal(node_list);
	for(auto &node : node_list)
	{
		/* Skip abstract nodes */
		if (node->getKind() != Common::NodeKindLeaf)
			continue;

		/* Get node's basic block */
		if (!dynamic_cast<Common::LeafNode*>(node))
			continue;

		basic_block = dynamic_cast<llvm2si::BasicBlock*>((dynamic_cast<Common::LeafNode*>(node)->GetBasicBlock()));
		if (!basic_block)
			continue;

		for(auto &inst : basic_block->getInstList())
		{
			for (unsigned int i = 0; i < inst->out->getSize(); i++) {
				if (inst->out->Test(i)) {
					for (unsigned int j = i; j < inst->out->getSize(); j++) {
						if (inst->out->Test(j))
							interferenceGraph.Set(i, j, true);
					}
				}
			}

		}
	}

	// Creates a register map
	std::vector<int> registerMap;
	int n = 256; // Should be set to number of registers available

	for (int i = 0; i < interferenceGraph.GetSize(); i++) {
		misc::Bitmap b(n); // clear

		for (int j = 0; j < i; j++) {
			if (interferenceGraph.Get(i,j)) {
				b.Set(j, true);
			}
		}

		unsigned int count;
		for (count = 0; count < b.getSize(); count++)
			if (b[count] == false)
				break;

		assert (count != b.getSize());

		registerMap.push_back(count);
	}
}


void Function::LiveRegisterAnalysisBitmapDump() {
	std::ofstream file("BitmapDump", std::ofstream::out);

	int i = 0;

	std::list<Common::Node *> node_list;
	tree.PostorderTraversal(node_list);
	for (auto &node : node_list)
	{
		/* Skip abstract nodes */
		if (node->getKind() != Common::NodeKindLeaf)
			continue;

		/* Get node's basic block */
		if (!dynamic_cast<Common::LeafNode*>(node))
			continue;

		llvm2si::BasicBlock *basic_block = dynamic_cast<llvm2si::BasicBlock*>((dynamic_cast<Common::LeafNode*>(node)->GetBasicBlock()));
		if (!basic_block)
			continue;

		file << "Basic Block: " << i << std::endl;

		/* Assigns blank bitmaps of size num_sregs to each of the
		 * basic block's bitmap fields
		 */
		file << "--Def Bitmap:\t";
		basic_block->def->Dump(file);

		file << "--Use Bitmap:\t";
		basic_block->use->Dump(file);

		file << "--In Bitmap:\t";
		basic_block->in->Dump(file);

		file << "--Out Bitmap:\t";
		basic_block->out->Dump(file);

		file << "--Instruction List:\n";
		//int j = 0;
		/* Iterate over instructions in basic block */
		//si2bin::Inst *inst;

//		for (auto &inst : basic_block->getInstList())
//		{
//			file << "\tLive After Instruction " << j << ": ";
//			inst->out->Dump(file);
//
//			j++;
//		}

		file << std::endl;

		i++;
	}

	/* Free structures */
	file.close();
}


Arg *Function::TranslateConstant(llvm::Constant *llvm_const)
{
	/* Check constant type */
	llvm::Type *llvm_type = llvm_const->getType();
	if (llvm_type->isIntegerTy())
	{
		/* Only 32-bit constants supported for now. We need to figure
		 * out what to do with the sign extension otherwise. */
		if (!llvm_type->isIntegerTy(32))
			misc::panic("%s: only 32-bit integer constant supported "
					" (%d-bit found)", __FUNCTION__,
					llvm_type->getIntegerBitWidth());

		/* Create argument */
		llvm::ConstantInt *llvm_int_const = misc::cast
				<llvm::ConstantInt *>(llvm_const);
		int value = llvm_int_const->getZExtValue();
		return new ArgLiteral(value);
	}
	else
	{
		misc::panic("%s: constant type not supported (%d)",
				__FUNCTION__, llvm_type->getTypeID());
		return NULL;
	}
}


Arg *Function::TranslateValue(llvm::Value *llvm_value, Symbol *&symbol)
{
	/* Returned symbol is null by default */
	symbol = nullptr;

	/* Treat constants separately */
	llvm::Constant *llvm_const = dynamic_cast<llvm::Constant *>(llvm_value);
	if (llvm_const)
		return TranslateConstant(llvm_const);

	/* Get name */
	std::string name = llvm_value->getName();
	if (name.empty())
		misc::panic("%s: anonymous value", __FUNCTION__);

	/* Look up symbol */
	symbol = symbol_table.Lookup(name);
	if (!symbol)
		misc::fatal("%s: %s: symbol not found", __FUNCTION__, name.c_str());

	/* Create argument based on symbol type */
	Arg *arg;
	switch (symbol->getType())
	{

	case SymbolVectorRegister:

		arg = new ArgVectorRegister(symbol->getReg());
		break;

	case SymbolScalarRegister:

		arg = new ArgScalarRegister(symbol->getReg());
		break;

	default:

		arg = nullptr;
		misc::panic("%s: invalid symbol type (%d)", __FUNCTION__,
				symbol->getType());
	}

	/* Return argument and symbol */
	return arg;
}


Arg *Function::ConstToVReg(BasicBlock *basic_block, Arg *arg)
{
	/* If argument is not a literal of any kind, don't convert. */
	if (!arg->isConstant())
		return arg;

	/* Allocate vector register */
	int vreg = AllocVReg();
	Arg *ret_arg = new ArgVectorRegister(vreg);

	/* Copy constant to vector register.
	 * v_mov_b32 <vreg>, <const>
	 */
	Inst *inst = new Inst(SI::INST_V_MOV_B32,
			new ArgVectorRegister(vreg),
			arg);
	basic_block->AddInst(inst);

	/* Return new argument */
	return ret_arg;
}


int Function::AllocSReg(int count, int align)
{
	num_sregs = (num_sregs + align - 1) / align * align;
	num_sregs += count;
	return num_sregs - count;
}


int Function::AllocVReg(int count, int align)
{
	num_vregs = (num_vregs + align - 1) / align * align;
	num_vregs += count;
	return num_vregs - count;
}


}  /* namespace llvm2si */
