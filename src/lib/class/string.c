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
#include <ctype.h>
#include <stdarg.h>

#include <lib/mhandle/mhandle.h>

#include "list.h"
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
	self->text = xrealloc(self->text, self->size);
}


void StringCreate(String *self, const char *text)
{
	/* Initialize */
	self->text = xstrdup(text ? text : "");
	self->length = strlen(text);
	self->size = self->length + 1;

	/* Virtual functions */
	asObject(self)->Dump = StringDump;
	asObject(self)->Compare = StringCompare;
	asObject(self)->Hash = StringHash;
	asObject(self)->Clone = StringClone;
}


void StringDestroy(String *self)
{
	free(self->text);
}


Object *StringClone(Object *self)
{
	String *string;

	string = new(String, asString(self)->text);
	return asObject(string);
}


void StringDump(Object *self, FILE *f)
{
	String *s = asString(self);
	fprintf(f, "%s", s->text);
}


int StringCompare(Object *self, Object *object)
{
	String *s1 = asString(self);
	String *s2 = asString(object);

	return strcmp(s1->text, s2->text);
}


int StringCompareCase(Object *self, Object *object)
{
	String *s1 = asString(self);
	String *s2 = asString(object);

	return strcasecmp(s1->text, s2->text);
}


unsigned int StringHash(Object *self)
{
	char *text;

	unsigned int c;
	unsigned int hash;
	unsigned int prime;

	text = asString(self)->text;
	hash = 5381;
	prime = 16777619;
	while (*text)
	{
		c = * (unsigned char *) text;
		hash = (hash ^ c) * prime;
		text++;
	}
	return hash;
}


unsigned int StringHashCase(Object *self)
{
	char *text;

	unsigned int c;
	unsigned int hash;
	unsigned int prime;

	text = asString(self)->text;
	hash = 5381;
	prime = 16777619;
	while (*text)
	{
		c = tolower(* (unsigned char *) text);
		hash = (hash ^ c) * prime;
		text++;
	}
	return hash;
}


void StringClear(String *self)
{
	self->text[0] = '\0';
	self->length = 0;
}


static void __StringReplace(String *self, int pos, size_t count, const char *str)
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
	memmove(self->text + pos + length, self->text + pos + count,
			self->length - pos - count + 1);
	self->length = new_length;

	/* Copy new string */
	memmove(self->text + pos, str, length);
}


void StringReplace(String *self, int pos, size_t count, const char *fmt, ...)
{
	char buf[STRING_MAX_SIZE];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);
	__StringReplace(self, pos, count, buf);
}


static void __StringInsert(String *self, int pos, const char *str)
{
	__StringReplace(self, pos, 0, str);
}


void StringInsert(String *self, int pos, const char *fmt, ...)
{
	char buf[STRING_MAX_SIZE];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);
	__StringInsert(self, pos, buf);
}


static void __StringConcat(String *self, const char *str)
{
	__StringReplace(self, self->length, 0, str);
}


void StringConcat(String *self, const char *fmt, ...)
{
	char buf[STRING_MAX_SIZE];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);
	__StringConcat(self, buf);
}


void StringErase(String *self, int pos, size_t count)
{
	__StringReplace(self, pos, count, "");
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
	old_char = self->text[pos + count];
	self->text[pos + count] = '\0';

	/* Create new string and restore current string */
	new_str = new(String, self->text + pos);
	self->text[pos + count] = old_char;

	/* Return new string */
	return new_str;
}


void StringTrimLeft(String *self, const char *set)
{
	int pos = 0;

	while (pos < self->length && CharInSet(self->text[pos], set))
		++pos;
	StringErase(self, 0, pos);
}


void StringTrimRight(String *self, const char *set)
{
	while (self->length && CharInSet(self->text[self->length - 1], set))
		self->text[--self->length] = '\0';
}


void StringTrim(String *self, const char *set)
{
	StringTrimLeft(self, set);
	StringTrimRight(self, set);
}


List *StringTokenize(String *self, const char *set)
{
	List *list;
	String *token;

	int token_start;
	int is_end;
	int i;

	/* Create list */
	list = new(List);

	/* Extract tokens */
	token_start = -1;
	for (i = 0; i <= self->length; i++)
	{
		/* End of string */
		is_end = i == self->length;

		/* Start a token */
		if (!is_end && !CharInSet(self->text[i], set)
				&& token_start == -1)
			token_start = i;

		/* End a token */
		if (token_start > -1 && (is_end ||
				CharInSet(self->text[i], set)))
		{
			token = StringSubStr(self, token_start, i - token_start);
			ListAdd(list, asObject(token));
			token_start = -1;
		}
	}

	/* Return token list */
	return list;
}
