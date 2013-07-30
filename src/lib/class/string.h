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

void StringCreate(String *self, const char *str);
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

/* Remove consecutive characters in 'set' from the beginning, the end, or both
 * ends of the string. */
void StringTrimLeft(String *self, const char *set);
void StringTrimRight(String *self, const char *set);
void StringTrim(String *self, const char *set);

/* Create a list (and return it) with string representing all tokens. Argument
 * 'set' is a string where each character is a possible separator. The caller is
 * responsible for freeing all returned strings, as well as the list itself. */
List *StringTokenize(String *self, const char *set);

/* Set a new value for the 'case_sensitive' flag, and update the virtual
 * functions used to compare and hash the string. */
void StringSetCaseSensitive(String *self, int case_sensitive);

#endif

