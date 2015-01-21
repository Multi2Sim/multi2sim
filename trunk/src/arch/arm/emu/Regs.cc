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
#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Regs.h"

namespace ARM
{

Regs::Regs() : sl(0), fp(0), ip(0), sp(0), lr(0), pc(0)
{
	for(int i = 0; i <= 9; i++)
	{
		setRegister(i, 0);
	}

	getCPSR().mode = ARM_MODE_USER;
}


Regs::~Regs()
{

}


unsigned int Regs::getRegister(int n)
{
	// Get the register according to the register number
	unsigned int ret_reg;
	switch (n)
	{
	case 0:

		ret_reg = r0;
		break;

	case 1:
		ret_reg = r1;
		break;

	case 2:
		ret_reg = r2;
		break;

	case 3:

		ret_reg = r3;
		break;

	case 4:

		ret_reg = r4;
		break;

	case 5:

		ret_reg = r5;
		break;

	case 6:

		ret_reg = r6;
		break;

	case 7:

		ret_reg = r7;
		break;

	case 8:

		ret_reg = r8;
		break;

	case 9:

		ret_reg = r9;
		break;

	default:
		throw misc::Panic(misc::fmt("the register number is not recognized"));
	}
	return ret_reg;
}


void Regs::setRegister(int n, unsigned int value)
{
	// Set the register according to the register number
	switch (n)
	{
	case 0:

		r0 = value;
		break;

	case 1:
		r1 = value;
		break;

	case 2:
		r2 = value;
		break;

	case 3:

		r3 = value;
		break;

	case 4:

		r4 = value;
		break;

	case 5:

		r5 = value;
		break;

	case 6:

		r6 = value;
		break;

	case 7:

		r7 = value;
		break;

	case 8:

		r8 = value;
		break;

	case 9:

		r9 = value;
		break;

	default:
		throw misc::Panic(misc::fmt("the register number is not recognized"));
	}
}

}
