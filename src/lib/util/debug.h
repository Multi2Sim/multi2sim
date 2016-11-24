/*
 *  Libstruct
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

#ifndef LIB_UTIL_DEBUG_H
#define LIB_UTIL_DEBUG_H

#include <stdio.h>
#include <stdlib.h>


/* Initialization and finalization */
void debug_init(void);
void debug_done(void);

/* Create new category for debugging, which will be passed as first parameter
 * to the 'debug' function. If a file name is given, an implicit call to
 * 'debug_assign_file' is performed. */
int debug_new_category(char *filename);

/* Switch the status of a debugging category. By default, the
 * debugging messages are on, while there is an opened file to be dumped into. */
#define debug_on(category) ((category) ? __debug_on((category)) : (void) 0)
#define debug_off(category) ((category) ? __debug_off((category)) : (void) 0)
void __debug_on(int category);
void __debug_off(int category);

/* Return true if a message to this debug category would be dumped, that is,
 * if debug is on and the file is not NULL. */
#define debug_status(category) ((category) ? __debug_status((category)) : 0)
int __debug_status(int category);

/* Return the file associated with a category. */
#define debug_file(category) ((category) ? __debug_file((category)) : 0)
FILE *__debug_file(int category);

/* Flush associated file */
#define debug_flush(category) ((category) ? __debug_flush((category)) : (void) 0)
void __debug_flush(int category);

/* Set spaces for next debug messages */
#define debug_tab(category) ((category) ? __debug_tab((category)) : (void) 0)
#define debug_tab_inc(category) ((category) ? __debug_tab_inc((category)) : (void) 0)
#define debug_tab_dec(category) ((category) ? __debug_tab_dec((category)) : (void) 0)
void __debug_tab(int category, int space_count);
void __debug_tab_inc(int category, int space_count);
void __debug_tab_dec(int category, int space_count);

/* Dump a debugging message. */
#define debug(category, ...) ((category) ? __debug((category), __VA_ARGS__) : (void) 0)
void __debug(int category, char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

/* Dump a buffer */
#define debug_buffer(category, buffer_name, buffer, size) \
	((category) ? __debug_buffer((category), (buffer_name), (buffer), (size)) : (void) 0)
void __debug_buffer(int category, char *buffer_name, void *buffer, int size);

/* Other messages */
void warning(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void fatal(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void panic(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));


#endif

