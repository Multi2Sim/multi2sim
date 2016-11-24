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

	// Micro-instruction dependences.
	// WARNING: if modified, update 'Uinst::dep_name' (Uinst.cc)
	// WARNING: if modified, update 'Uinst::dep_map' (Uinst.cc)
	enum Dep
	{
		DepNone = 0,

		// Integer dependences

		DepEax = 1,
		DepEcx = 2,
		DepEdx = 3,
		DepEbx = 4,
		DepEsp = 5,
		DepEbp = 6,
		DepEsi = 7,
		DepEdi = 8,

		DepEs = 9,
		DepCs = 10,
		DepSs = 11,
		DepDs = 12,
		DepFs = 13,
		DepGs = 14,

		DepZps = 15,
		DepOf = 16,
		DepCf = 17,
		DepDf = 18,

		DepAux = 19,  // Intermediate results for uinsts
		DepAux2 = 20,
		DepEa = 21,  // Internal - Effective address
		DepData = 22,  // Internal - Data for load/store

		DepIntFirst = DepEax,
		DepIntLast = DepData,
		DepIntCount = DepIntLast - DepIntFirst + 1,

		DepFlagFirst = DepZps,
		DepFlagLast = DepDf,
		DepFlagCount = DepFlagLast - DepFlagFirst + 1,


		// Floating-point dependences

		DepSt0 = 23,  // FP registers
		DepSt1 = 24,
		DepSt2 = 25,
		DepSt3 = 26,
		DepSt4 = 27,
		DepSt5 = 28,
		DepSt6 = 29,
		DepSt7 = 30,
		DepFpst = 31,  // FP status word
		DepFpcw = 32,  // FP control word
		DepFpaux = 33,  // Auxiliary FP reg

		DepFpFirst = DepSt0,
		DepFpLast = DepFpaux,
		DepFpCount = DepFpLast - DepFpFirst + 1,

		DepFpStackFirst = DepSt0,
		DepFpStackLast  = DepSt7,
		DepFpStackCount = DepFpStackLast - DepFpStackFirst + 1,


		// XMM dependences

		DepXmm0 = 34,
		DepXmm1 = 35,
		DepXmm2 = 36,
		DepXmm3 = 37,
		DepXmm4 = 38,
		DepXmm5 = 39,
		DepXmm6 = 40,
		DepXmm7 = 41,
		DepXmmData = 42,

		DepXmmFirst = DepXmm0,
		DepXmmLast = DepXmmData,
		DepXmmCount = DepXmmLast - DepXmmFirst + 1,


		// Special dependences

		DepRm8 = 0x100,
		DepRm16 = 0x101,
		DepRm32 = 0x102,

		DepIr8 = 0x200,
		DepIr16 = 0x201,
		DepIr32 = 0x202,

		DepR8 = 0x300,
		DepR16 = 0x301,
		DepR32 = 0x302,

		DepSreg = 0x400,

		DepMem8 = 0x500,
		DepMem16 = 0x501,
		DepMem32 = 0x502,
		DepMem64 = 0x503,
		DepMem80 = 0x504,
		DepMem128 = 0x505,

		DepEaseg = 0x601,  // Effective address - segment
		DepEabas = 0x602,  // Effective address - base
		DepEaidx = 0x603,  // Effective address - index

		DepSti = 0x700,  // FP - ToS+Index

		DepXmmm32 = 0x800,
		DepXmmm64 = 0x801,
		DepXmmm128 = 0x802,
		
		DepXmm = 0x900
	};

	/// String map for values of type Dep
	static const misc::StringMap dep_map;

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
	Dep dep[MaxDeps] = {};

	// Pointers to input and output dependences, pointing to internal
	// positions of 'dep', and initialized in constructor
	Dep *idep = dep;
	Dep *odep = dep + MaxIDeps;

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
	static bool isIntegerDependency(int dep)
	{
		return dep >= DepIntFirst && dep <= DepIntLast;
	}

	/// Return \c true if a micro-instruction dependency is a floating-point
	/// register.
	static bool isFloatingPointDependency(int dep)
	{
		return dep >= DepFpFirst && dep <= DepFpLast;
	}

	/// Return \c true if a micro-instruction dependency is an XMM register
	static bool isXmmDependency(int dep)
	{
		return dep >= DepXmmFirst
				&& dep <= DepXmmLast;
	}
	
	/// Return \c true if a micro-instruction dependency is a flag
	static bool isFlagDependency(int dep)
	{
		return dep >= DepFlagFirst && dep <= DepFlagLast;
	}
	
	/// Return \c true if a micro-instruction dependency is valid
	static bool isValidDependency(int dep)
	{
		return isIntegerDependency(dep) ||
				isFloatingPointDependency(dep) ||
				isXmmDependency(dep);
	}

	/// Return the name of a dependence. This is equivalent to querying
	/// string map \c uinst_dep_map.
	static const char *getDependencyName(Dep dep)
	{
		return dep_map.MapValue(dep);
	}
	
	/// Get micro-instruction information
	static Info *getInfo(Opcode opcode)
	{
		assert(opcode >= OpcodeNop && opcode < OpcodeCount);
		return &info[opcode];
	}




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

	/// Return the flags associated with the micro-instruction. Each opcode
	/// has a fixed set of associated flags.
	int getFlags() const { return info[opcode].flags; }

	/// Return the micor-instruction name. Each opcode has a fixed name.
	const std::string &getName() const { return info[opcode].name; }

	/// Return a dependence at position \a index, which must be a value
	/// between 0 and MaxDeps.
	Dep getDep(int index) const
	{
		assert(misc::inRange(index, 0, MaxDeps - 1));
		return dep[index];
	}

	/// Return an input dependence. Argument \a index must be a value
	/// between 0 and MaxIDeps - 1.
	Dep getIDep(int index) const
	{
		assert(misc::inRange(index, 0, MaxIDeps - 1));
		return idep[index];
	}

	/// Return an output dependence. Argument \a index must be a value
	/// between 0 and MaxODeps - 1.
	Dep getODep(int index) const
	{
		assert(misc::inRange(index, 0, MaxODeps - 1));
		return odep[index];
	}

	/// Get the address of a memory access micro-instruction
	unsigned getAddress() const { return address; }

	/// Get the size of a memory access micro-instruction
	int getSize() const { return size; }

	/// Set an input dependence. Argument \a index must be a value between
	/// 0 and MaxIDeps - 1. Argument \a dep should be an \c DepXXX
	/// constant.
	void setIDep(int index, int dep)
	{
		assert(misc::inRange(index, 0, MaxIDeps - 1));
		idep[index] = (Dep) dep;
	}

	/// Set an output dependence. Argument \a index must be a value between
	/// 0 and MaxODeps - 1.
	void setODep(int index, int dep)
	{
		assert(misc::inRange(index, 0, MaxODeps - 1));
		odep[index] = (Dep) dep;
	}

	/// Set a dependence using a global index. Argument \a index must be a
	/// value between 0 and MaxDeps - 1. Argument \a dep should be an
	/// \c DepXXX constant.
	void setDep(int index, int dep)
	{
		assert(misc::inRange(index, 0, MaxDeps - 1));
		this->dep[index] = (Dep) dep;
	}

	/// Add an input dependence to the instruction if there is room for it.
	/// If all dependences are taken, return \c false.
	bool addIDep(Dep dep);

	/// Add an output dependence to the instruction if there is room for it.
	/// If all dependences are taken, return \c false.
	bool addODep(Dep dep);

	/// Set the address and size of a memory access micro-instruction
	void setMemoryAccess(unsigned address, int size)
	{
		this->address = address;
		this->size = size;
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

