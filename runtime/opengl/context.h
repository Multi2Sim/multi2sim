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
struct opengl_program_obj_t;
struct opengl_vertex_array_obj_t;
struct opengl_buffer_binding_points_t;

#ifndef MAX_INDEXED_TARGETS
#define MAX_INDEXED_TARGETS 16
#endif

/* OpenGL context state */
struct opengl_context_state_t
{
	unsigned char enable_alpha_test : 1;
	unsigned char enable_auto_normal : 1;
	unsigned char enable_blend : 1;
	unsigned char enable_clip_plane0 : 1;
	unsigned char enable_clip_plane1 : 1;
	unsigned char enable_clip_plane2 : 1;
	unsigned char enable_clip_plane3 : 1;
	unsigned char enable_clip_plane4 : 1;
	unsigned char enable_clip_plane5 : 1;
	unsigned char enable_color_logic_op : 1;
	unsigned char enable_color_material : 1;
	unsigned char enable_color_sum : 1;
	unsigned char enable_color_table : 1;
	unsigned char enable_convolution_1d : 1;
	unsigned char enable_convolution_2d : 1;
	unsigned char enable_cull_face : 1;
	unsigned char enable_depth_test : 1;
	unsigned char enable_dither : 1;
	unsigned char enable_fog : 1;
	unsigned char enable_histogram : 1;
	unsigned char enable_index_logic_op : 1;
	unsigned char enable_light0 : 1;
	unsigned char enable_light1 : 1;
	unsigned char enable_light2 : 1;
	unsigned char enable_light3 : 1;
	unsigned char enable_light4 : 1;
	unsigned char enable_light5 : 1;
	unsigned char enable_light6 : 1;
	unsigned char enable_light7 : 1;
	unsigned char enable_lighting : 1;
	unsigned char enable_line_smooth : 1;
	unsigned char enable_line_stipple : 1;
	unsigned char enable_map1_color_4 : 1;
	unsigned char enable_map1_index : 1;
	unsigned char enable_map1_normal : 1;
	unsigned char enable_map1_texture_coord_1 : 1;
	unsigned char enable_map1_texture_coord_2 : 1;
	unsigned char enable_map1_texture_coord_3 : 1;
	unsigned char enable_map1_texture_coord_4 : 1;
	unsigned char enable_map1_vertex_3 : 1;
	unsigned char enable_map1_vertex_4 : 1;
	unsigned char enable_map2_color_4 : 1;
	unsigned char enable_map2_index : 1;
	unsigned char enable_map2_normal : 1;
	unsigned char enable_map2_texture_coord_1 : 1;
	unsigned char enable_map2_texture_coord_2 : 1;
	unsigned char enable_map2_texture_coord_3 : 1;
	unsigned char enable_map2_texture_coord_4 : 1;
	unsigned char enable_map2_vertex_3 : 1;
	unsigned char enable_map2_vertex_4 : 1;
	unsigned char enable_minmax : 1;
	unsigned char enable_multisample : 1;
	unsigned char enable_normalize : 1;
	unsigned char enable_point_smooth : 1;
	unsigned char enable_point_sprite : 1;
	unsigned char enable_polygon_offset_fill : 1;
	unsigned char enable_polygon_offset_line : 1;
	unsigned char enable_polygon_offset_point : 1;
	unsigned char enable_polygon_smooth : 1;
	unsigned char enable_polygon_stipple : 1;
	unsigned char enable_post_color_matrix_color_table : 1;
	unsigned char enable_post_convolution_color_table : 1;
	unsigned char enable_rescale_normal : 1;
	unsigned char enable_sample_alpha_to_coverage : 1;
	unsigned char enable_sample_alpha_to_one : 1;
	unsigned char enable_sample_coverage : 1;
	unsigned char enable_separable_2d : 1;
	unsigned char enable_scissor_test : 1;
	unsigned char enable_stencil_test : 1;
	unsigned char enable_texture_1d : 1;
	unsigned char enable_texture_2d : 1;
	unsigned char enable_texture_3d : 1;
	unsigned char enable_texture_cube_map : 1;
	unsigned char enable_texture_gen_q : 1;
	unsigned char enable_texture_gen_r : 1;
	unsigned char enable_texture_gen_s : 1;
	unsigned char enable_texture_gen_t : 1;
	unsigned char enable_vertex_program_point_size : 1;
	unsigned char enable_vertex_program_two_side : 1;
};

/* OpenGL context properties */
struct opengl_context_props_t
{
	/* Version & identification */
	unsigned int gl_major;
	unsigned int gl_minor;
	unsigned int glsl_major;
	unsigned int glsl_minor;
	char *vendor;
	char *renderer;
	char info[50];

	/* Clear related */
	float *color;
	double depth;
	int stencil;

	/* Viewport */
	int vp_x;
	int vp_y;
	int vp_w;
	int vp_h;
	float vp_far;
	float vp_near;
};

/* OpenGL context */
struct opengl_context_t
{
	/* Error */
	unsigned int opengl_error;

	/* Context state and properties */
	struct opengl_context_state_t *state;
	struct opengl_context_props_t *props;

	/* Binding points */
	struct opengl_buffer_binding_points_t *buffer_binding_points;
	struct opengl_indexed_buffer_binding_points_t *idxed_buffer_binding_points;
	struct opengl_program_obj_t *program_binding_point;
	struct opengl_vertex_array_obj_t *vao_binding_point;

	/* Repositories for container objects that are exclusive to a OpenGL context */
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
void opengl_context_set_error(struct opengl_context_t *context, unsigned int err);

#endif
