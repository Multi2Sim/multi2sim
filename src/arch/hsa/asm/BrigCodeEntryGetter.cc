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
#include <lib/cpp/Error.h>

#include "BrigDef.h"
#include "BrigDataEntry.h"
#include "BrigSection.h"
#include "BrigFile.h"
#include "BrigOperandEntry.h"
#include "BrigCodeEntry.h"

namespace HSA
{

std::unique_ptr<BrigCodeEntry> BrigCodeEntry::Next() const
{
	unsigned int next_offset = getOffset() + getSize();
	std::unique_ptr<BrigCodeEntry> next_entry =
			section->getEntryByOffset<BrigCodeEntry>(next_offset);
	return next_entry;
}


std::unique_ptr<BrigCodeEntry> BrigCodeEntry::NextTopLevelEntry() const
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


unsigned BrigCodeEntry::getSize() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return brig_base->byteCount;
}


unsigned BrigCodeEntry::getKind() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return (unsigned)brig_base->kind;
}


bool BrigCodeEntry::isInstruction() const
{
	if (getKind() >= BRIG_KIND_INST_BEGIN 
			&& getKind() < BRIG_KIND_INST_END)
		return true;
	return false;
}


unsigned int BrigCodeEntry::getHsailMajor() const
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


unsigned int BrigCodeEntry::getHsailMinor() const
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


unsigned int BrigCodeEntry::getBrigMajor() const
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


unsigned int BrigCodeEntry::getBrigMinor() const
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


BrigProfile BrigCodeEntry::getProfile() const
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


BrigMachineModel BrigCodeEntry::getMachineModel() const
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


std::string BrigCodeEntry::getName() const
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
	case BRIG_KIND_DIRECTIVE_FBARRIER:
	{
		struct BrigDirectiveFbarrier *dir = 
				(struct BrigDirectiveFbarrier *)base;
		return getBinary()->getStringByOffset(dir->name);
	}
	default: 
		throw misc::Panic(misc::fmt("Name field are not valid "
				"for entry 0x%x\n", getKind()));
	}
	return "";
}


bool BrigCodeEntry::isDefinition() const
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


bool BrigCodeEntry::isConst() const
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
	case BRIG_KIND_INST_MEM:
	{
		struct BrigInstMem *dir = 
				(struct BrigInstMem *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_SYMBOL_CONST);
	}
	default:
		throw misc::Panic("Get isConst is not valid for type\n");
	}
}


bool BrigCodeEntry::isArray() const
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


bool BrigCodeEntry::isFlexArray() const
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


unsigned char BrigCodeEntry::getLinkage() const
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


unsigned char BrigCodeEntry::getAllocation() const
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


unsigned char BrigCodeEntry::getSegment() const
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
	case BRIG_KIND_INST_MEM:
	{
		struct BrigInstMem *inst = 
				(struct BrigInstMem *)base;
		unsigned char segment = inst->segment;
		return segment;
	}
	default:
		throw misc::Panic("Get segment is not valid for kind.\n");
	}
}


unsigned long long BrigCodeEntry::getDim() const
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


unsigned short BrigCodeEntry::getOutArgCount() const
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


unsigned short BrigCodeEntry::getInArgCount() const
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


std::unique_ptr<BrigCodeEntry> BrigCodeEntry::getFirstInArg() const
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


std::unique_ptr<BrigCodeEntry> BrigCodeEntry::getFirstCodeBlockEntry() const
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


std::unique_ptr<BrigCodeEntry> BrigCodeEntry::getNextModuleEntry() const
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


BrigOpcode BrigCodeEntry::getOpcode() const
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


BrigTypeX BrigCodeEntry::getType() const
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


BrigAlignment BrigCodeEntry::getAlign() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_MEM:
	{
		struct BrigInstMem *inst = (struct BrigInstMem *)base;
		return (BrigAlignment)inst->align;
	}	
	default: 
		throw misc::Panic("Get align is not valid for kind\n");
	}	
}


std::unique_ptr<BrigDataEntry> BrigCodeEntry::getOperands() const
{
	if (getKind() >= BRIG_KIND_INST_BEGIN 
			&& getKind() < BRIG_KIND_INST_END)
	{
		struct BrigInstBase *inst = (struct BrigInstBase *)base;
		unsigned int offset = inst->operands;
		return getBinary()->getDataEntryByOffset(offset);
	}
	else if (getKind() == BRIG_KIND_DIRECTIVE_PRAGMA)
	{
		struct BrigDirectivePragma *dir = 
				(struct BrigDirectivePragma *)base;
		unsigned int offset = dir->operands;
		return getBinary()->getDataEntryByOffset(offset);
	}
	else
	{
		throw misc::Panic("Get operand is only vaid for instructions");
	}
	return std::unique_ptr<BrigDataEntry>(nullptr);
}


unsigned int BrigCodeEntry::getOperandCount() const
{
	if (isInstruction() || getKind() == BRIG_KIND_DIRECTIVE_PRAGMA)
	{
		return getOperands()->getByteCount() / 4;
	}
	else if (getKind() == BRIG_KIND_DIRECTIVE_PRAGMA)
	{
		return getOperands()->getByteCount() / 4;
	}
	else
	{
		throw misc::Panic("Get operand count is only vaid for instructions");
	}
}


std::unique_ptr<BrigOperandEntry> BrigCodeEntry::getOperand(unsigned char index) const
{
	if (isInstruction() || getKind() == BRIG_KIND_DIRECTIVE_PRAGMA)
	{
		std::unique_ptr<BrigDataEntry> operands = getOperands();
		if (index * 4 >= operands->getByteCount())
			throw misc::Panic("Operand index out of range");
		unsigned int offset = *(const unsigned int *)(
				operands->getBytes() + index * 4);
		return getBinary()->getOperandByOffset(offset);
	}
	else
	{
		throw misc::Panic("Get operand is only vaid for instructions");
	}
}


BrigTypeX BrigCodeEntry::getOperandType(unsigned char index) const
{
	// This function is only valid for instruction
	if (!isInstruction())
		throw misc::Panic("GetOperandType is only valid for"
				" instruction");

	// In default cases, return src type
	return getType();
}

}  // namespace HSA
