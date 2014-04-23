#include "BrigInstEntry.h"
#include "Asm.h"

namespace HSA{


const char *BrigInstEntry::opcode2str(InstOpcode opcode) const
{
	Asm *as = Asm::getInstance();
	const struct InstInfo *inst_info = as->getInstInfo();
	return inst_info[opcode].fmt_str;
}

bool BrigInstEntry::hasType() const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	switch(inst->opcode){
	case BRIG_OPCODE_CBR:
	case BRIG_OPCODE_BRN:
	case BRIG_OPCODE_CALL:
	case BRIG_OPCODE_RET:
	case BRIG_OPCODE_BARRIER:
	case BRIG_OPCODE_INITFBAR:
	case BRIG_OPCODE_JOINFBAR:
	case BRIG_OPCODE_WAITFBAR:
	case BRIG_OPCODE_ARRIVEFBAR:
	case BRIG_OPCODE_LEAVEFBAR:
	case BRIG_OPCODE_RELEASEFBAR:
	case BRIG_OPCODE_SYNC:
	case BRIG_OPCODE_NOP:
		return false;

	case BRIG_OPCODE_GCNMADS:
	case BRIG_OPCODE_GCNMADU:
	case BRIG_OPCODE_GCNMQSAD4:
	case BRIG_OPCODE_GCNREGIONALLOC:
		return false;
	
	default:
		return true;
	}
}

BrigInstEntry::BrigInstEntry(char *buf, BrigFile *file)
	:BrigEntry(buf, file)
{
}

int BrigInstEntry::getKind() const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	return inst->kind;
}

int BrigInstEntry::getOpcode() const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	return inst->opcode;
}

BrigInstEntry::DumpInstFn BrigInstEntry::dump_inst_fn[] = 
{
	&BrigInstEntry::DumpInstNone,
	&BrigInstEntry::DumpInstBasic,
	&BrigInstEntry::DumpInstAtomic,
	&BrigInstEntry::DumpInstAtomicImage,
	&BrigInstEntry::DumpInstCvt,
	&BrigInstEntry::DumpInstBar,
	&BrigInstEntry::DumpInstBr,
	&BrigInstEntry::DumpInstCmp,
	&BrigInstEntry::DumpInstFbar,
	&BrigInstEntry::DumpInstImage,
	&BrigInstEntry::DumpInstMem,
	&BrigInstEntry::DumpInstAddr,
	&BrigInstEntry::DumpInstMod,
	&BrigInstEntry::DumpInstSeg,
	&BrigInstEntry::DumpInstSourceType
};

void BrigInstEntry::DumpInstNone(std::ostream &os) const
{
	this->DumpInstUnsupported("None", os);
}

void BrigInstEntry::DumpInstBasic(std::ostream &os) const
{
	struct BrigInstBasic *inst = (struct BrigInstBasic *)base;
	if(inst->opcode < InstOpcodeCount)
	{
		os << this->opcode2str((InstOpcode)inst->opcode);
	}
	else
	{
		os << "<unknown opcode>";
	}
	if(this->hasType()) os << "_" << BrigEntry::type2str(inst->type);
	this->DumpInstUnsupported("Basic", os);
	os << "\n";
}

void BrigInstEntry::DumpInstAtomic(std::ostream &os) const
{
	this->DumpInstUnsupported("Atomic", os);
}

void BrigInstEntry::DumpInstAtomicImage(std::ostream &os) const
{
	this->DumpInstUnsupported("Image", os);
}

void BrigInstEntry::DumpInstCvt(std::ostream &os) const
{
	this->DumpInstUnsupported("Cvt", os);
}

void BrigInstEntry::DumpInstBar(std::ostream &os) const
{
	this->DumpInstUnsupported("Bar", os);
}

void BrigInstEntry::DumpInstBr(std::ostream &os) const
{
	this->DumpInstUnsupported("Br", os);
}

void BrigInstEntry::DumpInstCmp(std::ostream &os) const
{
	this->DumpInstUnsupported("Cmp", os);
}

void BrigInstEntry::DumpInstFbar(std::ostream &os) const
{
	this->DumpInstUnsupported("Fbar", os);
}

void BrigInstEntry::DumpInstImage(std::ostream &os) const
{
	this->DumpInstUnsupported("Image", os);
}

void BrigInstEntry::DumpInstMem(std::ostream &os) const
{
	this->DumpInstUnsupported("Mem", os);
}

void BrigInstEntry::DumpInstAddr(std::ostream &os) const
{
	this->DumpInstUnsupported("Addr", os);
}

void BrigInstEntry::DumpInstMod(std::ostream &os) const
{
	this->DumpInstUnsupported("Mod", os);
}

void BrigInstEntry::DumpInstSeg(std::ostream &os) const
{
	this->DumpInstUnsupported("Seg", os);
}

void BrigInstEntry::DumpInstSourceType(std::ostream &os) const
{
	this->DumpInstUnsupported("SourceType", os);
}

void BrigInstEntry::DumpInstUnsupported( 
		const char *kind, 
		std::ostream &os = std::cout
	) const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	os << "<unsupported kind:" << kind 
		<< "(" << inst->kind << ") " 
		<< "opcode:" << inst->opcode << " >\n";
}
}//end namespace
