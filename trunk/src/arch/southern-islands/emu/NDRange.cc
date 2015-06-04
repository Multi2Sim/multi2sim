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

#include <arch/southern-islands/asm/Arg.h>
#include <arch/x86/emu/Emu.h>
#include <arch/southern-islands/driver/Kernel.h>
#include <src/lib/cpp/Misc.h>

#include "NDRange.h"
#include "WorkGroup.h"
#include "Wavefront.h"
#include "WorkItem.h"

using namespace misc;

namespace SI
{

NDRange::NDRange(Emu *emu) : emu(emu)
{
	id = emu->getNewNDRangeID();
	address_space_index = emu->getNewNDRangeID();
	instruction_memory = misc::new_unique<mem::Memory>();
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
			throw Emu::Error("Local work size must be greater "
					"than 0");
	}
	this->local_size = local_size3[0] * 
		local_size3[1] * local_size3[2];

	// Check valid global/local sizes
	if (global_size3[0] < 1 || global_size3[1] < 1
			|| global_size3[2] < 1)
		throw Emu::Error("Invalid global size");
	if (local_size3[0] < 1 || local_size3[1] < 1
			|| local_size3[2] < 1)
		throw Emu::Error("Invalid local size");

	// Check divisibility of global by local sizes
	if ((global_size3[0] % local_size3[0])
			|| (global_size3[1] % local_size3[1])
			|| (global_size3[2] % local_size3[2]))
		throw Emu::Error("The global work size must be a multiple "
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
	instruction_size = size;
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
		throw Emu::Error("Cannot load kernel code");

	// Set up instruction memory
	SetupInstructionMemory(text_buffer, text_size, 0);
}

void NDRange::ConstantBufferWrite(int const_buffer_num,
		unsigned offset,
		void *pvalue,
		unsigned size)
{
	// Declare buffer description
	EmuBufferDesc buffer_desc;

	// Sanity check 
	assert(const_buffer_num < 2);
	
	// Check constant buffer sizes
	assert(const_buffer_num != 0 || offset + size < Emu::ConstBuf0Size);
	assert(const_buffer_num != 1 || offset + size < Emu::ConstBuf1Size);

	// Calculate address
	unsigned addr = this->const_buf_table +
		const_buffer_num * Emu::ConstBufTableEntrySize;

	// Read in buffer description
	emu->getGlobalMemory()->Read(addr, (unsigned) sizeof(EmuBufferDesc), 
		(char *) &buffer_desc);

	// Calculate new address
	addr = buffer_desc.base_addr;
	addr += offset;

	// Write 
	emu->getGlobalMemory()->Write(addr, size, (const char *) pvalue);
}

void NDRange::ConstantBufferRead(int const_buffer_num,
		unsigned offset,
		void *pvalue,
		unsigned size)
{
	// Declare buffer description
	EmuBufferDesc buffer_desc;

	// Sanity check 
	assert(const_buffer_num < 2);
	
	// Check constant buffer sizes
	assert(const_buffer_num != 0 || offset + size < Emu::ConstBuf0Size);
	assert(const_buffer_num != 1 || offset + size < Emu::ConstBuf1Size);

	// Calculate address
	unsigned addr = this->const_buf_table +
		const_buffer_num*Emu::ConstBufTableEntrySize;

	// Read in buffer description
	emu->getGlobalMemory()->Read(addr, sizeof(EmuBufferDesc), 
		(char *)&buffer_desc);

	// Calculate new address
	addr = buffer_desc.base_addr;
	addr += offset;

	// Read 
	emu->getGlobalMemory()->Read(addr, size, (char *) pvalue);
}

void NDRange::InsertBufferIntoUAVTable(EmuBufferDesc *buffer_desc, unsigned uav)
{
	assert(uav < Emu::MaxNumUAVs);
	assert(sizeof(*buffer_desc) <= Emu::UAVTableEntrySize);

	// Write the buffer resource descriptor into the UAV table
	unsigned addr = uav_table + uav*Emu::UAVTableEntrySize;

	emu->getGlobalMemory()->Write(addr, (unsigned)sizeof(*buffer_desc),
		(char *)buffer_desc);

	uav_table_entries[uav].valid = 1;
	uav_table_entries[uav].kind = 
		TableEntryKindEmuBufferDesc;
	uav_table_entries[uav].size = (unsigned)sizeof(*buffer_desc);
}

void NDRange::InsertBufferIntoVertexBufferTable(EmuBufferDesc *buffer_desc, 
	unsigned vertex_buffer)
{
	assert(vertex_buffer < Emu::MaxNumVertexBuffers);
	assert(sizeof(*buffer_desc) <= Emu::VertexBufferTableEntrySize);

	// Write the buffer resource descriptor into the Vertex Buffer table
	unsigned addr = vertex_buffer_table + vertex_buffer*Emu::VertexBufferTableEntrySize;

	emu->getGlobalMemory()->Write(addr, (unsigned)sizeof(*buffer_desc),
		(char *)buffer_desc);

	vertex_buffer_table_entries[vertex_buffer].valid = 1;
	vertex_buffer_table_entries[vertex_buffer].kind = 
		TableEntryKindEmuBufferDesc;
	vertex_buffer_table_entries[vertex_buffer].size = (unsigned)sizeof(*buffer_desc);
}

void NDRange::InsertBufferIntoConstantBufferTable(EmuBufferDesc *buffer_desc,
	unsigned const_buffer_num)
{
	assert(const_buffer_num < Emu::MaxNumConstBufs);
	assert(sizeof(*buffer_desc) <= Emu::ConstBufTableEntrySize);

	// Write the buffer resource descriptor into the constant buffer table
	unsigned addr = const_buf_table + const_buffer_num*Emu::ConstBufTableEntrySize;

	emu->getGlobalMemory()->Write(addr, (unsigned)sizeof(*buffer_desc), 
		(char *)buffer_desc);

	const_buf_table_entries[const_buffer_num].valid = 1;
	const_buf_table_entries[const_buffer_num].kind = 
		TableEntryKindEmuBufferDesc;
	uav_table_entries[const_buffer_num].size = sizeof(*buffer_desc);	
}

void NDRange::ImageIntoUAVTable(EmuImageDesc *image_desc, unsigned uav)
{
	assert(uav < Emu::MaxNumUAVs);
	assert(sizeof(*image_desc) <= Emu::UAVTableEntrySize);

	// Write the buffer resource descriptor into the UAV table
	unsigned addr = uav_table + uav*Emu::UAVTableEntrySize;

	emu->getGlobalMemory()->Write(addr, (unsigned)sizeof(*image_desc), 
		(char *)image_desc);

	uav_table_entries[uav].valid = 1;
	uav_table_entries[uav].kind = TableEntryKindImageDesc;
	uav_table_entries[uav].size = sizeof(*image_desc);

}

void NDRange::WaitingToRunning()
{
	// Add waiting work-groups to the list of running work-groups
	for (auto i = waiting_work_groups.begin(), e = waiting_work_groups.end(); 
		i != e; ++i)
		running_work_groups.push_back(std::move(*i));
}

void NDRange::AddWorkgroupIdToWaitingList(long work_group_id)
{
	// Add work-group to waiting list
	waiting_work_groups.push_back(work_group_id);
}


}  // namespace SI
