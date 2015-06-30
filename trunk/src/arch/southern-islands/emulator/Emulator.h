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

#ifndef ARCH_SOUTHERN_ISLANDS_EMULATOR_EMULATOR_H
#define ARCH_SOUTHERN_ISLANDS_EMULATOR_EMULATOR_H

#include <iostream>
#include <list>
#include <memory>

#include <arch/common/Emulator.h>
#include <arch/southern-islands/disassembler/Arg.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/Error.h>
#include <memory/Memory.h>

#include "NDRange.h"
#include "WorkGroup.h"
#include "WorkItem.h"


namespace SI
{

// Forward declarations
class Disassembler;
class NDRange;
class WorkGroup;


/// Southern Islands emulator.
class Emulator : public comm::Emulator
{

public:

	/// Buffer descriptor data format
	enum BufDescDataFmt
	{
		BufDescDataFmtInvalid = 0,
		BufDescDataFmt8,
		BufDescDataFmt16,
		BufDescDataFmt8_8,
		BufDescDataFmt32,
		BufDescDataFmt16_16,
		BufDescDataFmt10_11_11,
		BufDescDataFmt10_10_10_2,
		BufDescDataFmt2_10_10_10,
		BufDescDataFmt8_8_8_8,
		BufDescDataFmt32_32,
		BufDescDataFmt16_16_16_16,
		BufDescDataFmt32_32_32,
		BufDescDataFmt32_32_32_32
	};
	
	/// Buffer descriptor number format
	enum BufDescNumFmt
	{
		BufDescNumFmtInvalid = -1,  // Not part of SI spec
		BufDescNumFmtUnorm = 0,
		BufDescNumFmtSnorm,
		BufDescNumFmtUscaled,
		BufDescNumFmtSscaled,
		BufDescNumFmtUint,
		BufDescNumFmtSint,
		BufDescNumFmtSnormOgl,
		BufDescNumFmtFloat
	};

private:

	//
	// Static fields
	//

	// Debug file for ISA
	static std::string isa_debug_file;
	
	// Singleton
	static std::unique_ptr<Emulator> instance;




	//
	// Class members
	//

	// Associated disassembler
	Disassembler *disassembler = nullptr;

	// List of ND-ranges
	std::list<std::unique_ptr<NDRange>> ndranges;

	// Local to the GPU
	std::unique_ptr<mem::Memory> video_memory;

	// Pointer to the top of video memory
	unsigned video_memory_top = 0;

	// Shared with the CPU
	std::unique_ptr<mem::Memory> shared_memory;
	
	// Will point to video_mem or shared_mem
	mem::Memory *global_memory = nullptr;

	// Number of OpenCL kernels executed
	int ndrange_count = 0;              
	
	// Number of ndranges currently running
	int ndranges_running = 0;
	
	// Number of OpenCL work groups executed
	long long work_group_count = 0; 
	
	// Scalar ALU instructions executed
	long long scalar_alu_inst_count = 0; 
	
	// Scalar mem instructions executed
	long long scalar_mem_inst_count = 0; 
	
	// Branch instructions executed
	long long branch_inst_count = 0;     
	
	// Vector ALU instructions executed
	long long vector_alu_inst_count = 0; 
	
	// LDS instructions executed
	long long lds_inst_count = 0;        
	
	// Vector mem instructions executed
	long long vector_mem_inst_count = 0; 
	
	// Export instructions executed
	long long export_inst_count = 0;     

public:

	//
	// Error class
	//

	/// Exception for Southern Islands emulator
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Southern Islands emulator");
		}
	};
	



	//
	// Static fields
	//

	/// Debugger for ISA traces
	static misc::Debug isa_debug;

	/// Initialize a buffer description of type EmuBufferDesc
	static void createBufferDesc(unsigned base_addr, unsigned size,
			int num_elems, ArgDataType data_type, 
			WorkItem::BufferDescriptor *buffer_descriptor);

	/// Get the only instance of the Southern Islands emulator. If the
	/// instance does not exist yet, it will be created, and will remain
	/// allocated until the end of the execution.
	static Emulator *getInstance();

	/// Destroy the emulator singleton if allocated
	static void Destroy() { instance = nullptr; }
	
	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();




	//
	// Class members
	//

	/// Constructor
	Emulator();

	/// Return the number of allocated ND-ranges
	int getNumNDRanges() const { return ndranges.size(); }

	/// Return an iterator to the first allocated ND-range
	std::list<std::unique_ptr<NDRange>>::iterator getNDRangesBegin()
	{
		return ndranges.begin();
	}

	/// Return a past-the-end iterator to the list of allocated ND-ranges
	std::list<std::unique_ptr<NDRange>>::iterator getNDRangesEnd()
	{
		return ndranges.end();
	}

	/// Create a new ND-range and add a new ND-range to the list of
	/// allocated ND-ranges.
	NDRange *addNDRange();

	/// Remove an ND-range from the list of allocated ND-ranges and free it.
	/// All other references to this ND-range are invalidated.
	void RemoveNDRange(NDRange *ndrange);
	
	/// Returns an NDRange based on the NDRange ID field. If the id does not
	// refer to a valid NDRange, a nullptr is returned.
	/// \param id ID with which to select and NDRange.
	NDRange *getNDRangeById(unsigned id);

	/// Run one iteration of the emulation loop
	bool Run() override;

	/// Dump emulator state
	void Dump(std::ostream &os = std::cout) const;

	/// Dump emulator state (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os,
			const Emulator &emulator)
	{
		emulator.Dump(os);
		return os;
	}

	/// Get a new NDRange ID
	unsigned getNewNDRangeID() { return ndrange_count++; }
	
	/// Get global memory
	mem::Memory *getGlobalMemory() { return global_memory; }

	/// Get video_memory_top
	unsigned getVideoMemoryTop() const { return video_memory_top; }

	/// Get video_memory
	mem::Memory *getVideoMemory() { return video_memory.get(); }

	/// Set global_memory
	void setGlobalMemory(mem::Memory *memory) { global_memory = memory; }
	
	/// Set work_group_count
	void setWorkGroupCount(long long count) { work_group_count = count; }

	// FIXME - List of running NDRanges already exists. This might need to
	// go if the x86 emulator does not need the number of ndranges like in
	// the old C code.
	/// Increment the number of running ND-ranges
	void incNDRangesRunning() 
	{ 
		// Check number of ndranges running before incrementing
		assert(ndranges_running >= 0); 
		ndranges_running++; 
	}
	
	/// Decrement ndranges_running
	void decNDRangesRunning() 
	{ 
		// Check number number of ndranges running after decrementing
		ndranges_running--; 
		assert(ndranges_running  >= 0); 
	}
	
	/// Increment work_group_count
	void incWorkGroupCount() { work_group_count++; }

	/// Increment scalar_alu_inst_count
	void incScalarAluInstCount() { scalar_alu_inst_count++; }

	/// Increment scalar_mem_inst_count
	void incScalarMemInstCount() { scalar_mem_inst_count++; }

	/// Increment branch_inst_count
	void incBranchInstCount() { branch_inst_count++; }

	/// Increment vector_alu_inst_count
	void incVectorAluInstCount() { vector_alu_inst_count++; }

	/// Increment lds_inst_count
	void incLdsInstCount() { lds_inst_count++; }

	/// Increment vector_mem_inst_count
	void incVectorMemInstCount() { vector_mem_inst_count++; }

	/// Increment export_inst_count
	void incExportInstCount() { export_inst_count++; }

	/// Dump the statistics summary
	void DumpSummary(std::ostream &os);

	/// Increase video memory top
	void incVideoMemoryTop(unsigned inc) { video_memory_top += inc; }
};




}  // namespace SI

#endif

