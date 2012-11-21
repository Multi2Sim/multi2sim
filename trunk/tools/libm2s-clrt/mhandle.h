/*
 *  Libmhandle
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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
#define free(x) (mhandle_free((x), MHANDLE_AT))
#define malloc(sz) (mhandle_malloc((sz), MHANDLE_AT))
#define calloc(nmemb, sz) (mhandle_calloc((nmemb), (sz), MHANDLE_AT))
#define realloc(x, sz) (mhandle_realloc((x), (sz), MHANDLE_AT))
#define strdup(x) (mhandle_strdup((x), MHANDLE_AT))

#define xmalloc(sz) (mhandle_malloc((sz), MHANDLE_AT))
#define xcalloc(nmemb, sz) (mhandle_calloc((nmemb), (sz), MHANDLE_AT))
#define xrealloc(x, sz) (mhandle_realloc((x), (sz), MHANDLE_AT))
#define xstrdup(x) (mhandle_strdup((x), MHANDLE_AT))

#define mhandle_check() __mhandle_check(MHANDLE_AT)
#define mhandle_done() __mhandle_done()
#define mhandle_used_memory() __mhandle_used_memory()

#else

#define xmalloc(sz) (__xmalloc((sz), MHANDLE_AT))
#define xcalloc(nmemb, sz) (__xcalloc((nmemb), (sz), MHANDLE_AT))
#define xrealloc(x, sz) (__xrealloc((x), (sz), MHANDLE_AT))
#define xstrdup(x) (__xstrdup((x), MHANDLE_AT))

#define mhandle_check()
#define mhandle_done()
#define mhandle_used_memory() (0UL)

#endif



void *mhandle_malloc(size_t size, char *at);
void *mhandle_calloc(size_t nmemb, size_t size, char *at);
void *mhandle_realloc(void *ptr, size_t size, char *at);
char *mhandle_strdup(const char *s, char *at);
void mhandle_free(void *ptr, char *at);

void *__xmalloc(size_t size, char *at);
void *__xcalloc(size_t nmemb, size_t size, char *at);
void *__xrealloc(void *ptr, size_t size, char *at);
void *__xstrdup(const char *s, char *at);

void __mhandle_check(char *at);
void __mhandle_done();
unsigned long __mhandle_used_memory();

#endif

