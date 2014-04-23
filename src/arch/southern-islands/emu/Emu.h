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

// Forward declarations
namespace Driver
{
	class OpenCLSIDriver;
	class OpenGLSIDriver;
}

namespace mem
{
	class Memory;
}

namespace SI
{
	class Asm;
	class NDRange;
	class ShaderExport;
	class WorkGroup;
}

namespace SI
{
/// UAV Table
const unsigned EmuMaxNumUAVs = 16;
const unsigned EmuUAVTableEntrySize = 32;
const unsigned EmuUAVTableSize = EmuMaxNumUAVs * EmuUAVTableEntrySize;

/// Vertex buffer table
const unsigned EmuMaxNumVertexBuffers = 16;
const unsigned EmuVertexBufferTableEntrySize = 32;
const unsigned EmuVertexBufferTableSize = EmuMaxNumVertexBuffers *
		EmuVertexBufferTableEntrySize;

// Constant buffer table
const unsigned EmuMaxNumConstBufs = 16;
const unsigned EmuConstBufTableEntrySize = 16;
const unsigned EmuConstBufTableSize = EmuMaxNumConstBufs * EmuConstBufTableEntrySize;

// Resource table
const unsigned EmuMaxNumResources = 16;
const unsigned EmuResourceTableEntrySize = 32;
const unsigned EmuResourceTableSize = EmuMaxNumResources * EmuResourceTableEntrySize;

const unsigned EmuTotalTableSize = EmuUAVTableSize + EmuConstBufTableSize +
		EmuResourceTableSize + EmuVertexBufferTableSize;

// Constant buffers
const unsigned EmuConstBuf0Size = 160;  // Defined in Metadata.pdf
const unsigned EmuConstBuf1Size = 1024; // FIXME

const unsigned EmuTotalConstBufSize = EmuConstBuf0Size + EmuConstBuf1Size;

/// Buffer descriptor data format
enum EmuBufDescDataFmt
{
	EmuBufDescDataFmtInvalid = 0,
	EmuBufDescDataFmt8,
	EmuBufDescDataFmt16,
	EmuBufDescDataFmt8_8,
	EmuBufDescDataFmt32,
	EmuBufDescDataFmt16_16,
	EmuBufDescDataFmt10_11_11,
	EmuBufDescDataFmt10_10_10_2,
	EmuBufDescDataFmt2_10_10_10,
	EmuBufDescDataFmt8_8_8_8,
	EmuBufDescDataFmt32_32,
	EmuBufDescDataFmt16_16_16_16,
	EmuBufDescDataFmt32_32_32,
	EmuBufDescDataFmt32_32_32_32
};

/// Buffer descriptor number format
enum EmuBufDescNumFmt
{
	EmuBufDescNumFmtInvalid = -1,  // Not part of SI spec
	EmuBufDescNumFmtUnorm = 0,
	EmuBufDescNumFmtSnorm,
	EmuBufDescNumFmtUscaled,
	EmuBufDescNumFmtSscaled,
	EmuBufDescNumFmtUint,
	EmuBufDescNumFmtSint,
	EmuBufDescNumFmtSnormOgl,
	EmuBufDescNumFmtFloat
};

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


/// Configuration of the Southern Islands emulator
class EmuConfig
{
	long long max_cycles;
	long long max_inst;
	int max_kernels;

	std::string opengl_binary_name;

	int wavefront_size;
public:

	long long getMaxCycles() { return max_cycles; }
	long long getMaxInst() { return max_inst; }
	int getMaxKernels() { return max_kernels; }

	const std::string &getOpenGLBinaryName() { return opengl_binary_name; }
	int getWavefrontSize() { return wavefront_size; }
};


/// Southern Islands emulator.
class Emu
{
	// Associated disassembler
	std::unique_ptr<Asm> as;

	// Associated drivers
	Driver::OpenCLSIDriver *opencl_driver;

#ifdef HAVE_OPENGL
	Driver::OpenGLSIDriver *opengl_driver;
#endif

	// Memory spaces

	// Local to the GPU
	std::unique_ptr<mem::Memory> video_mem;
	unsigned video_mem_top;

	// Shared with the CPU
	std::unique_ptr<mem::Memory> shared_mem;
	
	// Will point to video_mem or shared_mem
	mem::Memory *global_mem;

	int address_space_index;

	// Current ND-Range
	NDRange *ndrange;

	// Work-group lists
	std::list<WorkGroup *> waiting_work_groups;
	std::list<WorkGroup *> running_work_groups;

	// Statistics
	int ndrange_count;               // Number of OpenCL kernels executed
	long long work_group_count;      // Number of OpenCL work groups executed
	long long scalar_alu_inst_count; // Scalar ALU instructions executed
	long long scalar_mem_inst_count; // Scalar mem instructions executed
	long long branch_inst_count;     // Branch instructions executed
	long long vector_alu_inst_count; // Vector ALU instructions executed
	long long lds_inst_count;        // LDS instructions executed
	long long vector_mem_inst_count; // Vector mem instructions executed
	long long export_inst_count;     // Export instructions executed

	// Unique instance of Southern Islands Emulator
	static std::unique_ptr<Emu> instance;

	// Private constructor. The only possible instance of the OpenCL Driver
	// can be obtained with a call to getInstance()
	Emu();

public:

	/// Debugger for ISA traces
	static misc::Debug debug;

	/// Emulator configuration;
	static EmuConfig config;

	/// Get the only instance of the Southern Islands emulator. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static Emu *getInstance();

	/// Dump emulator state
	void Dump(std::ostream &os) const;

	/// Dump emulator state (equivalent to Dump())
	friend std::ostream &operator<<(std::ostream &os, const Emu &emu) {
		emu.Dump(os);
		return os;
	}

	/// Getters
	///
	/// Get a new NDRange ID
	unsigned getNewNDRangeID() { return ndrange_count++; }

	/// Get a new address space index
	unsigned getNewAddressSpaceIdx() { return address_space_index++; }

	/// Get global memory
	mem::Memory *getGlobalMem() { return global_mem; }

	/// Get disassembler
	SI::Asm *getAsm() { return as.get(); }

	/// Get video_mem_top
	unsigned getVideoMemTop() const { return video_mem_top; }

	/// Get video_mem
	mem::Memory &getVideoMem() { return *(video_mem.get()); }

	/// Setters
	///
	/// Set global_mem
	void setGlobalMem(mem::Memory *memory) { global_mem = memory; }
	
	/// Set work_group_count
	void setWorkGroupCount(long long count) { work_group_count = count; }

	/// Set OpenCL driver
	void setDriverCL(Driver::OpenCLSIDriver *opencl_driver) {
		if (opencl_driver)
			this->opencl_driver = opencl_driver;
	}

#ifdef HAVE_OPENGL
	/// Set OpenGL driver
	void setDriverGL(Driver::OpenGLSIDriver *opengl_driver) {
		this->opengl_driver = opengl_driver;
	}
#endif

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
	void incVideoMemTop(unsigned inc) { video_mem_top += inc; }
};




}  // namespace SI

#endif

