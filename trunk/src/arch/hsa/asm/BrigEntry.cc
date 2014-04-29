#include "BrigEntry.h"
#include "BrigStrEntry.h"
#include "BrigDirEntry.h"
#include "BrigInstEntry.h"
#include "lib/cpp/Misc.h"
#include "lib/cpp/String.h"

namespace HSA{
const char *BrigEntry::type2str(int type)
{
	switch(type)
	{
		case 0:	return "none";
		case 1: return "u8";
		case 2: return "u16";
		case 3: return "u32";
		case 4: return "u64";
		case 5: return "s8";
		case 6: return "s16";
		case 7: return "s32";
		case 8: return "s64";
		case 9: return "f16";
		case 10: return "f32";
		case 11: return "f64";
		case 12: return "b1";
		case 13: return "b8";
		case 14: return "b16";
		case 15: return "b32";
		case 16: return "b64";
		case 17: return "b128";
		case 18: return "samp";
		case 19: return "roimg";
		case 20: return "rwimg";
		case 21: return "fbar";

		case BRIG_TYPE_U8|BRIG_TYPE_PACK_32: return "u8x4";
		case BRIG_TYPE_U8|BRIG_TYPE_PACK_64: return "u8x8";
		case BRIG_TYPE_U8|BRIG_TYPE_PACK_128: return "u8x16";
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_32: return "u16x2";
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_64: return "u16x4";
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_128: return "u16x8";
		case BRIG_TYPE_U32|BRIG_TYPE_PACK_64: return "u32x2";
		case BRIG_TYPE_U32|BRIG_TYPE_PACK_128: return "u32x4";
		case BRIG_TYPE_U64|BRIG_TYPE_PACK_128: return "u64x2";
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_32: return "s8x4";
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_64: return "s8x8";
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_128: return "s8x16";
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_32: return "s16x2";
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_64: return "s16x4";
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_128: return "s16x8";
		case BRIG_TYPE_S32|BRIG_TYPE_PACK_64: return "s32x2";
		case BRIG_TYPE_S32|BRIG_TYPE_PACK_128: return "s32x4";
		case BRIG_TYPE_S64|BRIG_TYPE_PACK_128: return "s64x2";
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_32: return "f16x2";
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_64: return "f16x4";
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_128: return "f16x8";
		case BRIG_TYPE_F32|BRIG_TYPE_PACK_64: return "f32x2";
		case BRIG_TYPE_F32|BRIG_TYPE_PACK_128: return "f32x4";
		case BRIG_TYPE_F64|BRIG_TYPE_PACK_128: return "f64x2";
		default: 
			misc::warning("Unsupported type!");
			return "";
	}
	return "";
}

const char *BrigEntry::profile2str(int profile)
{
	if(profile == 1)
	{
		return "$full";
	}
	else if(profile == 0)
	{
		return "$base";
	}
	return "Invalid profile";
}

const char *BrigEntry::machineModel2str(int machineModel)
{
	if(machineModel == 1)
	{
		return "$large";
	}
	else if(machineModel == 0)
	{
		return "$small";
	}
	return "Invalid machine model";
}

const char *BrigEntry::align2str(unsigned char align)
{
	std::stringstream ss;
	if(align > 1)
	{
		ss << "align " << (unsigned int)align << ' ';	
	}
	return ss.str().c_str();
}


const char *BrigEntry::seg2str(unsigned char seg)
{
	switch(seg)
	{
		case 0:	return "";	
		case 1: return "flat";
		case 2: return "global";
		case 3: return "readonly";
		case 4: return "kernarg";
		case 5: return "group";
		case 6: return "private";
		case 7: return "spill";
		case 8: return "arg";
		default: misc::warning("Unsupported segment!");
	}
	return "";
}

void BrigEntry::dumpValueList(
		BrigDataOffset32_t data, 
		BrigType16_t type,
		uint32_t elementCount, 
		std::ostream &os = std::cout
	) const
{

}

void BrigEntry::dumpSymDecl(
		const BrigEntry *dir,
		std::ostream &os = std::cout
	)
{
	struct BrigDirectiveSymbol *sym = (struct BrigDirectiveSymbol *)dir->base;
	SymbolModifier modifier(sym->modifier.allBits);
	os << modifier.getLinkageStr();
	os << modifier.getConstStr();
	os << BrigEntry::align2str(sym->align);
	os << BrigEntry::seg2str(sym->segment);
	os << '_' << BrigEntry::type2str(sym->type);
	os << ' ' << BrigStrEntry::GetStringByOffset(dir->file, sym->name);
	uint64_t dim = (uint64_t(sym->dimHi) << 32) | uint64_t(sym->dimLo);
	if(
		modifier.isFlexArray() ||
		( dim == 0 && modifier.isArray() && modifier.isDeclaration() )
	){
		os << "[]";
	}
	else if(modifier.isArray())
	{
		os << "[" << dim << "]";
	}
}

char *BrigEntry::dumpArgs(
		char *arg, 
		unsigned short argCount, 
		std::ostream &os = std::cout
	)
{
	os << "(";	
	char *next = arg;
	if(argCount == 1)
	{
		misc::warning("Function with args are not supported!");
	}
	else if(argCount >1)
	{
		misc::warning("Function with args are not supported!");
	}	
	os << ")";
	return next;
}

void BrigEntry::dumpBody(
		int codeOffset,
		int nInst,
		bool isDecl, 
		std::ostream &os = std::cout
	) const
{
	if(!isDecl)
	{
		os << "\n{";
		os << "\n";
		BrigSection *bs = this->file->getBrigSection(BrigSectionCode);
		// +4 to skip section size field
		char * bufPtr = (char *)bs->getBuffer() + 4;
		for(int i=0; i<nInst; i++)
		{	
			//printf("Inst %d/%d", i, nInst);
			BrigInstEntry inst(bufPtr, this->file);
			inst.Dump(os);
			bufPtr = inst.next();
		}
		os << "\n}";
	}	
	os << ";\n";
}


BrigEntry::BrigEntry(char *buf, BrigFile* file)
{
	this->base = buf;
	this->file = file;
}

unsigned int BrigEntry::getSize() const
{
	uint16_t *size = (uint16_t *) base;
	// Considering the padding
	*size = ((*size) + 3 ) / 4 * 4;
	return *size;
}

void BrigEntry::dumpHex(std::ostream &os = std::cout) const
{
	unsigned char *buf = (unsigned char *)base;
	for(unsigned int i=0; i<this->getSize(); i++)
	{
		os << misc::fmt("%02x", buf[i]);
		if((i+1) % 4 == 0) os<<misc::fmt(" ");
	}
	os << "\n";
}

void BrigEntry::Dump(std::ostream &os = std::cout) const {};

	
}
