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

#ifndef X86_OPENGL_RAST_TRIANGLE_H
#define X86_OPENGL_RAST_TRIANGLE_H

#include <GL/glut.h>
#include "emu.h"
#include "opengl-rast-config.h"
#include "opengl-span.h"
#include "opengl-edge.h" 
#include "opengl-context.h"

#define MAX_GLUINT	0xffffffff

void x86_opengl_rasterizer_draw_tiangle(struct x86_opengl_context_t *ctx, struct x86_opengl_vertex_t *vtx0, struct x86_opengl_vertex_t *vtx1, struct x86_opengl_vertex_t *vtx2);

#endif
