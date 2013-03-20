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
#include "buffers.h"
#include "buffer-obj.h"
#include "context.h"
#include "light.h"
#include "matrix-stack.h"
#include "program.h"
#include "shader.h"
#include "transform-feedback.h"
#include "vertex.h"
#include "vertex-array.h"
#include "viewport.h"


struct opengl_context_capability_t *opengl_context_capability_create(void)
{
	/* Variables */
 	struct opengl_context_capability_t* cap;

 	/* Allocate */
 	cap = xcalloc(1, sizeof(struct opengl_context_capability_t));

	/* Set up initial value for each capability, only GL_DITHER and GL_MULTISAMPLE are GL_TRUE */
	memset(cap, 0, sizeof(struct opengl_context_capability_t));
	cap->is_dither = GL_TRUE;
	cap->is_multisample = GL_TRUE;

	/* Return */
	return cap;
}

void opengl_context_capability_free(struct opengl_context_capability_t *cap)
{
	/* Free */
	free(cap);
}

struct opengl_context_t *opengl_context_create(void)
{
	/* Variables */
	int width;
	int height;
	int i;

	/* Allocate */
	struct opengl_context_t *ctx;
	ctx = xcalloc(1, sizeof(struct opengl_context_t));

	/* Initialize frame buffers */
	width = 0;  
	height = 0;  
	ctx->draw_buffer = opengl_frame_buffer_create(width, height);
	ctx->read_buffer = opengl_frame_buffer_create(width, height);
	
	/* Initialize context capabilities */
	ctx->context_cap = opengl_context_capability_create();

	/* Initialize viewport */
	ctx->viewport = opengl_viewport_create();

	/* Initialize matrix stack */
	ctx->modelview_matrix_stack = opengl_matrix_stack_create(GL_MODELVIEW);
	ctx->projection_matrix_stack = opengl_matrix_stack_create(GL_PROJECTION);
	for (i = 0; i < MAX_TEXTURE_STACK_DEPTH; i++)
		ctx->texture_matrix_stack[i] = opengl_matrix_stack_create(GL_TEXTURE);
	ctx->color_matrix_stack = opengl_matrix_stack_create(GL_COLOR);
	
	/* FIXME: which one is the default current stack ? */
	ctx->current_matrix_stack = ctx->modelview_matrix_stack;

	/* Initialize vertex buffer */
	ctx->vertex_buffer = opengl_vertex_buffer_create();

	/* Initialize light */
	ctx->light = opengl_light_attrib_create();

	/* Initialize shader objects repository */
	ctx->shader_repo = opengl_shader_repo_create();

	/* Initialize program objects repository */
	ctx->program_repo = opengl_program_repo_create();

	/* Initialize VAO repository */
	ctx->vao_repo = opengl_vertex_array_obj_repo_create();

	/* Initialize Buffer objects repository */
	ctx->buf_repo = opengl_buffer_obj_repo_create();

	/* Initialize current color */
	GLfloat init_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	opengl_clamped_float_to_color_channel(init_color, ctx->current_color);

	/* Initialize current normal */
	ctx->current_normal[X_COMP] = 0.0f;
	ctx->current_normal[Y_COMP] = 0.0f;
	ctx->current_normal[Z_COMP] = 1.0f;
	ctx->current_normal[W_COMP] = 0.0f;

	/* Initialize VAO/VBO binding point */
	ctx->array_attrib = opengl_vertex_array_attrib_create();

	/* Transform feedback binding point */
	ctx->transform_feedback = opengl_transform_feedback_state_create();

	/* Return */
	return ctx;
}

void opengl_context_free(struct opengl_context_t *ctx)
{
	/* Variables */
	int i;

	/* Free context capabilities*/
	opengl_context_capability_free(ctx->context_cap);

	/* Free framebuffers */
	opengl_frame_buffer_free(ctx->draw_buffer);
	opengl_frame_buffer_free(ctx->read_buffer);

	/* Free viewport */
	opengl_viewport_free(ctx->viewport);

	/* Free matrix stacks */
	opengl_matrix_stack_free(ctx->modelview_matrix_stack);
	opengl_matrix_stack_free(ctx->projection_matrix_stack);
	for (i = 0; i < MAX_TEXTURE_STACK_DEPTH; i++)
	{
		opengl_matrix_stack_free(ctx->texture_matrix_stack[i]);	
	}
	opengl_matrix_stack_free(ctx->color_matrix_stack);

	/* Free vertex buffer */
	opengl_vertex_buffer_free(ctx->vertex_buffer);

	/* Free light */
	opengl_light_attrib_free(ctx->light);

	/* Free program objects repository*/
	opengl_program_repo_free(ctx->program_repo);

	/* Free shader objects repository */
	opengl_shader_repo_free(ctx->shader_repo);

	/* Free VAO repository */
	opengl_vertex_array_obj_repo_free(ctx->vao_repo);

	/* Free Buffer Object repository */
	opengl_buffer_obj_repo_free(ctx->buf_repo);

	/* Free VAO/VBO binding points */
	opengl_vertex_array_attrib_free(ctx->array_attrib);

	/* Free Transform Feedback binding point */
	opengl_transform_feedback_state_free(ctx->transform_feedback);

	free(ctx);
}

struct opengl_matrix_t *opengl_context_get_current_matrix(struct opengl_context_t *ctx)
{
	/* Variables */
	struct opengl_matrix_t *mtx;

	/* Get current matrix */
	mtx = list_get(ctx->current_matrix_stack->stack, ctx->current_matrix_stack->depth);
	opengl_debug("\t\tCurrent matrix ptr = %p\n", mtx);
	
	/* Return */
	return mtx;
}

struct opengl_current_attrib_t *opengl_current_attrib_create()
{
	struct opengl_current_attrib_t *crnt;

	crnt = xcalloc(1, sizeof(struct opengl_current_attrib_t));

	return crnt;
}

void opengl_current_attrib_free(struct opengl_current_attrib_t *crnt)
{
	free(crnt);
}

struct opengl_buffer_obj_t *opengl_context_get_bound_buffer(unsigned int target, struct opengl_context_t *ctx)
{
	struct opengl_buffer_obj_t *buf_obj;

	switch(target)
	{
	case GL_ARRAY_BUFFER:
	{
		buf_obj = ctx->array_attrib->curr_vbo;
		break;
	}
#ifdef GL_ATOMIC_COUNTER_BUFFER
	case GL_ATOMIC_COUNTER_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
#endif
	case GL_COPY_READ_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
	case GL_COPY_WRITE_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
	case GL_DRAW_INDIRECT_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
#ifdef GL_DISPATCH_INDIRECT_BUFFER
	case GL_DISPATCH_INDIRECT_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
#endif
	case GL_ELEMENT_ARRAY_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
	case GL_PIXEL_PACK_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
	case GL_PIXEL_UNPACK_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
#ifdef GL_SHADER_STORAGE_BUFFER
	case GL_SHADER_STORAGE_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
#endif
	case GL_TEXTURE_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
	case GL_TRANSFORM_FEEDBACK_BUFFER:
	{
		buf_obj = ctx->transform_feedback->curr_buf;
		break;
	}
	case GL_UNIFORM_BUFFER:
	{
		buf_obj = NULL;
		break;
	}
	default:
		break;
	}

	if (!buf_obj)
	{
		opengl_debug("\t\tNo Buffer Object attached to binding point!\n");
		return NULL;		
	}
	else
	{
		opengl_debug("\t\tFind Buffer Object #%d [%p] on binding point\n", buf_obj->id, buf_obj);
		return buf_obj;
	}
}
