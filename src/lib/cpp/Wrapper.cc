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

#include <string>

#include "Misc.h"
#include "Wrapper.h"

using namespace Misc;


const char *StringMapValueWrap(StringMapWrap *map, int value)
{
	StringMapItem *item = (StringMapItem *) map;
	return StringMapValue(item, value);
}


const char *StringMapValueErrWrap(StringMapWrap *map, int value, int *error_ptr)
{
	const char *result;
	bool error;

	StringMapItem *item = (StringMapItem *) map;
	result = StringMapValue(item, value, error);
	if (error_ptr)
		*error_ptr = error;
	return result;
}


int StringMapStringWrap(StringMapWrap *map, const char *text)
{
	StringMapItem *item = (StringMapItem *) map;
	return StringMapString(item, text);
}


int StringMapStringErrWrap(StringMapWrap *map, const char *text, int *error_ptr)
{
	int result;
	bool error;

	StringMapItem *item = (StringMapItem *) map;
	result = StringMapString(item, text, error);
	if (error_ptr)
		*error_ptr = error;
	return result;
}


int StringMapStringCaseWrap(StringMapWrap *map, const char *text)
{
	StringMapItem *item = (StringMapItem *) map;
	return StringMapStringCase(item, text);
}


int StringMapStringCaseErrWrap(StringMapWrap *map, const char *text, int *error_ptr)
{
	int result;
	bool error;

	StringMapItem *item = (StringMapItem *) map;
	result = StringMapStringCase(item, text, error);
	if (error_ptr)
		*error_ptr = error;
	return result;
}


void StringMapFlagsWrap(StringMapWrap *map, unsigned int flags, char *text, int size)
{
	StringMapItem *item = (StringMapItem *) map;
	std::string s = StringMapFlags(item, flags);
	snprintf(text, size, "%s", s.c_str());
}


void StringMapGetValuesWrap(StringMapWrap *map, char *text, int size)
{
	StringMapItem *item = (StringMapItem *) map;
	std::string s = StringMapGetValues(item);
	snprintf(text, size, "%s", s.c_str());
}

