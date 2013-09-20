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

#ifndef LIB_CPP_MISC_H
#define LIB_CPP_MISC_H

namespace Misc
{


/*
 * Inline bit manipulation
 */

inline unsigned int GetBit(unsigned int x, int low, int high)
{
	return (x >> low) & ((1u << (high - low + 1)) - 1);
}



/*
 * Deprecated string manipulation
 */

/* Dump formatted string into a buffer with a specific size. Its size is then
 * decreased, and the buffer is advanced to the end of the dumped string.
 * This function is useful for being used in other functions that dump
 * several strings into a buffer, with the header
 *   obj_dump(struct obj_t *obj, char *buf, int size); */
void str_printf(char **pbuf, int *psize, const char *fmt, ...)
		__attribute__ ((format (printf, 3, 4)));




/*
 * Output messages
 */

void warning(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void fatal(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void panic(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));



} /* namespace Misc */

#endif

