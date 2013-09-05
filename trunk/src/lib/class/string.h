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

#ifndef LIB_CLASS_STRING_H
#define LIB_CLASS_STRING_H

#include "class.h"


/*
 * Class 'String'
 */

CLASS_BEGIN(String, Object)

	char *text;

	/* Number of characters */
	size_t length;

	/* Space allocated for buffer 'text' */
	size_t size;

CLASS_END(String)

/* Default constructor */
void StringCreate(String *self, const char *text);

/* Create a new string with the text given in 'text' truncated to a maximum
 * length of 'max_length' characters (not including the null-termination). It is
 * safe to pass a not null-terminated string in 'text', as long as it is safe to
 * read 'max_length' characters from 'text'. */
void StringCreateMaxLength(String *self, const char *text, int max_length);

/* Destructor */
void StringDestroy(String *self);

/* Clone a string object */
Object *StringClone(Object *self);

/* Dump string into file descriptor */
void StringDump(Object *self, FILE *f);

/* Compare two strings in alphabetical order, returning -1, 0, or 1 if the first
 * string is less, equal, or greater than the second, respectively. The second
 * version of the function does a case-insensitive comparison. */
int StringCompare(Object *self, Object *object);
int StringCompareCase(Object *self, Object *object);

/* Calculate a 32-bit hash value for the string. The second version of the
 * function provides a case-insensitive hash value. */
unsigned int StringHash(Object *self);
unsigned int StringHashCase(Object *self);

/* Convert the string into an empty string */
void StringClear(String *self);

/* Set a new text for the string */
void StringSet(String *self, const char *fmt, ...)
		__attribute__((format(printf, 2, 3)));

/* Read a line from a file. The final '\n' character is omitted form the
 * resulting string. The function returns 0 on success and non-zero in case the
 * file could not be read. */
int StringRead(String *self, FILE *f);

/* Replace 'count' characters starting at position 'pos' by the string given in
 * 'text'. Negative values for 'pos' represent positions relative to the end of
 * the string, where -1 is the last character. */
void StringReplace(String *self, int pos, size_t count, const char *fmt, ...)
		__attribute__((format(printf, 4, 5)));

/* Insert 'text' at position 'pos' of the string. Negative values for 'pos'
 * represent positions relative to the end of the string. */
void StringInsert(String *self, int pos, const char *fmt, ...)
		__attribute__((format(printf, 3, 4)));

/* Concatenate 'text' with the content of the string. */
void StringConcat(String *self, const char *fmt, ...)
		__attribute__((format(printf, 2, 3)));

/* Erase 'count' characters starting at position 'pos' of the string. Negative
 * values for 'pos' are relative to the end of the string. */
void StringErase(String *self, int pos, size_t count);

/* Create a new string as a substring starting at position 'pos' with 'count'
 * characters. If the value of 'pos' is invalid or 'count' is 0, an empty string
 * is returned. The returned string must be freed by the caller with a delete()
 * call. */
String *StringSubStr(String *self, int pos, size_t count);

/* Count the number of occurrences of a substring inside of the string. */
int StringCount(String *self, const char *sub_str);

/* Return the first occurrence of a substring inside of the string, or -1 if the
 * substring is not present. */
int StringIndex(String *self, const char *sub_str);

/* Remove consecutive characters in 'set' from the beginning, the end, or both
 * ends of the string. */
void StringTrimLeft(String *self, const char *set);
void StringTrimRight(String *self, const char *set);
void StringTrim(String *self, const char *set);

/* Replace consecutive spaces/tabs/newlines in a string by one single space. Any
 * spaces/tabs/newlines at the beginning and end of the string are removed. */
void StringSingleSpaces(String *self);

/* Convert to lower/upper case. */
void StringToLower(String *self);
void StringToUpper(String *self);

/* Create a list (and return it) with string representing all tokens. Argument
 * 'set' is a string where each character is a possible separator. The caller is
 * responsible for freeing all returned strings, as well as the list itself. */
List *StringTokenize(String *self, const char *set);

/* Set a new value for the 'case_sensitive' flag, and update the virtual
 * functions used to compare and hash the string. */
void StringSetCaseSensitive(String *self, int case_sensitive);

/* Convert a string 'str' into an integer, accepting the following modifiers.
 * If conversion fails due to wrong formatting of 'str', an error code is returned
 * in argument 'err' (can be NULL).
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
int StringToInt(String *self, int *error_ptr);
long long StringToInt64(String *self, int *error_ptr);




/*
 * Public Functions
 */

/* String error codes */
typedef enum
{
	StringErrorOK = 0,
	StringErrorBase,
	StringErrorFormat,
	StringErrorRange
} StringError;

/* Return a description of an error code returned by 'StringXXX' functions. */
char *StringGetErrorString(int error);

int StringDigitToInt(char digit, int base, int *error_ptr);




/*
 * String Maps
 */

typedef struct
{
	char *string;
	int value;
} StringMap[];

char *StringMapValue(StringMap map, int value);
char *StringMapValueErr(StringMap map, int value, int *err_ptr);

int StringMapString(StringMap map, char *string);
int StringMapStringErr(StringMap map, char *string, int *err_ptr);

int StringMapStringCase(StringMap map, char *string);
int StringMapStringCaseErr(StringMap map, char *string, int *err_ptr);

String *StringMapFlags(StringMap map, unsigned int flags);

/* Return a string with a list of values present in the string map, set off by
 * brackets, and separated by commas. It is the responsibility of the caller to
 * free the returned string. */
String *StringMapGetValues(StringMap map);

#endif

