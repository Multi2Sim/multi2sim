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


std::unique_ptr<BrigEntry> BrigEntry::next() const
{
	unsigned int next_offset = getOffset() + getSize();
	return section->getEntryByOffset(next_offset);
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
	struct BrigDirectiveComment *dir = (struct BrigDirectiveComment *)base;
	DumpIndent(os);
	os << misc::fmt("%s\n", 
			getBinary()->getStringByOffset(dir->name).c_str());
}


void BrigEntry::DumpDirControl(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "CONTROL");
}


void BrigEntry::DumpDirExtension(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "EXTENSION");
}


void BrigEntry::DumpDirFbarrier(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "FBARRIER");
}


void BrigEntry::DumpDirFunction(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "FUNCTION");
}


void BrigEntry::DumpDirIndirectFunction(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "INDIRECT_FUNCTION");
}


void BrigEntry::DumpDirKernel(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "KERNEL");
}


void BrigEntry::DumpDirLabel(std::ostream &os = std::cout) const
{
	os << "\n";
	struct BrigDirectiveLabel *label = (struct BrigDirectiveLabel *)base;
	os << getBinary()->getStringByOffset(label->name);
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
	os << misc::fmt("Directive: %s, not supported\n", "SIGNATURE");
}


void BrigEntry::DumpDirVariable(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "VARIABLE");
}


void BrigEntry::DumpDirVersion(std::ostream &os = std::cout) const
{
	os << misc::fmt("Directive: %s, not supported\n", "VERSION");
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
	os << misc::fmt("Instruction: %s, not supported\n", "BASIC");
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

}  // namespace HSA


