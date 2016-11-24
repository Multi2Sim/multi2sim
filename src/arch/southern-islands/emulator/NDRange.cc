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

#include <arch/southern-islands/disassembler/Argument.h>
#include <arch/southern-islands/driver/Driver.h>
#include <arch/southern-islands/driver/Kernel.h>
#include <src/lib/cpp/Misc.h>

#include "Emulator.h"
#include "NDRange.h"
#include "WorkGroup.h"
#include "Wavefront.h"
#include "WorkItem.h"


namespace SI
{

const int NDRange::MaxNumUAVs;
const int NDRange::UAVTableEntrySize;
const int NDRange::UAVTableSize;

const int NDRange::MaxNumVertexBuffers;
const int NDRange::VertexBufferTableEntrySize;
const int NDRange::VertexBufferTableSize;

const int NDRange::MaxNumConstBufs;
const int NDRange::ConstBufTableEntrySize;
const int NDRange::ConstBufTableSize;

const int NDRange::MaxNumResources;
const int NDRange::ResourceTableEntrySize;
const int NDRange::ResourceTableSize;

const int NDRange::TotalTableSize;

const int NDRange::ConstBuf0Size;
const int NDRange::ConstBuf1Size;
	
const int NDRange::TotalConstBufSize;



NDRange::NDRange()
{
	// For efficiency, store emulator instance
	emulator = Emulator::getInstance();

	// Initialize iterator in the emulator's ND-range list to an invalid
	// iterator. The emulator will take care of this field later.
	ndranges_iterator = emulator->getNDRangesEnd();

	// Assign ID
	id = emulator->getNewNDRangeID();

	// Initialize instruction memor - FIXME to be removed if allocated
	// statically.
	instruction_memory = misc::new_unique<mem::Memory>();
	instruction_memory->setSafe(true);
}


void NDRange::SetupSize(unsigned *global_size, unsigned *local_size,
	int work_dim)
{
	// Default value
	global_size3[1] = 1;
	global_size3[2] = 1;
	local_size3[1] = 1;
	local_size3[2] = 1;
	this->work_dim = work_dim;

	// Global work sizes
	for (int i = 0; i < work_dim; i++)
		global_size3[i] = global_size[i];
	
	this->global_size = global_size3[0] *
		global_size3[1] * global_size3[2];

	// Local work sizes
	for (int i = 0; i < work_dim; i++)
	{
		local_size3[i] = local_size[i];
		if (local_size3[i] < 1)
			throw Emulator::Error("Local work size must be greater "
					"than 0");
	}
	this->local_size = local_size3[0] * 
		local_size3[1] * local_size3[2];

	// Check valid global/local sizes
	if (global_size3[0] < 1 || global_size3[1] < 1
			|| global_size3[2] < 1)
		throw Emulator::Error("Invalid global size");
	if (local_size3[0] < 1 || local_size3[1] < 1
			|| local_size3[2] < 1)
		throw Emulator::Error("Invalid local size");

	// Check divisibility of global by local sizes
	if ((global_size3[0] % local_size3[0])
			|| (global_size3[1] % local_size3[1])
			|| (global_size3[2] % local_size3[2]))
		throw Emulator::Error("The global work size must be a multiple "
				"of the local size");

	// Calculate number of groups
	for (int i = 0; i < 3; i++)
	{
		group_count3[i] = global_size3[i] / 
			local_size3[i];
	}
	group_count = group_count3[0] * 
		group_count3[1] * group_count3[2];

}


void NDRange::SetupInstructionMemory(const char *buf, unsigned size, unsigned pc)
{

	// Copy instructions from buffer to instruction memory
	instruction_memory->Map(pc, size,
		mem::Memory::AccessRead | mem::Memory::AccessWrite);
	instruction_memory->Write(pc, size, buf);
	instruction_buffer_size = size;
	instruction_address = pc;

	// Save a copy of buffer in NDRange
	instruction_buffer = misc::new_unique_array<char>(size);
	instruction_memory->Read(pc, size, instruction_buffer.get());
}


void NDRange::InitializeFromKernel(Kernel *kernel)
{
	// Get SI encoding dictionary
	BinaryDictEntry *si_enc = kernel->getKernelBinaryFile()->GetSIDictEntry();

	// Initialize registers and local memory requirements 
	local_mem_top = kernel->getLocalMemorySize();
	num_sgpr_used = si_enc->num_sgpr;
	num_vgpr_used = si_enc->num_vgpr;
	wg_id_sgpr = si_enc->compute_pgm_rsrc2->user_sgpr;

	// Copy user elements from kernel to ND-Range 
	user_element_count = si_enc->num_user_elements;
	for (unsigned i = 0; i < user_element_count; ++i)
		user_elements[i] = si_enc->user_elements[i];
	
	// Set up instruction memory 
	// Initialize wavefront instruction buffer and PC 
	const char *text_buffer = si_enc->text_section->getBuffer();
	unsigned text_size = si_enc->text_section->getSize();
	if (!text_size)
		throw Emulator::Error("Cannot load kernel code");

	// Set up instruction memory
	SetupInstructionMemory(text_buffer, text_size, 0);
}


void NDRange::ConstantBufferWrite(int const_buffer_num,
		unsigned offset,
		void *pvalue,
		unsigned size)
{
	// Declare buffer description
	WorkItem::WorkItem::BufferDescriptor buffer_descriptor;

	// Sanity check 
	assert(const_buffer_num < 2);
	
	// Check constant buffer sizes
	assert(const_buffer_num != 1 || offset + size < NDRange::ConstBuf1Size);
	assert(const_buffer_num != 0 || offset + size < NDRange::ConstBuf0Size);

	// Calculate address
	unsigned addr = this->const_buf_table +
		const_buffer_num * ConstBufTableEntrySize;

	// Read in buffer description
	emulator->getGlobalMemory()->Read(addr, (unsigned) sizeof(WorkItem::BufferDescriptor), 
		(char *) &buffer_descriptor);

	// Calculate new address
	addr = buffer_descriptor.base_addr;
	addr += offset;

	// Write 
	emulator->getGlobalMemory()->Write(addr, size, (const char *) pvalue);
}


void NDRange::ConstantBufferRead(
		int const_buffer_num,
		unsigned offset,
		void *pvalue,
		unsigned size)
{
	// Declare buffer description
	WorkItem::BufferDescriptor buffer_descriptor;

	// Sanity check 
	assert(const_buffer_num < 2);
	
	// Check constant buffer sizes
	assert(const_buffer_num != 0 || offset + size < ConstBuf1Size);
	assert(const_buffer_num != 1 || offset + size < ConstBuf0Size);

	// Calculate address
	unsigned addr = this->const_buf_table +
		const_buffer_num * ConstBufTableEntrySize;

	// Read in buffer description
	emulator->getGlobalMemory()->Read(addr, sizeof(WorkItem::BufferDescriptor), 
		(char *)&buffer_descriptor);

	// Calculate new address
	addr = buffer_descriptor.base_addr;
	addr += offset;

	// Read 
	emulator->getGlobalMemory()->Read(addr, size, (char *) pvalue);
}


void NDRange::InsertBufferIntoUAVTable(
		WorkItem::BufferDescriptor *buffer_descriptor,
		unsigned uav)
{
	assert(uav < MaxNumUAVs);
	assert(sizeof(*buffer_descriptor) <= UAVTableEntrySize);

	// Write the buffer resource descriptor into the UAV table
	unsigned addr = uav_table + uav * UAVTableEntrySize;

	emulator->getGlobalMemory()->Write(addr, (unsigned)sizeof(*buffer_descriptor),
		(char *)buffer_descriptor);

	uav_table_entries[uav].valid = 1;
	uav_table_entries[uav].kind = 
		TableEntryKindEmuBufferDesc;
	uav_table_entries[uav].size = (unsigned)sizeof(*buffer_descriptor);
}


void NDRange::InsertBufferIntoVertexBufferTable(
		WorkItem::BufferDescriptor *buffer_descriptor, 
		unsigned vertex_buffer)
{
	assert(vertex_buffer < MaxNumVertexBuffers);
	assert(sizeof(*buffer_descriptor) <= VertexBufferTableEntrySize);

	// Write the buffer resource descriptor into the Vertex Buffer table
	unsigned addr = vertex_buffer_table + vertex_buffer * VertexBufferTableEntrySize;

	emulator->getGlobalMemory()->Write(addr, (unsigned)sizeof(*buffer_descriptor),
		(char *)buffer_descriptor);

	vertex_buffer_table_entries[vertex_buffer].valid = 1;
	vertex_buffer_table_entries[vertex_buffer].kind = 
		TableEntryKindEmuBufferDesc;
	vertex_buffer_table_entries[vertex_buffer].size = (unsigned)sizeof(*buffer_descriptor);
}


void NDRange::InsertBufferIntoConstantBufferTable(
		WorkItem::BufferDescriptor *buffer_descriptor,
		unsigned const_buffer_num)
{
	assert(const_buffer_num < MaxNumConstBufs);
	assert(sizeof(*buffer_descriptor) <= ConstBufTableEntrySize);

	// Write the buffer resource descriptor into the constant buffer table
	unsigned addr = const_buf_table + const_buffer_num * ConstBufTableEntrySize;

	emulator->getGlobalMemory()->Write(addr, (unsigned)sizeof(*buffer_descriptor), 
		(char *)buffer_descriptor);

	const_buf_table_entries[const_buffer_num].valid = 1;
	const_buf_table_entries[const_buffer_num].kind = 
		TableEntryKindEmuBufferDesc;
	uav_table_entries[const_buffer_num].size = sizeof(*buffer_descriptor);	
}


void NDRange::ImageIntoUAVTable(
		WorkItem::ImageDescriptor *image_descriptor,
		unsigned uav)
{
	assert(uav < MaxNumUAVs);
	assert(sizeof(*image_descriptor) <= UAVTableEntrySize);

	// Write the buffer resource descriptor into the UAV table
	unsigned addr = uav_table + uav * UAVTableEntrySize;

	emulator->getGlobalMemory()->Write(addr, (unsigned)sizeof(*image_descriptor), 
		(char *)image_descriptor);

	uav_table_entries[uav].valid = 1;
	uav_table_entries[uav].kind = TableEntryKindImageDesc;
	uav_table_entries[uav].size = sizeof(*image_descriptor);

}


void NDRange::AddWorkgroupIdToWaitingList(long work_group_id)
{
	// Add work-group to waiting list
	waiting_work_groups.push_back(work_group_id);
}
			

WorkGroup *NDRange::ScheduleWorkGroup(unsigned id)
{
	// Create work-group
	auto it = work_groups.emplace(work_groups.end(),
			misc::new_unique<WorkGroup>(this, id));
	
	// Save iterator
	WorkGroup *work_group = work_groups.back().get();
	work_group->work_groups_iterator = it;

	// Debug info
	Emulator::scheduler_debug <<
			misc::fmt("[NDRange %d] "
			"work group %d scheduled\n",
			this->id, work_group->getId());
 
	// Return new work-group
	return work_group;
}


void NDRange::RemoveWorkGroup(WorkGroup *work_group)
{
	// Debug info
	Emulator::scheduler_debug << 
			misc::fmt("[NDRange %d] "
			"work group %d removed\n",
			id, work_group->getId());

	// Erase work group
	assert(work_group->work_groups_iterator != work_groups.end());
	work_groups.erase(work_group->work_groups_iterator);
}

void NDRange::WakeupContext()
{
	// Check if there is a suspended context and if the ndrange has
	// completed all of its work groups.
	if (suspended_context && isWaitingWorkGroupsEmpty() && 
			isRunningWorkGroupsEmpty())                        
	{       
		// There are no more work groups to execute, so a suspended
		// context can continue.
		suspended_context->Wakeup();

		// If the context has been woken up, there is no need to hold
		// on to the pointer
		suspended_context = nullptr;
	}                                                                            
}


}

