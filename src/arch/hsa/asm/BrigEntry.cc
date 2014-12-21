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
#include "BrigEntry.h"

namespace HSA
{

std::map<unsigned, BrigEntry::DumpEntryFn> BrigEntry::dump_entry_fn = 
{
	{BRIG_KIND_DIRECTIVE_ARG_BLOCK_END, &BrigEntry::DumpDirArgBlockEnd},
	{BRIG_KIND_DIRECTIVE_ARG_BLOCK_START, &BrigEntry::DumpDirArgBlockStart},
	{BRIG_KIND_DIRECTIVE_COMMENT, &BrigEntry::DumpDirComment},
	{BRIG_KIND_DIRECTIVE_CONTROL, &BrigEntry::DumpDirControl},
	{BRIG_KIND_DIRECTIVE_EXTENSION,  &BrigEntry::DumpDirExtension},
	{BRIG_KIND_DIRECTIVE_FBARRIER, &BrigEntry::DumpDirFbarrier},
	{BRIG_KIND_DIRECTIVE_FUNCTION, &BrigEntry::DumpDirFunction},
	{BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION,  &BrigEntry::DumpDirIndirectFunction},
	{BRIG_KIND_DIRECTIVE_KERNEL, &BrigEntry::DumpDirKernel},
	{BRIG_KIND_DIRECTIVE_LABEL, &BrigEntry::DumpDirLabel},
	{BRIG_KIND_DIRECTIVE_LOC, &BrigEntry::DumpDirLoc},
	{BRIG_KIND_DIRECTIVE_PRAGMA, &BrigEntry::DumpDirPragma},
	{BRIG_KIND_DIRECTIVE_SIGNATURE,  &BrigEntry::DumpDirSignature},
	{BRIG_KIND_DIRECTIVE_VARIABLE, &BrigEntry::DumpDirVariable},
	{BRIG_KIND_DIRECTIVE_VERSION, &BrigEntry::DumpDirVersion},

	{BRIG_KIND_INST_ADDR, &BrigEntry::DumpInstAddr},
	{BRIG_KIND_INST_ATOMIC, &BrigEntry::DumpInstAtomic},
	{BRIG_KIND_INST_BASIC, &BrigEntry::DumpInstBasic},
	{BRIG_KIND_INST_BR, &BrigEntry::DumpInstBr},
	{BRIG_KIND_INST_CMP, &BrigEntry::DumpInstCmp},
	{BRIG_KIND_INST_CVT, &BrigEntry::DumpInstCvt},
	{BRIG_KIND_INST_IMAGE, &BrigEntry::DumpInstImage},
	{BRIG_KIND_INST_LANE, &BrigEntry::DumpInstLane},
	{BRIG_KIND_INST_MEM, &BrigEntry::DumpInstMem},
	{BRIG_KIND_INST_MEM_FENCE, &BrigEntry::DumpInstMemFence},
	{BRIG_KIND_INST_MOD, &BrigEntry::DumpInstMod},
	{BRIG_KIND_INST_QUERY_IMAGE, &BrigEntry::DumpInstQueryImage},
	{BRIG_KIND_INST_QUERY_SAMPLER, &BrigEntry::DumpInstQuerySampler},
	{BRIG_KIND_INST_QUEUE, &BrigEntry::DumpInstQueue},
	{BRIG_KIND_INST_SEG, &BrigEntry::DumpInstSeg},
	{BRIG_KIND_INST_SEG_CVT, &BrigEntry::DumpInstSegCvt},
	{BRIG_KIND_INST_SIGNAL,  &BrigEntry::DumpInstSignal},
	{BRIG_KIND_INST_SOURCE_TYPE, &BrigEntry::DumpInstSourceType},

	{BRIG_KIND_OPERAND_ADDRESS, &BrigEntry::DumpOperandAddress},
	{BRIG_KIND_OPERAND_DATA, &BrigEntry::DumpOperandData},
	{BRIG_KIND_OPERAND_CODE_LIST, &BrigEntry::DumpOperandCodeList},
	{BRIG_KIND_OPERAND_CODE_REF, &BrigEntry::DumpOperandCodeRef},
	{BRIG_KIND_OPERAND_IMAGE_PROPERTIES, &BrigEntry::DumpOperandImageProperties},
	{BRIG_KIND_OPERAND_OPERAND_LIST, &BrigEntry::DumpOperandOperandList},
	{BRIG_KIND_OPERAND_REG,  &BrigEntry::DumpOperandReg},
	{BRIG_KIND_OPERAND_SAMPLER_PROPERTIES, &BrigEntry::DumpOperandSamplerProperties},
	{BRIG_KIND_OPERAND_STRING, &BrigEntry::DumpOperandString},
	{BRIG_KIND_OPERAND_WAVESIZE, &BrigEntry::DumpOperandWavesize},
};


BrigEntry::BrigEntry(const char *buf, const BrigSection *section):
		base(buf),
		section(section)
{
}


BrigFile *BrigEntry::getBinary() const
{
	return section->getBinary();
}


unsigned BrigEntry::getSize() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return brig_base->byteCount;
}


unsigned BrigEntry::getKind() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return (unsigned)brig_base->kind;
}


unsigned int BrigEntry::getOffset() const
{
	return base - section->getBuffer();
}


std::unique_ptr<BrigEntry> BrigEntry::Next() const
{
	unsigned int next_offset = getOffset() + getSize();
	return section->getEntryByOffset(next_offset);
}


std::unique_ptr<BrigEntry> BrigEntry::NextTopLevelEntry() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		return getNextModuleEntry();
	}
	default:
	{
		return Next();
	}
	}
}



void BrigEntry::DumpIndent(std::ostream &os = std::cout) const
{
	Asm *as = Asm::getInstance();
	for(int i = 0; i < as->getIndent(); i++)
		os << "\t";
}


void BrigEntry::DumpDirArgBlockEnd(std::ostream &os = std::cout) const
{
	Asm *as = Asm::getInstance();

	// First reduce the indent level, then output the close bracket
	as->IndentLess();
	DumpIndent(os);
	os << "}\n";
}


void BrigEntry::DumpDirArgBlockStart(std::ostream &os = std::cout) const
{
	Asm *as = Asm::getInstance();
	DumpIndent(os);
	os << "{\n";

	// All arguments within the bracket should indent one more level
	as->IndentMore();
}


void BrigEntry::DumpDirComment(std::ostream &os = std::cout) const
{
	DumpIndent(os);
	os << getName() << "\n";
}


void BrigEntry::DumpDirControl(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "CONTROL");
}


void BrigEntry::DumpDirExtension(std::ostream &os = std::cout) const
{
	os << "extension ";
	AsmService::DumpStringLiteral(getName(), os);
	os << ";\n";
}


void BrigEntry::DumpDirFbarrier(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "FBarrier");
}


void BrigEntry::DumpDirFunction(std::ostream &os = std::cout) const
{
	// Dump prefix
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


void BrigEntry::DumpDirIndirectFunction(std::ostream &os = std::cout) const
{
	// Dump prefix
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);

	// Dump function name
	os << "inderict function " << getName();

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


void BrigEntry::DumpDirKernel(std::ostream &os = std::cout) const
{
	// Dump prefix
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


void BrigEntry::DumpDirLabel(std::ostream &os = std::cout) const
{
	os << "\n";
	os << getName();
	os << ":\n";
}


void BrigEntry::DumpDirLoc(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "LOC");
}


void BrigEntry::DumpDirPragma(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "PRAGMA");
}


void BrigEntry::DumpDirSignature(std::ostream &os = std::cout) const
{
	AsmService::DumpWithSpace(AsmService::DeclToString(
			!isDefinition()), os);
	AsmService::DumpWithSpace(AsmService::LinkageToString(
			getLinkage()), os);

	os << "signature " << getName();

	// Dump arguments
	DumpArguments(Next(), getOutArgCount(), os);
	DumpArguments(getFirstInArg(), getInArgCount(), os);

	os << ";\n";
}


void BrigEntry::DumpDirVariable(std::ostream &os = std::cout) const
{
	DumpSymbolDeclaration(os);

	// FIXME Deal with variable init

	os << ";\n";
}


void BrigEntry::DumpDirVersion(std::ostream &os = std::cout) const
{
	os << "version ";
	os << getHsailMajor() << ":";
	os << getHsailMinor() << ":";
	os << AsmService::ProfileToString(getProfile()) << ":";
	os << AsmService::MachineModelToString(getMachineModel()) << ";";
	os << "\n";
}


void BrigEntry::DumpInstAddr(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "ADDR");
}


void BrigEntry::DumpInstAtomic(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "ATOMIC");
}


void BrigEntry::DumpInstBasic(std::ostream &os = std::cout) const
{
	os << AsmService::OpcodeToString(getOpcode());
	if (AsmService::InstUseType(getOpcode()))
	{
		AsmService::DumpUnderscore(
				AsmService::TypeToString(getType()), os);
	}
	os << "\n";
}


void BrigEntry::DumpInstBr(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "BR");
}


void BrigEntry::DumpInstCmp(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "CMP");
}


void BrigEntry::DumpInstCvt(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "CVT");
}


void BrigEntry::DumpInstImage(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "IMAGE");
}


void BrigEntry::DumpInstLane(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "LANE");
}


void BrigEntry::DumpInstMem(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "MEM");
}


void BrigEntry::DumpInstMemFence(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "MEM_FENCE");
}


void BrigEntry::DumpInstMod(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "ADDR");
}


void BrigEntry::DumpInstQueryImage(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "QUERY_IMAGE");
}


void BrigEntry::DumpInstQuerySampler(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "QUERY_SAMPLER");
}


void BrigEntry::DumpInstQueue(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "QUEUE");
}


void BrigEntry::DumpInstSeg(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SEG");
}


void BrigEntry::DumpInstSegCvt(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SEG_CVT");
}


void BrigEntry::DumpInstSignal(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SIGNAL");
}


void BrigEntry::DumpInstSourceType(std::ostream &os = std::cout) const
{
	os << misc::fmt("Instruction: %s, not supported\n", "SOURCE_TYPE");
}

	
void BrigEntry::DumpOperandAddress(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "ADDRESS");
}


void BrigEntry::DumpOperandData(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "DATA");
}


void BrigEntry::DumpOperandCodeList(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "CODE_LIST");
}


void BrigEntry::DumpOperandCodeRef(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "CODE_REF");
}


void BrigEntry::DumpOperandImageProperties(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "IMAGE_PROPERTIES");
}


void BrigEntry::DumpOperandOperandList(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "OPERAND_LIST");
}


void BrigEntry::DumpOperandReg(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "REG");
}


void BrigEntry::DumpOperandSamplerProperties(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "SAMPLER_PROPERTIES");
}


void BrigEntry::DumpOperandString(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "STRING");
}


void BrigEntry::DumpOperandWavesize(std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "WAVESIZE");
}


void BrigEntry::DumpSymbolDeclaration(std::ostream &os = std::cout) const
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


void BrigEntry::DumpArguments(std::unique_ptr<BrigEntry> arg, 
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
			DumpIndent();
			arg->DumpSymbolDeclaration(os);
			if (i > 1)
				os << ",";
			arg = arg->Next();
		}	
		Asm::getInstance()->IndentLess();
	}
	os << ")";
}


void BrigEntry::DumpBody(std::unique_ptr<BrigEntry> start, 
		std::unique_ptr<BrigEntry> end,
		std::ostream &os = std::cout) const
{
	os << "\n{\n";
	
	Asm::getInstance()->IndentMore();
	//os << misc::fmt("end: %p\n", end.get());
	std::unique_ptr<BrigEntry> entry = std::move(start);
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


unsigned int BrigEntry::getHsailMajor() const
{
	struct BrigDirectiveVersion *directive = 
			(struct BrigDirectiveVersion *)base;

	if (directive->base.kind != BRIG_KIND_DIRECTIVE_VERSION)
	{
		throw misc::Panic("hsailMajor field is only valid for "
				"directive version");
	}

	return directive->hsailMajor;
}


unsigned int BrigEntry::getHsailMinor() const
{
	struct BrigDirectiveVersion *directive = 
			(struct BrigDirectiveVersion *)base;

	if (directive->base.kind != BRIG_KIND_DIRECTIVE_VERSION)
	{
		throw misc::Panic("hsailMinor field is only valid for "
				"directive version");
	}

	return directive->hsailMinor;

}


unsigned int BrigEntry::getBrigMajor() const
{
	struct BrigDirectiveVersion *directive = 
			(struct BrigDirectiveVersion *)base;

	if (directive->base.kind != BRIG_KIND_DIRECTIVE_VERSION)
	{
		throw misc::Panic("brigMajor field is only valid for "
				"directive version");
	}

	return directive->brigMajor;

}


unsigned int BrigEntry::getBrigMinor() const
{
	struct BrigDirectiveVersion *directive = 
			(struct BrigDirectiveVersion *)base;

	if (directive->base.kind != BRIG_KIND_DIRECTIVE_VERSION)
	{
		throw misc::Panic("brigMinor field is only valid for "
				"directive version");
	}

	return directive->brigMinor;

}


BrigProfile BrigEntry::getProfile() const
{
	struct BrigDirectiveVersion *directive = 
			(struct BrigDirectiveVersion *)base;

	if (directive->base.kind != BRIG_KIND_DIRECTIVE_VERSION)
	{
		throw misc::Panic("brigMinor field is only valid for "
				"directive version");
	}

	return (BrigProfile)directive->profile;

}


BrigMachineModel BrigEntry::getMachineModel() const
{
	struct BrigDirectiveVersion *directive = 
			(struct BrigDirectiveVersion *)base;

	if (directive->base.kind != BRIG_KIND_DIRECTIVE_VERSION)
	{
		throw misc::Panic("brigMinor field is only valid for "
				"directive version");
	}

	return (BrigMachineModel)directive->machineModel;
}


std::string BrigEntry::getName() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_COMMENT:
	case BRIG_KIND_DIRECTIVE_LABEL:
	case BRIG_KIND_DIRECTIVE_EXTENSION:
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveComment *dir = 
				(struct BrigDirectiveComment *)base;
		return getBinary()->getStringByOffset(dir->name);
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;
		return getBinary()->getStringByOffset(dir->name);
	}
	default: 
		throw misc::Panic(misc::fmt("Name field are not valid "
				"for entry\n"));
	}
	return "";
}


bool BrigEntry::isDefinition() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_DEFINITION);
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_DEFINITION);
	}
	default:
		throw misc::Panic("Get isDefinition is not valid for type\n");
	}
}


bool BrigEntry::isConst() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_CONST);
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_CONST);
	}
	default:
		throw misc::Panic("Get isConst is not valid for type\n");
	}
}


bool BrigEntry::isArray() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_ARRAY);
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_ARRAY);
	}
	default:
		throw misc::Panic("Get isArray is not valid for type\n");
	}
}


bool BrigEntry::isFlexArray() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_FLEX_ARRAY);
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_FLEX_ARRAY);
	}
	default:
		throw misc::Panic("Get isFlexArray is not valid for type\n");
	}
}


unsigned char BrigEntry::getLinkage() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		unsigned char linkage = dir->linkage;
		return linkage;
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		return dir->linkage;
	}	
	default:
		throw misc::Panic("Get linkage is not valid for type.\n");
	}
}


unsigned char BrigEntry::getAllocation() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		unsigned char allocation = dir->allocation;
		return allocation;
	}
	default: 
		throw misc::Panic("Get allocation is not valid for kind.\n");
	}
}


unsigned char BrigEntry::getSegment() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		unsigned char segment = dir->segment;
		return segment;
	}
	default:
		throw misc::Panic("Get segment is not valid for kind.\n");
	}
}


unsigned long long BrigEntry::getDim() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		unsigned long long *dim = (unsigned long long *)&dir->dim;
		return *dim;
	}
	default:
		throw misc::Panic("Get dim is not valid for kind.\n");
	}
}


unsigned short BrigEntry::getOutArgCount() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		return dir->outArgCount;
	}	
	default:
		throw misc::Panic("Get outArgCount is not valid for kind.\n");
	}	
}


unsigned short BrigEntry::getInArgCount() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		return dir->inArgCount;
	}
	default:
		throw misc::Panic("Get inArgCount is not valid for kind.\n");	
	}	
}


std::unique_ptr<BrigEntry> BrigEntry::getFirstInArg() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned int offset =  dir->firstInArg;
		return getBinary()->getCodeEntryByOffset(offset);
	}
	default:
		throw misc::Panic("Get inArgCount is not valid for kind.\n");	
	}	
}


std::unique_ptr<BrigEntry> BrigEntry::getFirstCodeBlockEntry() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned int offset =  dir->firstCodeBlockEntry;
		return getBinary()->getCodeEntryByOffset(offset);
	}
	default:
		throw misc::Panic("Get firstCodeBlockEntry is not valid for "
			"kind.\n");	
	}
}


std::unique_ptr<BrigEntry> BrigEntry::getNextModuleEntry() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned int offset =  dir->nextModuleEntry;
		return getBinary()->getCodeEntryByOffset(offset);
	}
	default:
		throw misc::Panic("Get nextModuleEntry is not valid for kind.\n");	
	}
}


BrigOpcode BrigEntry::getOpcode() const
{
	/// Check if the entry is an inst
	if (getKind() < BRIG_KIND_INST_BEGIN 
			|| getKind() >= BRIG_KIND_INST_END)
	{
		throw misc::Panic(misc::fmt(
				"Getting opcode is invalid for "
				"entry of kind %d\n", getKind()));
	}

	// Return opcode	
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	return (BrigOpcode)inst->opcode;
}


BrigTypeX BrigEntry::getType() const
{
	/// Check if the entry is an inst
	if (getKind() >= BRIG_KIND_INST_BEGIN 
			&& getKind() < BRIG_KIND_INST_END)
	{
		// Return type
		struct BrigInstBase *inst = (struct BrigInstBase *)base;
		return (BrigTypeX)inst->type;
	}
	else if(getKind() == BRIG_KIND_DIRECTIVE_VARIABLE)
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		return (BrigTypeX)dir->type;
	}
	else
	{
		throw misc::Panic("Get type is not vaild for kind\n");
	}

	

}

}  // namespace HSA


