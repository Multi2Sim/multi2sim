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


#include <lib/class/class.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <memory/memory.h>

#include "isa.h"
#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"



/*
 * Class 'SIWavefront'
 */

/* Helper function for initializing the wavefront. */
static void SIWavefrontInitSReg(SIWavefront *self)
{
	SIInstReg *sreg = &self->sreg[0];

	/* Integer inline constants. */
	for(int i = 128; i < 193; i++)
		sreg[i].as_int = i - 128;
	for(int i = 193; i < 209; i++)
		sreg[i].as_int = -(i - 192);

	/* Inline floats. */
	sreg[240].as_float = 0.5;
	sreg[241].as_float = -0.5;
	sreg[242].as_float = 1.0;
	sreg[243].as_float = -1.0;
	sreg[244].as_float = 2.0;
	sreg[245].as_float = -2.0;
	sreg[246].as_float = 4.0;
	sreg[247].as_float = -4.0;
}


void SIWavefrontCreate(SIWavefront *self, int id, SIWorkGroup *work_group)
{
	SINDRange *ndrange = work_group->ndrange;
	SIEmu *emu = ndrange->emu;
	struct SIAsmWrap *as = emu->as;

	int work_item_id;

	/* Initialize */
	self->inst = SIInstWrapCreate(as);
	self->work_group = work_group;
	self->id = id;
	SIWavefrontInitSReg(self);

	/* Create work items */
	self->work_items = xcalloc(si_emu_wavefront_size, sizeof(void *));
	SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
	{
		self->work_items[work_item_id] = new(SIWorkItem, work_item_id, self);
		self->work_items[work_item_id]->work_group = work_group;
		self->work_items[work_item_id]->id_in_wavefront = work_item_id;
	}

	/* Create scalar work item */
	self->scalar_work_item = new(SIWorkItem, 0, self);
	self->scalar_work_item->work_group = work_group;
}


void SIWavefrontDestroy(SIWavefront *self)
{
	free(self->work_items);
	SIInstWrapFree(self->inst);
}


/* Execute the next instruction for the wavefront */
void SIWavefrontExecute(SIWavefront *self)
{
	SINDRange *ndrange;
	SIWorkGroup *work_group;
	SIWorkItem *work_item;
	struct SIInstWrap *inst;
	SIEmu *emu;

	SIInstOpcode opcode;
	SIInstFormat format;
	SIInstBytes *bytes;

	int work_item_id;
	int op;

	/* Get current work-group */
	work_group = self->work_group;
	ndrange = work_group->ndrange;
	emu = ndrange->emu;
	work_item = NULL;
	inst = self->inst;

	/* Reset instruction flags */
	self->vector_mem_write = 0;
	self->vector_mem_read = 0;
	self->vector_mem_atomic = 0;
	self->scalar_mem_read = 0;
	self->lds_write = 0;
	self->lds_read = 0;
	self->mem_wait = 0;
	self->at_barrier = 0;
	self->barrier_inst = 0;
	self->vector_mem_glc = 0;

	assert(!self->finished);
	
	/* Grab the instruction at PC and update the pointer */
	SIInstWrapDecode(inst, ndrange->inst_buffer + self->pc, 0);
	self->inst_size = SIInstWrapGetSize(self->inst);
	opcode = SIInstWrapGetOpcode(self->inst);
	format = SIInstWrapGetFormat(self->inst);
	bytes = SIInstWrapGetBytes(self->inst);
	op = SIInstWrapGetOp(inst);

	/* Stats */
	asEmu(emu)->instructions++;
	self->emu_inst_count++;
	self->inst_count++;

	/* Dump instruction string when debugging */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("\n");
		assert(debug_file(si_isa_debug_category));
		////// FIXME Breaking the ISA dump by mixing C++ ostream with
		///// C file descriptor
		//SIInstWrapDump(inst, debug_file(si_isa_debug_category));
	}

	/* Execute the current instruction */
	switch (SIInstWrapGetFormat(inst))
	{

	/* Scalar ALU Instructions */
	case SIInstFormatSOP1:
	{
		/* Stats */
		emu->scalar_alu_inst_count++;
		self->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = self->scalar_work_item;
		(*si_isa_inst_func[opcode])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatSOP2:
	{
		/* Stats */
		emu->scalar_alu_inst_count++;
		self->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = self->scalar_work_item;
		(*si_isa_inst_func[opcode])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatSOPP:
	{
		/* Stats */
		if (bytes->sopp.op > 1 &&
			bytes->sopp.op < 10)
		{
			emu->branch_inst_count++;
			self->branch_inst_count++;
		} else
		{
			emu->scalar_alu_inst_count++;
			self->scalar_alu_inst_count++;
		}

		/* Only one work item executes the instruction */
		work_item = self->scalar_work_item;
		(*si_isa_inst_func[opcode])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatSOPC:
	{
		/* Stats */
		emu->scalar_alu_inst_count++;
		self->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = self->scalar_work_item;
		(*si_isa_inst_func[opcode])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatSOPK:
	{
		/* Stats */
		emu->scalar_alu_inst_count++;
		self->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = self->scalar_work_item;
		(*si_isa_inst_func[opcode])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	/* Scalar Memory Instructions */
	case SIInstFormatSMRD:
	{
		/* Stats */
		emu->scalar_mem_inst_count++;
		self->scalar_mem_inst_count++;

		/* Only one work item executes the instruction */
		work_item = self->scalar_work_item;
		(*si_isa_inst_func[opcode])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	/* Vector ALU Instructions */
	case SIInstFormatVOP2:
	{
		/* Stats */
		emu->vector_alu_inst_count++;
		self->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if(SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatVOP1:
	{
		/* Stats */
		emu->vector_alu_inst_count++;
		self->vector_alu_inst_count++;

		/* Special case: V_READFIRSTLANE_B32 */
		if (bytes->vop1.op == 2)
		{
			/* Instruction ignores execution mask and is only 
			 * executed on one work item. Execute on the first 
			 * active work item from the least significant bit 
			 * in EXEC. (if exec is 0, execute work item 0) */
			work_item = self->work_items[0];
			if (SIWorkItemReadSReg(work_item, SI_EXEC) == 0 && 
				SIWorkItemReadSReg(work_item, SI_EXEC + 1) == 0)
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
			else {
				SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, 
					work_item_id)
				{
					work_item = self->
						work_items[work_item_id];
					if(SIWavefrontIsWorkItemActive(
						self, 
						work_item->id_in_wavefront))
					{
						(*si_isa_inst_func[
						 	opcode])(
							work_item, inst);
						break;
					}
				}
			}
		}
		else
		{
			/* Execute the instruction */
			SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, 
				work_item_id)
			{
				work_item = self->work_items[work_item_id];
				if(SIWavefrontIsWorkItemActive(self, 
					work_item->id_in_wavefront))
				{
					(*si_isa_inst_func[opcode])(
						work_item, inst);
				}
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatVOPC:
	{
		/* Stats */
		emu->vector_alu_inst_count++;
		self->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if(SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}
	
	case SIInstFormatVOP3a:
	{
		/* Stats */
		emu->vector_alu_inst_count++;
		self->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if(SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatVOP3b:
	{
		/* Stats */
		emu->vector_alu_inst_count++;
		self->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if(SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatVINTRP:
	{
		/* Stats */
		emu->vector_alu_inst_count++;
		self->vector_alu_inst_count++;

		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if(SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}
 	
		break;
	}

	case SIInstFormatDS:
	{
		/* Stats */
		emu->lds_inst_count++;
		self->lds_inst_count++;

		/* Record access type */
		if ((op >= 13 && op < 16) ||
			(op >= 30 && op < 32) ||
			(op >= 77 && op < 80))
		{
			self->lds_write = 1;
		}
		else if (
			(op >= 54 && op < 61) ||
			(op >= 118 && op < 120))
		{
			self->lds_read = 1;
		}
		else 
		{
			fatal("%s: unimplemented LDS opcode", __FUNCTION__);
		}

		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if(SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	/* Vector Memory Instructions */
	case SIInstFormatMTBUF:
	{
		/* Stats */
		emu->vector_mem_inst_count++;
		self->vector_mem_inst_count++;

		/* Record access type */
		if (op >= 0 && op < 4)
			self->vector_mem_read = 1;
		else if (op >= 4 && op < 8)
			self->vector_mem_write = 1;
		else 
			fatal("%s: invalid mtbuf opcode", __FUNCTION__);
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if (SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatMUBUF:
	{
		/* Stats */
		emu->vector_mem_inst_count++;
		self->vector_mem_inst_count++;

		/* Record access type */
		if ((op >= 0 && op < 4) ||
			(op >= 8 && op < 15))
		{
			self->vector_mem_read = 1;
		}
		else if ((op >= 4 && op < 8) ||
			(op >= 24 && op < 30))
		{
			self->vector_mem_write = 1;
		}
		else if (op == 50)
		{
			self->vector_mem_atomic = 1;
		}
		else 
		{
			fatal("%s: unsupported mubuf opcode (%d)", 
				__FUNCTION__, op);
		}
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if (SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])
					(work_item, inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SIInstFormatEXP:
	{
		/* Stats */
		emu->export_inst_count++;
		self->export_inst_count++;

		/* Record access type */
		/* FIXME */
			
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = self->work_items[work_item_id];
			if (SIWavefrontIsWorkItemActive(self, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[opcode])
					(work_item, inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;

	}

	default:
	{
		fatal("%s: instruction type not implemented (%d)", 
			__FUNCTION__, format);
		break;
	}

	}

	/* Check if wavefront finished kernel execution */
	if (!self->finished)
	{
		/* Increment the PC */
		self->pc += self->inst_size;
	}
	else
	{
		/* Check if work-group finished kernel execution */
		if (work_group->wavefronts_completed_emu == 
			work_group->wavefront_count)
		{
			work_group->finished_emu = 1;
		}
	}
}

int SIWavefrontIsWorkItemActive(SIWavefront *self, 
	int id_in_wavefront) 
{
	int mask = 1;
	if(id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (self->sreg[SI_EXEC].as_uint & mask) >> 
			id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (self->sreg[SI_EXEC + 1].as_uint & mask) >> 
			(id_in_wavefront - 32);
	}
}

void SIWavefrontInitSRegWithValue(SIWavefront *self, 
	int sreg, unsigned int value)
{
	self->sreg[sreg].as_uint = value;
}

/* Puts a memory descriptor for a constant buffer (e.g. CB0) into sregs
 * (requires 4 consecutive registers to store the 128-bit structure) */
void SIWavefrontInitSRegWithConstantBuffer(SIWavefront *self, 
	int first_reg, int num_regs, int const_buf_num)
{
	struct si_buffer_desc_t buf_desc;
	SINDRange *ndrange = self->work_group->ndrange;
	SIEmu *emu = ndrange->emu;

	unsigned int buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buf_desc) == 16);
	assert(const_buf_num < SI_EMU_MAX_NUM_CONST_BUFS);
	assert(ndrange->const_buf_table_entries[const_buf_num].valid);

	buf_desc_addr = ndrange->const_buf_table +
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE;

	/* Read a descriptor from the constant buffer table (located 
	 * in global memory) */
	mem_read(emu->global_mem, buf_desc_addr, sizeof(buf_desc),
		&buf_desc);

	/* Store the descriptor in 4 scalar registers */
	memcpy(&self->sreg[first_reg], &buf_desc, sizeof(buf_desc));
}

/* Put a pointer to the constant buffer table into 2 consecutive sregs */
void SIWavefrontInitSRegWithConstantBufferTable(SIWavefront *self,
        int first_reg, int num_regs)
{
	SINDRange *ndrange = self->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->const_buf_table;

	memcpy(&self->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

/* Puts a memory descriptor for a UAV into sregs
 * (requires 4 consecutive registers to store the 128-bit structure) */
void SIWavefrontInitSRegWithUAV(SIWavefront *self, 
	int first_reg, int num_regs, int uav)
{
	struct si_buffer_desc_t buf_desc;

	SINDRange *ndrange = self->work_group->ndrange;
	SIEmu *emu = ndrange->emu;

	unsigned int buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buf_desc) == 16);
	assert(uav < SI_EMU_MAX_NUM_UAVS);
	assert(ndrange->uav_table_entries[uav].valid);

	buf_desc_addr = ndrange->uav_table + uav*SI_EMU_UAV_TABLE_ENTRY_SIZE;

	/* Read a descriptor from the constant buffer table (located 
	 * in global memory) */
	mem_read(emu->global_mem, buf_desc_addr, sizeof(buf_desc),
		&buf_desc);

	/* Store the descriptor in 4 scalar registers */
	memcpy(&self->sreg[first_reg], &buf_desc, sizeof(buf_desc));
}

/* Put a pointer to the UAV table into 2 consecutive sregs */
void SIWavefrontInitSRegWithUAVTable(SIWavefront *self, 
		int first_reg, int num_regs)
{
	SINDRange *ndrange = self->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->uav_table;

	memcpy(&self->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

/* Put a pointer to the Vertex Buffer table into 2 consecutive sregs */
void SIWavefrontInitSRegWithBufferTable(SIWavefront *self, 
		int first_reg, int num_regs)
{
	SINDRange *ndrange = self->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->vertex_buffer_table;

	memcpy(&self->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

/* Put a pointer to the Fetch Shader into 2 consecutive sregs */
void SIWavefrontInitSRegWithFetchShader(SIWavefront *self, 
	int first_reg, int num_regs)
{
	SINDRange *ndrange = self->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = ndrange->fs_buffer_ptr;

	memcpy(&self->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

