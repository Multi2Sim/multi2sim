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

#include <lib/cpp/String.h>

#include "Extended.h"


namespace x86
{


void Extended::DoubleToExtended(double f, unsigned char *x)
{
	asm volatile ("fldl %1; fstpt %0\n\t"
			: "=m" (*x) : "m" (f));
}


double Extended::ExtendedToDouble(const unsigned char *x)
{
	double f;
	asm volatile ("fldt %1; fstpl %0\n\t"
			: "=m" (f) : "m" (*x));
	return f;
}


void Extended::FloatToExtended(float f, unsigned char *x)
{
	asm volatile ("flds %1; fstpt %0\n\t"
			: "=m" (*x) : "m" (f));
}


float Extended::ExtendedToFloat(const unsigned char *x)
{
	float f;
	asm volatile ("fldt %1; fstps %0\n\t"
			: "=m" (f) : "m" (*x));
	return f;
}


void Extended::Dump(std::ostream &os) const
{
	double d = ExtendedToDouble(x);
	os << misc::fmt("%g", d);
}


}  // namespace x86

