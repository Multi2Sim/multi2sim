/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_KEPLER_EMU_THREAD_H
#define ARCH_KEPLER_EMU_THREAD_H

#include <arch/kepler/disassembler/Disassembler.h>
#include "../disassembler/Instruction.h"
#include "Emulator.h"
#include "Grid.h"
#include "Register.h"
#include "ThreadBlock.h"
#include "Warp.h"

namespace Kepler
{

class Emulator;
class Warp;
class ThreadBlock;
class Grid;

/// Abstract polymorphic class used to attach additional information to the
/// thread. The timing simulator can created objects derived from this class
/// and link them with the thread.

class ThreadData
{
public:
	virtual ~ThreadData();
};

class Thread
{

	/*
	/// Memory accesses types
	enum MemoryAccessType
	{
		MemoryAccessInvalid = 0,
		MemoryAccessRead,
		MemoryAccessWrite
	};

	/// Memory accesses attributes
	struct MemoryAccess
	{
		MemoryAccessType type;
		unsigned addr;
		unsigned size;
	};
	*/

private:

	// Emulator
	Emulator *emulator;

	// IDs
	int id;
	int id_in_thread_block;
	unsigned id_in_warp;

	// 3D IDs
	int id_3d[3];
	int id_in_thread_block_3d[3];

	// Warp, thread-block, and grid where it belongs
	Warp *warp;
	ThreadBlock *thread_block;
	Grid *grid;

	// Registers
	Register registers;

	// Last global memory access
	unsigned global_memory_access_address;
	unsigned global_memory_access_size;

	// Local Memory
	std::unique_ptr<mem::Memory> local_memory;

	// Local memory size
	unsigned local_memory_size; // currently set as 1MB

	// Local memory top local address
	unsigned local_memory_top_address;

	// Local memory top generic address
	unsigned local_memory_top_generic_address;

	// Emulation of ISA. This code expands to one function per ISA
	// instruction. For example:
#define DEFINST(_name, _fmt_str, ...) \
		void	ExecuteInst_##_name(Instruction *inst);
#include "../disassembler/Instruction.def"
#undef DEFINST

	// The special instruction appearing every 64 instructions
	// Not within ISA
	// FIXME in the future
	void ExecuteInst_Special();

	// Instruction execution table
	typedef void (Thread::*InstFunc)(Instruction *inst);
	InstFunc inst_func[Instruction::OpcodeCount];

	// Error massage for unimplemented instructions
	static void ISAUnimplemented(Instruction *inst);

	// Error massage of unsupported feature
	static void ISAUnsupportedFeature(Instruction *inst);

	// Fields below are used for architectural simulation only.
public :

	/// Constructor
	/// \param Warp Warp that it belongs to
	/// \id Global 1D identifier of the thread
	Thread(Warp *warp, int id);

	/// Get global id
	unsigned getId() const { return id; }

	/// Get id_in_warp;
	unsigned getIdInWarp() const { return id_in_warp; }

	/// Get warp_id
	unsigned getWarpId() const { return warp->getId(); }

	/// Get value of a GPR
	/// \param vreg GPR identifier
	unsigned ReadGPR(int gpr_id) { return registers.ReadGPR(gpr_id); }

	/// Get float type value of a GPR
	/// \param vreg GPR identifier
	float ReadFloatGPR(int gpr_id) { return registers.ReadFloatGPR(gpr_id); }

	/// Set value of a GPR
	/// \param gpr GPR idenfifier
	/// \param value Value given as an \a unsigned typed value
	void WriteGPR(int gpr_id, unsigned value)
	{
		registers.WriteGPR(gpr_id, value);
	}

	/// Set float value of a GPR
	/// \param gpr GPR idenfifier
	/// \param value Value given as an \a float typed value
	void WriteFloatGPR(int gpr_id, float value)
	{
		registers.WriteFloatGPR(gpr_id, value);
	}

	/// Get value of a SR
	/// \param vreg SR identifier
	unsigned ReadSpecialRegister(int special_register_id)
	{
		return registers.ReadSpecialRegister(special_register_id);
	}

	/// Set value of a SR
	/// \param gpr SR identifier
	/// \param value Value given as an \a unsigned typed value
	void WriteSpecialRegister(int special_register_id, unsigned value)
	{
		registers.WriteSpecialRegister(special_register_id, value);
	}

	/// Read value of a predicate register
	/// \param pr Predicate register identifier
	int ReadPredicate(int predicate_id)
	{
		return registers.ReadPredicate(predicate_id);
	}

	/// Write value of a predicate register
	/// \param pr predicate register identifier
	void WritePredicate(int pr_id, unsigned value)
	{
		registers.WritePredicate(pr_id, value);
	}

	/// Read value of Condition Code register
	unsigned ReadCC_ZF() { return registers.ReadCC_ZF(); }

	/// Read value of Condition Code register
	unsigned ReadCC_SF() { return registers.ReadCC_SF(); }

	/// Read value of Condition Code register
	unsigned ReadCC_CF() { return registers.ReadCC_CF(); }

	/// Read value of Condition Code register
	unsigned ReadCC_OF() { return registers.ReadCC_OF(); }

	/// Write value of Condition Code register
	void WriteCC_ZF(unsigned value) { registers.WriteCC_ZF(value); }

	/// Write value of Condition Code register
	void WriteCC_SF(unsigned value) { registers.WriteCC_SF(value); }

	/// Write value of Condition Code register
	void WriteCC_CF(unsigned value) { registers.WriteCC_CF(value); }

	/// Write value of Condition Code register
	void WriteCC_OF(unsigned value) { registers.WriteCC_OF(value); }

	/// Get value of the active thread mask
	int GetActive();

	/// Get the warp the thread belong to
	Warp* getWarp() const { return warp; }

	/// Set value of the active thread mask
	/// \param value Value given as an \a unsigned typed value
	void SetActive(unsigned value);

	/// Execute an instruction
	void Execute(Instruction::Opcode opcode, Instruction *inst);

	/// Execute special instruction
	void ExecuteSpecial();

	/// Read Register
	void Read_register(unsigned *dst, int gpr_id)
	{
		registers.Read_register(dst, gpr_id);
	}

	/// Write Register
	void Write_register(unsigned *src, int gpr_id)
	{
		registers.Write_register(src, gpr_id);
	}

};

} //namespace

#endif
