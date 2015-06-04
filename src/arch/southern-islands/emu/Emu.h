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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_EMU_H
#define ARCH_SOUTHERN_ISLANDS_EMU_EMU_H

#include <iostream>
#include <list>
#include <memory>

#include <lib/cpp/Debug.h>
#include <lib/cpp/Error.h>
#include <memory/Memory.h>


// Forward declarations
namespace Driver
{
	class OpenCLSIDriver;
}

namespace SI
{

// Forward declarations
class Asm;
class NDRange;
class ShaderExport;
class WorkGroup;

/// Table 8.5 in SI documentation
struct EmuBufferDesc
{
	unsigned long long base_addr : 48;   //    [47:0]
	unsigned int stride          : 14;   //   [61:48]
	unsigned int cache_swizzle   : 1;    //       62 
	unsigned int swizzle_enable  : 1;    //       63 
	unsigned int num_records     : 32;   //   [95:64]
	unsigned int dst_sel_x       : 3;    //   [98:96]
	unsigned int dst_sel_y       : 3;    //  [101:99]
	unsigned int dst_sel_z       : 3;    // [104:102]
	unsigned int dst_sel_w       : 3;    // [107:105]
	unsigned int num_format      : 3;    // [110:108]
	unsigned int data_format     : 4;    // [114:111]
	unsigned int elem_size       : 2;    // [116:115]
	unsigned int index_stride    : 2;    // [118:117]
	unsigned int add_tid_enable  : 1;    //      119 
	unsigned int reserved        : 1;    //      120 
	unsigned int hash_enable     : 1;    //      121 
	unsigned int heap            : 1;    //      122 
	unsigned int unused          : 3;    // [125:123]
	unsigned int type            : 2;    // [127:126]
}__attribute__((packed));


/// Table 8.11 in SI documentation
struct EmuImageDesc
{
	unsigned long long base_addr : 40;   //    [39:0]
	unsigned int mid_lod         : 12;   //   [51:40]
	unsigned int data_fmt        : 6;    //   [57:52]
	unsigned int num_fmt         : 4;    //   [61:58]
	unsigned int                 : 2;    //   [63:62]
	unsigned int width           : 14;   //   [77:64]
	unsigned int height          : 14;   //   [91:78]
	unsigned int perf_mod        : 3;    //   [94:92]
	unsigned int interlaced      : 1;    //       95 
	unsigned int dst_sel_x       : 3;    //   [98:96]
	unsigned int dst_sel_y       : 3;    //  [101:99]
	unsigned int dst_sel_z       : 3;    // [104:102]
	unsigned int dst_sel_w       : 3;    // [107:105]
	unsigned int base_level      : 4;    // [111:108]
	unsigned int last_level      : 4;    // [115:112]
	unsigned int tiling_idx      : 5;    // [120:116]
	unsigned int pow2pad         : 1;    //      121 
	unsigned int                 : 2;    // [123:122]
	unsigned int type            : 4;    // [127:124]
	unsigned int depth           : 13;   // [140:128]
	unsigned int pitch           : 14;   // [154:141]
	unsigned int                 : 5;    // [159:155]
	unsigned int base_array      : 13;   // [172:160]
	unsigned int last_array      : 13;   // [185:173]
	unsigned int                 : 6;    // [191:186]
	unsigned int min_lod_warn    : 12;   // [203:192]
	unsigned long long           : 52;   // [255:204]
}__attribute__((packed));

// Table 8.12 in SI documentation
struct EmuSamplerDesc
{
	unsigned int clamp_x            : 3;    //     [2:0]
	unsigned int clamp_y            : 3;    //     [5:3]
	unsigned int clamp_z            : 3;    //     [8:6]
	unsigned int max_aniso_ratio    : 3;    //    [11:9]
	unsigned int depth_cmp_func     : 3;    //   [14:12]
	unsigned int force_unnorm       : 1;    //       15 
	unsigned int aniso_thresh       : 3;    //   [18:16]
	unsigned int mc_coord_trunc     : 1;    //       19 
	unsigned int force_degamma      : 1;    //       20 
	unsigned int aniso_bias         : 6;    //   [26:21]
	unsigned int trunc_coord        : 1;    //       27 
	unsigned int disable_cube_wrap  : 1;    //       28 
	unsigned int filter_mode        : 2;    //   [30:29] 
	unsigned int                    : 1;    //       31  
	unsigned int min_lod            : 12;   //   [43:32] 
	unsigned int max_lod            : 12;   //   [55:44] 
	unsigned int perf_mip           : 4;    //   [59:56] 
	unsigned int perf_z             : 4;    //   [63:60] 
	unsigned int lod_bias           : 14;   //   [77:64] 
	unsigned int lod_bias_sec       : 6;    //   [83:78] 
	unsigned int xy_mag_filter      : 2;    //   [85:84] 
	unsigned int xy_min_filter      : 2;    //   [87:86] 
	unsigned int z_filter           : 2;    //   [89:88] 
	unsigned int mip_filter         : 2;    //   [91:90] 
	unsigned int mip_point_preclamp : 1;    //       92  
	unsigned int disable_lsb_cell   : 1;    //       93  
	unsigned int                    : 2;    //   [95:94] 
	unsigned int border_color_ptr   : 12;   //  [107:96]
	unsigned int                    : 18;   // [125:108]
	unsigned int border_color_type  : 2;    // [127:126]
}__attribute__((packed));

// Pointers get stored in 2 consecutive 32-bit registers
struct EmuMemPtr
{
	unsigned long long addr : 48;
	unsigned int unused     : 16;
}__attribute__((packed));


/// Southern Islands emulator.
class Emu
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
	// Configuration Options
	//

	// Max number of cycles
	static long long max_cycles;

	// Max number of instructions
	static long long max_inst;

	// Maximum number of kernels
	static int max_kernels;

	// OpenGL binary name
	static std::string opengl_binary_name;

	// Size of wavefront
	static int wavefront_size;

	//
	// Start of private members
	//

	// Associated disassembler
	Asm *as = nullptr;

	// Associated drivers
	::Driver::OpenCLSIDriver *opencl_driver = nullptr;

	// Local to the GPU
	std::unique_ptr<mem::Memory> video_memory;

	// Pointer to the top of video memory
	unsigned video_memory_top = 0;

	// Shared with the CPU
	std::unique_ptr<mem::Memory> shared_memory;
	
	// Will point to video_mem or shared_mem
	mem::Memory *global_memory = nullptr;

	int address_space_index = 0;

	// Current ND-Range
	NDRange *ndrange = nullptr;

	// Work-group lists
	std::list<WorkGroup *> waiting_work_groups;
	std::list<WorkGroup *> running_work_groups;

	// Statistics
	int ndrange_count = 0;              // Number of OpenCL kernels executed
	long long work_group_count = 0; // Number of OpenCL work groups executed
	long long scalar_alu_inst_count = 0; // Scalar ALU instructions executed
	long long scalar_mem_inst_count = 0; // Scalar mem instructions executed
	long long branch_inst_count = 0;     // Branch instructions executed
	long long vector_alu_inst_count = 0; // Vector ALU instructions executed
	long long lds_inst_count = 0;        // LDS instructions executed
	long long vector_mem_inst_count = 0; // Vector mem instructions executed
	long long export_inst_count = 0;     // Export instructions executed

	// Unique instance of Southern Islands Emulator
	static std::unique_ptr<Emu> instance;

	// Private constructor. The only possible instance of the OpenCL Driver
	// can be obtained with a call to getInstance()
	Emu();

public:
	/// UAV Table
	static const unsigned MaxNumUAVs = 16;
	static const unsigned UAVTableEntrySize;
	static const unsigned UAVTableSize;

	/// Vertex buffer table
	static const unsigned MaxNumVertexBuffers = 16;
	static const unsigned VertexBufferTableEntrySize;
	static const unsigned VertexBufferTableSize;

	/// Constant buffer table
	static const unsigned MaxNumConstBufs = 16;
	static const unsigned ConstBufTableEntrySize;
	static const unsigned ConstBufTableSize;

	/// Resource table
	static const unsigned MaxNumResources = 16;
	static const unsigned ResourceTableEntrySize;
	static const unsigned ResourceTableSize;

	static const unsigned TotalTableSize;

	/// Constant buffers
	static const unsigned ConstBuf0Size;  // Defined in Metadata.pdf
	static const unsigned ConstBuf1Size; // FIXME

	static const unsigned TotalConstBufSize;

	/// Exception for Southern Islands emulator
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Southern Islands emulator");
		}
	};

	/// Debugger for ISA traces
	static misc::Debug debug;

	/// Return the maximum number of emulation cycles as set by the 
	/// --si-max-cycles command-line option
	long long getMaxCycles() { return max_cycles; }
	
	/// Return the maximum number of instructions as set by the 
	/// --si-max-inst command-line option
	long long getMaxInst() { return max_inst; }

	/// Return the maximum number of kernels as set by the 
	/// --si-max-kernels command-line option
	int getMaxKernels() { return max_kernels; }

	const std::string &getOpenGLBinaryName() { return opengl_binary_name; }
	int getWavefrontSize() { return wavefront_size; }

	/// Get the only instance of the Southern Islands emulator. If the
	/// instance does not exist yet, it will be created, and will remain
	/// allocated until the end of the execution.
	static Emu *getInstance();

	/// Dump emulator state
	void Dump(std::ostream &os = std::cout) const;

	/// Dump emulator state (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os, const Emu &emu)
	{
		emu.Dump(os);
		return os;
	}

	///
	/// Getters
	///

	/// Get a new NDRange ID
	unsigned getNewNDRangeID() { return ndrange_count++; }

	/// Get a new address space index
	unsigned getNewAddressSpaceIdx() { return address_space_index++; }

	/// Get global memory
	mem::Memory *getGlobalMemory() { return global_memory; }

	/// Get video_memory_top
	unsigned getVideoMemoryTop() const { return video_memory_top; }

	/// Get video_memory
	mem::Memory *getVideoMemory() { return video_memory.get(); }

	///
	/// Setters
	///

	/// Set global_memory
	void setGlobalMemory(mem::Memory *memory) { global_memory = memory; }
	
	/// Set work_group_count
	void setWorkGroupCount(long long count) { work_group_count = count; }

	/// Set OpenCL driver
	void setOpenCLSIDriver(::Driver::OpenCLSIDriver *opencl_driver)
			{ this->opencl_driver = opencl_driver; }

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

	/// Run one iteration of the emulation loop
	void Run();

	/// Increase video memory top
	void incVideoMemoryTop(unsigned inc) { video_memory_top += inc; }
};




}  // namespace SI

#endif

