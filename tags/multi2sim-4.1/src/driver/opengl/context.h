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

#ifndef DRIVER_OPENGL_CONTEXT_H
#define DRIVER_OPENGL_CONTEXT_H

#include <GL/glut.h>

#include "color-channel.h"


#define MAX_TEXTURE_COORD_UNITS 8
#define MAX_TEXTURE_IMAGE_UNITS 16
#define MAX_TEXTURE_UNITS ((MAX_TEXTURE_COORD_UNITS > MAX_TEXTURE_IMAGE_UNITS) ? MAX_TEXTURE_COORD_UNITS : MAX_TEXTURE_IMAGE_UNITS)

#define MAX_VARYING	16

struct opengl_buffer_obj_t;
struct opengl_frame_buffer_t;
struct opengl_light_attrib_t;
struct opengl_matrix_stack_t;
struct opengl_program_t;
struct opengl_transform_feedback_state_t;
struct opengl_vertex_buffer_t;
struct opengl_viewport_attributes_t;
struct opengl_vertex_array_obj_t;
struct opengl_vertex_buffer_obj_t;

enum opengl_vertex_attrib_t
{
	VERT_ATTRIB_POS = 0,
	VERT_ATTRIB_WEIGHT = 1,
	VERT_ATTRIB_NORMAL = 2,
	VERT_ATTRIB_COLOR0 = 3,
	VERT_ATTRIB_COLOR1 = 4,
	VERT_ATTRIB_FOG = 5,
	VERT_ATTRIB_COLOR_INDEX = 6,
	VERT_ATTRIB_EDGEFLAG = 7,
	VERT_ATTRIB_TEX0 = 8,
	VERT_ATTRIB_TEX1 = 9,
	VERT_ATTRIB_TEX2 = 10,
	VERT_ATTRIB_TEX3 = 11,
	VERT_ATTRIB_TEX4 = 12,
	VERT_ATTRIB_TEX5 = 13,
	VERT_ATTRIB_TEX6 = 14,
	VERT_ATTRIB_TEX7 = 15,
	VERT_ATTRIB_POINT_SIZE = 16,
	VERT_ATTRIB_GENERIC0 = 17,
	VERT_ATTRIB_GENERIC1 = 18,
	VERT_ATTRIB_GENERIC2 = 19,
	VERT_ATTRIB_GENERIC3 = 20,
	VERT_ATTRIB_GENERIC4 = 21,
	VERT_ATTRIB_GENERIC5 = 22,
	VERT_ATTRIB_GENERIC6 = 23,
	VERT_ATTRIB_GENERIC7 = 24,
	VERT_ATTRIB_GENERIC8 = 25,
	VERT_ATTRIB_GENERIC9 = 26,
	VERT_ATTRIB_GENERIC10 = 27,
	VERT_ATTRIB_GENERIC11 = 28,
	VERT_ATTRIB_GENERIC12 = 29,
	VERT_ATTRIB_GENERIC13 = 30,
	VERT_ATTRIB_GENERIC14 = 31,
	VERT_ATTRIB_GENERIC15 = 32,
	VERT_ATTRIB_MAX = 33
};

enum opengl_frag_attrib_t
{
	FRAG_ATTRIB_WPOS = 0,
	FRAG_ATTRIB_COL0 = 1,
	FRAG_ATTRIB_COL1 = 2,
	FRAG_ATTRIB_FOGC = 3,
	FRAG_ATTRIB_TEX0 = 4,
	FRAG_ATTRIB_TEX1 = 5,
	FRAG_ATTRIB_TEX2 = 6,
	FRAG_ATTRIB_TEX3 = 7,
	FRAG_ATTRIB_TEX4 = 8,
	FRAG_ATTRIB_TEX5 = 9,
	FRAG_ATTRIB_TEX6 = 10,
	FRAG_ATTRIB_TEX7 = 11,
	FRAG_ATTRIB_FACE = 12,  /* front/back face */
	FRAG_ATTRIB_PNTC = 13,  /* sprite/point coord */
	FRAG_ATTRIB_CLIP_DIST0 = 14,
	FRAG_ATTRIB_CLIP_DIST1 = 15,
	FRAG_ATTRIB_VAR0 = 16,  /* shader varying */
	FRAG_ATTRIB_MAX = (FRAG_ATTRIB_VAR0 + MAX_VARYING)
};

/* OpenGL context capability */
struct opengl_context_capability_t
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

struct opengl_current_attrib_t
{
	GLfloat Attrib[VERT_ATTRIB_MAX][4];	/* Position, color, texcoords, etc */
	GLfloat RasterPos[4];
	GLfloat RasterDistance;
	GLfloat RasterColor[4];
	GLfloat RasterSecondaryColor[4];
	GLfloat RasterTexCoords[MAX_TEXTURE_COORD_UNITS][4];
	GLboolean RasterPosValid;
};

/* OpenGL context*/
struct opengl_context_t
{
	/* For recording OpenGL errors */
	int gl_error;

	/* OpenGL context capabilities */
	struct opengl_context_capability_t *context_cap;					/* context capabilities */

	/* For OpenGL fixed pipeline */
	struct opengl_frame_buffer_t *draw_buffer;						/* buffer for writing */
	struct opengl_frame_buffer_t *read_buffer;						/* buffer for reading */
	struct opengl_viewport_attributes_t *viewport;					/* viewport attributes */
	struct opengl_matrix_stack_t *modelview_matrix_stack;				/* modelview matrix stack */
	struct opengl_matrix_stack_t *projection_matrix_stack;				/* projection matrix stack */
	struct opengl_matrix_stack_t *texture_matrix_stack[MAX_TEXTURE_UNITS];		/* texture matrix stacks */
	struct opengl_matrix_stack_t *color_matrix_stack;					/* color matrix stack */
	struct opengl_matrix_stack_t *current_matrix_stack;					/* current matrix stack, points to one of above stacks */
	struct opengl_vertex_buffer_t *vertex_buffer;
	struct opengl_light_attrib_t *light;

	/* OpenGL objects repository */
	struct linked_list_t *shader_repo;							/* Shader repository contains all shader objects*/
	struct linked_list_t *program_repo;							/* Program repository contains all program objects */
	struct linked_list_t *vao_repo;								/* VAO repository contains all VAO */
	struct linked_list_t *buf_repo;								/* Buffer repository contains all Buffer Objects */

	/* OpenGL context binding points */
	GLchan current_color[4];								/* Binding point for color currently in use */
	GLfloat current_normal[4];								/* Binding point for normal currently in use */
	struct opengl_program_t *current_program;						/* Binding point for program currently in use */
	struct opengl_vertex_array_attrib_t *array_attrib;					/* Binding point for VAO and VBO */
	struct opengl_transform_feedback_state_t *transform_feedback;			/* Binding point for Tranform Feedback */
};

struct opengl_context_capability_t *opengl_context_capability_create(void);
void opengl_context_capability_free(struct opengl_context_capability_t *cap);

struct opengl_context_t *opengl_context_create(void);
void opengl_context_free(struct opengl_context_t *ctx);

struct opengl_matrix_t *opengl_context_get_current_matrix(struct opengl_context_t *ctx);

struct opengl_current_attrib_t *opengl_current_attrib_create();
void opengl_current_attrib_free(struct opengl_current_attrib_t *crnt);

struct opengl_buffer_obj_t *opengl_context_get_bound_buffer(unsigned int target, struct opengl_context_t *ctx);

#endif
