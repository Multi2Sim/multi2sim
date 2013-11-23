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

#ifndef ARCH_X86_EMU_EXTENDED_H
#define ARCH_X86_EMU_EXTENDED_H

#include <iostream>


namespace x86
{

/// Class representing a 10-byte extended floating-point value.
class Extended
{
	// Bytes
	char x[10];

	static void DoubleToExtended(double f, char *x);
	static double ExtendedToDouble(const char *x);
	static void FloatToExtended(float f, char *x);
	static float ExtendedToFloat(const char *x);

public:

	/// Empty constructor, initializing the value to 0
	Extended() : x() { }

	/// Initialize with a value of type \c double. This constructor can be
	/// conveniently used as an implicit cast constructor.
	Extended(double value) { DoubleToExtended(value, x); }

	/// Initialize with a value of type \c float. This constructor can be
	/// conveniently used as an implicit cast constructor.
	Extended(float value) { FloatToExtended(value, x); }

	/// Dump value to output stream, or standard output if argument \os is
	/// not specified.
	void Dump(std::ostream &os = std::cout) const;

	/// Dump value, same as Dump()
	friend std::ostream &operator<<(std::ostream &os, const Extended &e) {
		e.Dump(os);
		return os;
	}

	/// Cast to type \c float.
	operator float() { return ExtendedToFloat(x); }

	/// Cast to type \c double.
	operator double() { return ExtendedToDouble(x); }
};


}  // namespace x86

#endif

