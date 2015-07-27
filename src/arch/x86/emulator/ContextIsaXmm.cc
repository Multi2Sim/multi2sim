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

void Context::StoreXmm(const XmmValue &value)
{
	regs.getXMM(inst.getModRmReg()) = value;
}

void Context::LoadXmm(XmmValue &value)
{
	value = regs.getXMM(inst.getModRmReg());
}

void Context::StoreXmmM32(const XmmValue &value)
{
	unsigned int content = value.getAsUInt(0);
	if (inst.getModRmMod() == 3)
	{
		// Store into XMM
		regs.getXMM(inst.getModRmRm()).setAsUInt(0, content);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 4, (char *) &content);
}

void Context::LoadXmmM32(XmmValue &value)
{
	unsigned int content;
	if(inst.getModRmMod() == 3)
	{
		content = regs.getXMM(inst.getModRmRm()).getAsUInt(0);
	}
	else
	{
		MemoryRead(getEffectiveAddress(), 4, (char *) &content);
	}
	value.setAsUInt(0, content);
}

void Context::StoreXmmM64(const XmmValue &value)
{
	unsigned long long content = value.getAsUInt64(0);
	if(inst.getModRmMod() == 3)
	{
		// Store into XMM
		regs.getXMM(inst.getModRmRm()).setAsUInt64(0, content);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 8, (char *) &content);
}

void Context::LoadXmmM64(XmmValue &value)
{
	unsigned long long content;
	if(inst.getModRmMod() == 3)
	{
		content = regs.getXMM(inst.getModRmRm()).getAsUInt64(0);
	}
	else
	{
		MemoryRead(getEffectiveAddress(), 8, (char *) &content);
	}
	value.setAsUInt64(0, content);
}

void Context::StoreXmmM128(const XmmValue &value)
{
	unsigned long long contentLo = value.getAsUInt64(0);
	unsigned long long contentHi = value.getAsUInt64(1);
	if(inst.getModRmMod() == 3)
	{
		// Store into XMM
		regs.getXMM(inst.getModRmRm())
				.setAsUInt64(0, contentLo);
		regs.getXMM(inst.getModRmRm())
				.setAsUInt64(1, contentHi);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 8, (void*) &contentLo);
	MemoryWrite(getEffectiveAddress() + 8,
			8, (void*) &contentHi);
}

void Context::LoadXmmM128(XmmValue &value)
{
	unsigned long long content_lo;
	unsigned long long content_hi;

	if (inst.getModRmMod() == 3)
	{
		content_lo = regs.getXMM(inst.getModRmRm()).getAsUInt64(0);
		content_hi = regs.getXMM(inst.getModRmRm()).getAsUInt64(1);
	}
	else
	{
		MemoryRead(getEffectiveAddress(), 8, (char *) &content_lo);
		MemoryRead(getEffectiveAddress() + 8, 8, (char *) &content_hi);
	}
	value.setAsUInt64(0, content_lo);
	value.setAsUInt64(1, content_hi);
}

} // namespace x86
