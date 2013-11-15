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


#include <assert.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include <src/arch/southern-islands/asm/input.h>
#include <src/driver/opengl/si-shader.h>
#include "opengl-bin-file.h"


/* 
 * Forward declaration
 */

/* Vertex shader */
static struct opengl_si_enc_dict_vertex_shader_t *opengl_si_enc_dict_vertex_shader_create(struct opengl_si_shader_binary_t *parent);
static void opengl_si_bin_vertex_shader_free(struct opengl_si_enc_dict_vertex_shader_t *vs);
static void opengl_si_bin_vertex_shader_init(struct opengl_si_enc_dict_vertex_shader_t *vs);

/* Pixel shader */
static struct opengl_si_enc_dict_pixel_shader_t *opengl_si_enc_dict_pixel_shader_create(struct opengl_si_shader_binary_t *parent);
static void opengl_si_bin_pixel_shader_free(struct opengl_si_enc_dict_pixel_shader_t *ps);
static void opengl_si_bin_pixel_shader_init(struct opengl_si_enc_dict_pixel_shader_t *ps);


/*
 * Private Functions
 */

static struct str_map_t enc_dict_input_type_map =
{
	8, {
		{ "generic attribute",	OPENGL_SI_INPUT_ATTRIB },
		{ "primary color",	OPENGL_SI_INPUT_COLOR },
		{ "secondary color",	OPENGL_SI_INPUT_SECONDARYCOLOR },
		{ "texture coordinate",	OPENGL_SI_INPUT_TEXCOORD },
		{ "texture unit id",	OPENGL_SI_INPUT_TEXID },
		{ "buffer unit id",	OPENGL_SI_INPUT_BUFFERID },
		{ "constant buffer unit id",	OPENGL_SI_INPUT_CONSTANTBUFFERID },
		{ "texture resource id",	OPENGL_SI_INPUT_TEXTURERESOURCEID },
	}
};

static struct str_map_t enc_dict_input_swizzle_type_map =
{
	6, {
		{"x",	OPENGL_SI_SWIZZLE_X },
		{"y",	OPENGL_SI_SWIZZLE_Y },
		{"z",	OPENGL_SI_SWIZZLE_Z },
		{"w",	OPENGL_SI_SWIZZLE_W },
		{"0",	OPENGL_SI_SWIZZLE_0 },
		{"1",	OPENGL_SI_SWIZZLE_1 },
	}
};
static struct str_map_t enc_dict_output_type_map =
{
	14, {
		{ "pos",			OPENGL_SI_OUTPUT_POS },
		{ "point size",		OPENGL_SI_OUTPUT_POINTSIZE },
		{ "color",		OPENGL_SI_OUTPUT_COLOR },
		{ "secondary color",	OPENGL_SI_OUTPUT_SECONDARYCOLOR }, 
		{ "generic",		OPENGL_SI_OUTPUT_GENERIC },
		{ "depth",		OPENGL_SI_OUTPUT_DEPTH },
		{ "clip distance",	OPENGL_SI_OUTPUT_CLIPDISTANCE },
		{ "primitive id",	OPENGL_SI_OUTPUT_PRIMITIVEID },
		{ "layer",		OPENGL_SI_OUTPUT_LAYER },
		{ "viewport index",	OPENGL_SI_OUTPUT_VIEWPORTINDEX },
		{ "stencil ref",		OPENGL_SI_OUTPUT_STENCIL },
		{ "stencil value",	OPENGL_SI_OUTPUT_STENCIL_VALUE },
		{ "sample mask",	OPENGL_SI_OUTPUT_SAMPLEMASK },
		{ "stream id",		OPENGL_SI_OUTPUT_STREAMID},
	}
};

static struct str_map_t enc_dict_symbol_datatype_map =
{
	164, {
		{ "VOID",	OPENGL_SI_SYMBOL_DATATYPE_VOID },
		{ "BOOL",	OPENGL_SI_SYMBOL_DATATYPE_BOOL },
		{ "INT",		OPENGL_SI_SYMBOL_DATATYPE_INT },
		{ "UINT",	OPENGL_SI_SYMBOL_DATATYPE_UINT },
		{ "FLOAT",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT },
		{ "DOUBLE",	OPENGL_SI_SYMBOL_DATATYPE_DOUBLE },
		{ "FLOAT16",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16 },
		{ "DVEC2",	OPENGL_SI_SYMBOL_DATATYPE_DVEC2 },
		{ "DVEC3",	OPENGL_SI_SYMBOL_DATATYPE_DVEC3 },
		{ "DVEC4",	OPENGL_SI_SYMBOL_DATATYPE_DVEC4 },
		{ "VEC2",	OPENGL_SI_SYMBOL_DATATYPE_VEC2 },
		{ "VEC3",	OPENGL_SI_SYMBOL_DATATYPE_VEC3 },
		{ "VEC4",	OPENGL_SI_SYMBOL_DATATYPE_VEC4 },
		{ "BVEC2",	OPENGL_SI_SYMBOL_DATATYPE_BVEC2 },
		{ "BVEC3",	OPENGL_SI_SYMBOL_DATATYPE_BVEC3 },
		{ "BVEC4",	OPENGL_SI_SYMBOL_DATATYPE_BVEC4 },
		{ "IVEC2",	OPENGL_SI_SYMBOL_DATATYPE_IVEC2 },
		{ "IVEC3",	OPENGL_SI_SYMBOL_DATATYPE_IVEC3 },
		{ "IVEC4",	OPENGL_SI_SYMBOL_DATATYPE_IVEC4 },
		{ "UVEC2",	OPENGL_SI_SYMBOL_DATATYPE_UVEC2 },
		{ "UVEC3",	OPENGL_SI_SYMBOL_DATATYPE_UVEC3 },
		{ "UVEC4",	OPENGL_SI_SYMBOL_DATATYPE_UVEC4 },
		{ "F16VEC2",	OPENGL_SI_SYMBOL_DATATYPE_F16VEC2 },
		{ "F16VEC3",	OPENGL_SI_SYMBOL_DATATYPE_F16VEC3 },
		{ "F16VEC4",	OPENGL_SI_SYMBOL_DATATYPE_F16VEC4 },
		{ "MAT2",	OPENGL_SI_SYMBOL_DATATYPE_MAT2 },
		{ "MAT2X3",	OPENGL_SI_SYMBOL_DATATYPE_MAT2X3 },
		{ "MAT2X4",	OPENGL_SI_SYMBOL_DATATYPE_MAT2X4 },
		{ "MAT3X2",	OPENGL_SI_SYMBOL_DATATYPE_MAT3X2 },
		{ "MAT3",	OPENGL_SI_SYMBOL_DATATYPE_MAT3 },
		{ "MAT3X4",	OPENGL_SI_SYMBOL_DATATYPE_MAT3X4 },
		{ "MAT4X2",	OPENGL_SI_SYMBOL_DATATYPE_MAT4X2 },
		{ "MAT4X3",	OPENGL_SI_SYMBOL_DATATYPE_MAT4X3 },
		{ "MAT4",	OPENGL_SI_SYMBOL_DATATYPE_MAT4 },
		{ "DMAT2",	OPENGL_SI_SYMBOL_DATATYPE_DMAT2 },
		{ "DMAT2X3",	OPENGL_SI_SYMBOL_DATATYPE_DMAT2X3 },
		{ "DMAT2X4",	OPENGL_SI_SYMBOL_DATATYPE_DMAT2X4 },
		{ "DMAT3X2",	OPENGL_SI_SYMBOL_DATATYPE_DMAT3X2 },
		{ "DMAT3",	OPENGL_SI_SYMBOL_DATATYPE_DMAT3 },
		{ "DMAT3X4",	OPENGL_SI_SYMBOL_DATATYPE_DMAT3X4 },
		{ "DMAT4X2",	OPENGL_SI_SYMBOL_DATATYPE_DMAT4X2 },
		{ "DMAT4X3",	OPENGL_SI_SYMBOL_DATATYPE_DMAT4X3 },
		{ "DMAT4",	OPENGL_SI_SYMBOL_DATATYPE_DMAT4 },
		{ "F16MAT2",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT2 },
		{ "F16MAT2X3",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT2X3 },
		{ "F16MAT2X4",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT2X4 },
		{ "F16MAT3X2",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT3X2 },
		{ "F16MAT3",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT3 },
		{ "F16MAT3X4",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT3X4 },
		{ "F16MAT4X2",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT4X2 },
		{ "F16MAT4X3",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT4X3 },
		{ "F16MAT4",	OPENGL_SI_SYMBOL_DATATYPE_F16MAT4 },
		{ "SAMPLER_1D",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D },
		{ "SAMPLER_2D",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D },
		{ "SAMPLER_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_RECT },
		{ "SAMPLER_EXTERNAL",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_EXTERNAL },
		{ "SAMPLER_3D",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_3D },
		{ "SAMPLER_CUBE",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE },
		{ "SAMPLER_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D_ARRAY },
		{ "SAMPLER_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_ARRAY },
		{ "SAMPLER_CUBE_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE_ARRAY },
		{ "SAMPLER_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_BUFFER },
		{ "SAMPLER_RENDERBUFFER",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_RENDERBUFFER },
		{ "SAMPLER_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_MS },
		{ "SAMPLER_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_MS_ARRAY },
		{ "SAMPLER_1D_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D_SHADOW },
		{ "SAMPLER_2D_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_SHADOW },
		{ "SAMPLER_2D_RECT_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_RECT_SHADOW },
		{ "SAMPLER_CUBE_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE_SHADOW },
		{ "SAMPLER_1D_ARRAY_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D_ARRAY_SHADOW },
		{ "SAMPLER_2D_ARRAY_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_ARRAY_SHADOW },
		{ "SAMPLER_CUBE_ARRAY_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE_ARRAY_SHADOW },
		{ "INT_SAMPLER_1D",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_1D },
		{ "INT_SAMPLER_2D",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D },
		{ "INT_SAMPLER_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_RECT },
		{ "INT_SAMPLER_3D",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_3D },
		{ "INT_SAMPLER_CUBE",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_CUBE },
		{ "INT_SAMPLER_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_1D_ARRAY },
		{ "INT_SAMPLER_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_ARRAY },
		{ "INT_SAMPLER_CUBE_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_CUBE_ARRAY },
		{ "INT_SAMPLER_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_BUFFER },
		{ "INT_SAMPLER_RENDERBUFFER",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_RENDERBUFFER },
		{ "INT_SAMPLER_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_MS },
		{ "INT_SAMPLER_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_MS_ARRAY },
		{ "UNSIGNED_INT_SAMPLER_1D",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_1D },
		{ "UNSIGNED_INT_SAMPLER_2D",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D },
		{ "UNSIGNED_INT_SAMPLER_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_RECT },
		{ "UNSIGNED_INT_SAMPLER_3D",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_3D },
		{ "UNSIGNED_INT_SAMPLER_CUBE",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE },
		{ "UNSIGNED_INT_SAMPLER_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_1D_ARRAY },
		{ "UNSIGNED_INT_SAMPLER_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_ARRAY },
		{ "UNSIGNED_INT_SAMPLER_CUBE_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE_ARRAY },
		{ "UNSIGNED_INT_SAMPLER_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_BUFFER },
		{ "UNSIGNED_INT_SAMPLER_RENDERBUFFER",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_RENDERBUFFER },
		{ "UNSIGNED_INT_SAMPLER_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_MS },
		{ "UNSIGNED_INT_SAMPLER_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_MS_ARRAY },
		{ "FLOAT16_SAMPLER_1D",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D },
		{ "FLOAT16_SAMPLER_2D",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D },
		{ "FLOAT16_SAMPLER_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_RECT },
		{ "FLOAT16_SAMPLER_3D",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_3D },
		{ "FLOAT16_SAMPLER_CUBE",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE },
		{ "FLOAT16_SAMPLER_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_ARRAY },
		{ "FLOAT16_SAMPLER_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_ARRAY },
		{ "FLOAT16_SAMPLER_CUBE_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_ARRAY },
		{ "FLOAT16_SAMPLER_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_BUFFER },
		{ "FLOAT16_SAMPLER_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_MS },
		{ "FLOAT16_SAMPLER_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_MS_ARRAY },
		{ "FLOAT16_SAMPLER_1D_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_SHADOW },
		{ "FLOAT16_SAMPLER_2D_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_SHADOW },
		{ "FLOAT16_SAMPLER_2D_RECT_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_RECT_SHADOW },
		{ "FLOAT16_SAMPLER_CUBE_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_SHADOW },
		{ "FLOAT16_SAMPLER_1D_ARRAY_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_ARRAY_SHADOW },
		{ "FLOAT16_SAMPLER_2D_ARRAY_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_ARRAY_SHADOW },
		{ "FLOAT16_SAMPLER_CUBE_ARRAY_SHADOW",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_ARRAY_SHADOW },
		{ "IMAGE_1D",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_1D },
		{ "IMAGE_2D",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D },
		{ "IMAGE_3D",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_3D },
		{ "IMAGE_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_RECT },
		{ "IMAGE_CUBEMAP",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_CUBEMAP },
		{ "IMAGE_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_BUFFER },
		{ "IMAGE_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_1D_ARRAY },
		{ "IMAGE_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_ARRAY },
		{ "IMAGE_CUBEMAP_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_CUBEMAP_ARRAY },
		{ "IMAGE_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_MS },
		{ "IMAGE_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_MS_ARRAY },
		{ "INT_IMAGE_1D",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_1D },
		{ "INT_IMAGE_2D",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D },
		{ "INT_IMAGE_3D",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_3D },
		{ "INT_IMAGE_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_RECT },
		{ "INT_IMAGE_CUBEMAP",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_CUBEMAP },
		{ "INT_IMAGE_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_BUFFER },
		{ "INT_IMAGE_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_1D_ARRAY },
		{ "INT_IMAGE_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_ARRAY },
		{ "INT_IMAGE_CUBEMAP_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_CUBEMAP_ARRAY },
		{ "INT_IMAGE_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_MS },
		{ "INT_IMAGE_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_MS_ARRAY },
		{ "UNSIGNED_INT_IMAGE_1D",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_1D },
		{ "UNSIGNED_INT_IMAGE_2D",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D },
		{ "UNSIGNED_INT_IMAGE_3D",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_3D },
		{ "UNSIGNED_INT_IMAGE_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_RECT },
		{ "UNSIGNED_INT_IMAGE_CUBEMAP",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_CUBEMAP },
		{ "UNSIGNED_INT_IMAGE_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_BUFFER },
		{ "UNSIGNED_INT_IMAGE_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_1D_ARRAY },
		{ "UNSIGNED_INT_IMAGE_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_ARRAY },
		{ "UNSIGNED_INT_IMAGE_CUBEMAP_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_CUBEMAP_ARRAY },
		{ "UNSIGNED_INT_IMAGE_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_MS },
		{ "UNSIGNED_INT_IMAGE_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_MS_ARRAY },
		{ "FLOAT16_IMAGE_1D",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_1D },
		{ "FLOAT16_IMAGE_2D",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D },
		{ "FLOAT16_IMAGE_3D",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_3D },
		{ "FLOAT16_IMAGE_2D_RECT",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_RECT },
		{ "FLOAT16_IMAGE_CUBEMAP",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_CUBEMAP },
		{ "FLOAT16_IMAGE_BUFFER",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_BUFFER },
		{ "FLOAT16_IMAGE_1D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_1D_ARRAY },
		{ "FLOAT16_IMAGE_2D_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_ARRAY },
		{ "FLOAT16_IMAGE_CUBEMAP_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_CUBEMAP_ARRAY },
		{ "FLOAT16_IMAGE_2D_MS",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_MS },
		{ "FLOAT16_IMAGE_2D_MS_ARRAY",	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_MS_ARRAY },
		{ "SAMPLER_BUFFER_AMD",	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_BUFFER_AMD },
		{ "INT_SAMPLER_BUFFER_AMD",	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_BUFFER_AMD },
		{ "UNSIGNED_INT_SAMPLER_BUFFER_AMD",	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_BUFFER_AMD },
		{ "ATOMIC_COUNTER",	OPENGL_SI_SYMBOL_DATATYPE_ATOMIC_COUNTER },
		{ "STRUCT",	OPENGL_SI_SYMBOL_DATATYPE_STRUCT },
		{ "INTERFACE",	OPENGL_SI_SYMBOL_DATATYPE_INTERFACE },
	}
};

static struct str_map_t enc_dict_semantic_input_type_map =
{
	1, {
		{ "generic",	0 },
	}
};

static struct str_map_t enc_dict_semantic_output_type_map =
{
	7, {
		{ "generic",	0 },
		{ "unknown 1",	1 },
		{ "unknown 2",	2 },
		{ "unknown 3",	3 },
		{ "unknown 4",	4 },
		{ "unknown 5",	5 },
		{ "unknown 6",	6 },
	}
};

static struct str_map_t enc_dict_user_elements_type_map = 
{
	30, {
		{"unknown 0", 0},
		{"unknown 1", 1},
		{"unknown 2", 2},
		{"unknown 3", 3},
		{"unknown 4", 4},
		{"unknown 5", 5},
		{"unknown 6", 6},
		{"unknown 7", 7},
		{"unknown 8", 8},
		{"unknown 9", 9},
		{"unknown 10", 10},
		{"unknown 11", 11},
		{"unknown 12", 12},
		{"unknown 13", 13},
		{"unknown 14", 14},
		{"unknown 15", 15},
		{"SUB_PTR_FETCH_SHADER", 16},
		{"unknown 0", 17},
		{"unknown 0",18},
		{"unknown 0",19},
		{"unknown 0", 20},
		{"PTR_VERTEX_BUFFER_TABLE", 21},
		{"unknown 0", 22},
		{"unknown 0", 23},
		{"unknown 0", 24},
		{"unknown 0", 25},
		{"unknown 0", 26},
		{"unknown 0", 27},
		{"unknown 0", 28},
		{"unknown 0", 29},
	}
};

static struct str_map_t enc_dict_symbol_type_map =
{
	78, {
		{ "GLSL uniform", OPENGL_SI_SYMBOL_UNIFORM },
		{ "GLSL bindable uniform", OPENGL_SI_SYMBOL_BINDABLE_UNIFORM },
		{ "GLSL uniform block", OPENGL_SI_SYMBOL_UNIFORM_BLOCK },
		{ "Vertex position", OPENGL_SI_SYMBOL_ATTRIB_POSITION },
		{ "Vertex normal", OPENGL_SI_SYMBOL_ATTRIB_NORMAL },
		{ "Vertex primary color", OPENGL_SI_SYMBOL_ATTRIB_PRIMARY_COLOR },
		{ "Vertex secondary color", OPENGL_SI_SYMBOL_ATTRIB_SECONDARY_COLOR },
		{ "Vertex fog coord", OPENGL_SI_SYMBOL_ATTRIB_FOGCOORD },
		{ "Vertex edge flag", OPENGL_SI_SYMBOL_ATTRIB_EDGEFLAG },
		{ "Vertex texture coord 0", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD0 },
		{ "Vertex texture coord 1", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD1 },
		{ "Vertex texture coord 2", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD2 },
		{ "Vertex texture coord 3", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD3 },
		{ "Vertex texture coord 4", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD4 },
		{ "Vertex texture coord 5", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD5 },
		{ "Vertex texture coord 6", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD6 },
		{ "Vertex texture coord 7", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD7 },
		{ "Vertex texture coord 8", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD8 },
		{ "Vertex texture coord 9", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD9 },
		{ "Vertex texture coord 10", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD10 },
		{ "Vertex texture coord 11", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD11 },
		{ "Vertex texture coord 12", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD12 },
		{ "Vertex texture coord 13", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD13 },
		{ "Vertex texture coord 14", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD14 },
		{ "Vertex texture coord 15", OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD15 },
		{ "Generic attrib 0", OPENGL_SI_SYMBOL_ATTRIB_GENERIC0 },
		{ "Generic attrib 1", OPENGL_SI_SYMBOL_ATTRIB_GENERIC1 },
		{ "Generic attrib 2", OPENGL_SI_SYMBOL_ATTRIB_GENERIC2 },
		{ "Generic attrib 3", OPENGL_SI_SYMBOL_ATTRIB_GENERIC3 },
		{ "Generic attrib 4", OPENGL_SI_SYMBOL_ATTRIB_GENERIC4 },
		{ "Generic attrib 5", OPENGL_SI_SYMBOL_ATTRIB_GENERIC5 },
		{ "Generic attrib 6", OPENGL_SI_SYMBOL_ATTRIB_GENERIC6 },
		{ "Generic attrib 7", OPENGL_SI_SYMBOL_ATTRIB_GENERIC7 },
		{ "Generic attrib 8", OPENGL_SI_SYMBOL_ATTRIB_GENERIC8 },
		{ "Generic attrib 9", OPENGL_SI_SYMBOL_ATTRIB_GENERIC9 },
		{ "Generic attrib 10", OPENGL_SI_SYMBOL_ATTRIB_GENERIC10 },
		{ "Generic attrib 11", OPENGL_SI_SYMBOL_ATTRIB_GENERIC11 },
		{ "Generic attrib 12", OPENGL_SI_SYMBOL_ATTRIB_GENERIC12 },
		{ "Generic attrib 13", OPENGL_SI_SYMBOL_ATTRIB_GENERIC13 },
		{ "Generic attrib 14", OPENGL_SI_SYMBOL_ATTRIB_GENERIC14 },
		{ "Generic attrib 15", OPENGL_SI_SYMBOL_ATTRIB_GENERIC15 },
		{ "Generic attrib 16", OPENGL_SI_SYMBOL_ATTRIB_GENERIC16 },
		{ "Generic attrib 17", OPENGL_SI_SYMBOL_ATTRIB_GENERIC17 },
		{ "Generic attrib 18", OPENGL_SI_SYMBOL_ATTRIB_GENERIC18 },
		{ "Generic attrib 19", OPENGL_SI_SYMBOL_ATTRIB_GENERIC19 },
		{ "Generic attrib 20", OPENGL_SI_SYMBOL_ATTRIB_GENERIC20 },
		{ "Generic attrib 21", OPENGL_SI_SYMBOL_ATTRIB_GENERIC21 },
		{ "Generic attrib 22", OPENGL_SI_SYMBOL_ATTRIB_GENERIC22 },
		{ "Generic attrib 23", OPENGL_SI_SYMBOL_ATTRIB_GENERIC23 },
		{ "Generic attrib 24", OPENGL_SI_SYMBOL_ATTRIB_GENERIC24 },
		{ "Generic attrib 25", OPENGL_SI_SYMBOL_ATTRIB_GENERIC25 },
		{ "Generic attrib 26", OPENGL_SI_SYMBOL_ATTRIB_GENERIC26 },
		{ "Generic attrib 27", OPENGL_SI_SYMBOL_ATTRIB_GENERIC27 },
		{ "Generic attrib 28", OPENGL_SI_SYMBOL_ATTRIB_GENERIC28 },
		{ "Generic attrib 29", OPENGL_SI_SYMBOL_ATTRIB_GENERIC29 },
		{ "Generic attrib 30", OPENGL_SI_SYMBOL_ATTRIB_GENERIC30 },
		{ "Generic attrib 31", OPENGL_SI_SYMBOL_ATTRIB_GENERIC31 },
		{ "Generic attrib 32", OPENGL_SI_SYMBOL_ATTRIB_GENERIC32 },
		{ "Generic attrib 33", OPENGL_SI_SYMBOL_ATTRIB_GENERIC33 },
		{ "Generic attrib 34", OPENGL_SI_SYMBOL_ATTRIB_GENERIC34 },
		{ "Generic attrib 35", OPENGL_SI_SYMBOL_ATTRIB_GENERIC35 },
		{ "Generic attrib 36", OPENGL_SI_SYMBOL_ATTRIB_GENERIC36 },
		{ "Generic attrib 37", OPENGL_SI_SYMBOL_ATTRIB_GENERIC37 },
		{ "Generic attrib 38", OPENGL_SI_SYMBOL_ATTRIB_GENERIC38 },
		{ "Generic attrib 39", OPENGL_SI_SYMBOL_ATTRIB_GENERIC39 },
		{ "Varying out", OPENGL_SI_SYMBOL_FSVARYINGOUT },
		{ "VertexID", OPENGL_SI_SYMBOL_ATTRIB_VERTEXID },
		{ "InstanceID", OPENGL_SI_SYMBOL_ATTRIB_INSTANCEID },
		{ "GLSL Subroutine Uniform", OPENGL_SI_SYMBOL_SUBROUTINE_UNIFORM },
		{ "GLSL Subroutine Function", OPENGL_SI_SYMBOL_SUBROUTINE_FUNCTION },
		{ "base varying in symbol of separate mode", OPENGL_SI_SYMBOL_SEPARATE_VARYING_IN_BASE },
		{ "base varying out symbol of sparate mode", OPENGL_SI_SYMBOL_SEPARATE_VARYING_OUT_BASE },
		{ "derived varying in symbol of separate mode", OPENGL_SI_SYMBOL_SEPARATE_VARYING_IN_DERIVED },
		{ "derived varying out symbol of sparate mode", OPENGL_SI_SYMBOL_SEPARATE_VARYING_OUT_DERIVED },
		{ "varying in of nonseparate mode", OPENGL_SI_SYMBOL_NONSEPARATE_VARYING_IN },
		{ "varying out of nonseparate mode", OPENGL_SI_SYMBOL_NONSEPARATE_VARYING_OUT },
		{ "shader storage buffer", OPENGL_SI_SYMBOL_SHADER_STORAGE_BUFFER },
		{ "input valide bits for sparse texture", OPENGL_SI_SYMBOL_ATTRIB_INPUTVALID },
	}
};

static int opengl_si_shader_binary_get_isa_offset(struct opengl_si_shader_binary_t *shdr)
{
	struct opengl_si_enc_dict_vertex_shader_t *vs_enc;
	struct opengl_si_enc_dict_pixel_shader_t *ps_enc;
	int isa_offset;

	switch (shdr->shader_kind)
	{
		case OPENGL_SI_SHADER_VERTEX:
		{
			vs_enc = (struct opengl_si_enc_dict_vertex_shader_t *)shdr->enc_dict;
			isa_offset = vs_enc->meta->uSizeInBytes;
			break;
		}
		case OPENGL_SI_SHADER_PIXEL:
		{
			ps_enc = (struct opengl_si_enc_dict_pixel_shader_t *)shdr->enc_dict;
			isa_offset = ps_enc->meta->uSizeInBytes;
			break;
		}
		case OPENGL_SI_SHADER_GEOMETRY:
		{
			isa_offset = 3976;
			break;
		}
		default:
			isa_offset = 0;
	}

	return isa_offset;	
}

/* Dummy callback function */
static void opengl_si_bin_shader_free(void *shader)
{
}

static void opengl_si_shader_binary_set(struct opengl_si_shader_binary_t *shdr)
{
	switch(shdr->elf->header->e_flags)
	{
	case 0x0:
		shdr->shader_kind = OPENGL_SI_SHADER_VERTEX;
		shdr->enc_dict = opengl_si_enc_dict_vertex_shader_create(shdr);
		shdr->free_func = (opengl_si_shader_bin_free_func_t) &opengl_si_bin_vertex_shader_free;
		opengl_si_bin_vertex_shader_init(shdr->enc_dict);
		break;
	case 0x4:
		shdr->shader_kind = OPENGL_SI_SHADER_PIXEL;
		shdr->enc_dict = opengl_si_enc_dict_pixel_shader_create(shdr);
		shdr->free_func = (opengl_si_shader_bin_free_func_t) &opengl_si_bin_pixel_shader_free;
		opengl_si_bin_pixel_shader_init(shdr->enc_dict);		
		break;
	default:
		shdr->shader_kind = OPENGL_SI_SHADER_INVALID;
		shdr->free_func = (opengl_si_shader_bin_free_func_t) &opengl_si_bin_shader_free;
		break;
	}
}

static void opengl_si_shader_binary_set_isa(struct opengl_si_shader_binary_t *shdr)
{
	struct elf_section_t *section;
	int offset;
	int i;

	/* The ISA is in .text section */
	offset = opengl_si_shader_binary_get_isa_offset(shdr);
	LIST_FOR_EACH(shdr->elf->section_list, i)
	{
		section = list_get(shdr->elf->section_list, i);
		if (!strcmp(section->name, ".text"))
		{
			shdr->isa = xcalloc(1, sizeof(struct elf_buffer_t));
			shdr->isa->ptr = section->buffer.ptr + offset;
			shdr->isa->size = section->buffer.size - offset;
			shdr->isa->pos = 0;
		}
	}
}

/* Structure in .text section for VS */
static struct opengl_si_bin_vertex_shader_metadata_t *opengl_si_bin_vertex_shader_metadata_create()
{
	struct opengl_si_bin_vertex_shader_metadata_t *vs_meta;

	/* Allocate */
	vs_meta = xcalloc(1, sizeof(struct opengl_si_bin_vertex_shader_metadata_t));

	/* Return */
	return vs_meta;
}

static void opengl_si_bin_vertex_shader_metadata_free(struct opengl_si_bin_vertex_shader_metadata_t *vs_meta)
{
	/* Free */
	free(vs_meta);
}

static struct opengl_si_bin_vertex_shader_metadata_t *opengl_si_bin_vertex_shader_metadata_init_from_section(
	struct opengl_si_bin_vertex_shader_metadata_t *vs_meta, 
	struct elf_section_t *section)
{
	/* Make sure section is correct */
	assert(!strcmp(section->name, ".text"));

	/* Create and memcpy */
	memcpy(vs_meta, section->buffer.ptr, sizeof(struct opengl_si_bin_vertex_shader_metadata_t));
	if(sizeof(struct opengl_si_bin_vertex_shader_metadata_t) + vs_meta->CodeLenInByte != section->buffer.size)
		warning("Vertex Shader .text section size(%d) != metadata structure size(%zu) + ISA size(%d).",
			section->buffer.size, sizeof(struct opengl_si_bin_vertex_shader_metadata_t), 
			vs_meta->CodeLenInByte);

	/* Return */
	return vs_meta;
}

/* Structure in .text section for FS */
static struct opengl_si_bin_pixel_shader_metadata_t *opengl_si_bin_pixel_shader_metadata_create()
{
	struct opengl_si_bin_pixel_shader_metadata_t *fs_meta;

	/* Allocate */
	fs_meta = xcalloc(1, sizeof(struct opengl_si_bin_pixel_shader_metadata_t));

	/* Return */
	return fs_meta;
}

static void opengl_si_bin_pixel_shader_metadata_free(struct opengl_si_bin_pixel_shader_metadata_t *fs_meta)
{
	/* Free */
	free(fs_meta);
}

static struct opengl_si_bin_pixel_shader_metadata_t *opengl_si_bin_pixel_shader_metadata_init_from_section(
	struct opengl_si_bin_pixel_shader_metadata_t *fs_meta, 
	struct elf_section_t *section)
{
	/* Make sure section is correct */
	assert(!strcmp(section->name, ".text"));

	/* Create and memcpy */
	memcpy(fs_meta, section->buffer.ptr, sizeof(struct opengl_si_bin_pixel_shader_metadata_t));
	if (sizeof(struct opengl_si_bin_pixel_shader_metadata_t) + fs_meta->CodeLenInByte  != section->buffer.size)
		warning("Pixel Shader .text section size(%d) != metadata structure size(%zu) + ISA size(%d).",
				section->buffer.size, sizeof(struct opengl_si_bin_pixel_shader_metadata_t),
				fs_meta->CodeLenInByte);

	/* Return */
	return fs_meta;
}

/* Structure in .inputs section */
static struct opengl_si_bin_input_t *opengl_si_bin_input_create()
{
	struct opengl_si_bin_input_t *input;

	/* Allocate */
	input = xcalloc(1, sizeof(struct opengl_si_bin_input_t));

	/* Return */
	return input;
}

static void opengl_si_bin_input_free(struct opengl_si_bin_input_t *input)
{
	free(input);
}

static void opengl_si_bin_inputs_init_from_section(struct list_t *lst, struct elf_section_t *section)
{
	struct opengl_si_bin_input_t *input;
	int input_count;
	int i;

	assert(!strcmp(section->name, ".inputs"));

	/* Calculate # of input */
	if (section->buffer.size % sizeof(struct opengl_si_bin_input_t))
		fatal("Section size must be multiples of input structure.");
	else
	{
		input_count = section->buffer.size / sizeof(struct opengl_si_bin_input_t);
		for (i = 0; i < input_count; ++i)
		{
			input = opengl_si_bin_input_create();
			memcpy(input, section->buffer.ptr + i * sizeof(struct opengl_si_bin_input_t), 
				sizeof(struct opengl_si_bin_input_t));
			list_add(lst, input);
		}
	}
}

/* Structure in .outputs section */
static struct opengl_si_bin_output_t *opengl_si_bin_output_create()
{
	struct opengl_si_bin_output_t *output;

	/* Allocate */
	output = xcalloc(1, sizeof(struct opengl_si_bin_output_t));

	/* Return */
	return output;
}

static void opengl_si_bin_output_free(struct opengl_si_bin_output_t *output)
{
	free(output->name);
	free(output);
}

static void opengl_si_bin_outputs_init_from_section(struct list_t *lst, struct elf_section_t *section)
{
	struct opengl_si_bin_output_t *output;
	struct opengl_si_bin_output_t* output_ptr;	
	char *outname;
	char *bin_ptr;
	unsigned int name_offset;
	size_t len;
	int output_count;
	int i;

	/* Make sure section is correct */
	assert(!strcmp(section->name, ".outputs"));

	output_count = section->header->sh_entsize;
	bin_ptr = (char *) section->buffer.ptr;
	name_offset = sizeof(struct opengl_si_bin_output_t) - sizeof(char*);
	for (i = 0; i < output_count; ++i)
	{
		output_ptr = (struct opengl_si_bin_output_t *)bin_ptr;

		output = opengl_si_bin_output_create();
		outname = &bin_ptr[name_offset];
		if(*outname != '\0')
		{
			len = strlen(outname) + 1;
			output->name = xstrdup(outname);
			output->type = output_ptr->type;
			output->data_type = output_ptr->data_type;
			// output->array_size = output_ptr->array_size;
			bin_ptr += (name_offset + len);
		}
		else
		{
			bin_ptr += (name_offset + 1);
			output->name = NULL;
			output->type = output_ptr->type;
			output->data_type = output_ptr->data_type;
			output->voffset = output_ptr->voffset;
			output->poffset = output_ptr->poffset;
			// output->array_size = output_ptr->array_size;
		}
		list_add(lst, output);
	}
}

/* Structure in .info section */
static struct opengl_si_bin_info_t *opengl_si_bin_info_create()
{
	struct opengl_si_bin_info_t *info;

	/* Allocate */
	info = xcalloc(1, sizeof(struct opengl_si_bin_info_t));

	/* Return */	
	return info;
}

static void opengl_si_bin_info_free(struct opengl_si_bin_info_t *info)
{
	free(info);
}

static void opengl_si_bin_info_init_with_section(struct opengl_si_bin_info_t *info, struct elf_section_t *section)
{
	assert(!strcmp(section->name, ".info"));

	if (section->buffer.size != sizeof(struct opengl_si_bin_info_t))
		warning("Section size(%d) doesn't match info structure(%zu).",
			section->buffer.size, sizeof(struct opengl_si_bin_info_t));
	else
		memcpy(info, section->buffer.ptr, sizeof(struct opengl_si_bin_info_t));
}

/* Structure in .usageinfo section */
static struct opengl_si_bin_usageinfo_t *opengl_si_bin_usageinfo_create()
{
	struct opengl_si_bin_usageinfo_t *usageinfo;

	/* Allocate */
	usageinfo = xcalloc(1, sizeof(struct opengl_si_bin_usageinfo_t));

	/* Return */
	return usageinfo;
}

static void opengl_si_bin_usageinfo_free(struct opengl_si_bin_usageinfo_t *usageinfo)
{
	free(usageinfo);
}

static void opengl_si_bin_usageinfo_init_with_section(struct opengl_si_bin_usageinfo_t *usageinfo, struct elf_section_t *section)
{
	assert(!strcmp(section->name, ".usageinfo"));

	if (section->buffer.size != sizeof(struct opengl_si_bin_usageinfo_t))
		warning("Section size(%d) doesn't match usageinfo structure(%zu).",
			section->buffer.size, sizeof(struct opengl_si_bin_usageinfo_t));
	memcpy(usageinfo, section->buffer.ptr, sizeof(struct opengl_si_bin_usageinfo_t));
}

/* Structure in .symbol section */
static struct opengl_si_bin_symbol_t *opengl_si_bin_symbol_create()
{
	struct opengl_si_bin_symbol_t *symbol;

	/* Allocate */
	symbol = xcalloc(1, sizeof(struct opengl_si_bin_symbol_t));

	/* Return */
	return symbol;
}

static void opengl_si_bin_symbol_free(struct opengl_si_bin_symbol_t *symbol)
{
	if (symbol->name)
		free(symbol->name);
	if (symbol->baseName)
		free(symbol->baseName);
	if (symbol->uniformBlockName)
		free(symbol->uniformBlockName);
	if (symbol->mangledName)
		free(symbol->mangledName);
	free(symbol);
}

static void opengl_si_bin_symbol_init_with_section(struct list_t *symbol_list, struct elf_section_t *section)
{
	struct opengl_si_bin_symbol_t *symbol;
	int symbol_count;
	const int symbol_len = 136;
	char *symbol_name;
	void *bin_ptr;
	void *name_ptr;
	size_t len;
	int i;

	/* Make sure section is correct */
	assert(!strcmp(section->name, ".symbols"));

	bin_ptr = section->buffer.ptr;
	memcpy(&symbol_count, bin_ptr, 4);
	bin_ptr += 4;

	for (i = 0; i < symbol_count; ++i)
	{
		symbol = opengl_si_bin_symbol_create();
		memcpy(symbol, bin_ptr, symbol_len);

		name_ptr = bin_ptr + symbol_len;
		symbol_name = (char *)name_ptr;
		if(*symbol_name != '\0')
		{
			len = strlen(symbol_name) + 1;
			symbol->name = xstrdup(symbol_name);
			bin_ptr = name_ptr + len + 3;
		}
		else
		{
			bin_ptr = name_ptr + 3;
			symbol->name = NULL;
		}
		list_add(symbol_list, symbol);
	}
}

int opengl_si_bin_symbol_get_location(struct list_t *symbol_list, char *name)
{
	struct opengl_si_bin_symbol_t *symbol;
	int return_val = -1;
	int i;

	LIST_FOR_EACH(symbol_list, i)
	{
		symbol = list_get(symbol_list, i);
		if (!strcmp(symbol->name, name))
		{
			switch (symbol->type)
			{
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD0:
					return_val = 0;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD1:
					return_val = 1;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD2:
					return_val = 2;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD3:
					return_val = 3;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD4:
					return_val = 4;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD5:
					return_val = 5;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD6:
					return_val = 6;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD7:
					return_val = 7;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD8:
					return_val = 8;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD9:
					return_val = 9;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD10:
					return_val = 10;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD11:
					return_val = 11;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD12:
					return_val = 12;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD13:
					return_val = 13;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD14:
					return_val = 14;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD15:
					return_val = 15;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC0:
					return_val = 0;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC1:
					return_val = 1;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC2:
					return_val = 2;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC3:
					return_val = 3;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC4:
					return_val = 4;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC5:
					return_val = 5;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC6:
					return_val = 6;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC7:
					return_val = 7;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC8:
					return_val = 8;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC9:
					return_val = 9;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC10:
					return_val = 10;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC11:
					return_val = 11;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC12:
					return_val = 12;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC13:
					return_val = 13;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC14:
					return_val = 14;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC15:
					return_val = 15;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC16:
					return_val = 16;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC17:
					return_val = 17;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC18:
					return_val = 18;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC19:
					return_val = 19;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC20:
					return_val = 20;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC21:
					return_val = 21;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC22:
					return_val = 22;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC23:
					return_val = 23;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC24:
					return_val = 24;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC25:
					return_val = 25;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC26:
					return_val = 26;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC27:
					return_val = 27;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC28:
					return_val = 28;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC29:
					return_val = 29;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC30:
					return_val = 20;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC31:
					return_val = 31;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC32:
					return_val = 32;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC33:
					return_val = 33;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC34:
					return_val = 34;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC35:
					return_val = 35;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC36:
					return_val = 36;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC37:
					return_val = 37;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC38:
					return_val = 38;
					break;
				case OPENGL_SI_SYMBOL_ATTRIB_GENERIC39:
					return_val = 39;
					break;
				default:
					return_val = -1;
					break;
			}
		}
	}

	/* Return */
	return return_val;
}

static struct opengl_si_enc_dict_vertex_shader_t *opengl_si_enc_dict_vertex_shader_create(struct opengl_si_shader_binary_t *parent)
{
	struct opengl_si_enc_dict_vertex_shader_t *vs;

	/* Allocate */
	vs = xcalloc(1, sizeof(struct opengl_si_enc_dict_vertex_shader_t));
	vs->parent = parent;
	parent->enc_dict = vs;
	vs->meta = opengl_si_bin_vertex_shader_metadata_create();
	vs->inputs = list_create();
	vs->outputs = list_create();
	vs->info = opengl_si_bin_info_create();
	vs->usageinfo = opengl_si_bin_usageinfo_create();
	vs->symbols = list_create();

	/* Return */
	return vs;
}

static void opengl_si_bin_vertex_shader_free(struct opengl_si_enc_dict_vertex_shader_t *vs)
{
	struct opengl_si_bin_input_t *input;
	struct opengl_si_bin_output_t *output;
	struct opengl_si_bin_symbol_t *symbol;
	int i;

	opengl_si_bin_vertex_shader_metadata_free(vs->meta);

	LIST_FOR_EACH(vs->inputs, i)
	{
		input = list_get(vs->inputs, i);
		opengl_si_bin_input_free(input);
	}
	list_free(vs->inputs);

	LIST_FOR_EACH(vs->outputs, i)
	{
		output = list_get(vs->outputs, i);
		opengl_si_bin_output_free(output);
	}
	list_free(vs->outputs);

	opengl_si_bin_info_free(vs->info);

	opengl_si_bin_usageinfo_free(vs->usageinfo);

	LIST_FOR_EACH(vs->symbols, i)
	{
		symbol = list_get(vs->symbols, i);
		opengl_si_bin_symbol_free(symbol);
	}
	list_free(vs->symbols);

	free(vs);

}

static void opengl_si_bin_vertex_shader_init(struct opengl_si_enc_dict_vertex_shader_t *vs)
{
	struct opengl_si_shader_binary_t *parent;
	struct elf_file_t *elf;
	struct elf_section_t *section;
	int i;

	/* Get parent */
	parent = vs->parent;
	assert(parent);
	assert(parent->shader_kind == OPENGL_SI_SHADER_VERTEX);

	elf = parent->elf;

	/* Initialize from sections */
	LIST_FOR_EACH(elf->section_list, i)
	{
		section = list_get(elf->section_list, i);
		if (!strcmp(section->name, ".text"))
			opengl_si_bin_vertex_shader_metadata_init_from_section(vs->meta, section);
		else if (!strcmp(section->name, ".inputs"))
			opengl_si_bin_inputs_init_from_section(vs->inputs, section);
		else if (!strcmp(section->name, ".outputs"))
			opengl_si_bin_outputs_init_from_section(vs->outputs, section);
		else if (!strcmp(section->name, ".info"))
			opengl_si_bin_info_init_with_section(vs->info, section);
		else if(!strcmp(section->name, ".usageinfo"))
			opengl_si_bin_usageinfo_init_with_section(vs->usageinfo, section);
		else if(!strcmp(section->name, ".symbols"))
			opengl_si_bin_symbol_init_with_section(vs->symbols, section);
	}
}

static struct opengl_si_enc_dict_pixel_shader_t *opengl_si_enc_dict_pixel_shader_create(struct opengl_si_shader_binary_t *parent)
{
	struct opengl_si_enc_dict_pixel_shader_t *ps;

	/* Allocate */
	ps = xcalloc(1, sizeof(struct opengl_si_enc_dict_pixel_shader_t));
	ps->parent = parent;
	parent->enc_dict = ps;
	ps->meta = opengl_si_bin_pixel_shader_metadata_create();
	ps->inputs = list_create();
	ps->outputs = list_create();
	ps->info = opengl_si_bin_info_create();
	ps->usageinfo = opengl_si_bin_usageinfo_create();
	ps->symbols = list_create();

	/* Return */
	return ps;
}

static void opengl_si_bin_pixel_shader_free(struct opengl_si_enc_dict_pixel_shader_t *ps)
{
	struct opengl_si_bin_input_t *input;
	struct opengl_si_bin_output_t *output;
	struct opengl_si_bin_symbol_t *symbol;
	int i;

	opengl_si_bin_pixel_shader_metadata_free(ps->meta);
	LIST_FOR_EACH(ps->inputs, i)
	{
		input = list_get(ps->inputs, i);
		opengl_si_bin_input_free(input);
	}
	list_free(ps->inputs);
	LIST_FOR_EACH(ps->outputs, i)
	{
		output = list_get(ps->outputs, i);
		opengl_si_bin_output_free(output);
	}
	list_free(ps->outputs);

	opengl_si_bin_info_free(ps->info);

	opengl_si_bin_usageinfo_free(ps->usageinfo);

	LIST_FOR_EACH(ps->symbols, i)
	{
		symbol = list_get(ps->symbols, i);
		opengl_si_bin_symbol_free(symbol);
	}
	list_free(ps->symbols);	

	free(ps);

}

static void opengl_si_bin_pixel_shader_init(struct opengl_si_enc_dict_pixel_shader_t *ps)
{
	struct opengl_si_shader_binary_t *parent;
	struct elf_file_t *elf;
	struct elf_section_t *section;
	int i;

	/* Get parent */
	parent = ps->parent;
	assert(parent);
	assert(parent->shader_kind == OPENGL_SI_SHADER_PIXEL);

	elf = parent->elf;

	/* Initialize from sections */
	LIST_FOR_EACH(elf->section_list, i)
	{
		section = list_get(elf->section_list, i);
		if (!strcmp(section->name, ".text"))
			opengl_si_bin_pixel_shader_metadata_init_from_section(ps->meta, section);
		else if (!strcmp(section->name, ".inputs"))
			opengl_si_bin_inputs_init_from_section(ps->inputs, section);
		else if (!strcmp(section->name, ".outputs"))
			opengl_si_bin_outputs_init_from_section(ps->outputs, section);
		else if (!strcmp(section->name, ".info"))
			opengl_si_bin_info_init_with_section(ps->info, section);
		else if(!strcmp(section->name, ".usageinfo"))
			opengl_si_bin_usageinfo_init_with_section(ps->usageinfo, section);
	}

}

static struct list_t *opengl_si_shader_bin_list_create(struct elf_file_t *binary)
{
	struct list_t *lst;
	struct opengl_si_shader_binary_t *shader;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;
	int i;

	/* Create */
	lst = list_create();

	/* Add shaders to the list */
	LIST_FOR_EACH(binary->symbol_table, i)
	{
		symbol = list_get(binary->symbol_table, i);
		if (str_suffix(symbol->name, "ElfBinary_0_"))
		{
			section = list_get(binary->section_list, symbol->section);
			shader = opengl_si_shader_binary_create(section->buffer.ptr + symbol->value, 
				symbol->size, 
				symbol->name);
			list_add(lst, shader);
		}
	}

	/* Return */
	return lst;
}

static void opengl_si_shader_bin_list_free(struct list_t *shaders)
{
	struct opengl_si_shader_binary_t *shdr;
	int i;

	/* Free */
	LIST_FOR_EACH(shaders, i)
	{
		shdr = list_get(shaders, i);
		opengl_si_shader_binary_free(shdr);
	}

	list_free(shaders);
}

/*
 * Public functions
 */

struct opengl_si_program_binary_t *opengl_si_program_binary_create(const char *buffer_ptr,
		int size, const char *name)
{
	struct opengl_si_program_binary_t *program_bin;

	/* Allocate */
	program_bin = xcalloc(1, sizeof(struct opengl_si_program_binary_t));

	/* Initialize */
	program_bin->name = xstrdup(name);
	program_bin->binary = elf_file_create_from_buffer((char *) buffer_ptr, size, (char *) name);
	if (!program_bin->binary)
		fatal("Program binary is not ELF formatted, please upgrade your AMD Catalyst driver to version >= 13.1.");
	program_bin->shader_bins = opengl_si_shader_bin_list_create(program_bin->binary);

	/* Return */	
	return program_bin;
}

void opengl_si_program_binary_free(struct opengl_si_program_binary_t *program_bin)
{
	/* Free shader binary */
	free(program_bin->name);
	elf_file_free(program_bin->binary);
	opengl_si_shader_bin_list_free(program_bin->shader_bins);

	free(program_bin);
}

struct opengl_si_shader_binary_t *opengl_si_shader_binary_create(void *buffer, int size, char* name)
{
	struct opengl_si_shader_binary_t *shdr;

	/* Allocate */
	shdr = xcalloc(1, sizeof(struct opengl_si_shader_binary_t));

	/* Initialize */
	shdr->elf = elf_file_create_from_buffer(buffer, size, name);
	if (shdr->elf)
	{
		opengl_si_shader_binary_set(shdr);
		opengl_si_shader_binary_set_isa(shdr);
	}

	/* Return */
	return shdr;
}

void opengl_si_shader_binary_free(struct opengl_si_shader_binary_t *shdr)
{
	shdr->free_func(shdr->enc_dict);
	elf_file_free(shdr->elf);
	free(shdr->isa);
	free(shdr);
}

struct SIBinaryUserElement *opengl_si_bin_enc_user_element_create()
{
	struct SIBinaryUserElement *user_elem;

	/* Initialize */
	user_elem = xcalloc(1, sizeof(struct SIBinaryUserElement));
	
	/* Return */
	return user_elem;
}

void opengl_si_bin_enc_user_element_free(struct SIBinaryUserElement *user_elem)
{
	free(user_elem);
}

void opengl_si_shader_binary_debug_meta(struct opengl_si_shader_binary_t *shdr_bin)
{
	int i;
	struct list_t *input_list;
	struct opengl_si_bin_input_t *input;
	struct list_t *output_list;
	struct opengl_si_bin_output_t *output;
	struct list_t *symbol_list;
	struct opengl_si_bin_symbol_t *symbol;
	struct opengl_si_enc_dict_vertex_shader_t *enc_vs;
	struct opengl_si_enc_dict_pixel_shader_t *enc_ps;
	struct opengl_si_bin_vertex_shader_metadata_t *meta_vs;
	struct opengl_si_bin_pixel_shader_metadata_t *meta_ps;

	switch(shdr_bin->shader_kind)
	{

	case OPENGL_SI_SHADER_VERTEX:
	{
		enc_vs = (struct opengl_si_enc_dict_vertex_shader_t *)shdr_bin->enc_dict;
		meta_vs = enc_vs->meta;
		printf("-----------------------VS Data -------------------------\n");
		printf("Input info\n");
		input_list = enc_vs->inputs;
		LIST_FOR_EACH(input_list, i)
		{
			input = list_get(input_list, i);
			printf(" [%d], %s, virtual offset = %d, physical offset = %d, swizzles = %s, %s, %s, %s\n", 
				i, 
				str_map_value(&enc_dict_input_type_map, input->type), 
				input->voffset, input->poffset, 
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[0]),
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[1]),
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[2]),
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[3])
				);
		}
		printf("\n");
		printf("Output info\n");
		output_list = enc_vs->outputs;
		LIST_FOR_EACH(output_list, i)
		{
			output = list_get(output_list, i);
			printf(" [%d], %s, %s, virtual offset = %d, physical offset = %d, name = %s\n", 
				i, str_map_value(&enc_dict_output_type_map, output->type), 
				str_map_value(&enc_dict_symbol_datatype_map, output->data_type),
				output->voffset, output->poffset, output->name);
		}
		printf("\n");
		printf("Symbol info\n");
		symbol_list = enc_vs->symbols;
		LIST_FOR_EACH(symbol_list, i)
		{
			symbol = list_get(symbol_list, i);
			printf(" [%d] %s, %s, %s\n", i, symbol->name, 
				str_map_value(&enc_dict_symbol_type_map, symbol->type), 
				str_map_value(&enc_dict_symbol_datatype_map, symbol->dataType));
		}
		printf("\n");
		printf("Input Semantic Mappings\n");
		for (i = 0; i < meta_vs->numVsInSemantics; ++i)
		{
			printf(" [%d] %s, usageIdx %d, v[%d:%d]\n", i, 
				str_map_value(&enc_dict_semantic_input_type_map, meta_vs->vsInSemantics[i].usage), 
				meta_vs->vsInSemantics[i].usageIdx, meta_vs->vsInSemantics[i].dataVgpr, 
				meta_vs->vsInSemantics[i].dataVgpr + 3);
		}
		printf("Output Semantic Mappings\n");
		for (i = 0; i < meta_vs->numVsOutSemantics; ++i)
		{
			printf(" [%d] %s, usageIdx %d, paramIdx %d\n", i, 
				str_map_value(&enc_dict_semantic_output_type_map, meta_vs->vsOutSemantics[i].usage), 
				meta_vs->vsOutSemantics[i].usageIdx, meta_vs->vsOutSemantics[i].paramIdx);
		}
		printf("\n");
		printf("codeLenInByte\t= %d;Bytes\n", meta_vs->CodeLenInByte);
		printf("\n");
		printf("userElementCount\t= %d\n", meta_vs->u32UserElementCount);
		for (i = 0; i < meta_vs->u32UserElementCount; ++i)
		{
			printf(" userElements[%d]\t= %s, %d, s[%d,%d]\n", i, 
				str_map_value(&enc_dict_user_elements_type_map, meta_vs->pUserElement[i].dataClass), 
				meta_vs->pUserElement[i].apiSlot, meta_vs->pUserElement[i].startUserReg, 
				meta_vs->pUserElement[i].startUserReg + meta_vs->pUserElement[i].userRegCount  - 1);
		}
		printf("NumVgprs\t\t= %d\n", meta_vs->u32NumVgprs);
		printf("NumSgprs\t\t= %d\n", meta_vs->u32NumSgprs);
		printf("FloatMode\t\t= %d\n", meta_vs->u32FloatMode);
		printf("IeeeMode\t\t= %d\n", meta_vs->bIeeeMode);
		printf("UsesPrimID\t\t= %d\n", meta_vs->bUsesPrimId);
		printf("UsesVertexID\t\t= %d\n", meta_vs->bUsesVertexId);
		printf("ScratchSize\t\t= %d\n", meta_vs->scratchSize);
		for (i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
		{
			printf(" texResourceUsage[%d]\t\t= 0x%x\n", i, meta_vs->texResourceUsage[i]);
		}
		for (i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
		{
			printf(" fetch4ResourceUsage[%d]\t\t= 0x%x\n", i, meta_vs->fetch4ResourceUsage[i]);
		}
		for (i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
		{
			printf(" uavResourceUsage[%d]\t\t= 0x%x\n", i, meta_vs->uavResourceUsage[i]);
		}
		printf(" texSamplerUsage\t\t= 0x%x\n", meta_vs->texSamplerUsage);
		printf(" constBufUsage\t\t\t= 0x%x\n", meta_vs->constBufUsage);
		printf("SPI_SHADER_PGM_RSRC2_VS\t\t= 0x%08X\n", meta_vs->spiShaderPgmRsrc2Es);
		printf(" SSPRV:USER_SGPR\t\t= %d\n", meta_vs->spiShaderPgmRsrc2Vs.user_sgpr);
		printf("PA_CL_VS_OUT_CNTL\t\t= 0x%x\n", meta_vs->paClVsOutCntl);
		printf("SPI_VS_OUT_CONFIG\t\t= 0x%08X\n", meta_vs->spiVsOutConfigAsUint);
		printf(" SVOC:VS_EXPORT_COUNT\t\t= %d\n", meta_vs->spiVsOutConfig.vs_export_count);
		printf(" SVOC:VS_HALF_PACK\t\t= %d\n", meta_vs->spiVsOutConfig.vs_half_pack);
		printf(" SVOC:VS_EXPORTS_FOG\t\t= %d\n", meta_vs->spiVsOutConfig.vs_export_fog);
		printf(" SVOC:VS_OUT_FOG_VEC_ADDR\t= %d\n", meta_vs->spiVsOutConfig.vs_out_fog_vec_addr);
		printf("SPI_SHADER_POS_FORMAT\t\t= 0x%08X\n", meta_vs->spiShaderPosFormatAsUint);
		printf(" SSPF:PSO0_EXPORT_FORMAT\t= %d\n", meta_vs->spiShaderPosFormat.pos0_export_format);
		printf(" SSPF:PSO1_EXPORT_FORMAT\t= %d\n", meta_vs->spiShaderPosFormat.pos1_export_format);
		printf(" SSPF:PSO2_EXPORT_FORMAT\t= %d\n", meta_vs->spiShaderPosFormat.pos2_export_format);
		printf(" SSPF:PSO3_EXPORT_FORMAT\t= %d\n", meta_vs->spiShaderPosFormat.pos3_export_format);
		printf("VGT_STRMOUT_CONFIG\t\t= 0x%08X\n", meta_vs->vgtStrmoutConfig);
		printf(" VGT_STRMOUT_CONFIG:RAST_STREAM\t= %d\n", 0);
		printf(" VGT_STRMOUT_CONFIG:STREAMOUT_0_EN = %d\n", 0);
		printf(" VGT_STRMOUT_CONFIG:STREAMOUT_1_EN = %d\n", 0);
		printf(" VGT_STRMOUT_CONFIG:STREAMOUT_2_EN = %d\n", 0);
		printf(" VGT_STRMOUT_CONFIG:STREAMOUT_3_EN = %d\n", 0);
		printf("vgprCompCnt\t\t\t= %d\n", meta_vs->vgprCompCnt);
		printf("exportVertexSize\t\t= %d\n", meta_vs->exportVertexSize);
		break;
	}
	case OPENGL_SI_SHADER_PIXEL:
	{
		enc_ps = (struct opengl_si_enc_dict_pixel_shader_t *)shdr_bin->enc_dict;
		meta_ps = enc_ps->meta;
		printf("-----------------------PS Data -------------------------\n");
		printf("Input info\n");
		input_list = enc_ps->inputs;
		LIST_FOR_EACH(input_list, i)
		{
			input = list_get(input_list, i);
			printf(" [%d] %s, virtual offset = %d, physical offset = %d, swizzles = %s, %s, %s, %s\n", 
				i, 
				str_map_value(&enc_dict_input_type_map, input->type), 
				input->voffset, input->poffset, 
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[0]),
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[1]),
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[2]),
				str_map_value(&enc_dict_input_swizzle_type_map, input->swizzles[3])
				);
		}
		printf("\n");
		printf("Output info\n");
		output_list = enc_ps->outputs;
		LIST_FOR_EACH(output_list, i)
		{
			output = list_get(output_list, i);
			printf(" [%d] %s, %s, virtual offset = %d, physical offset = %d, name = %s\n", 
				i, str_map_value(&enc_dict_output_type_map, output->type), 
				str_map_value(&enc_dict_symbol_datatype_map, output->data_type),
				output->voffset, output->poffset, output->name);
		}
		printf("\n");
		printf("Symbol info\n");
		symbol_list = enc_ps->symbols;
		LIST_FOR_EACH(symbol_list, i)
		{
			symbol = list_get(symbol_list, i);
			printf(" [%d] %s, %s, %s\n", i, symbol->name, 
				str_map_value(&enc_dict_symbol_type_map, symbol->type), 
				str_map_value(&enc_dict_symbol_datatype_map, symbol->dataType));
		}
		printf("\n");

		printf("Input Semantic Mappings\n");
		for (i = 0; i < meta_ps->numPsInSemantics; ++i)
		{
			printf(" [%d] %s, usageIdx %d, inputIdx %d, defaultVal %d, flatShade %d\n", i, 
				str_map_value(&enc_dict_semantic_input_type_map, meta_ps->psInSemantics[i].usage), 
				meta_ps->psInSemantics[i].usageIdx, meta_ps->psInSemantics[i].inputIdx, 
				meta_ps->psInSemantics[i].defaultVal, meta_ps->psInSemantics[i].flatShade);
		}
		printf("\n");
		printf("codeLenInByte\t= %d;Bytes\n", meta_ps->CodeLenInByte);
		printf("\n");
		printf("userElementCount\t= %d\n", meta_ps->u32UserElementCount);
		for (i = 0; i < meta_ps->u32UserElementCount; ++i)
		{
			printf(" userElements[%d]\t= %s, %d, s[%d,%d]\n", i, 
				str_map_value(&enc_dict_user_elements_type_map, meta_ps->pUserElement[i].dataClass), 
				meta_ps->pUserElement[i].apiSlot, meta_ps->pUserElement[i].startUserReg, 
				meta_ps->pUserElement[i].startUserReg + meta_ps->pUserElement[i].userRegCount  - 1);
		}
		printf("NumVgprs\t\t= %d\n", meta_ps->u32NumVgprs);
		printf("NumSgprs\t\t= %d\n", meta_ps->u32NumSgprs);
		printf("FloatMode\t\t= %d\n", meta_ps->u32FloatMode);
		printf("IeeeMode\t\t= %d\n", meta_ps->bIeeeMode);
		printf("UsesPrimID\t\t= %d\n", meta_ps->bUsesPrimId);
		printf("UsesVertexID\t\t= %d\n", meta_ps->bUsesVertexId);
		printf("ScratchSize\t\t= %d\n", meta_ps->scratchSize);
		for (i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
		{
			printf(" texResourceUsage[%d]\t\t= 0x%x\n", i, meta_ps->texResourceUsage[i]);
		}
		for (i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
		{
			printf(" fetch4ResourceUsage[%d]\t\t= 0x%x\n", i, meta_ps->fetch4ResourceUsage[i]);
		}
		for (i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
		{
			printf(" uavResourceUsage[%d]\t\t= 0x%x\n", i, meta_ps->uavResourceUsage[i]);
		}
		printf(" texSamplerUsage\t\t= 0x%x\n", meta_ps->texSamplerUsage);
		printf(" constBufUsage\t\t\t= 0x%x\n", meta_ps->constBufUsage);
		printf("SPI_SHADER_PGM_RSRC2_PS\t\t= 0x%08X\n", meta_ps->spiShaderPgmRsrc2PsAsUint);
		printf(" SSPRV:USER_SGPR\t\t= %d\n", meta_ps->spiShaderPgmRsrc2Ps.user_sgpr);		
		printf("SPI_SHADER_Z_FORMAT\t\t= %d\n", meta_ps->spiShaderZFormat);
		printf("SPI_PS_IN_CNTRL\t\t\t= %d\n", meta_ps->spiPsInControl);
		printf("SPI_PS_INPUT_ADDR\t\t= 0x%08X\n", meta_ps->spiPsInputAddr);
		printf("SPI_PS_INPUT_ENABLE\t\t= %d\n", meta_ps->spiPsInputEna);
		printf("SPI_BARYCENTRIC_CONTROL\t\t= 0x%08X\n", meta_ps->spiBarycCntl);
		printf("DEPTH_BUFFER_SHADER_CNTRL\t= 0x%08X\n", meta_ps->dbShaderControl);
		printf("CONST_BUFFER_SHADER_MASK\t= 0x%08X\n", meta_ps->cbShaderMask);
		printf("EXPORT_PATCH_CODE_SIZE\t\t= %d\n", meta_ps->exportPatchCodeSize);
		printf("NUM_PS_EXPORTS\t\t\t= %d\n", meta_ps->numPsExports);
		printf("DUAL_BLENDING\t\t\t= %d\n", meta_ps->dualBlending);
		printf("DEFAULT_EXPORT_FMT\t\t= %d\n", meta_ps->defaultExportFmt);
		break;
	}
	default:
		break;
	}
}

