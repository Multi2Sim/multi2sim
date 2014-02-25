/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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


namespace x86
{

void Context::EmitUInstEffectiveAddress(UInst *uinst, int index)
{
}

void Context::ParseUInstIDep(UInst *uinst, int index)
{
	assert(misc::inRange(index, 0, UInstMaxIDeps - 1));
}

void Context::ParseUInstODep(UInst *uinst, int index)
{
	assert(misc::inRange(index, 0, UInstMaxODeps - 1));
}

void Context::ProcessNewUInst(UInst *uinst)
{
	// Emit effective address computation if needed.
	for (int i = 0; !uinst_effaddr_emitted && i < UInstMaxDeps; i++)
		EmitUInstEffectiveAddress(uinst, i);
	
	// Parse input dependences
	for (int i = 0; i < UInstMaxIDeps; i++)
		ParseUInstIDep(uinst, i);
	
	// Add micro-instruction to list
	uinst_list.emplace_back(uinst);
	
	// Parse output dependences
	for (int i = 0; i < UInstMaxODeps; i++)
		ParseUInstODep(uinst, i);
}


}  // namespace x86

