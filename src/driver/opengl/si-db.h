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

#ifndef DRIVER_OPENGL_SI_DEPTH_BUFFER_H
#define DRIVER_OPENGL_SI_DEPTH_BUFFER_H

enum gl_depth_func_t
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

struct opengl_depth_buffer_t
{
	float *buffer;
	int width;
	int height;
	enum gl_depth_func_t depth_func;
};


struct opengl_depth_buffer_t *opengl_depth_buffer_create();
void opengl_depth_buffer_free(struct opengl_depth_buffer_t *db);

int opengl_depth_buffer_test_pixel(struct opengl_depth_buffer_t *db, int x, int y, float depth_val, enum gl_depth_func_t depth_func);
void opengl_depth_buffer_set_pixel(struct opengl_depth_buffer_t *db, int x, int y, float depth_val);
int opengl_depth_buffer_test_and_set_pixel(struct opengl_depth_buffer_t *db, int x, int y, float depth_val, enum gl_depth_func_t depth_func);

void opengl_depth_buffer_resize(struct opengl_depth_buffer_t *db, unsigned int width, unsigned int height);
void opengl_depth_buffer_clear(struct opengl_depth_buffer_t *db);

#endif
