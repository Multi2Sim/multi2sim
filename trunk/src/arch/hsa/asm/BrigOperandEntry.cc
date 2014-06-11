#include "BrigOperandEntry.h"
#include "BrigDirEntry.h"
#include "BrigStrEntry.h"
#include "BrigInstEntry.h"
#include "BrigSection.h"

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

std::map<int, BrigOperandEntry::DumpImmedFn> 
BrigOperandEntry::dump_immed_fn = 
{
	{BRIG_TYPE_NONE, &BrigOperandEntry::dumpImmedNONE},
	{BRIG_TYPE_U8, &BrigOperandEntry::dumpImmedU8},
	{BRIG_TYPE_U16, &BrigOperandEntry::dumpImmedU16},
	{BRIG_TYPE_U32, &BrigOperandEntry::dumpImmedU32},
	{BRIG_TYPE_U64, &BrigOperandEntry::dumpImmedU64},
	{BRIG_TYPE_S8, &BrigOperandEntry::dumpImmedS8},
	{BRIG_TYPE_S16, &BrigOperandEntry::dumpImmedS16},
	{BRIG_TYPE_S32, &BrigOperandEntry::dumpImmedS32},
	{BRIG_TYPE_S64, &BrigOperandEntry::dumpImmedS64},
	{BRIG_TYPE_F16, &BrigOperandEntry::dumpImmedF16},
	{BRIG_TYPE_F32, &BrigOperandEntry::dumpImmedF32},
	{BRIG_TYPE_F64, &BrigOperandEntry::dumpImmedF64},
	{BRIG_TYPE_B1, &BrigOperandEntry::dumpImmedB1},
	{BRIG_TYPE_B8, &BrigOperandEntry::dumpImmedB8},
	{BRIG_TYPE_B16, &BrigOperandEntry::dumpImmedB16},
	{BRIG_TYPE_B32, &BrigOperandEntry::dumpImmedB32},
	{BRIG_TYPE_B64, &BrigOperandEntry::dumpImmedB64},
	{BRIG_TYPE_B128, &BrigOperandEntry::dumpImmedB128},
	{BRIG_TYPE_SAMP, &BrigOperandEntry::dumpImmedSAMP},
	{BRIG_TYPE_ROIMG, &BrigOperandEntry::dumpImmedROIMG},
	{BRIG_TYPE_RWIMG, &BrigOperandEntry::dumpImmedRWIMG},
	{BRIG_TYPE_FBAR, &BrigOperandEntry::dumpImmedFBAR},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_32 , &BrigOperandEntry::dumpImmedU8X4},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedU8X8},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedU8X16},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_32 , &BrigOperandEntry::dumpImmedU16X2},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedU16X4},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedU16X8},
	{BRIG_TYPE_U32 | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedU32X2},
	{BRIG_TYPE_U32 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedU32X4},
	{BRIG_TYPE_U64 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedU64X2},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_32 , &BrigOperandEntry::dumpImmedS8X4},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedS8X8},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedS8X16},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_32 , &BrigOperandEntry::dumpImmedS16X2},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedS16X4},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedS16X8},
	{BRIG_TYPE_S32 | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedS32X2},
	{BRIG_TYPE_S32 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedS32X4},
	{BRIG_TYPE_S64 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedS64X2},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_32 , &BrigOperandEntry::dumpImmedF16X2},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedF16X4},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedF16X8},
	{BRIG_TYPE_F32 | BRIG_TYPE_PACK_64 , &BrigOperandEntry::dumpImmedF32X2},
	{BRIG_TYPE_F32 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedF32X4},
	{BRIG_TYPE_F64 | BRIG_TYPE_PACK_128, &BrigOperandEntry::dumpImmedF64X2}
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
	DumpImmedFn fn = 
		BrigOperandEntry::dump_immed_fn[type];
	(this->*fn)(nullptr, os);

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

void BrigOperandEntry::dumpImmedNONE(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	dumpImmedUnsupported(os);
}
void BrigOperandEntry::dumpImmedU8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned char *value = (unsigned char *)ptr;
	os << std::dec << (int)*value;
}
void BrigOperandEntry::dumpImmedU16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned short *value = (unsigned short *)ptr;
	os << std::dec << *value;
}
void BrigOperandEntry::dumpImmedU32(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int *value = (unsigned int *)ptr;
	os << std::dec << *value;
}
void BrigOperandEntry::dumpImmedU64(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	uint64_t *value = (uint64_t *)ptr;
	os << std::dec << *value;
}
void BrigOperandEntry::dumpImmedS8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = (unsigned char *)operand->bytes;
	char *value = (char *)ptr;
	os << std::dec << (int)*value;
}
void BrigOperandEntry::dumpImmedS16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = (unsigned char *)operand->bytes;
	short *value = (short *)ptr;
	os << std::dec << *value;
}
void BrigOperandEntry::dumpImmedS32(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = (unsigned char *)operand->bytes;
	int *value = (int *)ptr;
	os << std::dec << *value;
}
void BrigOperandEntry::dumpImmedS64(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = (unsigned char *)operand->bytes;
	long long *value = (long long *)ptr;
	os << std::dec << *value;
}
void BrigOperandEntry::dumpImmedF16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	os << "0H";
	for(int i=0; i<2; i++)
	{
		unsigned char *value = &ptr[1-i];
		std::cout << std::setfill('0') << std::setw(2) << 
			std::hex << (int)*value;
		std::cout << std::dec;
	}
}
void BrigOperandEntry::dumpImmedF32(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	os << "0F";
	for(int i=0; i<4; i++)
	{
		unsigned char *value = &ptr[3-i];
		std::cout << std::setfill('0') << std::setw(2) << 
			std::hex << (int)*value;
		std::cout << std::dec;
	}
}
void BrigOperandEntry::dumpImmedF64(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	os << "0D";
	for(int i=0; i<8; i++)
	{
		unsigned char *value = (unsigned char*) &ptr[7-i];
		std::cout << std::setfill('0') << std::setw(2) << 
			std::hex << (int)*value;
		std::cout << std::dec;
	}
}
void BrigOperandEntry::dumpImmedB1(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	if(*ptr)
	{
		os << '1';
	}
	else
	{
		os << '0';
	}
}
void BrigOperandEntry::dumpImmedB8(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedU8(nullptr, os);
}
void BrigOperandEntry::dumpImmedB16(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedU16(nullptr, os);
}
void BrigOperandEntry::dumpImmedB32(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedU32(nullptr, os);
}
void BrigOperandEntry::dumpImmedB64(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedU64(nullptr, os);
}
void BrigOperandEntry::dumpImmedB128(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedU64(nullptr, os);
}
void BrigOperandEntry::dumpImmedSAMP(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedUnsupported(os);
}
void BrigOperandEntry::dumpImmedROIMG(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedUnsupported(os);
}
void BrigOperandEntry::dumpImmedRWIMG(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedUnsupported(os);
}
void BrigOperandEntry::dumpImmedFBAR(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	dumpImmedUnsupported(os);
}
void BrigOperandEntry::dumpImmedU8X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 1;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU8X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 1;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU8X16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 1;
	unsigned int pack = 16;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU16X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU16X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU16X8(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{	
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU32X2(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU32(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU32X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU32(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedU64X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU64(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS8X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 1;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS8X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 1;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS8X16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 1;
	unsigned int pack = 16;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS16X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS16X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS16X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS32X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS32(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS32X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS32(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedS64X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS64(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedF16X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedF16X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedF16X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedF32X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF32(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedF32X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF32(value, os);
		value -= size;
	}
	os << ")";
}
void BrigOperandEntry::dumpImmedF64X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	if(!ptr) ptr = operand->bytes;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF64(value, os);
		value -= size;
	}
	os << ")";
}

void BrigOperandEntry::dumpImmedUnsupported(std::ostream &os = std::cout) const
{
	struct BrigOperandImmed *operand = (struct BrigOperandImmed *)base;
	os << "<immed type: " << type2str(operand->type) 
		<< ", instType: " << type2str(inst->getType())
		<< ", byteCount: " << operand->byteCount
		<< " >";
}

} // end namespace HSA
