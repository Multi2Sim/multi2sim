/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
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

#ifndef ARCH_KEPLER_EMU_EMU_H
#define ARCH_KEPLER_EMU_EMU_H

#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include <arch/common/Arch.h>
#include <arch/common/Emulator.h>
#include <arch/kepler/disassembler/Disassembler.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <memory/Memory.h>


namespace Kepler
{

class Disassembler;
class Grid;
class CUDADriver;
class ThreadBlock;
class Thread;
class Function;

class Emulator : public comm::Emulator
{
	// Debugger file
	static std::string isa_debug_file;

	// Simulation kind
	static comm::Arch::SimKind sim_kind;

	// Emu singleton instance
	static std::unique_ptr<Emulator> instance;

	// Disassembler
	Disassembler *disassembler;

	// List of Grids created by the guest application
	std::vector<std::unique_ptr<Grid>> grids;

	// List of Grid
	std::list<Grid*> pending_grids;
	std::list<Grid*> running_grids;
	std::list<Grid*> finished_grids;

	// Memory
	std::unique_ptr<mem::Memory> global_memory;
	std::unique_ptr<mem::Memory> constant_memory;
	std::unique_ptr<mem::Memory> shared_memory;

	// Field initialized in constructor. Global memory total size
	unsigned global_memory_total_size;

	// Field initialized in constructor. Shared memory total size
	unsigned shared_memory_total_size;

	// Flags indicating whether the first 32 bytes of constant memory
	// are initialized. A warning will be issued by the simulator
	// if an uninitialized element is used by the kernel.
	bool const_mem_init[32];

	//
	// Statistics
	//

	// Number of kernels executed
	int num_grids = 0;

	// Number of CUDA thread block executed
	long long num_thread_blocks = 0;

	// Number of integer instructions executed
	long long num_integer_instructions = 0;

	// Number of single floating point instructions executed
	long long num_fp32_instructions = 0;

	// Number of double floating point instructions executed
	long long num_fp64_instructions = 0;

	// Number of flow control instructions executed
	long long num_control_instructions = 0;

	// Number of conversion instructions executed
	long long num_conversion_instructions = 0;

	// Number of load store instructions executed
	long long num_ls_instructions = 0;

	// Number of movement instructions executed
	long long num_movement_instructions = 0;

	// Number of predicate instructions executed
	long long num_predicate_instructions = 0;

	// Number of Miscellaneous instructions executed
	long long num_misc_instructions = 0;

	// Num of total instructions
	long long num_instructions = 0;

	/// Constructor
	Emulator();

public:

	/// Runtime error for Kepler
	class Error : public misc::Error
	{
	public:

		/// Constructor
		Error(const std::string &message) : misc::Error(message)
		{
			// Add module prefix
			AppendPrefix("Kepler emulator");
		}
	};

	/// Debugger
	static misc::Debug isa_debug;

	/// Kepler emulator maximum cycles
	long long max_cycles;

	/// Kepler emulator maximum number of instructions
	long long max_instructions;

	/// Kepler emulator maximum number of functions
	int max_functions;

	/// Warp size
	static const int warp_size = 32;

	// Field initialized in constructor. Global memory top address
	unsigned global_memory_top;

	// Field initialized in constructor. Global memory freed size
	unsigned global_memory_free_size;

	/// Get the only instance of the Kepler emulator. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static Emulator *getInstance();

	/// Return the simulation level set by command-line option
	/// '-kpl-sim-kind'
	static comm::Arch::SimKind getSimKind() { return sim_kind; }

	/// Get the assambler
	Disassembler *getAsm() const { return disassembler;}

	/// Get global memory top
	unsigned getGlobalMemoryTop() const { return global_memory_top; }

	/// Get global memory free size
	unsigned getGlobalMemoryFreeSize() const { return global_memory_free_size; }

	/// Get global memory Total size
	unsigned getGlobalMemoryTotalSize() const
	{
		return global_memory_total_size;
	}

	/// Get Shared memory total size
	unsigned getSharedMemoryTotalSize() const
	{
		return shared_memory_total_size;
	}

	/// Get global memory
	mem::Memory *getGlobalMemory() const  { return global_memory.get(); }

	/// Get constant memory
	mem::Memory *getConstMemory() const { return constant_memory.get();}

	/// Return the number of available grids
	int getNumGrids() { return grids.size(); }

	/// Return number of available pending grids
	int getNumPendingGrids() { return pending_grids.size(); }

	/// Return number of available running grids
	int getNumRunningGrids() { return running_grids.size(); }

	/// Return the grid with the given identifier, or `nullptr` if the
	/// identifier does not correspond to a valid module.
	Grid *getGrid(int index)
	{
		if (grids.size() > 0)
			return misc::inRange((unsigned) index, 0, grids.size()) ?
				grids[index].get() :
				nullptr;
		else
			return nullptr;
	}

	/// Set global memory top
	void SetGlobalMemoryTop(unsigned global_memory_top)
	{
		this->global_memory_top = global_memory_top;
	}

	/// Set global memory free size
	void setGlobalMemoryFreeSize(unsigned global_memory_free_size)
	{
		this->global_memory_free_size = global_memory_free_size;
	}

	/// Increase number of integer instructions by 1
	void incNumIntegerInstructions() { num_integer_instructions++; }

	/// Increase number of fp32 instructions by 1
	void incNumFP32Instructions() { num_fp32_instructions++; }

	/// Increase number of fp64 instructions by 1
	void incNumFP64Instructions() { num_fp64_instructions++; }

	/// Increase number of control instructions by 1
	void incNumControlInstructions() { num_control_instructions++; }

	/// Increase number of conversion instructions by 1
	void incNumConversionInstructions() { num_conversion_instructions++; }

	/// Increase number of load store instructions by 1
	void incNumLSInstructions() { num_ls_instructions++; }

	/// Increase number of movement instructions by 1
	void incNumMovmentInstructions() { num_movement_instructions++; }

	/// Increase number of predicate instructions by 1
	void incNumPredicateInstructions() { num_predicate_instructions++; }

	/// Increase number of misc instructions by 1
	void incNumMiscInstructions() { num_misc_instructions++; }

	/// Increase number of total instructions by 1
	void incNumInstructions() { num_instructions++; }

	/// Increse global memory top
	void incGloablMemoryTop(unsigned inc) { global_memory_top += inc; }

	/// Dump Kepler Emulator in a human-readable fashion into an output
	/// stream (or standard output if argument \a os is omitted.
	void Dump(std::ostream &os = std::cout) const;

	/// Dump emulator state (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os, const Emulator &emu)
	{
		emu.Dump(os);
		return os;
	}

	/// Dump the statistics summary
	void DumpSummary(std::ostream &os) const;

	/// Run one iteration of the emulation loop
	bool Run();

	/// Write Constant Memory
	/// \param starting address to be written in
	/// \param size of data
	/// \param data buffer
	void WriteConstantMemory(unsigned address, unsigned size, const char *buffer)
	{
		constant_memory->Write(address, size, buffer);
	}

	/// Write Global Memory
	/// \param starting address to be written in
	/// \param size of data
	/// \param data buffer
	void WriteGlobalMemory(unsigned address, unsigned size, const char *buffer)
	{
		global_memory->Write(address, size, buffer);
	}

	/// Read Global Memory
	/// \param starting address to be read in
	/// \param size of data
	/// \param data buffer
	void ReadConstantMemory(unsigned address, unsigned size, char *buffer)
	{
		constant_memory->Read(address, size, buffer);
	}

	/// Read Global Memory
	/// \param starting address to be read in
	/// \param size of data
	/// \param data buffer
	void ReadGlobalMemory(unsigned address, unsigned size, char *buffer)
	{
		global_memory->Read(address, size, buffer);
	}

	/// Push an element into pending grid list
	void PushPendingGrid(Grid *grid);

	/// Pop front from pending grid list
	void PopPendingGrid()
	{
		pending_grids.pop_front();
	}

	/// Create a new grid to the grid list and return a pointer to it.
	Grid *addGrid(Function *function);

	/// Return an iterator to the first pending grid
	std::list<Grid*>::iterator getPendingGridsBegin()
	{
		return pending_grids.begin();
	}

	/// Return an iterator to the last pending grid
	std::list<Grid*>::iterator getPendingGridsEnd()
	{
		return pending_grids.end();
	}

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};

}  //namespace Kepler

#endif
