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

#include <GL/glut.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>

#include "frame-buffer.h"
#include "glut.h"



/*
 * GLUT Frame Buffer
 */

struct glut_frame_buffer_t
{
	int *buffer;

	int width;
	int height;

	int flush_request;
};


static struct glut_frame_buffer_t *glut_frame_buffer;


void glut_frame_buffer_init(void)
{
	/* Initialize */
	glut_frame_buffer = xcalloc(1, sizeof(struct glut_frame_buffer_t));
	glut_frame_buffer->flush_request = 1;
}


void glut_frame_buffer_done(void)
{
	if (glut_frame_buffer->buffer)
		free(glut_frame_buffer->buffer);
	free(glut_frame_buffer);
}


void glut_frame_buffer_clear(void)
{
	/* Lock global GLUT mutex */
	pthread_mutex_lock(&glut_mutex);

	/* Clear buffer */
	if (glut_frame_buffer->buffer)
		memset(glut_frame_buffer->buffer, 0, glut_frame_buffer->width *
			glut_frame_buffer->height * sizeof(int));

	/* Unlock global GLUT mutex */
	pthread_mutex_unlock(&glut_mutex);
}


void glut_frame_buffer_resize(int width, int height)
{
	/* Lock global GLUT mutex */
	pthread_mutex_lock(&glut_mutex);

	/* Invalid size */
	if (width < 1 || height < 1)
		fatal("%s: invalid size (width = %d, height = %d)\n",
			__FUNCTION__, width, height);

	/* If same size, just clear it. */
	if (glut_frame_buffer->width == width && glut_frame_buffer->height == height)
	{
		memset(glut_frame_buffer->buffer, 0, glut_frame_buffer->width *
			glut_frame_buffer->height * sizeof(int));
		goto out;
	}

	/* Free previous buffer */
	if (glut_frame_buffer->buffer)
		free(glut_frame_buffer->buffer);

	/* Store new size */
	glut_frame_buffer->buffer = xcalloc(width * height, sizeof(int));
	glut_frame_buffer->width = width;
	glut_frame_buffer->height = height;

out:
	/* Unlock global GLUT mutex */
	pthread_mutex_unlock(&glut_mutex);
}


void glut_frame_buffer_pixel(int x, int y, int color)
{
	/* Lock global GLUT mutex */
	pthread_mutex_lock(&glut_mutex);

	/* Invalid color */
	if ((unsigned int) color > 0xffffff)
	{
		warning("%s: invalid pixel color", __FUNCTION__);
		goto out;
	}

	/* Invalid X coordinate */
	if (!IN_RANGE(x, 0, glut_frame_buffer->width - 1))
	{
		warning("%s: invalid X coordinate", __FUNCTION__);
		goto out;
	}

	/* Invalid Y coordinate */
	if (!IN_RANGE(y, 0, glut_frame_buffer->height - 1))
	{
		warning("%s: invalid Y coordinate", __FUNCTION__);
		goto out;
	}

	/* Set pixel */
	glut_frame_buffer->buffer[y * glut_frame_buffer->width + x] = color;

out:
	/* Unlock global GLUT mutex */
	pthread_mutex_unlock(&glut_mutex);
}


void glut_frame_buffer_get_size(int *width, int *height)
{
	/* Lock global GLUT mutex */
	pthread_mutex_lock(&glut_mutex);

	/* Get variables */
	if (width)
		*width = glut_frame_buffer->width;
	if (height)
		*height = glut_frame_buffer->height;

	/* Unlock global GLUT mutex */
	pthread_mutex_unlock(&glut_mutex);
}


void glut_frame_buffer_flush_request(void)
{
	/* Lock */
	pthread_mutex_lock(&glut_mutex);

	/* Set flush request flag */
	glut_frame_buffer->flush_request = 1;

	/* Unlock */
	pthread_mutex_unlock(&glut_mutex);
}


/* Update the host frame buffer with the contents of the guest frame buffer. This
 * function should be called by the child GLUT thread that created the GLUT window. */
void glut_frame_buffer_flush_if_requested(void)
{
	float red;
	float green;
	float blue;

	int color;

	int width;
	int height;

	int x;
	int y;

	/* Lock global GLUT mutex */
	pthread_mutex_lock(&glut_mutex);

	/* If flush request is not set, nothing to do. */
	if (!glut_frame_buffer->flush_request)
		goto out;

	/* Clear host frame buffer */
	width = glut_frame_buffer->width;
	height = glut_frame_buffer->height;
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(1.0);

	/* Copy guest to host frame buffer */
	glBegin(GL_POINTS);
	for (x = 0; x < width; x++)
	{
		for (y = 0; y < height; y++)
		{
			color = glut_frame_buffer->buffer[y * width + x];
			red = (float) (color >> 16) / 0xff;
			green = (float) ((color >> 8) & 0xff) / 0xff;
			blue = (float) (color & 0xff) / 0xff;
			glColor3f(red, green, blue);
			glVertex2i(x, y);
		}
	}

	/* Flush host frame buffer */
	glEnd();
	glFlush();

	/* Clear flush request flag */
	glut_frame_buffer->flush_request = 0;

out:
	/* Unlock */
	pthread_mutex_unlock(&glut_mutex);
}
