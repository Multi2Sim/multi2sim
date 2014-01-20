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

#include <lib/cpp/Misc.h>

#include "Emu.h"
#include "NDRange.h"
#include "Wavefront.h"
#include "WorkGroup.h"

using namespace misc;

namespace SI
{

/*
 * Private functions
 */


/*
 * Public functions
 */
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

	// 
}

void Wavefront::Execute()
{
	
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
