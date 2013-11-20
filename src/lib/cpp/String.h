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

#ifndef LIB_CPP_STRING_H
#define LIB_CPP_STRING_H

#include <string>
#include <vector>


namespace misc
{



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

std::string StringParagraph(const std::string &text,
		int indent = 0, int first_indent = 0,
		int width = 80);



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


} // namespace misc

#endif
