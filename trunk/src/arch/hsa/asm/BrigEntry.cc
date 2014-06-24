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

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "BrigEntry.h"
#include "BrigStrEntry.h"
#include "BrigDirEntry.h"
#include "BrigInstEntry.h"
#include "Asm.h"
#include "BrigImmed.h"

namespace HSA
{

const char *BrigEntry::type2str(int type)
{
	return type_to_str_map.MapValue(type);
}

misc::StringMap BrigEntry::type_to_str_map = 
{
	{"", 		0},
	{"u8", 		1},
	{"u16", 	2},
	{"u32", 	3},
	{"u64", 	4},
	{"s8", 		5},
	{"s16", 	6},
	{"s32", 	7},
	{"s64", 	8},
	{"f16", 	9},
	{"f32", 	10},
	{"f64", 	11},
	{"b1", 		12},
	{"b8", 		13},
	{"b16",		14},
	{"b32", 	15},
	{"b64", 	16},
	{"b128", 	17},
	{"samp", 	18},
	{"roimg",	19},
	{"rwimg", 	20},
	{"fbar", 	21},
	{"u8x4", 	BRIG_TYPE_U8|BRIG_TYPE_PACK_32},
	{"u8x8", 	BRIG_TYPE_U8|BRIG_TYPE_PACK_64},
	{"u8x16", 	BRIG_TYPE_U8|BRIG_TYPE_PACK_128},
	{"u16x2", 	BRIG_TYPE_U16|BRIG_TYPE_PACK_32},
	{"u16x4", 	BRIG_TYPE_U16|BRIG_TYPE_PACK_64},
	{"u16x8", 	BRIG_TYPE_U16|BRIG_TYPE_PACK_128},
	{"u32x2", 	BRIG_TYPE_U32|BRIG_TYPE_PACK_64},
	{"u32x4", 	BRIG_TYPE_U32|BRIG_TYPE_PACK_128},
	{"u64x2", 	BRIG_TYPE_U64|BRIG_TYPE_PACK_128},
	{"s8x4", 	BRIG_TYPE_S8|BRIG_TYPE_PACK_32},
	{"s8x8", 	BRIG_TYPE_S8|BRIG_TYPE_PACK_64},
	{"s8x16", 	BRIG_TYPE_S8|BRIG_TYPE_PACK_128},
	{"s16x2", 	BRIG_TYPE_S16|BRIG_TYPE_PACK_32},
	{"s16x4", 	BRIG_TYPE_S16|BRIG_TYPE_PACK_64},
	{"s16x8", 	BRIG_TYPE_S16|BRIG_TYPE_PACK_128},
	{"s32x2", 	BRIG_TYPE_S32|BRIG_TYPE_PACK_64},
	{"s32x4", 	BRIG_TYPE_S32|BRIG_TYPE_PACK_64},
	{"s64x2", 	BRIG_TYPE_S64|BRIG_TYPE_PACK_128},
	{"f16x2", 	BRIG_TYPE_F16|BRIG_TYPE_PACK_32},
	{"f16x4", 	BRIG_TYPE_F16|BRIG_TYPE_PACK_64},
	{"f16x8", 	BRIG_TYPE_F16|BRIG_TYPE_PACK_128},
	{"f32x2", 	BRIG_TYPE_F32|BRIG_TYPE_PACK_64},
	{"f32x4", 	BRIG_TYPE_F32|BRIG_TYPE_PACK_128},
	{"f64x2", 	BRIG_TYPE_F64|BRIG_TYPE_PACK_128}
};

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
		ss << "align " << (unsigned)align << ' ';	
	}
	return ss.str().c_str();
}


const char *BrigEntry::seg2str(unsigned char seg)
{
	switch(seg)
	{
		case 0:	return "";	
		case 1: return "";
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

misc::StringMap BrigEntry::sem_to_str_map = 
{
	{"", 0},
	{"", 1},
	{"acq", 2},
	{"rel", 3},
	{"ar", 4},
	{"part_acq", 5},
	{"part_rel", 6},
	{"part_ar", 7}
};

const char *BrigEntry::sem2str(unsigned char modifier) const
{
	unsigned char sem = modifier & BRIG_MEMORY_SEMANTIC;
	return sem_to_str_map.MapValue(sem);
}

void BrigEntry::dumpValueList(
		BrigDataOffset32_t data,
		BrigType16_t type,
		uint32_t elementCount,
		BrigFile *file,
		std::ostream &os = std::cout
	)
{
	BrigSection *stringSection = file->getBrigSection(BrigSectionString);
	unsigned char *temp = (unsigned char *)stringSection->getBuffer();
	temp += 4;
	temp += data;
	for(unsigned int i=0; i<elementCount; i++)
	{
		if(i>0) os << ", ";
		BrigImmed immed(temp, type);
		immed.Dump(os);;
	}
}

void BrigEntry::dumpValue(char *value) const
{
}

void BrigEntry::dumpSymDecl(
		const BrigEntry *dir,
		std::ostream &os = std::cout
	)
{
	struct BrigDirectiveSymbol *sym = (struct BrigDirectiveSymbol *)dir->base;
	SymbolModifier modifier(sym->modifier.allBits);
	// extern, static
	os << modifier.getLinkageStr();
	// const
	os << modifier.getConstStr();
	// align
	os << BrigEntry::align2str(sym->align);
	// segment
	os << BrigEntry::seg2str(sym->segment);
	// type
	os << '_' << BrigEntry::type2str(sym->type);
	// name
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
		BrigFile * file,
		std::ostream &os = std::cout
	)
{
	os << "(";	
	char *next = arg;
	if (argCount == 1)
	{
		// Only one argument, dump it directly;
		BrigDirEntry dir(next, file);
		dumpSymDecl(&dir, os);
		next = dir.nextTop();
	}
	else if (argCount >1)
	{
		// More than 1 arguments, dump each argument in a new line
		Asm *as = Asm::getInstance();
		as->IndentMore();
		for (int i  = 0; i < argCount; i++)
		{
			if (i > 0) os << ",";
			os << "\n";
			dumpIndent(os);
			BrigDirEntry dir(next, file);
			dumpSymDecl(&dir, os);
			next = dir.nextTop();
		}
		as->IndentLess();
	}	
	os << ")";
	return next;
}

void BrigEntry::dumpBody(
		int codeOffset,
		int nInst,
		char *next,
		bool isDecl, 
		std::ostream &os = std::cout
	) const
{
	// If it is a declaration, only dump semicolumn
	// Otherwise, dump the content
	if(!isDecl)
	{
		Asm *as = Asm::getInstance();

		// Retrieve next (not top level) directive to dump in the future
		struct BrigDirectiveBase *dirPtr = 
				(struct BrigDirectiveBase *)next;

		// Write a new line and the open bracket
		os << "\n{\n";
		as->IndentMore();

		// Get the code section and its raw buffer
		BrigSection *code_section = 
				this->file->getBrigSection(BrigSectionCode);
		char *code_buffer = (char *)code_section->getBuffer();
		char *buffer_pointer = code_buffer + codeOffset;

		// Traverse all the insts belong to the function or kernal
		for(int i=0; i<nInst; i++)
		{	

			// Dump all directives should appear before current line
			// of instuction
			// FIXME:  the next five lines of code should be a 
			// 	stand-alone function
			while(dirPtr && dirPtr->code <= (BrigCodeOffset32_t)(buffer_pointer - code_buffer))
			{
				BrigDirEntry dir((char *)dirPtr, this->file);
				dir.Dump(os);
				dirPtr = (struct BrigDirectiveBase *)dir.nextTop();
			}
			
			// Create and dump the inst at buffer_pointer
			BrigInstEntry inst(buffer_pointer, this->file);
			dumpIndent(os);
			inst.Dump(os);
			buffer_pointer = inst.next();
		}

		// Decrease the indentation level and close the bracket
		as->IndentLess();
		os << "}";
	}	
	os << ";\n";
}

void BrigEntry::dumpIndent(std::ostream &os = std::cout)
{
	Asm *as = Asm::getInstance();
	for(int i = 0; i < as->getIndent(); i++)
		os << "\t";
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
