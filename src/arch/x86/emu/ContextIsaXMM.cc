/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include "Context.h"

#define __UNIMPLEMENTED__ IsaError("%s: unimplemented instruction", \
		__FUNCTION__);

namespace x86
{

void Context::StoreXMM(const XMMValue &value)
{
	this->regs.getXMM(this->inst.getModRmReg()) = value;
}

void Context::LoadXMM(XMMValue &value)
{
	value = this->regs.getXMM(this->inst.getModRmReg());
}

void Context::StoreXMMM32(const XMMValue &value)
{
	unsigned int content = value.getAsUInt(0);
	if(this->inst.getModRmMod() == 3)
	{
		// Store into XMM
		this->regs.getXMM(this->inst.getModRmReg())
				.setAsUInt(0, content);
		return;
	}
	this->MemoryWrite((unsigned int)this->effective_address,
			4, (void*) &content);
}

void Context::LoadXMMM32(XMMValue &value)
{
	unsigned int content;
	if(this->inst.getModRmMod() == 3)
	{
		content = this->regs
				.getXMM(this->inst.getModRmReg())
				.getAsUInt(0);
	}
	else
	{
		this->MemoryRead((unsigned int)this->effective_address,
				4, (void *) &content);
	}
	value.setAsUInt(0, content);
}

void Context::StoreXMMM64(const XMMValue &value)
{
	unsigned long long content = value.getAsUInt64(0);
	if(this->inst.getModRmMod() == 3)
	{
		// Store into XMM
		this->regs.getXMM(this->inst.getModRmReg())
				.setAsUInt64(0, content);
		return;
	}
	this->MemoryWrite((unsigned int)this->effective_address,
			8, (void*) &content);
}

void Context::LoadXMMM64(XMMValue &value)
{
	unsigned long long content;
	if(this->inst.getModRmMod() == 3)
	{
		content = this->regs
				.getXMM(this->inst.getModRmReg())
				.getAsUInt64(0);
	}
	else
	{
		this->MemoryRead((unsigned int)this->effective_address,
				8, (void *) &content);
	}
	value.setAsUInt(0, content);
}

void Context::StoreXMMM128(const XMMValue &value)
{
	unsigned long long contentLo = value.getAsUInt64(0);
	unsigned long long contentHi = value.getAsUInt64(1);
	if(this->inst.getModRmMod() == 3)
	{
		// Store into XMM
		this->regs.getXMM(this->inst.getModRmReg())
				.setAsUInt64(0, contentLo);
		this->regs.getXMM(this->inst.getModRmReg())
				.setAsUInt64(1, contentHi);
		return;
	}
	this->MemoryWrite((unsigned int)this->effective_address,
			8, (void*) &contentLo);
	this->MemoryWrite((unsigned int)this->effective_address + 8,
			8, (void*) &contentHi);
}

void Context::LoadXMMM128(XMMValue &value)
{
	unsigned long long contentLo;
	unsigned long long contentHi;
	if(this->inst.getModRmMod() == 3)
	{
		contentLo = this->regs
				.getXMM(this->inst.getModRmReg())
				.getAsUInt64(0);
		contentHi = this->regs
				.getXMM(this->inst.getModRmReg())
				.getAsUInt64(1);
	}
	else
	{
		this->MemoryRead((unsigned int)this->effective_address,
				8, (void *) &contentLo);
		this->MemoryRead((unsigned int)this->effective_address + 8,
				8, (void *) &contentHi);
	}
	value.setAsUInt(0, contentLo);
	value.setAsUInt(0, contentHi);
}

} // namespace x86
