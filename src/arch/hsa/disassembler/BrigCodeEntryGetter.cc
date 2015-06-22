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

#include "Brig.h"
#include "AsmService.h"
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


BrigKind BrigCodeEntry::getKind() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return (BrigKind)brig_base->kind;
}


bool BrigCodeEntry::isInstruction() const
{
	if (getKind() >= BRIG_KIND_INST_BEGIN 
			&& getKind() < BRIG_KIND_INST_END)
		return true;
	return false;
}


bool BrigCodeEntry::useType() const
{
	if (!isInstruction()){
		throw misc::Panic("UseType not valid for type");
	}
	switch(getOpcode())
	{
	case BRIG_OPCODE_ARRIVEFBAR:
	case BRIG_OPCODE_BARRIER:
	case BRIG_OPCODE_BR:
	case BRIG_OPCODE_CALL:
	case BRIG_OPCODE_GCNMADS:
	case BRIG_OPCODE_GCNMADU:
	case BRIG_OPCODE_GCNMQSAD4:
	case BRIG_OPCODE_GCNREGIONALLOC:
	case BRIG_OPCODE_INITFBAR:
	case BRIG_OPCODE_JOINFBAR:
	case BRIG_OPCODE_LEAVEFBAR:
	case BRIG_OPCODE_NOP:
	case BRIG_OPCODE_RELEASEFBAR:
	case BRIG_OPCODE_RET:
	case BRIG_OPCODE_WAITFBAR:
	case BRIG_OPCODE_WAVEBARRIER:
		return false;
	default:
		return true;
	}
}


unsigned int BrigCodeEntry::getHsailMajor() const
{
	if (getKind() != BRIG_KIND_DIRECTIVE_MODULE)
		KindError("GetHsailMajor");
	BrigDirectiveModule *module = (BrigDirectiveModule *)base;	
	return module->hsailMajor;
}


unsigned int BrigCodeEntry::getHsailMinor() const
{
	if (getKind() != BRIG_KIND_DIRECTIVE_MODULE)
		KindError("GetHsailMinor");
	BrigDirectiveModule *module = (BrigDirectiveModule *)base;	
	return module->hsailMinor;
}


BrigProfile BrigCodeEntry::getProfile() const
{
	if (getKind() != BRIG_KIND_DIRECTIVE_MODULE)
		KindError("GetProfile");
	BrigDirectiveModule *module = (BrigDirectiveModule *)base;	
	return (BrigProfile)module->profile;
}


BrigMachineModel BrigCodeEntry::getMachineModel() const
{
	if (getKind() != BRIG_KIND_DIRECTIVE_MODULE)
		KindError("GetMachineModel");
	BrigDirectiveModule *module = (BrigDirectiveModule *)base;	
	return (BrigMachineModel)module->machineModel;
}


BrigRound BrigCodeEntry::getDefaultFloatRound() const
{
	if (getKind() != BRIG_KIND_DIRECTIVE_MODULE)
		KindError("GetDefaultFloatRound");
	BrigDirectiveModule *module = (BrigDirectiveModule *)base;	
	return (BrigRound)module->defaultFloatRound;
}


BrigWidth BrigCodeEntry::getDefaultWidth() const
{
	if (!isInstruction()){
		throw misc::Panic("UseType not valid for type");
	}
	switch(getOpcode())
	{
	case BRIG_OPCODE_ACTIVELANECOUNT: 
		return BRIG_WIDTH_1;
	case BRIG_OPCODE_ACTIVELANEPERMUTE: 
		return BRIG_WIDTH_1;
        case BRIG_OPCODE_ACTIVELANEID: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_ACTIVELANEMASK: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_ALLOCA: 
        	return BRIG_WIDTH_NONE;
        case BRIG_OPCODE_ARRIVEFBAR: 
        	return BRIG_WIDTH_WAVESIZE;
        case BRIG_OPCODE_BARRIER: 
        	return BRIG_WIDTH_ALL;
        case BRIG_OPCODE_BR: 
        	return BRIG_WIDTH_ALL;
        case BRIG_OPCODE_CALL: 
        	return BRIG_WIDTH_ALL;
        case BRIG_OPCODE_CBR: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_GCNLD: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_GCNST: 
        	return BRIG_WIDTH_NONE;
        case BRIG_OPCODE_ICALL: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_JOINFBAR: 
        	return BRIG_WIDTH_WAVESIZE;
        case BRIG_OPCODE_LD: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_LEAVEFBAR: 
        	return BRIG_WIDTH_WAVESIZE;
        case BRIG_OPCODE_SBR: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_SCALL: 
        	return BRIG_WIDTH_1;
        case BRIG_OPCODE_ST: 
        	return BRIG_WIDTH_NONE;
        case BRIG_OPCODE_WAITFBAR: 
        	return BRIG_WIDTH_WAVESIZE;
        case BRIG_OPCODE_WAVEBARRIER: 
        	return BRIG_WIDTH_WAVESIZE;
        default:
            return BRIG_WIDTH_NONE;
            break;
	}
	return BRIG_WIDTH_NONE;
}


BrigWidth BrigCodeEntry::getWidth() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_BR:
	{
		struct BrigInstBr *inst = (struct BrigInstBr *)base;
		return (BrigWidth)inst->width;
	}
	case BRIG_KIND_INST_MEM:
	{
		struct BrigInstMem *inst = (struct BrigInstMem *)base;
		return (BrigWidth)inst->width;
	}
	case BRIG_KIND_INST_LANE:
	{
		struct BrigInstLane *inst = (struct BrigInstLane *)base;
		return (BrigWidth)inst->width;
	}
	default:
		KindError("GetWidth");
	}
	return BRIG_WIDTH_NONE;
}


bool BrigCodeEntry::isCallInst() const
{
	if (!isInstruction())
		throw misc::Panic("IsCallInst not valid for kind");
	if (getOpcode() == BRIG_OPCODE_CALL || 
		getOpcode() == BRIG_OPCODE_SCALL || 
		getOpcode() == BRIG_OPCODE_ICALL)
		return true;
	return false;
}


std::string BrigCodeEntry::getName() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_MODULE:

	{
		BrigDirectiveModule *dir = (BrigDirectiveModule *)base;
		return getBinary()->getStringByOffset(dir->name);
	}

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
		return !!(modifier & BRIG_VARIABLE_DEFINITION);
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_VARIABLE_DEFINITION);
	}
	case BRIG_KIND_DIRECTIVE_FBARRIER:
	{
		struct BrigDirectiveFbarrier *dir = 
				(struct BrigDirectiveFbarrier *)base;
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_VARIABLE_DEFINITION);
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
		return !!(modifier & BRIG_VARIABLE_CONST);
	}
	case BRIG_KIND_DIRECTIVE_KERNEL:
	case BRIG_KIND_DIRECTIVE_FUNCTION:
	case BRIG_KIND_DIRECTIVE_SIGNATURE:
	case BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION:
	{
		struct BrigDirectiveExecutable *dir = 
				(struct BrigDirectiveExecutable *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_VARIABLE_CONST);
	}
	case BRIG_KIND_INST_MEM:
	{
		struct BrigInstMem *dir = 
				(struct BrigInstMem *)base;	
		unsigned char modifier = (unsigned char)dir->modifier.allBits;
		return !!(modifier & BRIG_MEMORY_CONST);
	}
	default:
		KindError("IsConst");
	}
	return false;
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
	case BRIG_KIND_DIRECTIVE_FBARRIER:
	{
		struct BrigDirectiveFbarrier *dir = 
				(struct BrigDirectiveFbarrier *)base;
		return dir->linkage;
	}	
	default:
		throw misc::Panic("Get linkage is not valid for type.\n");
	}
}


BrigAllocation BrigCodeEntry::getAllocation() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		unsigned char allocation = dir->allocation;
		return (BrigAllocation)allocation;
	}
	default: 
		throw misc::Panic("Get allocation is not valid for kind.\n");
	}
}


BrigSegment BrigCodeEntry::getSegment() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		unsigned char segment = dir->segment;
		return (BrigSegment)segment;
	}
	case BRIG_KIND_INST_MEM:
	{
		struct BrigInstMem *inst = 
				(struct BrigInstMem *)base;
		unsigned char segment = inst->segment;
		return (BrigSegment)segment;
	}
	case BRIG_KIND_INST_ADDR:
	{
		struct BrigInstAddr *inst = 
				(struct BrigInstAddr *)base;
		unsigned char segment = inst->segment;
		return (BrigSegment)segment;
	}
	case BRIG_KIND_INST_SEG_CVT:
	{
		struct BrigInstSegCvt *inst = 
				(struct BrigInstSegCvt *)base;
		unsigned char segment = inst->segment;
		return (BrigSegment)segment;
	}
	case BRIG_KIND_INST_ATOMIC:
	{
		struct BrigInstAtomic *inst = 
				(struct BrigInstAtomic *)base;
		unsigned char segment = inst->segment;
		return (BrigSegment)segment;
	}
	case BRIG_KIND_INST_QUEUE:
	{
		struct BrigInstQueue *inst = 
				(struct BrigInstQueue *)base;
		unsigned char segment = inst->segment;
		return (BrigSegment)segment;
	}
	case BRIG_KIND_INST_SEG:
	{
		struct BrigInstSeg *inst = 
				(struct BrigInstSeg *)base;
		unsigned char segment = inst->segment;
		return (BrigSegment)segment;
	}
	default:
		KindError("GetSegment");
	}
	return BRIG_SEGMENT_NONE;
}


bool BrigCodeEntry::isArray() const
{
	switch(getKind())
	{
	case BRIG_KIND_DIRECTIVE_VARIABLE:

	{
		BrigType type = getType();
		if (type & BRIG_TYPE_ARRAY)
			return true;
		else
			return false;
	}

	default:

		KindError("GetDim");

	}
	return false;

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
		KindError("GetInArgCount");
	}
	return 0;
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
		KindError("GetFirstInArg");
	}

	return std::unique_ptr<BrigCodeEntry>(nullptr);
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
		KindError("GetFirstCodeBlockEntry");
	}

	return std::unique_ptr<BrigCodeEntry>(nullptr);
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
	if (!isInstruction())
	{
		KindError("GetOpcode");
	}

	// Return opcode	
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	return (BrigOpcode)inst->opcode;
}


BrigType BrigCodeEntry::getType() const
{
	/// Check if the entry is an inst
	if (getKind() >= BRIG_KIND_INST_BEGIN 
			&& getKind() < BRIG_KIND_INST_END)
	{
		// Return type
		struct BrigInstBase *inst = (struct BrigInstBase *)base;
		return (BrigType)inst->type;
	}
	else if(getKind() == BRIG_KIND_DIRECTIVE_VARIABLE)
	{
		struct BrigDirectiveVariable *dir = 
				(struct BrigDirectiveVariable *)base;
		return (BrigType)dir->type;
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


BrigType BrigCodeEntry::getOperandType(unsigned char index) const
{
	// This function is only valid for instruction
	if (!isInstruction())
		throw misc::Panic("GetOperandType is only valid for"
				" instruction");

	// Create a table 
	unsigned short opcode = getOpcode();
	switch (index)
	{
	case 0:
		if (opcode == BRIG_OPCODE_ARRIVEFBAR) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_BARRIER) return BRIG_TYPE_NONE;
		if (opcode == BRIG_OPCODE_BR) return BRIG_TYPE_NONE;
		if (opcode == BRIG_OPCODE_CALL) return BRIG_TYPE_NONE;
		if (opcode == BRIG_OPCODE_ICALL) return BRIG_TYPE_NONE;
		if (opcode == BRIG_OPCODE_INITFBAR) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_JOINFBAR) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_RELEASEFBAR) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_WAITFBAR) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_MEMFENCE) return BRIG_TYPE_NONE;
		break;
	case 1:
		if (opcode == BRIG_OPCODE_ACTIVELANECOUNT) return getSourceType();
		if (opcode == BRIG_OPCODE_ACTIVELANEMASK) return getSourceType();
		if (opcode == BRIG_OPCODE_ACTIVELANECOUNT) return getSourceType();
		if (opcode == BRIG_OPCODE_BITMASK) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_CLASS) return getSourceType();
		if (opcode == BRIG_OPCODE_CMP) return getSourceType();
		if (opcode == BRIG_OPCODE_CVT) return getSourceType();
		if (opcode == BRIG_OPCODE_CMOV) return BRIG_TYPE_B1;
		if (opcode == BRIG_OPCODE_PACKCVT) return getSourceType();
		if (opcode == BRIG_OPCODE_UNPACKCVT) return getSourceType();
		if (opcode == BRIG_OPCODE_SAD) return getSourceType();
		if (opcode == BRIG_OPCODE_SADHI) return getSourceType();
		if (opcode == BRIG_OPCODE_POPCOUNT) return getSourceType();
		if (opcode == BRIG_OPCODE_COMBINE) return getSourceType();
		if (opcode == BRIG_OPCODE_EXPAND) return getSourceType();
		if (opcode == BRIG_OPCODE_FIRSTBIT) return getSourceType();
		if (opcode == BRIG_OPCODE_LASTBIT) return getSourceType();
		if (opcode == BRIG_OPCODE_SEGMENTP) return getSourceType();
		if (opcode == BRIG_OPCODE_STOF) return getSourceType();
		if (opcode == BRIG_OPCODE_FTOS) return getSourceType();
		if (opcode == BRIG_OPCODE_CURRENTWORKGROUPSIZE) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_GRIDGROUPS) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_GRIDSIZE) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_LDF) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_WORKGROUPID) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_WORKGROUPSIZE) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_WORKITEMABSID) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_WORKITEMID) return BRIG_TYPE_U32;
		break;
	case 2:
		if (opcode == BRIG_OPCODE_BITEXTRACT) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_BITMASK) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_CLASS) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_SHL) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_SHR) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_UNPACK) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_UNPACKCVT) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_CMP) return getSourceType();
		if (opcode == BRIG_OPCODE_PACK) return getSourceType();
		if (opcode == BRIG_OPCODE_PACKCVT) return getSourceType();
		if (opcode == BRIG_OPCODE_SAD) return getSourceType();
		if (opcode == BRIG_OPCODE_SADHI) return getSourceType();
		if (opcode == BRIG_OPCODE_LDIMAGE) return getCoordType();
		if (opcode == BRIG_OPCODE_STIMAGE) return getCoordType();
		if (opcode == BRIG_OPCODE_ACTIVELANEPERMUTE) return BRIG_TYPE_U32;
		break;
	case 3:
		if (opcode == BRIG_OPCODE_BITEXTRACT) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_BITINSERT) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_PACK) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_SAD) return BRIG_TYPE_U32;
		if (opcode == BRIG_OPCODE_SHUFFLE) return BRIG_TYPE_B32;
		if (opcode == BRIG_OPCODE_SIGNAL) 
		{
			if (getSignalOperation() == BRIG_ATOMIC_WAITTIMEOUT_EQ ||
					getSignalOperation() == BRIG_ATOMIC_WAITTIMEOUT_NE ||
					getSignalOperation() == BRIG_ATOMIC_WAITTIMEOUT_LT ||
					getSignalOperation() == BRIG_ATOMIC_WAITTIMEOUT_GTE)
				return BRIG_TYPE_U64;
		}
		if (opcode == BRIG_OPCODE_RDIMAGE) return getCoordType();
		break;
	case 4:
		if (opcode == BRIG_OPCODE_BITINSERT) return BRIG_TYPE_B32;
		if (opcode == BRIG_OPCODE_PACKCVT) return getSourceType();
		if (opcode == BRIG_OPCODE_ACTIVELANEPERMUTE) return BRIG_TYPE_B1;
		break;
	default:
		throw misc::Panic("Operand index out of range");
	}

	// In default cases, return inst type
	return getType();
}


BrigCompareOperation BrigCodeEntry::getCompareOperation() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_CMP:
	{
		struct BrigInstCmp *inst = (struct BrigInstCmp *)base;
		return (BrigCompareOperation)inst->compare;
	}	
	default: 
		KindError("GetCompareOperation");
	}
	return BRIG_COMPARE_EQ;
}


BrigAluModifier BrigCodeEntry::getAluModifier() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_CMP:
	{
		struct BrigInstCmp *inst = (struct BrigInstCmp *)base;
		return inst->modifier;
	}	
	case BRIG_KIND_INST_MOD:
	{
		struct BrigInstMod *inst = (struct BrigInstMod *)base;
		return inst->modifier;
	}
	case BRIG_KIND_INST_CVT:
	{
		struct BrigInstCvt *inst = (struct BrigInstCvt *)base;
		return inst->modifier;
	}
	default: 
		KindError("GetAluModifier");
	}	
	throw misc::Panic("Error");
}


BrigType BrigCodeEntry::getSourceType() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_CMP:
	{
		struct BrigInstCmp *inst = (struct BrigInstCmp *)base;
		return (BrigType)inst->sourceType;
	}	
	case BRIG_KIND_INST_SOURCE_TYPE:
	{
		struct BrigInstSourceType *inst = 
				(struct BrigInstSourceType *)base;
		return (BrigType)inst->sourceType;
	}
	case BRIG_KIND_INST_SEG_CVT:
	{
		struct BrigInstSegCvt *inst = 
				(struct BrigInstSegCvt *)base;
		return (BrigType)inst->sourceType;
	}
	case BRIG_KIND_INST_CVT:
	{
		struct BrigInstCvt *inst = (struct BrigInstCvt *)base;
		return (BrigType)inst->sourceType;
	}
	case BRIG_KIND_INST_LANE:
	{
		struct BrigInstLane *inst = (struct BrigInstLane *)base;
		return (BrigType)inst->sourceType;
	}
	default: 
		KindError("GetSourceType");
	}	
	return BRIG_TYPE_NONE;
}


BrigRound BrigCodeEntry::getRound() const
{
	switch(getKind()) 
	{
	case BRIG_KIND_INST_MOD:
	
	{
		BrigInstMod *inst = (BrigInstMod *)base;	
		return (BrigRound)inst->round;
		break;
	}

	case BRIG_KIND_INST_CVT:
	
	{
		BrigInstCvt *inst = (BrigInstCvt *)base;	
		return (BrigRound)inst->round;
		break;
	}

	default:

		KindError("GetRound");
		break;

	}
	return BRIG_ROUND_NONE;
}


BrigRound BrigCodeEntry::getDefaultRounding() const
{
	switch(getOpcode())
	{
	case BRIG_OPCODE_ABS:

		return BRIG_ROUND_NONE;

	case BRIG_OPCODE_ADD:
	case BRIG_OPCODE_SUB:
	case BRIG_OPCODE_MUL:
	case BRIG_OPCODE_DIV:

	{
		if (AsmService::isInteger(getType()))
		{
			return BRIG_ROUND_NONE;
		}
		if (AsmService::isFloat(getType()))
		{
			return BRIG_ROUND_FLOAT_DEFAULT;
		}
		return BRIG_ROUND_NONE;
	}

	case BRIG_OPCODE_CVT:

	{
		if ((getSourceType() == BRIG_TYPE_B1 ||
				AsmService::isInteger(getSourceType())) &&
				AsmService::isInteger(getType()))
		{
			return BRIG_ROUND_NONE;
		}
		else if (getSourceType() == BRIG_TYPE_B1 &&
				AsmService::isFloat(getType()))
		{
			return BRIG_ROUND_NONE;
		}
		else if (AsmService::isInteger(getSourceType()) && 
				AsmService::isFloat(getType()))
		{
			return BRIG_ROUND_FLOAT_DEFAULT;
		}
		else if (AsmService::isFloat(getSourceType()) &&
			getType() == BRIG_TYPE_B1)
		{
			return BRIG_ROUND_NONE;
		}
		else if (AsmService::isFloat(getSourceType()) &&
				AsmService::isInteger(getType()))
		{
			return BRIG_ROUND_INTEGER_ZERO;
		}
		else if(getSourceType() == BRIG_TYPE_F32 && 
				getType() == BRIG_TYPE_F16)
		{
			return BRIG_ROUND_FLOAT_DEFAULT;
		}
		else if (getSourceType() == BRIG_TYPE_F64 && 
				(getType() == BRIG_TYPE_F32 ||
				 getType() == BRIG_TYPE_F16))
		{
			return BRIG_ROUND_FLOAT_DEFAULT;
		}
		else if (AsmService::isFloat(getSourceType()) &&
				AsmService::isFloat(getType()))
		{
			return BRIG_ROUND_NONE;
		}
		break;
	}

	default:

		return BRIG_ROUND_NONE;

	}
	return BRIG_ROUND_NONE;
}


BrigPack BrigCodeEntry::getPack() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_MOD:
	{
		struct BrigInstMod *inst = (struct BrigInstMod *)base;
		return (BrigPack)(inst->pack);
	}
	case BRIG_KIND_INST_CMP:
	{
		struct BrigInstCmp *inst = (struct BrigInstCmp *)base;
		return (BrigPack)(inst->pack);
	}	
	default: 
		KindError("GetPack");
	}
	return BRIG_PACK_NONE;
}


unsigned BrigCodeEntry::getVectorModifier() const
{
	switch(getOpcode())
	{
	case BRIG_OPCODE_LD:
	case BRIG_OPCODE_GCNLD:
	case BRIG_OPCODE_ST:
	case BRIG_OPCODE_GCNST:
	case BRIG_OPCODE_EXPAND:
	case BRIG_OPCODE_RDIMAGE:
	case BRIG_OPCODE_LDIMAGE:
	case BRIG_OPCODE_STIMAGE:
	case BRIG_OPCODE_ACTIVELANEMASK:
	{
		auto operand = getOperand(0);
		if(operand->getKind() == BRIG_KIND_OPERAND_OPERAND_LIST)
			return operand->getElementCount();
		else
			return 0;
	}
	case BRIG_OPCODE_COMBINE:
	{
		auto operand = getOperand(1);
		if(operand->getKind() == BRIG_KIND_OPERAND_OPERAND_LIST)
			return operand->getElementCount();
		else
			return 0;
	}
	default:
		return 0;
	}
}


unsigned char BrigCodeEntry::getEquivClass() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_MEM:
	{
		struct BrigInstMem *inst = (struct BrigInstMem *)base;
		return inst->equivClass;
	}
	case BRIG_KIND_INST_ATOMIC:
	{
		struct BrigInstAtomic *inst = (struct BrigInstAtomic *)base;
		return inst->equivClass;
	}
	case BRIG_KIND_INST_IMAGE:
	{
		struct BrigInstImage *inst = (struct BrigInstImage *)base;
		return inst->equivClass;
	}
	default:
		KindError("GetEquivClass");
	}
	return 0;
}


bool BrigCodeEntry::isNoNull() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_SEG_CVT:
	{
		struct BrigInstSegCvt *inst = (struct BrigInstSegCvt *)base;
		unsigned char modifier = inst->modifier.allBits;
		return !!(modifier & BRIG_SEG_CVT_NONULL);
	}
	default:
		KindError("IsNoNull");
	}
	return false;
}


BrigAtomicOperation BrigCodeEntry::getAtomicOperation() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_ATOMIC:
	{
		struct BrigInstAtomic *inst = (struct BrigInstAtomic *)base;
		return (BrigAtomicOperation)inst->atomicOperation;
	}
	default:
		KindError("GetAtomicOperation");
	}	
	return BRIG_ATOMIC_ADD;
}


BrigMemoryOrder BrigCodeEntry::getMemoryOrder() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_ATOMIC:
	{
		struct BrigInstAtomic *inst = (struct BrigInstAtomic *)base;
		return (BrigMemoryOrder)inst->memoryOrder;
	}
	case BRIG_KIND_INST_SIGNAL:
	{
		struct BrigInstSignal *inst = (struct BrigInstSignal *)base;
		return (BrigMemoryOrder)inst->memoryOrder;
	}
	case BRIG_KIND_INST_MEM_FENCE:
	{
		struct BrigInstMemFence *inst = (struct BrigInstMemFence *)base;
		return (BrigMemoryOrder)inst->memoryOrder;
	}
	case BRIG_KIND_INST_QUEUE:
	{
		struct BrigInstQueue *inst = (struct BrigInstQueue *)base;
		return (BrigMemoryOrder)inst->memoryOrder;
	}
	default:
		KindError("GetMemoryOrder");
	}	
	return BRIG_MEMORY_ORDER_NONE;
}


BrigMemoryScope BrigCodeEntry::getMemoryScope() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_ATOMIC:
	{
		struct BrigInstAtomic *inst = (struct BrigInstAtomic *)base;
		return (BrigMemoryScope)inst->memoryScope;
	}
	default:
		KindError("GetMemoryScope");
	}	
	return BRIG_MEMORY_SCOPE_NONE;	
}


BrigAtomicOperation BrigCodeEntry::getSignalOperation() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_SIGNAL:
	{
		struct BrigInstSignal *inst = (struct BrigInstSignal *)base;
		return (BrigAtomicOperation)inst->signalOperation;
	}
	default:
		KindError("GetSignalOperation");
	}	
	return BRIG_ATOMIC_ADD;	
}


BrigType BrigCodeEntry::getSignalType() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_SIGNAL:
	{
		struct BrigInstSignal *inst = (struct BrigInstSignal *)base;
		return (BrigType)inst->signalType;
	}
	default:
		KindError("GetSignalType");
	}	
	return BRIG_TYPE_NONE;	
}


BrigImageGeometry BrigCodeEntry::getGeometry() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_IMAGE:
	{
		struct BrigInstImage *inst = (struct BrigInstImage *)base;
		return (BrigImageGeometry)inst->geometry;
	}
	case BRIG_KIND_INST_QUERY_IMAGE:
	{
		struct BrigInstQueryImage *inst = 
				(struct BrigInstQueryImage *)base;
		return (BrigImageGeometry)inst->geometry;
	}
	default:
		KindError("GetGeometry");
	}
	return BRIG_GEOMETRY_1D;
}


BrigType BrigCodeEntry::getImageType() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_IMAGE:
	{
		struct BrigInstImage *inst = (struct BrigInstImage *)base;
		return (BrigType)inst->imageType;
	}
	case BRIG_KIND_INST_QUERY_IMAGE:
	{
		struct BrigInstQueryImage *inst = 
				(struct BrigInstQueryImage *)base;
		return (BrigType)inst->imageType;
	}
	default:
		KindError("GetImageType");
	}	
	return BRIG_TYPE_NONE;		
}


BrigType BrigCodeEntry::getCoordType() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_IMAGE:

	{
		struct BrigInstImage *inst = (struct BrigInstImage *)base;
		return (BrigType)inst->coordType;
	}

	default:

		KindError("GetCoordType");

	}	
	return BRIG_TYPE_NONE;	
}


BrigMemoryScope BrigCodeEntry::getGlobalSegmentScope() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_MEM_FENCE:
	{
		struct BrigInstMemFence *inst = (struct BrigInstMemFence *)base;
		return (BrigMemoryScope)inst->globalSegmentMemoryScope;
	}
	default:
		KindError("GetGlobalSegmentScope");
	}	
	return BRIG_MEMORY_SCOPE_NONE;	
}


BrigMemoryScope BrigCodeEntry::getGroupSegmentScope() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_MEM_FENCE:
	{
		struct BrigInstMemFence *inst = (struct BrigInstMemFence *)base;
		return (BrigMemoryScope)inst->groupSegmentMemoryScope;
	}
	default:
		KindError("GetGlobalSegmentScope");
	}	
	return BRIG_MEMORY_SCOPE_NONE;
}


BrigMemoryScope BrigCodeEntry::getImageSegmentScope() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_MEM_FENCE:
	{
		struct BrigInstMemFence *inst = (struct BrigInstMemFence *)base;
		return (BrigMemoryScope)inst->imageSegmentMemoryScope;
	}
	default:
		KindError("GetGlobalSegmentScope");
	}	
	return BRIG_MEMORY_SCOPE_NONE;
}


BrigImageQuery BrigCodeEntry::getImageQuery() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_QUERY_IMAGE:
	{
		struct BrigInstQueryImage *inst = 
				(struct BrigInstQueryImage *)base;
		return (BrigImageQuery)inst->imageQuery;
	}
	default:
		KindError("GetImageQuery");
	}	
	return BRIG_IMAGE_QUERY_WIDTH;
}


BrigSamplerQuery BrigCodeEntry::getSamplerQuery() const
{
	switch(getKind())
	{
	case BRIG_KIND_INST_QUERY_SAMPLER:
	{
		struct BrigInstQuerySampler *inst = 
				(struct BrigInstQuerySampler *)base;
		return (BrigSamplerQuery)inst->samplerQuery;
	}
	default:
		KindError("GetSamplerQuery");
	}	
	return BRIG_SAMPLER_QUERY_ADDRESSING;	
}

}  // namespace HSA
