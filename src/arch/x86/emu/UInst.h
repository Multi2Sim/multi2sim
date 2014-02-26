/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_X86_EMU_UINST_H
#define ARCH_X86_EMU_UINST_H

#include <cassert>
#include <iostream>
#include <string>

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>


namespace x86
{


// Micro-instruction dependences.
// WARNING: if modified, update 'uinst_dep_name' (UInst.cc)
// WARNING: if modified, update 'uinst_dep_map' (UInst.cc)
enum UInstDep
{
	UInstDepNone = 0,

	// Integer dependences

	UInstDepEax = 1,
	UInstDepEcx = 2,
	UInstDepEdx = 3,
	UInstDepEbx = 4,
	UInstDepEsp = 5,
	UInstDepEbp = 6,
	UInstDepEsi = 7,
	UInstDepEdi = 8,

	UInstDepEs = 9,
	UInstDepCs = 10,
	UInstDepSs = 11,
	UInstDepDs = 12,
	UInstDepFs = 13,
	UInstDepGs = 14,

	UInstDepZps = 15,
	UInstDepOf = 16,
	UInstDepCf = 17,
	UInstDepDf = 18,

	UInstDepAux = 19,  // Intermediate results for uinsts
	UInstDepAux2 = 20,
	UInstDepEa = 21,  // Internal - Effective address
	UInstDepData = 22,  // Internal - Data for load/store

	UInstDepIntFirst = UInstDepEax,
	UInstDepIntLast = UInstDepData,
	UInstDepIntCount = UInstDepIntLast - UInstDepIntFirst + 1,

	UInstDepFlagFirst = UInstDepZps,
	UInstDepFlagLast = UInstDepDf,
	UInstDepFlagCount = UInstDepFlagLast - UInstDepFlagFirst + 1,


	// Floating-point dependences

	UInstDepSt0 = 23,  // FP registers
	UInstDepSt1 = 24,
	UInstDepSt2 = 25,
	UInstDepSt3 = 26,
	UInstDepSt4 = 27,
	UInstDepSt5 = 28,
	UInstDepSt6 = 29,
	UInstDepSt7 = 30,
	UInstDepFpst = 31,  // FP status word
	UInstDepFpcw = 32,  // FP control word
	UInstDepFpaux = 33,  // Auxiliary FP reg

	UInstDepFpFirst = UInstDepSt0,
	UInstDepFpLast = UInstDepFpaux,
	UInstDepFpCount = UInstDepFpLast - UInstDepFpFirst + 1,

	UInstDepFpStackFirst = UInstDepSt0,
	UInstDepFpStackLast  = UInstDepSt7,
	UInstDepFpStackCount = UInstDepFpStackLast - UInstDepFpStackFirst + 1,


	// XMM dependences

	UInstDepXmm0 = 34,
	UInstDepXmm1 = 35,
	UInstDepXmm2 = 36,
	UInstDepXmm3 = 37,
	UInstDepXmm4 = 38,
	UInstDepXmm5 = 39,
	UInstDepXmm6 = 40,
	UInstDepXmm7 = 41,
	UInstDepXmmData = 42,

	UInstDepXmmFirst = UInstDepXmm0,
	UInstDepXmmLast = UInstDepXmmData,
	UInstDepXmmCount = UInstDepXmmLast - UInstDepXmmFirst + 1,


	// Special dependences

	UInstDepRm8 = 0x100,
	UInstDepRm16 = 0x101,
	UInstDepRm32 = 0x102,

	UInstDepIr8 = 0x200,
	UInstDepIr16 = 0x201,
	UInstDepIr32 = 0x202,

	UInstDepR8 = 0x300,
	UInstDepR16 = 0x301,
	UInstDepR32 = 0x302,

	UInstDepSreg = 0x400,

	UInstDepMem8 = 0x500,
	UInstDepMem16 = 0x501,
	UInstDepMem32 = 0x502,
	UInstDepMem64 = 0x503,
	UInstDepMem80 = 0x504,
	UInstDepMem128 = 0x505,

	UInstDepEaseg = 0x601,  // Effective address - segment
	UInstDepEabas = 0x602,  // Effective address - base
	UInstDepEaidx = 0x603,  // Effective address - index

	UInstDepSti = 0x700,  // FP - ToS+Index

	UInstDepXmmm32 = 0x800,
	UInstDepXmmm64 = 0x801,
	UInstDepXmmm128 = 0x802,
	
	UInstDepXmm = 0x900
};

/// String map for UInstDep elements
extern misc::StringMap uinst_dep_map;


enum UInstFlag
{
	UInstFlagInt		= 0x001,  // Arithmetic integer instruction
	UInstFlagLogic		= 0x002,  // Logic computation
	UInstFlagFp		= 0x004,  // Floating-point micro-instruction
	UInstFlagMem		= 0x008,  // Memory micro-instructions
	UInstFlagCtrl		= 0x010,  // Micro-instruction affecting control flow
	UInstFlagCond		= 0x020,  // Conditional branch
	UInstFlagUncond		= 0x040,  // Unconditional jump
	UInstFlagXmm		= 0x080   // XMM micro-instruction
};


// Micro-instruction opcodes.
// WARNING: when the set of micro-instructions is modified, also update:
//   - Variable UInst::info (file UInst.cc).
//   - Variable 'fu_class_table' (src/arch/x86/timing/fu.c).
//   - Multi2Sim Guide (CISC instruction decoding)
enum UInstOpcode
{
	UInstNop = 0,

	UInstMove,
	UInstAdd,
	UInstSub,
	UInstMult,
	UInstDiv,
	UInstEffaddr,

	UInstAnd,
	UInstOr,
	UInstXor,
	UInstNot,
	UInstShift,
	UInstSign,

	UInstFpMove,
	UInstFpSign,
	UInstFpRound,

	UInstFpAdd,
	UInstFpSub,
	UInstFpComp,
	UInstFpMult,
	UInstFpDiv,

	UInstFpExp,
	UInstFpLog,
	UInstFpSin,
	UInstFpCos,
	UInstFpSincos,
	UInstFpTan,
	UInstFpAtan,
	UInstFpSqrt,

	UInstFpPush,
	UInstFpPop,

	UInstXmmAnd,
	UInstXmmOr,
	UInstXmmXor,
	UInstXmmNot,
	UInstXmmNand,
	UInstXmmShift,
	UInstXmmSign,

	UInstXmmAdd,
	UInstXmmSub,
	UInstXmmComp,
	UInstXmmMult,
	UInstXmmDiv,

	UInstXmmFpAdd,
	UInstXmmFpSub,
	UInstXmmFpComp,
	UInstXmmFpMult,
	UInstXmmFpDiv,

	UInstXmmFpSqrt,

	UInstXmmMove,
	UInstXmmShuf,
	UInstXmmConv,

	UInstLoad,
	UInstStore,
	UInstPrefetch,

	UInstCall,
	UInstRet,
	UInstJump,
	UInstBranch,
	UInstIbranch,

	UInstSyscall,

	// Last element
	UInstOpcodeCount
};


/// Information about a micro-instruction. Table UInst::info contains elements
/// of this type.
struct UInstInfo
{
	/// Name
	std::string name;

	/// Flags, specified as a bitmap of element enumerated in UInstFlag
	unsigned flags;
};


// Constants
const int UInstMaxIDeps = 3;
const int UInstMaxODeps = 4;
const int UInstMaxDeps = UInstMaxIDeps + UInstMaxODeps;


/// Class representing an x86 micro-instruction
class UInst
{
	// Table of micro-instruction information, index by a micro-instruction
	// opcode.
	static UInstInfo info[UInstOpcodeCount];

	// Unique identifier
	UInstOpcode opcode;

	// All dependences
	UInstDep dep[UInstMaxDeps];

	// Pointers to input and output dependences, pointing to internal
	// positions of 'dep'.
	UInstDep *idep;
	UInstDep *odep;

	// Memory accesses
	unsigned address;
	int size;

public:

	/// Return \c true if a micro-instruction dependency is an integer
	/// register
	static bool isDepIntReg(UInstDep dep) { return dep >= UInstDepIntFirst
			&& dep <= UInstDepIntLast; }

	/// Return \c true if a micro-instruction dependency is a floating-point
	/// register.
	static bool isDepFpReg(UInstDep dep) { return dep >= UInstDepFpFirst
			&& dep <= UInstDepFpLast; }

	/// Return \c true if a micro-instruction dependency is an XMM register
	static bool isDepXmmReg(UInstDep dep) { return dep >= UInstDepXmmFirst
			&& dep <= UInstDepXmmLast; }
	
	/// Return \c true if a micro-instruction dependency is a flag
	static bool isDepFlag(UInstDep dep) { return dep >= UInstDepFlagFirst
			&& dep <= UInstDepFlagLast; }
	
	/// Return \c true if a micro-instruction dependency is valid
	static bool isValidDep(UInstDep dep) { return isDepIntReg(dep) &&
			isDepFpReg(dep) && isDepXmmReg(dep); }

	/// Return the name of a dependence. This is equivalent to querying
	/// string map \c uinst_dep_map.
	static const char *getDepName(UInstDep dep) {
			return uinst_dep_map.MapValue(dep); }

	/// Create a micro-instruction with a given \a opcode
	UInst(UInstOpcode opcode);
	
	/// Return the micro-instruction opcode
	UInstOpcode getOpcode() const { return opcode; }

	/// Modify the micro-instruction opcode
	void setOpcode(UInstOpcode opcode) { this->opcode = opcode; }

	/// Return a dependence at position \a index, which must be a value
	/// between 0 and UInstMaxDeps.
	UInstDep getDep(int index) const {
		assert(misc::inRange(index, 0, UInstMaxDeps - 1));
		return dep[index];
	}

	/// Return an input dependence. Argument \a index must be a value
	/// between 0 and UInstMaxIDeps - 1.
	UInstDep getIDep(int index) const {
		assert(misc::inRange(index, 0, UInstMaxIDeps - 1));
		return idep[index];
	}

	/// Return an output dependence. Argument \a index must be a value
	/// between 0 and UInstMaxODeps - 1.
	UInstDep getODep(int index) const {
		assert(misc::inRange(index, 0, UInstMaxODeps - 1));
		return odep[index];
	}

	/// Get the address of a memory access micro-instruction
	unsigned getAddress() const { return address; }

	/// Get the size of a memory access micro-instruction
	int getSize() const { return size; }

	/// Set an input dependence. Argument \a index must be a value between
	/// 0 and UInstMaxIDeps - 1. Argument \a dep should be an \c UInstDepXXX
	/// constant.
	void setIDep(int index, int dep) {
		assert(misc::inRange(index, 0, UInstMaxIDeps - 1));
		idep[index] = (UInstDep) dep;
	}

	/// Set an output dependence. Argument \a index must be a value between
	/// 0 and UInstMaxODeps - 1.
	void setODep(int index, int dep) {
		assert(misc::inRange(index, 0, UInstMaxODeps - 1));
		odep[index] = (UInstDep) dep;
	}

	/// Set a dependence using a global index. Argument \a index must be a
	/// value between 0 and UInstMaxDeps - 1. Argument \a dep should be an
	/// \c UInstDepXXX constant.
	void setDep(int index, int dep) {
		assert(misc::inRange(index, 0, UInstMaxDeps - 1));
		this->dep[index] = (UInstDep) dep;
	}

	/// Add an input dependence to the instruction if there is room for it.
	/// If all dependences are taken, return \c false.
	bool addIDep(UInstDep dep);

	/// Add an output dependence to the instruction if there is room for it.
	/// If all dependences are taken, return \c false.
	bool addODep(UInstDep dep);

	/// Set the address and size of a memory access micro-instruction
	void setMemoryAccess(unsigned address, int size) {
		this->address = address;
		this->size = size;
		assert(size != 0);
	}

	/// Dump the micro-instruction into an output stream
	void Dump(std::ostream &os = std::cout) const;

	/// Short-hand invocation to Dump()
	friend std::ostream &operator<<(std::ostream &os, const UInst &uinst) {
		uinst.Dump(os);
		return os;
	}

};

}  // namespace UInst

#endif

