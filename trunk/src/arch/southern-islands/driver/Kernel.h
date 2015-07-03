
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

#ifndef ARCH_SI_KERNEL_H
#define ARCH_SI_KERNEL_H

#include <memory>
#include <string>

#include <src/arch/southern-islands/disassembler/Argument.h>
#include <src/arch/southern-islands/disassembler/Binary.h>
#include <src/arch/southern-islands/emulator/WorkItem.h>


namespace SI
{

// Forward declaration
class NDRange;
class Program;

class Kernel
{
	// Kernel identifier
	int id = 0;

	// Kernel name
	std::string name;

	// Program it belongs to
	Program *program = nullptr;

	// Excerpts of program binary
	ELFReader::Symbol *metadata_symbol = nullptr;
	ELFReader::Symbol *header_symbol = nullptr;
	ELFReader::Symbol *kernel_symbol = nullptr;

	// List of kernel arguments
	std::vector<std::unique_ptr<Argument>> arguments;

	// AMD kernel binary (internal ELF)
	std::unique_ptr<Binary> binary_file;

	// Memory requirements
	int local_memory_size = 0;
	int private_memory_size = 0;

	// Kernel function metadata
	int func_uniqueid = 0;  // Id of kernel function

	// Check for head token 
	void Expect(std::vector<std::string> &token_list, std::string head_token);

	// Check for int 
	void ExpectInt(std::vector<std::string> &token_list);

	// Check size of token_list
	void ExpectCount(std::vector<std::string> &token_list, unsigned count);

	// Load metadata
	void LoadMetaDataV3();
	
	// Check metadata version
	void LoadMetaData();

	// Create buffer description
	void CreateBufferDescriptor(
			unsigned base_addr, 
			unsigned size, 
			int num_elems,
			Argument::DataType data_type,
			WorkItem::BufferDescriptor *buffer_descriptor);

public:
	Kernel(int id, const std::string &name, Program *program);

	//
	// Getters
	//

	/// Get kernel binary
	Binary *getKernelBinaryFile() const { return binary_file.get(); }

	/// Get associated program. 
	///
	/// \return Returns a pointer to a program or a nullptr if uninitialized
	Program *getProgram() const { return program; }

	/// Get reference of arguments list
	std::vector<std::unique_ptr<Argument>> &getArgs() { return arguments; }

	/// Get pointer of an argument by index in arguments list
	Argument *getArgByIndex(unsigned idx) { return arguments[idx].get(); }

	/// Get count of arguments
	unsigned getNumArguments() const { return arguments.size(); }

	/// Get mem_size_local
	int getLocalMemorySize() const { return local_memory_size; }

	/// Get mem_size_private
	int getPrivateMemorySize() const { return private_memory_size; }

	/// Get kernel id
	int getId() const { return id; }

	/// Get kernel name
	std::string getName() const { return name; }

	/// Setup NDRange constant buffers
	void SetupNDRangeConstantBuffers(NDRange *ndrange);

	/// Setup NDRange arguments
	void SetupNDRangeArgs(NDRange *ndrange);

	/// Setup NDRange state
	void DebugNDRangeState(NDRange *ndrange);

	/// Create NDRange tables
	// TODO - Add param: MMU *gpu_mmu
	void CreateNDRangeTables(NDRange *ndrange); 
	
	/// Create constant buffers
	// TODO - Add param: MMU *gpu_mmu
	void CreateNDRangeConstantBuffers(NDRange *ndrange);
	
	/// FIXME not implemented yet
	static void FlushNDRangeBuffers(NDRange *ndrange /*SIGpu *gpu, X86Emu *x86_emu*/);
	void NDRangeSetupMMU(NDRange *ndrange);
};

}  // namespace SI

#endif /* ARCH_SI_KERNEL_H */
