/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CPP_MISC_H
#define LIB_CPP_MISC_H

#include <cassert>
#include <iostream>
#include <vector>


namespace misc
{


//
// Inline bit manipulation
//

inline unsigned getBits32(unsigned x, int high, int low) {
	return (x >> low) & ((1u << (high - low + 1)) - 1);
}

inline bool getBit32(unsigned x, int bit) {
	return x & (1u << bit);
}

inline unsigned long long getBits64(unsigned long long x, int high, int low) {
	return (x >> low) & ((1ull << (high - low + 1)) - 1);
}

inline bool getBit64(unsigned long long x, int bit) {
	return x & (1ull << bit);
}

inline unsigned clearBits32(unsigned x, int high, int low)
{
	return x & (((1ull << low) - 1) | ~((1ull << (high + 1)) - 1));
}

inline unsigned truncBits32(unsigned x, int num)
{
	return x & ((1ull << num) - 1);
}

inline unsigned setBits32(unsigned x, int high, int low,
		unsigned value)
{
	return clearBits32(x, high, low) | (truncBits32(value, high
			- low + 1) << low);
}

inline unsigned setBit32(unsigned x, int bit)
{
	return x | (1u << bit);
}

inline unsigned clearBit32(unsigned x, int bit)
{
	return x & ~(1u << bit);
}

inline unsigned setBit32(unsigned x, int bit, bool value)
{
	return value ? setBit32(x, bit) : clearBit32(x, bit);
}

inline unsigned SignExtend32(unsigned x, unsigned b)
{
	return x & (1u << (b - 1)) ? x | ~((1u << b) - 1) :
			x & ((1u << b) - 1);
}

inline unsigned long long SignExtend64(unsigned long long x,
		unsigned b)
{
	return x & (1ull << (b - 1)) ? x | ~((1ull << b) - 1) :
			x & ((1ull << b) - 1);
}



//
// Output messages
//

/// Warning message
void warning(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

/// Error message terminating the program as a result of an invalid user input
/// or runtime error.
void fatal(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

/// Error message aborting the program as a result of a logic error in the
/// program execution.
void panic(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));



//
// File system
//

/// Return the current working directory
std::string getCwd();



//
// Miscelaneous
//


inline bool inRange(int value, int min, int max) {
	return value >= min && value <= max;
}

inline unsigned RoundUp(unsigned n, unsigned align) {
	assert(!(align & (align - 1)));
	return (n + (align - 1)) & ~(align - 1);
}

inline unsigned RoundDown(unsigned n, unsigned align) {
	assert(!(align & (align - 1)));
	return n & ~(align - 1);
}

template<class X, class Y> inline X cast(const Y &val) {
	X result = dynamic_cast<X>(val);
	assert(result && "cast<Ty>() argument of incompatible type");
	return result;
}



} // namespace misc

#endif

