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

#ifndef X86_OPENGL_CONTEXT_H
#define X86_OPENGL_CONTEXT_H

#include <GL/glut.h>
#include <mem-system/mem-system.h>
#include "emu.h"

#include "opengl-buffers.h"
#include "opengl-viewport.h"
#include "opengl-matrix.h"
#include "opengl-matrix-stack.h"
#include "opengl-vertex.h"

#define MAX_TEXTURE_COORD_UNITS 8
#define MAX_TEXTURE_IMAGE_UNITS 16
#define MAX_TEXTURE_UNITS ((MAX_TEXTURE_COORD_UNITS > MAX_TEXTURE_IMAGE_UNITS) ? MAX_TEXTURE_COORD_UNITS : MAX_TEXTURE_IMAGE_UNITS)

/* OpenGL context capability */
struct x86_opengl_context_capability_t
{
	GLboolean is_alpha_test;
	GLboolean is_auto_normal;
	GLboolean is_blend;
	GLboolean is_clip_plane0;
	GLboolean is_clip_plane1;
	GLboolean is_clip_plane2;
	GLboolean is_clip_plane3;
	GLboolean is_clip_plane4;
	GLboolean is_clip_plane5;
	GLboolean is_color_logic_op;
	GLboolean is_color_material;
	GLboolean is_color_sum;
	GLboolean is_color_table;
	GLboolean is_convolution_1d;
	GLboolean is_convolution_2d;
	GLboolean is_cull_face;
	GLboolean is_depth_test;
	GLboolean is_dither;
	GLboolean is_fog;
	GLboolean is_histogram;
	GLboolean is_index_logic_op;
	GLboolean is_light0;
	GLboolean is_light1;
	GLboolean is_light2;
	GLboolean is_light3;
	GLboolean is_light4;
	GLboolean is_light5;
	GLboolean is_light6;
	GLboolean is_light7;
	GLboolean is_lighting;
	GLboolean is_line_smooth;
	GLboolean is_line_stipple;
	GLboolean is_map1_color_4;
	GLboolean is_map1_index;
	GLboolean is_map1_normal;
	GLboolean is_map1_texture_coord_1;
	GLboolean is_map1_texture_coord_2;
	GLboolean is_map1_texture_coord_3;
	GLboolean is_map1_texture_coord_4;
	GLboolean is_map1_vertex_3;
	GLboolean is_map1_vertex_4;
	GLboolean is_map2_color_4;
	GLboolean is_map2_index;
	GLboolean is_map2_normal;
	GLboolean is_map2_texture_coord_1;
	GLboolean is_map2_texture_coord_2;
	GLboolean is_map2_texture_coord_3;
	GLboolean is_map2_texture_coord_4;
	GLboolean is_map2_vertex_3;
	GLboolean is_map2_vertex_4;
	GLboolean is_minmax;
	GLboolean is_multisample;
	GLboolean is_normalize;
	GLboolean is_point_smooth;
	GLboolean is_point_sprite;
	GLboolean is_polygon_offset_fill;
	GLboolean is_polygon_offset_line;
	GLboolean is_polygon_offset_point;
	GLboolean is_polygon_smooth;
	GLboolean is_polygon_stipple;
	GLboolean is_post_color_matrix_color_table;
	GLboolean is_post_convolution_color_table;
	GLboolean is_rescale_normal;
	GLboolean is_sample_alpha_to_coverage;
	GLboolean is_sample_alpha_to_one;
	GLboolean is_sample_coverage;
	GLboolean is_separable_2d;
	GLboolean is_scissor_test;
	GLboolean is_stencil_test;
	GLboolean is_texture_1d;
	GLboolean is_texture_2d;
	GLboolean is_texture_3d;
	GLboolean is_texture_cube_map;
	GLboolean is_texture_gen_q;
	GLboolean is_texture_gen_r;
	GLboolean is_texture_gen_s;
	GLboolean is_texture_gen_t;
	GLboolean is_vertex_program_point_size;
	GLboolean is_vertex_program_two_side;
};

/* OpenGL context*/
struct x86_opengl_context_t
{
	struct x86_opengl_context_capability_t *context_cap;					/* context capabilities */

	struct x86_opengl_frame_buffer_t *draw_buffer;						/* buffer for writing */
	struct x86_opengl_frame_buffer_t *read_buffer;						/* buffer for reading */

	struct x86_opengl_viewport_attributes_t *viewport;						/* viewport attributes */

	struct x86_opengl_matrix_stack_t *modelview_matrix_stack;					/* modelview matrix stack */
	struct x86_opengl_matrix_stack_t *projection_matrix_stack;					/* projection matrix stack */
	struct x86_opengl_matrix_stack_t *texture_matrix_stack[MAX_TEXTURE_UNITS];		/* texture matrix stacks */
	struct x86_opengl_matrix_stack_t *color_matrix_stack;					/* color matrix stack */
	struct x86_opengl_matrix_stack_t *current_matrix_stack;					/* current matrix stack, points to one of above stacks */

	struct x86_opengl_vertex_buffer_t *vertex_buffer;

	GLchan current_color[4];
};

struct x86_opengl_context_capability_t *x86_opengl_context_capability_create(void);
void x86_opengl_context_capability_free(struct x86_opengl_context_capability_t *cap);

struct x86_opengl_context_t *x86_opengl_context_create(void);
void x86_opengl_context_free(struct x86_opengl_context_t *ctx);
struct x86_opengl_matrix_t *x86_opengl_context_get_current_matrix(struct x86_opengl_context_t *ctx);

#endif
