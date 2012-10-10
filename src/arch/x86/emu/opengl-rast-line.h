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

#ifndef X86_OPENGL_RAST_LINE_H
#define X86_OPENGL_RAST_LINE_H

#include <GL/glut.h>
#include "emu.h"
#include "glut-frame-buffer.h"
#include "opengl-rast-config.h"
#include "opengl-span.h"
#include "opengl-edge.h"
#include "opengl-context.h"

/* Bresenham's line algorithm */
void x86_opengl_rasterizer_draw_line(struct x86_opengl_context_t *ctx, GLint x1, GLint y1, GLint x2, GLint y2, GLuint color);

#endif

