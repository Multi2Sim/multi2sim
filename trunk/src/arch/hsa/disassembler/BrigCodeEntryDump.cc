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

#include "AsmService.h"
#include "Brig.h"
#include "BrigCodeEntry.h"
#include "BrigOperandEntry.h"
#include "BrigSection.h"
#include "Disassembler.h"


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
	{BRIG_KIND_DIRECTIVE_MODULE, &BrigCodeEntry::DumpDirModule},
	{BRIG_KIND_DIRECTIVE_PRAGMA, &BrigCodeEntry::DumpDirPragma},
	{BRIG_KIND_DIRECTIVE_SIGNATURE,  &BrigCodeEntry::DumpDirSignature},
	{BRIG_KIND_DIRECTIVE_VARIABLE, &BrigCodeEntry::DumpDirVariable},

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
	Disassembler *disassembler = Disassembler::getInstance();
	for(int i = 0; i < disassembler->getIndent(); i++)
		os << "\t";
}


void BrigCodeEntry::DumpDirArgBlockEnd(std::ostream &os = std::cout) const
{
	Disassembler *disassembler = Disassembler::getInstance();

	// First reduce the indent level, then output the close bracket
	disassembler->IndentLess();
	DumpIndent(os);
	os << "}\n";
}


void BrigCodeEntry::DumpDirArgBlockStart(std::ostream &os = std::cout) const
{
	Disassembler *disassembler = Disassembler::getInstance();
	DumpIndent(os);
	os << "{\n";

	// All arguments within the bracket should indent one more level
	disassembler->IndentMore();
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
	DumpIndent(os);
	AsmService::DumpWithSpace(AsmService::DeclToString(!isDefinition()), 
			os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(getLinkage()), 
			os);
	os << "fbarrier " << getName() << ";\n";

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


void BrigCodeEntry::DumpDirModule(std::ostream &os = std::cout) const
{
	os << "module ";
	os << getName() << ":";
	os << getHsailMajor() << ":";
	os << getHsailMinor() <<  ":";
	os << AsmService::ProfileToString(getProfile()) << ":";
	os << AsmService::MachineModelToString(getMachineModel()) << ":";
	std::string roundStr = AsmService::RoundingToString(
			getDefaultFloatRound());
	if (roundStr == "")
		roundStr = "$default";
	os << roundStr << ";\n";
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


void BrigCodeEntry::DumpInstAddr(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());	
	AsmService::DumpUnderscore(AsmService::SegmentToString(getSegment()), 
			os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()),
			os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstAtomic(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::AtomicOperationToString(
			getAtomicOperation()), os);
	AsmService::DumpUnderscore(AsmService::SegmentToString(
			getSegment()), os);
	AsmService::DumpUnderscore(AsmService::MemoryOrderToString(
			getMemoryOrder()), os);
	AsmService::DumpUnderscore(AsmService::MemoryScopeToString(
			getMemoryScope()), os);

	// Dump equiv class
	if (getEquivClass() != 0)
	{
		AsmService::DumpUnderscore(AsmService::EquivClassToString(
				getEquivClass()), os);
	}
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstBasic(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	if (useType())
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
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());

	// Dump width
	if (getDefaultWidth() != getWidth())
		AsmService::DumpUnderscore(AsmService::WidthToString(
				getWidth()), os);

	// Dump type
	if (useType()) 
		AsmService::DumpUnderscore(AsmService::TypeToString(
				getType()), os);

	// Dump operands
	if (isCallInst())
		DumpCallArguments(os);
	else if (getOpcode() == BRIG_OPCODE_SBR)
		DumpSbrArguments(os);
	else 
		DumpOperands(os);

	// Finish dumping
	os << ";\n";
}


void BrigCodeEntry::DumpInstCmp(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::CompareOperationToString(
			getCompareOperation()), os);
	AsmService::DumpUnderscore(AsmService::AluModifierToString(
			getAluModifier()), os);
	// AsmService::DumpUnderscore(AsmService::RoundingToString(
	//		getRounding()), os);
	AsmService::DumpUnderscore(AsmService::PackToString(
			getPack()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getSourceType()), 
			os);

	// Dump operands
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstCvt(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::AluModifierToString(
			getAluModifier()), os);
	if (getRound() != getDefaultRounding())
	{
		AsmService::DumpUnderscore(AsmService::RoundingToString(
				getRound()), os);
	}
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getSourceType()), 
			os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstImage(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::VectorModifierToString(
			getVectorModifier()), os);
	AsmService::DumpUnderscore(AsmService::ImageGeometryToString(
			getGeometry()), os);
	if (getEquivClass() != 0)
	{
		AsmService::DumpUnderscore(AsmService::EquivClassToString(
				getEquivClass()), os);
	}
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getImageType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getCoordType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstLane(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::VectorModifierToString(
			getVectorModifier()), os);
	if (getWidth() != getDefaultWidth())
	{
		AsmService::DumpUnderscore(AsmService::WidthToString(
				getWidth()), os);
	}
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getSourceType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstMem(std::ostream &os = std::cout) const
{
	// Dump indent
	DumpIndent(os);

	// Dump instruction
	os << AsmService::OpcodeToString(getOpcode());	
	AsmService::DumpUnderscore(AsmService::VectorModifierToString(
			getVectorModifier()), os);
	if (getOpcode() != BRIG_OPCODE_ALLOCA)
		AsmService::DumpUnderscore(AsmService::SegmentToString(
				getSegment()), os);
	AsmService::DumpUnderscore(AsmService::AlignToString(getAlign()), os);
	AsmService::DumpUnderscore(AsmService::ConstToString(isConst()), os);

	// Print equivalance class
	if (getEquivClass() != 0)
	{
		AsmService::DumpUnderscore(AsmService::EquivClassToString(
				getEquivClass()), os);
	}

	// Print width
	if (getWidth() != getDefaultWidth())
	{
		AsmService::DumpUnderscore(AsmService::WidthToString(
				getWidth()), os);
	}

	// Print type
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);

	// Dump operands;
	DumpOperands(os);

	// Finish dumping
	os << ";\n";
}


void BrigCodeEntry::DumpInstMemFence(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::MemoryOrderToString(
			getMemoryOrder()), os);
	if (getGlobalSegmentScope() != BRIG_MEMORY_SCOPE_NONE)
	{
		AsmService::DumpUnderscore(AsmService::MemoryScopeToString(
				getGlobalSegmentScope()), os);
	}
	os << ";\n";
}


void BrigCodeEntry::DumpInstMod(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::AluModifierToString(
			getAluModifier()), os);
	if (getRound() != getDefaultRounding())
		AsmService::DumpUnderscore(AsmService::RoundingToString(
				getRound()), os);
	AsmService::DumpUnderscore(AsmService::PackToString(getPack()), os);
	if (useType())
		AsmService::DumpUnderscore(AsmService::TypeToString(getType()), 
				os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstQueryImage(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::ImageGeometryToString(
			getGeometry()), os);
	AsmService::DumpUnderscore(AsmService::ImageQueryToString(
			getImageQuery()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getImageType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstQuerySampler(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::SamplerQueryToString(
			getSamplerQuery()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstQueue(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::SegmentToString(
			getSegment()), os);
	AsmService::DumpUnderscore(AsmService::MemoryOrderToString(
			getMemoryOrder()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstSeg(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::SegmentToString(
			getSegment()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstSegCvt(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::SegmentToString(getSegment()), 
			os);
	AsmService::DumpUnderscore(AsmService::NoNullToString(isNoNull()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getSourceType()), 
			os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstSignal(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::AtomicOperationToString(
			getSignalOperation()), os);
	AsmService::DumpUnderscore(AsmService::MemoryOrderToString(
			getMemoryOrder()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(
			getSignalType()), os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpInstSourceType(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << AsmService::OpcodeToString(getOpcode());
	AsmService::DumpUnderscore(AsmService::VectorModifierToString(
			getVectorModifier()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);
	AsmService::DumpUnderscore(AsmService::TypeToString(getSourceType()), 
			os);
	DumpOperands(os);
	os << ";\n";
}


void BrigCodeEntry::DumpSymbolDeclaration(std::ostream &os = std::cout) const
{
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);
	AsmService::DumpWithSpace(AsmService::AllocationToString(
			getAllocation(), getSegment()), os);
	// FIXME: add align support
	AsmService::DumpWithSpace(AsmService::ConstToString(
			isConst()), os);

	// Dump memory segment and type
	os << AsmService::SegmentToString(getSegment());
	AsmService::DumpUnderscore(AsmService::TypeToString(getType()), os);

	// Dump name if exist
	if (getName() != "")
	{
		os << " " << getName();
	}

	// Dump square bracket if the symbol is array
	if (isArray() && getDim() == 0)
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
		Disassembler::getInstance()->IndentMore();	
		for (unsigned int i = 0; i < count; i++)
		{
			os << "\n";
			DumpIndent(os);
			arg->DumpSymbolDeclaration(os);
			if (i > 1)
				os << ",";
			arg = arg->Next();
		}	
		Disassembler::getInstance()->IndentLess();
	}
	os << ")";
}


void BrigCodeEntry::DumpBody(std::unique_ptr<BrigCodeEntry> start, 
		std::unique_ptr<BrigCodeEntry> end,
		std::ostream &os = std::cout) const
{
	os << "\n{\n";
	
	Disassembler::getInstance()->IndentMore();
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
	Disassembler::getInstance()->IndentLess();
	
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
		BrigType type = getOperandType(i);
		operand->Dump(type, os);
	}	
}


void BrigCodeEntry::DumpCallArguments(std::ostream &os = std::cout) const
{
	if (!isCallInst())
		throw misc::Panic("DumpCallArgument not valid for kind");

	os << "\t";

	auto operand1 = getOperand(1);
	operand1->Dump(BRIG_TYPE_NONE, os);

	auto operand0 = getOperand(0);
	if (operand0.get())
	{
		os << " ";
		operand0->Dump(BRIG_TYPE_NONE, os);
	}

	if (getOperandCount() >= 3)
	{
		auto operand2 = getOperand(2);
		if (operand2.get())
		{
			os << " ";
			operand2->Dump(BRIG_TYPE_NONE, os);
		}
	}

	if (getOperandCount() >= 4)
	{
		auto operand3 = getOperand(3);
		if (operand3->getKind() == BRIG_KIND_OPERAND_CODE_LIST)
		{
			os << " [";
			for (unsigned int i = 0; 
				i < operand3->getElementCount(); i++)
			{
				if (i > 0) os << ", ";
				os << operand3->getElement(i)->getName();
			}
			os << "]";
		}
		else if(operand3->getKind() == BRIG_KIND_OPERAND_CODE_REF)
		{
			os << " ";	
			operand3->Dump(BRIG_TYPE_NONE, os);
		}
	}
}


void BrigCodeEntry::DumpSbrArguments(std::ostream &os = std::cout) const
{
	os << "\t";
	auto operand0 = getOperand(0);
	operand0->Dump(BRIG_TYPE_NONE, os);
	os << " ";

	// Print operand code list in square barcket
	auto operand1 = getOperand(1);
	os << "[";
	for (unsigned int i = 0; i < operand1->getElementCount(); i++)
	{
		if (i > 0) 
			os << ", "	;
		os << operand1->getElement(i)->getName();
	}
	os << "]";
}


void BrigCodeEntry::KindError(const std::string &str) const
{
	throw misc::Panic(misc::fmt("%s not valid for kind %s", 
			str.c_str(), 
			AsmService::KindToString(getKind()).c_str()));
}

}  // namespace HSA


