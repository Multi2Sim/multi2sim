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

	char *str;

	/* Number of characters */
	size_t length;

	/* Space allocated for buffer 'str' */
	size_t size;

CLASS_END(String)

void StringCreate(String *self, const char *str);
void StringDestroy(String *self);

/* Inherited from class 'Object' */
void StringDump(Object *self, FILE *f);
Object *StringCopy(Object *self);
unsigned int StringHash(Object *self);

/* Compare two strings in alphabetical order, returning -1, 0, or 1 if the first
 * string is less, equal, or greater than the second, respectively. */
int StringCompare(Object *self, Object *object);

/* Convert the string into an empty string */
void StringClear(String *self);

/* Replace 'count' characters starting at position 'pos' by the string given in
 * 'str'. Negative values for 'pos' represent positions relative to the end of
 * the string, where -1 is the last character. */
void StringReplace(String *self, int pos, size_t count, const char *str);
void StringReplaceFmt(String *self, int pos, size_t count, const char *fmt, ...)
		__attribute__((format(printf, 4, 5)));

/* Insert 'str' at position 'pos' of the string. Negative values for 'pos'
 * represent positions relative to the end of the string. */
void StringInsert(String *self, int pos, const char *str);
void StringInsertFmt(String *self, int pos, const char *fmt, ...)
		__attribute__((format(printf, 3, 4)));

/* Concatenate 'str' with the content of the string. */
void StringConcat(String *self, const char *str);
void StringConcatFmt(String *self, const char *fmt, ...)
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

#endif

