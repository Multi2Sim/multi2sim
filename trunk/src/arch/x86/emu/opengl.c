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

#include <x86-emu.h>
#include <mem-system.h>
#include <GL/gl.h>
#include <opengl-context.h>

static char *err_x86_opengl_code =
	"\tAn invalid function code was generated by your application in a OpenGL system\n"
	"\tcall. Probably, this means that your application is using an incompatible\n"
	"\tversion of the Multi2Sim OpenGL runtime library ('libm2s-opengl'). Please\n"
	"\trecompile your application and try again.\n";

/* Debug */
int x86_opengl_debug_category;


/* List of OPENGL runtime calls */
enum x86_opengl_call_t
{
	x86_opengl_call_invalid = 0,
#define X86_OPENGL_DEFINE_CALL(name, code) x86_opengl_call_##name = code,
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL
	x86_opengl_call_count
};


/* List of OPENGL runtime call names */
char *x86_opengl_call_name[x86_opengl_call_count + 1] =
{
	NULL,
#define X86_OPENGL_DEFINE_CALL(name, code) #name,
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL
	NULL
};


/* Forward declarations of OPENGL runtime functions */
#define X86_OPENGL_DEFINE_CALL(name, code) \
	static int x86_opengl_func_##name(struct x86_ctx_t *ctx);
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL


/* List of OPENGL runtime functions */
typedef int (*x86_opengl_func_t)(struct x86_ctx_t *ctx);
static x86_opengl_func_t x86_opengl_func_table[x86_opengl_call_count + 1] =
{
	NULL,
#define X86_OPENGL_DEFINE_CALL(name, code) x86_opengl_func_##name,
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL
	NULL
};


/*
 * OpenGL global variables
 */

/* OpenGL Context */
struct x86_opengl_context_t *x86_opengl_ctx;

void x86_opengl_init(void)
{
	x86_opengl_ctx = x86_opengl_context_create();
}


void x86_opengl_done(void)
{
	if (x86_opengl_ctx != NULL)
		x86_opengl_context_free(x86_opengl_ctx);
}


int x86_opengl_call(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	/* Variables */
	int code;
	int ret;

	/* Function code */
	code = regs->ebx;
	if (code <= x86_opengl_call_invalid || code >= x86_opengl_call_count)
		fatal("%s: invalid OpenGL function (code %d).\n%s",
			__FUNCTION__, code, err_x86_opengl_code);

	/* Debug */
	x86_opengl_debug("OpenGL runtime call '%s' (code %d)\n",
		x86_opengl_call_name[code], code);

	/* Call OPENGL function */
	assert(x86_opengl_func_table[code]);
	ret = x86_opengl_func_table[code](ctx);

	/* Return value */
	return ret;
}

/*
 * OpenGL call #1 - glDrawBuffer
 *
 * glDrawBuffer - specify which color buffers are to be drawn into
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glDrawBuffer(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	/* Variables */
	unsigned int mode_ptr;

	/* Read arguments */
	mode_ptr = regs->ecx;
	x86_opengl_debug("\tmode_ptr=0x%x\n", mode_ptr);

	GLenum mode;

	mem_read(mem, mode_ptr, sizeof(GLenum), &mode);

	/* Set color buffers */
	fatal("Not implemented yet!\n");

	/* Return success */
	return 0;
}


/*
 * OpenGL call #2 - glReadBuffer
 *
 * glReadBuffer - select a color buffer source for pixels
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glReadBuffer(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int mode_ptr;

	/* Read arguments */
	mode_ptr = regs->ecx;
	x86_opengl_debug("\tmode_ptr=0x%x\n", mode_ptr);

	GLenum mode;
	mem_read(mem, mode_ptr, sizeof(GLenum), &mode);

	/* Set color buffers */
	fatal("Not implemented yet!\n");

	/* Return success */
	return 0;
}
/*
 * OpenGL call #3 - glEnable
 *
 * glEnable - enable server-side GL capabilities
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glEnable(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int cap_ptr;

	/* Read arguments */
	cap_ptr = regs->ecx;
	x86_opengl_debug("\tcap_ptr=0x%x\n", cap_ptr);

	GLenum cap;

	mem_read(mem, cap_ptr, sizeof(GLenum), &cap);

	switch(cap)
	{

	case GL_ALPHA_TEST:

		{
			x86_opengl_ctx->context_cap->is_alpha_test = GL_TRUE;
			x86_opengl_debug("\tGL_ALPHA_TEST enabled!\n");
			break;
		}

	case GL_AUTO_NORMAL:

		{
			x86_opengl_ctx->context_cap->is_auto_normal = GL_TRUE;
			x86_opengl_debug("\tGL_AUTO_NORMAL enabled!\n");
			break;
		}

	case GL_BLEND:

		{
			x86_opengl_ctx->context_cap->is_blend = GL_TRUE;
			x86_opengl_debug("\tGL_BLEND enabled!\n");
			break;
		}

	case GL_CLIP_PLANE0:

		{
			x86_opengl_ctx->context_cap->is_clip_plane0 = GL_TRUE;
			x86_opengl_debug("\tGL_CLIP_PLANE0 enabled!\n");
			break;
		}

	case GL_CLIP_PLANE1:

		{
			x86_opengl_ctx->context_cap->is_clip_plane1 = GL_TRUE;
			x86_opengl_debug("\tGL_CLIP_PLANE1 enabled!\n");
			break;
		}

	case GL_CLIP_PLANE2:

		{
			x86_opengl_ctx->context_cap->is_clip_plane2 = GL_TRUE;
			x86_opengl_debug("\tGL_CLIP_PLANE2 enabled!\n");
			break;
		}

	case GL_CLIP_PLANE3:

		{
			x86_opengl_ctx->context_cap->is_clip_plane3 = GL_TRUE;
			x86_opengl_debug("\tGL_CLIP_PLANE3 enabled!\n");
			break;
		}

	case GL_CLIP_PLANE4:

		{
			x86_opengl_ctx->context_cap->is_clip_plane4 = GL_TRUE;
			x86_opengl_debug("\tGL_CLIP_PLANE4 enabled!\n");
			break;
		}

	case GL_CLIP_PLANE5:

		{
			x86_opengl_ctx->context_cap->is_clip_plane5 = GL_TRUE;
			x86_opengl_debug("\tGL_CLIP_PLANE5 enabled!\n");
			break;
		}

	case GL_COLOR_LOGIC_OP:

		{
			x86_opengl_ctx->context_cap->is_color_logic_op = GL_TRUE;
			x86_opengl_debug("\tGL_COLOR_LOGIC_OP enabled!\n");
			break;
		}

	case GL_COLOR_MATERIAL:

		{
			x86_opengl_ctx->context_cap->is_color_material = GL_TRUE;
			x86_opengl_debug("\tGL_COLOR_MATERIAL enabled!\n");
			break;
		}

	case GL_COLOR_SUM:

		{
			x86_opengl_ctx->context_cap->is_color_sum = GL_TRUE;
			x86_opengl_debug("\tGL_COLOR_SUM enabled!\n");
			break;
		}

	case GL_COLOR_TABLE:

		{
			x86_opengl_ctx->context_cap->is_color_table = GL_TRUE;
			x86_opengl_debug("\tGL_COLOR_TABLE enabled!\n");
			break;
		}

	case GL_CONVOLUTION_1D:

		{
			x86_opengl_ctx->context_cap->is_convolution_1d = GL_TRUE;
			x86_opengl_debug("\tGL_CONVOLUTION_1D enabled!\n");
			break;
		}

	case GL_CONVOLUTION_2D:

		{
			x86_opengl_ctx->context_cap->is_convolution_2d = GL_TRUE;
			x86_opengl_debug("\tGL_CONVOLUTION_2D enabled!\n");
			break;
		}

	case GL_CULL_FACE:

		{
			x86_opengl_ctx->context_cap->is_cull_face = GL_TRUE;
			x86_opengl_debug("\tGL_CULL_FACE enabled!\n");
			break;
		}

	case GL_DEPTH_TEST:

		{
			x86_opengl_ctx->context_cap->is_depth_test = GL_TRUE;
			x86_opengl_debug("\tGL_DEPTH_TEST enabled!\n");
			break;
		}

	case GL_DITHER:

		{
			x86_opengl_ctx->context_cap->is_dither = GL_TRUE;
			x86_opengl_debug("\tGL_DITHER enabled!\n");
			break;
		}

	case GL_FOG:

		{
			x86_opengl_ctx->context_cap->is_fog = GL_TRUE;
			x86_opengl_debug("\tGL_FOG enabled!\n");
			break;
		}

	case GL_HISTOGRAM:

		{
			x86_opengl_ctx->context_cap->is_histogram = GL_TRUE;
			x86_opengl_debug("\tGL_HISTOGRAM enabled!\n");
			break;
		}

	case GL_INDEX_LOGIC_OP:

		{
			x86_opengl_ctx->context_cap->is_index_logic_op = GL_TRUE;
			x86_opengl_debug("\tGL_INDEX_LOGIC_OP enabled!\n");
			break;
		}

	case GL_LIGHT0:

		{
			x86_opengl_ctx->context_cap->is_light0 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT0 enabled!\n");
			break;
		}

	case GL_LIGHT1:

		{
			x86_opengl_ctx->context_cap->is_light1 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT1 enabled!\n");
			break;
		}

	case GL_LIGHT2:

		{
			x86_opengl_ctx->context_cap->is_light2 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT2 enabled!\n");
			break;
		}

	case GL_LIGHT3:

		{
			x86_opengl_ctx->context_cap->is_light3 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT3 enabled!\n");
			break;
		}

	case GL_LIGHT4:

		{
			x86_opengl_ctx->context_cap->is_light4 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT4 enabled!\n");
			break;
		}

	case GL_LIGHT5:

		{
			x86_opengl_ctx->context_cap->is_light5 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT5 enabled!\n");
			break;
		}

	case GL_LIGHT6:

		{
			x86_opengl_ctx->context_cap->is_light6 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT6 enabled!\n");
			break;
		}

	case GL_LIGHT7:

		{
			x86_opengl_ctx->context_cap->is_light7 = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHT7 enabled!\n");
			break;
		}

	case GL_LIGHTING:

		{
			x86_opengl_ctx->context_cap->is_lighting = GL_TRUE;
			x86_opengl_debug("\tGL_LIGHTING enabled!\n");
			break;
		}

	case GL_LINE_SMOOTH:

		{
			x86_opengl_ctx->context_cap->is_line_smooth = GL_TRUE;
			x86_opengl_debug("\tGL_LINE_SMOOTH enabled!\n");
			break;
		}

	case GL_LINE_STIPPLE:

		{
			x86_opengl_ctx->context_cap->is_line_stipple = GL_TRUE;
			x86_opengl_debug("\tGL_LINE_STIPPLE enabled!\n");
			break;
		}

	case GL_MAP1_COLOR_4:

		{
			x86_opengl_ctx->context_cap->is_map1_color_4 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_COLOR_4 enabled!\n");
			break;
		}

	case GL_MAP1_INDEX:

		{
			x86_opengl_ctx->context_cap->is_map1_index = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_INDEX enabled!\n");
			break;
		}

	case GL_MAP1_NORMAL:

		{
			x86_opengl_ctx->context_cap->is_map1_normal = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_NORMAL enabled!\n");
			break;
		}

	case GL_MAP1_TEXTURE_COORD_1:

		{
			x86_opengl_ctx->context_cap->is_map1_texture_coord_1 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_TEXTURE_COORD_1 enabled!\n");
			break;
		}

	case GL_MAP1_TEXTURE_COORD_2:

		{
			x86_opengl_ctx->context_cap->is_map1_texture_coord_2 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_TEXTURE_COORD_2 enabled!\n");
			break;
		}

	case GL_MAP1_TEXTURE_COORD_3:

		{
			x86_opengl_ctx->context_cap->is_map1_texture_coord_3 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_TEXTURE_COORD_3 enabled!\n");
			break;
		}

	case GL_MAP1_TEXTURE_COORD_4:

		{
			x86_opengl_ctx->context_cap->is_map1_texture_coord_4 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_TEXTURE_COORD_4 enabled!\n");
			break;
		}

	case GL_MAP1_VERTEX_3:

		{
			x86_opengl_ctx->context_cap->is_map1_vertex_3 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_VERTEX_3 enabled!\n");
			break;
		}

	case GL_MAP1_VERTEX_4:

		{
			x86_opengl_ctx->context_cap->is_map1_vertex_4 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP1_VERTEX_4 enabled!\n");
			break;
		}

	case GL_MAP2_COLOR_4:

		{
			x86_opengl_ctx->context_cap->is_map2_color_4 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_COLOR_4 enabled!\n");
			break;
		}

	case GL_MAP2_INDEX:

		{
			x86_opengl_ctx->context_cap->is_map2_index = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_INDEX enabled!\n");
			break;
		}

	case GL_MAP2_NORMAL:

		{
			x86_opengl_ctx->context_cap->is_map2_normal = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_NORMAL enabled!\n");
			break;
		}

	case GL_MAP2_TEXTURE_COORD_1:

		{
			x86_opengl_ctx->context_cap->is_map2_texture_coord_1 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_TEXTURE_COORD_1 enabled!\n");
			break;
		}

	case GL_MAP2_TEXTURE_COORD_2:

		{
			x86_opengl_ctx->context_cap->is_map2_texture_coord_2 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_TEXTURE_COORD_2 enabled!\n");
			break;
		}

	case GL_MAP2_TEXTURE_COORD_3:

		{
			x86_opengl_ctx->context_cap->is_map2_texture_coord_3 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_TEXTURE_COORD_3 enabled!\n");
			break;
		}

	case GL_MAP2_TEXTURE_COORD_4:

		{
			x86_opengl_ctx->context_cap->is_map2_texture_coord_4 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_TEXTURE_COORD_4 enabled!\n");
			break;
		}

	case GL_MAP2_VERTEX_3:

		{
			x86_opengl_ctx->context_cap->is_map2_vertex_3 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_VERTEX_3 enabled!\n");
			break;
		}

	case GL_MAP2_VERTEX_4:

		{
			x86_opengl_ctx->context_cap->is_map2_vertex_4 = GL_TRUE;
			x86_opengl_debug("\tGL_MAP2_VERTEX_4 enabled!\n");
			break;
		}

	case GL_MINMAX:

		{
			x86_opengl_ctx->context_cap->is_minmax = GL_TRUE;
			x86_opengl_debug("\tGL_MINMAX enabled!\n");
			break;
		}

	case GL_MULTISAMPLE:

		{
			x86_opengl_ctx->context_cap->is_multisample = GL_TRUE;
			x86_opengl_debug("\tGL_MULTISAMPLE enabled!\n");
			break;
		}

	case GL_NORMALIZE:

		{
			x86_opengl_ctx->context_cap->is_normalize = GL_TRUE;
			x86_opengl_debug("\tGL_NORMALIZE enabled!\n");
			break;
		}

	case GL_POINT_SMOOTH:

		{
			x86_opengl_ctx->context_cap->is_point_smooth = GL_TRUE;
			x86_opengl_debug("\tGL_POINT_SMOOTH enabled!\n");
			break;
		}

	case GL_POINT_SPRITE:

		{
			x86_opengl_ctx->context_cap->is_point_sprite = GL_TRUE;
			x86_opengl_debug("\tGL_POINT_SPRITE enabled!\n");
			break;
		}

	case GL_POLYGON_OFFSET_FILL:

		{
			x86_opengl_ctx->context_cap->is_polygon_offset_fill = GL_TRUE;
			x86_opengl_debug("\tGL_POLYGON_OFFSET_FILL enabled!\n");
			break;
		}

	case GL_POLYGON_OFFSET_LINE:

		{
			x86_opengl_ctx->context_cap->is_polygon_offset_line = GL_TRUE;
			x86_opengl_debug("\tGL_POLYGON_OFFSET_LINE enabled!\n");
			break;
		}

	case GL_POLYGON_OFFSET_POINT:

		{
			x86_opengl_ctx->context_cap->is_polygon_offset_point = GL_TRUE;
			x86_opengl_debug("\tGL_POLYGON_OFFSET_POINT enabled!\n");
			break;
		}

	case GL_POLYGON_SMOOTH:

		{
			x86_opengl_ctx->context_cap->is_polygon_smooth = GL_TRUE;
			x86_opengl_debug("\tGL_POLYGON_SMOOTH enabled!\n");
			break;
		}

	case GL_POLYGON_STIPPLE:

		{
			x86_opengl_ctx->context_cap->is_polygon_stipple = GL_TRUE;
			x86_opengl_debug("\tGL_POLYGON_STIPPLE enabled!\n");
			break;
		}

	case GL_POST_COLOR_MATRIX_COLOR_TABLE:

		{
			x86_opengl_ctx->context_cap->is_post_color_matrix_color_table = GL_TRUE;
			x86_opengl_debug("\tGL_POST_COLOR_MATRIX_COLOR_TABLE enabled!\n");
			break;
		}

	case GL_POST_CONVOLUTION_COLOR_TABLE:

		{
			x86_opengl_ctx->context_cap->is_post_convolution_color_table = GL_TRUE;
			x86_opengl_debug("\tGL_POST_CONVOLUTION_COLOR_TABLE enabled!\n");
			break;
		}

	case GL_RESCALE_NORMAL:

		{
			x86_opengl_ctx->context_cap->is_rescale_normal = GL_TRUE;
			x86_opengl_debug("\tGL_RESCALE_NORMAL enabled!\n");
			break;
		}

	case GL_SAMPLE_ALPHA_TO_COVERAGE:

		{
			x86_opengl_ctx->context_cap->is_sample_alpha_to_coverage = GL_TRUE;
			x86_opengl_debug("\tGL_SAMPLE_ALPHA_TO_COVERAGE enabled!\n");
			break;
		}

	case GL_SAMPLE_ALPHA_TO_ONE:

		{
			x86_opengl_ctx->context_cap->is_sample_alpha_to_one = GL_TRUE;
			x86_opengl_debug("\tGL_SAMPLE_ALPHA_TO_ONE enabled!\n");
			break;
		}

	case GL_SAMPLE_COVERAGE:

		{
			x86_opengl_ctx->context_cap->is_sample_coverage = GL_TRUE;
			x86_opengl_debug("\tGL_SAMPLE_COVERAGE enabled!\n");
			break;
		}

	case GL_SEPARABLE_2D:

		{
			x86_opengl_ctx->context_cap->is_separable_2d = GL_TRUE;
			x86_opengl_debug("\tGL_SEPARABLE_2D enabled!\n");
			break;
		}

	case GL_SCISSOR_TEST:

		{
			x86_opengl_ctx->context_cap->is_scissor_test = GL_TRUE;
			x86_opengl_debug("\tGL_SCISSOR_TEST enabled!\n");
			break;
		}

	case GL_STENCIL_TEST:

		{
			x86_opengl_ctx->context_cap->is_stencil_test = GL_TRUE;
			x86_opengl_debug("\tGL_STENCIL_TEST enabled!\n");
			break;
		}

	case GL_TEXTURE_1D:

		{
			x86_opengl_ctx->context_cap->is_texture_1d = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_1D enabled!\n");
			break;
		}

	case GL_TEXTURE_2D:

		{
			x86_opengl_ctx->context_cap->is_texture_2d = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_2D enabled!\n");
			break;
		}

	case GL_TEXTURE_3D:

		{
			x86_opengl_ctx->context_cap->is_texture_3d = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_3D enabled!\n");
			break;
		}

	case GL_TEXTURE_CUBE_MAP:

		{
			x86_opengl_ctx->context_cap->is_texture_cube_map = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_CUBE_MAP enabled!\n");
			break;
		}

	case GL_TEXTURE_GEN_Q:

		{
			x86_opengl_ctx->context_cap->is_texture_gen_q = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_GEN_Q enabled!\n");
			break;
		}

	case GL_TEXTURE_GEN_R:

		{
			x86_opengl_ctx->context_cap->is_texture_gen_r = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_GEN_R enabled!\n");
			break;
		}

	case GL_TEXTURE_GEN_S:

		{
			x86_opengl_ctx->context_cap->is_texture_gen_s = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_GEN_S enabled!\n");
			break;
		}

	case GL_TEXTURE_GEN_T:

		{
			x86_opengl_ctx->context_cap->is_texture_gen_t = GL_TRUE;
			x86_opengl_debug("\tGL_TEXTURE_GEN_T enabled!\n");
			break;
		}

	case GL_VERTEX_PROGRAM_POINT_SIZE:

		{
			x86_opengl_ctx->context_cap->is_vertex_program_point_size = GL_TRUE;
			x86_opengl_debug("\tGL_VERTEX_PROGRAM_POINT_SIZE enabled!\n");
			break;
		}

	case GL_VERTEX_PROGRAM_TWO_SIDE:

		{
			x86_opengl_ctx->context_cap->is_vertex_program_two_side = GL_TRUE;
			x86_opengl_debug("\tGL_VERTEX_PROGRAM_TWO_SIDE enabled!\n");
			break;
		}

	default:
		break;
	}

	/* Return success */
	return 0;
}

/*
 * OpenGL call #4 - glViewport
 *
 * glViewport - set the viewport
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glViewport(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int args_ptr;

	/* Read arguments */
	args_ptr = regs->ecx;

	x86_opengl_debug("\targs_ptr=0x%x\n", args_ptr);

	/* Get function info */
	int func_args[4];

	/* GLint == GLsizei == int */
	mem_read(mem, args_ptr, 4*sizeof(GLint), func_args);
	x86_opengl_ctx->viewport->x = func_args[0];
	x86_opengl_ctx->viewport->y = func_args[1];
	x86_opengl_ctx->viewport->width = func_args[2];
	x86_opengl_ctx->viewport->height = func_args[3];

	/* Initialize */
	x86_opengl_debug("\tviewport: x=%d, y=%d, width=%d, height=%d\n",
				x86_opengl_ctx->viewport->x, x86_opengl_ctx->viewport->y, 
				x86_opengl_ctx->viewport->width, x86_opengl_ctx->viewport->height);

	/* Update vertex buffer */
	// int i;
	// int j;
	// struct x86_opengl_vertex_group_t *vtxgp = NULL;
	// struct x86_opengl_vertex_t *vtx = NULL;

	// int vtxgp_count = list_count(x86_opengl_ctx->vertex_buffer->vertex_groups); 
	// for (i = 0; i < vtxgp_count; ++i)
	// {
	// 	vtxgp = list_get(x86_opengl_ctx->vertex_buffer->vertex_groups, i);
	// 	int vtx_count = list_count(vtxgp->vertex_list);
	// 	for (j = 0; j < vtx_count; ++j)
	// 	{
	// 		/* Update all vertex in vertex buffer */
	// 		vtx = list_get(vtxgp->vertex_list, j);
	// 		vtx->x = (vtx->x/2.0 + 1) * x86_opengl_ctx->viewport->width*0.5+ x86_opengl_ctx->viewport->x;
	// 		vtx->y = (vtx->y/2.0 + 1) * x86_opengl_ctx->viewport->height*0.5+ x86_opengl_ctx->viewport->y;		
	// 		x86_opengl_debug("\tViewported Vertex \t[%f, %f, %f, %f]\n", vtx->x, vtx->y, vtx->z, vtx->w);		
	// 	}
	// }


	/* Update frame buffer */
	/* FIXME: Viewport size != frame buffer size, it only selects a part of frame buffer */
	x86_opengl_frame_buffer_resize(x86_opengl_ctx->draw_buffer, 
		x86_opengl_ctx->viewport->width, x86_opengl_ctx->viewport->height);
	x86_opengl_frame_buffer_resize(x86_opengl_ctx->read_buffer, 
		x86_opengl_ctx->viewport->width, x86_opengl_ctx->viewport->height);
	x86_glut_frame_buffer_resize(x86_opengl_ctx->viewport->width, x86_opengl_ctx->viewport->height);

	/* Return */
	return 0;	
}

/*
 * OpenGL call #5 - glMatrixMode
 *
 * glMatrixMode - specify which matrix is the current matrix
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glMatrixMode(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int mtx_mode_ptr;
	GLenum mtx_mod;

	/* Read arguments */
	mtx_mode_ptr = regs->ecx;
	x86_opengl_debug("\tmtx_mode_ptr=0x%x\n", mtx_mode_ptr);

	mem_read(mem, mtx_mode_ptr, sizeof(GLenum), &mtx_mod);

	/* Set up current matrix */
	switch(mtx_mod)
	{
		case	GL_MODELVIEW:
		{
			x86_opengl_ctx->current_matrix_stack = x86_opengl_ctx->modelview_matrix_stack;
			break;
		}
			
		case	GL_PROJECTION:
		{
			x86_opengl_ctx->current_matrix_stack = x86_opengl_ctx->projection_matrix_stack;
			break;
		}

		case	GL_TEXTURE:
		{
			/* FIXME: choose which one? */
			x86_opengl_ctx->current_matrix_stack = x86_opengl_ctx->texture_matrix_stack[0];
			break;
		}

		case	GL_COLOR:
		{
			x86_opengl_ctx->current_matrix_stack = x86_opengl_ctx->color_matrix_stack;
			break;
		}
		default:
			break;
	}

	/* Return */
	return 0;	
}

/*
 * OpenGL call #6 - glLoadIdentity
 *
 * glLoadIdentity - replace the current matrix with the identity matrix
 *		       the current matrix in any mode is the matrix on the top of the stack for that mode.
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glLoadIdentity(struct x86_ctx_t *ctx)
{
	/* Pop from current stack and free the matrix */
	if (x86_opengl_ctx->current_matrix_stack->depth > 0)
		x86_opengl_matrix_stack_pop(x86_opengl_ctx->current_matrix_stack);

	/* Push identity matrix to the current matrix stack */
	struct x86_opengl_matrix_t *mtx = x86_opengl_matrix_create(MATRIX_IDENTITY);
	x86_opengl_matrix_stack_push(x86_opengl_ctx->current_matrix_stack, mtx);

	/* Return */
	return 0;	
}

/*
 * OpenGL call #7 - glOrtho
 *
 * glOrtho - multiply the current matrix with an orthographic matrix
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glOrtho(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int args_ptr;

	int i;
	int j;

	/* Read arguments */
	args_ptr = regs->ecx;

	x86_opengl_debug("\targs_ptr=0x%x\n", args_ptr);

	/* Get function info */
	GLdouble func_args[6];
	mem_read(mem, args_ptr, 6 * sizeof(GLdouble), func_args);
	for (i = 0; i < 6; i++)
		x86_opengl_debug("\t\targs[%d] = %f (0x%f)\n",
			i, func_args[i], func_args[i]);

	/* Current matrix multiplies orthographic matrix */
	struct x86_opengl_matrix_t *mtx_ortho;
	struct x86_opengl_matrix_t *mtx_curr;

	/* Calculate orthographic matrix */
	GLfloat ortho_matrix[16];

	GLfloat left = func_args[0];
	GLfloat right = func_args[1];
	GLfloat bottom = func_args[2];
	GLfloat top = func_args[3];
	GLfloat nearval = func_args[4];
	GLfloat farval = func_args[5];

	#define M(row,col)  ortho_matrix[col*4+row]
	M(0,0) = 2.0F / (right-left);
	M(0,1) = 0.0F;
	M(0,2) = 0.0F;
	M(0,3) = -(right+left) / (right-left);

	M(1,0) = 0.0F;
	M(1,1) = 2.0F / (top-bottom);
	M(1,2) = 0.0F;
	M(1,3) = -(top+bottom) / (top-bottom);

	M(2,0) = 0.0F;
	M(2,1) = 0.0F;
	M(2,2) = -2.0F / (farval-nearval);
	M(2,3) = -(farval+nearval) / (farval-nearval);

	M(3,0) = 0.0F;
	M(3,1) = 0.0F;
	M(3,2) = 0.0F;
	M(3,3) = 1.0F;
	#undef M

	/* Initialize orthographic matrix */
	mtx_ortho = x86_opengl_matrix_create(MATRIX_GENERAL);
	memcpy(mtx_ortho->matrix, ortho_matrix, 16*sizeof(GLfloat));

	x86_opengl_debug("\t\tOrtho:\n");
	for (i = 0; i < 4; ++i)
	{
		x86_opengl_debug("\t\t");
		for (j = 0; j < 4; ++j)
		{
			x86_opengl_debug("%f\t", mtx_ortho->matrix[j*4+i]);
		}
		x86_opengl_debug("\n");
	}

	mtx_curr = x86_opengl_context_get_current_matrix(x86_opengl_ctx);
	x86_opengl_debug("\t\tCurr:\n");
	for (i = 0; i < 4; ++i)
	{
		x86_opengl_debug("\t\t");
		for (j = 0; j < 4; ++j)
		{
			x86_opengl_debug("%f\t", mtx_curr->matrix[j*4+i]);
		}
		x86_opengl_debug("\n");
	}

	x86_opengl_matrix_mul_matrix(mtx_curr, mtx_curr, mtx_ortho);
	x86_opengl_debug("\t\tCurr mul Ortho:\n");
	for (i = 0; i < 4; ++i)
	{
		x86_opengl_debug("\t\t");
		for (j = 0; j < 4; ++j)
		{
			x86_opengl_debug("%f\t", mtx_curr->matrix[j*4+i]);
		}
		x86_opengl_debug("\n");
	}

	/* Free */
	x86_opengl_matrix_free(mtx_ortho);

	/* Return */
	return 0;	
}

/*
 * OpenGL call #8 - glClear
 *
 * glClear - clear buffers to present values
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glClear(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int mask_ptr;
	GLbitfield mask;

	/* Read arguments */
	mask_ptr = regs->ecx;
	mem_read(mem, mask_ptr, sizeof(GLbitfield), &mask);
	x86_opengl_debug("\tmask_ptr=0x%x\n", mask_ptr);
	x86_opengl_debug("\tmask=0x%x\n", mask);

	/* Clear frame buffers*/
	x86_opengl_frame_buffer_clear(x86_opengl_ctx->draw_buffer, mask);
	x86_opengl_frame_buffer_clear(x86_opengl_ctx->read_buffer, mask);

	/* Return */
	return 0;	
}

/*
 * OpenGL call #9 - glBegin
 *
 * glBegin - delimit the vertices of a primitive or a group of like primitives
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glBegin(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int mode_ptr;
	GLenum mode;

	/* Read arguments */
	mode_ptr = regs->ecx;
	mem_read(mem, mode_ptr, sizeof(GLenum), &mode);
	x86_opengl_debug("\tmode_ptr=0x%x\n", mode_ptr);
	x86_opengl_debug("\tmode=0x%x\n", mode);

	/* Preparation a primitive group for glVertex to insert vertex */
	struct x86_opengl_vertex_group_t *vtxgp;
	vtxgp = x86_opengl_vertex_group_create(mode);
	x86_opengl_vertex_buffer_add_vertex_group(x86_opengl_ctx->vertex_buffer, vtxgp);

	/* Return */
	return 0;	
}

/*
 * OpenGL call #10 - glEnd
 *
 * glEnd - delimit the vertices of a primitive or a group of like primitives
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glEnd(struct x86_ctx_t *ctx)
{
	/* Set current vertex group to NULL */
	x86_opengl_ctx->vertex_buffer->current_vertex_group = NULL;

	/* Return */
	return 0;	
}

/*
 * OpenGL call #11 - glVertex2f
 *
 * glVertex2f - specify a vertex
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glVertex2f(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int args_ptr;
	int i;

	/* Read arguments */
	args_ptr = regs->ecx;

	x86_opengl_debug("\targs_ptr=0x%x\n", args_ptr);

	/* Get function info */
	GLfloat func_args[2];
	mem_read(mem, args_ptr, 2 * sizeof(GLfloat), func_args);
	for (i = 0; i < 2; i++)
		x86_opengl_debug("\t\targs[%d] = %f\n",
			i, func_args[i]);

	/* Add a vertex */
	struct x86_opengl_vertex_t *vertex;
	/* x=x. y=y, z=0.0f, w= 1.0f */	
	vertex = x86_opengl_vertex_create(func_args[0], func_args[1], (GLfloat)0.0f, (GLfloat)1.0f);
	x86_opengl_vertex_buffer_add_vertex(x86_opengl_ctx->vertex_buffer, vertex);

	/* Return */
	return 0;	
}

/*
 * OpenGL call #12 - glVertex3f
 *
 * glVertex3f - specify a vertex
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glVertex3f(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int args_ptr;
	int i;

	/* Read arguments */
	args_ptr = regs->ecx;

	x86_opengl_debug("\targs_ptr=0x%x\n", args_ptr);

	/* Get function info */
	GLfloat func_args[3];
	mem_read(mem, args_ptr, 3 * sizeof(GLfloat), func_args);
	for (i = 0; i < 3; i++)
		x86_opengl_debug("\t\targs[%d] = %f\n",
			i, func_args[i]);

	/* Add a vertex to vertex buffer */
	struct x86_opengl_vertex_t *vertex;
	/* x=x. y=y, z=z, w= 1.0f */
	vertex = x86_opengl_vertex_create(func_args[0], func_args[1], func_args[2], (GLfloat)1.0f);
	x86_opengl_vertex_buffer_add_vertex(x86_opengl_ctx->vertex_buffer, vertex);

	/* Return */
	return 0;	
}

/*
 * OpenGL call #13 - glFlush
 *
 * glFlush -  force execution of GL commands in finite time
 *
 * @return
 *	The function always returns 0
 */

/* Bresenham's line algorithm */
static void x86_opengl_drawline(GLint x1, GLint y1, GLint x2, GLint y2, GLint color)
{

	GLint s_x;
	GLint s_y;
	GLint e_x;
	GLint e_y;

	if (x1 == x2)
	{
		s_y = y1 < y2 ? y1 : y2;
		e_y = y1 > y2 ? y1: y2;
		x86_glut_frame_buffer_pixel(x1, s_y, color);		
		while(s_y < e_y)
		{ 
			s_y++;
			x86_glut_frame_buffer_pixel(x1, s_y, color);				
		}
		return;
	}

	GLfloat m = (y2 - y1) / (x2 - x1);
	x86_opengl_debug("\t\tSlope = %f\n", m);

	if (x1 < x2)
	{
		s_x = x1;
		s_y = y1;
		e_x = x2;
		e_y = y2;
	} else {
		s_x = x2;
		s_y = y2;
		e_x = x1;
		e_y = y1;		
	}

	if (m >= 0.0f && m < 1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dy -dx;
		GLint incrE = 2*dy;
		GLint incrNE = 2*(dy-dx);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			x++;
			if ( e <= 0)
				e += incrE;
			else
			{
				y++;
				e += incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}

	if ( m >= 1.0f )
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dx -dy;
		GLint incrE = 2*dx;
		GLint incrNE = 2*(dx-dy);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(y < e_y)
		{
			y++;
			if ( e <= 0)
				e += incrE;
			else
			{
				x++;
				e += incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}

	if (m < 0.0f && m > -1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dy -dx;
		GLint incrE = 2*dy;
		GLint incrNE = 2*(dy-dx);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			x++;
			if ( e >= 0)
				e -= incrE;
			else
			{
				y--;
				e -= incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}

	if (m <= -1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dx -dy;
		GLint incrE = 2*dx;
		GLint incrNE = 2*(dx-dy);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(y > e_y)
		{
			y--;
			if ( e >= 0)
				e -= incrE;
			else
			{
				x++;
				e -= incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}
}

static int x86_opengl_func_glFlush(struct x86_ctx_t *ctx)
{
	int i;
	int j;
	int width;
	int height;	
	struct x86_opengl_vertex_group_t *vtxgp = NULL;
	struct x86_opengl_vertex_t *vtx = NULL;
	struct x86_opengl_vertex_t *vtx_0 = NULL;
	struct x86_opengl_vertex_t *vtx_1 = NULL;
	struct x86_opengl_matrix_t *mtx = NULL;
	mtx = 	x86_opengl_context_get_current_matrix(x86_opengl_ctx);

	int vtxgp_count = list_count(x86_opengl_ctx->vertex_buffer->vertex_groups); 

	/* Clear frame buffer */
	x86_glut_frame_buffer_clear();

	/* Draw pattern */
	x86_glut_frame_buffer_get_size(&width, &height);
	x86_opengl_debug("\t\tViewport \t[%d, %d, %d, %d]\n", x86_opengl_ctx->viewport->x, x86_opengl_ctx->viewport->y, x86_opengl_ctx->viewport->width, x86_opengl_ctx->viewport->height);

	for (i = 0; i < vtxgp_count; ++i)
	{
		vtxgp = list_get(x86_opengl_ctx->vertex_buffer->vertex_groups, i);
		int vtx_count = list_count(vtxgp->vertex_list);
		for (j = 0; j < vtx_count; ++j)
		{
			/* Multiply ModelView Matrix */

			/* Multiply Perspective Matrix */
			vtx = list_get(vtxgp->vertex_list, j);
			x86_opengl_matrix_mul_vertex(vtx, mtx);
			/* Perspective division */
			vtx->x /= vtx->w;
			vtx->y /= vtx->w;
			x86_opengl_debug("\t\tUpdated Vertex \t[%f, %f, %f, %f]\n", vtx->x, vtx->y, vtx->z, vtx->w);
			/* To screen coordinate */			
			vtx->x = (vtx->x + 1) * x86_opengl_ctx->viewport->width*0.5+ x86_opengl_ctx->viewport->x;
			vtx->y = (vtx->y + 1) * x86_opengl_ctx->viewport->height*0.5+ x86_opengl_ctx->viewport->y;		
			x86_opengl_debug("\t\tScreen position\t[%f, %f, %f, %f]\n", vtx->x, vtx->y, vtx->z, vtx->w);
		}
		for (j = 0; j < vtx_count; ++j)
		{
			/* Draw */
			switch(vtxgp->primitive_type)
			{
				case GL_POINTS:
				{
					vtx = list_get(vtxgp->vertex_list, j);
					x86_opengl_debug("\t\tPoint position\t[%f, %f]\n", vtx->x, vtx->y);
					x86_glut_frame_buffer_pixel(vtx->x, vtx->y, 0xffffff);
					break;				
				}
				case GL_LINES:
				{
					for (j = 0; j < vtx_count / 2; ++j)
					{
						vtx_0 = list_get(vtxgp->vertex_list, 2*j);			
						vtx_1 = list_get(vtxgp->vertex_list, 2*j+1);
						x86_opengl_debug("\t\tLine starts \t[%f, %f]\n", vtx_0->x, vtx_0->y);
						x86_opengl_debug("\t\tLine ends \t[%f, %f]\n", vtx_1->x, vtx_1->y);
						x86_opengl_drawline(vtx_0->x, vtx_0->y, vtx_1->x, vtx_1->y, 0xffffff);
					}
					break;
				}
				case GL_LINE_LOOP:
				{
					for (j = 0; j < vtx_count; ++j)
					{
						vtx_0 = list_get(vtxgp->vertex_list, j);			
						vtx_1 = list_get(vtxgp->vertex_list, (j+1) % vtx_count);
						x86_opengl_debug("\t\tLine starts \t[%f, %f]\n", vtx_0->x, vtx_0->y);
						x86_opengl_debug("\t\tLine ends \t[%f, %f]\n", vtx_1->x, vtx_1->y);				
						x86_opengl_drawline(vtx_0->x, vtx_0->y, vtx_1->x, vtx_1->y, 0xffffff);
					}				
					break;
				}
				case GL_LINE_STRIP:
				{
					for (j = 0; j < vtx_count - 1; ++j)
					{
						vtx_0 = list_get(vtxgp->vertex_list, j);			
						vtx_1 = list_get(vtxgp->vertex_list, j+1);
						x86_opengl_debug("\t\tLine starts \t[%f, %f]\n", vtx_0->x, vtx_0->y);
						x86_opengl_debug("\t\tLine ends \t[%f, %f]\n", vtx_1->x, vtx_1->y);				
						x86_opengl_drawline(vtx_0->x, vtx_0->y, vtx_1->x, vtx_1->y, 0xffffff);
					}
					break;
				}
				case GL_TRIANGLES:
				{
					break;
				}
				case GL_TRIANGLE_STRIP:
				{
					break;
				}
				case GL_TRIANGLE_FAN:
				{
					break;
				}
				case GL_QUADS:
				{
					break;
				}
				case GL_QUAD_STRIP:
				{
					break;
				}
				case GL_POLYGON:
				{
					break;
				}
				default:
					break;
			}
		}
	}

	/* Refresh host GLUT window */
	x86_glut_frame_buffer_flush_request();	

	/* Send to rasterizer to generate final image */

	/* Clean old vertex buffer and create a new one */
	x86_opengl_vertex_buffer_free(x86_opengl_ctx->vertex_buffer);
	x86_opengl_ctx->vertex_buffer = x86_opengl_vertex_buffer_create();

	/* Return */
	return 0;	
}

/*
 * OpenGL call #14 - glColor3f
 *
 * glColor3f - set the current color
 *
 * @return
 *	The function always returns 0
 */

static int x86_opengl_func_glColor3f(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int args_ptr;
	int i;

	/* Read arguments */
	args_ptr = regs->ecx;

	x86_opengl_debug("\targs_ptr=0x%x\n", args_ptr);

	/* Get function info */
	GLfloat func_args[3];
	mem_read(mem, args_ptr, 3 * sizeof(GLfloat), func_args);
	for (i = 0; i < 3; i++)
		x86_opengl_debug("\t\targs[%d] = %f\n",
			i, func_args[i]);

	/* Set the current color */

	/* Return */
	return 0;	
}

