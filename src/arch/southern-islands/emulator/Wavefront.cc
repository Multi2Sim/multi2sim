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

#include <arch/southern-islands/disassembler/Disassembler.h>
#include <arch/southern-islands/disassembler/Instruction.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/Misc.h>

#include "Emulator.h"
#include "NDRange.h"
#include "Wavefront.h"
#include "WorkGroup.h"
#include "WorkItem.h"


namespace SI
{

unsigned Wavefront::getSregUint(int sreg) const
{
	unsigned value;

	assert(sreg >= 0);
	assert(sreg != 104);
	assert(sreg != 105);
	assert(sreg != 125);
	assert((sreg < 209) || (sreg > 239));
	assert((sreg < 248) || (sreg > 250));
	assert(sreg != 254);
	assert(sreg < 256);

	if (sreg == Instruction::RegisterVccz)
	{
		if (this->sreg[Instruction::RegisterVcc].as_uint == 0 && 
			this->sreg[Instruction::RegisterVcc+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	if (sreg == Instruction::RegisterExecz)
	{
		if (this->sreg[Instruction::RegisterExec].as_uint == 0 && 
			this->sreg[Instruction::RegisterExec+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	else
	{
		value = this->sreg[sreg].as_uint;
	}

	// Statistics
	work_group->incSregReadCount();

	return value;
}


void Wavefront::setSregUint(int sreg, unsigned int value)
{
	assert(sreg >= 0);
	assert(sreg != 104);
	assert(sreg != 105);
	assert(sreg != 125);
	assert((sreg < 209) || (sreg > 239));
	assert((sreg < 248) || (sreg > 250));
	assert(sreg != 254);
	assert(sreg < 256);

	this->sreg[sreg].as_uint = value;

	// Update VCCZ and EXECZ if necessary.
	if (sreg == Instruction::RegisterVcc || sreg == Instruction::RegisterVcc + 1)
	{
		this->sreg[Instruction::RegisterVccz].as_uint = 
			!this->sreg[Instruction::RegisterVcc].as_uint &
			!this->sreg[Instruction::RegisterVcc + 1].as_uint;
	}
	if (sreg == Instruction::RegisterExec || sreg == Instruction::RegisterExec + 1)
	{
		this->sreg[Instruction::RegisterExecz].as_uint = 
			!this->sreg[Instruction::RegisterExec].as_uint &
			!this->sreg[Instruction::RegisterExec + 1].as_uint;
	}

	// Statistics
	work_group->incSregWriteCount();

}


Wavefront::Wavefront(WorkGroup *work_group, int id)
{
	this->work_group = work_group;
	this->id = id;

	// Integer inline constants.
	for(int i = 128; i < 193; i++)
		sreg[i].as_int = i - 128;
	for(int i = 193; i < 209; i++)
		sreg[i].as_int = -(i - 192);

	// Inline floats.
	sreg[240].as_float = 0.5;
	sreg[241].as_float = -0.5;
	sreg[242].as_float = 1.0;
	sreg[243].as_float = -1.0;
	sreg[244].as_float = 2.0;
	sreg[245].as_float = -2.0;
	sreg[246].as_float = 4.0;
	sreg[247].as_float = -4.0;

	// FIXME:: Create work items at here ?
	// self->work_items = xcalloc(si_emu_wavefront_size, sizeof(void *));
	// for(auto i = work_items_begin; i != work_items_end; ++i)
	// {
	// 	self->work_items[work_item_id] = new(SIWorkItem, work_item_id, self);
	// 	self->work_items[work_item_id]->work_group = work_group;
	// 	self->work_items[work_item_id]->id_in_wavefront = work_item_id;
	// }

	// Get emulator instance
	Emulator *emulator = Emulator::getInstance();

	// Create scalar work item
	this->scalar_work_item.reset(new WorkItem(this, 0));
	scalar_work_item->setWorkGroup(this->work_group);
	scalar_work_item->setGlobalMemory(emulator->getGlobalMemory());
}


void Wavefront::Execute()
{
	// Get current work-group
	WorkGroup *work_group = this->work_group;
	NDRange *ndrange = work_group->getNDRange();
	Emulator *emulator = ndrange->getEmulator();
	WorkItem *work_item = NULL;
	this->instruction = misc::new_unique<Instruction>();

	// Reset instruction flags
	vector_memory_write = 0;
	vector_memory_read = 0;
	vector_memory_atomic = 0;
	scalar_memory_read = 0;
	lds_write = false;
	lds_read = false;
	memory_wait = false;
	at_barrier = false;
	barrier_instruction = false;
	vector_memory_global_coherency = false;

	// Make sure the program has not finished yet
	assert(!finished);
	
	// Grab the instruction at PC and update the pointer 

	// Make sure the program counter is not outside the instruction memory
	unsigned total_inst_buffer_size = ndrange->getInstructionBufferSize();
	assert(total_inst_buffer_size > pc);

	// Find out the remaining size of the instruction memory
	unsigned remaining_inst_buffer_size = total_inst_buffer_size - pc;

	// Create a buffer based on the remaining instruction memory
	auto inst_buffer = misc::new_unique_array<char>(
			remaining_inst_buffer_size);
	ndrange->getInstructionMemory()->Read(pc, remaining_inst_buffer_size, 
			inst_buffer.get());

	// Decode the next instruction in the instruction memory
	// Note: We need to pass the whole buffer for a single instruction
	// because we don't know the size of the next instruction yet
	instruction->Decode(inst_buffer.get(), pc);

	// Update the statistics
	emulator->incNumInstructions();

	// Extract the properties of the newest instruction
	this->inst_size = instruction->getSize();
	Instruction::Opcode opcode = instruction->getOpcode();
	Instruction::Format format = instruction->getFormat();
	Instruction::Bytes *bytes = instruction->getBytes();
	int op = instruction->getOp();

	// Create stringstream for debugging
	std::stringstream ss;

	// Dump instruction string when debugging
	switch (format)
	{

	// Scalar ALU Instructions
	case Instruction::FormatSOP1:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incScalarAluInstCount();
		scalar_alu_instruction_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, instruction.get());

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatSOP2:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}
		
		// Stats
		emulator->incScalarAluInstCount();
		scalar_alu_instruction_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, instruction.get());

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";
		
		break;
	}

	case Instruction::FormatSOPP:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		if (bytes->sopp.op > 1 &&
			bytes->sopp.op < 10)
		{
			emulator->incBranchInstCount();
			branch_instruction_count++;
		} else
		{
			emulator->incScalarAluInstCount();
			scalar_alu_instruction_count++;
		}

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, instruction.get());

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatSOPC:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incScalarAluInstCount();
		scalar_alu_instruction_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, instruction.get());

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatSOPK:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incScalarAluInstCount();
		scalar_alu_instruction_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, instruction.get());

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	// Scalar Memory Instructions
	case Instruction::FormatSMRD:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incScalarMemInstCount();
		scalar_memory_instruction_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, instruction.get());

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	// Vector ALU Instructions
	case Instruction::FormatVOP2:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorAluInstCount();
		vector_alu_instruction_count++;
	
		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end;
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
				work_item->Execute(opcode, instruction.get());
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatVOP1:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorAluInstCount();
		vector_alu_instruction_count++;

		// Special case: V_READFIRSTLANE_B32
		if (bytes->vop1.op == 2)
		{
			// Instruction ignores execution mask and is only 
			// executed on one work item. Execute on the first 
			// active work item from the least significant bit 
			// in EXEC. (if exec is 0, execute work item 0)
			work_item = (work_items_begin[0]).get();
			if (work_item->ReadSReg(Instruction::RegisterExec) == 0 && 
				work_item->ReadSReg(Instruction::RegisterExec + 1) == 0)
			{
				work_item->Execute(opcode, instruction.get());
			}
			else 
			{
				for (auto it = work_items_begin, e = work_items_end; 
						it != e; ++it)
				{
					work_item = (*it).get();
					if (isWorkItemActive(work_item->getIdInWavefront()))
					{
						work_item->Execute(opcode, instruction.get());
					}
				}
			}
		}
		else
		{
			// Execute the instruction
			for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
			{
				work_item = (*it).get();
				if (isWorkItemActive(work_item->getIdInWavefront()))
				{
					work_item->Execute(opcode, instruction.get());
				}
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatVOPC:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorAluInstCount();
		vector_alu_instruction_count++;
	
		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}
	
	case Instruction::FormatVOP3a:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorAluInstCount();
		vector_alu_instruction_count++;
	
		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatVOP3b:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorAluInstCount();
		vector_alu_instruction_count++;
	
		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatVINTRP:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorAluInstCount();
		vector_alu_instruction_count++;

		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";
 	
		break;
	}

	case Instruction::FormatDS:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incLdsInstCount();
		lds_instruction_count++;

		// Record access type
		if ((op >= 13 && op < 16) ||
			(op >= 30 && op < 32) ||
			(op >= 77 && op < 80))
		{
			lds_write = 1;
		}
		else if (
			(op >= 54 && op < 61) ||
			(op >= 118 && op < 120))
		{
			lds_read = 1;
		}
		else 
		{
			throw misc::Panic("Unimplemented LDS opcode");
		}

		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	// Vector Memory Instructions
	case Instruction::FormatMTBUF:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorMemInstCount();
		vector_memory_instruction_count++;

		// Record access type
		if (op >= 0 && op < 4)
		{
			vector_memory_read = true;
		}
		else if (op >= 4 && op < 8)
		{
			vector_memory_write = true;
		}
		else
		{
			throw Emulator::Error("Invalid MTBUF opcode");
		}
	
		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatMUBUF:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incVectorMemInstCount();
		vector_memory_instruction_count++;

		// Record access type
		if ((op >= 0 && op < 4) ||
			(op >= 8 && op < 15))
		{
			vector_memory_read = true;
		}
		else if ((op >= 4 && op < 8) ||
			(op >= 24 && op < 30))
		{
			vector_memory_write = true;
		}
		else if (op == 50)
		{
			vector_memory_atomic = true;
		}
		else 
		{
			throw misc::Panic(misc::fmt("Unsupported MUBUF "
					"opcode (%d)", op));
		}
	
		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;
	}

	case Instruction::FormatEXP:
	{
		// Dump instruction string when debugging
		if (Emulator::isa_debug)
		{
			instruction->Dump(ss);
			instruction->DumpAddress(ss);
			Emulator::isa_debug << ss.str();
		}

		// Stats
		emulator->incExportInstCount();
		export_instruction_count++;

		// Record access type
		// FIXME
			
		// Execute the instruction
		for (auto it = work_items_begin, e = work_items_end; 
				it != e; ++it)
		{
			work_item = (*it).get();
			if (isWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, instruction.get());
			}
		}

		// Add newlines between each instruction
		Emulator::isa_debug << "\n\n";

		break;

	}

	default:
		
		throw misc::Panic(misc::fmt("Unsupported instruction type (%d)",
				format));

	}

	// Check if wavefront finished kernel execution
	if (!finished)
	{
		// Increment the PC
		pc += this->inst_size;
	}
	else
	{
		// Check if work-group finished kernel execution
		if (work_group->getWavefrontsCompletedEmu() == 
			work_group->getWavefrontsInWorkgroup())
		{
			// Mark work group as finished
			work_group->setFinished(true);	
		}
	}
}


bool Wavefront::isWorkItemActive(int id_in_wavefront)
{
	int mask = 1;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (sreg[Instruction::RegisterExec].as_uint & mask) >> 
			id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (sreg[Instruction::RegisterExec + 1].as_uint & mask) >> 
			(id_in_wavefront - 32);
	}	
}


void Wavefront::setSReg(int sreg, unsigned value)
{
	assert(sreg > 0 && sreg < 104);
	this->sreg[sreg].as_uint = value;
}


void Wavefront::setSRegWithConstantBuffer(int first_reg, int num_regs, 
	int cb)
{
	WorkItem::BufferDescriptor buffer_descriptor;
	NDRange *ndrange = work_group->getNDRange();
	Emulator *emulator = ndrange->getEmulator();

	unsigned buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buffer_descriptor) == 16);
	assert(cb < (int) NDRange::MaxNumConstBufs);
	assert(ndrange->getConstBuffer(cb)->valid);

	buf_desc_addr = ndrange->getConstBufferTableAddr() +
		cb * NDRange::ConstBufTableEntrySize;

	// Read a descriptor from the constant buffer table (located 
	// in global memory) 
	emulator->getGlobalMemory()->Read(buf_desc_addr, sizeof(buffer_descriptor),
		(char *)&buffer_descriptor);

	// Store the descriptor in 4 scalar registers 
	setSregUint(first_reg, ((unsigned *)&buffer_descriptor)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&buffer_descriptor)[1]);
	setSregUint(first_reg + 2, ((unsigned *)&buffer_descriptor)[2]);
	setSregUint(first_reg + 3, ((unsigned *)&buffer_descriptor)[3]);
}


void Wavefront::setSRegWithConstantBufferTable(int first_reg, int num_regs)
{
	NDRange *ndrange = work_group->getNDRange();
	WorkItem::MemoryPointer memory_pointer;

	assert(num_regs == 2);
	assert(sizeof(memory_pointer) == 8);

	memory_pointer.addr = (unsigned int)ndrange->getConstBufferTableAddr();

	setSregUint(first_reg, ((unsigned *)&memory_pointer)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&memory_pointer)[1]);
}


void Wavefront::setSRegWithUAV(int first_reg, int num_regs, int uav)
{
	WorkItem::BufferDescriptor buffer_descriptor;
	NDRange *ndrange = work_group->getNDRange();
	Emulator *emulator = ndrange->getEmulator();

	unsigned buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buffer_descriptor) == 16);
	assert(uav < (int) NDRange::MaxNumUAVs);
	assert(ndrange->getUAV(uav)->valid);

	buf_desc_addr = ndrange->getUAVTableAddr() +
		uav * NDRange::UAVTableEntrySize;

	// Read a descriptor from the constant buffer table (located 
	// in global memory) 
	emulator->getGlobalMemory()->Read(buf_desc_addr, sizeof(buffer_descriptor),
		(char *)&buffer_descriptor);

	// Store the descriptor in 4 scalar registers 
	setSregUint(first_reg, ((unsigned *)&buffer_descriptor)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&buffer_descriptor)[1]);
	setSregUint(first_reg + 2, ((unsigned *)&buffer_descriptor)[2]);
	setSregUint(first_reg + 3, ((unsigned *)&buffer_descriptor)[3]);
}


void Wavefront::setSRegWithUAVTable(int first_reg, int num_regs)
{
	NDRange *ndrange = work_group->getNDRange();
	WorkItem::MemoryPointer memory_pointer;

	assert(num_regs == 2);
	assert(sizeof(memory_pointer) == 8);

	memory_pointer.addr = (unsigned int)ndrange->getUAVTableAddr();

	setSregUint(first_reg, ((unsigned *)&memory_pointer)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&memory_pointer)[1]);
}


}  // namespace SI 
