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


/*
 * Inline bit manipulation
 */

inline unsigned GetBits32(unsigned x, int high, int low) {
	return (x >> low) & ((1u << (high - low + 1)) - 1);
}

inline bool GetBit32(unsigned x, int bit) {
	return x & (1u << bit);
}

inline unsigned long long GetBits64(unsigned long long x, int high, int low) {
	return (x >> low) & ((1ull << (high - low + 1)) - 1);
}

inline bool GetBit64(unsigned long long x, int bit) {
	return x & (1ull << bit);
}

inline unsigned ClearBits32(unsigned x, int high, int low)
{
	return x & (((1ull << low) - 1) | ~((1ull << (high + 1)) - 1));
}

inline unsigned TruncateBits32(unsigned x, int num)
{
	return x & ((1ull << num) - 1);
}

inline unsigned SetBits32(unsigned x, int high, int low,
		unsigned value)
{
	return ClearBits32(x, high, low) | (TruncateBits32(value, high
			- low + 1) << low);
}

inline unsigned SetBit32(unsigned x, int bit)
{
	return x | (1u << bit);
}

inline unsigned ClearBit32(unsigned x, int bit)
{
	return x & ~(1u << bit);
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



/*
 * Output messages
 */

void warning(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void fatal(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void panic(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));




/*
 * Miscelaneous
 */


inline bool InRange(int value, int min, int max) {
	return value >= min && value <= max;
}

inline unsigned RoundUp(unsigned n, unsigned align) {
	assert(!(n & (n - 1)));
	return (n + (align - 1)) & ~(align - 1);
}

inline unsigned RoundDown(unsigned n, unsigned align) {
	assert(!(n & (n - 1)));
	return n & ~(align - 1);
}

template<class X, class Y> inline X cast(const Y &val) {
	X result = dynamic_cast<X>(val);
	assert(result && "cast<Ty>() argument of incompatible type");
	return result;
}



} // namespace misc

#endif

