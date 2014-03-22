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

#ifdef __cplusplus

#include <iostream>
#include <list>
#include <memory>

#include <mem-system/Memory.h>
#include <arch/kepler/asm/Asm.h>

namespace Kepler
{

class Asm;
class Grid;
class CUDADriver;
class ThreadBlock;
class Thread;
struct KplInstWrap;

typedef void (*InstFunc)(Kepler::Thread *thread, Inst *inst);

/*
 * Class 'KplEmu'
 */
class Emu
{
	static std::unique_ptr<Emu> instance;
	// Disassembler
	Asm *as;

	// Grids
	std::list<Grid *> grids;
	std::list<Grid *> pending_grids;
	std::list<Grid *> running_grids;
	std::list<Grid *> finished_grids;

	// Memory
	mem::Memory *global_mem;
	mem::Memory *const_mem;
	mem::Memory *shared_mem;

	// Global memory parameters
	unsigned global_mem_top;
	unsigned global_mem_free_size;
	unsigned global_mem_total_size;

	// Flags indicating whether the first 32 bytes of constant memory
	// are initialized. A warning will be issued by the simulator
	// if an uninitialized element is used by the kernel.
	bool const_mem_init[32];

	// Instruction emulation table
	InstFunc inst_func[InstOpcodeCount];

	// Stats		//make it clear
	/*
	long long alu_inst_count;  // ALU instructions executed
	long long branch_inst_count;  // Branch instructions executed
	long long ldst_inst_count;  // LDST instructions executed
*/
	long long branch_inst_count;
	long long alu_inst_count;
	long long shared_mem_inst_count;
	long long global_mem_inst_count;

	/// Constructor
	Emu();

public:

	///Kepler emulator maximum cycles
	long long emu_max_cycles;

	///Kepler emulator maximum number of instructions
	long long emu_max_inst;

	///Kepler emulator maximum number of functions
	int emu_max_functions;
	static const int emu_warp_size = 32;

	/// Get the only instance of the Kepler emulator. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static Emu *getInstance();

	// Getters
	/// Get grid list size
	unsigned getGridSize() { return grids.size(); }

	/// Get the assambler
	Asm *getAsm() const { return as;}

	/// Get global memory top
	unsigned getGlobalMemTop() const { return global_mem_top; }

	/// Get instruction emulation table
	InstFunc getInstFunc(InstOpcode inst) { return inst_func[inst]; }

	/// Get global memory free size
	unsigned getGlobalMemFreeSize() const { return global_mem_free_size; }

	/// Get global memory Total size
	unsigned getGlobalMemTotalSize() const { return global_mem_total_size; }

	/// Get ALU instruction count
	unsigned getAluInstCount() const { return alu_inst_count; }

	// Setter
	/// set global memory top
	void SetGlobalMemTop(unsigned value) { global_mem_top = value; }

	/// Set global memory free size
	void setGlobalMemFreeSize(unsigned value) { global_mem_free_size = value; }

	/// Set global memory total size
	void setGlobalMemTotalSize(unsigned value) { global_mem_total_size = value; }

	/// Increment ALU instruction counter
	void incAluInstCount() { alu_inst_count ++; }

	/// Dump Kepler Emulator in a human-readable fashion into an output stream (or
	/// standard output if argument \a os is omitted.
	void Dump(std::ostream &os = std::cout) const;

	/// Dump emulator state (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os, const Emu &emu) {
		emu.Dump(os);
		return os;
	}

	/// Dump the statistics summary
	void DumpSummary(std::ostream &os);

	/// Run one iteration of the emulation loop
	bool Run();

	/// push back an element in grids list
	void GridsPushBack(Grid *grid);

	/// Write Constant Memory
	/// \param starting address to be written in
	/// \param size of data
	/// \param data buffer
	void WriteConstMem(unsigned addr, unsigned size, const char *buf);

	/// Write Global Memory
	/// \param starting address to be written in
	/// \param size of data
	/// \param data buffer
	void WriteGlobalMem(unsigned addr, unsigned size, const char *buf);

	/// Read Global Memory
	/// \param starting address to be read in
	/// \param size of data
	/// \param data buffer
	void ReadConstMem(unsigned addr, unsigned size, char *buf);

	/// Read Global Memory
	/// \param starting address to be read in
	/// \param size of data
	/// \param data buffer
	void ReadGlobalMem(unsigned addr, unsigned size, char *buf);

	/// Push an element into pending grid list
	void PushPendingGrid(Grid *grid);
};


}	//namespace

#endif

#endif
