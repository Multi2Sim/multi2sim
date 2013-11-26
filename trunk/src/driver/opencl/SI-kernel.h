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

#ifndef DRIVER_OPENCL_SI_KERNEL_H
#define DRIVER_OPENCL_SI_KERNEL_H


#include <memory>
#include <string>

#include <src/arch/southern-islands/asm/Arg.h>
#include <src/mem-system/Memory.h>

#include "SI-program.h"

namespace SIDriver
{

class NDRange;

class Kernel
{
	// Kernel identifier
	int id;

	// Kernel name
	std::string name;

	// Program it belongs to
	Program *program;

	// List of kernel arguments
	std::vector<std::unique_ptr<SI::Arg>> args;


	// Memory requirements 
	int mem_size_local;
	int mem_size_private;

	// Kernel function metadata
	int func_uniqueid;  // Id of kernel function

public:
	Kernel(int id, std::string name);

	///
	void setupNDRangeConstantBuffers(NDRange *ndrange);

	/// 
	void setupNDRangeArgs(NDRange *ndrange);

	///
	void debugNDRangeState(NDRange *ndrange);

	/// FIXME
	void flushNDRangeBuffers(NDRange *ndrange /*SIGpu *gpu, X86Emu *x86_emu*/);
	void createNDRangeTables(NDRange *ndrange, Memory::Memory *gpu_mmu);
	void createNDRangeConstantBuffers(NDRange *ndrange, Memory::Memory *gpu_mmu);
	void NDRangeSetupMMU(NDRange *ndrange);
	
};

}  // namespace SIDriver

#if 0
/*
 * Kernel
 */


struct opencl_si_kernel_t
{
	int id;
	char *name;

	/* Program that kernel belongs to */
	struct opencl_si_program_t *program;

	/* List of kernel arguments, elements of type 'SIArg' */
	List *arg_list;

	/* Excerpts of program binary */
	struct elf_buffer_t metadata_buffer;
	struct elf_buffer_t header_buffer;
	struct elf_buffer_t kernel_buffer;

	/* AMD Kernel binary (internal ELF) */
	struct SIBinary *bin_file;

	/* Memory requirements */
	int mem_size_local;
	int mem_size_private;

	/* Kernel function metadata */
	int func_uniqueid;  /* Id of kernel function */
};

struct opencl_si_kernel_t *opencl_si_kernel_create(int id,
	struct opencl_si_program_t *program, char *name);
void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel);

void opencl_si_kernel_setup_ndrange_constant_buffers(
	SINDRange *ndrange);
void opencl_si_kernel_setup_ndrange_args(struct opencl_si_kernel_t *kernel, 
	SINDRange *ndrange);
void opencl_si_kernel_debug_ndrange_state(struct opencl_si_kernel_t *kernel, 
	SINDRange *ndrange);
void opencl_si_kernel_flush_ndrange_buffers(SINDRange *ndrange, SIGpu *gpu,
	X86Emu *x86_emu);
void opencl_si_kernel_create_ndrange_tables(SINDRange *ndrange, MMU *gpu_mmu);
void opencl_si_kernel_create_ndrange_constant_buffers(SINDRange *ndrange, 
	MMU *gpu_mmu);
void opencl_si_ndrange_setup_mmu(SINDRange *ndrange, MMU *cpu_mmu,
	int cpu_address_space_index, MMU *gpu_mmu, 
	unsigned int internal_tables_ptr, 
	unsigned int constant_buffers_ptr);

#endif

#endif

