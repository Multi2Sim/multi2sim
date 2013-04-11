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

#ifndef RUNTIME_OPENGL_CONTEXT_H
#define RUNTIME_OPENGL_CONTEXT_H

#include "opengl.h"

/* Forward declarations*/
struct linked_list_t;
struct opengl_buffer_binding_points_t;

/* OpenGL context state */
struct opengl_context_state_t
{
	unsigned char enable_alpha_test;
	unsigned char enable_auto_normal;
	unsigned char enable_blend;
	unsigned char enable_clip_plane0;
	unsigned char enable_clip_plane1;
	unsigned char enable_clip_plane2;
	unsigned char enable_clip_plane3;
	unsigned char enable_clip_plane4;
	unsigned char enable_clip_plane5;
	unsigned char enable_color_logic_op;
	unsigned char enable_color_material;
	unsigned char enable_color_sum;
	unsigned char enable_color_table;
	unsigned char enable_convolution_1d;
	unsigned char enable_convolution_2d;
	unsigned char enable_cull_face;
	unsigned char enable_depth_test;
	unsigned char enable_dither;
	unsigned char enable_fog;
	unsigned char enable_histogram;
	unsigned char enable_index_logic_op;
	unsigned char enable_light0;
	unsigned char enable_light1;
	unsigned char enable_light2;
	unsigned char enable_light3;
	unsigned char enable_light4;
	unsigned char enable_light5;
	unsigned char enable_light6;
	unsigned char enable_light7;
	unsigned char enable_lighting;
	unsigned char enable_line_smooth;
	unsigned char enable_line_stipple;
	unsigned char enable_map1_color_4;
	unsigned char enable_map1_index;
	unsigned char enable_map1_normal;
	unsigned char enable_map1_texture_coord_1;
	unsigned char enable_map1_texture_coord_2;
	unsigned char enable_map1_texture_coord_3;
	unsigned char enable_map1_texture_coord_4;
	unsigned char enable_map1_vertex_3;
	unsigned char enable_map1_vertex_4;
	unsigned char enable_map2_color_4;
	unsigned char enable_map2_index;
	unsigned char enable_map2_normal;
	unsigned char enable_map2_texture_coord_1;
	unsigned char enable_map2_texture_coord_2;
	unsigned char enable_map2_texture_coord_3;
	unsigned char enable_map2_texture_coord_4;
	unsigned char enable_map2_vertex_3;
	unsigned char enable_map2_vertex_4;
	unsigned char enable_minmax;
	unsigned char enable_multisample;
	unsigned char enable_normalize;
	unsigned char enable_point_smooth;
	unsigned char enable_point_sprite;
	unsigned char enable_polygon_offset_fill;
	unsigned char enable_polygon_offset_line;
	unsigned char enable_polygon_offset_point;
	unsigned char enable_polygon_smooth;
	unsigned char enable_polygon_stipple;
	unsigned char enable_post_color_matrix_color_table;
	unsigned char enable_post_convolution_color_table;
	unsigned char enable_rescale_normal;
	unsigned char enable_sample_alpha_to_coverage;
	unsigned char enable_sample_alpha_to_one;
	unsigned char enable_sample_coverage;
	unsigned char enable_separable_2d;
	unsigned char enable_scissor_test;
	unsigned char enable_stencil_test;
	unsigned char enable_texture_1d;
	unsigned char enable_texture_2d;
	unsigned char enable_texture_3d;
	unsigned char enable_texture_cube_map;
	unsigned char enable_texture_gen_q;
	unsigned char enable_texture_gen_r;
	unsigned char enable_texture_gen_s;
	unsigned char enable_texture_gen_t;
	unsigned char enable_vertex_program_point_size;
	unsigned char enable_vertex_program_two_side;
};

/* OpenGL context properties */
struct opengl_context_props_t
{
	unsigned int gl_major;
	unsigned int gl_minor;
	unsigned int glsl_major;
	unsigned int glsl_minor;
	char *vendor;
	char *renderer;
	char info[50];
};

/* OpenGL context */
struct opengl_context_t
{
	/* Context state and properties */
	struct opengl_context_state_t *state;
	struct opengl_context_props_t *props;

	/* Binding points */
	struct opengl_buffer_binding_points_t *buffer_binding_points;
	struct opengl_program_obj_t *program_binding_point;

	/* Repositories for container objects that cannot be shared */
	struct linked_list_t *renderbuf_repo;
	struct linked_list_t *prog_pipe_repo;
	struct linked_list_t *framebuf_repo;
	struct linked_list_t *vao_repo;
	struct linked_list_t *transfeed_repo;
	struct linked_list_t *query_repo;
};

extern struct opengl_context_t *opengl_ctx;

void opengl_context_props_set_version(unsigned int major, unsigned int minor, struct opengl_context_props_t *props);

void opengl_context_init();
void opengl_context_destroy();

#endif
