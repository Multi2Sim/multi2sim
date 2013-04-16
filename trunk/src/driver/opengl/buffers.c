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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "buffers.h"
#include "opengl.h"


struct opengl_render_buffer_t *opengl_render_buffer_create(int width, int height)
{
	struct opengl_render_buffer_t *rb;

	/* Allocate */
	rb = xcalloc(1, sizeof(struct opengl_render_buffer_t));

	/* Initialize */
	rb->width = width;
	rb->height = height;
	rb->buffer = xcalloc(1, width * height * sizeof(GLuint));

	/* Return */	
	return rb;
}

void opengl_render_buffer_free(struct opengl_render_buffer_t *rb)
{
	free(rb->buffer);
	free(rb);
}

void opengl_render_buffer_clear(struct opengl_render_buffer_t *rb, int clear_value)
{
	if(rb)
		memset(rb->buffer, clear_value, rb->width * rb->height * sizeof(GLuint));
}

int opengl_render_buffer_resize(struct opengl_render_buffer_t *rb, int width, int height)
{
	/* Invalid size */
	if (width < 1 || height < 1)
		fatal("%s: invalid size (width = %d, height = %d)\n",
			__FUNCTION__, width, height);

	/* If same size, just clear it. */
	if (rb->width == width && rb->height == height)
	{
		/* FIXME, currently set value == 1 */
		opengl_render_buffer_clear(rb, 0);
		return 0;
	}

	opengl_debug("\t\tBuffer resized, W x H = %d x %d\n", width, height);

	/* Free previous buffer */
	if (rb->buffer)
		free(rb->buffer);

	/* Allocate new buffer */
	rb->buffer = xcalloc(width * height, sizeof(GLuint));

	/* Store new size */
	rb->width = width;
	rb->height = height;

	/* Return */
	return 0;	
}

struct opengl_frame_buffer_t *opengl_frame_buffer_create(int width, int height)
{
	/* Variables */
	int i;
	struct opengl_frame_buffer_t *fb;

	/* Allocate */
	fb = xcalloc(1, sizeof(struct opengl_frame_buffer_t));

	/* Initialization */
	fb->width = width;
	fb->height = height;
	for (i = 0; i < COLOR_BUFFER_COUNT; ++i)
	{
		fb->color_buffer[i] = opengl_render_buffer_create(width, height);
	}
	fb->depth_buffer = opengl_render_buffer_create(width, height);
	fb->stencil_buffer = opengl_render_buffer_create(width, height);

	/* Return */
	return fb;
}

void opengl_frame_buffer_free(struct opengl_frame_buffer_t *fb)
{
	int i;
	for (i = 0; i < COLOR_BUFFER_COUNT; ++i)
	{
		opengl_render_buffer_free(fb->color_buffer[i]);
	}
	opengl_render_buffer_free(fb->depth_buffer);
	opengl_render_buffer_free(fb->stencil_buffer);

	free(fb);
}

void opengl_frame_buffer_clear(struct opengl_frame_buffer_t *fb, GLbitfield mask)
{
	int i;
	int clear_value;

	/* Get current set value */
	/* FIXME */
	clear_value = 0;
	/* Clear buffer */
	if (fb)
		/* Clear buffers */
		if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT))
			opengl_debug("\t\tInvalid mask!\n");

		if ((mask & GL_COLOR_BUFFER_BIT) == GL_COLOR_BUFFER_BIT) 
		{
			opengl_debug("\t\tColor buffer cleared to %d\n", clear_value);
			for (i = 0; i < COLOR_BUFFER_COUNT; ++i)
				opengl_render_buffer_clear(fb->color_buffer[i], clear_value);
		}

		if ((mask & GL_DEPTH_BUFFER_BIT) == GL_DEPTH_BUFFER_BIT) 
		{
			opengl_debug("\t\tDepth buffer cleared to %d\n", clear_value);
			opengl_render_buffer_clear(fb->depth_buffer, clear_value);
		}

		if ((mask & GL_STENCIL_BUFFER_BIT) == GL_STENCIL_BUFFER_BIT) {
			opengl_debug("\t\tStencil buffer cleared to %d\n", clear_value);
			opengl_render_buffer_clear(fb->stencil_buffer, clear_value);
		}
		
}

int opengl_frame_buffer_resize(struct opengl_frame_buffer_t *fb, int width, int height)
{
	int i;
	/* Invalid size */
	if (width < 1 || height < 1)
		fatal("%s: invalid size (width = %d, height = %d)\n",
			__FUNCTION__, width, height);

	/* If same size, just clear it. */
	if (fb->width == width && fb->height == height)
	{
		/* FIXME */
		opengl_frame_buffer_clear(fb, 0);
		return 0;
	}	

	/* Resize buffers */
	for (i = 0; i < COLOR_BUFFER_COUNT; ++i)
	{
		opengl_render_buffer_resize(fb->color_buffer[i], width, height);
	}
	opengl_render_buffer_resize(fb->depth_buffer, width, height);
	opengl_render_buffer_resize(fb->stencil_buffer, width, height);

	/* Store new size */
	fb->width = width;
	fb->height = height;

	/* Return */
	return 0;
}
