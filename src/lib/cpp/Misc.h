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
#include <memory>
#include <vector>


namespace misc
{


//
// Bitwise Functions
//
inline unsigned short getBits16(unsigned short x, int high, int low)
{
	return (x >> low) & ((1u << (high - low + 1)) - 1);
}

inline unsigned getBits32(unsigned x, int high, int low)
{
	return (x >> low) & ((1u << (high - low + 1)) - 1);
}

inline bool getBit32(unsigned x, int bit)
{
	return x & (1u << bit);
}

inline unsigned long long getBits64(unsigned long long x, int high, int low)
{
	return (x >> low) & ((1ull << (high - low + 1)) - 1);
}

inline bool getBit64(unsigned long long x, int bit)
{
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
// Numeric Functions
//

/// Return the logarithm in base 2 of the argument. The value given as an
/// argument must be a power of 2 and greater than 0.
///
/// \throw
///	This function throws an `misc::Panic` exception is the value is 0
///	or is not a valid power of 2.
unsigned LogBase2(unsigned value);




//
// Output messages
//

/// Warning message
void Warning(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

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

/// Return the full path for a relative path.
///
/// \param path
///	The path to convert. If this is a relative path (i.e., does not start
///	with a '/' character, it will be converted to an absolute path. If it is
///	already an absolute path, it will be returned directly. If the string is
///	empty, an empty string is returned.
///
/// \param cwd (optional)
///	Base directory used for conversions to absolute paths. If no value is
///	given, the current directory will be used.
///
/// \return
///	Absolute path, or empty string if \a path is empty.
std::string getFullPath(const std::string &path, const std::string &cwd = "");


/// Returns the extension of a file name.
/// 
/// \param path
///	The file name with the extension 
///
/// \return
/// 	The extension of the filename, or empty if \a path does not have 
/// 	an extension
std::string getExtension(const std::string &path);


/// Obtain a file name without its extension. If the file name has not
/// extension, the same name is returned.
///
/// \param path
///	The file name with an extension.
std::string getBaseName(const std::string &path);




//
// Miscellaneous
//


inline bool inRange(int value, int min, int max)
{
	return value >= min && value <= max;
}


inline unsigned RoundUp(unsigned n, unsigned align)
{
	assert(!(align & (align - 1)));
	return (n + (align - 1)) & ~(align - 1);
}


inline unsigned RoundDown(unsigned n, unsigned align)
{
	assert(!(align & (align - 1)));
	return n & ~(align - 1);
}


template<class X, class Y> inline X cast(const Y &val)
{
	X result = dynamic_cast<X>(val);
	assert(result && "cast<Ty>() argument of incompatible type");
	return result;
}


template<typename T, typename... Args> std::shared_ptr<T>
		new_shared(Args&&... args)
{
	return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
}


template<typename T, typename... Args> std::unique_ptr<T>
		new_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}


/// Create a unique pointer to an array of elements of type T. The elements of
/// the array will be initialized to zero. The following example allocates a
/// dynamic array of 10 integers initialized to zero:
///
///    std::unique_ptr<int[]> A;
///    A = misc::new_unique_array<int>(10);
///
template<typename T> std::unique_ptr<T[]>new_unique_array(int size)
{
	return std::unique_ptr<T[]>(new T[size]());
}





} // namespace misc

#endif

