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

#ifndef ARCH_HSA_DISASSEMBLER_ASMSERVICE_H
#define ARCH_HSA_DISASSEMBLER_ASMSERVICE_H

#include <map>

#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>

#include "Brig.h"

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

	// Map rounding to string
	static misc::StringMap rounding_to_str_map;

	// Map pack to string
	static misc::StringMap pack_to_str_map;

	// Map kind to string
	static misc::StringMap kind_to_str_map;

	// Map atomic operation to string
	static misc::StringMap atomic_operation_to_str_map;

	// Map memory order to string
	static misc::StringMap memory_order_to_str_map;

	// Map memory scope to string
	static misc::StringMap memory_scope_to_str_map;

	// Map image geometry to string
	static misc::StringMap image_geometry_to_str_map;

	// Map image query to string
	static misc::StringMap image_query_to_str_map;

	// Map type to number of bytes of its size
	static std::map<int, unsigned> type_to_size_map;

public:

	/// Check if type is an integer
	static bool isInteger(BrigType type);

	/// Check if type is float
	static bool isFloat(BrigType type);

	/// Check if type is packed
	static bool isPacked(BrigType type);

	/// Check if type is array
	static bool isArray(BrigType type);




	//
	// String translation functions
	//

	/// Convert opcode to opcode string
	static std::string OpcodeToString(BrigOpcode opcode);

	/// Convert type to type string
	static std::string TypeToString(BrigType type);

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

	/// Conver register to string
	static std::string RegisterToString(BrigRegisterKind register_kind,
			unsigned short register_number);

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
	static std::string KindToString(BrigKind kind);

	/// Convert nonull to string
	static std::string NoNullToString(bool is_no_null);

	/// Convert equivalance class to string
	static std::string EquivClassToString(unsigned char equiv_class);

	/// Convert atomic operation to string
	static std::string AtomicOperationToString(BrigAtomicOperation 
			atomic_operation);

	/// Convert memory order to string
	static std::string MemoryOrderToString(BrigMemoryOrder memory_order);

	/// Convert memory scope to string
	static std::string MemoryScopeToString(BrigMemoryScope memory_scope);
	
	/// Convert image geometry to string
	static std::string ImageGeometryToString(BrigImageGeometry geometry);

	/// Convert image query to string
	static std::string ImageQueryToString(BrigImageQuery query);

	/// Convert sampler query to string
	static std::string SamplerQueryToString(BrigSamplerQuery query);

	/// Convert type to its size
	static unsigned TypeToSize(BrigType type);

	/// Get register size in bytes by its name
	static unsigned getSizeInByteByRegisterName(const std::string &name);




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


