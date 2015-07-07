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

#ifndef ARCH_X86_EMULATOR_UINST_H
#define ARCH_X86_EMULATOR_UINST_H

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


/// Class representing an x86 micro-instruction
class Uinst
{
public:

	// Micro-instruction opcodes.
	// WARNING: when the set of micro-instructions is modified, also update:
	//   - Variable UInst::info (file UInst.cc).
	//   - ALU timing information (src/arch/x86/timing/ALU.cc).
	//   - Multi2Sim Guide (CISC instruction decoding)
	enum Opcode
	{
		OpcodeNop = 0,

		OpcodeMove,
		OpcodeAdd,
		OpcodeSub,
		OpcodeMult,
		OpcodeDiv,
		OpcodeEffaddr,

		OpcodeAnd,
		OpcodeOr,
		OpcodeXor,
		OpcodeNot,
		OpcodeShift,
		OpcodeSign,

		OpcodeFpMove,
		OpcodeFpSign,
		OpcodeFpRound,

		OpcodeFpAdd,
		OpcodeFpSub,
		OpcodeFpComp,
		OpcodeFpMult,
		OpcodeFpDiv,

		OpcodeFpExp,
		OpcodeFpLog,
		OpcodeFpSin,
		OpcodeFpCos,
		OpcodeFpSincos,
		OpcodeFpTan,
		OpcodeFpAtan,
		OpcodeFpSqrt,

		OpcodeFpPush,
		OpcodeFpPop,

		OpcodeXmmAnd,
		OpcodeXmmOr,
		OpcodeXmmXor,
		OpcodeXmmNot,
		OpcodeXmmNand,
		OpcodeXmmShift,
		OpcodeXmmSign,

		OpcodeXmmAdd,
		OpcodeXmmSub,
		OpcodeXmmComp,
		OpcodeXmmMult,
		OpcodeXmmDiv,

		OpcodeXmmFpAdd,
		OpcodeXmmFpSub,
		OpcodeXmmFpComp,
		OpcodeXmmFpMult,
		OpcodeXmmFpDiv,

		OpcodeXmmFpSqrt,

		OpcodeXmmMove,
		OpcodeXmmShuf,
		OpcodeXmmConv,

		OpcodeLoad,
		OpcodeStore,
		OpcodePrefetch,

		OpcodeCall,
		OpcodeRet,
		OpcodeJump,
		OpcodeBranch,
		OpcodeIbranch,

		OpcodeSyscall,

		// Last element
		OpcodeCount
	};

	/// Flags providing information about micro-instructions
	enum Flag
	{
		FlagInt		= 0x001,  // Arithmetic integer instruction
		FlagLogic	= 0x002,  // Logic computation
		FlagFp		= 0x004,  // Floating-point micro-instruction
		FlagMem		= 0x008,  // Memory micro-instructions
		FlagCtrl	= 0x010,  // Micro-instruction affecting control flow
		FlagCond	= 0x020,  // Conditional branch
		FlagUncond	= 0x040,  // Unconditional jump
		FlagXmm		= 0x080   // XMM micro-instruction
	};

	/// Information about a micro-instruction. Table UInst::info contains
	/// elements of this type.
	struct Info
	{
		/// Name
		std::string name;

		/// Flags, specified as a bitmap of element enumerated in Flag
		unsigned flags;
	};

	/// Maximum number of input dependencies
	static const int MaxIDeps = 3;

	/// Maximum number of output dependencies
	static const int MaxODeps = 4;

	/// Total number of dependencies
	static const int MaxDeps = MaxIDeps + MaxODeps;


private:

	// Table of micro-instruction information, index by a micro-instruction
	// opcode.
	static Info info[OpcodeCount];

	// Unique identifier for micro-instruction, initialized in constructor
	Opcode opcode;

	// All dependences
	UInstDep dep[MaxDeps] = {};

	// Pointers to input and output dependences, pointing to internal
	// positions of 'dep', and initialized in constructor
	UInstDep *idep = dep;
	UInstDep *odep = dep + MaxIDeps;

	// Address of the last memory access for this instruction, if it is
	// a memory micro-instruction
	unsigned address = 0;

	// Size of the memory access, if the micro-instruction is a memory
	// access.
	int size = 0;

public:

	//
	// Static functions
	//

	/// Return \c true if a micro-instruction dependency is an integer
	/// register
	static bool isDepIntReg(UInstDep dep)
	{
		return dep >= UInstDepIntFirst
				&& dep <= UInstDepIntLast;
	}

	/// Return \c true if a micro-instruction dependency is a floating-point
	/// register.
	static bool isDepFpReg(UInstDep dep)
	{
		return dep >= UInstDepFpFirst
				&& dep <= UInstDepFpLast;
	}

	/// Return \c true if a micro-instruction dependency is an XMM register
	static bool isDepXmmReg(UInstDep dep)
	{
		return dep >= UInstDepXmmFirst
				&& dep <= UInstDepXmmLast;
	}
	
	/// Return \c true if a micro-instruction dependency is a flag
	static bool isDepFlag(UInstDep dep)
	{
		return dep >= UInstDepFlagFirst
				&& dep <= UInstDepFlagLast;
	}
	
	/// Return \c true if a micro-instruction dependency is valid
	static bool isValidDep(UInstDep dep)
	{
		return isDepIntReg(dep) &&
				isDepFpReg(dep) &&
				isDepXmmReg(dep);
	}

	/// Return the name of a dependence. This is equivalent to querying
	/// string map \c uinst_dep_map.
	static const char *getDepName(UInstDep dep)
	{
		return uinst_dep_map.MapValue(dep);
	}
	
	/// Get micro-instruction information
	static Info *getInfo() { return info; }




	//
	// Member functions
	//

	/// Create a micro-instruction with a given \a opcode
	Uinst(Opcode opcode) : opcode(opcode)
	{
	}
	
	/// Return the micro-instruction opcode
	Opcode getOpcode() const { return opcode; }

	/// Modify the micro-instruction opcode
	void setOpcode(Opcode opcode) { this->opcode = opcode; }

	/// Return a dependence at position \a index, which must be a value
	/// between 0 and MaxDeps.
	UInstDep getDep(int index) const
	{
		assert(misc::inRange(index, 0, MaxDeps - 1));
		return dep[index];
	}

	/// Return an input dependence. Argument \a index must be a value
	/// between 0 and MaxIDeps - 1.
	UInstDep getIDep(int index) const
	{
		assert(misc::inRange(index, 0, MaxIDeps - 1));
		return idep[index];
	}

	/// Return an output dependence. Argument \a index must be a value
	/// between 0 and MaxODeps - 1.
	UInstDep getODep(int index) const
	{
		assert(misc::inRange(index, 0, MaxODeps - 1));
		return odep[index];
	}

	/// Get the address of a memory access micro-instruction
	unsigned getAddress() const { return address; }

	/// Get the size of a memory access micro-instruction
	int getSize() const { return size; }

	/// Set an input dependence. Argument \a index must be a value between
	/// 0 and MaxIDeps - 1. Argument \a dep should be an \c UInstDepXXX
	/// constant.
	void setIDep(int index, int dep)
	{
		assert(misc::inRange(index, 0, MaxIDeps - 1));
		idep[index] = (UInstDep) dep;
	}

	/// Set an output dependence. Argument \a index must be a value between
	/// 0 and MaxODeps - 1.
	void setODep(int index, int dep)
	{
		assert(misc::inRange(index, 0, MaxODeps - 1));
		odep[index] = (UInstDep) dep;
	}

	/// Set a dependence using a global index. Argument \a index must be a
	/// value between 0 and MaxDeps - 1. Argument \a dep should be an
	/// \c UInstDepXXX constant.
	void setDep(int index, int dep)
	{
		assert(misc::inRange(index, 0, MaxDeps - 1));
		this->dep[index] = (UInstDep) dep;
	}

	/// Add an input dependence to the instruction if there is room for it.
	/// If all dependences are taken, return \c false.
	bool addIDep(UInstDep dep);

	/// Add an output dependence to the instruction if there is room for it.
	/// If all dependences are taken, return \c false.
	bool addODep(UInstDep dep);

	/// Set the address and size of a memory access micro-instruction
	void setMemoryAccess(unsigned address, int size)
	{
		this->address = address;
		this->size = size;
		assert(size != 0);
	}

	/// Dump the micro-instruction into an output stream
	void Dump(std::ostream &os = std::cout) const;

	/// Short-hand invocation to Dump()
	friend std::ostream &operator<<(std::ostream &os, const Uinst &uinst)
	{
		uinst.Dump(os);
		return os;
	}
};

}  // namespace UInst

#endif

