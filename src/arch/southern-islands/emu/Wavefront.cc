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

#if 0



#endif
	
}  // namespace SI 
