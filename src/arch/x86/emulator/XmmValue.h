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

#ifndef ARCH_X86_EMU_XMM_VALUE_H
#define ARCH_X86_EMU_XMM_VALUE_H

#include <iostream>
#include <cstring>

#include <lib/cpp/Misc.h>


namespace x86
{

/// Class representing a 128-bit XMM register value
class XmmValue
{
	// Bytes
	union
	{
		unsigned char as_uchar[16];
		char as_char[16];
	
		unsigned short as_ushort[8];
		short as_short[8];
	
		unsigned int as_uint[4];
		int as_int[4];
	
		unsigned long long as_uint64[2];
		long long as_int64[2];
	
		float as_float[4];
		double as_double[2];
	};

public:

	/// Default constructor, initializing XMM value to 0
	XmmValue() : as_uchar() { }

	/// Get content as a vector of 16 unsigned 8-bit integers
	unsigned char *getAsUChar() { return as_uchar; }

	/// Get content as a vector of 8 unsigned 16-bit integers
	unsigned short *getAsUShort() { return as_ushort; }

	/// Get content as a vector of 4 unsigned 32-bit integers
	unsigned *getAsUInt() { return as_uint; }

	/// Get content as a vector of 2 unsigned 64-bit integers
	unsigned long long *getAsUInt64() { return as_uint64; }

	/// Get content as a vector of 16 signed 8-bit integers
	char *getAsChar() { return as_char; }

	/// Get content as a vector of 8 signed 16-bit integers
	short *getAsShort() { return as_short; }

	/// Get content as a vector of 4 signed 32-bit integers
	int *getAsInt() { return as_int; }

	/// Get content as a vector of 2 signed 64-bit integers
	long long *getAsInt64() { return as_int64; }

	/// Get content as a vector of 4 \c float values
	float *getAsFloat() { return as_float; }

	/// Get content as a vector of 2 \c double values
	double *getAsDouble() { return as_double; }

	/// Get the 8-bit unsigned integer at position \a index
	unsigned char getAsUChar(int index) const
	{
		assert(misc::inRange(index, 0, 15));
		return as_uchar[index];
	}

	/// Get the 16-bit unsigned integer at position \a index
	unsigned short getAsUShort(int index) const
	{
		assert(misc::inRange(index, 0, 7));
		return as_ushort[index];
	}

	/// Get the 32-bit unsigned integer at position \a index
	unsigned getAsUInt(int index) const
	{
		assert(misc::inRange(index, 0, 3));
		return as_uint[index];
	}

	/// Get the 64-bit unsigned integer at position \a index
	unsigned long long getAsUInt64(int index) const
	{
		assert(misc::inRange(index, 0, 1));
		return as_uint64[index];
	}

	/// Get the 8-bit signed integer at position \a index
	char getAsChar(int index) const
	{
		assert(misc::inRange(index, 0, 15));
		return as_char[index];
	}

	/// Get the 16-bit signed integer at position \a index
	short getAsShort(int index) const
	{
		assert(misc::inRange(index, 0, 7));
		return as_short[index];
	}

	/// Get the 32-bit signed integer at position \a index
	int getAsInt(int index) const
	{
		assert(misc::inRange(index, 0, 3));
		return as_int[index];
	}

	/// Get the 64-bit signed integer at position \a index
	long long getAsInt64(int index) const
	{
		assert(misc::inRange(index, 0, 1));
		return as_int64[index];
	}

	/// Get the \c float value at position \a index
	float getAsFloat(int index) const
	{
		assert(misc::inRange(index, 0, 3));
		return as_float[index];
	}

	/// Get the \c double value at position \a index
	double getAsDouble(int index) const
	{
		assert(misc::inRange(index, 0, 1));
		return as_double[index];
	}

	/// Set unsigned 8-bit integer at position \a index
	void setAsUChar(int index, unsigned char value)
	{
		assert(misc::inRange(index, 0, 15));
		as_uchar[index] = value;
	}

	/// Set unsigned 16-bit integer at position \a index
	void setAsUShort(int index, unsigned short value)
	{
		assert(misc::inRange(index, 0, 7));
		as_ushort[index] = value;
	}

	/// Set unsigned 32-bit integer at position \a index
	void setAsUInt(int index, unsigned value)
	{
		assert(misc::inRange(index, 0, 3));
		as_uint[index] = value;
	}

	/// Set unsigned 64-bit integer at position \a index
	void setAsUInt64(int index, unsigned long long value)
	{
		assert(misc::inRange(index, 0, 1));
		as_uint64[index] = value;
	}

	/// Set signed 8-bit integer at position \a index
	void setAsChar(int index, char value)
	{
		assert(misc::inRange(index, 0, 15));
		as_char[index] = value;
	}

	/// Set signed 16-bit integer at position \a index
	void setAsShort(int index, short value)
	{
		assert(misc::inRange(index, 0, 7));
		as_short[index] = value;
	}

	/// Set signed 32-bit integer at position \a index
	void setAsInt(int index, int value)
	{
		assert(misc::inRange(index, 0, 3));
		as_int[index] = value;
	}

	/// Set signed 64-bit integer at position \a index
	void setAsInt64(int index, long long value)
	{
		assert(misc::inRange(index, 0, 1));
		as_int64[index] = value;
	}

	/// Set \c float value at position \a index
	void setAsFloat(int index, float value)
	{
		assert(misc::inRange(index, 0, 3));
		as_float[index] = value;
	}

	/// Set \c double value at position \a index
	void setAsDouble(int index, double value)
	{
		assert(misc::inRange(index, 0, 1));
		as_double[index] = value;
	}

	/// Copy the content of another value
	void setWithMemcpy(XmmValue *source)
	{
		memcpy(this->as_char, source->as_char, 16);
	}

	/// set each 8-bits to value
	void setWithMemset(unsigned int index, char value, unsigned int num)
	{
		assert(misc::inRange(index, 0, 16));
		assert(misc::inRange(index + num, 0, 16));
		memset(as_uchar + index, value, num);
	}

	/// Dump as sequence of 8-bit integers
	void DumpAsChar(std::ostream &os = std::cout) const;

	/// Dump as sequence of 16-bit integers
	void DumpAsShort(std::ostream &os = std::cout) const;

	/// Dump as sequence of 32-bit integers
	void DumpAsInt(std::ostream &os = std::cout) const;

	/// Dump as sequence of 64-bit integers
	void DumpAsInt64(std::ostream &os = std::cout) const;

	/// Dump as sequence of single-precision numbers
	void DumpAsFloat(std::ostream &os = std::cout) const;

	/// Dump as sequence of double-precision numbers
	void DumpAsDouble(std::ostream &os = std::cout) const;

	/// Equivalent to DumpAsChar()
	void Dump(std::ostream &os = std::cout) const
	{
		DumpAsChar(os);
	}

	/// Dump value as sequence of bytes, same as Dump()
	friend std::ostream &operator<<(std::ostream &os, const XmmValue &xmm)
	{
		xmm.Dump(os);
		return os;
	}
};


}  // namespace x86

#endif

