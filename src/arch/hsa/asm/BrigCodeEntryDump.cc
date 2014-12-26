/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include <lib/cpp/String.h>

#include "Asm.h"
#include "BrigDef.h"
#include "BrigSection.h"
#include "AsmService.h"
#include "BrigCodeEntry.h"
#include "BrigOperandEntry.h"

namespace HSA
{

std::map<unsigned, BrigCodeEntry::DumpEntryFn> BrigCodeEntry::dump_entry_fn = 
{
	{BRIG_KIND_DIRECTIVE_ARG_BLOCK_END, &BrigCodeEntry::DumpDirArgBlockEnd},
	{BRIG_KIND_DIRECTIVE_ARG_BLOCK_START, &BrigCodeEntry::DumpDirArgBlockStart},
	{BRIG_KIND_DIRECTIVE_COMMENT, &BrigCodeEntry::DumpDirComment},
	{BRIG_KIND_DIRECTIVE_CONTROL, &BrigCodeEntry::DumpDirControl},
	{BRIG_KIND_DIRECTIVE_EXTENSION,  &BrigCodeEntry::DumpDirExtension},
	{BRIG_KIND_DIRECTIVE_FBARRIER, &BrigCodeEntry::DumpDirFbarrier},
	{BRIG_KIND_DIRECTIVE_FUNCTION, &BrigCodeEntry::DumpDirFunction},
	{BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION,  &BrigCodeEntry::DumpDirIndirectFunction},
	{BRIG_KIND_DIRECTIVE_KERNEL, &BrigCodeEntry::DumpDirKernel},
	{BRIG_KIND_DIRECTIVE_LABEL, &BrigCodeEntry::DumpDirLabel},
	{BRIG_KIND_DIRECTIVE_LOC, &BrigCodeEntry::DumpDirLoc},
	{BRIG_KIND_DIRECTIVE_PRAGMA, &BrigCodeEntry::DumpDirPragma},
	{BRIG_KIND_DIRECTIVE_SIGNATURE,  &BrigCodeEntry::DumpDirSignature},
	{BRIG_KIND_DIRECTIVE_VARIABLE, &BrigCodeEntry::DumpDirVariable},
	{BRIG_KIND_DIRECTIVE_VERSION, &BrigCodeEntry::DumpDirVersion},

	{BRIG_KIND_INST_ADDR, &BrigCodeEntry::DumpInstAddr},
	{BRIG_KIND_INST_ATOMIC, &BrigCodeEntry::DumpInstAtomic},
	{BRIG_KIND_INST_BASIC, &BrigCodeEntry::DumpInstBasic},
	{BRIG_KIND_INST_BR, &BrigCodeEntry::DumpInstBr},
	{BRIG_KIND_INST_CMP, &BrigCodeEntry::DumpInstCmp},
	{BRIG_KIND_INST_CVT, &BrigCodeEntry::DumpInstCvt},
	{BRIG_KIND_INST_IMAGE, &BrigCodeEntry::DumpInstImage},
	{BRIG_KIND_INST_LANE, &BrigCodeEntry::DumpInstLane},
	{BRIG_KIND_INST_MEM, &BrigCodeEntry::DumpInstMem},
	{BRIG_KIND_INST_MEM_FENCE, &BrigCodeEntry::DumpInstMemFence},
	{BRIG_KIND_INST_MOD, &BrigCodeEntry::DumpInstMod},
	{BRIG_KIND_INST_QUERY_IMAGE, &BrigCodeEntry::DumpInstQueryImage},
	{BRIG_KIND_INST_QUERY_SAMPLER, &BrigCodeEntry::DumpInstQuerySampler},
	{BRIG_KIND_INST_QUEUE, &BrigCodeEntry::DumpInstQueue},
	{BRIG_KIND_INST_SEG, &BrigCodeEntry::DumpInstSeg},
	{BRIG_KIND_INST_SEG_CVT, &BrigCodeEntry::DumpInstSegCvt},
	{BRIG_KIND_INST_SIGNAL,  &BrigCodeEntry::DumpInstSignal},
	{BRIG_KIND_INST_SOURCE_TYPE, &BrigCodeEntry::DumpInstSourceType},
};


void BrigCodeEntry::DumpIndent(std::ostream &os = std::cout) const
{
	Asm *as = Asm::getInstance();
	for(int i = 0; i < as->getIndent(); i++)
		os << "\t";
}


void BrigCodeEntry::DumpDirArgBlockEnd(std::ostream &os = std::cout) const
{
	Asm *as = Asm::getInstance();

	// First reduce the indent level, then output the close bracket
	as->IndentLess();
	DumpIndent(os);
	os << "}\n";
}


void BrigCodeEntry::DumpDirArgBlockStart(std::ostream &os = std::cout) const
{
	Asm *as = Asm::getInstance();
	DumpIndent(os);
	os << "{\n";

	// All arguments within the bracket should indent one more level
	as->IndentMore();
}


void BrigCodeEntry::DumpDirComment(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << getName() << "\n";
}


void BrigCodeEntry::DumpDirControl(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "CONTROL");
}


void BrigCodeEntry::DumpDirExtension(std::ostream &os = std::cout) const
{
	os << "extension ";
	AsmService::DumpStringLiteral(getName(), os);
	os << ";\n";
}


void BrigCodeEntry::DumpDirFbarrier(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "FBarrier");
}


void BrigCodeEntry::DumpDirFunction(std::ostream &os = std::cout) const
{
	// Dump prefix
	os << "\n";
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);

	// Dump name
	os << "function " << getName();

	// Dump arguments
	DumpArguments(Next(), getOutArgCount(), os);
	DumpArguments(getFirstInArg(), getInArgCount(), os);

	// Dump body
	if (isDefinition())
	{
		DumpBody(getFirstCodeBlockEntry(), getNextModuleEntry(), os);
	}

	os << ";\n";
}


void BrigCodeEntry::DumpDirIndirectFunction(std::ostream &os = std::cout) const
{
	// Dump prefix
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);

	// Dump function name
	os << "indirect function " << getName();

	// Dump arguments
	DumpArguments(Next(), getOutArgCount(), os);
	DumpArguments(getFirstInArg(), getInArgCount(), os);

	// Dump body
	// Dump body
	if (isDefinition())
	{
		DumpBody(getFirstCodeBlockEntry(), getNextModuleEntry(), os);
	}

	os << ";\n";
}


void BrigCodeEntry::DumpDirKernel(std::ostream &os = std::cout) const
{
	// Dump prefix
	os << "\n";
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);

	// Dump name
	os << "kernel " << getName();

	// Dump arguments
	DumpArguments(getFirstInArg(), getInArgCount(), os);

	// Dump body
	if (isDefinition())
	{
		DumpBody(getFirstCodeBlockEntry(), getNextModuleEntry(), os);
	}

	os << ";\n";
}


void BrigCodeEntry::DumpDirLabel(std::ostream &os = std::cout) const
{
	os << "\n";
	os << getName();
	os << ":\n";
}


void BrigCodeEntry::DumpDirLoc(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "LOC");
}


void BrigCodeEntry::DumpDirPragma(std::ostream &os = std::cout) const
{
	os << "pragma ";

	if (getOperandCount() > 0)
		os << ' ';
	for (unsigned i = 0; i < getOperandCount(); i++)
	{
		if (i > 0) 
			os << ", ";
		auto operand = getOperand(i);
		operand->Dump(BRIG_TYPE_U64, os);
	}

	os << ";\n";
}


void BrigCodeEntry::DumpDirSignature(std::ostream &os = std::cout) const
{
	// Dump prefix
	os << "\n";
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);

	// Dump signature name
	os << "signature " << getName();

	// Dump arguments
	DumpArguments(Next(), getOutArgCount(), os);
	DumpArguments(getFirstInArg(), getInArgCount(), os);
	os << ";\n";
}


void BrigCodeEntry::DumpDirVariable(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	DumpSymbolDeclaration(os);

	// FIXME Deal with variable init

	os << ";\n";
}


void BrigCodeEntry::DumpDirVersion(std::ostream &os = std::cout) const
{
	os << "version ";
	os << getHsailMajor() << ":";
	os << getHsailMinor() << ":";
	os << AsmService::ProfileToString(getProfile()) << ":";
	os << AsmService::MachineModelToString(getMachineModel()) << ";";
	os << "\n";
}


void BrigCodeEntry::DumpInstAddr(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "ADDR");
}


void BrigCodeEntry::DumpInstAtomic(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "ATOMIC");
}


void BrigCodeEntry::DumpInstBasic(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	if (AsmService::InstUseType(getOpcode()))
	{
		AsmService::DumpUnderscore(
				AsmService::TypeToString(getType()), os);
	}

	// Dump operands;
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstBr(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "BR");
}


void BrigCodeEntry::DumpInstCmp(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "CMP");
}


void BrigCodeEntry::DumpInstCvt(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "CVT");
}


void BrigCodeEntry::DumpInstImage(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "IMAGE");
}


void BrigCodeEntry::DumpInstLane(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "LANE");
}


void BrigCodeEntry::DumpInstMem(std::ostream &os = std::cout) const
{
	// Dump indent
	DumpIndent(os);

	// Dump instruction
	os << AsmService::OpcodeToString(getOpcode());	
	if (getOpcode() != BRIG_OPCODE_ALLOCA)
		AsmService::DumpUnderscore(AsmService::SegmentToString(
				getSegment()), os);
	AsmService::DumpUnderscore(AsmService::AlignToString(getAlign()), os);
	AsmService::DumpUnderscore(AsmService::ConstToString(isConst()), os);

	// TODO: equiv2str & width

	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);

	// Dump operands;
	DumpOperands(os);

	// Finish dumping
	os << ";\n";
}


void BrigCodeEntry::DumpInstMemFence(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "MEM_FENCE");
}


void BrigCodeEntry::DumpInstMod(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "ADDR");
}


void BrigCodeEntry::DumpInstQueryImage(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "QUERY_IMAGE");
}


void BrigCodeEntry::DumpInstQuerySampler(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "QUERY_SAMPLER");
}


void BrigCodeEntry::DumpInstQueue(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "QUEUE");
}


void BrigCodeEntry::DumpInstSeg(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SEG");
}


void BrigCodeEntry::DumpInstSegCvt(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SEG_CVT");
}


void BrigCodeEntry::DumpInstSignal(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SIGNAL");
}


void BrigCodeEntry::DumpInstSourceType(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SOURCE_TYPE");
}


void BrigCodeEntry::DumpSymbolDeclaration(std::ostream &os = std::cout) const
{
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);
	AsmService::DumpWithSpace(AsmService::AllocationToString(
			getAllocation()), os);
	// FIXME: add align and const support

	// Dump memory segment and type
	os << AsmService::SegmentToString(getSegment());
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);

	// Dump name if exist
	if (getName() != "")
	{
		os << " " << getName();
	}

	// Dump square bracket if the symbol is array
	if (isFlexArray())
	{
		os << "[]";
	}
	else if(isArray())
	{
		os << "[" << getDim() << "]";
	}

}


void BrigCodeEntry::DumpArguments(std::unique_ptr<BrigCodeEntry> arg, 
			unsigned int count, 
			std::ostream &os = std::cout) const
{
	os << "(";
	if (count == 1)
	{
		arg->DumpSymbolDeclaration(os);
	}
	else if(count > 1)
	{
		Asm::getInstance()->IndentMore();	
		for (unsigned int i = 0; i < count; i++)
		{
			os << "\n";
			DumpIndent(os);
			arg->DumpSymbolDeclaration(os);
			if (i > 1)
				os << ",";
			arg = arg->Next();
		}	
		Asm::getInstance()->IndentLess();
	}
	os << ")";
}


void BrigCodeEntry::DumpBody(std::unique_ptr<BrigCodeEntry> start, 
		std::unique_ptr<BrigCodeEntry> end,
		std::ostream &os = std::cout) const
{
	os << "\n{\n";
	
	Asm::getInstance()->IndentMore();
	//os << misc::fmt("end: %p\n", end.get());
	std::unique_ptr<BrigCodeEntry> entry = std::move(start);
	while(true)
	{
		// Terminate if end reached
		if (entry.get() == nullptr)
			break;	
		else if (end.get() && end->base == entry->base)
			break;

		// Dump entry
		entry->Dump(os);
		entry = entry->Next();
		
	}
	Asm::getInstance()->IndentLess();
	
	os << "}";
}


void BrigCodeEntry::DumpOperands(std::ostream &os = std::cout) const
{
	// Print a tab
	if (getOperandCount() > 0)
		os << "\t";

	// Traverse all the operands and print
	for (unsigned int i = 0; i < getOperandCount(); i++)
	{
		if (i > 0) 
			os << ", ";
		std::unique_ptr<BrigOperandEntry> operand = getOperand(i);
		BrigTypeX type = getOperandType(i);
		operand->Dump(type, os);
	}	
}

}  // namespace HSA


