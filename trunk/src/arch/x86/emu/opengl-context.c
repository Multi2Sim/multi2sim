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

#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "opengl-context.h"


struct x86_opengl_context_capability_t *x86_opengl_context_capability_create(void)
{
	/* Variables */
 	struct x86_opengl_context_capability_t* cap;

 	cap = calloc(1, sizeof(struct x86_opengl_context_capability_t));
	if (!cap)
		fatal("%s: out of memory", __FUNCTION__);

	/* Set up initial value for each capability, initial value for each capability is GL_FALSE, except GL_DITHER and GL_MULTISAMPLE */
	memset(cap, 0, sizeof(struct x86_opengl_context_capability_t));
	cap->is_dither = GL_TRUE;
	cap->is_multisample = GL_TRUE;

	/* Return */
	return cap;
}

void x86_opengl_context_capability_free(struct x86_opengl_context_capability_t *cap)
{
	free(cap);
}

struct x86_opengl_context_t *x86_opengl_context_create(void)
{
	/* Variables */
	int width;
	int height;
	int i;

	/* Allocate */
	struct x86_opengl_context_t *ctx;
	ctx = calloc(1, sizeof(struct x86_opengl_context_t));
	if(!ctx)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize frame buffers */
	width = 0;  // FIXME
	height = 0;  // FIXME
	ctx->draw_buffer = x86_opengl_frame_buffer_create(width, height);
	ctx->read_buffer = x86_opengl_frame_buffer_create(width, height);
	
	/* Initialize context capabilities */
	ctx->context_cap = x86_opengl_context_capability_create();

	/* Initialize viewport */
	ctx->viewport = x86_opengl_viewport_create();

	/* Initialize matrix stack */
	ctx->modelview_matrix_stack = x86_opengl_matrix_stack_create(GL_MODELVIEW);
	ctx->projection_matrix_stack = x86_opengl_matrix_stack_create(GL_PROJECTION);
	for (i = 0; i < MAX_TEXTURE_STACK_DEPTH; i++)
	{
		ctx->texture_matrix_stack[i] = x86_opengl_matrix_stack_create(GL_TEXTURE);
	}
	ctx->color_matrix_stack = x86_opengl_matrix_stack_create(GL_COLOR);
	
	/* FIXME: which one is the default current stack ? */
	ctx->current_matrix_stack = ctx->modelview_matrix_stack;

	/* Initialize vertex buffer */
	ctx->vertex_buffer = x86_opengl_vertex_buffer_create();

	/* Initialize current color */
	GLfloat init_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	x86_opengl_clamped_float_to_color_channel(init_color, ctx->current_color);

	/* Return */
	return ctx;
}

void x86_opengl_context_free(struct x86_opengl_context_t *ctx)
{
	/* Variables */
	int i;

	/* Free context capabilities*/
	x86_opengl_context_capability_free(ctx->context_cap);

	/* Free framebuffers */
	x86_opengl_frame_buffer_free(ctx->draw_buffer);
	x86_opengl_frame_buffer_free(ctx->read_buffer);

	/* Free viewport */
	x86_opengl_viewport_free(ctx->viewport);

	/* Free matrix stacks */
	x86_opengl_matrix_stack_free(ctx->modelview_matrix_stack);
	x86_opengl_matrix_stack_free(ctx->projection_matrix_stack);
	for (i = 0; i < MAX_TEXTURE_STACK_DEPTH; i++)
	{
		x86_opengl_matrix_stack_free(ctx->texture_matrix_stack[i]);	
	}
	x86_opengl_matrix_stack_free(ctx->color_matrix_stack);

	/* Free vertex buffer */
	x86_opengl_vertex_buffer_free(ctx->vertex_buffer);

	free(ctx);
}

struct x86_opengl_matrix_t *x86_opengl_context_get_current_matrix(struct x86_opengl_context_t *ctx)
{
	/* Variables */
	struct x86_opengl_matrix_t *mtx;

	/* Get current matrix */
	mtx = list_get(ctx->current_matrix_stack->stack, ctx->current_matrix_stack->depth);

	/* Return */
	return mtx;
}
