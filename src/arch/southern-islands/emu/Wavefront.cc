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

#include <lib/cpp/Debug.h>
#include <lib/cpp/Misc.h>

#include "Emu.h"
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

	if (sreg == SI_VCCZ)
	{
		if (this->sreg[SI_VCC].as_uint == 0 && 
			this->sreg[SI_VCC+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	if (sreg == SI_EXECZ)
	{
		if (this->sreg[SI_EXEC].as_uint == 0 && 
			this->sreg[SI_EXEC+1].as_uint == 0)
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
	if (sreg == SI_VCC || sreg == SI_VCC + 1)
	{
		this->sreg[SI_VCCZ].as_uint = 
			!this->sreg[SI_VCC].as_uint &
			!this->sreg[SI_VCC + 1].as_uint;
	}
	if (sreg == SI_EXEC || sreg == SI_EXEC + 1)
	{
		this->sreg[SI_EXECZ].as_uint = 
			!this->sreg[SI_EXEC].as_uint &
			!this->sreg[SI_EXEC + 1].as_uint;
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

	// Create scalar work item
	this->scalar_work_item.reset(new WorkItem(this, 0));
	scalar_work_item->setWorkGroup(this->work_group);
}

void Wavefront::Execute()
{
	// Get current work-group
	WorkGroup *work_group = this->work_group;
	NDRange *ndrange = work_group->getNDRange();
	Emu *emu = ndrange->getEmu();
	WorkItem *work_item = NULL;
	std::unique_ptr<Inst> inst(new Inst(emu->getAsm()));

	// Reset instruction flags
	vector_mem_write = 0;
	vector_mem_read = 0;
	vector_mem_atomic = 0;
	scalar_mem_read = 0;
	lds_write = false;
	lds_read = false;
	mem_wait = false;
	at_barrier = false;
	barrier_inst = false;
	vector_mem_global_coherency = false;

	assert(!finished);
	
	// Grab the instruction at PC and update the pointer 
	unsigned inst_addr = ndrange->getInstAddr();
	unsigned inst_size = ndrange->getInstSize();
	char *inst_buffer = ndrange->getInstBuffer();
	ndrange->getInstMem()->Read(inst_addr, inst_size, inst_buffer);
	inst->Decode(inst_buffer, pc);

	this->inst_size = inst->getSize();
	InstOpcode opcode = inst->getOpcode();
	InstFormat format = inst->getFormat();
	InstBytes *bytes = inst->getBytes();
	int op = inst->getOp();

	// FIXME: Stats
	// asEmu(emu)->instructions++;
	// emu_inst_count++;
	// inst_count++;

	// Dump instruction string when debugging
	if (Emu::debug)
		inst->Dump(Emu::debug);

	// Execute the current instruction
	switch (format)
	{

	// Scalar ALU Instructions
	case InstFormatSOP1:
	{
		// Stats
		emu->incScalarAluInstCount();
		scalar_alu_inst_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, inst.get());

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatSOP2:
	{
		// Stats
		emu->incScalarAluInstCount();
		scalar_alu_inst_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, inst.get());

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatSOPP:
	{
		// Stats
		if (bytes->sopp.op > 1 &&
			bytes->sopp.op < 10)
		{
			emu->incBranchInstCount();
			branch_inst_count++;
		} else
		{
			emu->incScalarAluInstCount();
			scalar_alu_inst_count++;
		}

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, inst.get());

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatSOPC:
	{
		// Stats
		emu->incScalarAluInstCount();
		scalar_alu_inst_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, inst.get());

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatSOPK:
	{
		// Stats
		emu->incScalarAluInstCount();
		scalar_alu_inst_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, inst.get());

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	// Scalar Memory Instructions
	case InstFormatSMRD:
	{
		// Stats
		emu->incScalarMemInstCount();
		scalar_mem_inst_count++;

		// Only one work item executes the instruction
		work_item = scalar_work_item.get();
		work_item->Execute(opcode, inst.get());

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	// Vector ALU Instructions
	case InstFormatVOP2:
	{
		// Stats
		emu->incVectorAluInstCount();
		vector_alu_inst_count++;
	
		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatVOP1:
	{
		// Stats
		emu->incVectorAluInstCount();
		vector_alu_inst_count++;

		// Special case: V_READFIRSTLANE_B32
		if (bytes->vop1.op == 2)
		{
			/* Instruction ignores execution mask and is only 
			 * executed on one work item. Execute on the first 
			 * active work item from the least significant bit 
			 * in EXEC. (if exec is 0, execute work item 0) */
			work_item = (work_items_begin[0]).get();
			if (work_item->ReadSReg(SI_EXEC) == 0 && 
				work_item->ReadSReg(SI_EXEC + 1) == 0)
			{
				work_item->Execute(opcode, inst.get());
			}
			else 
			{
				for(auto i = work_items_begin; i != work_items_end; ++i)
				{
					work_item = (*i).get();
					if(getWorkItemActive(work_item->getIdInWavefront()))
					{
						work_item->Execute(opcode, inst.get());
					}
				}
			}
		}
		else
		{
			// Execute the instruction
			for(auto i = work_items_begin; i != work_items_end; ++i)
			{
				work_item = (*i).get();
				if(getWorkItemActive(work_item->getIdInWavefront()))
				{
					work_item->Execute(opcode, inst.get());
				}
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatVOPC:
	{
		// Stats
		emu->incVectorAluInstCount();
		vector_alu_inst_count++;
	
		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}
	
	case InstFormatVOP3a:
	{
		// Stats
		emu->incVectorAluInstCount();
		vector_alu_inst_count++;
	
		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatVOP3b:
	{
		// Stats
		emu->incVectorAluInstCount();
		vector_alu_inst_count++;
	
		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatVINTRP:
	{
		// Stats
		emu->incVectorAluInstCount();
		vector_alu_inst_count++;

		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}
 	
		break;
	}

	case InstFormatDS:
	{
		// Stats
		emu->incLdsInstCount();
		lds_inst_count++;

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
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	// Vector Memory Instructions
	case InstFormatMTBUF:
	{
		// Stats
		emu->incVectorMemInstCount();
		vector_mem_inst_count++;

		// Record access type
		if (op >= 0 && op < 4)
		{
			vector_mem_read = true;
		}
		else if (op >= 4 && op < 8)
		{
			vector_mem_write = true;
		}
		else
		{
			throw Emu::Error("Invalid MTBUF opcode");
		}
	
		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatMUBUF:
	{
		// Stats
		emu->incVectorMemInstCount();
		vector_mem_inst_count++;

		// Record access type
		if ((op >= 0 && op < 4) ||
			(op >= 8 && op < 15))
		{
			vector_mem_read = true;
		}
		else if ((op >= 4 && op < 8) ||
			(op >= 24 && op < 30))
		{
			vector_mem_write = true;
		}
		else if (op == 50)
		{
			vector_mem_atomic = true;
		}
		else 
		{
			throw misc::Panic(misc::fmt("Unsupported MUBUF "
					"opcode (%d)", op));
		}
	
		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

		break;
	}

	case InstFormatEXP:
	{
		// Stats
		emu->incExportInstCount();
		export_inst_count++;

		// Record access type
		// FIXME
			
		// Execute the instruction
		for(auto i = work_items_begin; i != work_items_end; ++i)
		{
			work_item = (*i).get();
			if(getWorkItemActive(work_item->getIdInWavefront()))
			{
				work_item->Execute(opcode, inst.get());
			}
		}

		if (Emu::debug)
		{
			Emu::debug << misc::fmt("\n");
		}

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
		pc += inst_size;
	}
	else
	{
		// Check if work-group finished kernel execution
		if (work_group->getWavefrontsCompletedEmu() == 
			work_group->getWavefrontsInWorkgroup())
		{
			work_group->setFinishedEmu(true);
		}
	}
}
	
bool Wavefront::getWorkItemActive(int id_in_wavefront)
{
	int mask = 1;
	if(id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (sreg[SI_EXEC].as_uint & mask) >> 
			id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (sreg[SI_EXEC + 1].as_uint & mask) >> 
			(id_in_wavefront - 32);
	}	
}

void Wavefront::setSReg(int sreg, unsigned value)
{
	this->sreg[sreg].as_uint = value;
}

void Wavefront::setSRegWithConstantBuffer(int first_reg, int num_regs, 
	int cb)
{
	EmuBufferDesc buf_desc;
	NDRange *ndrange = work_group->getNDRange();
	Emu *emu = ndrange->getEmu();

	unsigned buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buf_desc) == 16);
	assert(cb < (int)EmuMaxNumConstBufs);
	assert(ndrange->getConstBuffer(cb)->valid);

	buf_desc_addr = ndrange->getConstBufferTableAddr() +
		cb*EmuConstBufTableEntrySize;

	// Read a descriptor from the constant buffer table (located 
	// in global memory) 
	emu->getGlobalMem()->Read(buf_desc_addr, sizeof(buf_desc),
		(char *)&buf_desc);

	// Store the descriptor in 4 scalar registers 
	setSregUint(first_reg, ((unsigned *)&buf_desc)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&buf_desc)[1]);
	setSregUint(first_reg + 2, ((unsigned *)&buf_desc)[2]);
	setSregUint(first_reg + 3, ((unsigned *)&buf_desc)[3]);
}

void Wavefront::setSRegWithConstantBufferTable(int first_reg, int num_regs)
{
	NDRange *ndrange = work_group->getNDRange();
	EmuMemPtr mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->getConstBufferTableAddr();

	setSregUint(first_reg, ((unsigned *)&mem_ptr)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&mem_ptr)[1]);
}


void Wavefront::setSRegWithUAV(int first_reg, int num_regs, int uav)
{
	EmuBufferDesc buf_desc;
	NDRange *ndrange = work_group->getNDRange();
	Emu *emu = ndrange->getEmu();

	unsigned buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buf_desc) == 16);
	assert(uav < (int)EmuMaxNumUAVs);
	assert(ndrange->getUAV(uav)->valid);

	buf_desc_addr = ndrange->getUAVTableAddr() +
		uav*EmuUAVTableEntrySize;

	// Read a descriptor from the constant buffer table (located 
	// in global memory) 
	emu->getGlobalMem()->Read(buf_desc_addr, sizeof(buf_desc),
		(char *)&buf_desc);

	// Store the descriptor in 4 scalar registers 
	setSregUint(first_reg, ((unsigned *)&buf_desc)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&buf_desc)[1]);
	setSregUint(first_reg + 2, ((unsigned *)&buf_desc)[2]);
	setSregUint(first_reg + 3, ((unsigned *)&buf_desc)[3]);
}


void Wavefront::setSRegWithUAVTable(int first_reg, int num_regs)
{
	NDRange *ndrange = work_group->getNDRange();
	EmuMemPtr mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->getUAVTableAddr();

	setSregUint(first_reg, ((unsigned *)&mem_ptr)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&mem_ptr)[1]);
}

void Wavefront::setSRegWithVertexBufferTable(int first_reg, int num_regs)
{
	NDRange *ndrange = work_group->getNDRange();
	EmuMemPtr mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->getVertexBufferTableAddr();

	setSregUint(first_reg, ((unsigned *)&mem_ptr)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&mem_ptr)[1]);
}

void Wavefront::setSRegWithFetchShader(int first_reg, int num_regs)
{
	NDRange *ndrange = work_group->getNDRange();
	EmuMemPtr mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->getFetchShaderAddr();

	setSregUint(first_reg, ((unsigned *)&mem_ptr)[0]);
	setSregUint(first_reg + 1, ((unsigned *)&mem_ptr)[1]);	
}


}  // namespace SI 
