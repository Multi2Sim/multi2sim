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
	unsigned char x[10];

public:

	/// Convert a double into an extended
	static void DoubleToExtended(double f, unsigned char *x);

	/// Convert an extended into a double
	static double ExtendedToDouble(const unsigned char *x);

	/// Convert a float into an extended
	static void FloatToExtended(float f, unsigned char *x);

	/// Convert an extended into a float
	static float ExtendedToFloat(const unsigned char *x);

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
	friend std::ostream &operator<<(std::ostream &os, const Extended &e)
	{
		e.Dump(os);
		return os;
	}

	/// Return the represented extended number as a 32-bit \c float.
	float getFloat() const { return ExtendedToFloat(x); }

	/// Return the 64-bit \c double representation
	double getDouble() const { return ExtendedToDouble(x); }

	/// Return the extended value as a sequence of 10 bytes
	unsigned char *getValue() { return x; }
};


}  // namespace x86

#endif

