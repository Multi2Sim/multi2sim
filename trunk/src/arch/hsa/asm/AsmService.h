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

#ifndef ARCH_HSA_ASM_ASMSERVICE_H
#define ARCH_HSA_ASM_ASMSERVICE_H

#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>

#include "BrigDef.h"

namespace HSA
{

/// AsmService provide static functions for assembly service
class AsmService
{
protected:
	
	// Map opcode to opcode string
	static misc::StringMap opcode_to_str_map;

	// Map type to type string
	static misc::StringMap type_to_str_map;

	// Map linkage to linkage string
	static misc::StringMap linkage_to_str_map;

	// Map segment to segment string
	static misc::StringMap segment_to_str_map;

	// Map alignment to alignment string
	static misc::StringMap align_to_str_map;

	// Map register kind to string
	static misc::StringMap register_kind_to_str_map;

	// Map width to string
	static misc::StringMap width_to_str_map;

	// Map compare operation to string
	static misc::StringMap compare_operation_to_str_map;

	/// Map rounding to string
	static misc::StringMap rounding_to_str_map;

	/// Map pack to string
	static misc::StringMap pack_to_str_map;

	/// Map kind to string
	static misc::StringMap kind_to_str_map;

public:




	//
	// String translation functions
	//

	/// Convert opcode to opcode string
	static std::string OpcodeToString(BrigOpcode opcode);

	/// Convert type to type string
	static std::string TypeToString(BrigTypeX type);

	/// Convert profile to profile string
	static std::string ProfileToString(BrigProfile profile);

	/// Convert machine model to machine model string
	static std::string MachineModelToString(BrigMachineModel machine);

	/// Convert declaration to string
	static std::string DeclToString(bool decl);

	/// Convert linkage to linkage string
	static std::string LinkageToString(unsigned char linkage);

	/// Convert allocation to allocation string
	static std::string AllocationToString(BrigAllocation allocation, 
			BrigSegment segment);

	/// Convert segment to segment string
	static std::string SegmentToString(unsigned char segment);

	/// Convert alignment to alignment string
	static std::string AlignToString(BrigAlignment align);

	/// Convert const to const string
	static std::string ConstToString(bool isConst);

	/// Convert register kind to string
	static std::string RegisterKindToString(BrigRegisterKind register_kind);

	/// Convert width to string
	static std::string WidthToString(BrigWidth width);

	/// Convert compare operation to string
	static std::string CompareOperationToString(BrigCompareOperation compare);

	/// Convert alu modifier to string
	static std::string AluModifierToString(BrigAluModifier modifier);

	/// Convert rounding to string
	static std::string RoundingToString(BrigRound rounding);

	/// Convert pack to string
	static std::string PackToString(BrigPack pack);

	/// Convert vector modifier to string
	static std::string VectorModifierToString(unsigned vectorModifier);

	/// Convert kind to string
	static std::string KindToString(BrigKinds kind);
	


	//
	// Dumping auxilliary functions
	//

	/// Dump a string with a leading underscore if the string is not empty
	static void DumpUnderscore(const std::string &string, 
			std::ostream &os);

	/// Dump a string with quotes and escaped charator
	static void DumpStringLiteral(const std::string &string, 
			std::ostream &os);

	/// Dump a string with an empty space after it, if the sting is not 
	/// empty
	static void DumpWithSpace(const std::string &string,
			std::ostream &os);
	
};

}

#endif


