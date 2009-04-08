/*
 *  Libmhandle
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#ifndef MHANDLE_H
#define MHANDLE_H

#include <stdlib.h>
#include <string.h>

#define MHANDLE_STRINGIFY(x) #x
#define MHANDLE_TOSTRING(x) MHANDLE_STRINGIFY(x)
#define MHANDLE_AT __FILE__ ":" MHANDLE_TOSTRING(__LINE__)

#ifdef MHANDLE

#undef strdup
#define free(X) (mhandle_free(X, MHANDLE_AT))
#define malloc(X) (mhandle_malloc(X, MHANDLE_AT))
#define calloc(X, Y) (mhandle_calloc(X, Y, MHANDLE_AT))
#define realloc(X, Y) (mhandle_realloc(X, Y, MHANDLE_AT))
#define strdup(X) (mhandle_strdup(X, MHANDLE_AT))
#define mhandle_check() __mhandle_check(MHANDLE_AT)
#define mhandle_done() __mhandle_done()
#define mhandle_used_memory() __mhandle_used_memory()

#else

#define mhandle_check()
#define mhandle_done()
#define mhandle_used_memory() (0UL)

#endif // MHANDLE


void *mhandle_malloc(unsigned long size, char *at);
void *mhandle_calloc(unsigned long nmemb, unsigned long size, char *at);
void *mhandle_realloc(void *ptr, unsigned long size, char *at);
char *mhandle_strdup(const char *s, char *at);
void mhandle_free(void *ptr, char *at);

void __mhandle_check(char *at);
void __mhandle_done();
unsigned long __mhandle_used_memory();

#endif
