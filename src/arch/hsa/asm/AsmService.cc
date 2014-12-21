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

#include "AsmService.h"

namespace HSA
{

misc::StringMap AsmService::opcode_to_str_map = 
{
#define DEFINST(name, opcode, opstr ) \
	{opstr, opcode},	
#include "Inst.def"
#undef DEFINST

	{"invalid", 140}
};


misc::StringMap AsmService::type_to_str_map = 
{
	{"", BRIG_TYPE_NONE},
	{"u8", BRIG_TYPE_U8},
	{"u16", BRIG_TYPE_U16},
	{"u32", BRIG_TYPE_U32},
	{"u64", BRIG_TYPE_U64},
	{"s8", BRIG_TYPE_S8},
	{"s16", BRIG_TYPE_S16},
	{"s32", BRIG_TYPE_S32},
	{"s64", BRIG_TYPE_S64},
	{"f16", BRIG_TYPE_F16},
	{"f32", BRIG_TYPE_F32},
	{"f64", BRIG_TYPE_F64},
	{"b1", BRIG_TYPE_B1},
	{"b8", BRIG_TYPE_B8},
	{"b16", BRIG_TYPE_B16},
	{"b32", BRIG_TYPE_B32},
	{"b64", BRIG_TYPE_B64},
	{"b128", BRIG_TYPE_B128},
	{"samp", BRIG_TYPE_SAMP},
	{"roimg", BRIG_TYPE_ROIMG},
	{"woimg", BRIG_TYPE_WOIMG},
	{"rwimg", BRIG_TYPE_RWIMG},
	{"sig32", BRIG_TYPE_SIG32},
	{"sig64", BRIG_TYPE_SIG64},
	{"u8x4", BRIG_TYPE_U8 | BRIG_TYPE_PACK_32},
	{"u8x8", BRIG_TYPE_U8 | BRIG_TYPE_PACK_64},
	{"u8x16", BRIG_TYPE_U8 | BRIG_TYPE_PACK_128},
	{"u16x2", BRIG_TYPE_U16 | BRIG_TYPE_PACK_32},
	{"u16x4", BRIG_TYPE_U16 | BRIG_TYPE_PACK_64},
	{"u16x8", BRIG_TYPE_U16 | BRIG_TYPE_PACK_128},
	{"u32x2", BRIG_TYPE_U32 | BRIG_TYPE_PACK_64},
	{"u32x4", BRIG_TYPE_U32 | BRIG_TYPE_PACK_128},
	{"u64x2", BRIG_TYPE_U64 | BRIG_TYPE_PACK_128},
	{"s8x4", BRIG_TYPE_S8 | BRIG_TYPE_PACK_32},
	{"s8x8", BRIG_TYPE_S8 | BRIG_TYPE_PACK_64},
	{"s8x16", BRIG_TYPE_S8 | BRIG_TYPE_PACK_128},
	{"s16x2", BRIG_TYPE_S16 | BRIG_TYPE_PACK_32},
	{"s16x4", BRIG_TYPE_S16 | BRIG_TYPE_PACK_64},
	{"s16x8", BRIG_TYPE_S16 | BRIG_TYPE_PACK_128},
	{"s32x2", BRIG_TYPE_S32 | BRIG_TYPE_PACK_64},
	{"s32x4", BRIG_TYPE_S32 | BRIG_TYPE_PACK_128},
	{"s64x2", BRIG_TYPE_S64 | BRIG_TYPE_PACK_128},
	{"f16x2", BRIG_TYPE_F16 | BRIG_TYPE_PACK_32},
	{"f16x4", BRIG_TYPE_F16 | BRIG_TYPE_PACK_64},
	{"f16x8", BRIG_TYPE_F16 | BRIG_TYPE_PACK_128},
	{"f32x2", BRIG_TYPE_F32 | BRIG_TYPE_PACK_64},
	{"f32x4", BRIG_TYPE_F32 | BRIG_TYPE_PACK_128},
	{"f64x2", BRIG_TYPE_F64 | BRIG_TYPE_PACK_128}
};


misc::StringMap AsmService::linkage_to_str_map = 
{
	{"", BRIG_LINKAGE_NONE},
	{"prog", BRIG_LINKAGE_PROGRAM},
	{"", BRIG_LINKAGE_MODULE},
	{"", BRIG_LINKAGE_FUNCTION},
	{"", BRIG_LINKAGE_ARG}
};


misc::StringMap AsmService::allocation_to_str_map = 
{
	{"", BRIG_ALLOCATION_NONE},
	{"", BRIG_ALLOCATION_PROGRAM},
	{"agent", BRIG_ALLOCATION_AGENT},
	{"", BRIG_ALLOCATION_AUTOMATIC}
};


misc::StringMap AsmService::segment_to_str_map = 
{
	{"", BRIG_SEGMENT_NONE},
	{"flat", BRIG_SEGMENT_FLAT},
	{"global", BRIG_SEGMENT_GLOBAL},
	{"readonly", BRIG_SEGMENT_READONLY},
	{"kernarg", BRIG_SEGMENT_KERNARG},
	{"group", BRIG_SEGMENT_GROUP},
	{"private", BRIG_SEGMENT_PRIVATE},
	{"spill", BRIG_SEGMENT_SPILL},
	{"arg", BRIG_SEGMENT_ARG},
	{"region", BRIG_SEGMENT_EXTSPACE0}
};


std::string AsmService::OpcodeToString(BrigOpcode opcode)
{
	return opcode_to_str_map.MapValue(opcode);
}


std::string AsmService::TypeToString(BrigTypeX type)
{
	return type_to_str_map.MapValue(type);
}


std::string AsmService::ProfileToString(BrigProfile profile)
{
	switch(profile)
	{
	case BRIG_PROFILE_BASE:
		return "$base";
	case BRIG_PROFILE_FULL:
		return "$full";
	default:
		throw misc::Panic("Unrecognized profile.");
	}

	return "";
}


std::string AsmService::MachineModelToString(BrigMachineModel machine)
{
	switch(machine)
	{
	case BRIG_MACHINE_LARGE:
		return "$large";
	case BRIG_MACHINE_SMALL:
		return "$small";
	default:
		throw misc::Panic("Unrecognized machine model.");
	}
	
	return "";
}


std::string AsmService::DeclToString(bool decl)
{
	if (decl)
		return "decl";
	return "";
}


std::string AsmService::LinkageToString(unsigned char linkage)
{
	return linkage_to_str_map.MapValue(linkage);	
}


std::string AsmService::AllocationToString(unsigned char allocation)
{
	return allocation_to_str_map.MapValue(allocation);
}


std::string AsmService::SegmentToString(unsigned char segment)
{
	return segment_to_str_map.MapValue(segment);
}


bool AsmService::InstUseType(BrigOpcode opcode)
{
	switch(opcode)
	{
	case BRIG_OPCODE_ARRIVEFBAR:
	case BRIG_OPCODE_BARRIER:
	case BRIG_OPCODE_BR:
	case BRIG_OPCODE_CALL:
	case BRIG_OPCODE_GCNMADS:
	case BRIG_OPCODE_GCNMADU:
	case BRIG_OPCODE_GCNMQSAD4:
	case BRIG_OPCODE_GCNREGIONALLOC:
	case BRIG_OPCODE_INITFBAR:
	case BRIG_OPCODE_JOINFBAR:
	case BRIG_OPCODE_LEAVEFBAR:
	case BRIG_OPCODE_NOP:
	case BRIG_OPCODE_RELEASEFBAR:
	case BRIG_OPCODE_RET:
	case BRIG_OPCODE_WAITFBAR:
	case BRIG_OPCODE_WAVEBARRIER:
		return false;
	default:
		return true;
	}
	return true;
}


void AsmService::DumpUnderscore(const std::string &string, 
		std::ostream &os = std::cout)
{
	if (string != "")
		os << "_" << string;
}


void AsmService::DumpStringLiteral(const std::string &string, 
		std::ostream &os = std::cout)
{
	os << '"';	

	// Traverse all charators
	const char *str_data = string.c_str();
	for (unsigned int i = 0; i < string.length(); i++)
	{
		unsigned char c = str_data[i];
		switch(c)
		{
		case '\a': os << "\\a"; break;
		case '\b': os << "\\b"; break;
		case '\f': os << "\\f"; break;
		case '\n': os << "\\n"; break;
		case '\r': os << "\\r"; break;
		case '\t': os << "\\t"; break;
		case '\"': os << "\\\""; break;
		case '\\': os << "\\\\"; break;
		default:
			if (c  >= 32 && c < 127)
			{
				os << c;
			}
			else
			{
				os << misc::fmt("\\x%02x", c);
			}
			break;
		}
	}

	os << '"';
}


void AsmService::DumpWithSpace(const std::string &string, 
		std::ostream &os = std::cout)
{
	if (string != "")
		os << string << " ";
}

}  // namespace HSA


