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
	os << "<unsupported operand immed>";
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
	os << "<unsupported operand reg_vector>";
}
void BrigOperandEntry::dumpOperandAddress(std::ostream &os = std::cout) const
{
	os << "<unsupported operand address>";
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
	os << "<unsupported operand argument_ref>";
}
void BrigOperandEntry::dumpOperandArgumentList(std::ostream &os = std::cout) const
{
	os << "<unsupported operand argument_list>";
}
void BrigOperandEntry::dumpOperandFunctionRef(std::ostream &os = std::cout) const
{
	os << "<unsupported operand function_ref>";
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
	os << "<unsupported operand fbarrier_ref>";
}

char *BrigOperandEntry::GetOperandBufferByOffset(BrigFile *file, unsigned int offset)
{
	BrigSection *bs = file->getBrigSection(BrigSectionOperand);
	char *buf = (char *)bs->getBuffer();
	return buf + offset;
}

} // end namespace HSA
