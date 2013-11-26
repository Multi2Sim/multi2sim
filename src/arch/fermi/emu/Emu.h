//
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
 

#ifndef ARCH_FERMI_EMU_EMU_H
#define ARCH_FERMI_EMU_EMU_H

#include <iostream>
#include <list>

#include <lib/cpp/Debug.h>
#include <mem-system/Memory.h>


namespace Frm
{

class Asm;
class Grid;
class CUDADriver;
class ThreadBlock;

class Emu
{
	// Disassembler
	Asm *as;

	// List of grids
	std::list<Grid *> grids;
	std::list<Grid *> pending_grids;
	std::list<Grid *> running_grids;
	std::list<Grid *> finished_grids;

	// Global memory 
	Memory::Memory *global_mem;
	unsigned int global_mem_top;
	Memory::Memory *shared_mem; // shared with the CPU 

	// Constant memory, which is organized as 16 banks of 64KB each. 
	Memory::Memory *const_mem;

	// Flags indicating whether the first 32 bytes of constant memory
	// are initialized. A warning will be issued by the simulator
	// if an uninitialized element is used by the kernel. 
	int const_mem_init[32];

	unsigned int free_global_mem_size;
	unsigned int total_global_mem_size;

	// Instruction emulation table 
	FrmEmuInstFunc inst_func[FrmInstOpcodeCount];

	// Stats 
	int grid_count;  // Number of CUDA functions executed 
	long long scalar_alu_inst_count;  // Scalar ALU instructions executed 
	long long scalar_mem_inst_count;  // Scalar mem instructions executed 
	long long branch_inst_count;  // Branch instructions executed 
	long long vector_alu_inst_count;  // Vector ALU instructions executed 
	long long lds_inst_count;  // LDS instructions executed 
	long long vector_mem_inst_count;  // Vector mem instructions executed 
	long long export_inst_count; // Export instruction executed 

public:
	/// Constructor
	Emu(Asm *as);

	/// Dump emulator state
	void Dump(std::ostream &os) const;

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

