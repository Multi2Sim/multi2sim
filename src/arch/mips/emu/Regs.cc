/*
 *  Multi2Sim
 *  Copyright (C) 2012  Sida Gu (gu.sid@husky.neu.edu)
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
#include "Regs.h"

namespace mips
{
Regs::Regs()
{	// initialize the GPR,and single precision fp regs
	for(int i =0; i < GPR_COUNT; i++)
	{
		regs_R[i] = 0;
		s[i] = 0;
	}

	// initialize the double precision fp regs
	for(int i =0; i < GPR_COUNT/2; i++)
	{
		d[i] = 0;
	}

}

unsigned int Regs::readGPR(int reg)
{
	return regs_R[reg];
}


}// namespace mips
