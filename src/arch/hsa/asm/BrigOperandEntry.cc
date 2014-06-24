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

#include "BrigOperandEntry.h"
#include "BrigDirEntry.h"
#include "BrigStrEntry.h"
#include "BrigInstEntry.h"
#include "BrigSection.h"
#include "BrigImmed.h"

#include <iomanip>

namespace HSA
{

BrigOperandEntry::BrigOperandEntry(
		char *buf, 
		BrigFile *file, 
		const BrigInstEntry *inst,
		unsigned char index
	)
	: BrigEntry(buf, file)
{
	this->inst = inst;
	this->index = index;
}

unsigned short BrigOperandEntry::getKind() const
{
	struct BrigOperand *op = (struct BrigOperand *)base;
	return op->kind;
}

BrigOperandEntry::DumpOperandFn BrigOperandEntry::dump_operand_fn[] = 
{
	&BrigOperandEntry::dumpOperandImmed,
	&BrigOperandEntry::dumpOperandWavesize,
	&BrigOperandEntry::dumpOperandReg,
	&BrigOperandEntry::dumpOperandRegVector,
	&BrigOperandEntry::dumpOperandAddress,
	&BrigOperandEntry::dumpOperandLabelRef,
	&BrigOperandEntry::dumpOperandArgumentRef,
	&BrigOperandEntry::dumpOperandArgumentList,
	&BrigOperandEntry::dumpOperandFunctionRef,
	&BrigOperandEntry::dumpOperandFunctionList,
	&BrigOperandEntry::dumpOperandSignatureRef,
	&BrigOperandEntry::dumpOperandFbarrierRef
};

int BrigOperandEntry::getOperandType() const
{
	if(inst->getOpcode() == BRIG_OPCODE_SHL  && index ==2)
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_SHR  && index ==2)
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_BITEXTRACT 
		&& (index == 2 || index == 3))
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_BITMASK
		&& (index == 1 || index == 2))
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_BITINSERT
		&& (index == 3 || index == 4))
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_CMOV
		&& index == 1 )
	{
		struct BrigInstBasic* i = 
			(struct BrigInstBasic *)inst->getBuffer();
		if(i->type <= 31)
			return BRIG_TYPE_B1;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_CLASS
		&& index == 2 )
	{
		return BRIG_TYPE_U32;
	}
	else if( (inst->getOpcode() == BRIG_OPCODE_SAD
		|| inst->getOpcode() == BRIG_OPCODE_SADHI )
		&& index == 3 )
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_UNPACKCVT
		&& index == 2 )
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_MASKLANE
		&& index == 1 )
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getOpcode() == BRIG_OPCODE_ALLOCA
		&& index == 1 )
	{
		return BRIG_TYPE_U32;
	}
	else if(inst->getKind() == BRIG_INST_SOURCE_TYPE ||
		inst->getKind() == BRIG_INST_CMP ||
		inst->getKind() == BRIG_INST_CVT ||
		inst->getKind() == BRIG_INST_SEG)
	{
		struct BrigInstSourceType *i 
			= (struct BrigInstSourceType *)inst->getBuffer();
		return i->sourceType;
	}
	else
	{
		return inst->getType();
	}
	return inst->getType();
}

void BrigOperandEntry::dumpOperandImmed(std::ostream &os = std::cout) const
{
	int type = getOperandType();
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	BrigImmed immed( operand->bytes, type );
	immed.Dump(os);

}
void BrigOperandEntry::dumpOperandWavesize(std::ostream &os = std::cout) const
{
	os << "WAVESIZE";
}
void BrigOperandEntry::dumpOperandReg(std::ostream &os = std::cout) const
{
	struct BrigOperandReg *op = (struct BrigOperandReg *)base;
	os << BrigStrEntry::GetStringByOffset(file, op->reg);
}
void BrigOperandEntry::dumpOperandRegVector(std::ostream &os = std::cout) const
{
	struct BrigOperandRegVector *operand = (struct BrigOperandRegVector *)base;
	os << "(";
	for(int i=0; i<operand->regCount; i++)
	{
		if(i>0) os << ",";
		os << BrigStrEntry::GetStringByOffset(file, operand->regs[i]);
	}
	os << ")";
}
void BrigOperandEntry::dumpOperandAddress(std::ostream &os = std::cout) const
{
	struct BrigOperandAddress *operand = (struct BrigOperandAddress *)base;
	struct BrigDirectiveSymbol *dirBase 
		= (struct BrigDirectiveSymbol *)BrigDirEntry::GetDirByOffset(
				file, operand->symbol
			);
	std::string reg = BrigStrEntry::GetStringByOffset(file, operand->reg);
	long long offset = 
		(uint64_t(operand->offsetHi) << 32) | uint64_t(operand->offsetLo);
	if(operand->symbol)
	{
		os << "[" <<
			BrigStrEntry::GetStringByOffset(file, dirBase->name) 
			<< ']';
	}
	if(operand->reg)
	{
		os << "[" << reg;
		if(offset > 0) os << '+' << std::dec << offset;
		if(offset < 0) os << '-' << std::dec << -offset;
		os << "]";
	}
	else if (offset > 0 || !operand->symbol)
	{
		os << "[" << offset << "]";
	}
}
void BrigOperandEntry::dumpOperandLabelRef(std::ostream &os = std::cout) const
{
	struct BrigOperandRef *op = (struct BrigOperandRef *)base;
	BrigSection* bs = file->getBrigSection(BrigSectionDirective);
	char *buf = (char *)bs->getBuffer();
	buf += op->ref; 
	struct BrigDirectiveBase *dir = (struct BrigDirectiveBase *)buf;
	if(dir->kind == BRIG_DIRECTIVE_LABEL)
	{
		struct BrigDirectiveLabel *label = (struct BrigDirectiveLabel *)buf;
		os << BrigStrEntry::GetStringByOffset(file, label->name);
	}
	else if(dir->kind == BRIG_DIRECTIVE_LABEL_TARGETS)
	{
		struct BrigDirectiveLabelTargets *targets
			= (struct BrigDirectiveLabelTargets *)dir;
		buf = (char *)bs->getBuffer();
		buf += targets->label;
		struct BrigDirectiveLabel *label = (struct BrigDirectiveLabel *)buf;
		os << BrigStrEntry::GetStringByOffset(file, label->name);
	}
	else
	{
		misc::panic("OperandLabelRef can only ref to label of label target!");
	}
}
void BrigOperandEntry::dumpOperandArgumentRef(std::ostream &os = std::cout) const
{
	struct BrigOperandArgumentRef *operand = 
		(struct BrigOperandArgumentRef *)base;
	struct BrigDirectiveSymbol *dir = 
		(struct BrigDirectiveSymbol *)
		BrigDirEntry::GetDirByOffset(file, operand->ref);
	os << BrigStrEntry::GetStringByOffset(file, dir->name);
}
void BrigOperandEntry::dumpOperandArgumentList(std::ostream &os = std::cout) const
{
	struct BrigOperandArgumentList *operand = 
		(struct BrigOperandArgumentList *)base;
	unsigned count = operand->elementCount;
	if(count == 1 && !operand->elements[0] ) { count = 0; } // Empty list
	os << "(";
	for(unsigned i=0; i<count; i++)
	{
		if(i > 0){ os << ","; }
		struct BrigDirectiveSymbol *dir = 
			(struct BrigDirectiveSymbol *)
			BrigDirEntry::GetDirByOffset(file, operand->elements[i]);
		os << BrigStrEntry::GetStringByOffset(file, dir->name);	
	}
	os << ")";
}
void BrigOperandEntry::dumpOperandFunctionRef(std::ostream &os = std::cout) const
{
	struct BrigOperandFunctionRef *operand = 
		(struct BrigOperandFunctionRef *)base;
	struct BrigDirectiveFunction *dir = 
		(struct BrigDirectiveFunction *)BrigDirEntry::GetDirByOffset(
					file, operand->ref
				);
	os << BrigStrEntry::GetStringByOffset(file, dir->name);	
}
void BrigOperandEntry::dumpOperandFunctionList(std::ostream &os = std::cout) const
{
	os << "<unsupported operand function_list>";
}
void BrigOperandEntry::dumpOperandSignatureRef(std::ostream &os = std::cout) const
{
	os << "<unsupported operand signature_ref>";
}
void BrigOperandEntry::dumpOperandFbarrierRef(std::ostream &os = std::cout) const
{
	struct BrigOperandFbarrierRef *operand = 
		(struct BrigOperandFbarrierRef *)base;
	struct BrigDirectiveFbarrier *dir = 
		(struct BrigDirectiveFbarrier *)
		BrigDirEntry::GetDirByOffset(file, operand->ref);
	os << BrigStrEntry::GetStringByOffset(file, dir->name);
}

char *BrigOperandEntry::GetOperandBufferByOffset(
		BrigFile *file, 
		unsigned int offset
	)
{
	BrigSection *bs = file->getBrigSection(BrigSectionOperand);
	char *buf = (char *)bs->getBuffer();
	return buf + offset;
}

} // end namespace HSA
