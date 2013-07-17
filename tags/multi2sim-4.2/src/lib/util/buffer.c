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


#include <lib/mhandle/mhandle.h>

#include "buffer.h"


#define MIN(X,Y) ((X)<(Y)?(X):(Y))
#define MAX(X,Y) ((X)>(Y)?(X):(Y))

struct buffer_t {
	void *data;
	int count, size;
	int head, tail;
};




/* Private Functions */

/* grow buffer */
static int grow(struct buffer_t *buffer, int size)
{
	void *data;
	
	/* allocate new memory */
	if (size < buffer->size)
		return 0;
	data = xcalloc(1, size);
	
	/* copy buffer contents */
	memcpy(data, buffer->data + buffer->head, buffer->size - buffer->head);
	memcpy(data + buffer->size - buffer->head, buffer->data, buffer->head);
	
	/* assign new fields */
	free(buffer->data);
	buffer->data = data;
	buffer->size = size;
	buffer->head = 0;
	buffer->tail = buffer->count;
	return 1;
}




/* Public Functions */

/* creation and destruction */
struct buffer_t *buffer_create(int size)
{
	struct buffer_t *buffer;
	buffer = xcalloc(1, sizeof(struct buffer_t));
	buffer->size = size;
	buffer->data = xcalloc(1, size);
	return buffer;
}


void buffer_free(struct buffer_t *buffer)
{
	free(buffer->data);
	free(buffer);
}


/* read */
int buffer_read(struct buffer_t *buffer, void *dest, int size)
{
	int right, left;
	
	/* compute bytes that will be read at the right/left side */
	size = MIN(size, buffer->count);
	right = MIN(buffer->size - buffer->head, size);
	left = size - right;
	
	/* read bytes */
	memcpy(dest, buffer->data + buffer->head, right);
	memcpy(dest + right, buffer->data, left);
	buffer->head = (buffer->head + size) % buffer->size;
	buffer->count -= size;
	
	return size;
}


/* write */
int buffer_write(struct buffer_t *buffer, void *dest, int size)
{
	int right, left;
	
	/* grow buffer */
	while (buffer->count + size > buffer->size)
		if (!grow(buffer, MAX(buffer->count + size, buffer->size * 2)))
			return 0;
	
	/* write */
	right = MIN(buffer->size - buffer->tail, size);
	left = size - right;
	memcpy(buffer->data + buffer->tail, dest, right);
	memcpy(buffer->data, dest + right, left);
	buffer->tail = (buffer->tail + size) % buffer->size;
	buffer->count += size;
	
	return size;
}


/* count */
int buffer_count(struct buffer_t *buffer)
{
	return buffer->count;
}
