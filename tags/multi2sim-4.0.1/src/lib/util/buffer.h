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

#ifndef LIB_UTIL_BUFFER_H
#define LIB_UTIL_BUFFER_H

struct buffer_t;

/* creation and destruction */
struct buffer_t *buffer_create(int size);
void buffer_free(struct buffer_t *buffer);

/* read/write */
int buffer_read(struct buffer_t *buffer, void *dest, int size);
int buffer_write(struct buffer_t *buffer, void *src, int size);
int buffer_count(struct buffer_t *buffer);

#endif
