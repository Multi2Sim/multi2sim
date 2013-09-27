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

#include <iostream>
#include <vector>


namespace Misc
{


/*
 * Inline bit manipulation
 */

inline unsigned int GetBit(unsigned int x, int high, int low)
{
	return (x >> low) & ((1u << (high - low + 1)) - 1);
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
		__attribute__ ((format (printf, 3, 4)));

inline bool CharInSet(char c, std::string set) { return set.find(c) !=
		std::string::npos; }

void StringTrimLeft(std::string& s, std::string set = " \t\n\r");
void StringTrimRight(std::string& s, std::string set = " \t\n\r");
void StringTrim(std::string& s, std::string set = " \t\n\r");
void StringSingleSpaces(std::string& s, std::string set =  " \t\n\r");

void StringToLower(std::string& s);
void StringToUpper(std::string& s);

/* Split a string in tokens and place them in the list of strings passed by
 * reference in the first argument. Optionally, argument 'set' can specify the
 * characters considered as tokens separators. */
void StringTokenize(std::vector<std::string>& tokens, std::string s,
		std::string set = " \t\n\r");

/* Convert 'digit' in base 'base' into an integer. */
int StringDigitToInt(char digit, int base);
int StringDigitToInt(char digit, int base, StringError& error);

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
int StringToInt(std::string s);
int StringToInt(std::string s, StringError& error);
long long StringToInt64(std::string s);
long long StringToInt64(std::string s, StringError& error);




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

int StringMapString(StringMap map, const char *text);
int StringMapString(StringMap map, const char *text, bool &error);

int StringMapStringCase(StringMap map, const char *text);
int StringMapStringCase(StringMap map, const char *text, bool &error);

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
 * Miscellaneous
 */

inline bool InRange(int value, int min, int max)
{
	return value >= min && value <= max;
}



} /* namespace Misc */

#endif

