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

inline unsigned int GetBits32(unsigned int x, int high, int low)
{
	return (x >> low) & ((1u << (high - low + 1)) - 1);
}

inline unsigned long long GetBits64(unsigned long long x, int high, int low)
{
	return (x >> low) & ((1ull << (high - low + 1)) - 1);
}

inline unsigned int ClearBits32(unsigned int x, int high, int low)
{
	return x & (((1ull << low) - 1) | ~((1ull << (high + 1)) - 1));
}

inline unsigned int TruncateBits32(unsigned int x, int num)
{
	return x & ((1ull << num) - 1);
}

inline unsigned int SetBits32(unsigned int x, int high, int low,
		unsigned int value)
{
	return ClearBits32(x, high, low) | (TruncateBits32(value, high
			- low + 1) << low);
}

inline unsigned int SignExtend32(unsigned int x, unsigned int b)
{
	return x & (1u << (b - 1)) ? x | ~((1u << b) - 1) :
			x & ((1u << b) - 1);
}

inline unsigned long long SignExtend64(unsigned long long x,
		unsigned long long b)
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


/* Macros to count the number of arguments passed to a macro using the '...'
 * notation in its prototype. Using PP_NARG(__VA_ARGS__) will return a number
 * between 1 and 64 containing the number of elements in __VA_ARGS__. */
#define PP_NARG(...) \
	PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
	PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
	_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
	_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
	_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
	_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
	_61,_62,_63,  N, ...) N
#define PP_RSEQ_N() \
	63,62,61,60,                   \
	59,58,57,56,55,54,53,52,51,50, \
	49,48,47,46,45,44,43,42,41,40, \
	39,38,37,36,35,34,33,32,31,30, \
	29,28,27,26,25,24,23,22,21,20, \
	19,18,17,16,15,14,13,12,11,10, \
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0



inline bool InRange(int value, int min, int max)
{
	return value >= min && value <= max;
}


template<class X, class Y> inline X cast(const Y &val)
{
	X result = dynamic_cast<X>(val);
	assert(result && "cast<Ty>() argument of incompatible type");
	return result;
}



} /* namespace Misc */

#endif
