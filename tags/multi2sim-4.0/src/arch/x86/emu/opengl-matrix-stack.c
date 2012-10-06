#include <opengl-matrix-stack.h>

struct x86_opengl_matrix_stack_t *x86_opengl_matrix_stack_create(GLenum mode)
{
	struct x86_opengl_matrix_stack_t *stack;
	struct x86_opengl_matrix_t *mtx;

	/* Allocate */
	stack = calloc(1, sizeof(struct x86_opengl_matrix_stack_t));
	if(!stack)
		fatal("%s: out of memory", __FUNCTION__);

	mtx = x86_opengl_matrix_create(MATRIX_GENERAL);

	/* Initialize */
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

void x86_opengl_matrix_stack_free(struct x86_opengl_matrix_stack_t *mtx_stack)
{
	/* Free matrices in the list */
	while (list_count(mtx_stack->stack))
		x86_opengl_matrix_free(list_remove_at(mtx_stack->stack, 0));
	/* Free list */
	list_free(mtx_stack->stack);
	/* Free stack */
	free(mtx_stack);
}

int x86_opengl_matrix_stack_push(struct x86_opengl_matrix_stack_t *mtx_stack, struct x86_opengl_matrix_t *mtx)
{
	if (mtx_stack->depth == mtx_stack->max_depth)
		fatal("Stack overflow, max depth = %d\n", mtx_stack->max_depth);
	x86_opengl_debug("\tpush: curr_depth = %d, max_depth = %d\n", 
		mtx_stack->depth, mtx_stack->max_depth);
	mtx_stack->depth += 1;
	list_push(mtx_stack->stack, mtx);
	return 0;
}

struct x86_opengl_matrix_t *x86_opengl_matrix_stack_pop(struct x86_opengl_matrix_stack_t *mtx_stack)
{
	/* Variables */
	struct x86_opengl_matrix_t *mtx;

	/* Pop from stack */
	if (mtx_stack->depth == 0 )
		fatal("Stack underflow, max depth = %d\n", mtx_stack->max_depth);
	x86_opengl_debug("\tpop: curr_depth = %d, max_depth = %d\n", 
		mtx_stack->depth, mtx_stack->max_depth);

	mtx_stack->depth -= 1;
	mtx = list_pop(mtx_stack->stack);
	if (mtx == NULL)
		fatal("Empty stack!\n");
	x86_opengl_matrix_free(mtx);

	/* Return */
	return mtx;
}

