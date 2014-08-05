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

#ifndef HSA_ASM_BRIGINSTENTRY_H
#define HSA_ASM_BRIGINSTENTRY_H

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "BrigEntry.h"


namespace HSA
{

/// InstOpcode: I do not use the enumeration from the BrigDef.h for the
///  following reasons:
///		1. I need the InstOpcodeCount for the total number of opcodes.
enum InstOpcode
{
#define DEFINST(_name, _opstr) \
	Inst_##_name,
#include "Inst.def"
#undef DEFINST
	InstOpcodeCount
};

struct InstInfo
{
	InstOpcode opcode;
	const char *name;
	const char *fmt_str;
};

/// Represents a instruction entry in the .code section
class BrigInstEntry : public BrigEntry
{

protected:

	// convert opcode to string
	const char *opcode2str(InstOpcode opcode) const;

	// Convert rouding to string considering the default rouding should be 
	// ignored.
	template <typename BrigInst>
	const char *rounding2str(BrigInst* base) const;

	// map rouning method to string
	static misc::StringMap rounding_to_str_map;

	// Convert ALU modifier to string
	const char *modifier2str(unsigned short modifier) const;

	// Vector to string map
	static misc::StringMap v_to_str_map;

	// Aux function to convert operand to vector string
	const char *operandV2str(char *operand) const;

	// Convert vector to string, return string like v2, v4...
	const char *v2str(char *inst) const;

	// Compare operation to string map
	static misc::StringMap comp_op_to_str_map;

	// Convert compare operation string 
	const char *cmpOp2str(unsigned char opcode) const;

	// Pack to string map
	static misc::StringMap pack_to_str_map;

	// Convert pack to string
	const char *pack2str(unsigned char pack) const;

	// Width to string map
	static misc::StringMap width_to_str_map;

	// Returns default width
	template<class T>
	int getDefaultWidth(T *inst) const;
	
	// Convert width to string
	template<typename T>
	const char *width2str(T *inst) const;

	// Convert equiv to string
	const char *equiv2str(unsigned char val) const;

	// Returns "aligned" if the modifier says it is aligned
	// Note it is different from BrigEntry::align2str function
	const char *aligned2str(unsigned char modifier) const;
	
	// atomic operation to string map
	static misc::StringMap atomic_op_to_str_map;

	// Convert atomic operation to string
	const char *atomicOp2str(unsigned atomicOperation) const;

	// image geometry to string map
	static misc::StringMap image_geo_to_str_map;

	// Convert image geometry to string
	const char *imageGeo2str(unsigned geometry) const;
	
	// Memory fence to string map
	static misc::StringMap mem_fence_to_str_map;

	// Returns the default memory fence
	template<typename T>
	int getDefaultMemFence(T* inst) const;

	// Convert memory fence to string
	template<typename T>
	const char *memFence2str(T* inst) const;

	// Returns true is the instruction has type
	bool hasType() const;

	// Dumps calls operands. The call inst put the function name as the second
	// operand. But it should be dumped first
	void dumpCallOperands(std::ostream &os) const;

	// Dumps inst's operands
	void dumpOperands(std::ostream &os) const;

public:

	/// Constructor
	BrigInstEntry(char *buf, BrigFile *file);

	/// Returns the type field
	int getKind() const;

	/// Returns the opcode
	int getOpcode() const;

	/// Returns the type of the inst
	unsigned short getType() const;

	/// Returns the source_type field
	unsigned short getSourceType() const;

	// Returns the pointer to the operand indexed with i
	char *getOperand(int i) const;

	/// Prototype of functions that dump the inst
	typedef void (BrigInstEntry::*DumpInstFn)(std::ostream &os) const;

	/// Dumps the assembly of the instruction
	void Dump(std::ostream &os = std::cout) const
	{
		DumpInstFn fn = BrigInstEntry::dump_inst_fn[this->getKind()];
		(this->*fn)(os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const BrigInstEntry &inst)
	{
		inst.Dump(os);
		return os;
	}
	
	/// Dump functions for different kinds
	void DumpInstNone(std::ostream &os) const;
	void DumpInstBasic(std::ostream &os) const;
	void DumpInstAtomic(std::ostream &os) const;
	void DumpInstAtomicImage(std::ostream &os) const;
	void DumpInstCvt(std::ostream &os) const;
	void DumpInstBar(std::ostream &os) const;
	void DumpInstBr(std::ostream &os) const;
	void DumpInstCmp(std::ostream &os) const;
	void DumpInstFbar(std::ostream &os) const;
	void DumpInstImage(std::ostream &os) const;
	void DumpInstMem(std::ostream &os) const;
	void DumpInstAddr(std::ostream &os) const;
	void DumpInstMod(std::ostream &os) const;
	void DumpInstSeg(std::ostream &os) const;
	void DumpInstSourceType(std::ostream &os) const;
	
	/// Temporarily used, dump unimplemented insts
	void DumpInstUnsupported(const char *kind, std::ostream &os) const;

	/// Function list for dump inst. One function for a kind
	static DumpInstFn dump_inst_fn[15];

	/// Dump debug information for this inst
	void DebugInst();

	/// Returns the pointer to the directive 
	static char *GetInstByOffset(BrigFile *file, BrigCodeOffset32_t offset);
};

}  // namespace HSA

#endif
