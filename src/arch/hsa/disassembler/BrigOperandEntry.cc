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

#include "Brig.h"
#include "BrigImmed.h"
#include "BrigFile.h"
#include "BrigDataEntry.h"
#include "BrigCodeEntry.h"
#include "AsmService.h"
#include "BrigOperandEntry.h"

namespace HSA
{

std::map<unsigned, BrigOperandEntry::DumpEntryFn> BrigOperandEntry::dump_entry_fn = 
{
	{BRIG_KIND_OPERAND_ADDRESS, &BrigOperandEntry::DumpOperandAddress},
	{BRIG_KIND_OPERAND_ALIGN, &BrigOperandEntry::DumpOperandAlign},
	{BRIG_KIND_OPERAND_CODE_LIST, &BrigOperandEntry::DumpOperandCodeList},
	{BRIG_KIND_OPERAND_CODE_REF, &BrigOperandEntry::DumpOperandCodeRef},
	{BRIG_KIND_OPERAND_CONSTANT_BYTES, &BrigOperandEntry::DumpOperandConstantBytes},
	{BRIG_KIND_OPERAND_CONSTANT_IMAGE, &BrigOperandEntry::DumpOperandConstantImage},
	{BRIG_KIND_OPERAND_CONSTANT_OPERAND_LIST, &BrigOperandEntry::DumpOperandConstantOperandList},
	{BRIG_KIND_OPERAND_CONSTANT_SAMPLER, &BrigOperandEntry::DumpOperandConstantSampler},
	{BRIG_KIND_OPERAND_OPERAND_LIST, &BrigOperandEntry::DumpOperandOperandList},
	{BRIG_KIND_OPERAND_REGISTER,  &BrigOperandEntry::DumpOperandRegister},
	{BRIG_KIND_OPERAND_STRING, &BrigOperandEntry::DumpOperandString},
	{BRIG_KIND_OPERAND_WAVESIZE, &BrigOperandEntry::DumpOperandWavesize},
};


void BrigOperandEntry::KindError(const std::string &str) const
{
	throw misc::Panic(misc::fmt("%s not valid for kind %s", 
			str.c_str(), 
			AsmService::KindToString(getKind()).c_str()));
}


BrigOperandEntry::BrigOperandEntry(const char *buf) :
		BrigEntry(buf)
{
}


BrigKind BrigOperandEntry::getKind() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return (BrigKind)brig_base->kind;
}

	
void BrigOperandEntry::DumpOperandAddress(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	auto symbol = getSymbol();
	auto reg = getReg();
	unsigned long long offset = getOffset();

	if (symbol.get())
	{
		os << misc::fmt("[%s]", symbol->getName().c_str());	
	}

	if (reg.get())
	{
		os << "[";
		reg->Dump(BRIG_TYPE_NONE, os);
		if (offset > 0)
			os << "+" << offset;
		if (offset < 0)
			os << "-" << -offset;
		os << "]";
	}
	else if (offset != 0 || !symbol.get())
	{
		if (symbol.get() && offset < 0)
		{
			os << "[" << "-" << -offset << "]";
		}
		else
		{
			os << "[" << (unsigned long long)offset << "]";
		}
	}
}


void BrigOperandEntry::DumpOperandAlign(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "ALIGN");
}


void BrigOperandEntry::DumpOperandCodeList(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	os << "(";
	for (unsigned int i = 0; i < getElementCount(); i++)
	{
		if (i > 0) os << ", ";
		os << getElement(i)->getName();
	}
	os << ")";
}


void BrigOperandEntry::DumpOperandCodeRef(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	os << getRef()->getName();
}


void BrigOperandEntry::DumpOperandConstantBytes(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	auto immed = misc::new_unique<BrigImmed>(getBytes(), type);
	immed->Dump(os);
}


void BrigOperandEntry::DumpOperandConstantImage(BrigType type = BRIG_TYPE_NONE,
		std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "CONSTANT_IMAGE");
}


void BrigOperandEntry::DumpOperandConstantOperandList(BrigType type = BRIG_TYPE_NONE,
		std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "CONSTANT_OPERAND_LIST");
}


void BrigOperandEntry::DumpOperandConstantSampler(BrigType type = BRIG_TYPE_NONE,
		std::ostream &os = std::cout) const
{
	os << misc::fmt("Operand: %s, not supported\n", "CONSTANT_SAMPLER");
}


void BrigOperandEntry::DumpOperandOperandList(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	os << "(";
	DumpListOfOperand(getElements().get(), type, os);
	os << ")";
}


void BrigOperandEntry::DumpOperandRegister(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	os << AsmService::RegisterKindToString(getRegKind()) << getRegNumber();
}


void BrigOperandEntry::DumpOperandString(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	AsmService::DumpStringLiteral(getString(), os);
}


void BrigOperandEntry::DumpOperandWavesize(BrigType type = BRIG_TYPE_NONE, 
		std::ostream &os = std::cout) const
{
	os << "WAVESIZE";
}


void BrigOperandEntry::DumpListOfOperand(BrigDataEntry *operands, 
		BrigType type, std::ostream& os = std::cout) const
{
	unsigned size = operands->getByteCount() / 4;
	for (unsigned i = 0; i < size; i++)
	{
		if (i > 0) os << ", ";
		auto operand = getOperandElement(i);
		operand->Dump(type, os);
	}
}


const unsigned char *BrigOperandEntry::getBytes() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_CONSTANT_BYTES:
	{
		struct BrigOperandConstantBytes *operand = 
				(struct BrigOperandConstantBytes *)base;
		auto data = getBinary()->getDataEntryByOffset(operand->bytes);
		return data->getBytes();
	}
	default:
		KindError("GetData");
	}
	return nullptr;
}


std::unique_ptr<BrigCodeEntry> BrigOperandEntry::getSymbol() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_ADDRESS:
	{
		struct BrigOperandAddress *operand = 
				(struct BrigOperandAddress *)base;
		return getBinary()->getCodeEntryByOffset(operand->symbol);
	}
	default:
		KindError("GetSymbol");
	}
	return std::unique_ptr<BrigCodeEntry>(nullptr);
}


std::unique_ptr<BrigOperandEntry> BrigOperandEntry::getReg() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_ADDRESS:
	{
		struct BrigOperandAddress *operand = 
				(struct BrigOperandAddress *)base;
		return getBinary()->getOperandByOffset(operand->reg);
	}
	default:
		KindError("GetOperandEntry");
	}
	return std::unique_ptr<BrigOperandEntry>(nullptr);
}


unsigned long long BrigOperandEntry::getOffset() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_ADDRESS:
	{
		struct BrigOperandAddress *operand = 
				(struct BrigOperandAddress *)base;
		return *(unsigned long long *)(&operand->offset);
	}
	default:
		throw misc::Panic("GetReg is not vaild for type");
	}
}


BrigRegisterKind BrigOperandEntry::getRegKind() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_REGISTER:
	{
		struct BrigOperandRegister *operand = 
				(struct BrigOperandRegister *)base;
		return (BrigRegisterKind)operand->regKind;
	}
	default:
		KindError("GetRegKind");
	}
	return BRIG_REGISTER_KIND_CONTROL;
}


unsigned short BrigOperandEntry::getRegNumber() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_REGISTER:
	{
		struct BrigOperandRegister *operand = 
				(struct BrigOperandRegister *)base;
		return operand->regNum;
	}
	default:
		KindError("GerRegNumber");
	}
	return 0;
}


std::string BrigOperandEntry::getRegisterName() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_REGISTER:
	{
		return AsmService::RegisterToString(getRegKind(),
				getRegNumber());
	}
	default:
		KindError("GetRegisterName");
	}
	return "";
}


std::string BrigOperandEntry::getString() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_STRING:
	{
		struct BrigOperandString *operand = 
				(struct BrigOperandString *)base;
		return getBinary()->getStringByOffset(operand->string);
	}
	default:
		throw misc::Panic("GetString is not valid for type");
	}
}


std::unique_ptr<BrigCodeEntry> BrigOperandEntry::getRef() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_CODE_REF:
	{
		struct BrigOperandCodeRef *operand = 
				(struct BrigOperandCodeRef *)base;
		return getBinary()->getCodeEntryByOffset(operand->ref);
	}
	default:
		KindError("GetRef");
	}
	return std::unique_ptr<BrigCodeEntry>(nullptr);
}


std::unique_ptr<BrigDataEntry> BrigOperandEntry::getElements() const
{
	switch(getKind())
	{
	case BRIG_KIND_OPERAND_OPERAND_LIST:
	{
		struct BrigOperandOperandList *operand
			= (struct BrigOperandOperandList *)base;
		auto data_entry = getBinary()->getDataEntryByOffset(
				operand->elements);
		return data_entry;
	}
	case BRIG_KIND_OPERAND_CODE_LIST:
	{
		struct BrigOperandCodeList *operand
			= (struct BrigOperandCodeList *)base;
		auto data_entry = getBinary()->getDataEntryByOffset(
				operand->elements);
		return data_entry;
	}
	default:
		KindError("GetElements");
	}
	return std::unique_ptr<BrigDataEntry>(nullptr);
}


unsigned int BrigOperandEntry::getElementCount() const
{
	auto data_entry = getElements();
	return data_entry->getByteCount() / 4;
}


std::unique_ptr<BrigCodeEntry> BrigOperandEntry::getElement(unsigned int index) const
{
	if (index >= getElementCount())
		throw misc::Panic("GetElement out of range");
	auto data_entry = getElements();
	unsigned int offset = 
			*(unsigned int *)(data_entry->getBytes() + 
			index * 4);
	return getBinary()->getCodeEntryByOffset(offset);
}


std::unique_ptr<BrigOperandEntry> BrigOperandEntry::getOperandElement(unsigned int index) const
{
	if (index >= getElementCount())
		throw misc::Panic("GetElement out of range");
	auto data_entry = getElements();
	unsigned int offset = 
			*(unsigned int *)(data_entry->getBytes() + 
			index * 4);
	return getBinary()->getOperandByOffset(offset);
}

}  // namespace HSA
