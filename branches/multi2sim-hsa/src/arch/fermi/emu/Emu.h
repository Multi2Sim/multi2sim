/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_FERMI_EMU_EMU_H
#define ARCH_FERMI_EMU_EMU_H

#include <iostream>
#include <list>

#include <arch/fermi/asm/Wrapper.h>
#include <lib/cpp/Debug.h>
#include <memory/Memory.h>


namespace Frm
{

class Asm;
class Grid;
class CUDADriver;
class ThreadBlock;
class Thread;
struct FrmInstWrap;

class Emu
{
	// Disassembler
	Asm *as;

	// Grids
	std::list<Grid *> grids;
	std::list<Grid *> pending_grids;
	std::list<Grid *> running_grids;
	std::list<Grid *> finished_grids;

	// Memory
	Memory::Memory *global_mem;
	Memory::Memory *const_mem;
	Memory::Memory *shared_mem;

	// Global memory parameters
	unsigned global_mem_top;
	unsigned global_mem_free_size;
	unsigned global_mem_total_size;

	// Flags indicating whether the first 32 bytes of constant memory
	// are initialized. A warning will be issued by the simulator
	// if an uninitialized element is used by the kernel. 
	bool const_mem_init[32];

	// Instruction emulation table 
	typedef void (*InstFunc)(Frm::Thread *thread, struct FrmInstWrap *inst);
	InstFunc inst_func[FrmInstOpcodeCount];

	// Stats 
	long long alu_inst_count;  // ALU instructions executed
	long long branch_inst_count;  // Branch instructions executed 
	long long ldst_inst_count;  // LDST instructions executed

	/// Constructor
	Emu(Asm *as);

	// Unique instance of Fermi emulator
	static std::unique_ptr<Emu> instance;

public:
	/// Get the only instance of the Fermi emulator. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static Emu *getInstance();

	/// Dump emulator state
	void Dump(std::ostream &os = std::cout) const;

	/// Dump emulator state (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os, const Emu &emu) {
		emu.Dump(os);
		return os;
	}

	/// Dump the statistics summary
	void DumpSummary(std::ostream &os);

	/// Run one iteration of the emulation loop
	void Run();
};

}  // namespace

#endif

