/*
 *  Libstruct
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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>


/* Initialization and finalization */
void debug_init(void);
void debug_done(void);

/* Create new category for debugging, which will be passed as first parameter
 * to the 'debug' function. */
int debug_new_category(void);

/* Assign a file to a category. The file is opened with "wt" flags. On error,
 * the function returns 0, otherwise, non-0. This file is closed automatically
 * when calling debug_done, in case it's not "stdout" or "stderr". */
FILE *debug_assign_file(int category, char *filename);

/* Switch the status of a debugging category. By default, the
 * debugging messages are on, while there is an opened file to be dumped into. */
void debug_on(int category);
void debug_off(int category);

/* Return true if a message to this debug category would be dumped, that is,
 * if debug is on and the file is not NULL. */
int debug_status(int category);

/* Return the file associated with a category. */
FILE *debug_file(int category);

/* Flush associated file */
void debug_flush(int category);

/* Set spaces for next debug messages */
void debug_tab(int category, int space_count);
void debug_tab_inc(int category, int space_count);
void debug_tab_dec(int category, int space_count);

/* Dump a debugging message. */
void debug(int category, char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

/* Other messages */
void warning(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void fatal(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void panic(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

/* Dump formatted string into a buffer with a specific size. Its size is then
 * decreased, and the buffer is advanced to the end of the dumped string.
 * This function is useful for being used in other functions that dump
 * several strings into a buffer, with the header
 *   obj_dump(struct obj_t *obj, char *buf, int size); */
void dump_buf(char **pbuf, int *psize, char *fmt, ...) __attribute__ ((format (printf, 3, 4)));


#endif
