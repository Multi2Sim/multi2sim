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
 
#ifndef DRIVER_OPENGL_BUFFERS_H
#define DRIVER_OPENGL_BUFFERS_H


#include <GL/glut.h>

#define COLOR_BUFFER_COUNT 4

/* OpenGL render buffer */
struct x86_opengl_render_buffer_t
{
	GLuint width;
	GLuint height;
	GLuint *buffer;
};

/* OpenGL frame buffer 
 * a frame buffer contains a set of render buffers, typically:
 * 4 color buffers: front left & front right, back left & back right
 * 1 depth buffer
 * 1 stencil buffer
 */
struct x86_opengl_frame_buffer_t
{
	GLsizei width;
	GLsizei height;
	struct x86_opengl_render_buffer_t *color_buffer[COLOR_BUFFER_COUNT];
	struct x86_opengl_render_buffer_t *depth_buffer;
	struct x86_opengl_render_buffer_t *stencil_buffer;
};

/* OpenGL render buffer */
struct x86_opengl_render_buffer_t *x86_opengl_render_buffer_create(int width, int height);
void x86_opengl_render_buffer_free(struct x86_opengl_render_buffer_t *rb);
void x86_opengl_render_buffer_clear(struct x86_opengl_render_buffer_t *rb, int clear_value);
int x86_opengl_render_buffer_resize(struct x86_opengl_render_buffer_t *rb, int width, int height);

/* OpenGL frame buffer */
struct x86_opengl_frame_buffer_t *x86_opengl_frame_buffer_create(int width, int height);
void x86_opengl_frame_buffer_free(struct x86_opengl_frame_buffer_t *fb);
void x86_opengl_frame_buffer_clear(struct x86_opengl_frame_buffer_t *fb, GLbitfield mask);
int x86_opengl_frame_buffer_resize(struct x86_opengl_frame_buffer_t *fb, int width, int height);


#endif
