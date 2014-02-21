/*
 *  Multi2Sim
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

#include <unistd.h>
#include <stdio.h>

#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/mhandle/mhandle.h>
#include "si-db.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */

struct opengl_depth_buffer_t *opengl_depth_buffer_create()
{
	struct opengl_depth_buffer_t *depth;

	/* Allocate */
	depth = xcalloc(1, sizeof(struct opengl_depth_buffer_t));
	depth->depth_func = GL_LESS;

	/* Return */
	return depth;
}

void opengl_depth_buffer_free(struct opengl_depth_buffer_t *db)
{
	if (db->buffer)
		free(db->buffer);
	
	free(db);
}

void opengl_depth_buffer_resize(struct opengl_depth_buffer_t *db, unsigned int width, unsigned int height)
{
	int i;

	/* Invalid size */
	if (width < 1 || height < 1)
		fatal("%s: invalid size (width = %d, height = %d)\n",
			__FUNCTION__, width, height);

	/* If same size, just clear it. */
	if (db->width == width && db->height == height)
	{
		for (i = 0; i < db->width * db->height; ++i)
			db->buffer[i] = 1.0f;
		return;
	}

	/* Free previous buffer */
	if (db->buffer)
		free(db->buffer);

	/* Store new size */
	db->buffer = xcalloc(width * height, sizeof(float));
	for (i = 0; i < width * height; ++i)
		db->buffer[i] = 1.0f;

	db->width = width;
	db->height = height;
}

int opengl_depth_buffer_test_pixel(struct opengl_depth_buffer_t *db, int x, int y, float depth_val, enum gl_depth_func_t depth_func)
{
	int is_pass = 0;
	float depth;

	/* Invalid X coordinate */
	if (!IN_RANGE(x, 0, db->width - 1))
	{
		warning("%s: invalid X coordinate", __FUNCTION__);
		is_pass = 0;
	}

	/* Invalid Y coordinate */
	if (!IN_RANGE(y, 0, db->height - 1))
	{
		warning("%s: invalid Y coordinate", __FUNCTION__);
		is_pass = 0;
	}

	depth = db->buffer[y * db->width + x];

	/* Test pixel */
	switch(depth_func)
	{
	
		case GL_NEVER:
			is_pass = 0;
			break;
		case GL_LESS:
			is_pass = depth_val < depth ? 1 : 0;
			break;
		case GL_EQUAL:
			is_pass = depth_val == depth ? 1 : 0;
			break;
		case GL_LEQUAL:
			is_pass = depth_val <= depth ? 1 : 0;
			break;
		case GL_GREATER:
			is_pass = depth_val > depth ? 1 : 0;
			break;
		case GL_NOTEQUAL:
			is_pass = depth_val != depth ? 1 : 0;
			break;
		case GL_GEQUAL:
			is_pass = depth_val >= depth ? 1 : 0;
			break;
		case GL_ALWAYS:
			is_pass = 1;
			break;
		default:
			is_pass = depth_val < depth ? 1 : 0;
			break;
	}

	/* Return */
	return is_pass;
}

void opengl_depth_buffer_set_pixel(struct opengl_depth_buffer_t *db, int x, int y, float depth_val)
{
	/* Invalid X coordinate */
	if (!IN_RANGE(x, 0, db->width - 1))
	{
		warning("%s: invalid X coordinate", __FUNCTION__);
		return;
	}

	/* Invalid Y coordinate */
	if (!IN_RANGE(y, 0, db->height - 1))
	{
		warning("%s: invalid Y coordinate", __FUNCTION__);
		return;
	}

	db->buffer[y * db->width + x] = depth_val;
}

int opengl_depth_buffer_test_and_set_pixel(struct opengl_depth_buffer_t *db, int x, int y, float depth_val, enum gl_depth_func_t depth_func)
{
	int is_pass;

	/* Invalid X coordinate */
	if (!IN_RANGE(x, 0, db->width - 1))
	{
		warning("%s: invalid X coordinate", __FUNCTION__);
		return 0;
	}

	/* Invalid Y coordinate */
	if (!IN_RANGE(y, 0, db->height - 1))
	{
		warning("%s: invalid Y coordinate", __FUNCTION__);
		return 0;
	}

	is_pass = opengl_depth_buffer_test_pixel(db, x, y, depth_val, depth_func);

	if (is_pass)
		opengl_depth_buffer_set_pixel(db, x, y, depth_val);

	return is_pass;
}

void opengl_depth_buffer_clear(struct opengl_depth_buffer_t *db)
{
	int i;
	
	if (db->buffer)
		for (i = 0; i < db->width * db->height; ++i)
			db->buffer[i] = 1.0f;

}
