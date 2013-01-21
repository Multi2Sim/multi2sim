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

#include "matrix.h"
#include "matrix-stack.h"
#include "opengl.h"


struct opengl_matrix_stack_t *opengl_matrix_stack_create(GLenum mode)
{
	struct opengl_matrix_stack_t *stack;
	struct opengl_matrix_t *mtx;

	/* Allocate */
	stack = xcalloc(1, sizeof(struct opengl_matrix_stack_t));
	if(!stack)
		fatal("%s: out of memory", __FUNCTION__);
	opengl_debug("\tCreated Matrix Stack %p\n", stack);

	/* Initialize */
	/* Initially, each of the stacks contains an identity matrix */
	mtx = opengl_matrix_create(MATRIX_IDENTITY);
	stack->stack = list_create();
	list_add(stack->stack, mtx);
	stack->depth = 0;

	switch (mode)
	{

	case GL_MODELVIEW:
	{
		stack->max_depth = MAX_MODELVIEW_STACK_DEPTH;
		break;
	}

	case GL_PROJECTION:
	{
		stack->max_depth = MAX_PROJECTION_STACK_DEPTH;
		break;
	}

	case GL_TEXTURE:
	{
		stack->max_depth = MAX_TEXTURE_STACK_DEPTH;
		break;
	}

	case GL_COLOR:
	{
		stack->max_depth = MAX_COLOR_STACK_DEPTH;
		break;
	}

	default:
		break;
	} 

	/* Return */	
	return stack;
}

void opengl_matrix_stack_free(struct opengl_matrix_stack_t *mtx_stack)
{
	/* Free matrices in the list */
	while (list_count(mtx_stack->stack))
		opengl_matrix_free(list_remove_at(mtx_stack->stack, 0));
	/* Free list */
	list_free(mtx_stack->stack);
	/* Free stack */
	free(mtx_stack);
}

int opengl_matrix_stack_push(struct opengl_matrix_stack_t *mtx_stack, struct opengl_matrix_t *mtx)
{
	int i;
	int j;

	if (mtx_stack->depth == mtx_stack->max_depth)
		fatal("Stack overflow, max depth = %d\n", mtx_stack->max_depth);
	if (mtx == NULL)
		fatal("Invalid Matrix\n");

	/* Debug */
	opengl_debug("\tCurrrent stack %p, depth = %d, max_depth = %d\n", mtx_stack, mtx_stack->depth, mtx_stack->max_depth );
	opengl_debug("\tPushing: mtx = %p, mtx->matrix = %p\n", mtx, mtx->matrix);
	for (i = 0; i < 4; ++i)
	{
		opengl_debug("\t\t");
		for (j = 0; j < 4; ++j)
		{
			opengl_debug("%f\t", mtx->matrix[j*4+i]);
		}
		opengl_debug("\n");
	}

	list_push(mtx_stack->stack, mtx);
	mtx_stack->depth += 1;

	return 0;
}

struct opengl_matrix_t *opengl_matrix_stack_pop(struct opengl_matrix_stack_t *mtx_stack)
{
	/* Variables */
	struct opengl_matrix_t *mtx;
	int i;
	int j;

	/* Pop from stack */
	if (mtx_stack->depth == 0 )
		fatal("Stack underflow, max depth = %d\n", mtx_stack->max_depth);
	mtx = list_pop(mtx_stack->stack);
	if (mtx == NULL)
		fatal("Empty stack!\n");
	/* Debug info */
	opengl_debug("\tCurrrent stack %p, depth = %d, max_depth = %d\n", mtx_stack, mtx_stack->depth, mtx_stack->max_depth );
	opengl_debug("\tPoping: mtx = %p, mtx->matrix = %p\n", mtx, mtx->matrix);
	for (i = 0; i < 4; ++i)
	{
		opengl_debug("\t\t");
		for (j = 0; j < 4; ++j)
		{
			opengl_debug("%f\t", mtx->matrix[j*4+i]);
		}
		opengl_debug("\n");
	}

	opengl_matrix_free(mtx);
	mtx_stack->depth -= 1;

	/* Return */
	return mtx;
}

struct opengl_matrix_t *opengl_matrix_stack_top(struct opengl_matrix_stack_t *mtx_stack)
{
	struct opengl_matrix_t *mtx;
	int i;
	int j;
	
	mtx = list_top(mtx_stack->stack);
	opengl_debug("\t\tMatrix on top of stack: mtx = %p, mtx->matrix = %p\n", mtx, mtx->matrix);
	for (i = 0; i < 4; ++i)
	{
		opengl_debug("\t\t");
		for (j = 0; j < 4; ++j)
		{
			opengl_debug("%f\t", mtx->matrix[j*4+i]);
		}
		opengl_debug("\n");
	}

	return mtx;
}

struct opengl_matrix_t *opengl_matrix_stack_bottom(struct opengl_matrix_stack_t *mtx_stack)
{
	struct opengl_matrix_t *mtx;
	int i;
	int j;
	
	mtx = list_bottom(mtx_stack->stack);
	opengl_debug("\t\tMatrix on bottom of stack: mtx = %p, mtx->matrix = %p\n", mtx, mtx->matrix);
	for (i = 0; i < 4; ++i)
	{
		opengl_debug("\t\t");
		for (j = 0; j < 4; ++j)
		{
			opengl_debug("%f\t", mtx->matrix[j*4+i]);
		}
		opengl_debug("\n");
	}

	return mtx;	
}

