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

#include <sstream>

#include "BrigInstEntry.h"
#include "BrigOperandEntry.h"
#include "Asm.h"

namespace HSA
{

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
	{"", 1},
	{"zero", 2},
	{"up", 3},
	{"down", 4},
	{"neari", 5},
	{"", 6},
	{"upi", 7},
	{"downi", 8},
	{"neari_sat", 9},
	{"zeroi_sat", 10},
	{"upi_sat", 11},
	{"downi_sat", 12}
};


const char *BrigInstEntry::modifier2str(unsigned short modifier) const
{
	unsigned short ftz = modifier & 16;
	if(ftz)
	{
		return "ftz";
	}
	return "";
}


misc::StringMap BrigInstEntry::pack_to_str_map = 
{
	{"", 0},
	{"pp", 1},
	{"ps", 2},
	{"sp", 3},
	{"ss", 4},
	{"s", 5},
	{"p", 6},
	{"pp_sat", 7},
	{"ps_sat", 8},
	{"sp_sat", 9},
	{"ss_sat", 10},
	{"s_sat", 11},
	{"p_sat", 12}
};


const char *BrigInstEntry::pack2str(unsigned char pack) const
{
	return BrigInstEntry::pack_to_str_map.MapValue(pack);
}


misc::StringMap BrigInstEntry::v_to_str_map = 
{
	{"v2", 2},
	{"v3", 3},
	{"v4", 4}
};


const char *BrigInstEntry::operandV2str(char *o) const
{
	struct BrigOperandBase *operand = (struct BrigOperandBase *)o;
	switch(operand->kind)
	{
	case BRIG_OPERAND_REG_VECTOR:
	{
		struct BrigOperandRegVector *v = (struct BrigOperandRegVector *)o;
		return BrigInstEntry::v_to_str_map.MapValue(v->regCount);		
	}
	case BRIG_OPERAND_REG:
		return "";
		break;
	case BRIG_OPERAND_IMMED:
		return "";
		break;
	default:
		return "/*INVALID*/";
	}
	
}


const char *BrigInstEntry::v2str(char* instPtr) const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)instPtr;
	switch(inst->opcode)
	{
	case BRIG_OPCODE_LD:
	case BRIG_OPCODE_ST:
	case BRIG_OPCODE_EXPAND:
	case BRIG_OPCODE_RDIMAGE:
	case BRIG_OPCODE_LDIMAGE:
	case BRIG_OPCODE_STIMAGE:
		return operandV2str(getOperand(0));
		break;
	case BRIG_OPCODE_COMBINE:
		return operandV2str(getOperand(1));
		break;
	default:
		break;
	}
	return "";
}


misc::StringMap BrigInstEntry::width_to_str_map = 
{
	{"", 0},
	{"width(1)", 1},
	{"width(2)", 2},
	{"width(4)", 3},
	{"width(8)", 4},
	{"width(16)", 5},
	{"width(32)", 6},
	{"width(64)", 7},
	{"width(128)", 8},
	{"width(256)", 9},
	{"width(512)", 10},
	{"width(1024)", 11},
	{"width(2048)", 12},
	{"width(4096)", 13},
	{"width(8192)", 14},
	{"width(16384)", 15},
	{"width(32768)", 16},
	{"width(65536)", 17},
	{"width(131072)", 18},
	{"width(262144)", 19},
	{"width(524288)", 20},
	{"width(1048576)", 21},
	{"width(2097152)", 22},
	{"width(4194304)", 23},
	{"width(8388608)", 24},
	{"width(16777216)", 25},
	{"width(33554432)", 26},
	{"width(67108864)", 27},
	{"width(134217728)", 28},
	{"width(268435456)", 29},
	{"width(536870912)", 30},
	{"width(1073741824)", 31},
	{"width(2147483648)", 32},
	{"width(WAVESIZE)", 33},
	{"width(all)", 34}
};


template<class T>
int BrigInstEntry::getDefaultWidth(T *inst) const
{
	if(inst->opcode == BRIG_OPCODE_BRN)
	{
		struct BrigOperandBase * operand = 
			(struct BrigOperandBase *)getOperand(0);
		if(operand->kind == BRIG_OPERAND_LABEL_REF)
		{
			return 34;
		}
		return 1;
	}
	else if(
		inst->opcode == BRIG_OPCODE_BARRIER ||
		inst->opcode == BRIG_OPCODE_CALL
	)
	{
		return 34;
	}
	else if(inst->opcode == BRIG_OPCODE_JOINFBAR ||
		inst->opcode == BRIG_OPCODE_LEAVEFBAR ||
		inst->opcode == BRIG_OPCODE_WAITFBAR ||
		inst->opcode == BRIG_OPCODE_ARRIVEFBAR)
	{
		return 33;
	}
	return 1;
}


template<class T>
const char *BrigInstEntry::width2str(T *inst) const
{
	if(inst->width == getDefaultWidth(inst))
	{
		return "";
	}
	return width_to_str_map.MapValue(inst->width);	
}


misc::StringMap BrigInstEntry::comp_op_to_str_map = 
{
	{"eq", 0},
	{"ne", 1},
	{"lt", 2},
	{"le", 3},
	{"gt", 4},
	{"ge", 5},
	{"equ", 6},
	{"neu", 7},
	{"ltu", 8},
	{"leu", 9},
	{"gtu", 10},
	{"geu", 11},
	{"num", 12},
	{"nan", 13},
	{"seq", 14},
	{"sne", 15},
	{"slt", 16},
	{"sle", 17},
	{"sgt", 18},
	{"sge", 19},
	{"sgeu", 20},
	{"sequ", 21},
	{"sneu", 22},
	{"sltu", 23},
	{"sleu", 24},
	{"snum", 25},
	{"snan", 26},
	{"sgtu", 27}
};


const char *BrigInstEntry::cmpOp2str(unsigned char op) const
{
	return comp_op_to_str_map.MapValue(op);
}


const char *BrigInstEntry::equiv2str(unsigned char val) const
{
	std::stringstream s;
	if(val != 0) s << "equiv(" << (unsigned) val << ")";
	return s.str().c_str();
}


const char *BrigInstEntry::aligned2str(unsigned char modifier) const
{
	unsigned char aligned = modifier & BRIG_MEMORY_ALIGNED;
	if(aligned) return "aligned";
	return "";
}


misc::StringMap BrigInstEntry::atomic_op_to_str_map = 
{
	{"and", 0},
	{"or", 1},
	{"xor", 2},
	{"cas", 3},
	{"exch", 4},
	{"add", 5},
	{"inc", 6},
	{"dec", 7},
	{"min", 8},
	{"max", 9},
	{"sub", 10},
};


const char *BrigInstEntry::atomicOp2str(unsigned atomicOperation) const
{
	return atomic_op_to_str_map.MapValue(atomicOperation);
}


misc::StringMap BrigInstEntry::image_geo_to_str_map =
{
	{"1d", 0},
	{"2d", 1},
	{"3d", 2},
	{"1da", 3},
	{"1db", 4},
	{"2da", 5}
};


const char *BrigInstEntry::imageGeo2str(unsigned geometry) const
{
	return image_geo_to_str_map.MapValue(geometry);
}


misc::StringMap BrigInstEntry::mem_fence_to_str_map = 
{
	{"fnone", 0},
	{"fgroup", 1},
	{"fglobal", 2},
	{"fboth", 3},
	{"fpartial", 4},
	{"fpartialboth", 5}
};


template<typename T>
int BrigInstEntry::getDefaultMemFence(T *inst) const
{
	if(inst->opcode == BRIG_OPCODE_BARRIER ||
		inst->opcode == BRIG_OPCODE_WAITFBAR ||
		inst->opcode == BRIG_OPCODE_ARRIVEFBAR ||
		inst->opcode == BRIG_OPCODE_SYNC){
		return 3;
	}
	return 0;
}


template<typename T>
const char *BrigInstEntry::memFence2str(T *inst) const
{
	if(inst->memoryFence == getDefaultMemFence(inst))
	{
		return "";
	}
	return mem_fence_to_str_map.MapValue(inst->memoryFence);
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


char *BrigInstEntry::getOperand(int i) const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	return BrigOperandEntry::GetOperandBufferByOffset(file,
			inst->operands[i]);
}


void BrigInstEntry::dumpCallOperands(std::ostream &os = std::cout) const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;

	// With call insts, dump operand 1 first, dump operand 0 then
	os << "\t";
	BrigOperandEntry op1(
		getOperand(1), file,
		this, 1
	);
	op1.Dump(os);
	os << " ";
	BrigOperandEntry op0(
		getOperand(0), file,
		this, 0
	);
	op0.Dump(os);

	// Dump other functions
	for(int i=2; i<5; i++)
	{
		if(inst->operands[i] == 0) return;
		if(i>0) os << " ";
		BrigOperandEntry op(
			getOperand(i), file,
			this, i
		);
		op.Dump(os);
	}	
}


void BrigInstEntry::dumpOperands(std::ostream &os = std::cout) const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	if(inst->operands[0] == 0 ) return;
	os << "\t";
	for(int i=0; i<5; i++)
	{
		if(inst->operands[i] == 0) return;
		if(i>0) os << ", ";
		BrigOperandEntry operand(getOperand(i), file, this, i);
		operand.Dump(os);
	}
}


BrigInstEntry::BrigInstEntry(char *buf, BrigFile *file)
	:BrigEntry(buf, file)
{
	//this->DebugInst();	
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


unsigned short BrigInstEntry::getType() const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	return inst->type;
}


BrigInstEntry::DumpInstFn BrigInstEntry::dump_inst_fn[] = 
{
	&BrigInstEntry::DumpInstNone,
	&BrigInstEntry::DumpInstBasic,
	&BrigInstEntry::DumpInstAtomic,
	&BrigInstEntry::DumpInstAtomicImage,
	&BrigInstEntry::DumpInstBar,
	&BrigInstEntry::DumpInstBr,
	&BrigInstEntry::DumpInstCmp,
	&BrigInstEntry::DumpInstCvt,
	&BrigInstEntry::DumpInstFbar,
	&BrigInstEntry::DumpInstImage,
	&BrigInstEntry::DumpInstMem,
	&BrigInstEntry::DumpInstAddr,
	&BrigInstEntry::DumpInstMod,
	&BrigInstEntry::DumpInstSeg,
	&BrigInstEntry::DumpInstSourceType
};


void BrigInstEntry::DumpInstNone(std::ostream &os = std::cout) const
{
	this->DumpInstUnsupported("None", os);
}


void BrigInstEntry::DumpInstBasic(std::ostream &os = std::cout) const
{
	struct BrigInstBasic *inst = (struct BrigInstBasic *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	if(this->hasType()) os << "_" << BrigEntry::type2str(inst->type);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstAtomic(std::ostream &os = std::cout) const
{
	struct BrigInstAtomic *inst = (struct BrigInstAtomic *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(atomicOp2str(inst->atomicOperation), os);
	//FIXME: diff from official disassembler
	dumpUnderscore(BrigEntry::seg2str(inst->segment), os); 
	dumpUnderscore(sem2str(inst->memorySemantic), os);
	dumpUnderscore(type2str(inst->type), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstAtomicImage(std::ostream &os = std::cout) const
{
	struct BrigInstAtomicImage *inst = (struct BrigInstAtomicImage *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(atomicOp2str(inst->atomicOperation), os);
	dumpUnderscore(imageGeo2str(inst->geometry), os);
	dumpUnderscore(type2str(inst->type), os);
	dumpUnderscore(type2str(inst->imageType), os);
	dumpUnderscore(type2str(inst->coordType), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstCvt(std::ostream &os = std::cout) const
{
	struct BrigInstCvt *inst = (struct BrigInstCvt *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(modifier2str(inst->modifier.allBits), os);
	dumpUnderscore(rounding2str(inst), os);
	dumpUnderscore(type2str(inst->type), os);
	dumpUnderscore(type2str(inst->sourceType), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstBar(std::ostream &os = std::cout) const
{
	struct BrigInstBar *inst = (struct BrigInstBar *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(width2str(inst), os);
	dumpUnderscore(memFence2str(inst), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstBr(std::ostream &os = std::cout) const
{
	struct BrigInstBr *inst = (struct BrigInstBr *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(width2str(inst), os);
	dumpUnderscore(modifier2str(inst->modifier.allBits), os);
	dumpUnderscore(rounding2str(inst), os);
	if(this->hasType()) dumpUnderscore(type2str(inst->type));
	if(inst->opcode == BRIG_OPCODE_CALL)
	{
		this->dumpCallOperands(os);
	}
	else
	{
		this->dumpOperands(os);
	}
	os << ";\n";
}


void BrigInstEntry::DumpInstCmp(std::ostream &os = std::cout) const
{
	struct BrigInstCmp *inst = (struct BrigInstCmp *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(cmpOp2str(inst->compare), os);
	dumpUnderscore(modifier2str(inst->modifier.allBits), os);
	dumpUnderscore(rounding2str(inst), os);
	dumpUnderscore(pack2str(inst->pack), os);
	dumpUnderscore(type2str(inst->type), os);
	dumpUnderscore(type2str(inst->sourceType), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstFbar(std::ostream &os = std::cout) const
{
	struct BrigInstFbar *inst = (struct BrigInstFbar *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(width2str(inst), os);
	if(inst->opcode==BRIG_OPCODE_WAITFBAR || 
		inst->opcode==BRIG_OPCODE_ARRIVEFBAR)
	{
		dumpUnderscore(memFence2str(inst), os);
	}
	if(this->hasType()) dumpUnderscore(type2str(inst->type), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstImage(std::ostream &os = std::cout) const
{
	struct BrigInstImage *inst = (struct BrigInstImage *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(v2str(base), os);
	dumpUnderscore(imageGeo2str(inst->geometry), os);
	dumpUnderscore(type2str(inst->type), os);
	dumpUnderscore(type2str(inst->imageType), os);
	dumpUnderscore(type2str(inst->coordType), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstMem(std::ostream &os = std::cout) const
{
	struct BrigInstMem *inst = (struct BrigInstMem *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(v2str(base), os);
	dumpUnderscore(width2str(inst), os);
	dumpUnderscore(BrigEntry::seg2str(inst->segment), os);
	dumpUnderscore(aligned2str(inst->modifier.allBits), os);
	dumpUnderscore(BrigEntry::sem2str(inst->modifier.allBits), os);
	dumpUnderscore(equiv2str(inst->equivClass), os);
	dumpUnderscore(type2str(inst->type), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstAddr(std::ostream &os = std::cout) const
{
	struct BrigInstAddr *inst = (struct BrigInstAddr *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(BrigEntry::seg2str(inst->segment));
	dumpUnderscore(BrigEntry::type2str(inst->type));
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstMod(std::ostream &os = std::cout) const
{
	struct BrigInstMod *inst = (struct BrigInstMod *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(modifier2str(inst->modifier.allBits), os);
	dumpUnderscore(rounding2str(inst), os);
	dumpUnderscore(pack2str(inst->pack), os);
	if(this->hasType()) os << "_" << BrigEntry::type2str(inst->type);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstSeg(std::ostream &os = std::cout) const
{
	struct BrigInstSeg *inst = (struct BrigInstSeg *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	dumpUnderscore(BrigEntry::seg2str(inst->segment), os);
	dumpUnderscore(BrigEntry::type2str(inst->type), os);
	dumpUnderscore(BrigEntry::type2str(inst->sourceType), os);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstSourceType(std::ostream &os = std::cout) const
{
	struct BrigInstSourceType *inst = (struct BrigInstSourceType *)base;
	os << this->opcode2str((InstOpcode)inst->opcode);
	
	dumpUnderscore(v2str(base), os);
	os << "_" << BrigEntry::type2str(inst->type);
	os << "_" << BrigEntry::type2str(inst->sourceType);
	this->dumpOperands(os);
	os << ";\n";
}


void BrigInstEntry::DumpInstUnsupported(const char *kind,
		std::ostream &os = std::cout) const
{
	struct BrigInstBase *inst = (struct BrigInstBase *)base;
	os << "<unsupported kind:" << kind 
		<< "(" << inst->kind << ") " 
		<< "opcode:" << inst->opcode << " >;\n";
}


void BrigInstEntry::DebugInst(){
	std::cout << "Inst kind: " << this->getKind() << "\n";
	std::cout << misc::fmt("Inst opcode: %d\n", this->getOpcode()); 	
}

}//end namespace
