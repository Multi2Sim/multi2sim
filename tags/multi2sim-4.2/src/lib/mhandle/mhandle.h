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

#ifndef LIB_MHANDLE_MHANDLE_H
#define LIB_MHANDLE_MHANDLE_H

#include <stdlib.h>
#include <string.h>

#define MHANDLE_STRINGIFY(x) #x
#define MHANDLE_TOSTRING(x) MHANDLE_STRINGIFY(x)
#define MHANDLE_AT __FILE__ ":" MHANDLE_TOSTRING(__LINE__)


#undef strdup
#define malloc(sz) __ERROR_USE_XMALLOC_INSTEAD__
#define calloc(nmemb, sz) __ERROR_USE_XCALLOC_INSTEAD__
#define realloc(x, sz) __ERROR_USE_XREALLOC_INSTEAD__
#define strdup(x) __ERROR_USE_XSTRDUP_INSTEAD__


#ifdef MHANDLE

#define xmalloc(sz) (mhandle_malloc((sz), MHANDLE_AT))
#define xcalloc(nmemb, sz) (mhandle_calloc((nmemb), (sz), MHANDLE_AT))
#define xrealloc(x, sz) (mhandle_realloc((x), (sz), MHANDLE_AT))
#define xstrdup(x) (mhandle_strdup((x), MHANDLE_AT))
#define free(x) (mhandle_free((x), MHANDLE_AT))

#define mhandle_check() __mhandle_check(MHANDLE_AT)
#define mhandle_done() __mhandle_done()
#define mhandle_used_memory() __mhandle_used_memory()
#define mhandle_register_ptr(ptr, size) __mhandle_register_ptr((ptr), (size), MHANDLE_AT)

#else

#define xmalloc(sz) (__xmalloc((sz), MHANDLE_AT))
#define xcalloc(nmemb, sz) (__xcalloc((nmemb), (sz), MHANDLE_AT))
#define xrealloc(x, sz) (__xrealloc((x), (sz), MHANDLE_AT))
#define xstrdup(x) (__xstrdup((x), MHANDLE_AT))

#define mhandle_check()
#define mhandle_done()
#define mhandle_used_memory() (0UL)
#define mhandle_register_ptr(ptr, size)

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

/* When memory has been allocated with a function other than those above, the
 * pointer can be registered using 'mhandle_register_ptr' to prevent function
 * 'free' from reporting invalid pointer errors. This pointer will not have the
 * extra corruption bytes. */
void __mhandle_register_ptr(void *ptr, unsigned long size, char *at);

#endif

