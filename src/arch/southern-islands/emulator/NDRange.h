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

#include <deque>
#include <list>
#include <memory>

#include <arch/common/Context.h>
#include <arch/southern-islands/disassembler/Binary.h>
#include <memory/Memory.h>
#include <memory/Mmu.h>

#include "WorkGroup.h"
#include "WorkItem.h"


// Forward declarations
namespace SI
{

// Forward declarations
class Emulator;
class Arg;
class WorkGroup;
class Kernel;
struct BinaryUserElement;


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


	
	
	
	//
	// Static fields
	//

	/// UAV Table
	static const int MaxNumUAVs = 16;
	static const int UAVTableEntrySize = 32;
	static const int UAVTableSize = MaxNumUAVs * UAVTableEntrySize;

	/// Vertex buffer table
	static const int MaxNumVertexBuffers = 16;
	static const int VertexBufferTableEntrySize = 32;
	static const int VertexBufferTableSize = MaxNumVertexBuffers * VertexBufferTableEntrySize;

	/// Constant buffer table
	static const int MaxNumConstBufs = 16;
	static const int ConstBufTableEntrySize = 16;
	static const int ConstBufTableSize = MaxNumConstBufs * ConstBufTableEntrySize;

	/// Resource table
	static const int MaxNumResources = 16;
	static const int ResourceTableEntrySize = 32;
	static const int ResourceTableSize = MaxNumResources * ResourceTableEntrySize;

	static const int TotalTableSize = UAVTableSize
			+ ConstBufTableSize
			+ ResourceTableSize
			+ VertexBufferTableSize;
	
	/// Constant buffers
	static const int ConstBuf0Size = 160;
	static const int ConstBuf1Size = 1024;

	static const int TotalConstBufSize = ConstBuf0Size + ConstBuf1Size;

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
	Emulator *emulator = nullptr;

	// Unique ND-range ID
	int id = 0;

	// Stage that the ND-range operates on
	Stage stage = StageCompute;

	// Work-groups allocated for this ND-Range
	std::list<std::unique_ptr<WorkGroup>> work_groups;

	// Work-group list of pending work groups, IDs only
	std::deque<long> waiting_work_groups;

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
	std::unique_ptr<mem::Memory> instruction_memory;
	std::unique_ptr<char[]> instruction_buffer;
	unsigned instruction_address = 0;
	unsigned instruction_buffer_size = 0;

	// Local memory top to assign to local arguments.
	// Initially it is equal to the size of local variables in 
	// kernel function.
	int local_mem_top = 0;

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
	TableEntry const_buf_table_entries[MaxNumConstBufs];

	// Addresses and entries of tables that reside in global memory
	unsigned resource_table = 0;
	TableEntry resource_table_entries[MaxNumResources];

	// Addresses and entries of tables that reside in global memory
	unsigned uav_table = 0;
	TableEntry uav_table_entries[MaxNumUAVs];

	// Addresses and entries of tables that reside in global memory
	unsigned vertex_buffer_table = 0;
	TableEntry vertex_buffer_table_entries[MaxNumVertexBuffers];

	// Addresses of the constant buffers
	unsigned cb0 = 0;
	unsigned cb1 = 0;
	
	// Pointer to a context that is suspended while waiting for the ndrange
	// to complete
	comm::Context *suspended_context = nullptr;

public:

	//
	// Class members
	//

	/// Iterator indicating the position of this ND-range in the emulator's
	/// list of allocated ND-ranges. This field is managed internally by the
	/// emulator.
	std::list<std::unique_ptr<NDRange>>::iterator ndranges_iterator;

	// Associated memory address space
	mem::Mmu::Space *address_space = nullptr;

	/// Constructor
	NDRange();

	/// Dump the state of the ND-range in a plain-text format into an output
	/// stream.
	void Dump(std::ostream &os) const;

	/// Short-hand notation for dumping ND-range.
	friend std::ostream &operator<<(std::ostream &os,
			const NDRange &ndrange)
	{
		ndrange.Dump(os);
		return os;
	}
	
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

	/// Get num_vgpr_used
	unsigned getNumVgprUsed() const { return num_vgpr_used; }
	
	/// Get pointer to local_mem_top
	int *getLocalMemTopPtr() { return &local_mem_top; }

	/// Get the 1D local size 
	unsigned getLocalSize1D() const { return local_size; }

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

	/// Return the number of work-groups currently allocated in the
	/// ND-Range list of work-groups.
	int getNumWorkGroups() const { return work_groups.size(); }

	/// Return an iterator to the first work-group in the ND-Range list of
	/// work-groups.
	std::list<std::unique_ptr<WorkGroup>>::iterator getWorkGroupsBegin()
	{
		return work_groups.begin();
	}

	/// Return a past-the-end iterator to the ND-Range list of work-groups.
	std::list<std::unique_ptr<WorkGroup>>::iterator getWorkGroupsEnd()
	{
		return work_groups.end();
	}

	/// Create a new work-group with the given identifier and keep ownership
	/// of it in the list of work-groups.
	WorkGroup *addWorkGroup(int id);

	/// Remove a work-group from the list of work-groups and free it. All
	/// references to this work-group will be invalidates after this call.
	void RemoveWorkGroup(WorkGroup *work_group);

	/// Get stage of NDRange
	Stage getStage() const { return stage; }

	/// Return the number of waiting work-groups
	unsigned getNumWaitingWorkgroups() const { return waiting_work_groups.size(); }

	/// Return the number of running work-groups
	unsigned getNumWorkgroups() const { return work_groups.size(); }

	/// Get id of NDRange
	int getId() const { return id; }

	/// Get index of scalar register which stores workgroup id
	unsigned getWorkgroupIdSreg() const { return wg_id_sgpr; }

	/// Get user_element_count
	unsigned getUserElementCount() const { return user_element_count; }

	/// Get instruction memory
	mem::Memory *getInstructionMemory() { return instruction_memory.get(); }

	/// Get buffer containing instructions
	char *getInstructionBuffer() const { return instruction_buffer.get(); }

	/// Get instruction address in instruction memory
	unsigned getInstructionAddress() const { return instruction_address; }

	/// Get size of instruction buffer
	unsigned getInstructionBufferSize() const { 
			return instruction_buffer_size; }

	/// Get user element object
	BinaryUserElement *getUserElement(int idx)
	{
		assert(idx >= 0 && idx <= BinaryMaxUserElements);
		return &user_elements[idx];
	}

	/// Get emu it belongs to
	Emulator *getEmulator() const { return emulator; }

	/// Get constant buffer entry from constant buffer table at index
	TableEntry *getConstBuffer(unsigned idx)
	{
		assert(idx >= 0 && idx <= MaxNumConstBufs);
		return &const_buf_table_entries[idx];
	}

	/// Get constant buffer address in global memory
	unsigned getConstBufferAddr(unsigned idx) const
	{
		assert(idx >= 0 && idx <= MaxNumConstBufs);
		if (idx == 0)
			return cb0;
		else
			return cb1;
	}

	/// Get uav entry from uav table at index
	TableEntry *getUAV(unsigned idx)
	{
		assert(idx >= 0 && idx <= MaxNumUAVs);
		return &uav_table_entries[idx];
	}

	/// Get constant buffer table address in global memory
	unsigned getConstBufferTableAddr() const { return const_buf_table; }

	/// Get reource table address in global memory
	unsigned getResourceTableAddr() const { return resource_table; }

	/// Get UAV table address in global memory
	unsigned getUAVTableAddr() const { return uav_table; }

	/// Get Vertex buffer table address in global memory
	unsigned getVertexBufferTableAddr() const { return vertex_buffer_table; }
	
	/// Returns a pointer to a suspended context. If there is no suspended
	/// context, a nullptr is returned.
	comm::Context *GetSuspendedContext() { return suspended_context; }

	/// Remove and return a work group id from the waiting list 
	long GetWaitingWorkGroup() 
	{ 
		// Get the work_group id at the front of the deque and
		// remove it from the deque
		long work_group_id = waiting_work_groups.front();
		waiting_work_groups.pop_front(); 
		
		// Return
		return work_group_id;
	} 

	/// Get empty status of running_work_groups
	bool isWaitingWorkGroupsEmpty() const 
	{ 
		return waiting_work_groups.empty(); 
	}
	
	/// Get empty status of running_work_groups
	bool isRunningWorkGroupsEmpty() const { return work_groups.empty(); }

	// Return an iterator to the first workgroup in the running_work_group list.
	std::list<std::unique_ptr<WorkGroup>>::iterator WorkGroupBegin()
	{ 
		return work_groups.begin();
	}

	// Return an iterator to the past-to-end iterator in the running_work_group list.
	std::list<std::unique_ptr<WorkGroup>>::iterator WorkGroupEnd()
	{ 
		return work_groups.end();
	}

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
	void setLastWorkgroupSent(bool value) { last_work_group_sent = value; }

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
	
	/// Save a suspended context that is waiting for the ndrange.
	void SetSuspendedContext(comm::Context *context) 
	{ 
		suspended_context = context; 
	}
	
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
	/// \param buffer_desc
	///	Buffer descriptor
	///
	/// \param uav
	///	Index in UAV table
	///
	void InsertBufferIntoUAVTable(
			WorkItem::BufferDescriptor *buffer_descriptor,
			unsigned uav);

	/// Insert a buffer descriptor into vertex buffer table
	///
	/// \param buffer_desc Buffer descriptor
	/// \param vertex_buffer Index in vertex buffer table
	void InsertBufferIntoVertexBufferTable(
			WorkItem::BufferDescriptor *buffer_descriptor,
			unsigned vertex_buffer);

	/// Insert a buffer descriptor into constant buffer table
	///
	/// \param buffer_desc Buffer descriptor
	/// \param const_buffer_num Number of contant buffer, 0 or 1
	void InsertBufferIntoConstantBufferTable(
			WorkItem::BufferDescriptor *buffer_descriptor,
			unsigned const_buffer_num);

	/// Insert a image descriptor into UAV(universal access view) table
	///
	/// \param image_descriptor
	///	Image descriptor
	///
	/// \param uav
	///	Index in UAV table
	///
	void ImageIntoUAVTable(
			WorkItem::ImageDescriptor *image_descriptor,
			unsigned uav);

	/// Move a workgroup the in waiting list to the running list. This 
	/// function will move all the work groups in the waiting list to the 
	/// running list
	WorkGroup *ScheduleWorkGroup(unsigned id);
	
	/// Add ID of workgroups to waitinglist
	void AddWorkgroupIdToWaitingList(long work_group_id);

	/// Wake up the suspended context if the ndrange has completed.
	void WakeupContext();
};


}  // namespace

#endif

