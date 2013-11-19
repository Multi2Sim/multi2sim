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
 * String manipulation
 */

enum StringError
{
	StringErrorOK = 0,
	StringErrorBase,
	StringErrorFormat,
	StringErrorRange
};

/* Return a string associated with an error code */
const char *StringGetErrorString(StringError error);

/* Dump formatted string into a buffer with a specific size. Its size is then
 * decreased, and the buffer is advanced to the end of the dumped string.
 * This function is useful for being used in other functions that dump
 * several strings into a buffer, with the header
 *   obj_dump(struct obj_t *obj, char *buf, int size); */
void str_printf(char **pbuf, int *psize, const char *fmt, ...)
		__attribute__ ((format(printf, 3, 4)));

std::string StringFmt(const char *fmt, ...)
		__attribute__ ((format(printf, 1, 2)));

inline bool CharInSet(char c, const std::string &set) {
	return set.find(c) != std::string::npos;
}

void StringTrimLeft(std::string &s, const std::string &set = " \t\n\r");
void StringTrimRight(std::string &s, const std::string &set = " \t\n\r");
void StringTrim(std::string &s, const std::string &set = " \t\n\r");
void StringSingleSpaces(std::string &s, const std::string &set =  " \t\n\r");

void StringToLower(std::string &s);
void StringToUpper(std::string &s);

bool StringPrefix(const std::string &s, const std::string &prefix);
bool StringSuffix(const std::string &s, const std::string &suffix);

/* Split a string in tokens and place them in the list of strings passed by
 * reference in the first argument. Optionally, argument 'set' can specify the
 * characters considered as tokens separators. */
void StringTokenize(const std::string &s, std::vector<std::string> &tokens,
		const std::string &set = " \t\n\r");

/* Convert 'digit' in base 'base' into an integer. */
int StringDigitToInt(char digit, int base);
int StringDigitToInt(char digit, int base, StringError &error);

/* Convert a string into an integer, accepting the following modifiers.
 * If conversion fails due to wrong formatting of the string, an error code is
 * returned in argument 'error' (optional).
 *
 * String prefixes:
 *   Prefix '0x' - Use base 16 for conversion.
 *   Prefix '0' - Use base 8 for conversion.
 * String suffixes:
 *   Suffix 'k' - Multiply by 1024.
 *   Suffix 'K' - Multiply by 1000.
 *   Suffix 'm' - Multiply by 1024*1024.
 *   Suffix 'M' - Multiply by 1000*1000.
 *   Suffix 'g' - Multiply by 1024*1024*1024.
 *   Suffix 'G' - Multiply by 1000*1000*1000.
 */
int StringToInt(const std::string &s);
int StringToInt(const std::string &s, StringError &error);
long long StringToInt64(const std::string &s);
long long StringToInt64(const std::string &s, StringError &error);




/*
 * String maps
 */

struct StringMapItem
{
	const char *text;
	int value;
};

typedef StringMapItem StringMap[];

const char *StringMapValue(StringMap map, int value);
const char *StringMapValue(StringMap map, int value, bool &error);

int StringMapString(StringMap map, const std::string &s);
int StringMapString(StringMap map, const std::string &s, bool &error);

int StringMapStringCase(StringMap map, const std::string &s);
int StringMapStringCase(StringMap map, const std::string &s, bool &error);

std::string StringMapFlags(StringMap map, unsigned int flags);

/* Return a string with a list of values present in the string map, set off in
 * brackets, and separated by commas. It is the responsibility of the caller to
 * free the returned string. */
std::string StringMapGetValues(StringMap map);



/*
 * Output messages
 */

void warning(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void fatal(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void panic(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));



/*
 * Debug
 */

class Debug
{
	std::string path;
	std::ostream *os;

	void Close();
public:
	Debug();
	~Debug();

	void setPath(const std::string &path);
	const std::string &getPath() { return path; }

	/* Dump a value into the output stream currently pointed to by the
	 * debug object. If the debugger has not been initialized with a call
	 * to SetPath(), this call is ignored. The argument can be of any
	 * type accepted by an std::ostream object. */
	template<typename T> Debug& operator<<(T val) {
		if (os)
			*os << val;
		return *this;
	}

	/* A debugger can be cast into a bool (e.g. within an 'if' condition)
	 * to check whether it has an active output stream or not. This is
	 * useful when many possibly costly operations are performed just
	 * to dump debug information. By checking whether the debugger is
	 * active or not in beforehand, multiple dump (<<) calls can be
	 * saved. */
	operator bool() { return os; }

	/* A debugger can also be cast into an std::ostream, returning a
	 * reference to its internal 'os' object. This should be done only when
	 * the output stream has been initialized properly with SetPath(). */
	operator std::ostream &() { return *os; }
};




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
