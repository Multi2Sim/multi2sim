
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

#include <src/arch/southern-islands/asm/Arg.h>
#include <src/arch/southern-islands/asm/Binary.h>

namespace SI
{

// Forward declaration
class NDRange;
class Program;
struct EmuBufferDesc;

class Kernel
{
	// Kernel identifier
	int id;

	// Kernel name
	std::string name;

	// Program it belongs to
	Program *program;

	// Excerpts of program binary
	ELFReader::Symbol *metadata_symbol;
	ELFReader::Symbol *header_symbol;
	ELFReader::Symbol *kernel_symbol;

	// List of kernel arguments
	std::vector<std::unique_ptr<SI::Arg>> args;

	// AMD kernel binary (internal ELF)
	std::unique_ptr<SI::Binary> bin_file;

	// Memory requirements
	int mem_size_local;
	int mem_size_private;

	// Kernel function metadata
	int func_uniqueid;  // Id of kernel function

	void Expect(std::vector<std::string> &token_list, std::string head_token);

	void ExpectInt(std::vector<std::string> &token_list);

	void ExpectCount(std::vector<std::string> &token_list, unsigned count);

	void LoadMetaDataV3();

	void LoadMetaData();

	void CreateBufferDesc(unsigned base_addr, 
			unsigned size, 
			int num_elems,
			ArgDataType data_type,
			EmuBufferDesc *buffer_desc);

public:
	Kernel(int id, const std::string &name, Program *program);

	/// Getters
	///
	/// Get kernel binary
	SI::Binary *getKernelBinary() const { return bin_file.get(); }

	/// Get associated program
	Program *getProgram() const { return program; }

	/// Get reference of arguments list
	std::vector<std::unique_ptr<SI::Arg>> &getArgs() { return args; }

	/// Get pointer of an argument by index in arguments list
	SI::Arg *getArgByIndex(unsigned idx) { return args[idx].get(); }

	/// Get count of arguments
	unsigned getArgsCount() const { return args.size(); }

	/// Get mem_size_local
	int getMemSizeLocal() const { return mem_size_local; }

	/// Get mem_size_private
	int getMemSizePrivate() const { return mem_size_private; }

	/// Get kernel id
	int getId() const { return id; }

	/// Get kernel name
	std::string getName() const { return name; }

	///
	void SetupNDRangeConstantBuffers(NDRange *ndrange);

	///
	void SetupNDRangeArgs(NDRange *ndrange);

	///
	void DebugNDRangeState(NDRange *ndrange);

	/// FIXME
	static void FlushNDRangeBuffers(NDRange *ndrange /*SIGpu *gpu, X86Emu *x86_emu*/);
	void CreateNDRangeTables(NDRange *ndrange /* MMU *gpu_mmu */);
	void CreateNDRangeConstantBuffers(NDRange *ndrange /*MMU *gpu_mmu*/);
	void NDRangeSetupMMU(NDRange *ndrange);

};

}  // namespace SI

#endif /* ARCH_SI_KERNEL_H */
