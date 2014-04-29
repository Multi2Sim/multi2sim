#include "BrigOperandEntry.h"
#include "BrigStrEntry.h"
#include "BrigSection.h"

namespace HSA
{

BrigOperandEntry::BrigOperandEntry(char *buf, BrigFile *file)
	: BrigEntry(buf, file)
{
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

void BrigOperandEntry::dumpOperandImmed(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandWavesize(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandReg(std::ostream &os = std::cout) const
{
	struct BrigOperandReg *op = (struct BrigOperandReg *)base;
	os << BrigStrEntry::GetStringByOffset(file, op->reg);
}
void BrigOperandEntry::dumpOperandRegVector(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandAddress(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandLabelRef(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandArgumentRef(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandArgumentList(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandFunctionRef(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandFunctionList(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandSignatureRef(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}
void BrigOperandEntry::dumpOperandFbarrierRef(std::ostream &os = std::cout) const
{
	os << "<unsupported operand>";
}

char *BrigOperandEntry::GetOperandBufferByOffset(BrigFile *file, unsigned int offset)
{
	BrigSection *bs = file->getBrigSection(BrigSectionOperand);
	char *buf = (char *)bs->getBuffer();
	return buf + offset;
}

} // end namespace HSA
