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
#include <lib/util/list.h>

#include "opengl.h"
#include "vertex.h"


struct x86_opengl_vertex_t *x86_opengl_vertex_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	struct x86_opengl_vertex_t *vtx;

	vtx = xcalloc(1, sizeof(struct x86_opengl_vertex_t));

	/* Initialize */
	vtx->pos[X_COMP] = x;
	vtx->pos[Y_COMP] = y;
	vtx->pos[Z_COMP] = z;
	vtx->pos[W_COMP] = w;

	x86_opengl_debug("\t\tCreate vertex \t[%f, %f, %f, %f]\n", vtx->pos[X_COMP], vtx->pos[Y_COMP], vtx->pos[Z_COMP], vtx->pos[W_COMP]);

	return vtx;
}
void x86_opengl_vertex_free(struct x86_opengl_vertex_t *vtx)
{
	x86_opengl_debug("\t\tFree vertex \t[%f, %f, %f, %f]\n", vtx->pos[X_COMP], vtx->pos[Y_COMP], vtx->pos[Z_COMP], vtx->pos[W_COMP]);

	free(vtx);
}

void x86_opengl_vertex_set_color(GLchan *color, struct x86_opengl_vertex_t *vtx)
{
	int i;
	for (i = 0; i < 4; ++i)
	{
		vtx->color[i] = color[i];
	}
}

int x86_opengl_vertex_get_color(struct x86_opengl_vertex_t *vtx)
{
	return ((vtx->color[0]) << 16) + ((vtx->color[1]) << 8) + vtx->color[2];
}

void x86_opengl_vertex_set_normal(GLfloat *nrml, struct x86_opengl_vertex_t *vtx)
{
	int i;
	for (i = 0; i < 4; ++i)
	{
		vtx->normal[i] = nrml[i];
	}	
}

struct x86_opengl_vertex_group_t *x86_opengl_vertex_group_create(GLenum primitive_type)
{
	struct x86_opengl_vertex_group_t * vtxgp;

	vtxgp = xcalloc(1, sizeof(struct x86_opengl_vertex_group_t));

	/* Initialize */
	vtxgp->primitive_type = primitive_type;
	vtxgp->vertex_list = list_create();

	x86_opengl_debug("\t\tPrimitive group type %d\n", primitive_type);

	/* Return */	
	return vtxgp;
}

void x86_opengl_vertex_group_free(struct x86_opengl_vertex_group_t *vtxgp)
{
	/* Free vertices in the list */
	while (list_count(vtxgp->vertex_list))
		x86_opengl_vertex_free(list_remove_at(vtxgp->vertex_list, 0));	

	list_free(vtxgp->vertex_list);
	free(vtxgp);
}

static void x86_opengl_vertex_group_add_vertex(struct x86_opengl_vertex_group_t *vtxgp, struct x86_opengl_vertex_t *vtx)
{
	list_add(vtxgp->vertex_list, vtx);
}

struct x86_opengl_vertex_buffer_t *x86_opengl_vertex_buffer_create()
{
	struct x86_opengl_vertex_buffer_t *vtxbf;

	vtxbf = xcalloc(1, sizeof(struct x86_opengl_vertex_buffer_t));

	/* Initialize */
	vtxbf->vertex_groups = list_create();
	vtxbf->current_vertex_group = list_tail(vtxbf->vertex_groups);

	/* Return */
	return vtxbf;
}

void x86_opengl_vertex_buffer_free(struct x86_opengl_vertex_buffer_t *vtxbf)
{
	if (vtxbf)
	{
		while (list_count(vtxbf->vertex_groups))
			x86_opengl_vertex_group_free(list_remove_at(vtxbf->vertex_groups, 0));

		list_free(vtxbf->vertex_groups);
		free(vtxbf);
	}
}

void x86_opengl_vertex_buffer_add_vertex_group(struct x86_opengl_vertex_buffer_t *vtxbf, struct x86_opengl_vertex_group_t *vtxgp)
{
	list_add(vtxbf->vertex_groups, vtxgp);
	vtxbf->current_vertex_group = vtxgp;
}

void x86_opengl_vertex_buffer_add_vertex(struct x86_opengl_vertex_buffer_t *vtxbf, struct x86_opengl_vertex_t *vtx)
{
	x86_opengl_debug("\t\tAdd vertex \t[%f, %f, %f, %f]\n", vtx->pos[X_COMP], vtx->pos[Y_COMP], vtx->pos[Z_COMP], vtx->pos[W_COMP]);

	x86_opengl_vertex_group_add_vertex(vtxbf->current_vertex_group, vtx);
}
