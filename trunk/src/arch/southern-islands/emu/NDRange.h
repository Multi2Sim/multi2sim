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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_NDRANGE_H
#define ARCH_SOUTHERN_ISLANDS_EMU_NDRANGE_H

#include <list>
#include <memory>

#include <arch/southern-islands/asm/Arg.h>
#include <arch/southern-islands/asm/Binary.h>

#include "Emu.h"


namespace SI
{

class Emu;
class WorkGroup;

struct BufferDesc;
struct ImageDesc;


/// Stage is used to determine V/SGPRs initialization convention when creating
/// wavefronts/workitems.
enum NDRangeStage
{	
	NDRangeStageInvalid = 0,
	NDRangeStageCompute,
	NDRangeStageVertexShader,
	NDRangeStageGeometryShader,
	NDRangeStagePixelShader
};


/// ?
class NDRange
{
	enum TableEntryKind
	{
		TableEntryKindInvalid = 0,
		TableEntryKindBufferDesc,
		TableEntryKindImageDesc,
		TableEntryKindSamplerDesc
	};

	struct TableEntry
	{
		bool valid;
		TableEntryKind kind;
		unsigned size;
	};

	// Southern Islands emulator
	Emu *emu;

	// Unique ND-range ID
	int id;

	// Stage that the ND-range operates on
	NDRangeStage stage;

	// Initialization data for Pixel Shader, dequeue from SPI module
	//SxPsInit sc_ps_init;  // FIXME - confusing names

	// Work-group lists
	std::list<std::unique_ptr<WorkGroup>> waiting_work_groups;
	std::list<std::unique_ptr<WorkGroup>> running_work_groups;
	std::list<std::unique_ptr<WorkGroup>> completed_work_groups;

	// Used by the driver
	bool last_work_group_sent;

	// Number of work dimensions
	unsigned work_dim;

	// 3D work size counters
	unsigned global_size3[3];  // Total number of work_items
	unsigned local_size3[3];  // Number of work_items in a group
	unsigned group_count3[3];  // Number of work_item groups

	// 1D work size counters. Each counter is equal to the multiplication
	// of each component in the corresponding 3D counter.
	unsigned global_size;
	unsigned local_size;
	unsigned group_count;

	// ABI data copied from the kernel
	unsigned user_element_count;
	BinaryUserElement user_elements[BinaryMaxUserElements];

	// Instruction memory containing Southern Islands ISA
	void *inst_buffer;
	unsigned inst_buffer_size;

	// Fetch shader memory containing Fetch shader instructions
	int fs_buffer_initialized;
	unsigned fs_buffer_ptr; // Relative offset
	unsigned fs_buffer_size;

	// Local memory top to assign to local arguments.
	// Initially it is equal to the size of local variables in 
	// kernel function.
	unsigned local_mem_top;

	// Each ND-Range has its own address space
	int address_space_index;

	// If true, it indicates that a flush of the caches is being performed,
	// evicting data modified by this kernel
	bool flushing;

	// Number of register used by each work-item. This fields determines
	// how many work-groups can be allocated per compute unit, among
	// others.
	unsigned num_vgpr_used;
	unsigned num_sgpr_used;
	unsigned wg_id_sgpr;

	// Addresses and entries of tables that reside in global memory
	unsigned const_buf_table;
	TableEntry const_buf_table_entries[EmuMaxNumConstBufs];

	// ?
	unsigned resource_table;
	TableEntry resource_table_entries[EmuMaxNumResources];

	// ?
	unsigned uav_table;
	TableEntry uav_table_entries[EmuMaxNumUAVs];

	// ?
	unsigned vertex_buffer_table;
	TableEntry vertex_buffer_table_entries[EmuMaxNumVertexBuffers];

	// Addresses of the constant buffers
	unsigned cb0;
	unsigned cb1;

	// List of kernel arguments with their values
	std::vector<std::unique_ptr<Arg>> args;

public:

	/// Constructor
	NDRange(Emu *emu);

	/// Dump the state of the ND-range in a plain-text format into an output
	/// stream.
	void Dump(std::ostream &os) const;

	/// Short-hand notation for dumping ND-range.
	friend std::ostream &operator<<(std::ostream &os,
			const NDRange &ndrange) {
		ndrange.Dump(os);
		return os;
	}

	/// Getters
	///
	/// Get local memory top address
	unsigned getLocalMemTop() const { return local_mem_top; }

	/// Setters
	///
	/// Set new size parameters of the ND-Range before it gets launched.
	///
	/// \param global_size Array of \a work_dim elements (3 at most)
	///        representing the global size.
	/// \param local_size Array of \a work_dim elements (3 at most)
	///        representing the local size.
	/// \param work_dim Number of dimensions in the ND-range.
	void SetupSize(unsigned *global_size, unsigned *local_size,
			int work_dim);

	/// ?
	void SetupFSMem(const char *buf, unsigned size, unsigned pc);

	/// Set up content of instruction memory
	///
	/// \param buf Buffer containing Southern Islands ISA instructions. This
	///        buffer must be loaded from the .text section of an internal
	///        binary by the caller.
	/// \param size Number of bytes in the instruction buffer
	/// \param pc Initial value of the program counter, relative to the
	///        beginning of the instruction buffer.
	void SetupInstMem(const char *buf, unsigned size, unsigned pc);

	/// ?
	void ConstantBufferWrite(int const_buf_num, unsigned offset,
			void *pvalue, unsigned size);

	/// ?
	void ConstantBufferRead(int const_buf_num, unsigned offset,
			void *pvalue, unsigned size);

	/// ?
	void InsertBufferIntoUAVTable(BufferDesc *buffer_desc, unsigned uav);

	/// ?
	void InsertBufferIntoVertexBufferTable(BufferDesc *buffer_desc,
			unsigned vertex_buffer);

	/// ?
	void InsertBufferIntoConstantBufferTable(BufferDesc *buffer_desc,
			unsigned const_buffer_num);

	/// ?
	void ImageIntoUAVTable(ImageDesc *image_desc, unsigned uav);

};

}  // namespace

#endif

