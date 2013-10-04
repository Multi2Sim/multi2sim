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

#ifndef LIB_CPP_WRAPPER_H
#define LIB_CPP_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif


/*
 * String Maps
 */

struct StringMapWrap;

const char *StringMapValueWrap(struct StringMapWrap *map, int value);
const char *StringMapValueErrWrap(struct StringMapWrap *map, int value, int *error);

int StringMapStringWrap(struct StringMapWrap *map, const char *text);
int StringMapStringErrWrap(struct StringMapWrap *map, const char *text, int *error);

int StringMapStringCaseWrap(struct StringMapWrap *map, const char *text);
int StringMapStringCaseErrWrap(struct StringMapWrap *map, const char *text, int *error);

void StringMapFlagsWrap(struct StringMapWrap *map, unsigned int flags, char *text, int size);
void StringMapGetValuesWrap(struct StringMapWrap *map, char *text, int size);



/*
 * Debug
 */

struct DebugWrap;

void DebugSetPathWrap(struct DebugWrap *debug, const char *path);


#ifdef __cplusplus
}
#endif


#endif /* LIB_CPP_WRAPPER */

