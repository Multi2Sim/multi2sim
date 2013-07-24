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

#include <assert.h>
#include <stdarg.h>

#include <lib/mhandle/mhandle.h>

#include "string.h"


/*
 * Helper functions
 */

static int CharInSet(char c, const char *set)
{
	while (*set)
	{
		if (c == *set)
			return 1;
		++set;
	}
	return 0;
}




/*
 * Class 'String'
 */

#define STRING_MAX_SIZE  (1 << 12)

static void StringGrow(String *self, size_t new_length)
{
	/* Increase size exponentially */
	while (self->size < new_length + 1)
		self->size <<= 1;
	
	/* Reallocate */
	self->str = xrealloc(self->str, self->size);
}


void StringCreate(String *self, const char *str)
{
	/* Initialize */
	self->str = xstrdup(str ? str : "");
	self->length = strlen(str);
	self->size = self->length + 1;

	/* Virtual functions */
	asObject(self)->Dump = StringDump;
	asObject(self)->Compare = StringCompare;
}


void StringDestroy(String *self)
{
	free(self->str);
}


void StringDump(Object *self, FILE *f)
{
	String *s = asString(self);
	fprintf(f, "%s", s->str);
}


int StringCompare(Object *self, Object *object)
{
	String *s1 = asString(self);
	String *s2 = asString(object);

	return strcmp(s1->str, s2->str);
}


void StringClear(String *self)
{
	self->str[0] = '\0';
	self->length = 0;
}


void StringReplace(String *self, int pos, size_t count, const char *str)
{
	int delta;
	int length;
	int new_length;

	/* Negative value for 'pos' is relative to the end of the string */
	if (pos < 0)
		pos += self->length;

	/* Invalid position */
	if (pos < 0 || pos > self->length)
		return;

	/* Truncate 'count' */
	if (count > self->length - pos)
		count = self->length - pos;

	/* Calculate length difference */
	length = strlen(str);
	delta = length - (int) count;
	new_length = (int) self->length + delta;

	/* Grow string if needed */
	if (new_length + 1 > self->size)
		StringGrow(self, new_length);

	/* Shift tail of string */
	memmove(self->str + pos + length, self->str + pos + count,
			self->length - pos - count + 1);
	self->length = new_length;

	/* Copy new string */
	memmove(self->str + pos, str, length);
}


void StringReplaceFmt(String *self, int pos, size_t count, const char *fmt, ...)
{
	char buf[STRING_MAX_SIZE];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);
	StringReplace(self, pos, count, buf);
}


void StringInsert(String *self, int pos, const char *str)
{
	StringReplace(self, pos, 0, str);
}


void StringInsertFmt(String *self, int pos, const char *fmt, ...)
{
	char buf[STRING_MAX_SIZE];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);
	StringInsert(self, pos, buf);
}


void StringConcat(String *self, const char *str)
{
	StringReplace(self, self->length, 0, str);
}


void StringConcatFmt(String *self, const char *fmt, ...)
{
	char buf[STRING_MAX_SIZE];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);
	StringConcat(self, buf);
}


void StringErase(String *self, int pos, size_t count)
{
	StringReplace(self, pos, count, "");
}


String *StringSubStr(String *self, int pos, size_t count)
{
	String *new_str;
	char old_char;

	/* Negative value for 'pos' is relative to the end of the string */
	if (pos < 0)
		pos += self->length;

	/* Invalid position */
	if (pos < 0 || pos > self->length)
		return new(String, "");

	/* Truncate 'count' */
	if (count > self->length - pos)
		count = self->length - pos;

	/* Temporarily truncate current string */
	old_char = self->str[pos + count];
	self->str[pos + count] = '\0';

	/* Create new string and restore current string */
	new_str = new(String, self->str + pos);
	self->str[pos + count] = old_char;

	/* Return new string */
	return new_str;
}


void StringTrimLeft(String *self, const char *set)
{
	int pos = 0;

	while (pos < self->length && CharInSet(self->str[pos], set))
		++pos;
	StringErase(self, 0, pos);
}


void StringTrimRight(String *self, const char *set)
{
	while (self->length && CharInSet(self->str[self->length - 1], set))
		self->str[--self->length] = '\0';
}


void StringTrim(String *self, const char *set)
{
	StringTrimLeft(self, set);
	StringTrimRight(self, set);
}

