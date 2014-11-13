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

#include <arch/kepler/asm/Asm.h>
#include <arch/kepler/asm/Inst.h>

#include "Grid.h"
#include "Register.h"
#include "ThreadBlock.h"
#include "Warp.h"

namespace Kepler
{

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
	unsigned global_mem_access_addr;
	unsigned global_mem_access_size;

	// Emulation of ISA. This code expands to one function per ISA
	// instruction. For example:
#define DEFINST(_name, _fmt_str, ...) \
		void	ExecuteInst_##_name(Inst *inst);
#include <arch/kepler/asm/Inst.def>
#undef DEFINST

	// The special instruction appearing every 64 instructions
	// Not within ISA
	// FIXME in the future
	void ExecuteInst_Special();

	// Instruction execution table
	typedef void (Thread::*InstFunc)(Inst *inst);
	InstFunc inst_func[InstOpcodeCount];

	// Error massage for unimplemented instructions
	static void ISAUnimplemented(Inst *inst);

	// Error massage of unsupported feature
	static void ISAUnsupportedFeature(Inst *inst);

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
	unsigned ReadSR(int sr_id) { return registers.ReadSR(sr_id); }

	/// Set value of a SR
	/// \param gpr SR identifier
	/// \param value Value given as an \a unsigned typed value
	void WriteSR(int sr_id, unsigned value)
	{
		registers.WriteSR(sr_id, value);
	}

	/// Read value of a predicate register
	/// \param pr Predicate register identifier
	int ReadPred(int pr_id) { return registers.ReadPred(pr_id); }

	/// Write value of a predicate register
	/// \param pr predicate register identifier
	void WritePred(int pr_id, unsigned value)
	{
		registers.WritePred(pr_id, value);
	}

	/// Read value of Condition Code register
	bool ReadCC() { return registers.ReadCC(); }

	/// Write value of Condition Code register
	void WriteCC(bool value) { registers.WriteCC(value); }

	/// Get value of the active thread mask
	int GetActive();

	/// Get the warp the thread belong to
	Warp* getWarp() const { return warp; }

	/// Set value of the active thread mask
	/// \param value Value given as an \a unsigned typed value
	void SetActive(unsigned value);

	/// Execute an instruction
	void Execute(InstOpcode opcode, Inst *inst);

	// Execute special instruction
	void ExecuteSpecial();
};

} //namespace

#endif
