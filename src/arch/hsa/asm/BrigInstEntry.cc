#include "BrigInstEntry.h"
#include "BrigOperandEntry.h"
#include "Asm.h"

namespace HSA{


const char *BrigInstEntry::opcode2str(InstOpcode opcode) const
{
	Asm *as = Asm::getInstance();
	const struct InstInfo *inst_info = as->getInstInfo();
	return inst_info[opcode].fmt_str;
}

template <typename BrigInst>
const char *BrigInstEntry::rounding2str(BrigInst* base) const
{
	unsigned short rounding;
	//unsigned short defaultRounding;
	rounding = base->modifier.allBits;
	rounding &= 15;
	//FIXME compare rouding with deaultRounding	
	return BrigInstEntry::rounding_to_str_map.MapValue(rounding);
}

misc::StringMap BrigInstEntry::rounding_to_str_map = 
{
	{"", 0},
	{"_near", 1},
	{"_zero", 2},
	{"_up", 3},
	{"_down", 4},
	{"_neari", 5},
	{"_zeroi", 6},
	{"_upi", 7},
	{"_downi", 8},
	{"_neari_sat", 9},
	{"_zeroi_sat", 10},
	{"_upi_sat", 11},
	{"_downi_sat", 12}
};

const char *BrigInstEntry::modifier2str(unsigned short modifier) const
{
	unsigned short ftz = modifier & 16;
	if(ftz)
	{
		return "_ftz";
	}
	return "";
}

misc::StringMap BrigInstEntry::pack_to_str_map = 
{
	{"", 0},
	{"_pp", 1},
	{"_ps", 2},
	{"_sp", 3},
	{"_ss", 4},
	{"_s", 5},
	{"_p", 6},
	{"_pp_sat", 7},
	{"_ps_sat", 8},
	{"_sp_sat", 9},
	{"_ss_sat", 10},
	{"_s_sat", 11},
	{"_p_sat", 12}
};

const char *BrigInstEntry::pack2str(unsigned char pack) const
{
	return BrigInstEntry::pack_to_str_map.MapValue(pack);
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

void BrigInstEntry::dumpOperands(std::ostream &os = std::cout) const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	os << "\t";
	for(int i=0; i<5; i++)
	{
		if(inst->operands[i] == 0 ) return;
		if(i>0) os << ", ";
		BrigOperandEntry op(
			BrigOperandEntry::GetOperandBufferByOffset(file, inst->operands[i]),
			file
		);
		op.Dump(os);
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
	os << this->opcode2str((InstOpcode)inst->opcode);
	if(this->hasType()) os << "_" << BrigEntry::type2str(inst->type);
	this->dumpOperands(os);
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
	struct BrigInstMod *inst = (struct BrigInstMod *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	os << this->modifier2str(inst->modifier.allBits);
	os << this->rounding2str(inst);
	os << this->pack2str(inst->pack);
	if(this->hasType()) os << "_" << BrigEntry::type2str(inst->type);
	this->dumpOperands(os);
	os << "\n";
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
