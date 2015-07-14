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

#include <arch/hsa/disassembler/AsmService.h>

#include "Emulator.h"
#include "Variable.h"
#include "SegmentManager.h"


namespace HSA
{

Variable::Variable(const std::string& name, BrigType type,
		unsigned long long dim, unsigned address,
		BrigSegment segment,
		bool isFormal = false) :
		name(name),
		type(type),
		segment(segment),
		size(AsmService::TypeToSize(type)),
		dim(dim),
		address(address),
		isFormal(isFormal)
{
}

}  // namespace HSA

