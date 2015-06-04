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

#include <arch/southern-islands/asm/Binary.h>
#include <driver/opengl/southern-islands/SPI.h>
#include <memory/Memory.h>

#include "Emu.h"


// Forward declarations
namespace SI
{
	class Emu;
	class Arg;
	class WorkGroup;
	class Kernel;

	struct BinaryUserElement;
	struct EmuBufferDesc;
	struct ImageDesc;	
}

namespace SI
{



/// This class represents an OpenCL NDRange. The NDRange is an index space
/// which can be one, two, or three dimensions.
class NDRange
{

public:
	
	/// Stage is used to determine V/SGPRs initialization convention when 
	/// creating wavefronts/workitems.
	enum Stage
	{	
		StageInvalid = 0,
		StageCompute,
		StageVertexShader,
		StageGeometryShader,
		StagePixelShader
	};

private:

	enum TableEntryKind
	{
		TableEntryKindInvalid = 0,
		TableEntryKindEmuBufferDesc,
		TableEntryKindImageDesc,
		TableEntryKindSamplerDesc
	};

	struct TableEntry
	{
		bool valid = false;
		TableEntryKind kind = TableEntryKindInvalid;
		unsigned size = 0;
	};

	// Southern Islands emulator
	Emu *emu = nullptr;

	// Unique ND-range ID
	int id = 0;

	// Stage that the ND-range operates on
	Stage stage = StageCompute;

	// Work-group lists, IDs only
	std::list<long> waiting_work_groups;
	std::list<long> running_work_groups;
	std::list<long> completed_work_groups;

	// Used by the driver
	bool last_work_group_sent = false;

	// Number of work dimensions
	unsigned work_dim = 0;

	// 3D work size counters
	unsigned global_size3[3];  // Total number of work_items
	unsigned local_size3[3];   // Number of work_items in a group
	unsigned group_count3[3];  // Number of work_item groups

	// 1D work size counters. Each counter is equal to the multiplication
	// of each component in the corresponding 3D counter.
	unsigned global_size = 0;
	unsigned local_size = 0;
	unsigned group_count = 0;

	// ABI data copied from the kernel
	unsigned user_element_count = 0;
	BinaryUserElement user_elements[BinaryMaxUserElements];

	// Instruction memory containing Southern Islands ISA
	std::unique_ptr<mem::Memory> inst_mem;
	std::unique_ptr<char[]> inst_buffer;
	unsigned inst_addr = 0;
	unsigned inst_size = 0;

	// Fetch shader memory containing Fetch shader instructions
	int fs_buffer_initialized = 0;
	unsigned fs_buffer_ptr = 0; // Relative offset
	unsigned fs_buffer_size = 0;

	// Local memory top to assign to local arguments.
	// Initially it is equal to the size of local variables in 
	// kernel function.
	int local_mem_top = 0;

	// Each ND-Range has its own address space
	int address_space_index = 0;

	// If true, it indicates that a flush of the caches is being performed,
	// evicting data modified by this kernel
	bool flushing = false;

	// Number of register used by each work-item. This fields determines
	// how many work-groups can be allocated per compute unit, among
	// others.
	unsigned num_vgpr_used = 0;
	unsigned num_sgpr_used = 0;
	unsigned wg_id_sgpr = 0;

	// Addresses and entries of tables that reside in global memory
	unsigned const_buf_table = 0;
	TableEntry const_buf_table_entries[Emu::MaxNumConstBufs];

	// Addresses and entries of tables that reside in global memory
	unsigned resource_table = 0;
	TableEntry resource_table_entries[Emu::MaxNumResources];

	// Addresses and entries of tables that reside in global memory
	unsigned uav_table = 0;
	TableEntry uav_table_entries[Emu::MaxNumUAVs];

	// Addresses and entries of tables that reside in global memory
	unsigned vertex_buffer_table = 0;
	TableEntry vertex_buffer_table_entries[Emu::MaxNumVertexBuffers];

	// Addresses of the constant buffers
	unsigned cb0 = 0;
	unsigned cb1 = 0;

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

	///
	/// Getters
	///

	/// Get work dim
	unsigned getWorkDim() const { return work_dim; }

	/// Get work dim pointer
	unsigned *getWorkDimPtr() { return &work_dim; }

	/// Get size of global size
	unsigned getGlobalSize(unsigned dim) const
	{ 
		assert(dim >= 0 && dim <= 2);
		return global_size3[dim];
	}

	/// Get global memory size pointer
	unsigned *getGlobalSizePtr(unsigned dim)
	{
		assert(dim >= 0 && dim <= 2);
		return &global_size3[dim];
	}
	
	/// Get local memory top address
	unsigned getLocalMemTop() const { return local_mem_top; }

	/// Get pointer to local_mem_top
	int *getLocalMemTopPtr() { return &local_mem_top; }

	/// Get size of local size
	unsigned getLocalSize(unsigned dim) const
	{ 
		assert(dim >= 0 && dim <= 2);
		return local_size3[dim];
	}

	/// Get global memory size pointer
	unsigned *getLocalSizePtr(unsigned dim)
	{
		assert(dim >= 0 && dim <= 2);
		return &local_size3[dim];
	}

	/// Get size of group count
	unsigned getGroupCount(unsigned dim) const
	{
		assert(dim >= 0 && dim <= 2);
		return group_count3[dim];
	}

	/// Get group count pointer
	unsigned *getGroupCountPtr(unsigned dim)
	{
		assert(dim >= 0 && dim <= 2);
		return &group_count3[dim];
	}

	/// Get stage of NDRange
	Stage getStage()	const { return stage; }

	/// Get count of waiting_work_groups
	unsigned getWaitingWorkgroupsCount() const { return waiting_work_groups.size(); }

	/// Get count of running_work_groups
	unsigned getRunningWorkgroupsCount() const { return running_work_groups.size(); }

	/// Get count of completed_work_groups
	unsigned getCompletedWorkgroupsCount() const { return completed_work_groups.size(); }

	/// Get id of NDRange
	int getId() const { return id; }

	/// Get index of scalar register which stores workgroup id
	unsigned getWorkgroupIdSreg() const { return wg_id_sgpr; }

	/// Get user_element_count
	unsigned getUserElementCount() const { return user_element_count; }

	/// Get instruction memory
	mem::Memory *getInstMem() { return inst_mem.get(); }

	/// Get buffer containing instructions
	char *getInstBuffer() const { return inst_buffer.get(); }

	/// Get instruction address in instruction memory
	unsigned getInstAddr() const { return inst_addr; }

	/// Get size of instructions
	unsigned getInstSize() const { return inst_size; }

	/// Get user element object
	BinaryUserElement *getUserElement(int idx)
	{
		assert(idx >= 0 && idx <= BinaryMaxUserElements);
		return &user_elements[idx];
	}

	/// Get address_space_index
	int getAddressSpaceIndex() const { return address_space_index; }

	/// Get emu it belongs to
	Emu *getEmu() const { return emu; }

	/// Get constant buffer entry from constant buffer table at index
	TableEntry *getConstBuffer(unsigned idx)
	{
		assert(idx >= 0 && idx <= Emu::MaxNumConstBufs);
		return &const_buf_table_entries[idx];
	}

	/// Get constant buffer address in global memory
	unsigned getConstBufferAddr(unsigned idx) const
	{
		assert(idx >= 0 && idx <= Emu::MaxNumConstBufs);
		if (idx == 0)
			return cb0;
		else
			return cb1;
	}

	/// Get uav entry from uav table at index
	TableEntry *getUAV(unsigned idx)
	{
		assert(idx >= 0 && idx <= Emu::MaxNumUAVs);
		return &uav_table_entries[idx];
	}

	// Get constant buffer table address in global memory
	unsigned getConstBufferTableAddr() const { return const_buf_table; }

	// Get reource table address in global memory
	unsigned getResourceTableAddr() const { return resource_table; }

	// Get UAV table address in global memory
	unsigned getUAVTableAddr() const { return uav_table; }

	// Get Vertex buffer table address in global memory
	unsigned getVertexBufferTableAddr() const { return vertex_buffer_table; }

	/// Get count of running_work_groups
	bool isRunningWorkGroupsEmpty() const { return running_work_groups.empty(); }

	// Return an iterator to the first workgroup in the running_work_group list.
	std::list<long>::iterator RunningWorkGroupBegin()
	{ 
		return running_work_groups.begin();
	}

	// Return an iterator to the past-to-end iterator in the running_work_group list.
	std::list<long>::iterator RunningWorkGroupEnd()
	{ 
		return running_work_groups.end();
	}

	///
	///Setters
	///

	/// Set address_space_index
	void setAddressSpaceIndex(int value) { address_space_index = value; }

	/// Set local_mem_top
	void setLocalMemTop(int value) { local_mem_top = value; }

	/// Increment local_mem_top
	void incLocalMemTop(int value) { local_mem_top += value; }

	/// Set num_vgpr_used
	void setNumVgprUsed(unsigned value) { num_vgpr_used = value; }

	/// Set num_sgpr_used
	void setNumSgprUsed(unsigned value) { num_sgpr_used = value; }

	/// Set wg_id_sgpr
	void setWgIdSgpr(unsigned value) { wg_id_sgpr = value; }

	/// Set last_work_group_sent
	void setLastWorkgroupSend(bool value) { last_work_group_sent = value; }

	/// Set const_buf_table
	void setConstBufferTable(unsigned value) { const_buf_table = value; }

	/// Set uav_table
	void setUAVTable(unsigned value) { uav_table = value; }

	/// Set resource_table
	void setResourceTable(unsigned value) { resource_table = value; }

	/// Set cb0
	void setCB0(unsigned value) { cb0 = value; }

	/// Set cb10
	void setCB1(unsigned value) { cb1 = value; }
	
	/// Set new size parameters of the ND-Range before it gets launched.
	///
	/// \param global_size Array of \a work_dim elements (3 at most)
	///        representing the global size.
	/// \param local_size Array of \a work_dim elements (3 at most)
	///        representing the local size.
	/// \param work_dim Number of dimensions in the ND-range.
	void SetupSize(unsigned *global_size, unsigned *local_size,
			int work_dim);

	/// Set up stage of NDRange
	void SetupStage(Stage stage) { this->stage = stage; }

	/// Set up content of instruction memory
	///
	/// \param buf Buffer containing Southern Islands ISA instructions. This
	///        buffer must be loaded from the .text section of an internal
	///        binary by the caller.
	/// \param size Number of bytes in the instruction buffer
	/// \param pc Initial value of the program counter
	void SetupInstructionMemory(const char *buf, unsigned size, 
			unsigned pc);

	/// Initialize from kernel information
	///
	/// \param kernel Kernel containing Southern Islands encoding dictionary
	void InitializeFromKernel(Kernel *kernel);

	/// Write to constant buffer(as a part of global memory)
	///
	/// \param const_buf_num Number of constant buffer, 0 or 1
	/// \param offset Offset from beginning of a contant buffer
	/// \param pvalue Source of data
	/// \param size Size of data
	void ConstantBufferWrite(int const_buf_num, unsigned offset,
			void *pvalue, unsigned size);

	/// Read from constant buffer
	///
	/// \param const_buf_num Index of constant buffer, 0 or 1
	/// \param offset Offset from beginning of a constant buffer
	/// \param pvalue Destination of data
	/// \param size Size of data
	void ConstantBufferRead(int const_buf_num, unsigned offset,
			void *pvalue, unsigned size);

	/// Insert a buffer descriptor into UAV(universal access view) table
	///
	/// \param buffer_desc Buffer descriptor
	/// \param uav Index in UAV table
	void InsertBufferIntoUAVTable(EmuBufferDesc *buffer_desc, unsigned uav);

	/// Insert a buffer descriptor into vertex buffer table
	///
	/// \param buffer_desc Buffer descriptor
	/// \param vertex_buffer Index in vertex buffer table
	void InsertBufferIntoVertexBufferTable(EmuBufferDesc *buffer_desc,
			unsigned vertex_buffer);

	/// Insert a buffer descriptor into constant buffer table
	///
	/// \param buffer_desc Buffer descriptor
	/// \param const_buffer_num Number of contant buffer, 0 or 1
	void InsertBufferIntoConstantBufferTable(EmuBufferDesc *buffer_desc,
			unsigned const_buffer_num);

	/// Insert a image descriptor into UAV(universal access view) table
	///
	/// \param image_desc Image descriptor
	/// \param uav Index in UAV table
	void ImageIntoUAVTable(EmuImageDesc *image_desc, unsigned uav);

	/// Move workgroups in waiting list to running list
	void WaitingToRunning();	

	/// Get initialization data from SPI module
	void ReceiveInitData(std::unique_ptr<DataForPixelShader> data);

	/// Add ID of workgroups to waitinglist
	void AddWorkgroupIdToWaitingList(long work_group_id);
};

}  // namespace

#endif

