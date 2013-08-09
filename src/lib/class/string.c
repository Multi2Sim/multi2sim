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


static void StringDoCreate(String *self)
{
	/* Virtual functions */
	asObject(self)->Dump = StringDump;
	asObject(self)->Compare = StringCompare;
	asObject(self)->Hash = StringHash;
	asObject(self)->Clone = StringClone;
}


void StringCreate(String *self, const char *text)
{
	/* Initialize */
	self->text = xstrdup(text ? text : "");
	self->length = strlen(text);
	self->size = self->length + 1;

	/* Common constructor */
	StringDoCreate(self);
}


void StringCreateMaxLength(String *self, const char *text, int max_length)
{
	int length;

	/* Find length of 'text' */
	for (length = 0; length < max_length; length++)
		if (!text[length])
			break;
	
	/* Create string */
	self->length = length;
	self->size = self->length + 1;
	self->text = xmalloc(self->size);

	/* Copy text */
	memcpy(self->text, text, length);
	self->text[length] = '\0';

	/* Common constructor */
	StringDoCreate(self);
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


int StringRead(String *self, FILE *f)
{
	char line[STRING_MAX_SIZE];
	char *line_ptr;

	/* Clear string */
	StringClear(self);

	/* Read line */
	line_ptr = fgets(line, sizeof line, f);
	if (!line_ptr)
		return 1;

	/* Set new string */
	StringConcat(self, "%s", line);

	/* Remove final '\n' */
	if (self->length && self->text[self->length - 1] == '\n')
		StringErase(self, -1, 1);

	/* Return success */
	return 0;
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


int StringCount(String *self, const char *sub_str)
{
	int i;
	int count;
	int length;

	count = 0;
	length = strlen(sub_str);
	for (i = 0; i < self->length - length + 1; i++)
		if (!strncmp(self->text + i, sub_str, length))
			count++;
	
	return count;
}


int StringIndex(String *self, const char *sub_str)
{
	int i;
	int length;

	length = strlen(sub_str);
	for (i = 0; i < self->length - length + 1; i++)
		if (!strncmp(self->text + i, sub_str, length))
			return i;
	
	return -1;
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


void StringSingleSpaces(String *self)
{
	char *src;
	char *dest;
	char *set;

	int new_length;
	int is_space;
	int was_space;
	int started;
	int i;

	set = "\n\t\r ";
	src = self->text;
	dest = self->text;
	new_length = 0;
	is_space = 0;
	was_space = 0;
	started = 0;
	for (i = 0; i < self->length; i++)
	{
		is_space = CharInSet(*src, set);
		if (is_space)
		{
			src++;
			if (!was_space && started)
			{
				*(dest++) = ' ';
				new_length++;
			}
		}
		else
		{
			*(dest++) = *(src++);
			new_length++;
			started = 1;
		}
		was_space = is_space;
	}

	/* Get rid of possible extra space at the end */
	if (new_length && is_space)
	{
		dest--;
		new_length--;
	}

	/* Null-terminate */
	*dest = '\0';

	/* Set new length */
	self->length = new_length;
	assert(new_length == strlen(self->text));
}


void StringToLower(String *self)
{
	int i;

	for (i = 0; i < self->length; i++)
		self->text[i] = tolower(self->text[i]);
}


void StringToUpper(String *self)
{
	int i;

	for (i = 0; i < self->length; i++)
		self->text[i] = toupper(self->text[i]);
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




/*
 * String Maps
 */

static char *string_map_unknown = "<unknown>";

char *StringMapValue(StringMap map, int value)
{
	return StringMapValueErr(map, value, NULL);
}


char *StringMapValueErr(StringMap map, int value, int *err_ptr)
{
	int index;

	/* Assume no error */
	if (err_ptr)
		*err_ptr = 0;

	/* Find value */
	for (index = 0; map[index].string; index++)
		if (map[index].value == value)
			return map[index].string;
	
	/* Error */
	if (err_ptr)
		*err_ptr = 1;

	/* Not found */
	return string_map_unknown;
}


int StringMapString(StringMap map, char *string)
{
	return StringMapStringErr(map, string, NULL);
}


int StringMapStringErr(StringMap map, char *string, int *err_ptr)
{
	int index;

	/* Assume no error */
	if (err_ptr)
		*err_ptr = 0;

	/* Find value */
	for (index = 0; map[index].string; index++)
		if (!strcmp(map[index].string, string))
			return map[index].value;

	/* Error */
	if (err_ptr)
		*err_ptr = 1;
	
	/* Not found */
	return 0;
}


int StringMapStringCase(StringMap map, char *string)
{
	return StringMapStringCaseErr(map, string, NULL);
}


int StringMapStringCaseErr(StringMap map, char *string, int *err_ptr)
{
	int index;

	/* Assume no error */
	if (err_ptr)
		*err_ptr = 0;

	/* Find value */
	for (index = 0; map[index].string; index++)
		if (!strcasecmp(map[index].string, string))
			return map[index].value;

	/* Error */
	if (err_ptr)
		*err_ptr = 1;
	
	/* Not found */
	return 0;
}


String *StringMapFlags(StringMap map, unsigned int flags)
{
	int i;
	int err;

	char *comma = "";
	char *s;

	String *string;

	string = new(String, "{");
	for (i = 0; i < 32; i++)
	{
		if (flags & (1 << i))
		{
			StringConcat(string, "%s", comma);
			s = StringMapValueErr(map, 1 << i, &err);
			if (err)
				StringConcat(string, "%d", 1 << i);
			else
				StringConcat(string, "%s", s);
			comma = "|";
		}
	}
	StringConcat(string, "}");

	/* Return created string */
	return string;
}

