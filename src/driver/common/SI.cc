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
#include <arch/southern-islands/emu/Emu.h>
#include <arch/southern-islands/emu/NDRange.h>
#include <arch/southern-islands/emu/WorkGroup.h>
#include <arch/southern-islands/emu/Wavefront.h>
#include <arch/southern-islands/emu/WorkItem.h>
#include <lib/cpp/Misc.h>

#include "SI.h"

using namespace misc;

namespace Driver
{

SI::NDRange *SICommon::getNDRangeById(int id)
{
	for( auto &ndrange : ndranges)
	{
		if (ndrange->getId() == id)
			return ndrange.get();
	}

	// Return nullptr if can't find in the ndrange list
	return nullptr;
}

void SICommon::AddNDRange(std::unique_ptr<SI::NDRange> ndrange)
{
	ndranges.push_back(std::move(ndrange));
}

void SICommon::RemoveNDRangeById(int id)
{
	for( auto n_i = getNDRangeBegin(), n_e = getNDRangeEnd(); n_i != n_e; n_i++ )
	{
		if ((*n_i)->getId() == id)
		{
			ndranges.erase(n_i);
			return;
		}
	}

	fatal("%s: invalid ndrange ID (%d)", __FUNCTION__, id);
}

}  // namespace Driver