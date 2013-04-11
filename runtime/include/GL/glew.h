
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

#ifndef RUNTIME_INCLUDE_GL_GLEW_H
#define RUNTIME_INCLUDE_GL_GLEW_H

#include "glu.h"


/* GLEW related */
/* error codes */
#define GLEW_OK 0
#define GLEW_NO_ERROR 0
#define GLEW_ERROR_NO_GL_VERSION 1  /* missing GL version */
#define GLEW_ERROR_GL_VERSION_10_ONLY 2  /* Need at least OpenGL 1.1 */
#define GLEW_ERROR_GLX_VERSION_11_ONLY 3  /* Need at least GLX 1.2 */

/* string codes */
#define GLEW_VERSION 1
#define GLEW_VERSION_MAJOR 2
#define GLEW_VERSION_MINOR 3
#define GLEW_VERSION_MICRO 4


#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1

#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914

#endif /* GL_VERSION_1_5 */

/* ----------------------------- GL_VERSION_2_0 ---------------------------- */

#ifndef GL_VERSION_2_0
#define GL_VERSION_2_0 1

#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_POINT_SPRITE 0x8861
#define GL_COORD_REPLACE 0x8862
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_COORDS 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE 0x8B4F
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#define GL_LOWER_LEFT 0x8CA1
#define GL_UPPER_LEFT 0x8CA2
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5
#endif /* GL_VERSION_2_0 */

/* ----------------------------- GL_VERSION_2_1 ---------------------------- */

#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1

#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_FLOAT_MAT2x3 0x8B65
#define GL_FLOAT_MAT2x4 0x8B66
#define GL_FLOAT_MAT3x2 0x8B67
#define GL_FLOAT_MAT3x4 0x8B68
#define GL_FLOAT_MAT4x2 0x8B69
#define GL_FLOAT_MAT4x3 0x8B6A
#define GL_SRGB 0x8C40
#define GL_SRGB8 0x8C41
#define GL_SRGB_ALPHA 0x8C42
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_SLUMINANCE_ALPHA 0x8C44
#define GL_SLUMINANCE8_ALPHA8 0x8C45
#define GL_SLUMINANCE 0x8C46
#define GL_SLUMINANCE8 0x8C47
#define GL_COMPRESSED_SRGB 0x8C48
#define GL_COMPRESSED_SRGB_ALPHA 0x8C49
#define GL_COMPRESSED_SLUMINANCE 0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B

#endif /* GL_VERSION_2_1 */

/* ----------------------------- GL_VERSION_3_0 ---------------------------- */

#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1

#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_NUM_EXTENSIONS 0x821D
#define GL_CONTEXT_FLAGS 0x821E
#define GL_DEPTH_BUFFER 0x8223
#define GL_STENCIL_BUFFER 0x8224
#define GL_COMPRESSED_RED 0x8225
#define GL_COMPRESSED_RG 0x8226
#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#define GL_CLAMP_VERTEX_COLOR 0x891A
#define GL_CLAMP_FRAGMENT_COLOR 0x891B
#define GL_CLAMP_READ_COLOR 0x891C
#define GL_FIXED_ONLY 0x891D
#define GL_TEXTURE_RED_TYPE 0x8C10
#define GL_TEXTURE_GREEN_TYPE 0x8C11
#define GL_TEXTURE_BLUE_TYPE 0x8C12
#define GL_TEXTURE_ALPHA_TYPE 0x8C13
#define GL_TEXTURE_LUMINANCE_TYPE 0x8C14
#define GL_TEXTURE_INTENSITY_TYPE 0x8C15
#define GL_TEXTURE_DEPTH_TYPE 0x8C16
#define GL_UNSIGNED_NORMALIZED 0x8C17
#define GL_TEXTURE_1D_ARRAY 0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY 0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
#define GL_R11F_G11F_B10F 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#define GL_RGB9_E5 0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#define GL_TEXTURE_SHARED_SIZE 0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_PRIMITIVES_GENERATED 0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD 0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS 0x8C8C
#define GL_SEPARATE_ATTRIBS 0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI 0x8D70
#define GL_RGB32UI 0x8D71
#define GL_RGBA16UI 0x8D76
#define GL_RGB16UI 0x8D77
#define GL_RGBA8UI 0x8D7C
#define GL_RGB8UI 0x8D7D
#define GL_RGBA32I 0x8D82
#define GL_RGB32I 0x8D83
#define GL_RGBA16I 0x8D88
#define GL_RGB16I 0x8D89
#define GL_RGBA8I 0x8D8E
#define GL_RGB8I 0x8D8F
#define GL_RED_INTEGER 0x8D94
#define GL_GREEN_INTEGER 0x8D95
#define GL_BLUE_INTEGER 0x8D96
#define GL_ALPHA_INTEGER 0x8D97
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_BGR_INTEGER 0x8D9A
#define GL_BGRA_INTEGER 0x8D9B
#define GL_SAMPLER_1D_ARRAY 0x8DC0
#define GL_SAMPLER_2D_ARRAY 0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
#define GL_SAMPLER_CUBE_SHADOW 0x8DC5
#define GL_UNSIGNED_INT_VEC2 0x8DC6
#define GL_UNSIGNED_INT_VEC3 0x8DC7
#define GL_UNSIGNED_INT_VEC4 0x8DC8
#define GL_INT_SAMPLER_1D 0x8DC9
#define GL_INT_SAMPLER_2D 0x8DCA
#define GL_INT_SAMPLER_3D 0x8DCB
#define GL_INT_SAMPLER_CUBE 0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
#define GL_QUERY_WAIT 0x8E13
#define GL_QUERY_NO_WAIT 0x8E14
#define GL_QUERY_BY_REGION_WAIT 0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT 0x8E16

#endif /* GL_VERSION_3_0 */

 /* ----------------------------- GL_VERSION_3_1 ---------------------------- */

#ifndef GL_VERSION_3_1
#define GL_VERSION_3_1 1

#define GL_TEXTURE_RECTANGLE 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE 0x84F8
#define GL_SAMPLER_2D_RECT 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW 0x8B64
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE 0x8C2B
#define GL_TEXTURE_BINDING_BUFFER 0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#define GL_TEXTURE_BUFFER_FORMAT 0x8C2E
#define GL_SAMPLER_BUFFER 0x8DC2
#define GL_INT_SAMPLER_2D_RECT 0x8DCD
#define GL_INT_SAMPLER_BUFFER 0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT 0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER 0x8DD8
#define GL_RED_SNORM 0x8F90
#define GL_RG_SNORM 0x8F91
#define GL_RGB_SNORM 0x8F92
#define GL_RGBA_SNORM 0x8F93
#define GL_R8_SNORM 0x8F94
#define GL_RG8_SNORM 0x8F95
#define GL_RGB8_SNORM 0x8F96
#define GL_RGBA8_SNORM 0x8F97
#define GL_R16_SNORM 0x8F98
#define GL_RG16_SNORM 0x8F99
#define GL_RGB16_SNORM 0x8F9A
#define GL_RGBA16_SNORM 0x8F9B
#define GL_SIGNED_NORMALIZED 0x8F9C
#define GL_PRIMITIVE_RESTART 0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX 0x8F9E
#define GL_BUFFER_ACCESS_FLAGS 0x911F
#define GL_BUFFER_MAP_LENGTH 0x9120
#define GL_BUFFER_MAP_OFFSET 0x9121

#endif /* GL_VERSION_3_1 */

/* ----------------------------- GL_VERSION_3_2 ---------------------------- */

#ifndef GL_VERSION_3_2
#define GL_VERSION_3_2 1

#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_LINES_ADJACENCY 0x000A
#define GL_LINE_STRIP_ADJACENCY 0x000B
#define GL_TRIANGLES_ADJACENCY 0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY 0x000D
#define GL_PROGRAM_POINT_SIZE 0x8642
#define GL_GEOMETRY_VERTICES_OUT 0x8916
#define GL_GEOMETRY_INPUT_TYPE 0x8917
#define GL_GEOMETRY_OUTPUT_TYPE 0x8918
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES 0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS 0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS 0x9125
#define GL_CONTEXT_PROFILE_MASK 0x9126

#endif /* GL_VERSION_3_3 */

/* ----------------------------- GL_VERSION_4_0 ---------------------------- */

#ifndef GL_VERSION_4_0
#define GL_VERSION_4_0 1

#define GL_GEOMETRY_SHADER_INVOCATIONS 0x887F
#define GL_SAMPLE_SHADING 0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE 0x8C37
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET 0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET 0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS 0x8E5D
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#define GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS 0x8F9F
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY 0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY 0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW 0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY 0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F

#endif /* GL_VERSION_4_0 */



#ifndef GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_buffer_object 1

#define GL_BUFFER_SIZE_ARB 0x8764
#define GL_BUFFER_USAGE_ARB 0x8765
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB 0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB 0x889F
#define GL_READ_ONLY_ARB 0x88B8
#define GL_WRITE_ONLY_ARB 0x88B9
#define GL_READ_WRITE_ARB 0x88BA
#define GL_BUFFER_ACCESS_ARB 0x88BB
#define GL_BUFFER_MAPPED_ARB 0x88BC
#define GL_BUFFER_MAP_POINTER_ARB 0x88BD
#define GL_STREAM_DRAW_ARB 0x88E0
#define GL_STREAM_READ_ARB 0x88E1
#define GL_STREAM_COPY_ARB 0x88E2
#define GL_STATIC_DRAW_ARB 0x88E4
#define GL_STATIC_READ_ARB 0x88E5
#define GL_STATIC_COPY_ARB 0x88E6
#define GL_DYNAMIC_DRAW_ARB 0x88E8
#define GL_DYNAMIC_READ_ARB 0x88E9
#define GL_DYNAMIC_COPY_ARB 0x88EA

#endif /* GL_ARB_vertex_buffer_object */

extern void glAccum (GLenum op, GLfloat value);
extern void glAlphaFunc (GLenum func, GLclampf ref);
extern GLboolean glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences);
extern void glArrayElement (GLint i);
extern void glBegin (GLenum mode);
extern void glBindTexture (GLenum target, GLuint texture);
extern void glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern void glBlendFunc (GLenum sfactor, GLenum dfactor);
extern void glCallList (GLuint list);
extern void glCallLists (GLsizei n, GLenum type, const GLvoid *lists);
extern void glClear (GLbitfield mask);
extern void glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void glClearDepth (GLclampd depth);
extern void glClearIndex (GLfloat c);
extern void glClearStencil (GLint s);
extern void glClipPlane (GLenum plane, const GLdouble *equation);
extern void glColor3b (GLbyte red, GLbyte green, GLbyte blue);
extern void glColor3bv (const GLbyte *v);
extern void glColor3d (GLdouble red, GLdouble green, GLdouble blue);
extern void glColor3dv (const GLdouble *v);
extern void glColor3f (GLfloat red, GLfloat green, GLfloat blue);
extern void glColor3fv (const GLfloat *v);
extern void glColor3i (GLint red, GLint green, GLint blue);
extern void glColor3iv (const GLint *v);
extern void glColor3s (GLshort red, GLshort green, GLshort blue);
extern void glColor3sv (const GLshort *v);
extern void glColor3ub (GLubyte red, GLubyte green, GLubyte blue);
extern void glColor3ubv (const GLubyte *v);
extern void glColor3ui (GLuint red, GLuint green, GLuint blue);
extern void glColor3uiv (const GLuint *v);
extern void glColor3us (GLushort red, GLushort green, GLushort blue);
extern void glColor3usv (const GLushort *v);
extern void glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern void glColor4bv (const GLbyte *v);
extern void glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern void glColor4dv (const GLdouble *v);
extern void glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void glColor4fv (const GLfloat *v);
extern void glColor4i (GLint red, GLint green, GLint blue, GLint alpha);
extern void glColor4iv (const GLint *v);
extern void glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern void glColor4sv (const GLshort *v);
extern void glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern void glColor4ubv (const GLubyte *v);
extern void glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern void glColor4uiv (const GLuint *v);
extern void glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern void glColor4usv (const GLushort *v);
extern void glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void glColorMaterial (GLenum face, GLenum mode);
extern void glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern void glCopyTexImage1D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
extern void glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void glCullFace (GLenum mode);
extern void glDeleteLists (GLuint list, GLsizei range);
extern void glDeleteTextures (GLsizei n, const GLuint *textures);
extern void glDepthFunc (GLenum func);
extern void glDepthMask (GLboolean flag);
extern void glDepthRange (GLclampd zNear, GLclampd zFar);
extern void glDisable (GLenum cap);
extern void glDisableClientState (GLenum array);
extern void glDrawArrays (GLenum mode, GLint first, GLsizei count);
extern void glDrawBuffer (GLenum mode);
extern void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
extern void glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void glEdgeFlag (GLboolean flag);
extern void glEdgeFlagPointer (GLsizei stride, const GLvoid *pointer);
extern void glEdgeFlagv (const GLboolean *flag);
extern void glEnable (GLenum cap);
extern void glEnableClientState (GLenum array);
extern void glEnd (void);
extern void glEndList (void);
extern void glEvalCoord1d (GLdouble u);
extern void glEvalCoord1dv (const GLdouble *u);
extern void glEvalCoord1f (GLfloat u);
extern void glEvalCoord1fv (const GLfloat *u);
extern void glEvalCoord2d (GLdouble u, GLdouble v);
extern void glEvalCoord2dv (const GLdouble *u);
extern void glEvalCoord2f (GLfloat u, GLfloat v);
extern void glEvalCoord2fv (const GLfloat *u);
extern void glEvalMesh1 (GLenum mode, GLint i1, GLint i2);
extern void glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern void glEvalPoint1 (GLint i);
extern void glEvalPoint2 (GLint i, GLint j);
extern void glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer);
extern void glFinish (void);
extern void glFlush (void);
extern void glFogf (GLenum pname, GLfloat param);
extern void glFogfv (GLenum pname, const GLfloat *params);
extern void glFogi (GLenum pname, GLint param);
extern void glFogiv (GLenum pname, const GLint *params);
extern void glFrontFace (GLenum mode);
extern void glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern GLuint glGenLists (GLsizei range);
extern void glGenTextures (GLsizei n, GLuint *textures);
extern void glGetBooleanv (GLenum pname, GLboolean *params);
extern void glGetClipPlane (GLenum plane, GLdouble *equation);
extern void glGetDoublev (GLenum pname, GLdouble *params);
extern GLenum glGetError (void);
extern void glGetFloatv (GLenum pname, GLfloat *params);
extern void glGetIntegerv (GLenum pname, GLint *params);
extern void glGetLightfv (GLenum light, GLenum pname, GLfloat *params);
extern void glGetLightiv (GLenum light, GLenum pname, GLint *params);
extern void glGetMapdv (GLenum target, GLenum query, GLdouble *v);
extern void glGetMapfv (GLenum target, GLenum query, GLfloat *v);
extern void glGetMapiv (GLenum target, GLenum query, GLint *v);
extern void glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params);
extern void glGetMaterialiv (GLenum face, GLenum pname, GLint *params);
extern void glGetPixelMapfv (GLenum map, GLfloat *values);
extern void glGetPixelMapuiv (GLenum map, GLuint *values);
extern void glGetPixelMapusv (GLenum map, GLushort *values);
extern void glGetPointerv (GLenum pname, GLvoid* *params);
extern void glGetPolygonStipple (GLubyte *mask);
extern const GLubyte * glGetString (GLenum name);
extern void glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetTexEnviv (GLenum target, GLenum pname, GLint *params);
extern void glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params);
extern void glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params);
extern void glGetTexGeniv (GLenum coord, GLenum pname, GLint *params);
extern void glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
extern void glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
extern void glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
extern void glHint (GLenum target, GLenum mode);
extern void glIndexMask (GLuint mask);
extern void glIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glIndexd (GLdouble c);
extern void glIndexdv (const GLdouble *c);
extern void glIndexf (GLfloat c);
extern void glIndexfv (const GLfloat *c);
extern void glIndexi (GLint c);
extern void glIndexiv (const GLint *c);
extern void glIndexs (GLshort c);
extern void glIndexsv (const GLshort *c);
extern void glIndexub (GLubyte c);
extern void glIndexubv (const GLubyte *c);
extern void glInitNames (void);
extern void glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer);
extern GLboolean glIsEnabled (GLenum cap);
extern GLboolean glIsList (GLuint list);
extern GLboolean glIsTexture (GLuint texture);
extern void glLightModelf (GLenum pname, GLfloat param);
extern void glLightModelfv (GLenum pname, const GLfloat *params);
extern void glLightModeli (GLenum pname, GLint param);
extern void glLightModeliv (GLenum pname, const GLint *params);
extern void glLightf (GLenum light, GLenum pname, GLfloat param);
extern void glLightfv (GLenum light, GLenum pname, const GLfloat *params);
extern void glLighti (GLenum light, GLenum pname, GLint param);
extern void glLightiv (GLenum light, GLenum pname, const GLint *params);
extern void glLineStipple (GLint factor, GLushort pattern);
extern void glLineWidth (GLfloat width);
extern void glListBase (GLuint base);
extern void glLoadIdentity (void);
extern void glLoadMatrixd (const GLdouble *m);
extern void glLoadMatrixf (const GLfloat *m);
extern void glLoadName (GLuint name);
extern void glLogicOp (GLenum opcode);
extern void glMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
extern void glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
extern void glMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
extern void glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
extern void glMapGrid1d (GLint un, GLdouble u1, GLdouble u2);
extern void glMapGrid1f (GLint un, GLfloat u1, GLfloat u2);
extern void glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern void glMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern void glMaterialf (GLenum face, GLenum pname, GLfloat param);
extern void glMaterialfv (GLenum face, GLenum pname, const GLfloat *params);
extern void glMateriali (GLenum face, GLenum pname, GLint param);
extern void glMaterialiv (GLenum face, GLenum pname, const GLint *params);
extern void glMatrixMode (GLenum mode);
extern void glMultMatrixd (const GLdouble *m);
extern void glMultMatrixf (const GLfloat *m);
extern void glNewList (GLuint list, GLenum mode);
extern void glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz);
extern void glNormal3bv (const GLbyte *v);
extern void glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz);
extern void glNormal3dv (const GLdouble *v);
extern void glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz);
extern void glNormal3fv (const GLfloat *v);
extern void glNormal3i (GLint nx, GLint ny, GLint nz);
extern void glNormal3iv (const GLint *v);
extern void glNormal3s (GLshort nx, GLshort ny, GLshort nz);
extern void glNormal3sv (const GLshort *v);
extern void glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void glPassThrough (GLfloat token);
extern void glPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat *values);
extern void glPixelMapuiv (GLenum map, GLsizei mapsize, const GLuint *values);
extern void glPixelMapusv (GLenum map, GLsizei mapsize, const GLushort *values);
extern void glPixelStoref (GLenum pname, GLfloat param);
extern void glPixelStorei (GLenum pname, GLint param);
extern void glPixelTransferf (GLenum pname, GLfloat param);
extern void glPixelTransferi (GLenum pname, GLint param);
extern void glPixelZoom (GLfloat xfactor, GLfloat yfactor);
extern void glPointSize (GLfloat size);
extern void glPolygonMode (GLenum face, GLenum mode);
extern void glPolygonOffset (GLfloat factor, GLfloat units);
extern void glPolygonStipple (const GLubyte *mask);
extern void glPopAttrib (void);
extern void glPopClientAttrib (void);
extern void glPopMatrix (void);
extern void glPopName (void);
extern void glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities);
extern void glPushAttrib (GLbitfield mask);
extern void glPushClientAttrib (GLbitfield mask);
extern void glPushMatrix (void);
extern void glPushName (GLuint name);
extern void glRasterPos2d (GLdouble x, GLdouble y);
extern void glRasterPos2dv (const GLdouble *v);
extern void glRasterPos2f (GLfloat x, GLfloat y);
extern void glRasterPos2fv (const GLfloat *v);
extern void glRasterPos2i (GLint x, GLint y);
extern void glRasterPos2iv (const GLint *v);
extern void glRasterPos2s (GLshort x, GLshort y);
extern void glRasterPos2sv (const GLshort *v);
extern void glRasterPos3d (GLdouble x, GLdouble y, GLdouble z);
extern void glRasterPos3dv (const GLdouble *v);
extern void glRasterPos3f (GLfloat x, GLfloat y, GLfloat z);
extern void glRasterPos3fv (const GLfloat *v);
extern void glRasterPos3i (GLint x, GLint y, GLint z);
extern void glRasterPos3iv (const GLint *v);
extern void glRasterPos3s (GLshort x, GLshort y, GLshort z);
extern void glRasterPos3sv (const GLshort *v);
extern void glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void glRasterPos4dv (const GLdouble *v);
extern void glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void glRasterPos4fv (const GLfloat *v);
extern void glRasterPos4i (GLint x, GLint y, GLint z, GLint w);
extern void glRasterPos4iv (const GLint *v);
extern void glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w);
extern void glRasterPos4sv (const GLshort *v);
extern void glReadBuffer (GLenum mode);
extern void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
extern void glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern void glRectdv (const GLdouble *v1, const GLdouble *v2);
extern void glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern void glRectfv (const GLfloat *v1, const GLfloat *v2);
extern void glRecti (GLint x1, GLint y1, GLint x2, GLint y2);
extern void glRectiv (const GLint *v1, const GLint *v2);
extern void glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern void glRectsv (const GLshort *v1, const GLshort *v2);
extern GLint glRenderMode (GLenum mode);
extern void glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void glScaled (GLdouble x, GLdouble y, GLdouble z);
extern void glScalef (GLfloat x, GLfloat y, GLfloat z);
extern void glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
extern void glSelectBuffer (GLsizei size, GLuint *buffer);
extern void glShadeModel (GLenum mode);
extern void glStencilFunc (GLenum func, GLint ref, GLuint mask);
extern void glStencilMask (GLuint mask);
extern void glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
extern void glTexCoord1d (GLdouble s);
extern void glTexCoord1dv (const GLdouble *v);
extern void glTexCoord1f (GLfloat s);
extern void glTexCoord1fv (const GLfloat *v);
extern void glTexCoord1i (GLint s);
extern void glTexCoord1iv (const GLint *v);
extern void glTexCoord1s (GLshort s);
extern void glTexCoord1sv (const GLshort *v);
extern void glTexCoord2d (GLdouble s, GLdouble t);
extern void glTexCoord2dv (const GLdouble *v);
extern void glTexCoord2f (GLfloat s, GLfloat t);
extern void glTexCoord2fv (const GLfloat *v);
extern void glTexCoord2i (GLint s, GLint t);
extern void glTexCoord2iv (const GLint *v);
extern void glTexCoord2s (GLshort s, GLshort t);
extern void glTexCoord2sv (const GLshort *v);
extern void glTexCoord3d (GLdouble s, GLdouble t, GLdouble r);
extern void glTexCoord3dv (const GLdouble *v);
extern void glTexCoord3f (GLfloat s, GLfloat t, GLfloat r);
extern void glTexCoord3fv (const GLfloat *v);
extern void glTexCoord3i (GLint s, GLint t, GLint r);
extern void glTexCoord3iv (const GLint *v);
extern void glTexCoord3s (GLshort s, GLshort t, GLshort r);
extern void glTexCoord3sv (const GLshort *v);
extern void glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void glTexCoord4dv (const GLdouble *v);
extern void glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void glTexCoord4fv (const GLfloat *v);
extern void glTexCoord4i (GLint s, GLint t, GLint r, GLint q);
extern void glTexCoord4iv (const GLint *v);
extern void glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q);
extern void glTexCoord4sv (const GLshort *v);
extern void glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glTexEnvf (GLenum target, GLenum pname, GLfloat param);
extern void glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params);
extern void glTexEnvi (GLenum target, GLenum pname, GLint param);
extern void glTexEnviv (GLenum target, GLenum pname, const GLint *params);
extern void glTexGend (GLenum coord, GLenum pname, GLdouble param);
extern void glTexGendv (GLenum coord, GLenum pname, const GLdouble *params);
extern void glTexGenf (GLenum coord, GLenum pname, GLfloat param);
extern void glTexGenfv (GLenum coord, GLenum pname, const GLfloat *params);
extern void glTexGeni (GLenum coord, GLenum pname, GLint param);
extern void glTexGeniv (GLenum coord, GLenum pname, const GLint *params);
extern void glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexParameterf (GLenum target, GLenum pname, GLfloat param);
extern void glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
extern void glTexParameteri (GLenum target, GLenum pname, GLint param);
extern void glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
extern void glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTranslated (GLdouble x, GLdouble y, GLdouble z);
extern void glTranslatef (GLfloat x, GLfloat y, GLfloat z);
extern void glVertex2d (GLdouble x, GLdouble y);
extern void glVertex2dv (const GLdouble *v);
extern void glVertex2f (GLfloat x, GLfloat y);
extern void glVertex2fv (const GLfloat *v);
extern void glVertex2i (GLint x, GLint y);
extern void glVertex2iv (const GLint *v);
extern void glVertex2s (GLshort x, GLshort y);
extern void glVertex2sv (const GLshort *v);
extern void glVertex3d (GLdouble x, GLdouble y, GLdouble z);
extern void glVertex3dv (const GLdouble *v);
extern void glVertex3f (GLfloat x, GLfloat y, GLfloat z);
extern void glVertex3fv (const GLfloat *v);
extern void glVertex3i (GLint x, GLint y, GLint z);
extern void glVertex3iv (const GLint *v);
extern void glVertex3s (GLshort x, GLshort y, GLshort z);
extern void glVertex3sv (const GLshort *v);
extern void glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void glVertex4dv (const GLdouble *v);
extern void glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void glVertex4fv (const GLfloat *v);
extern void glVertex4i (GLint x, GLint y, GLint z, GLint w);
extern void glVertex4iv (const GLint *v);
extern void glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w);
extern void glVertex4sv (const GLshort *v);
extern void glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

#ifndef RUNTIME_GLEW_API_H
#define glCopyTexSubImage3D __glewCopyTexSubImage3D
#define glDrawRangeElements __glewDrawRangeElements
#define glTexImage3D __glewTexImage3D
#define glTexSubImage3D __glewTexSubImage3D
#define glActiveTexture __glewActiveTexture
#define glClientActiveTexture __glewClientActiveTexture
#define glCompressedTexImage1D __glewCompressedTexImage1D
#define glCompressedTexImage2D __glewCompressedTexImage2D
#define glCompressedTexImage3D __glewCompressedTexImage3D
#define glCompressedTexSubImage1D __glewCompressedTexSubImage1D
#define glCompressedTexSubImage2D __glewCompressedTexSubImage2D
#define glCompressedTexSubImage3D __glewCompressedTexSubImage3D
#define glGetCompressedTexImage __glewGetCompressedTexImage
#define glLoadTransposeMatrixd __glewLoadTransposeMatrixd
#define glLoadTransposeMatrixf __glewLoadTransposeMatrixf
#define glMultTransposeMatrixd __glewMultTransposeMatrixd
#define glMultTransposeMatrixf __glewMultTransposeMatrixf
#define glMultiTexCoord1d __glewMultiTexCoord1d
#define glMultiTexCoord1dv __glewMultiTexCoord1dv
#define glMultiTexCoord1f __glewMultiTexCoord1f
#define glMultiTexCoord1fv __glewMultiTexCoord1fv
#define glMultiTexCoord1i __glewMultiTexCoord1i
#define glMultiTexCoord1iv __glewMultiTexCoord1iv
#define glMultiTexCoord1s __glewMultiTexCoord1s
#define glMultiTexCoord1sv __glewMultiTexCoord1sv
#define glMultiTexCoord2d __glewMultiTexCoord2d
#define glMultiTexCoord2dv __glewMultiTexCoord2dv
#define glMultiTexCoord2f __glewMultiTexCoord2f
#define glMultiTexCoord2fv __glewMultiTexCoord2fv
#define glMultiTexCoord2i __glewMultiTexCoord2i
#define glMultiTexCoord2iv __glewMultiTexCoord2iv
#define glMultiTexCoord2s __glewMultiTexCoord2s
#define glMultiTexCoord2sv __glewMultiTexCoord2sv
#define glMultiTexCoord3d __glewMultiTexCoord3d
#define glMultiTexCoord3dv __glewMultiTexCoord3dv
#define glMultiTexCoord3f __glewMultiTexCoord3f
#define glMultiTexCoord3fv __glewMultiTexCoord3fv
#define glMultiTexCoord3i __glewMultiTexCoord3i
#define glMultiTexCoord3iv __glewMultiTexCoord3iv
#define glMultiTexCoord3s __glewMultiTexCoord3s
#define glMultiTexCoord3sv __glewMultiTexCoord3sv
#define glMultiTexCoord4d __glewMultiTexCoord4d
#define glMultiTexCoord4dv __glewMultiTexCoord4dv
#define glMultiTexCoord4f __glewMultiTexCoord4f
#define glMultiTexCoord4fv __glewMultiTexCoord4fv
#define glMultiTexCoord4i __glewMultiTexCoord4i
#define glMultiTexCoord4iv __glewMultiTexCoord4iv
#define glMultiTexCoord4s __glewMultiTexCoord4s
#define glMultiTexCoord4sv __glewMultiTexCoord4sv
#define glSampleCoverage __glewSampleCoverage
#define glBlendColor __glewBlendColor
#define glBlendEquation __glewBlendEquation
#define glBlendFuncSeparate __glewBlendFuncSeparate
#define glFogCoordPointer __glewFogCoordPointer
#define glFogCoordd __glewFogCoordd
#define glFogCoorddv __glewFogCoorddv
#define glFogCoordf __glewFogCoordf
#define glFogCoordfv __glewFogCoordfv
#define glMultiDrawArrays __glewMultiDrawArrays
#define glMultiDrawElements __glewMultiDrawElements
#define glPointParameterf __glewPointParameterf
#define glPointParameterfv __glewPointParameterfv
#define glPointParameteri __glewPointParameteri
#define glPointParameteriv __glewPointParameteriv
#define glSecondaryColor3b __glewSecondaryColor3b
#define glSecondaryColor3bv __glewSecondaryColor3bv
#define glSecondaryColor3d __glewSecondaryColor3d
#define glSecondaryColor3dv __glewSecondaryColor3dv
#define glSecondaryColor3f __glewSecondaryColor3f
#define glSecondaryColor3fv __glewSecondaryColor3fv
#define glSecondaryColor3i __glewSecondaryColor3i
#define glSecondaryColor3iv __glewSecondaryColor3iv
#define glSecondaryColor3s __glewSecondaryColor3s
#define glSecondaryColor3sv __glewSecondaryColor3sv
#define glSecondaryColor3ub __glewSecondaryColor3ub
#define glSecondaryColor3ubv __glewSecondaryColor3ubv
#define glSecondaryColor3ui __glewSecondaryColor3ui
#define glSecondaryColor3uiv __glewSecondaryColor3uiv
#define glSecondaryColor3us __glewSecondaryColor3us
#define glSecondaryColor3usv __glewSecondaryColor3usv
#define glSecondaryColorPointer __glewSecondaryColorPointer
#define glWindowPos2d __glewWindowPos2d
#define glWindowPos2dv __glewWindowPos2dv
#define glWindowPos2f __glewWindowPos2f
#define glWindowPos2fv __glewWindowPos2fv
#define glWindowPos2i __glewWindowPos2i
#define glWindowPos2iv __glewWindowPos2iv
#define glWindowPos2s __glewWindowPos2s
#define glWindowPos2sv __glewWindowPos2sv
#define glWindowPos3d __glewWindowPos3d
#define glWindowPos3dv __glewWindowPos3dv
#define glWindowPos3f __glewWindowPos3f
#define glWindowPos3fv __glewWindowPos3fv
#define glWindowPos3i __glewWindowPos3i
#define glWindowPos3iv __glewWindowPos3iv
#define glWindowPos3s __glewWindowPos3s
#define glWindowPos3sv __glewWindowPos3sv
#define glBeginQuery __glewBeginQuery
#define glBindBuffer __glewBindBuffer
#define glBufferData __glewBufferData
#define glBufferSubData __glewBufferSubData
#define glDeleteBuffers __glewDeleteBuffers
#define glDeleteQueries __glewDeleteQueries
#define glEndQuery __glewEndQuery
#define glGenBuffers __glewGenBuffers
#define glGenQueries __glewGenQueries
#define glGetBufferParameteriv __glewGetBufferParameteriv
#define glGetBufferPointerv __glewGetBufferPointerv
#define glGetBufferSubData __glewGetBufferSubData
#define glGetQueryObjectiv __glewGetQueryObjectiv
#define glGetQueryObjectuiv __glewGetQueryObjectuiv
#define glGetQueryiv __glewGetQueryiv
#define glIsBuffer __glewIsBuffer
#define glIsQuery __glewIsQuery
#define glMapBuffer __glewMapBuffer
#define glUnmapBuffer __glewUnmapBuffer
#define glAttachShader __glewAttachShader
#define glBindAttribLocation __glewBindAttribLocation
#define glBlendEquationSeparate __glewBlendEquationSeparate
#define glCompileShader __glewCompileShader
#define glCreateProgram __glewCreateProgram
#define glCreateShader __glewCreateShader
#define glDeleteProgram __glewDeleteProgram
#define glDeleteShader __glewDeleteShader
#define glDetachShader __glewDetachShader
#define glDisableVertexAttribArray __glewDisableVertexAttribArray
#define glDrawBuffers __glewDrawBuffers
#define glEnableVertexAttribArray __glewEnableVertexAttribArray
#define glGetActiveAttrib __glewGetActiveAttrib
#define glGetActiveUniform __glewGetActiveUniform
#define glGetAttachedShaders __glewGetAttachedShaders
#define glGetAttribLocation __glewGetAttribLocation
#define glGetProgramInfoLog __glewGetProgramInfoLog
#define glGetProgramiv __glewGetProgramiv
#define glGetShaderInfoLog __glewGetShaderInfoLog
#define glGetShaderSource __glewGetShaderSource
#define glGetShaderiv __glewGetShaderiv
#define glGetUniformLocation __glewGetUniformLocation
#define glGetUniformfv __glewGetUniformfv
#define glGetUniformiv __glewGetUniformiv
#define glGetVertexAttribPointerv __glewGetVertexAttribPointerv
#define glGetVertexAttribdv __glewGetVertexAttribdv
#define glGetVertexAttribfv __glewGetVertexAttribfv
#define glGetVertexAttribiv __glewGetVertexAttribiv
#define glIsProgram __glewIsProgram
#define glIsShader __glewIsShader
#define glLinkProgram __glewLinkProgram
#define glShaderSource __glewShaderSource
#define glStencilFuncSeparate __glewStencilFuncSeparate
#define glStencilMaskSeparate __glewStencilMaskSeparate
#define glStencilOpSeparate __glewStencilOpSeparate
#define glUniform1f __glewUniform1f
#define glUniform1fv __glewUniform1fv
#define glUniform1i __glewUniform1i
#define glUniform1iv __glewUniform1iv
#define glUniform2f __glewUniform2f
#define glUniform2fv __glewUniform2fv
#define glUniform2i __glewUniform2i
#define glUniform2iv __glewUniform2iv
#define glUniform3f __glewUniform3f
#define glUniform3fv __glewUniform3fv
#define glUniform3i __glewUniform3i
#define glUniform3iv __glewUniform3iv
#define glUniform4f __glewUniform4f
#define glUniform4fv __glewUniform4fv
#define glUniform4i __glewUniform4i
#define glUniform4iv __glewUniform4iv
#define glUniformMatrix2fv __glewUniformMatrix2fv
#define glUniformMatrix3fv __glewUniformMatrix3fv
#define glUniformMatrix4fv __glewUniformMatrix4fv
#define glUseProgram __glewUseProgram
#define glValidateProgram __glewValidateProgram
#define glVertexAttrib1d __glewVertexAttrib1d
#define glVertexAttrib1dv __glewVertexAttrib1dv
#define glVertexAttrib1f __glewVertexAttrib1f
#define glVertexAttrib1fv __glewVertexAttrib1fv
#define glVertexAttrib1s __glewVertexAttrib1s
#define glVertexAttrib1sv __glewVertexAttrib1sv
#define glVertexAttrib2d __glewVertexAttrib2d
#define glVertexAttrib2dv __glewVertexAttrib2dv
#define glVertexAttrib2f __glewVertexAttrib2f
#define glVertexAttrib2fv __glewVertexAttrib2fv
#define glVertexAttrib2s __glewVertexAttrib2s
#define glVertexAttrib2sv __glewVertexAttrib2sv
#define glVertexAttrib3d __glewVertexAttrib3d
#define glVertexAttrib3dv __glewVertexAttrib3dv
#define glVertexAttrib3f __glewVertexAttrib3f
#define glVertexAttrib3fv __glewVertexAttrib3fv
#define glVertexAttrib3s __glewVertexAttrib3s
#define glVertexAttrib3sv __glewVertexAttrib3sv
#define glVertexAttrib4Nbv __glewVertexAttrib4Nbv
#define glVertexAttrib4Niv __glewVertexAttrib4Niv
#define glVertexAttrib4Nsv __glewVertexAttrib4Nsv
#define glVertexAttrib4Nub __glewVertexAttrib4Nub
#define glVertexAttrib4Nubv __glewVertexAttrib4Nubv
#define glVertexAttrib4Nuiv __glewVertexAttrib4Nuiv
#define glVertexAttrib4Nusv __glewVertexAttrib4Nusv
#define glVertexAttrib4bv __glewVertexAttrib4bv
#define glVertexAttrib4d __glewVertexAttrib4d
#define glVertexAttrib4dv __glewVertexAttrib4dv
#define glVertexAttrib4f __glewVertexAttrib4f
#define glVertexAttrib4fv __glewVertexAttrib4fv
#define glVertexAttrib4iv __glewVertexAttrib4iv
#define glVertexAttrib4s __glewVertexAttrib4s
#define glVertexAttrib4sv __glewVertexAttrib4sv
#define glVertexAttrib4ubv __glewVertexAttrib4ubv
#define glVertexAttrib4uiv __glewVertexAttrib4uiv
#define glVertexAttrib4usv __glewVertexAttrib4usv
#define glVertexAttribPointer __glewVertexAttribPointer
#define glUniformMatrix2x3fv __glewUniformMatrix2x3fv
#define glUniformMatrix2x4fv __glewUniformMatrix2x4fv
#define glUniformMatrix3x2fv __glewUniformMatrix3x2fv
#define glUniformMatrix3x4fv __glewUniformMatrix3x4fv
#define glUniformMatrix4x2fv __glewUniformMatrix4x2fv
#define glUniformMatrix4x3fv __glewUniformMatrix4x3fv
#define glBeginConditionalRender __glewBeginConditionalRender
#define glBeginTransformFeedback __glewBeginTransformFeedback
#define glBindFragDataLocation __glewBindFragDataLocation
#define glClampColor __glewClampColor
#define glClearBufferfi __glewClearBufferfi
#define glClearBufferfv __glewClearBufferfv
#define glClearBufferiv __glewClearBufferiv
#define glClearBufferuiv __glewClearBufferuiv
#define glColorMaski __glewColorMaski
#define glDisablei __glewDisablei
#define glEnablei __glewEnablei
#define glEndConditionalRender __glewEndConditionalRender
#define glEndTransformFeedback __glewEndTransformFeedback
#define glGetBooleani_v __glewGetBooleani_v
#define glGetFragDataLocation __glewGetFragDataLocation
#define glGetStringi __glewGetStringi
#define glGetTexParameterIiv __glewGetTexParameterIiv
#define glGetTexParameterIuiv __glewGetTexParameterIuiv
#define glGetTransformFeedbackVarying __glewGetTransformFeedbackVarying
#define glGetUniformuiv __glewGetUniformuiv
#define glGetVertexAttribIiv __glewGetVertexAttribIiv
#define glGetVertexAttribIuiv __glewGetVertexAttribIuiv
#define glIsEnabledi __glewIsEnabledi
#define glTexParameterIiv __glewTexParameterIiv
#define glTexParameterIuiv __glewTexParameterIuiv
#define glTransformFeedbackVaryings __glewTransformFeedbackVaryings
#define glUniform1ui __glewUniform1ui
#define glUniform1uiv __glewUniform1uiv
#define glUniform2ui __glewUniform2ui
#define glUniform2uiv __glewUniform2uiv
#define glUniform3ui __glewUniform3ui
#define glUniform3uiv __glewUniform3uiv
#define glUniform4ui __glewUniform4ui
#define glUniform4uiv __glewUniform4uiv
#define glVertexAttribI1i __glewVertexAttribI1i
#define glVertexAttribI1iv __glewVertexAttribI1iv
#define glVertexAttribI1ui __glewVertexAttribI1ui
#define glVertexAttribI1uiv __glewVertexAttribI1uiv
#define glVertexAttribI2i __glewVertexAttribI2i
#define glVertexAttribI2iv __glewVertexAttribI2iv
#define glVertexAttribI2ui __glewVertexAttribI2ui
#define glVertexAttribI2uiv __glewVertexAttribI2uiv
#define glVertexAttribI3i __glewVertexAttribI3i
#define glVertexAttribI3iv __glewVertexAttribI3iv
#define glVertexAttribI3ui __glewVertexAttribI3ui
#define glVertexAttribI3uiv __glewVertexAttribI3uiv
#define glVertexAttribI4bv __glewVertexAttribI4bv
#define glVertexAttribI4i __glewVertexAttribI4i
#define glVertexAttribI4iv __glewVertexAttribI4iv
#define glVertexAttribI4sv __glewVertexAttribI4sv
#define glVertexAttribI4ubv __glewVertexAttribI4ubv
#define glVertexAttribI4ui __glewVertexAttribI4ui
#define glVertexAttribI4uiv __glewVertexAttribI4uiv
#define glVertexAttribI4usv __glewVertexAttribI4usv
#define glVertexAttribIPointer __glewVertexAttribIPointer
#define glDrawArraysInstanced __glewDrawArraysInstanced
#define glDrawElementsInstanced __glewDrawElementsInstanced
#define glPrimitiveRestartIndex __glewPrimitiveRestartIndex
#define glTexBuffer __glewTexBuffer
#define glFramebufferTexture __glewFramebufferTexture
#define glGetBufferParameteri64v __glewGetBufferParameteri64v
#define glGetInteger64i_v __glewGetInteger64i_v
#define glVertexAttribDivisor __glewVertexAttribDivisor
#define glBlendEquationSeparatei __glewBlendEquationSeparatei
#define glBlendEquationi __glewBlendEquationi
#define glBlendFuncSeparatei __glewBlendFuncSeparatei
#define glBlendFunci __glewBlendFunci
#define glMinSampleShading __glewMinSampleShading
#define glTbufferMask3DFX __glewTbufferMask3DFX
#define glDebugMessageCallbackAMD __glewDebugMessageCallbackAMD
#define glDebugMessageEnableAMD __glewDebugMessageEnableAMD
#define glDebugMessageInsertAMD __glewDebugMessageInsertAMD
#define glGetDebugMessageLogAMD __glewGetDebugMessageLogAMD
#define glBlendEquationIndexedAMD __glewBlendEquationIndexedAMD
#define glBlendEquationSeparateIndexedAMD __glewBlendEquationSeparateIndexedAMD
#define glBlendFuncIndexedAMD __glewBlendFuncIndexedAMD
#define glBlendFuncSeparateIndexedAMD __glewBlendFuncSeparateIndexedAMD
#define glDeleteNamesAMD __glewDeleteNamesAMD
#define glGenNamesAMD __glewGenNamesAMD
#define glIsNameAMD __glewIsNameAMD
#define glBeginPerfMonitorAMD __glewBeginPerfMonitorAMD
#define glDeletePerfMonitorsAMD __glewDeletePerfMonitorsAMD
#define glEndPerfMonitorAMD __glewEndPerfMonitorAMD
#define glGenPerfMonitorsAMD __glewGenPerfMonitorsAMD
#define glGetPerfMonitorCounterDataAMD __glewGetPerfMonitorCounterDataAMD
#define glGetPerfMonitorCounterInfoAMD __glewGetPerfMonitorCounterInfoAMD
#define glGetPerfMonitorCounterStringAMD __glewGetPerfMonitorCounterStringAMD
#define glGetPerfMonitorCountersAMD __glewGetPerfMonitorCountersAMD
#define glGetPerfMonitorGroupStringAMD __glewGetPerfMonitorGroupStringAMD
#define glGetPerfMonitorGroupsAMD __glewGetPerfMonitorGroupsAMD
#define glSelectPerfMonitorCountersAMD __glewSelectPerfMonitorCountersAMD
#define glSetMultisamplefvAMD __glewSetMultisamplefvAMD
#define glTessellationFactorAMD __glewTessellationFactorAMD
#define glTessellationModeAMD __glewTessellationModeAMD
#define glDrawElementArrayAPPLE __glewDrawElementArrayAPPLE
#define glDrawRangeElementArrayAPPLE __glewDrawRangeElementArrayAPPLE
#define glElementPointerAPPLE __glewElementPointerAPPLE
#define glMultiDrawElementArrayAPPLE __glewMultiDrawElementArrayAPPLE
#define glMultiDrawRangeElementArrayAPPLE __glewMultiDrawRangeElementArrayAPPLE
#define glDeleteFencesAPPLE __glewDeleteFencesAPPLE
#define glFinishFenceAPPLE __glewFinishFenceAPPLE
#define glFinishObjectAPPLE __glewFinishObjectAPPLE
#define glGenFencesAPPLE __glewGenFencesAPPLE
#define glIsFenceAPPLE __glewIsFenceAPPLE
#define glSetFenceAPPLE __glewSetFenceAPPLE
#define glTestFenceAPPLE __glewTestFenceAPPLE
#define glTestObjectAPPLE __glewTestObjectAPPLE
#define glBufferParameteriAPPLE __glewBufferParameteriAPPLE
#define glFlushMappedBufferRangeAPPLE __glewFlushMappedBufferRangeAPPLE
#define glGetObjectParameterivAPPLE __glewGetObjectParameterivAPPLE
#define glObjectPurgeableAPPLE __glewObjectPurgeableAPPLE
#define glObjectUnpurgeableAPPLE __glewObjectUnpurgeableAPPLE
#define glGetTexParameterPointervAPPLE __glewGetTexParameterPointervAPPLE
#define glTextureRangeAPPLE __glewTextureRangeAPPLE
#define glBindVertexArrayAPPLE __glewBindVertexArrayAPPLE
#define glDeleteVertexArraysAPPLE __glewDeleteVertexArraysAPPLE
#define glGenVertexArraysAPPLE __glewGenVertexArraysAPPLE
#define glIsVertexArrayAPPLE __glewIsVertexArrayAPPLE
#define glFlushVertexArrayRangeAPPLE __glewFlushVertexArrayRangeAPPLE
#define glVertexArrayParameteriAPPLE __glewVertexArrayParameteriAPPLE
#define glVertexArrayRangeAPPLE __glewVertexArrayRangeAPPLE
#define glDisableVertexAttribAPPLE __glewDisableVertexAttribAPPLE
#define glEnableVertexAttribAPPLE __glewEnableVertexAttribAPPLE
#define glIsVertexAttribEnabledAPPLE __glewIsVertexAttribEnabledAPPLE
#define glMapVertexAttrib1dAPPLE __glewMapVertexAttrib1dAPPLE
#define glMapVertexAttrib1fAPPLE __glewMapVertexAttrib1fAPPLE
#define glMapVertexAttrib2dAPPLE __glewMapVertexAttrib2dAPPLE
#define glMapVertexAttrib2fAPPLE __glewMapVertexAttrib2fAPPLE
#define glClearDepthf __glewClearDepthf
#define glDepthRangef __glewDepthRangef
#define glGetShaderPrecisionFormat __glewGetShaderPrecisionFormat
#define glReleaseShaderCompiler __glewReleaseShaderCompiler
#define glShaderBinary __glewShaderBinary
#define glBindFragDataLocationIndexed __glewBindFragDataLocationIndexed
#define glGetFragDataIndex __glewGetFragDataIndex
#define glCreateSyncFromCLeventARB __glewCreateSyncFromCLeventARB
#define glClampColorARB __glewClampColorARB
#define glCopyBufferSubData __glewCopyBufferSubData
#define glDebugMessageCallbackARB __glewDebugMessageCallbackARB
#define glDebugMessageControlARB __glewDebugMessageControlARB
#define glDebugMessageInsertARB __glewDebugMessageInsertARB
#define glGetDebugMessageLogARB __glewGetDebugMessageLogARB
#define glDrawBuffersARB __glewDrawBuffersARB
#define glBlendEquationSeparateiARB __glewBlendEquationSeparateiARB
#define glBlendEquationiARB __glewBlendEquationiARB
#define glBlendFuncSeparateiARB __glewBlendFuncSeparateiARB
#define glBlendFunciARB __glewBlendFunciARB
#define glDrawElementsBaseVertex __glewDrawElementsBaseVertex
#define glDrawElementsInstancedBaseVertex __glewDrawElementsInstancedBaseVertex
#define glDrawRangeElementsBaseVertex __glewDrawRangeElementsBaseVertex
#define glMultiDrawElementsBaseVertex __glewMultiDrawElementsBaseVertex
#define glDrawArraysIndirect __glewDrawArraysIndirect
#define glDrawElementsIndirect __glewDrawElementsIndirect
#define glBindFramebuffer __glewBindFramebuffer
#define glBindRenderbuffer __glewBindRenderbuffer
#define glBlitFramebuffer __glewBlitFramebuffer
#define glCheckFramebufferStatus __glewCheckFramebufferStatus
#define glDeleteFramebuffers __glewDeleteFramebuffers
#define glDeleteRenderbuffers __glewDeleteRenderbuffers
#define glFramebufferRenderbuffer __glewFramebufferRenderbuffer
#define glFramebufferTexture1D __glewFramebufferTexture1D
#define glFramebufferTexture2D __glewFramebufferTexture2D
#define glFramebufferTexture3D __glewFramebufferTexture3D
#define glFramebufferTextureLayer __glewFramebufferTextureLayer
#define glGenFramebuffers __glewGenFramebuffers
#define glGenRenderbuffers __glewGenRenderbuffers
#define glGenerateMipmap __glewGenerateMipmap
#define glGetFramebufferAttachmentParameteriv __glewGetFramebufferAttachmentParameteriv
#define glGetRenderbufferParameteriv __glewGetRenderbufferParameteriv
#define glIsFramebuffer __glewIsFramebuffer
#define glIsRenderbuffer __glewIsRenderbuffer
#define glRenderbufferStorage __glewRenderbufferStorage
#define glRenderbufferStorageMultisample __glewRenderbufferStorageMultisample
#define glFramebufferTextureARB __glewFramebufferTextureARB
#define glFramebufferTextureFaceARB __glewFramebufferTextureFaceARB
#define glFramebufferTextureLayerARB __glewFramebufferTextureLayerARB
#define glProgramParameteriARB __glewProgramParameteriARB
#define glGetProgramBinary __glewGetProgramBinary
#define glProgramBinary __glewProgramBinary
#define glProgramParameteri __glewProgramParameteri
#define glGetUniformdv __glewGetUniformdv
#define glProgramUniform1dEXT __glewProgramUniform1dEXT
#define glProgramUniform1dvEXT __glewProgramUniform1dvEXT
#define glProgramUniform2dEXT __glewProgramUniform2dEXT
#define glProgramUniform2dvEXT __glewProgramUniform2dvEXT
#define glProgramUniform3dEXT __glewProgramUniform3dEXT
#define glProgramUniform3dvEXT __glewProgramUniform3dvEXT
#define glProgramUniform4dEXT __glewProgramUniform4dEXT
#define glProgramUniform4dvEXT __glewProgramUniform4dvEXT
#define glProgramUniformMatrix2dvEXT __glewProgramUniformMatrix2dvEXT
#define glProgramUniformMatrix2x3dvEXT __glewProgramUniformMatrix2x3dvEXT
#define glProgramUniformMatrix2x4dvEXT __glewProgramUniformMatrix2x4dvEXT
#define glProgramUniformMatrix3dvEXT __glewProgramUniformMatrix3dvEXT
#define glProgramUniformMatrix3x2dvEXT __glewProgramUniformMatrix3x2dvEXT
#define glProgramUniformMatrix3x4dvEXT __glewProgramUniformMatrix3x4dvEXT
#define glProgramUniformMatrix4dvEXT __glewProgramUniformMatrix4dvEXT
#define glProgramUniformMatrix4x2dvEXT __glewProgramUniformMatrix4x2dvEXT
#define glProgramUniformMatrix4x3dvEXT __glewProgramUniformMatrix4x3dvEXT
#define glUniform1d __glewUniform1d
#define glUniform1dv __glewUniform1dv
#define glUniform2d __glewUniform2d
#define glUniform2dv __glewUniform2dv
#define glUniform3d __glewUniform3d
#define glUniform3dv __glewUniform3dv
#define glUniform4d __glewUniform4d
#define glUniform4dv __glewUniform4dv
#define glUniformMatrix2dv __glewUniformMatrix2dv
#define glUniformMatrix2x3dv __glewUniformMatrix2x3dv
#define glUniformMatrix2x4dv __glewUniformMatrix2x4dv
#define glUniformMatrix3dv __glewUniformMatrix3dv
#define glUniformMatrix3x2dv __glewUniformMatrix3x2dv
#define glUniformMatrix3x4dv __glewUniformMatrix3x4dv
#define glUniformMatrix4dv __glewUniformMatrix4dv
#define glUniformMatrix4x2dv __glewUniformMatrix4x2dv
#define glUniformMatrix4x3dv __glewUniformMatrix4x3dv
#define glColorSubTable __glewColorSubTable
#define glColorTable __glewColorTable
#define glColorTableParameterfv __glewColorTableParameterfv
#define glColorTableParameteriv __glewColorTableParameteriv
#define glConvolutionFilter1D __glewConvolutionFilter1D
#define glConvolutionFilter2D __glewConvolutionFilter2D
#define glConvolutionParameterf __glewConvolutionParameterf
#define glConvolutionParameterfv __glewConvolutionParameterfv
#define glConvolutionParameteri __glewConvolutionParameteri
#define glConvolutionParameteriv __glewConvolutionParameteriv
#define glCopyColorSubTable __glewCopyColorSubTable
#define glCopyColorTable __glewCopyColorTable
#define glCopyConvolutionFilter1D __glewCopyConvolutionFilter1D
#define glCopyConvolutionFilter2D __glewCopyConvolutionFilter2D
#define glGetColorTable __glewGetColorTable
#define glGetColorTableParameterfv __glewGetColorTableParameterfv
#define glGetColorTableParameteriv __glewGetColorTableParameteriv
#define glGetConvolutionFilter __glewGetConvolutionFilter
#define glGetConvolutionParameterfv __glewGetConvolutionParameterfv
#define glGetConvolutionParameteriv __glewGetConvolutionParameteriv
#define glGetHistogram __glewGetHistogram
#define glGetHistogramParameterfv __glewGetHistogramParameterfv
#define glGetHistogramParameteriv __glewGetHistogramParameteriv
#define glGetMinmax __glewGetMinmax
#define glGetMinmaxParameterfv __glewGetMinmaxParameterfv
#define glGetMinmaxParameteriv __glewGetMinmaxParameteriv
#define glGetSeparableFilter __glewGetSeparableFilter
#define glHistogram __glewHistogram
#define glMinmax __glewMinmax
#define glResetHistogram __glewResetHistogram
#define glResetMinmax __glewResetMinmax
#define glSeparableFilter2D __glewSeparableFilter2D
#define glDrawArraysInstancedARB __glewDrawArraysInstancedARB
#define glDrawElementsInstancedARB __glewDrawElementsInstancedARB
#define glVertexAttribDivisorARB __glewVertexAttribDivisorARB
#define glFlushMappedBufferRange __glewFlushMappedBufferRange
#define glMapBufferRange __glewMapBufferRange
#define glCurrentPaletteMatrixARB __glewCurrentPaletteMatrixARB
#define glMatrixIndexPointerARB __glewMatrixIndexPointerARB
#define glMatrixIndexubvARB __glewMatrixIndexubvARB
#define glMatrixIndexuivARB __glewMatrixIndexuivARB
#define glMatrixIndexusvARB __glewMatrixIndexusvARB
#define glSampleCoverageARB __glewSampleCoverageARB
#define glActiveTextureARB __glewActiveTextureARB
#define glClientActiveTextureARB __glewClientActiveTextureARB
#define glMultiTexCoord1dARB __glewMultiTexCoord1dARB
#define glMultiTexCoord1dvARB __glewMultiTexCoord1dvARB
#define glMultiTexCoord1fARB __glewMultiTexCoord1fARB
#define glMultiTexCoord1fvARB __glewMultiTexCoord1fvARB
#define glMultiTexCoord1iARB __glewMultiTexCoord1iARB
#define glMultiTexCoord1ivARB __glewMultiTexCoord1ivARB
#define glMultiTexCoord1sARB __glewMultiTexCoord1sARB
#define glMultiTexCoord1svARB __glewMultiTexCoord1svARB
#define glMultiTexCoord2dARB __glewMultiTexCoord2dARB
#define glMultiTexCoord2dvARB __glewMultiTexCoord2dvARB
#define glMultiTexCoord2fARB __glewMultiTexCoord2fARB
#define glMultiTexCoord2fvARB __glewMultiTexCoord2fvARB
#define glMultiTexCoord2iARB __glewMultiTexCoord2iARB
#define glMultiTexCoord2ivARB __glewMultiTexCoord2ivARB
#define glMultiTexCoord2sARB __glewMultiTexCoord2sARB
#define glMultiTexCoord2svARB __glewMultiTexCoord2svARB
#define glMultiTexCoord3dARB __glewMultiTexCoord3dARB
#define glMultiTexCoord3dvARB __glewMultiTexCoord3dvARB
#define glMultiTexCoord3fARB __glewMultiTexCoord3fARB
#define glMultiTexCoord3fvARB __glewMultiTexCoord3fvARB
#define glMultiTexCoord3iARB __glewMultiTexCoord3iARB
#define glMultiTexCoord3ivARB __glewMultiTexCoord3ivARB
#define glMultiTexCoord3sARB __glewMultiTexCoord3sARB
#define glMultiTexCoord3svARB __glewMultiTexCoord3svARB
#define glMultiTexCoord4dARB __glewMultiTexCoord4dARB
#define glMultiTexCoord4dvARB __glewMultiTexCoord4dvARB
#define glMultiTexCoord4fARB __glewMultiTexCoord4fARB
#define glMultiTexCoord4fvARB __glewMultiTexCoord4fvARB
#define glMultiTexCoord4iARB __glewMultiTexCoord4iARB
#define glMultiTexCoord4ivARB __glewMultiTexCoord4ivARB
#define glMultiTexCoord4sARB __glewMultiTexCoord4sARB
#define glMultiTexCoord4svARB __glewMultiTexCoord4svARB
#define glBeginQueryARB __glewBeginQueryARB
#define glDeleteQueriesARB __glewDeleteQueriesARB
#define glEndQueryARB __glewEndQueryARB
#define glGenQueriesARB __glewGenQueriesARB
#define glGetQueryObjectivARB __glewGetQueryObjectivARB
#define glGetQueryObjectuivARB __glewGetQueryObjectuivARB
#define glGetQueryivARB __glewGetQueryivARB
#define glIsQueryARB __glewIsQueryARB
#define glPointParameterfARB __glewPointParameterfARB
#define glPointParameterfvARB __glewPointParameterfvARB
#define glProvokingVertex __glewProvokingVertex
#define glGetnColorTableARB __glewGetnColorTableARB
#define glGetnCompressedTexImageARB __glewGetnCompressedTexImageARB
#define glGetnConvolutionFilterARB __glewGetnConvolutionFilterARB
#define glGetnHistogramARB __glewGetnHistogramARB
#define glGetnMapdvARB __glewGetnMapdvARB
#define glGetnMapfvARB __glewGetnMapfvARB
#define glGetnMapivARB __glewGetnMapivARB
#define glGetnMinmaxARB __glewGetnMinmaxARB
#define glGetnPixelMapfvARB __glewGetnPixelMapfvARB
#define glGetnPixelMapuivARB __glewGetnPixelMapuivARB
#define glGetnPixelMapusvARB __glewGetnPixelMapusvARB
#define glGetnPolygonStippleARB __glewGetnPolygonStippleARB
#define glGetnSeparableFilterARB __glewGetnSeparableFilterARB
#define glGetnTexImageARB __glewGetnTexImageARB
#define glGetnUniformdvARB __glewGetnUniformdvARB
#define glGetnUniformfvARB __glewGetnUniformfvARB
#define glGetnUniformivARB __glewGetnUniformivARB
#define glGetnUniformuivARB __glewGetnUniformuivARB
#define glReadnPixelsARB __glewReadnPixelsARB
#define glMinSampleShadingARB __glewMinSampleShadingARB
#define glBindSampler __glewBindSampler
#define glDeleteSamplers __glewDeleteSamplers
#define glGenSamplers __glewGenSamplers
#define glGetSamplerParameterIiv __glewGetSamplerParameterIiv
#define glGetSamplerParameterIuiv __glewGetSamplerParameterIuiv
#define glGetSamplerParameterfv __glewGetSamplerParameterfv
#define glGetSamplerParameteriv __glewGetSamplerParameteriv
#define glIsSampler __glewIsSampler
#define glSamplerParameterIiv __glewSamplerParameterIiv
#define glSamplerParameterIuiv __glewSamplerParameterIuiv
#define glSamplerParameterf __glewSamplerParameterf
#define glSamplerParameterfv __glewSamplerParameterfv
#define glSamplerParameteri __glewSamplerParameteri
#define glSamplerParameteriv __glewSamplerParameteriv
#define glActiveShaderProgram __glewActiveShaderProgram
#define glBindProgramPipeline __glewBindProgramPipeline
#define glCreateShaderProgramv __glewCreateShaderProgramv
#define glDeleteProgramPipelines __glewDeleteProgramPipelines
#define glGenProgramPipelines __glewGenProgramPipelines
#define glGetProgramPipelineInfoLog __glewGetProgramPipelineInfoLog
#define glGetProgramPipelineiv __glewGetProgramPipelineiv
#define glIsProgramPipeline __glewIsProgramPipeline
#define glProgramUniform1d __glewProgramUniform1d
#define glProgramUniform1dv __glewProgramUniform1dv
#define glProgramUniform1f __glewProgramUniform1f
#define glProgramUniform1fv __glewProgramUniform1fv
#define glProgramUniform1i __glewProgramUniform1i
#define glProgramUniform1iv __glewProgramUniform1iv
#define glProgramUniform1ui __glewProgramUniform1ui
#define glProgramUniform1uiv __glewProgramUniform1uiv
#define glProgramUniform2d __glewProgramUniform2d
#define glProgramUniform2dv __glewProgramUniform2dv
#define glProgramUniform2f __glewProgramUniform2f
#define glProgramUniform2fv __glewProgramUniform2fv
#define glProgramUniform2i __glewProgramUniform2i
#define glProgramUniform2iv __glewProgramUniform2iv
#define glProgramUniform2ui __glewProgramUniform2ui
#define glProgramUniform2uiv __glewProgramUniform2uiv
#define glProgramUniform3d __glewProgramUniform3d
#define glProgramUniform3dv __glewProgramUniform3dv
#define glProgramUniform3f __glewProgramUniform3f
#define glProgramUniform3fv __glewProgramUniform3fv
#define glProgramUniform3i __glewProgramUniform3i
#define glProgramUniform3iv __glewProgramUniform3iv
#define glProgramUniform3ui __glewProgramUniform3ui
#define glProgramUniform3uiv __glewProgramUniform3uiv
#define glProgramUniform4d __glewProgramUniform4d
#define glProgramUniform4dv __glewProgramUniform4dv
#define glProgramUniform4f __glewProgramUniform4f
#define glProgramUniform4fv __glewProgramUniform4fv
#define glProgramUniform4i __glewProgramUniform4i
#define glProgramUniform4iv __glewProgramUniform4iv
#define glProgramUniform4ui __glewProgramUniform4ui
#define glProgramUniform4uiv __glewProgramUniform4uiv
#define glProgramUniformMatrix2dv __glewProgramUniformMatrix2dv
#define glProgramUniformMatrix2fv __glewProgramUniformMatrix2fv
#define glProgramUniformMatrix2x3dv __glewProgramUniformMatrix2x3dv
#define glProgramUniformMatrix2x3fv __glewProgramUniformMatrix2x3fv
#define glProgramUniformMatrix2x4dv __glewProgramUniformMatrix2x4dv
#define glProgramUniformMatrix2x4fv __glewProgramUniformMatrix2x4fv
#define glProgramUniformMatrix3dv __glewProgramUniformMatrix3dv
#define glProgramUniformMatrix3fv __glewProgramUniformMatrix3fv
#define glProgramUniformMatrix3x2dv __glewProgramUniformMatrix3x2dv
#define glProgramUniformMatrix3x2fv __glewProgramUniformMatrix3x2fv
#define glProgramUniformMatrix3x4dv __glewProgramUniformMatrix3x4dv
#define glProgramUniformMatrix3x4fv __glewProgramUniformMatrix3x4fv
#define glProgramUniformMatrix4dv __glewProgramUniformMatrix4dv
#define glProgramUniformMatrix4fv __glewProgramUniformMatrix4fv
#define glProgramUniformMatrix4x2dv __glewProgramUniformMatrix4x2dv
#define glProgramUniformMatrix4x2fv __glewProgramUniformMatrix4x2fv
#define glProgramUniformMatrix4x3dv __glewProgramUniformMatrix4x3dv
#define glProgramUniformMatrix4x3fv __glewProgramUniformMatrix4x3fv
#define glUseProgramStages __glewUseProgramStages
#define glValidateProgramPipeline __glewValidateProgramPipeline
#define glAttachObjectARB __glewAttachObjectARB
#define glCompileShaderARB __glewCompileShaderARB
#define glCreateProgramObjectARB __glewCreateProgramObjectARB
#define glCreateShaderObjectARB __glewCreateShaderObjectARB
#define glDeleteObjectARB __glewDeleteObjectARB
#define glDetachObjectARB __glewDetachObjectARB
#define glGetActiveUniformARB __glewGetActiveUniformARB
#define glGetAttachedObjectsARB __glewGetAttachedObjectsARB
#define glGetHandleARB __glewGetHandleARB
#define glGetInfoLogARB __glewGetInfoLogARB
#define glGetObjectParameterfvARB __glewGetObjectParameterfvARB
#define glGetObjectParameterivARB __glewGetObjectParameterivARB
#define glGetShaderSourceARB __glewGetShaderSourceARB
#define glGetUniformLocationARB __glewGetUniformLocationARB
#define glGetUniformfvARB __glewGetUniformfvARB
#define glGetUniformivARB __glewGetUniformivARB
#define glLinkProgramARB __glewLinkProgramARB
#define glShaderSourceARB __glewShaderSourceARB
#define glUniform1fARB __glewUniform1fARB
#define glUniform1fvARB __glewUniform1fvARB
#define glUniform1iARB __glewUniform1iARB
#define glUniform1ivARB __glewUniform1ivARB
#define glUniform2fARB __glewUniform2fARB
#define glUniform2fvARB __glewUniform2fvARB
#define glUniform2iARB __glewUniform2iARB
#define glUniform2ivARB __glewUniform2ivARB
#define glUniform3fARB __glewUniform3fARB
#define glUniform3fvARB __glewUniform3fvARB
#define glUniform3iARB __glewUniform3iARB
#define glUniform3ivARB __glewUniform3ivARB
#define glUniform4fARB __glewUniform4fARB
#define glUniform4fvARB __glewUniform4fvARB
#define glUniform4iARB __glewUniform4iARB
#define glUniform4ivARB __glewUniform4ivARB
#define glUniformMatrix2fvARB __glewUniformMatrix2fvARB
#define glUniformMatrix3fvARB __glewUniformMatrix3fvARB
#define glUniformMatrix4fvARB __glewUniformMatrix4fvARB
#define glUseProgramObjectARB __glewUseProgramObjectARB
#define glValidateProgramARB __glewValidateProgramARB
#define glGetActiveSubroutineName __glewGetActiveSubroutineName
#define glGetActiveSubroutineUniformName __glewGetActiveSubroutineUniformName
#define glGetActiveSubroutineUniformiv __glewGetActiveSubroutineUniformiv
#define glGetProgramStageiv __glewGetProgramStageiv
#define glGetSubroutineIndex __glewGetSubroutineIndex
#define glGetSubroutineUniformLocation __glewGetSubroutineUniformLocation
#define glGetUniformSubroutineuiv __glewGetUniformSubroutineuiv
#define glUniformSubroutinesuiv __glewUniformSubroutinesuiv
#define glCompileShaderIncludeARB __glewCompileShaderIncludeARB
#define glDeleteNamedStringARB __glewDeleteNamedStringARB
#define glGetNamedStringARB __glewGetNamedStringARB
#define glGetNamedStringivARB __glewGetNamedStringivARB
#define glIsNamedStringARB __glewIsNamedStringARB
#define glNamedStringARB __glewNamedStringARB
#define glClientWaitSync __glewClientWaitSync
#define glDeleteSync __glewDeleteSync
#define glFenceSync __glewFenceSync
#define glGetInteger64v __glewGetInteger64v
#define glGetSynciv __glewGetSynciv
#define glIsSync __glewIsSync
#define glWaitSync __glewWaitSync
#define glPatchParameterfv __glewPatchParameterfv
#define glPatchParameteri __glewPatchParameteri
#define glTexBufferARB __glewTexBufferARB
#define glCompressedTexImage1DARB __glewCompressedTexImage1DARB
#define glCompressedTexImage2DARB __glewCompressedTexImage2DARB
#define glCompressedTexImage3DARB __glewCompressedTexImage3DARB
#define glCompressedTexSubImage1DARB __glewCompressedTexSubImage1DARB
#define glCompressedTexSubImage2DARB __glewCompressedTexSubImage2DARB
#define glCompressedTexSubImage3DARB __glewCompressedTexSubImage3DARB
#define glGetCompressedTexImageARB __glewGetCompressedTexImageARB
#define glGetMultisamplefv __glewGetMultisamplefv
#define glSampleMaski __glewSampleMaski
#define glTexImage2DMultisample __glewTexImage2DMultisample
#define glTexImage3DMultisample __glewTexImage3DMultisample
#define glGetQueryObjecti64v __glewGetQueryObjecti64v
#define glGetQueryObjectui64v __glewGetQueryObjectui64v
#define glQueryCounter __glewQueryCounter
#define glBindTransformFeedback __glewBindTransformFeedback
#define glDeleteTransformFeedbacks __glewDeleteTransformFeedbacks
#define glDrawTransformFeedback __glewDrawTransformFeedback
#define glGenTransformFeedbacks __glewGenTransformFeedbacks
#define glIsTransformFeedback __glewIsTransformFeedback
#define glPauseTransformFeedback __glewPauseTransformFeedback
#define glResumeTransformFeedback __glewResumeTransformFeedback
#define glBeginQueryIndexed __glewBeginQueryIndexed
#define glDrawTransformFeedbackStream __glewDrawTransformFeedbackStream
#define glEndQueryIndexed __glewEndQueryIndexed
#define glGetQueryIndexediv __glewGetQueryIndexediv
#define glLoadTransposeMatrixdARB __glewLoadTransposeMatrixdARB
#define glLoadTransposeMatrixfARB __glewLoadTransposeMatrixfARB
#define glMultTransposeMatrixdARB __glewMultTransposeMatrixdARB
#define glMultTransposeMatrixfARB __glewMultTransposeMatrixfARB
#define glBindBufferBase __glewBindBufferBase
#define glBindBufferRange __glewBindBufferRange
#define glGetActiveUniformBlockName __glewGetActiveUniformBlockName
#define glGetActiveUniformBlockiv __glewGetActiveUniformBlockiv
#define glGetActiveUniformName __glewGetActiveUniformName
#define glGetActiveUniformsiv __glewGetActiveUniformsiv
#define glGetIntegeri_v __glewGetIntegeri_v
#define glGetUniformBlockIndex __glewGetUniformBlockIndex
#define glGetUniformIndices __glewGetUniformIndices
#define glUniformBlockBinding __glewUniformBlockBinding
#define glBindVertexArray __glewBindVertexArray
#define glDeleteVertexArrays __glewDeleteVertexArrays
#define glGenVertexArrays __glewGenVertexArrays
#define glIsVertexArray __glewIsVertexArray
#define glGetVertexAttribLdv __glewGetVertexAttribLdv
#define glVertexAttribL1d __glewVertexAttribL1d
#define glVertexAttribL1dv __glewVertexAttribL1dv
#define glVertexAttribL2d __glewVertexAttribL2d
#define glVertexAttribL2dv __glewVertexAttribL2dv
#define glVertexAttribL3d __glewVertexAttribL3d
#define glVertexAttribL3dv __glewVertexAttribL3dv
#define glVertexAttribL4d __glewVertexAttribL4d
#define glVertexAttribL4dv __glewVertexAttribL4dv
#define glVertexAttribLPointer __glewVertexAttribLPointer
#define glVertexBlendARB __glewVertexBlendARB
#define glWeightPointerARB __glewWeightPointerARB
#define glWeightbvARB __glewWeightbvARB
#define glWeightdvARB __glewWeightdvARB
#define glWeightfvARB __glewWeightfvARB
#define glWeightivARB __glewWeightivARB
#define glWeightsvARB __glewWeightsvARB
#define glWeightubvARB __glewWeightubvARB
#define glWeightuivARB __glewWeightuivARB
#define glWeightusvARB __glewWeightusvARB
#define glBindBufferARB __glewBindBufferARB
#define glBufferDataARB __glewBufferDataARB
#define glBufferSubDataARB __glewBufferSubDataARB
#define glDeleteBuffersARB __glewDeleteBuffersARB
#define glGenBuffersARB __glewGenBuffersARB
#define glGetBufferParameterivARB __glewGetBufferParameterivARB
#define glGetBufferPointervARB __glewGetBufferPointervARB
#define glGetBufferSubDataARB __glewGetBufferSubDataARB
#define glIsBufferARB __glewIsBufferARB
#define glMapBufferARB __glewMapBufferARB
#define glUnmapBufferARB __glewUnmapBufferARB
#define glBindProgramARB __glewBindProgramARB
#define glDeleteProgramsARB __glewDeleteProgramsARB
#define glDisableVertexAttribArrayARB __glewDisableVertexAttribArrayARB
#define glEnableVertexAttribArrayARB __glewEnableVertexAttribArrayARB
#define glGenProgramsARB __glewGenProgramsARB
#define glGetProgramEnvParameterdvARB __glewGetProgramEnvParameterdvARB
#define glGetProgramEnvParameterfvARB __glewGetProgramEnvParameterfvARB
#define glGetProgramLocalParameterdvARB __glewGetProgramLocalParameterdvARB
#define glGetProgramLocalParameterfvARB __glewGetProgramLocalParameterfvARB
#define glGetProgramStringARB __glewGetProgramStringARB
#define glGetProgramivARB __glewGetProgramivARB
#define glGetVertexAttribPointervARB __glewGetVertexAttribPointervARB
#define glGetVertexAttribdvARB __glewGetVertexAttribdvARB
#define glGetVertexAttribfvARB __glewGetVertexAttribfvARB
#define glGetVertexAttribivARB __glewGetVertexAttribivARB
#define glIsProgramARB __glewIsProgramARB
#define glProgramEnvParameter4dARB __glewProgramEnvParameter4dARB
#define glProgramEnvParameter4dvARB __glewProgramEnvParameter4dvARB
#define glProgramEnvParameter4fARB __glewProgramEnvParameter4fARB
#define glProgramEnvParameter4fvARB __glewProgramEnvParameter4fvARB
#define glProgramLocalParameter4dARB __glewProgramLocalParameter4dARB
#define glProgramLocalParameter4dvARB __glewProgramLocalParameter4dvARB
#define glProgramLocalParameter4fARB __glewProgramLocalParameter4fARB
#define glProgramLocalParameter4fvARB __glewProgramLocalParameter4fvARB
#define glProgramStringARB __glewProgramStringARB
#define glVertexAttrib1dARB __glewVertexAttrib1dARB
#define glVertexAttrib1dvARB __glewVertexAttrib1dvARB
#define glVertexAttrib1fARB __glewVertexAttrib1fARB
#define glVertexAttrib1fvARB __glewVertexAttrib1fvARB
#define glVertexAttrib1sARB __glewVertexAttrib1sARB
#define glVertexAttrib1svARB __glewVertexAttrib1svARB
#define glVertexAttrib2dARB __glewVertexAttrib2dARB
#define glVertexAttrib2dvARB __glewVertexAttrib2dvARB
#define glVertexAttrib2fARB __glewVertexAttrib2fARB
#define glVertexAttrib2fvARB __glewVertexAttrib2fvARB
#define glVertexAttrib2sARB __glewVertexAttrib2sARB
#define glVertexAttrib2svARB __glewVertexAttrib2svARB
#define glVertexAttrib3dARB __glewVertexAttrib3dARB
#define glVertexAttrib3dvARB __glewVertexAttrib3dvARB
#define glVertexAttrib3fARB __glewVertexAttrib3fARB
#define glVertexAttrib3fvARB __glewVertexAttrib3fvARB
#define glVertexAttrib3sARB __glewVertexAttrib3sARB
#define glVertexAttrib3svARB __glewVertexAttrib3svARB
#define glVertexAttrib4NbvARB __glewVertexAttrib4NbvARB
#define glVertexAttrib4NivARB __glewVertexAttrib4NivARB
#define glVertexAttrib4NsvARB __glewVertexAttrib4NsvARB
#define glVertexAttrib4NubARB __glewVertexAttrib4NubARB
#define glVertexAttrib4NubvARB __glewVertexAttrib4NubvARB
#define glVertexAttrib4NuivARB __glewVertexAttrib4NuivARB
#define glVertexAttrib4NusvARB __glewVertexAttrib4NusvARB
#define glVertexAttrib4bvARB __glewVertexAttrib4bvARB
#define glVertexAttrib4dARB __glewVertexAttrib4dARB
#define glVertexAttrib4dvARB __glewVertexAttrib4dvARB
#define glVertexAttrib4fARB __glewVertexAttrib4fARB
#define glVertexAttrib4fvARB __glewVertexAttrib4fvARB
#define glVertexAttrib4ivARB __glewVertexAttrib4ivARB
#define glVertexAttrib4sARB __glewVertexAttrib4sARB
#define glVertexAttrib4svARB __glewVertexAttrib4svARB
#define glVertexAttrib4ubvARB __glewVertexAttrib4ubvARB
#define glVertexAttrib4uivARB __glewVertexAttrib4uivARB
#define glVertexAttrib4usvARB __glewVertexAttrib4usvARB
#define glVertexAttribPointerARB __glewVertexAttribPointerARB
#define glBindAttribLocationARB __glewBindAttribLocationARB
#define glGetActiveAttribARB __glewGetActiveAttribARB
#define glGetAttribLocationARB __glewGetAttribLocationARB
#define glColorP3ui __glewColorP3ui
#define glColorP3uiv __glewColorP3uiv
#define glColorP4ui __glewColorP4ui
#define glColorP4uiv __glewColorP4uiv
#define glMultiTexCoordP1ui __glewMultiTexCoordP1ui
#define glMultiTexCoordP1uiv __glewMultiTexCoordP1uiv
#define glMultiTexCoordP2ui __glewMultiTexCoordP2ui
#define glMultiTexCoordP2uiv __glewMultiTexCoordP2uiv
#define glMultiTexCoordP3ui __glewMultiTexCoordP3ui
#define glMultiTexCoordP3uiv __glewMultiTexCoordP3uiv
#define glMultiTexCoordP4ui __glewMultiTexCoordP4ui
#define glMultiTexCoordP4uiv __glewMultiTexCoordP4uiv
#define glNormalP3ui __glewNormalP3ui
#define glNormalP3uiv __glewNormalP3uiv
#define glSecondaryColorP3ui __glewSecondaryColorP3ui
#define glSecondaryColorP3uiv __glewSecondaryColorP3uiv
#define glTexCoordP1ui __glewTexCoordP1ui
#define glTexCoordP1uiv __glewTexCoordP1uiv
#define glTexCoordP2ui __glewTexCoordP2ui
#define glTexCoordP2uiv __glewTexCoordP2uiv
#define glTexCoordP3ui __glewTexCoordP3ui
#define glTexCoordP3uiv __glewTexCoordP3uiv
#define glTexCoordP4ui __glewTexCoordP4ui
#define glTexCoordP4uiv __glewTexCoordP4uiv
#define glVertexAttribP1ui __glewVertexAttribP1ui
#define glVertexAttribP1uiv __glewVertexAttribP1uiv
#define glVertexAttribP2ui __glewVertexAttribP2ui
#define glVertexAttribP2uiv __glewVertexAttribP2uiv
#define glVertexAttribP3ui __glewVertexAttribP3ui
#define glVertexAttribP3uiv __glewVertexAttribP3uiv
#define glVertexAttribP4ui __glewVertexAttribP4ui
#define glVertexAttribP4uiv __glewVertexAttribP4uiv
#define glVertexP2ui __glewVertexP2ui
#define glVertexP2uiv __glewVertexP2uiv
#define glVertexP3ui __glewVertexP3ui
#define glVertexP3uiv __glewVertexP3uiv
#define glVertexP4ui __glewVertexP4ui
#define glVertexP4uiv __glewVertexP4uiv
#define glDepthRangeArrayv __glewDepthRangeArrayv
#define glDepthRangeIndexed __glewDepthRangeIndexed
#define glGetDoublei_v __glewGetDoublei_v
#define glGetFloati_v __glewGetFloati_v
#define glScissorArrayv __glewScissorArrayv
#define glScissorIndexed __glewScissorIndexed
#define glScissorIndexedv __glewScissorIndexedv
#define glViewportArrayv __glewViewportArrayv
#define glViewportIndexedf __glewViewportIndexedf
#define glViewportIndexedfv __glewViewportIndexedfv
#define glWindowPos2dARB __glewWindowPos2dARB
#define glWindowPos2dvARB __glewWindowPos2dvARB
#define glWindowPos2fARB __glewWindowPos2fARB
#define glWindowPos2fvARB __glewWindowPos2fvARB
#define glWindowPos2iARB __glewWindowPos2iARB
#define glWindowPos2ivARB __glewWindowPos2ivARB
#define glWindowPos2sARB __glewWindowPos2sARB
#define glWindowPos2svARB __glewWindowPos2svARB
#define glWindowPos3dARB __glewWindowPos3dARB
#define glWindowPos3dvARB __glewWindowPos3dvARB
#define glWindowPos3fARB __glewWindowPos3fARB
#define glWindowPos3fvARB __glewWindowPos3fvARB
#define glWindowPos3iARB __glewWindowPos3iARB
#define glWindowPos3ivARB __glewWindowPos3ivARB
#define glWindowPos3sARB __glewWindowPos3sARB
#define glWindowPos3svARB __glewWindowPos3svARB
#define glDrawBuffersATI __glewDrawBuffersATI
#define glDrawElementArrayATI __glewDrawElementArrayATI
#define glDrawRangeElementArrayATI __glewDrawRangeElementArrayATI
#define glElementPointerATI __glewElementPointerATI
#define glGetTexBumpParameterfvATI __glewGetTexBumpParameterfvATI
#define glGetTexBumpParameterivATI __glewGetTexBumpParameterivATI
#define glTexBumpParameterfvATI __glewTexBumpParameterfvATI
#define glTexBumpParameterivATI __glewTexBumpParameterivATI
#define glAlphaFragmentOp1ATI __glewAlphaFragmentOp1ATI
#define glAlphaFragmentOp2ATI __glewAlphaFragmentOp2ATI
#define glAlphaFragmentOp3ATI __glewAlphaFragmentOp3ATI
#define glBeginFragmentShaderATI __glewBeginFragmentShaderATI
#define glBindFragmentShaderATI __glewBindFragmentShaderATI
#define glColorFragmentOp1ATI __glewColorFragmentOp1ATI
#define glColorFragmentOp2ATI __glewColorFragmentOp2ATI
#define glColorFragmentOp3ATI __glewColorFragmentOp3ATI
#define glDeleteFragmentShaderATI __glewDeleteFragmentShaderATI
#define glEndFragmentShaderATI __glewEndFragmentShaderATI
#define glGenFragmentShadersATI __glewGenFragmentShadersATI
#define glPassTexCoordATI __glewPassTexCoordATI
#define glSampleMapATI __glewSampleMapATI
#define glSetFragmentShaderConstantATI __glewSetFragmentShaderConstantATI
#define glMapObjectBufferATI __glewMapObjectBufferATI
#define glUnmapObjectBufferATI __glewUnmapObjectBufferATI
#define glPNTrianglesfATI __glewPNTrianglesfATI
#define glPNTrianglesiATI __glewPNTrianglesiATI
#define glStencilFuncSeparateATI __glewStencilFuncSeparateATI
#define glStencilOpSeparateATI __glewStencilOpSeparateATI
#define glArrayObjectATI __glewArrayObjectATI
#define glFreeObjectBufferATI __glewFreeObjectBufferATI
#define glGetArrayObjectfvATI __glewGetArrayObjectfvATI
#define glGetArrayObjectivATI __glewGetArrayObjectivATI
#define glGetObjectBufferfvATI __glewGetObjectBufferfvATI
#define glGetObjectBufferivATI __glewGetObjectBufferivATI
#define glGetVariantArrayObjectfvATI __glewGetVariantArrayObjectfvATI
#define glGetVariantArrayObjectivATI __glewGetVariantArrayObjectivATI
#define glIsObjectBufferATI __glewIsObjectBufferATI
#define glNewObjectBufferATI __glewNewObjectBufferATI
#define glUpdateObjectBufferATI __glewUpdateObjectBufferATI
#define glVariantArrayObjectATI __glewVariantArrayObjectATI
#define glGetVertexAttribArrayObjectfvATI __glewGetVertexAttribArrayObjectfvATI
#define glGetVertexAttribArrayObjectivATI __glewGetVertexAttribArrayObjectivATI
#define glVertexAttribArrayObjectATI __glewVertexAttribArrayObjectATI
#define glClientActiveVertexStreamATI __glewClientActiveVertexStreamATI
#define glNormalStream3bATI __glewNormalStream3bATI
#define glNormalStream3bvATI __glewNormalStream3bvATI
#define glNormalStream3dATI __glewNormalStream3dATI
#define glNormalStream3dvATI __glewNormalStream3dvATI
#define glNormalStream3fATI __glewNormalStream3fATI
#define glNormalStream3fvATI __glewNormalStream3fvATI
#define glNormalStream3iATI __glewNormalStream3iATI
#define glNormalStream3ivATI __glewNormalStream3ivATI
#define glNormalStream3sATI __glewNormalStream3sATI
#define glNormalStream3svATI __glewNormalStream3svATI
#define glVertexBlendEnvfATI __glewVertexBlendEnvfATI
#define glVertexBlendEnviATI __glewVertexBlendEnviATI
#define glVertexStream2dATI __glewVertexStream2dATI
#define glVertexStream2dvATI __glewVertexStream2dvATI
#define glVertexStream2fATI __glewVertexStream2fATI
#define glVertexStream2fvATI __glewVertexStream2fvATI
#define glVertexStream2iATI __glewVertexStream2iATI
#define glVertexStream2ivATI __glewVertexStream2ivATI
#define glVertexStream2sATI __glewVertexStream2sATI
#define glVertexStream2svATI __glewVertexStream2svATI
#define glVertexStream3dATI __glewVertexStream3dATI
#define glVertexStream3dvATI __glewVertexStream3dvATI
#define glVertexStream3fATI __glewVertexStream3fATI
#define glVertexStream3fvATI __glewVertexStream3fvATI
#define glVertexStream3iATI __glewVertexStream3iATI
#define glVertexStream3ivATI __glewVertexStream3ivATI
#define glVertexStream3sATI __glewVertexStream3sATI
#define glVertexStream3svATI __glewVertexStream3svATI
#define glVertexStream4dATI __glewVertexStream4dATI
#define glVertexStream4dvATI __glewVertexStream4dvATI
#define glVertexStream4fATI __glewVertexStream4fATI
#define glVertexStream4fvATI __glewVertexStream4fvATI
#define glVertexStream4iATI __glewVertexStream4iATI
#define glVertexStream4ivATI __glewVertexStream4ivATI
#define glVertexStream4sATI __glewVertexStream4sATI
#define glVertexStream4svATI __glewVertexStream4svATI
#define glGetUniformBufferSizeEXT __glewGetUniformBufferSizeEXT
#define glGetUniformOffsetEXT __glewGetUniformOffsetEXT
#define glUniformBufferEXT __glewUniformBufferEXT
#define glBlendColorEXT __glewBlendColorEXT
#define glBlendEquationSeparateEXT __glewBlendEquationSeparateEXT
#define glBlendFuncSeparateEXT __glewBlendFuncSeparateEXT
#define glBlendEquationEXT __glewBlendEquationEXT
#define glColorSubTableEXT __glewColorSubTableEXT
#define glCopyColorSubTableEXT __glewCopyColorSubTableEXT
#define glLockArraysEXT __glewLockArraysEXT
#define glUnlockArraysEXT __glewUnlockArraysEXT
#define glConvolutionFilter1DEXT __glewConvolutionFilter1DEXT
#define glConvolutionFilter2DEXT __glewConvolutionFilter2DEXT
#define glConvolutionParameterfEXT __glewConvolutionParameterfEXT
#define glConvolutionParameterfvEXT __glewConvolutionParameterfvEXT
#define glConvolutionParameteriEXT __glewConvolutionParameteriEXT
#define glConvolutionParameterivEXT __glewConvolutionParameterivEXT
#define glCopyConvolutionFilter1DEXT __glewCopyConvolutionFilter1DEXT
#define glCopyConvolutionFilter2DEXT __glewCopyConvolutionFilter2DEXT
#define glGetConvolutionFilterEXT __glewGetConvolutionFilterEXT
#define glGetConvolutionParameterfvEXT __glewGetConvolutionParameterfvEXT
#define glGetConvolutionParameterivEXT __glewGetConvolutionParameterivEXT
#define glGetSeparableFilterEXT __glewGetSeparableFilterEXT
#define glSeparableFilter2DEXT __glewSeparableFilter2DEXT
#define glBinormalPointerEXT __glewBinormalPointerEXT
#define glTangentPointerEXT __glewTangentPointerEXT
#define glCopyTexImage1DEXT __glewCopyTexImage1DEXT
#define glCopyTexImage2DEXT __glewCopyTexImage2DEXT
#define glCopyTexSubImage1DEXT __glewCopyTexSubImage1DEXT
#define glCopyTexSubImage2DEXT __glewCopyTexSubImage2DEXT
#define glCopyTexSubImage3DEXT __glewCopyTexSubImage3DEXT
#define glCullParameterdvEXT __glewCullParameterdvEXT
#define glCullParameterfvEXT __glewCullParameterfvEXT
#define glDepthBoundsEXT __glewDepthBoundsEXT
#define glBindMultiTextureEXT __glewBindMultiTextureEXT
#define glCheckNamedFramebufferStatusEXT __glewCheckNamedFramebufferStatusEXT
#define glClientAttribDefaultEXT __glewClientAttribDefaultEXT
#define glCompressedMultiTexImage1DEXT __glewCompressedMultiTexImage1DEXT
#define glCompressedMultiTexImage2DEXT __glewCompressedMultiTexImage2DEXT
#define glCompressedMultiTexImage3DEXT __glewCompressedMultiTexImage3DEXT
#define glCompressedMultiTexSubImage1DEXT __glewCompressedMultiTexSubImage1DEXT
#define glCompressedMultiTexSubImage2DEXT __glewCompressedMultiTexSubImage2DEXT
#define glCompressedMultiTexSubImage3DEXT __glewCompressedMultiTexSubImage3DEXT
#define glCompressedTextureImage1DEXT __glewCompressedTextureImage1DEXT
#define glCompressedTextureImage2DEXT __glewCompressedTextureImage2DEXT
#define glCompressedTextureImage3DEXT __glewCompressedTextureImage3DEXT
#define glCompressedTextureSubImage1DEXT __glewCompressedTextureSubImage1DEXT
#define glCompressedTextureSubImage2DEXT __glewCompressedTextureSubImage2DEXT
#define glCompressedTextureSubImage3DEXT __glewCompressedTextureSubImage3DEXT
#define glCopyMultiTexImage1DEXT __glewCopyMultiTexImage1DEXT
#define glCopyMultiTexImage2DEXT __glewCopyMultiTexImage2DEXT
#define glCopyMultiTexSubImage1DEXT __glewCopyMultiTexSubImage1DEXT
#define glCopyMultiTexSubImage2DEXT __glewCopyMultiTexSubImage2DEXT
#define glCopyMultiTexSubImage3DEXT __glewCopyMultiTexSubImage3DEXT
#define glCopyTextureImage1DEXT __glewCopyTextureImage1DEXT
#define glCopyTextureImage2DEXT __glewCopyTextureImage2DEXT
#define glCopyTextureSubImage1DEXT __glewCopyTextureSubImage1DEXT
#define glCopyTextureSubImage2DEXT __glewCopyTextureSubImage2DEXT
#define glCopyTextureSubImage3DEXT __glewCopyTextureSubImage3DEXT
#define glDisableClientStateIndexedEXT __glewDisableClientStateIndexedEXT
#define glDisableClientStateiEXT __glewDisableClientStateiEXT
#define glDisableVertexArrayAttribEXT __glewDisableVertexArrayAttribEXT
#define glDisableVertexArrayEXT __glewDisableVertexArrayEXT
#define glEnableClientStateIndexedEXT __glewEnableClientStateIndexedEXT
#define glEnableClientStateiEXT __glewEnableClientStateiEXT
#define glEnableVertexArrayAttribEXT __glewEnableVertexArrayAttribEXT
#define glEnableVertexArrayEXT __glewEnableVertexArrayEXT
#define glFlushMappedNamedBufferRangeEXT __glewFlushMappedNamedBufferRangeEXT
#define glFramebufferDrawBufferEXT __glewFramebufferDrawBufferEXT
#define glFramebufferDrawBuffersEXT __glewFramebufferDrawBuffersEXT
#define glFramebufferReadBufferEXT __glewFramebufferReadBufferEXT
#define glGenerateMultiTexMipmapEXT __glewGenerateMultiTexMipmapEXT
#define glGenerateTextureMipmapEXT __glewGenerateTextureMipmapEXT
#define glGetCompressedMultiTexImageEXT __glewGetCompressedMultiTexImageEXT
#define glGetCompressedTextureImageEXT __glewGetCompressedTextureImageEXT
#define glGetDoubleIndexedvEXT __glewGetDoubleIndexedvEXT
#define glGetDoublei_vEXT __glewGetDoublei_vEXT
#define glGetFloatIndexedvEXT __glewGetFloatIndexedvEXT
#define glGetFloati_vEXT __glewGetFloati_vEXT
#define glGetFramebufferParameterivEXT __glewGetFramebufferParameterivEXT
#define glGetMultiTexEnvfvEXT __glewGetMultiTexEnvfvEXT
#define glGetMultiTexEnvivEXT __glewGetMultiTexEnvivEXT
#define glGetMultiTexGendvEXT __glewGetMultiTexGendvEXT
#define glGetMultiTexGenfvEXT __glewGetMultiTexGenfvEXT
#define glGetMultiTexGenivEXT __glewGetMultiTexGenivEXT
#define glGetMultiTexImageEXT __glewGetMultiTexImageEXT
#define glGetMultiTexLevelParameterfvEXT __glewGetMultiTexLevelParameterfvEXT
#define glGetMultiTexLevelParameterivEXT __glewGetMultiTexLevelParameterivEXT
#define glGetMultiTexParameterIivEXT __glewGetMultiTexParameterIivEXT
#define glGetMultiTexParameterIuivEXT __glewGetMultiTexParameterIuivEXT
#define glGetMultiTexParameterfvEXT __glewGetMultiTexParameterfvEXT
#define glGetMultiTexParameterivEXT __glewGetMultiTexParameterivEXT
#define glGetNamedBufferParameterivEXT __glewGetNamedBufferParameterivEXT
#define glGetNamedBufferPointervEXT __glewGetNamedBufferPointervEXT
#define glGetNamedBufferSubDataEXT __glewGetNamedBufferSubDataEXT
#define glGetNamedFramebufferAttachmentParameterivEXT __glewGetNamedFramebufferAttachmentParameterivEXT
#define glGetNamedProgramLocalParameterIivEXT __glewGetNamedProgramLocalParameterIivEXT
#define glGetNamedProgramLocalParameterIuivEXT __glewGetNamedProgramLocalParameterIuivEXT
#define glGetNamedProgramLocalParameterdvEXT __glewGetNamedProgramLocalParameterdvEXT
#define glGetNamedProgramLocalParameterfvEXT __glewGetNamedProgramLocalParameterfvEXT
#define glGetNamedProgramStringEXT __glewGetNamedProgramStringEXT
#define glGetNamedProgramivEXT __glewGetNamedProgramivEXT
#define glGetNamedRenderbufferParameterivEXT __glewGetNamedRenderbufferParameterivEXT
#define glGetPointerIndexedvEXT __glewGetPointerIndexedvEXT
#define glGetPointeri_vEXT __glewGetPointeri_vEXT
#define glGetTextureImageEXT __glewGetTextureImageEXT
#define glGetTextureLevelParameterfvEXT __glewGetTextureLevelParameterfvEXT
#define glGetTextureLevelParameterivEXT __glewGetTextureLevelParameterivEXT
#define glGetTextureParameterIivEXT __glewGetTextureParameterIivEXT
#define glGetTextureParameterIuivEXT __glewGetTextureParameterIuivEXT
#define glGetTextureParameterfvEXT __glewGetTextureParameterfvEXT
#define glGetTextureParameterivEXT __glewGetTextureParameterivEXT
#define glGetVertexArrayIntegeri_vEXT __glewGetVertexArrayIntegeri_vEXT
#define glGetVertexArrayIntegervEXT __glewGetVertexArrayIntegervEXT
#define glGetVertexArrayPointeri_vEXT __glewGetVertexArrayPointeri_vEXT
#define glGetVertexArrayPointervEXT __glewGetVertexArrayPointervEXT
#define glMapNamedBufferEXT __glewMapNamedBufferEXT
#define glMapNamedBufferRangeEXT __glewMapNamedBufferRangeEXT
#define glMatrixFrustumEXT __glewMatrixFrustumEXT
#define glMatrixLoadIdentityEXT __glewMatrixLoadIdentityEXT
#define glMatrixLoadTransposedEXT __glewMatrixLoadTransposedEXT
#define glMatrixLoadTransposefEXT __glewMatrixLoadTransposefEXT
#define glMatrixLoaddEXT __glewMatrixLoaddEXT
#define glMatrixLoadfEXT __glewMatrixLoadfEXT
#define glMatrixMultTransposedEXT __glewMatrixMultTransposedEXT
#define glMatrixMultTransposefEXT __glewMatrixMultTransposefEXT
#define glMatrixMultdEXT __glewMatrixMultdEXT
#define glMatrixMultfEXT __glewMatrixMultfEXT
#define glMatrixOrthoEXT __glewMatrixOrthoEXT
#define glMatrixPopEXT __glewMatrixPopEXT
#define glMatrixPushEXT __glewMatrixPushEXT
#define glMatrixRotatedEXT __glewMatrixRotatedEXT
#define glMatrixRotatefEXT __glewMatrixRotatefEXT
#define glMatrixScaledEXT __glewMatrixScaledEXT
#define glMatrixScalefEXT __glewMatrixScalefEXT
#define glMatrixTranslatedEXT __glewMatrixTranslatedEXT
#define glMatrixTranslatefEXT __glewMatrixTranslatefEXT
#define glMultiTexBufferEXT __glewMultiTexBufferEXT
#define glMultiTexCoordPointerEXT __glewMultiTexCoordPointerEXT
#define glMultiTexEnvfEXT __glewMultiTexEnvfEXT
#define glMultiTexEnvfvEXT __glewMultiTexEnvfvEXT
#define glMultiTexEnviEXT __glewMultiTexEnviEXT
#define glMultiTexEnvivEXT __glewMultiTexEnvivEXT
#define glMultiTexGendEXT __glewMultiTexGendEXT
#define glMultiTexGendvEXT __glewMultiTexGendvEXT
#define glMultiTexGenfEXT __glewMultiTexGenfEXT
#define glMultiTexGenfvEXT __glewMultiTexGenfvEXT
#define glMultiTexGeniEXT __glewMultiTexGeniEXT
#define glMultiTexGenivEXT __glewMultiTexGenivEXT
#define glMultiTexImage1DEXT __glewMultiTexImage1DEXT
#define glMultiTexImage2DEXT __glewMultiTexImage2DEXT
#define glMultiTexImage3DEXT __glewMultiTexImage3DEXT
#define glMultiTexParameterIivEXT __glewMultiTexParameterIivEXT
#define glMultiTexParameterIuivEXT __glewMultiTexParameterIuivEXT
#define glMultiTexParameterfEXT __glewMultiTexParameterfEXT
#define glMultiTexParameterfvEXT __glewMultiTexParameterfvEXT
#define glMultiTexParameteriEXT __glewMultiTexParameteriEXT
#define glMultiTexParameterivEXT __glewMultiTexParameterivEXT
#define glMultiTexRenderbufferEXT __glewMultiTexRenderbufferEXT
#define glMultiTexSubImage1DEXT __glewMultiTexSubImage1DEXT
#define glMultiTexSubImage2DEXT __glewMultiTexSubImage2DEXT
#define glMultiTexSubImage3DEXT __glewMultiTexSubImage3DEXT
#define glNamedBufferDataEXT __glewNamedBufferDataEXT
#define glNamedBufferSubDataEXT __glewNamedBufferSubDataEXT
#define glNamedCopyBufferSubDataEXT __glewNamedCopyBufferSubDataEXT
#define glNamedFramebufferRenderbufferEXT __glewNamedFramebufferRenderbufferEXT
#define glNamedFramebufferTexture1DEXT __glewNamedFramebufferTexture1DEXT
#define glNamedFramebufferTexture2DEXT __glewNamedFramebufferTexture2DEXT
#define glNamedFramebufferTexture3DEXT __glewNamedFramebufferTexture3DEXT
#define glNamedFramebufferTextureEXT __glewNamedFramebufferTextureEXT
#define glNamedFramebufferTextureFaceEXT __glewNamedFramebufferTextureFaceEXT
#define glNamedFramebufferTextureLayerEXT __glewNamedFramebufferTextureLayerEXT
#define glNamedProgramLocalParameter4dEXT __glewNamedProgramLocalParameter4dEXT
#define glNamedProgramLocalParameter4dvEXT __glewNamedProgramLocalParameter4dvEXT
#define glNamedProgramLocalParameter4fEXT __glewNamedProgramLocalParameter4fEXT
#define glNamedProgramLocalParameter4fvEXT __glewNamedProgramLocalParameter4fvEXT
#define glNamedProgramLocalParameterI4iEXT __glewNamedProgramLocalParameterI4iEXT
#define glNamedProgramLocalParameterI4ivEXT __glewNamedProgramLocalParameterI4ivEXT
#define glNamedProgramLocalParameterI4uiEXT __glewNamedProgramLocalParameterI4uiEXT
#define glNamedProgramLocalParameterI4uivEXT __glewNamedProgramLocalParameterI4uivEXT
#define glNamedProgramLocalParameters4fvEXT __glewNamedProgramLocalParameters4fvEXT
#define glNamedProgramLocalParametersI4ivEXT __glewNamedProgramLocalParametersI4ivEXT
#define glNamedProgramLocalParametersI4uivEXT __glewNamedProgramLocalParametersI4uivEXT
#define glNamedProgramStringEXT __glewNamedProgramStringEXT
#define glNamedRenderbufferStorageEXT __glewNamedRenderbufferStorageEXT
#define glNamedRenderbufferStorageMultisampleCoverageEXT __glewNamedRenderbufferStorageMultisampleCoverageEXT
#define glNamedRenderbufferStorageMultisampleEXT __glewNamedRenderbufferStorageMultisampleEXT
#define glProgramUniform1fEXT __glewProgramUniform1fEXT
#define glProgramUniform1fvEXT __glewProgramUniform1fvEXT
#define glProgramUniform1iEXT __glewProgramUniform1iEXT
#define glProgramUniform1ivEXT __glewProgramUniform1ivEXT
#define glProgramUniform1uiEXT __glewProgramUniform1uiEXT
#define glProgramUniform1uivEXT __glewProgramUniform1uivEXT
#define glProgramUniform2fEXT __glewProgramUniform2fEXT
#define glProgramUniform2fvEXT __glewProgramUniform2fvEXT
#define glProgramUniform2iEXT __glewProgramUniform2iEXT
#define glProgramUniform2ivEXT __glewProgramUniform2ivEXT
#define glProgramUniform2uiEXT __glewProgramUniform2uiEXT
#define glProgramUniform2uivEXT __glewProgramUniform2uivEXT
#define glProgramUniform3fEXT __glewProgramUniform3fEXT
#define glProgramUniform3fvEXT __glewProgramUniform3fvEXT
#define glProgramUniform3iEXT __glewProgramUniform3iEXT
#define glProgramUniform3ivEXT __glewProgramUniform3ivEXT
#define glProgramUniform3uiEXT __glewProgramUniform3uiEXT
#define glProgramUniform3uivEXT __glewProgramUniform3uivEXT
#define glProgramUniform4fEXT __glewProgramUniform4fEXT
#define glProgramUniform4fvEXT __glewProgramUniform4fvEXT
#define glProgramUniform4iEXT __glewProgramUniform4iEXT
#define glProgramUniform4ivEXT __glewProgramUniform4ivEXT
#define glProgramUniform4uiEXT __glewProgramUniform4uiEXT
#define glProgramUniform4uivEXT __glewProgramUniform4uivEXT
#define glProgramUniformMatrix2fvEXT __glewProgramUniformMatrix2fvEXT
#define glProgramUniformMatrix2x3fvEXT __glewProgramUniformMatrix2x3fvEXT
#define glProgramUniformMatrix2x4fvEXT __glewProgramUniformMatrix2x4fvEXT
#define glProgramUniformMatrix3fvEXT __glewProgramUniformMatrix3fvEXT
#define glProgramUniformMatrix3x2fvEXT __glewProgramUniformMatrix3x2fvEXT
#define glProgramUniformMatrix3x4fvEXT __glewProgramUniformMatrix3x4fvEXT
#define glProgramUniformMatrix4fvEXT __glewProgramUniformMatrix4fvEXT
#define glProgramUniformMatrix4x2fvEXT __glewProgramUniformMatrix4x2fvEXT
#define glProgramUniformMatrix4x3fvEXT __glewProgramUniformMatrix4x3fvEXT
#define glPushClientAttribDefaultEXT __glewPushClientAttribDefaultEXT
#define glTextureBufferEXT __glewTextureBufferEXT
#define glTextureImage1DEXT __glewTextureImage1DEXT
#define glTextureImage2DEXT __glewTextureImage2DEXT
#define glTextureImage3DEXT __glewTextureImage3DEXT
#define glTextureParameterIivEXT __glewTextureParameterIivEXT
#define glTextureParameterIuivEXT __glewTextureParameterIuivEXT
#define glTextureParameterfEXT __glewTextureParameterfEXT
#define glTextureParameterfvEXT __glewTextureParameterfvEXT
#define glTextureParameteriEXT __glewTextureParameteriEXT
#define glTextureParameterivEXT __glewTextureParameterivEXT
#define glTextureRenderbufferEXT __glewTextureRenderbufferEXT
#define glTextureSubImage1DEXT __glewTextureSubImage1DEXT
#define glTextureSubImage2DEXT __glewTextureSubImage2DEXT
#define glTextureSubImage3DEXT __glewTextureSubImage3DEXT
#define glUnmapNamedBufferEXT __glewUnmapNamedBufferEXT
#define glVertexArrayColorOffsetEXT __glewVertexArrayColorOffsetEXT
#define glVertexArrayEdgeFlagOffsetEXT __glewVertexArrayEdgeFlagOffsetEXT
#define glVertexArrayFogCoordOffsetEXT __glewVertexArrayFogCoordOffsetEXT
#define glVertexArrayIndexOffsetEXT __glewVertexArrayIndexOffsetEXT
#define glVertexArrayMultiTexCoordOffsetEXT __glewVertexArrayMultiTexCoordOffsetEXT
#define glVertexArrayNormalOffsetEXT __glewVertexArrayNormalOffsetEXT
#define glVertexArraySecondaryColorOffsetEXT __glewVertexArraySecondaryColorOffsetEXT
#define glVertexArrayTexCoordOffsetEXT __glewVertexArrayTexCoordOffsetEXT
#define glVertexArrayVertexAttribIOffsetEXT __glewVertexArrayVertexAttribIOffsetEXT
#define glVertexArrayVertexAttribOffsetEXT __glewVertexArrayVertexAttribOffsetEXT
#define glVertexArrayVertexOffsetEXT __glewVertexArrayVertexOffsetEXT
#define glColorMaskIndexedEXT __glewColorMaskIndexedEXT
#define glDisableIndexedEXT __glewDisableIndexedEXT
#define glEnableIndexedEXT __glewEnableIndexedEXT
#define glGetBooleanIndexedvEXT __glewGetBooleanIndexedvEXT
#define glGetIntegerIndexedvEXT __glewGetIntegerIndexedvEXT
#define glIsEnabledIndexedEXT __glewIsEnabledIndexedEXT
#define glDrawArraysInstancedEXT __glewDrawArraysInstancedEXT
#define glDrawElementsInstancedEXT __glewDrawElementsInstancedEXT
#define glDrawRangeElementsEXT __glewDrawRangeElementsEXT
#define glFogCoordPointerEXT __glewFogCoordPointerEXT
#define glFogCoorddEXT __glewFogCoorddEXT
#define glFogCoorddvEXT __glewFogCoorddvEXT
#define glFogCoordfEXT __glewFogCoordfEXT
#define glFogCoordfvEXT __glewFogCoordfvEXT
#define glFragmentColorMaterialEXT __glewFragmentColorMaterialEXT
#define glFragmentLightModelfEXT __glewFragmentLightModelfEXT
#define glFragmentLightModelfvEXT __glewFragmentLightModelfvEXT
#define glFragmentLightModeliEXT __glewFragmentLightModeliEXT
#define glFragmentLightModelivEXT __glewFragmentLightModelivEXT
#define glFragmentLightfEXT __glewFragmentLightfEXT
#define glFragmentLightfvEXT __glewFragmentLightfvEXT
#define glFragmentLightiEXT __glewFragmentLightiEXT
#define glFragmentLightivEXT __glewFragmentLightivEXT
#define glFragmentMaterialfEXT __glewFragmentMaterialfEXT
#define glFragmentMaterialfvEXT __glewFragmentMaterialfvEXT
#define glFragmentMaterialiEXT __glewFragmentMaterialiEXT
#define glFragmentMaterialivEXT __glewFragmentMaterialivEXT
#define glGetFragmentLightfvEXT __glewGetFragmentLightfvEXT
#define glGetFragmentLightivEXT __glewGetFragmentLightivEXT
#define glGetFragmentMaterialfvEXT __glewGetFragmentMaterialfvEXT
#define glGetFragmentMaterialivEXT __glewGetFragmentMaterialivEXT
#define glLightEnviEXT __glewLightEnviEXT
#define glBlitFramebufferEXT __glewBlitFramebufferEXT
#define glRenderbufferStorageMultisampleEXT __glewRenderbufferStorageMultisampleEXT
#define glBindFramebufferEXT __glewBindFramebufferEXT
#define glBindRenderbufferEXT __glewBindRenderbufferEXT
#define glCheckFramebufferStatusEXT __glewCheckFramebufferStatusEXT
#define glDeleteFramebuffersEXT __glewDeleteFramebuffersEXT
#define glDeleteRenderbuffersEXT __glewDeleteRenderbuffersEXT
#define glFramebufferRenderbufferEXT __glewFramebufferRenderbufferEXT
#define glFramebufferTexture1DEXT __glewFramebufferTexture1DEXT
#define glFramebufferTexture2DEXT __glewFramebufferTexture2DEXT
#define glFramebufferTexture3DEXT __glewFramebufferTexture3DEXT
#define glGenFramebuffersEXT __glewGenFramebuffersEXT
#define glGenRenderbuffersEXT __glewGenRenderbuffersEXT
#define glGenerateMipmapEXT __glewGenerateMipmapEXT
#define glGetFramebufferAttachmentParameterivEXT __glewGetFramebufferAttachmentParameterivEXT
#define glGetRenderbufferParameterivEXT __glewGetRenderbufferParameterivEXT
#define glIsFramebufferEXT __glewIsFramebufferEXT
#define glIsRenderbufferEXT __glewIsRenderbufferEXT
#define glRenderbufferStorageEXT __glewRenderbufferStorageEXT
#define glFramebufferTextureEXT __glewFramebufferTextureEXT
#define glFramebufferTextureFaceEXT __glewFramebufferTextureFaceEXT
#define glProgramParameteriEXT __glewProgramParameteriEXT
#define glProgramEnvParameters4fvEXT __glewProgramEnvParameters4fvEXT
#define glProgramLocalParameters4fvEXT __glewProgramLocalParameters4fvEXT
#define glBindFragDataLocationEXT __glewBindFragDataLocationEXT
#define glGetFragDataLocationEXT __glewGetFragDataLocationEXT
#define glGetUniformuivEXT __glewGetUniformuivEXT
#define glGetVertexAttribIivEXT __glewGetVertexAttribIivEXT
#define glGetVertexAttribIuivEXT __glewGetVertexAttribIuivEXT
#define glUniform1uiEXT __glewUniform1uiEXT
#define glUniform1uivEXT __glewUniform1uivEXT
#define glUniform2uiEXT __glewUniform2uiEXT
#define glUniform2uivEXT __glewUniform2uivEXT
#define glUniform3uiEXT __glewUniform3uiEXT
#define glUniform3uivEXT __glewUniform3uivEXT
#define glUniform4uiEXT __glewUniform4uiEXT
#define glUniform4uivEXT __glewUniform4uivEXT
#define glVertexAttribI1iEXT __glewVertexAttribI1iEXT
#define glVertexAttribI1ivEXT __glewVertexAttribI1ivEXT
#define glVertexAttribI1uiEXT __glewVertexAttribI1uiEXT
#define glVertexAttribI1uivEXT __glewVertexAttribI1uivEXT
#define glVertexAttribI2iEXT __glewVertexAttribI2iEXT
#define glVertexAttribI2ivEXT __glewVertexAttribI2ivEXT
#define glVertexAttribI2uiEXT __glewVertexAttribI2uiEXT
#define glVertexAttribI2uivEXT __glewVertexAttribI2uivEXT
#define glVertexAttribI3iEXT __glewVertexAttribI3iEXT
#define glVertexAttribI3ivEXT __glewVertexAttribI3ivEXT
#define glVertexAttribI3uiEXT __glewVertexAttribI3uiEXT
#define glVertexAttribI3uivEXT __glewVertexAttribI3uivEXT
#define glVertexAttribI4bvEXT __glewVertexAttribI4bvEXT
#define glVertexAttribI4iEXT __glewVertexAttribI4iEXT
#define glVertexAttribI4ivEXT __glewVertexAttribI4ivEXT
#define glVertexAttribI4svEXT __glewVertexAttribI4svEXT
#define glVertexAttribI4ubvEXT __glewVertexAttribI4ubvEXT
#define glVertexAttribI4uiEXT __glewVertexAttribI4uiEXT
#define glVertexAttribI4uivEXT __glewVertexAttribI4uivEXT
#define glVertexAttribI4usvEXT __glewVertexAttribI4usvEXT
#define glVertexAttribIPointerEXT __glewVertexAttribIPointerEXT
#define glGetHistogramEXT __glewGetHistogramEXT
#define glGetHistogramParameterfvEXT __glewGetHistogramParameterfvEXT
#define glGetHistogramParameterivEXT __glewGetHistogramParameterivEXT
#define glGetMinmaxEXT __glewGetMinmaxEXT
#define glGetMinmaxParameterfvEXT __glewGetMinmaxParameterfvEXT
#define glGetMinmaxParameterivEXT __glewGetMinmaxParameterivEXT
#define glHistogramEXT __glewHistogramEXT
#define glMinmaxEXT __glewMinmaxEXT
#define glResetHistogramEXT __glewResetHistogramEXT
#define glResetMinmaxEXT __glewResetMinmaxEXT
#define glIndexFuncEXT __glewIndexFuncEXT
#define glIndexMaterialEXT __glewIndexMaterialEXT
#define glApplyTextureEXT __glewApplyTextureEXT
#define glTextureLightEXT __glewTextureLightEXT
#define glTextureMaterialEXT __glewTextureMaterialEXT
#define glMultiDrawArraysEXT __glewMultiDrawArraysEXT
#define glMultiDrawElementsEXT __glewMultiDrawElementsEXT
#define glSampleMaskEXT __glewSampleMaskEXT
#define glSamplePatternEXT __glewSamplePatternEXT
#define glColorTableEXT __glewColorTableEXT
#define glGetColorTableEXT __glewGetColorTableEXT
#define glGetColorTableParameterfvEXT __glewGetColorTableParameterfvEXT
#define glGetColorTableParameterivEXT __glewGetColorTableParameterivEXT
#define glGetPixelTransformParameterfvEXT __glewGetPixelTransformParameterfvEXT
#define glGetPixelTransformParameterivEXT __glewGetPixelTransformParameterivEXT
#define glPixelTransformParameterfEXT __glewPixelTransformParameterfEXT
#define glPixelTransformParameterfvEXT __glewPixelTransformParameterfvEXT
#define glPixelTransformParameteriEXT __glewPixelTransformParameteriEXT
#define glPixelTransformParameterivEXT __glewPixelTransformParameterivEXT
#define glPointParameterfEXT __glewPointParameterfEXT
#define glPointParameterfvEXT __glewPointParameterfvEXT
#define glPolygonOffsetEXT __glewPolygonOffsetEXT
#define glProvokingVertexEXT __glewProvokingVertexEXT
#define glBeginSceneEXT __glewBeginSceneEXT
#define glEndSceneEXT __glewEndSceneEXT
#define glSecondaryColor3bEXT __glewSecondaryColor3bEXT
#define glSecondaryColor3bvEXT __glewSecondaryColor3bvEXT
#define glSecondaryColor3dEXT __glewSecondaryColor3dEXT
#define glSecondaryColor3dvEXT __glewSecondaryColor3dvEXT
#define glSecondaryColor3fEXT __glewSecondaryColor3fEXT
#define glSecondaryColor3fvEXT __glewSecondaryColor3fvEXT
#define glSecondaryColor3iEXT __glewSecondaryColor3iEXT
#define glSecondaryColor3ivEXT __glewSecondaryColor3ivEXT
#define glSecondaryColor3sEXT __glewSecondaryColor3sEXT
#define glSecondaryColor3svEXT __glewSecondaryColor3svEXT
#define glSecondaryColor3ubEXT __glewSecondaryColor3ubEXT
#define glSecondaryColor3ubvEXT __glewSecondaryColor3ubvEXT
#define glSecondaryColor3uiEXT __glewSecondaryColor3uiEXT
#define glSecondaryColor3uivEXT __glewSecondaryColor3uivEXT
#define glSecondaryColor3usEXT __glewSecondaryColor3usEXT
#define glSecondaryColor3usvEXT __glewSecondaryColor3usvEXT
#define glSecondaryColorPointerEXT __glewSecondaryColorPointerEXT
#define glActiveProgramEXT __glewActiveProgramEXT
#define glCreateShaderProgramEXT __glewCreateShaderProgramEXT
#define glUseShaderProgramEXT __glewUseShaderProgramEXT
#define glBindImageTextureEXT __glewBindImageTextureEXT
#define glMemoryBarrierEXT __glewMemoryBarrierEXT
#define glActiveStencilFaceEXT __glewActiveStencilFaceEXT
#define glTexSubImage1DEXT __glewTexSubImage1DEXT
#define glTexSubImage2DEXT __glewTexSubImage2DEXT
#define glTexSubImage3DEXT __glewTexSubImage3DEXT
#define glTexImage3DEXT __glewTexImage3DEXT
#define glFramebufferTextureLayerEXT __glewFramebufferTextureLayerEXT
#define glTexBufferEXT __glewTexBufferEXT
#define glClearColorIiEXT __glewClearColorIiEXT
#define glClearColorIuiEXT __glewClearColorIuiEXT
#define glGetTexParameterIivEXT __glewGetTexParameterIivEXT
#define glGetTexParameterIuivEXT __glewGetTexParameterIuivEXT
#define glTexParameterIivEXT __glewTexParameterIivEXT
#define glTexParameterIuivEXT __glewTexParameterIuivEXT
#define glAreTexturesResidentEXT __glewAreTexturesResidentEXT
#define glBindTextureEXT __glewBindTextureEXT
#define glDeleteTexturesEXT __glewDeleteTexturesEXT
#define glGenTexturesEXT __glewGenTexturesEXT
#define glIsTextureEXT __glewIsTextureEXT
#define glPrioritizeTexturesEXT __glewPrioritizeTexturesEXT
#define glTextureNormalEXT __glewTextureNormalEXT
#define glGetQueryObjecti64vEXT __glewGetQueryObjecti64vEXT
#define glGetQueryObjectui64vEXT __glewGetQueryObjectui64vEXT
#define glBeginTransformFeedbackEXT __glewBeginTransformFeedbackEXT
#define glBindBufferBaseEXT __glewBindBufferBaseEXT
#define glBindBufferOffsetEXT __glewBindBufferOffsetEXT
#define glBindBufferRangeEXT __glewBindBufferRangeEXT
#define glEndTransformFeedbackEXT __glewEndTransformFeedbackEXT
#define glGetTransformFeedbackVaryingEXT __glewGetTransformFeedbackVaryingEXT
#define glTransformFeedbackVaryingsEXT __glewTransformFeedbackVaryingsEXT
#define glArrayElementEXT __glewArrayElementEXT
#define glColorPointerEXT __glewColorPointerEXT
#define glDrawArraysEXT __glewDrawArraysEXT
#define glEdgeFlagPointerEXT __glewEdgeFlagPointerEXT
#define glIndexPointerEXT __glewIndexPointerEXT
#define glNormalPointerEXT __glewNormalPointerEXT
#define glTexCoordPointerEXT __glewTexCoordPointerEXT
#define glVertexPointerEXT __glewVertexPointerEXT
#define glGetVertexAttribLdvEXT __glewGetVertexAttribLdvEXT
#define glVertexArrayVertexAttribLOffsetEXT __glewVertexArrayVertexAttribLOffsetEXT
#define glVertexAttribL1dEXT __glewVertexAttribL1dEXT
#define glVertexAttribL1dvEXT __glewVertexAttribL1dvEXT
#define glVertexAttribL2dEXT __glewVertexAttribL2dEXT
#define glVertexAttribL2dvEXT __glewVertexAttribL2dvEXT
#define glVertexAttribL3dEXT __glewVertexAttribL3dEXT
#define glVertexAttribL3dvEXT __glewVertexAttribL3dvEXT
#define glVertexAttribL4dEXT __glewVertexAttribL4dEXT
#define glVertexAttribL4dvEXT __glewVertexAttribL4dvEXT
#define glVertexAttribLPointerEXT __glewVertexAttribLPointerEXT
#define glBeginVertexShaderEXT __glewBeginVertexShaderEXT
#define glBindLightParameterEXT __glewBindLightParameterEXT
#define glBindMaterialParameterEXT __glewBindMaterialParameterEXT
#define glBindParameterEXT __glewBindParameterEXT
#define glBindTexGenParameterEXT __glewBindTexGenParameterEXT
#define glBindTextureUnitParameterEXT __glewBindTextureUnitParameterEXT
#define glBindVertexShaderEXT __glewBindVertexShaderEXT
#define glDeleteVertexShaderEXT __glewDeleteVertexShaderEXT
#define glDisableVariantClientStateEXT __glewDisableVariantClientStateEXT
#define glEnableVariantClientStateEXT __glewEnableVariantClientStateEXT
#define glEndVertexShaderEXT __glewEndVertexShaderEXT
#define glExtractComponentEXT __glewExtractComponentEXT
#define glGenSymbolsEXT __glewGenSymbolsEXT
#define glGenVertexShadersEXT __glewGenVertexShadersEXT
#define glGetInvariantBooleanvEXT __glewGetInvariantBooleanvEXT
#define glGetInvariantFloatvEXT __glewGetInvariantFloatvEXT
#define glGetInvariantIntegervEXT __glewGetInvariantIntegervEXT
#define glGetLocalConstantBooleanvEXT __glewGetLocalConstantBooleanvEXT
#define glGetLocalConstantFloatvEXT __glewGetLocalConstantFloatvEXT
#define glGetLocalConstantIntegervEXT __glewGetLocalConstantIntegervEXT
#define glGetVariantBooleanvEXT __glewGetVariantBooleanvEXT
#define glGetVariantFloatvEXT __glewGetVariantFloatvEXT
#define glGetVariantIntegervEXT __glewGetVariantIntegervEXT
#define glGetVariantPointervEXT __glewGetVariantPointervEXT
#define glInsertComponentEXT __glewInsertComponentEXT
#define glIsVariantEnabledEXT __glewIsVariantEnabledEXT
#define glSetInvariantEXT __glewSetInvariantEXT
#define glSetLocalConstantEXT __glewSetLocalConstantEXT
#define glShaderOp1EXT __glewShaderOp1EXT
#define glShaderOp2EXT __glewShaderOp2EXT
#define glShaderOp3EXT __glewShaderOp3EXT
#define glSwizzleEXT __glewSwizzleEXT
#define glVariantPointerEXT __glewVariantPointerEXT
#define glVariantbvEXT __glewVariantbvEXT
#define glVariantdvEXT __glewVariantdvEXT
#define glVariantfvEXT __glewVariantfvEXT
#define glVariantivEXT __glewVariantivEXT
#define glVariantsvEXT __glewVariantsvEXT
#define glVariantubvEXT __glewVariantubvEXT
#define glVariantuivEXT __glewVariantuivEXT
#define glVariantusvEXT __glewVariantusvEXT
#define glWriteMaskEXT __glewWriteMaskEXT
#define glVertexWeightPointerEXT __glewVertexWeightPointerEXT
#define glVertexWeightfEXT __glewVertexWeightfEXT
#define glVertexWeightfvEXT __glewVertexWeightfvEXT
#define glImportSyncEXT __glewImportSyncEXT
#define glFrameTerminatorGREMEDY __glewFrameTerminatorGREMEDY
#define glStringMarkerGREMEDY __glewStringMarkerGREMEDY
#define glGetImageTransformParameterfvHP __glewGetImageTransformParameterfvHP
#define glGetImageTransformParameterivHP __glewGetImageTransformParameterivHP
#define glImageTransformParameterfHP __glewImageTransformParameterfHP
#define glImageTransformParameterfvHP __glewImageTransformParameterfvHP
#define glImageTransformParameteriHP __glewImageTransformParameteriHP
#define glImageTransformParameterivHP __glewImageTransformParameterivHP
#define glMultiModeDrawArraysIBM __glewMultiModeDrawArraysIBM
#define glMultiModeDrawElementsIBM __glewMultiModeDrawElementsIBM
#define glColorPointerListIBM __glewColorPointerListIBM
#define glEdgeFlagPointerListIBM __glewEdgeFlagPointerListIBM
#define glFogCoordPointerListIBM __glewFogCoordPointerListIBM
#define glIndexPointerListIBM __glewIndexPointerListIBM
#define glNormalPointerListIBM __glewNormalPointerListIBM
#define glSecondaryColorPointerListIBM __glewSecondaryColorPointerListIBM
#define glTexCoordPointerListIBM __glewTexCoordPointerListIBM
#define glVertexPointerListIBM __glewVertexPointerListIBM
#define glColorPointervINTEL __glewColorPointervINTEL
#define glNormalPointervINTEL __glewNormalPointervINTEL
#define glTexCoordPointervINTEL __glewTexCoordPointervINTEL
#define glVertexPointervINTEL __glewVertexPointervINTEL
#define glTexScissorFuncINTEL __glewTexScissorFuncINTEL
#define glTexScissorINTEL __glewTexScissorINTEL
#define glBufferRegionEnabledEXT __glewBufferRegionEnabledEXT
#define glDeleteBufferRegionEXT __glewDeleteBufferRegionEXT
#define glDrawBufferRegionEXT __glewDrawBufferRegionEXT
#define glNewBufferRegionEXT __glewNewBufferRegionEXT
#define glReadBufferRegionEXT __glewReadBufferRegionEXT
#define glResizeBuffersMESA __glewResizeBuffersMESA
#define glWindowPos2dMESA __glewWindowPos2dMESA
#define glWindowPos2dvMESA __glewWindowPos2dvMESA
#define glWindowPos2fMESA __glewWindowPos2fMESA
#define glWindowPos2fvMESA __glewWindowPos2fvMESA
#define glWindowPos2iMESA __glewWindowPos2iMESA
#define glWindowPos2ivMESA __glewWindowPos2ivMESA
#define glWindowPos2sMESA __glewWindowPos2sMESA
#define glWindowPos2svMESA __glewWindowPos2svMESA
#define glWindowPos3dMESA __glewWindowPos3dMESA
#define glWindowPos3dvMESA __glewWindowPos3dvMESA
#define glWindowPos3fMESA __glewWindowPos3fMESA
#define glWindowPos3fvMESA __glewWindowPos3fvMESA
#define glWindowPos3iMESA __glewWindowPos3iMESA
#define glWindowPos3ivMESA __glewWindowPos3ivMESA
#define glWindowPos3sMESA __glewWindowPos3sMESA
#define glWindowPos3svMESA __glewWindowPos3svMESA
#define glWindowPos4dMESA __glewWindowPos4dMESA
#define glWindowPos4dvMESA __glewWindowPos4dvMESA
#define glWindowPos4fMESA __glewWindowPos4fMESA
#define glWindowPos4fvMESA __glewWindowPos4fvMESA
#define glWindowPos4iMESA __glewWindowPos4iMESA
#define glWindowPos4ivMESA __glewWindowPos4ivMESA
#define glWindowPos4sMESA __glewWindowPos4sMESA
#define glWindowPos4svMESA __glewWindowPos4svMESA
#define glBeginConditionalRenderNV __glewBeginConditionalRenderNV
#define glEndConditionalRenderNV __glewEndConditionalRenderNV
#define glCopyImageSubDataNV __glewCopyImageSubDataNV
#define glClearDepthdNV __glewClearDepthdNV
#define glDepthBoundsdNV __glewDepthBoundsdNV
#define glDepthRangedNV __glewDepthRangedNV
#define glEvalMapsNV __glewEvalMapsNV
#define glGetMapAttribParameterfvNV __glewGetMapAttribParameterfvNV
#define glGetMapAttribParameterivNV __glewGetMapAttribParameterivNV
#define glGetMapControlPointsNV __glewGetMapControlPointsNV
#define glGetMapParameterfvNV __glewGetMapParameterfvNV
#define glGetMapParameterivNV __glewGetMapParameterivNV
#define glMapControlPointsNV __glewMapControlPointsNV
#define glMapParameterfvNV __glewMapParameterfvNV
#define glMapParameterivNV __glewMapParameterivNV
#define glGetMultisamplefvNV __glewGetMultisamplefvNV
#define glSampleMaskIndexedNV __glewSampleMaskIndexedNV
#define glTexRenderbufferNV __glewTexRenderbufferNV
#define glDeleteFencesNV __glewDeleteFencesNV
#define glFinishFenceNV __glewFinishFenceNV
#define glGenFencesNV __glewGenFencesNV
#define glGetFenceivNV __glewGetFenceivNV
#define glIsFenceNV __glewIsFenceNV
#define glSetFenceNV __glewSetFenceNV
#define glTestFenceNV __glewTestFenceNV
#define glGetProgramNamedParameterdvNV __glewGetProgramNamedParameterdvNV
#define glGetProgramNamedParameterfvNV __glewGetProgramNamedParameterfvNV
#define glProgramNamedParameter4dNV __glewProgramNamedParameter4dNV
#define glProgramNamedParameter4dvNV __glewProgramNamedParameter4dvNV
#define glProgramNamedParameter4fNV __glewProgramNamedParameter4fNV
#define glProgramNamedParameter4fvNV __glewProgramNamedParameter4fvNV
#define glRenderbufferStorageMultisampleCoverageNV __glewRenderbufferStorageMultisampleCoverageNV
#define glProgramVertexLimitNV __glewProgramVertexLimitNV
#define glProgramEnvParameterI4iNV __glewProgramEnvParameterI4iNV
#define glProgramEnvParameterI4ivNV __glewProgramEnvParameterI4ivNV
#define glProgramEnvParameterI4uiNV __glewProgramEnvParameterI4uiNV
#define glProgramEnvParameterI4uivNV __glewProgramEnvParameterI4uivNV
#define glProgramEnvParametersI4ivNV __glewProgramEnvParametersI4ivNV
#define glProgramEnvParametersI4uivNV __glewProgramEnvParametersI4uivNV
#define glProgramLocalParameterI4iNV __glewProgramLocalParameterI4iNV
#define glProgramLocalParameterI4ivNV __glewProgramLocalParameterI4ivNV
#define glProgramLocalParameterI4uiNV __glewProgramLocalParameterI4uiNV
#define glProgramLocalParameterI4uivNV __glewProgramLocalParameterI4uivNV
#define glProgramLocalParametersI4ivNV __glewProgramLocalParametersI4ivNV
#define glProgramLocalParametersI4uivNV __glewProgramLocalParametersI4uivNV
#define glGetUniformi64vNV __glewGetUniformi64vNV
#define glGetUniformui64vNV __glewGetUniformui64vNV
#define glProgramUniform1i64NV __glewProgramUniform1i64NV
#define glProgramUniform1i64vNV __glewProgramUniform1i64vNV
#define glProgramUniform1ui64NV __glewProgramUniform1ui64NV
#define glProgramUniform1ui64vNV __glewProgramUniform1ui64vNV
#define glProgramUniform2i64NV __glewProgramUniform2i64NV
#define glProgramUniform2i64vNV __glewProgramUniform2i64vNV
#define glProgramUniform2ui64NV __glewProgramUniform2ui64NV
#define glProgramUniform2ui64vNV __glewProgramUniform2ui64vNV
#define glProgramUniform3i64NV __glewProgramUniform3i64NV
#define glProgramUniform3i64vNV __glewProgramUniform3i64vNV
#define glProgramUniform3ui64NV __glewProgramUniform3ui64NV
#define glProgramUniform3ui64vNV __glewProgramUniform3ui64vNV
#define glProgramUniform4i64NV __glewProgramUniform4i64NV
#define glProgramUniform4i64vNV __glewProgramUniform4i64vNV
#define glProgramUniform4ui64NV __glewProgramUniform4ui64NV
#define glProgramUniform4ui64vNV __glewProgramUniform4ui64vNV
#define glUniform1i64NV __glewUniform1i64NV
#define glUniform1i64vNV __glewUniform1i64vNV
#define glUniform1ui64NV __glewUniform1ui64NV
#define glUniform1ui64vNV __glewUniform1ui64vNV
#define glUniform2i64NV __glewUniform2i64NV
#define glUniform2i64vNV __glewUniform2i64vNV
#define glUniform2ui64NV __glewUniform2ui64NV
#define glUniform2ui64vNV __glewUniform2ui64vNV
#define glUniform3i64NV __glewUniform3i64NV
#define glUniform3i64vNV __glewUniform3i64vNV
#define glUniform3ui64NV __glewUniform3ui64NV
#define glUniform3ui64vNV __glewUniform3ui64vNV
#define glUniform4i64NV __glewUniform4i64NV
#define glUniform4i64vNV __glewUniform4i64vNV
#define glUniform4ui64NV __glewUniform4ui64NV
#define glUniform4ui64vNV __glewUniform4ui64vNV
#define glColor3hNV __glewColor3hNV
#define glColor3hvNV __glewColor3hvNV
#define glColor4hNV __glewColor4hNV
#define glColor4hvNV __glewColor4hvNV
#define glFogCoordhNV __glewFogCoordhNV
#define glFogCoordhvNV __glewFogCoordhvNV
#define glMultiTexCoord1hNV __glewMultiTexCoord1hNV
#define glMultiTexCoord1hvNV __glewMultiTexCoord1hvNV
#define glMultiTexCoord2hNV __glewMultiTexCoord2hNV
#define glMultiTexCoord2hvNV __glewMultiTexCoord2hvNV
#define glMultiTexCoord3hNV __glewMultiTexCoord3hNV
#define glMultiTexCoord3hvNV __glewMultiTexCoord3hvNV
#define glMultiTexCoord4hNV __glewMultiTexCoord4hNV
#define glMultiTexCoord4hvNV __glewMultiTexCoord4hvNV
#define glNormal3hNV __glewNormal3hNV
#define glNormal3hvNV __glewNormal3hvNV
#define glSecondaryColor3hNV __glewSecondaryColor3hNV
#define glSecondaryColor3hvNV __glewSecondaryColor3hvNV
#define glTexCoord1hNV __glewTexCoord1hNV
#define glTexCoord1hvNV __glewTexCoord1hvNV
#define glTexCoord2hNV __glewTexCoord2hNV
#define glTexCoord2hvNV __glewTexCoord2hvNV
#define glTexCoord3hNV __glewTexCoord3hNV
#define glTexCoord3hvNV __glewTexCoord3hvNV
#define glTexCoord4hNV __glewTexCoord4hNV
#define glTexCoord4hvNV __glewTexCoord4hvNV
#define glVertex2hNV __glewVertex2hNV
#define glVertex2hvNV __glewVertex2hvNV
#define glVertex3hNV __glewVertex3hNV
#define glVertex3hvNV __glewVertex3hvNV
#define glVertex4hNV __glewVertex4hNV
#define glVertex4hvNV __glewVertex4hvNV
#define glVertexAttrib1hNV __glewVertexAttrib1hNV
#define glVertexAttrib1hvNV __glewVertexAttrib1hvNV
#define glVertexAttrib2hNV __glewVertexAttrib2hNV
#define glVertexAttrib2hvNV __glewVertexAttrib2hvNV
#define glVertexAttrib3hNV __glewVertexAttrib3hNV
#define glVertexAttrib3hvNV __glewVertexAttrib3hvNV
#define glVertexAttrib4hNV __glewVertexAttrib4hNV
#define glVertexAttrib4hvNV __glewVertexAttrib4hvNV
#define glVertexAttribs1hvNV __glewVertexAttribs1hvNV
#define glVertexAttribs2hvNV __glewVertexAttribs2hvNV
#define glVertexAttribs3hvNV __glewVertexAttribs3hvNV
#define glVertexAttribs4hvNV __glewVertexAttribs4hvNV
#define glVertexWeighthNV __glewVertexWeighthNV
#define glVertexWeighthvNV __glewVertexWeighthvNV
#define glBeginOcclusionQueryNV __glewBeginOcclusionQueryNV
#define glDeleteOcclusionQueriesNV __glewDeleteOcclusionQueriesNV
#define glEndOcclusionQueryNV __glewEndOcclusionQueryNV
#define glGenOcclusionQueriesNV __glewGenOcclusionQueriesNV
#define glGetOcclusionQueryivNV __glewGetOcclusionQueryivNV
#define glGetOcclusionQueryuivNV __glewGetOcclusionQueryuivNV
#define glIsOcclusionQueryNV __glewIsOcclusionQueryNV
#define glProgramBufferParametersIivNV __glewProgramBufferParametersIivNV
#define glProgramBufferParametersIuivNV __glewProgramBufferParametersIuivNV
#define glProgramBufferParametersfvNV __glewProgramBufferParametersfvNV
#define glFlushPixelDataRangeNV __glewFlushPixelDataRangeNV
#define glPixelDataRangeNV __glewPixelDataRangeNV
#define glPointParameteriNV __glewPointParameteriNV
#define glPointParameterivNV __glewPointParameterivNV
#define glGetVideoi64vNV __glewGetVideoi64vNV
#define glGetVideoivNV __glewGetVideoivNV
#define glGetVideoui64vNV __glewGetVideoui64vNV
#define glGetVideouivNV __glewGetVideouivNV
#define glPresentFrameDualFillNV __glewPresentFrameDualFillNV
#define glPresentFrameKeyedNV __glewPresentFrameKeyedNV
#define glPrimitiveRestartIndexNV __glewPrimitiveRestartIndexNV
#define glPrimitiveRestartNV __glewPrimitiveRestartNV
#define glCombinerInputNV __glewCombinerInputNV
#define glCombinerOutputNV __glewCombinerOutputNV
#define glCombinerParameterfNV __glewCombinerParameterfNV
#define glCombinerParameterfvNV __glewCombinerParameterfvNV
#define glCombinerParameteriNV __glewCombinerParameteriNV
#define glCombinerParameterivNV __glewCombinerParameterivNV
#define glFinalCombinerInputNV __glewFinalCombinerInputNV
#define glGetCombinerInputParameterfvNV __glewGetCombinerInputParameterfvNV
#define glGetCombinerInputParameterivNV __glewGetCombinerInputParameterivNV
#define glGetCombinerOutputParameterfvNV __glewGetCombinerOutputParameterfvNV
#define glGetCombinerOutputParameterivNV __glewGetCombinerOutputParameterivNV
#define glGetFinalCombinerInputParameterfvNV __glewGetFinalCombinerInputParameterfvNV
#define glGetFinalCombinerInputParameterivNV __glewGetFinalCombinerInputParameterivNV
#define glCombinerStageParameterfvNV __glewCombinerStageParameterfvNV
#define glGetCombinerStageParameterfvNV __glewGetCombinerStageParameterfvNV
#define glGetBufferParameterui64vNV __glewGetBufferParameterui64vNV
#define glGetIntegerui64vNV __glewGetIntegerui64vNV
#define glGetNamedBufferParameterui64vNV __glewGetNamedBufferParameterui64vNV
#define glIsBufferResidentNV __glewIsBufferResidentNV
#define glIsNamedBufferResidentNV __glewIsNamedBufferResidentNV
#define glMakeBufferNonResidentNV __glewMakeBufferNonResidentNV
#define glMakeBufferResidentNV __glewMakeBufferResidentNV
#define glMakeNamedBufferNonResidentNV __glewMakeNamedBufferNonResidentNV
#define glMakeNamedBufferResidentNV __glewMakeNamedBufferResidentNV
#define glProgramUniformui64NV __glewProgramUniformui64NV
#define glProgramUniformui64vNV __glewProgramUniformui64vNV
#define glUniformui64NV __glewUniformui64NV
#define glUniformui64vNV __glewUniformui64vNV
#define glTextureBarrierNV __glewTextureBarrierNV
#define glTexImage2DMultisampleCoverageNV __glewTexImage2DMultisampleCoverageNV
#define glTexImage3DMultisampleCoverageNV __glewTexImage3DMultisampleCoverageNV
#define glTextureImage2DMultisampleCoverageNV __glewTextureImage2DMultisampleCoverageNV
#define glTextureImage2DMultisampleNV __glewTextureImage2DMultisampleNV
#define glTextureImage3DMultisampleCoverageNV __glewTextureImage3DMultisampleCoverageNV
#define glTextureImage3DMultisampleNV __glewTextureImage3DMultisampleNV
#define glActiveVaryingNV __glewActiveVaryingNV
#define glBeginTransformFeedbackNV __glewBeginTransformFeedbackNV
#define glBindBufferBaseNV __glewBindBufferBaseNV
#define glBindBufferOffsetNV __glewBindBufferOffsetNV
#define glBindBufferRangeNV __glewBindBufferRangeNV
#define glEndTransformFeedbackNV __glewEndTransformFeedbackNV
#define glGetActiveVaryingNV __glewGetActiveVaryingNV
#define glGetTransformFeedbackVaryingNV __glewGetTransformFeedbackVaryingNV
#define glGetVaryingLocationNV __glewGetVaryingLocationNV
#define glTransformFeedbackAttribsNV __glewTransformFeedbackAttribsNV
#define glTransformFeedbackVaryingsNV __glewTransformFeedbackVaryingsNV
#define glBindTransformFeedbackNV __glewBindTransformFeedbackNV
#define glDeleteTransformFeedbacksNV __glewDeleteTransformFeedbacksNV
#define glDrawTransformFeedbackNV __glewDrawTransformFeedbackNV
#define glGenTransformFeedbacksNV __glewGenTransformFeedbacksNV
#define glIsTransformFeedbackNV __glewIsTransformFeedbackNV
#define glPauseTransformFeedbackNV __glewPauseTransformFeedbackNV
#define glResumeTransformFeedbackNV __glewResumeTransformFeedbackNV
#define glVDPAUFiniNV __glewVDPAUFiniNV
#define glVDPAUGetSurfaceivNV __glewVDPAUGetSurfaceivNV
#define glVDPAUInitNV __glewVDPAUInitNV
#define glVDPAUIsSurfaceNV __glewVDPAUIsSurfaceNV
#define glVDPAUMapSurfacesNV __glewVDPAUMapSurfacesNV
#define glVDPAURegisterOutputSurfaceNV __glewVDPAURegisterOutputSurfaceNV
#define glVDPAURegisterVideoSurfaceNV __glewVDPAURegisterVideoSurfaceNV
#define glVDPAUSurfaceAccessNV __glewVDPAUSurfaceAccessNV
#define glVDPAUUnmapSurfacesNV __glewVDPAUUnmapSurfacesNV
#define glVDPAUUnregisterSurfaceNV __glewVDPAUUnregisterSurfaceNV
#define glFlushVertexArrayRangeNV __glewFlushVertexArrayRangeNV
#define glVertexArrayRangeNV __glewVertexArrayRangeNV
#define glGetVertexAttribLi64vNV __glewGetVertexAttribLi64vNV
#define glGetVertexAttribLui64vNV __glewGetVertexAttribLui64vNV
#define glVertexAttribL1i64NV __glewVertexAttribL1i64NV
#define glVertexAttribL1i64vNV __glewVertexAttribL1i64vNV
#define glVertexAttribL1ui64NV __glewVertexAttribL1ui64NV
#define glVertexAttribL1ui64vNV __glewVertexAttribL1ui64vNV
#define glVertexAttribL2i64NV __glewVertexAttribL2i64NV
#define glVertexAttribL2i64vNV __glewVertexAttribL2i64vNV
#define glVertexAttribL2ui64NV __glewVertexAttribL2ui64NV
#define glVertexAttribL2ui64vNV __glewVertexAttribL2ui64vNV
#define glVertexAttribL3i64NV __glewVertexAttribL3i64NV
#define glVertexAttribL3i64vNV __glewVertexAttribL3i64vNV
#define glVertexAttribL3ui64NV __glewVertexAttribL3ui64NV
#define glVertexAttribL3ui64vNV __glewVertexAttribL3ui64vNV
#define glVertexAttribL4i64NV __glewVertexAttribL4i64NV
#define glVertexAttribL4i64vNV __glewVertexAttribL4i64vNV
#define glVertexAttribL4ui64NV __glewVertexAttribL4ui64NV
#define glVertexAttribL4ui64vNV __glewVertexAttribL4ui64vNV
#define glVertexAttribLFormatNV __glewVertexAttribLFormatNV
#define glBufferAddressRangeNV __glewBufferAddressRangeNV
#define glColorFormatNV __glewColorFormatNV
#define glEdgeFlagFormatNV __glewEdgeFlagFormatNV
#define glFogCoordFormatNV __glewFogCoordFormatNV
#define glGetIntegerui64i_vNV __glewGetIntegerui64i_vNV
#define glIndexFormatNV __glewIndexFormatNV
#define glNormalFormatNV __glewNormalFormatNV
#define glSecondaryColorFormatNV __glewSecondaryColorFormatNV
#define glTexCoordFormatNV __glewTexCoordFormatNV
#define glVertexAttribFormatNV __glewVertexAttribFormatNV
#define glVertexAttribIFormatNV __glewVertexAttribIFormatNV
#define glVertexFormatNV __glewVertexFormatNV
#define glAreProgramsResidentNV __glewAreProgramsResidentNV
#define glBindProgramNV __glewBindProgramNV
#define glDeleteProgramsNV __glewDeleteProgramsNV
#define glExecuteProgramNV __glewExecuteProgramNV
#define glGenProgramsNV __glewGenProgramsNV
#define glGetProgramParameterdvNV __glewGetProgramParameterdvNV
#define glGetProgramParameterfvNV __glewGetProgramParameterfvNV
#define glGetProgramStringNV __glewGetProgramStringNV
#define glGetProgramivNV __glewGetProgramivNV
#define glGetTrackMatrixivNV __glewGetTrackMatrixivNV
#define glGetVertexAttribPointervNV __glewGetVertexAttribPointervNV
#define glGetVertexAttribdvNV __glewGetVertexAttribdvNV
#define glGetVertexAttribfvNV __glewGetVertexAttribfvNV
#define glGetVertexAttribivNV __glewGetVertexAttribivNV
#define glIsProgramNV __glewIsProgramNV
#define glLoadProgramNV __glewLoadProgramNV
#define glProgramParameter4dNV __glewProgramParameter4dNV
#define glProgramParameter4dvNV __glewProgramParameter4dvNV
#define glProgramParameter4fNV __glewProgramParameter4fNV
#define glProgramParameter4fvNV __glewProgramParameter4fvNV
#define glProgramParameters4dvNV __glewProgramParameters4dvNV
#define glProgramParameters4fvNV __glewProgramParameters4fvNV
#define glRequestResidentProgramsNV __glewRequestResidentProgramsNV
#define glTrackMatrixNV __glewTrackMatrixNV
#define glVertexAttrib1dNV __glewVertexAttrib1dNV
#define glVertexAttrib1dvNV __glewVertexAttrib1dvNV
#define glVertexAttrib1fNV __glewVertexAttrib1fNV
#define glVertexAttrib1fvNV __glewVertexAttrib1fvNV
#define glVertexAttrib1sNV __glewVertexAttrib1sNV
#define glVertexAttrib1svNV __glewVertexAttrib1svNV
#define glVertexAttrib2dNV __glewVertexAttrib2dNV
#define glVertexAttrib2dvNV __glewVertexAttrib2dvNV
#define glVertexAttrib2fNV __glewVertexAttrib2fNV
#define glVertexAttrib2fvNV __glewVertexAttrib2fvNV
#define glVertexAttrib2sNV __glewVertexAttrib2sNV
#define glVertexAttrib2svNV __glewVertexAttrib2svNV
#define glVertexAttrib3dNV __glewVertexAttrib3dNV
#define glVertexAttrib3dvNV __glewVertexAttrib3dvNV
#define glVertexAttrib3fNV __glewVertexAttrib3fNV
#define glVertexAttrib3fvNV __glewVertexAttrib3fvNV
#define glVertexAttrib3sNV __glewVertexAttrib3sNV
#define glVertexAttrib3svNV __glewVertexAttrib3svNV
#define glVertexAttrib4dNV __glewVertexAttrib4dNV
#define glVertexAttrib4dvNV __glewVertexAttrib4dvNV
#define glVertexAttrib4fNV __glewVertexAttrib4fNV
#define glVertexAttrib4fvNV __glewVertexAttrib4fvNV
#define glVertexAttrib4sNV __glewVertexAttrib4sNV
#define glVertexAttrib4svNV __glewVertexAttrib4svNV
#define glVertexAttrib4ubNV __glewVertexAttrib4ubNV
#define glVertexAttrib4ubvNV __glewVertexAttrib4ubvNV
#define glVertexAttribPointerNV __glewVertexAttribPointerNV
#define glVertexAttribs1dvNV __glewVertexAttribs1dvNV
#define glVertexAttribs1fvNV __glewVertexAttribs1fvNV
#define glVertexAttribs1svNV __glewVertexAttribs1svNV
#define glVertexAttribs2dvNV __glewVertexAttribs2dvNV
#define glVertexAttribs2fvNV __glewVertexAttribs2fvNV
#define glVertexAttribs2svNV __glewVertexAttribs2svNV
#define glVertexAttribs3dvNV __glewVertexAttribs3dvNV
#define glVertexAttribs3fvNV __glewVertexAttribs3fvNV
#define glVertexAttribs3svNV __glewVertexAttribs3svNV
#define glVertexAttribs4dvNV __glewVertexAttribs4dvNV
#define glVertexAttribs4fvNV __glewVertexAttribs4fvNV
#define glVertexAttribs4svNV __glewVertexAttribs4svNV
#define glVertexAttribs4ubvNV __glewVertexAttribs4ubvNV
#define glBeginVideoCaptureNV __glewBeginVideoCaptureNV
#define glBindVideoCaptureStreamBufferNV __glewBindVideoCaptureStreamBufferNV
#define glBindVideoCaptureStreamTextureNV __glewBindVideoCaptureStreamTextureNV
#define glEndVideoCaptureNV __glewEndVideoCaptureNV
#define glGetVideoCaptureStreamdvNV __glewGetVideoCaptureStreamdvNV
#define glGetVideoCaptureStreamfvNV __glewGetVideoCaptureStreamfvNV
#define glGetVideoCaptureStreamivNV __glewGetVideoCaptureStreamivNV
#define glGetVideoCaptureivNV __glewGetVideoCaptureivNV
#define glVideoCaptureNV __glewVideoCaptureNV
#define glVideoCaptureStreamParameterdvNV __glewVideoCaptureStreamParameterdvNV
#define glVideoCaptureStreamParameterfvNV __glewVideoCaptureStreamParameterfvNV
#define glVideoCaptureStreamParameterivNV __glewVideoCaptureStreamParameterivNV
#define glClearDepthfOES __glewClearDepthfOES
#define glClipPlanefOES __glewClipPlanefOES
#define glDepthRangefOES __glewDepthRangefOES
#define glFrustumfOES __glewFrustumfOES
#define glGetClipPlanefOES __glewGetClipPlanefOES
#define glOrthofOES __glewOrthofOES
#define glDetailTexFuncSGIS __glewDetailTexFuncSGIS
#define glGetDetailTexFuncSGIS __glewGetDetailTexFuncSGIS
#define glFogFuncSGIS __glewFogFuncSGIS
#define glGetFogFuncSGIS __glewGetFogFuncSGIS
#define glSampleMaskSGIS __glewSampleMaskSGIS
#define glSamplePatternSGIS __glewSamplePatternSGIS
#define glGetSharpenTexFuncSGIS __glewGetSharpenTexFuncSGIS
#define glSharpenTexFuncSGIS __glewSharpenTexFuncSGIS
#define glTexImage4DSGIS __glewTexImage4DSGIS
#define glTexSubImage4DSGIS __glewTexSubImage4DSGIS
#define glGetTexFilterFuncSGIS __glewGetTexFilterFuncSGIS
#define glTexFilterFuncSGIS __glewTexFilterFuncSGIS
#define glAsyncMarkerSGIX __glewAsyncMarkerSGIX
#define glDeleteAsyncMarkersSGIX __glewDeleteAsyncMarkersSGIX
#define glFinishAsyncSGIX __glewFinishAsyncSGIX
#define glGenAsyncMarkersSGIX __glewGenAsyncMarkersSGIX
#define glIsAsyncMarkerSGIX __glewIsAsyncMarkerSGIX
#define glPollAsyncSGIX __glewPollAsyncSGIX
#define glFlushRasterSGIX __glewFlushRasterSGIX
#define glTextureFogSGIX __glewTextureFogSGIX
#define glFragmentColorMaterialSGIX __glewFragmentColorMaterialSGIX
#define glFragmentLightModelfSGIX __glewFragmentLightModelfSGIX
#define glFragmentLightModelfvSGIX __glewFragmentLightModelfvSGIX
#define glFragmentLightModeliSGIX __glewFragmentLightModeliSGIX
#define glFragmentLightModelivSGIX __glewFragmentLightModelivSGIX
#define glFragmentLightfSGIX __glewFragmentLightfSGIX
#define glFragmentLightfvSGIX __glewFragmentLightfvSGIX
#define glFragmentLightiSGIX __glewFragmentLightiSGIX
#define glFragmentLightivSGIX __glewFragmentLightivSGIX
#define glFragmentMaterialfSGIX __glewFragmentMaterialfSGIX
#define glFragmentMaterialfvSGIX __glewFragmentMaterialfvSGIX
#define glFragmentMaterialiSGIX __glewFragmentMaterialiSGIX
#define glFragmentMaterialivSGIX __glewFragmentMaterialivSGIX
#define glGetFragmentLightfvSGIX __glewGetFragmentLightfvSGIX
#define glGetFragmentLightivSGIX __glewGetFragmentLightivSGIX
#define glGetFragmentMaterialfvSGIX __glewGetFragmentMaterialfvSGIX
#define glGetFragmentMaterialivSGIX __glewGetFragmentMaterialivSGIX
#define glFrameZoomSGIX __glewFrameZoomSGIX
#define glPixelTexGenSGIX __glewPixelTexGenSGIX
#define glReferencePlaneSGIX __glewReferencePlaneSGIX
#define glSpriteParameterfSGIX __glewSpriteParameterfSGIX
#define glSpriteParameterfvSGIX __glewSpriteParameterfvSGIX
#define glSpriteParameteriSGIX __glewSpriteParameteriSGIX
#define glSpriteParameterivSGIX __glewSpriteParameterivSGIX
#define glTagSampleBufferSGIX __glewTagSampleBufferSGIX
#define glColorTableParameterfvSGI __glewColorTableParameterfvSGI
#define glColorTableParameterivSGI __glewColorTableParameterivSGI
#define glColorTableSGI __glewColorTableSGI
#define glCopyColorTableSGI __glewCopyColorTableSGI
#define glGetColorTableParameterfvSGI __glewGetColorTableParameterfvSGI
#define glGetColorTableParameterivSGI __glewGetColorTableParameterivSGI
#define glGetColorTableSGI __glewGetColorTableSGI
#define glFinishTextureSUNX __glewFinishTextureSUNX
#define glGlobalAlphaFactorbSUN __glewGlobalAlphaFactorbSUN
#define glGlobalAlphaFactordSUN __glewGlobalAlphaFactordSUN
#define glGlobalAlphaFactorfSUN __glewGlobalAlphaFactorfSUN
#define glGlobalAlphaFactoriSUN __glewGlobalAlphaFactoriSUN
#define glGlobalAlphaFactorsSUN __glewGlobalAlphaFactorsSUN
#define glGlobalAlphaFactorubSUN __glewGlobalAlphaFactorubSUN
#define glGlobalAlphaFactoruiSUN __glewGlobalAlphaFactoruiSUN
#define glGlobalAlphaFactorusSUN __glewGlobalAlphaFactorusSUN
#define glReadVideoPixelsSUN __glewReadVideoPixelsSUN
#define glReplacementCodePointerSUN __glewReplacementCodePointerSUN
#define glReplacementCodeubSUN __glewReplacementCodeubSUN
#define glReplacementCodeubvSUN __glewReplacementCodeubvSUN
#define glReplacementCodeuiSUN __glewReplacementCodeuiSUN
#define glReplacementCodeuivSUN __glewReplacementCodeuivSUN
#define glReplacementCodeusSUN __glewReplacementCodeusSUN
#define glReplacementCodeusvSUN __glewReplacementCodeusvSUN
#define glColor3fVertex3fSUN __glewColor3fVertex3fSUN
#define glColor3fVertex3fvSUN __glewColor3fVertex3fvSUN
#define glColor4fNormal3fVertex3fSUN __glewColor4fNormal3fVertex3fSUN
#define glColor4fNormal3fVertex3fvSUN __glewColor4fNormal3fVertex3fvSUN
#define glColor4ubVertex2fSUN __glewColor4ubVertex2fSUN
#define glColor4ubVertex2fvSUN __glewColor4ubVertex2fvSUN
#define glColor4ubVertex3fSUN __glewColor4ubVertex3fSUN
#define glColor4ubVertex3fvSUN __glewColor4ubVertex3fvSUN
#define glNormal3fVertex3fSUN __glewNormal3fVertex3fSUN
#define glNormal3fVertex3fvSUN __glewNormal3fVertex3fvSUN
#define glReplacementCodeuiColor3fVertex3fSUN __glewReplacementCodeuiColor3fVertex3fSUN
#define glReplacementCodeuiColor3fVertex3fvSUN __glewReplacementCodeuiColor3fVertex3fvSUN
#define glReplacementCodeuiColor4fNormal3fVertex3fSUN __glewReplacementCodeuiColor4fNormal3fVertex3fSUN
#define glReplacementCodeuiColor4fNormal3fVertex3fvSUN __glewReplacementCodeuiColor4fNormal3fVertex3fvSUN
#define glReplacementCodeuiColor4ubVertex3fSUN __glewReplacementCodeuiColor4ubVertex3fSUN
#define glReplacementCodeuiColor4ubVertex3fvSUN __glewReplacementCodeuiColor4ubVertex3fvSUN
#define glReplacementCodeuiNormal3fVertex3fSUN __glewReplacementCodeuiNormal3fVertex3fSUN
#define glReplacementCodeuiNormal3fVertex3fvSUN __glewReplacementCodeuiNormal3fVertex3fvSUN
#define glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN
#define glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN
#define glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN __glewReplacementCodeuiTexCoord2fNormal3fVertex3fSUN
#define glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN __glewReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN
#define glReplacementCodeuiTexCoord2fVertex3fSUN __glewReplacementCodeuiTexCoord2fVertex3fSUN
#define glReplacementCodeuiTexCoord2fVertex3fvSUN __glewReplacementCodeuiTexCoord2fVertex3fvSUN
#define glReplacementCodeuiVertex3fSUN __glewReplacementCodeuiVertex3fSUN
#define glReplacementCodeuiVertex3fvSUN __glewReplacementCodeuiVertex3fvSUN
#define glTexCoord2fColor3fVertex3fSUN __glewTexCoord2fColor3fVertex3fSUN
#define glTexCoord2fColor3fVertex3fvSUN __glewTexCoord2fColor3fVertex3fvSUN
#define glTexCoord2fColor4fNormal3fVertex3fSUN __glewTexCoord2fColor4fNormal3fVertex3fSUN
#define glTexCoord2fColor4fNormal3fVertex3fvSUN __glewTexCoord2fColor4fNormal3fVertex3fvSUN
#define glTexCoord2fColor4ubVertex3fSUN __glewTexCoord2fColor4ubVertex3fSUN
#define glTexCoord2fColor4ubVertex3fvSUN __glewTexCoord2fColor4ubVertex3fvSUN
#define glTexCoord2fNormal3fVertex3fSUN __glewTexCoord2fNormal3fVertex3fSUN
#define glTexCoord2fNormal3fVertex3fvSUN __glewTexCoord2fNormal3fVertex3fvSUN
#define glTexCoord2fVertex3fSUN __glewTexCoord2fVertex3fSUN
#define glTexCoord2fVertex3fvSUN __glewTexCoord2fVertex3fvSUN
#define glTexCoord4fColor4fNormal3fVertex4fSUN __glewTexCoord4fColor4fNormal3fVertex4fSUN
#define glTexCoord4fColor4fNormal3fVertex4fvSUN __glewTexCoord4fColor4fNormal3fVertex4fvSUN
#define glTexCoord4fVertex4fSUN __glewTexCoord4fVertex4fSUN
#define glTexCoord4fVertex4fvSUN __glewTexCoord4fVertex4fvSUN
#define glAddSwapHintRectWIN __glewAddSwapHintRectWIN
#endif


extern __attribute__ ((visibility("default"))) GLenum glewInit ();
extern __attribute__ ((visibility("default"))) GLboolean glewIsSupported (const char* name);

extern __attribute__ ((visibility("default"))) GLboolean glewExperimental;
extern __attribute__ ((visibility("default"))) GLboolean glewGetExtension (const char* name);
extern __attribute__ ((visibility("default"))) const GLubyte* glewGetErrorString (GLenum error);
extern __attribute__ ((visibility("default"))) const GLubyte* glewGetString (GLenum name);

extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXSUBIMAGE3DPROC __glewCopyTexSubImage3D;
extern __attribute__ ((visibility("default"))) PFNGLDRAWRANGEELEMENTSPROC __glewDrawRangeElements;
extern __attribute__ ((visibility("default"))) PFNGLTEXIMAGE3DPROC __glewTexImage3D;
extern __attribute__ ((visibility("default"))) PFNGLTEXSUBIMAGE3DPROC __glewTexSubImage3D;

extern __attribute__ ((visibility("default"))) PFNGLACTIVETEXTUREPROC __glewActiveTexture;
extern __attribute__ ((visibility("default"))) PFNGLCLIENTACTIVETEXTUREPROC __glewClientActiveTexture;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXIMAGE1DPROC __glewCompressedTexImage1D;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXIMAGE2DPROC __glewCompressedTexImage2D;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXIMAGE3DPROC __glewCompressedTexImage3D;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC __glewCompressedTexSubImage1D;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC __glewCompressedTexSubImage2D;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC __glewCompressedTexSubImage3D;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMPRESSEDTEXIMAGEPROC __glewGetCompressedTexImage;
extern __attribute__ ((visibility("default"))) PFNGLLOADTRANSPOSEMATRIXDPROC __glewLoadTransposeMatrixd;
extern __attribute__ ((visibility("default"))) PFNGLLOADTRANSPOSEMATRIXFPROC __glewLoadTransposeMatrixf;
extern __attribute__ ((visibility("default"))) PFNGLMULTTRANSPOSEMATRIXDPROC __glewMultTransposeMatrixd;
extern __attribute__ ((visibility("default"))) PFNGLMULTTRANSPOSEMATRIXFPROC __glewMultTransposeMatrixf;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1DPROC __glewMultiTexCoord1d;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1DVPROC __glewMultiTexCoord1dv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1FPROC __glewMultiTexCoord1f;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1FVPROC __glewMultiTexCoord1fv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1IPROC __glewMultiTexCoord1i;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1IVPROC __glewMultiTexCoord1iv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1SPROC __glewMultiTexCoord1s;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1SVPROC __glewMultiTexCoord1sv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2DPROC __glewMultiTexCoord2d;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2DVPROC __glewMultiTexCoord2dv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2FPROC __glewMultiTexCoord2f;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2FVPROC __glewMultiTexCoord2fv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2IPROC __glewMultiTexCoord2i;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2IVPROC __glewMultiTexCoord2iv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2SPROC __glewMultiTexCoord2s;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2SVPROC __glewMultiTexCoord2sv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3DPROC __glewMultiTexCoord3d;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3DVPROC __glewMultiTexCoord3dv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3FPROC __glewMultiTexCoord3f;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3FVPROC __glewMultiTexCoord3fv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3IPROC __glewMultiTexCoord3i;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3IVPROC __glewMultiTexCoord3iv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3SPROC __glewMultiTexCoord3s;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3SVPROC __glewMultiTexCoord3sv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4DPROC __glewMultiTexCoord4d;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4DVPROC __glewMultiTexCoord4dv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4FPROC __glewMultiTexCoord4f;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4FVPROC __glewMultiTexCoord4fv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4IPROC __glewMultiTexCoord4i;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4IVPROC __glewMultiTexCoord4iv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4SPROC __glewMultiTexCoord4s;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4SVPROC __glewMultiTexCoord4sv;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLECOVERAGEPROC __glewSampleCoverage;

extern __attribute__ ((visibility("default"))) PFNGLBLENDCOLORPROC __glewBlendColor;
extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONPROC __glewBlendEquation;
extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCSEPARATEPROC __glewBlendFuncSeparate;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDPOINTERPROC __glewFogCoordPointer;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDDPROC __glewFogCoordd;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDDVPROC __glewFogCoorddv;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDFPROC __glewFogCoordf;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDFVPROC __glewFogCoordfv;
extern __attribute__ ((visibility("default"))) PFNGLMULTIDRAWARRAYSPROC __glewMultiDrawArrays;
extern __attribute__ ((visibility("default"))) PFNGLMULTIDRAWELEMENTSPROC __glewMultiDrawElements;
extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERFPROC __glewPointParameterf;
extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERFVPROC __glewPointParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERIPROC __glewPointParameteri;
extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERIVPROC __glewPointParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3BPROC __glewSecondaryColor3b;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3BVPROC __glewSecondaryColor3bv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3DPROC __glewSecondaryColor3d;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3DVPROC __glewSecondaryColor3dv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3FPROC __glewSecondaryColor3f;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3FVPROC __glewSecondaryColor3fv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3IPROC __glewSecondaryColor3i;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3IVPROC __glewSecondaryColor3iv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3SPROC __glewSecondaryColor3s;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3SVPROC __glewSecondaryColor3sv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UBPROC __glewSecondaryColor3ub;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UBVPROC __glewSecondaryColor3ubv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UIPROC __glewSecondaryColor3ui;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UIVPROC __glewSecondaryColor3uiv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3USPROC __glewSecondaryColor3us;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3USVPROC __glewSecondaryColor3usv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLORPOINTERPROC __glewSecondaryColorPointer;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2DPROC __glewWindowPos2d;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2DVPROC __glewWindowPos2dv;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2FPROC __glewWindowPos2f;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2FVPROC __glewWindowPos2fv;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2IPROC __glewWindowPos2i;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2IVPROC __glewWindowPos2iv;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2SPROC __glewWindowPos2s;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2SVPROC __glewWindowPos2sv;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3DPROC __glewWindowPos3d;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3DVPROC __glewWindowPos3dv;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3FPROC __glewWindowPos3f;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3FVPROC __glewWindowPos3fv;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3IPROC __glewWindowPos3i;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3IVPROC __glewWindowPos3iv;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3SPROC __glewWindowPos3s;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3SVPROC __glewWindowPos3sv;

extern __attribute__ ((visibility("default"))) PFNGLBEGINQUERYPROC __glewBeginQuery;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERPROC __glewBindBuffer;
extern __attribute__ ((visibility("default"))) PFNGLBUFFERDATAPROC __glewBufferData;
extern __attribute__ ((visibility("default"))) PFNGLBUFFERSUBDATAPROC __glewBufferSubData;
extern __attribute__ ((visibility("default"))) PFNGLDELETEBUFFERSPROC __glewDeleteBuffers;
extern __attribute__ ((visibility("default"))) PFNGLDELETEQUERIESPROC __glewDeleteQueries;
extern __attribute__ ((visibility("default"))) PFNGLENDQUERYPROC __glewEndQuery;
extern __attribute__ ((visibility("default"))) PFNGLGENBUFFERSPROC __glewGenBuffers;
extern __attribute__ ((visibility("default"))) PFNGLGENQUERIESPROC __glewGenQueries;
extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERPARAMETERIVPROC __glewGetBufferParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERPOINTERVPROC __glewGetBufferPointerv;
extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERSUBDATAPROC __glewGetBufferSubData;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTIVPROC __glewGetQueryObjectiv;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTUIVPROC __glewGetQueryObjectuiv;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYIVPROC __glewGetQueryiv;
extern __attribute__ ((visibility("default"))) PFNGLISBUFFERPROC __glewIsBuffer;
extern __attribute__ ((visibility("default"))) PFNGLISQUERYPROC __glewIsQuery;
extern __attribute__ ((visibility("default"))) PFNGLMAPBUFFERPROC __glewMapBuffer;
extern __attribute__ ((visibility("default"))) PFNGLUNMAPBUFFERPROC __glewUnmapBuffer;

extern __attribute__ ((visibility("default"))) PFNGLATTACHSHADERPROC __glewAttachShader;
extern __attribute__ ((visibility("default"))) PFNGLBINDATTRIBLOCATIONPROC __glewBindAttribLocation;
extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONSEPARATEPROC __glewBlendEquationSeparate;
extern __attribute__ ((visibility("default"))) PFNGLCOMPILESHADERPROC __glewCompileShader;
extern __attribute__ ((visibility("default"))) PFNGLCREATEPROGRAMPROC __glewCreateProgram;
extern __attribute__ ((visibility("default"))) PFNGLCREATESHADERPROC __glewCreateShader;
extern __attribute__ ((visibility("default"))) PFNGLDELETEPROGRAMPROC __glewDeleteProgram;
extern __attribute__ ((visibility("default"))) PFNGLDELETESHADERPROC __glewDeleteShader;
extern __attribute__ ((visibility("default"))) PFNGLDETACHSHADERPROC __glewDetachShader;
extern __attribute__ ((visibility("default"))) PFNGLDISABLEVERTEXATTRIBARRAYPROC __glewDisableVertexAttribArray;
extern __attribute__ ((visibility("default"))) PFNGLDRAWBUFFERSPROC __glewDrawBuffers;
extern __attribute__ ((visibility("default"))) PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEATTRIBPROC __glewGetActiveAttrib;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEUNIFORMPROC __glewGetActiveUniform;
extern __attribute__ ((visibility("default"))) PFNGLGETATTACHEDSHADERSPROC __glewGetAttachedShaders;
extern __attribute__ ((visibility("default"))) PFNGLGETATTRIBLOCATIONPROC __glewGetAttribLocation;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMINFOLOGPROC __glewGetProgramInfoLog;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMIVPROC __glewGetProgramiv;
extern __attribute__ ((visibility("default"))) PFNGLGETSHADERINFOLOGPROC __glewGetShaderInfoLog;
extern __attribute__ ((visibility("default"))) PFNGLGETSHADERSOURCEPROC __glewGetShaderSource;
extern __attribute__ ((visibility("default"))) PFNGLGETSHADERIVPROC __glewGetShaderiv;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMLOCATIONPROC __glewGetUniformLocation;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMFVPROC __glewGetUniformfv;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMIVPROC __glewGetUniformiv;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBPOINTERVPROC __glewGetVertexAttribPointerv;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBDVPROC __glewGetVertexAttribdv;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBFVPROC __glewGetVertexAttribfv;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBIVPROC __glewGetVertexAttribiv;
extern __attribute__ ((visibility("default"))) PFNGLISPROGRAMPROC __glewIsProgram;
extern __attribute__ ((visibility("default"))) PFNGLISSHADERPROC __glewIsShader;
extern __attribute__ ((visibility("default"))) PFNGLLINKPROGRAMPROC __glewLinkProgram;
extern __attribute__ ((visibility("default"))) PFNGLSHADERSOURCEPROC __glewShaderSource;
extern __attribute__ ((visibility("default"))) PFNGLSTENCILFUNCSEPARATEPROC __glewStencilFuncSeparate;
extern __attribute__ ((visibility("default"))) PFNGLSTENCILMASKSEPARATEPROC __glewStencilMaskSeparate;
extern __attribute__ ((visibility("default"))) PFNGLSTENCILOPSEPARATEPROC __glewStencilOpSeparate;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1FPROC __glewUniform1f;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1FVPROC __glewUniform1fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1IPROC __glewUniform1i;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1IVPROC __glewUniform1iv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2FPROC __glewUniform2f;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2FVPROC __glewUniform2fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2IPROC __glewUniform2i;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2IVPROC __glewUniform2iv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3FPROC __glewUniform3f;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3FVPROC __glewUniform3fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3IPROC __glewUniform3i;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3IVPROC __glewUniform3iv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4FPROC __glewUniform4f;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4FVPROC __glewUniform4fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4IPROC __glewUniform4i;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4IVPROC __glewUniform4iv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX2FVPROC __glewUniformMatrix2fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX3FVPROC __glewUniformMatrix3fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX4FVPROC __glewUniformMatrix4fv;
extern __attribute__ ((visibility("default"))) PFNGLUSEPROGRAMPROC __glewUseProgram;
extern __attribute__ ((visibility("default"))) PFNGLVALIDATEPROGRAMPROC __glewValidateProgram;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1DPROC __glewVertexAttrib1d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1DVPROC __glewVertexAttrib1dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1FPROC __glewVertexAttrib1f;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1FVPROC __glewVertexAttrib1fv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1SPROC __glewVertexAttrib1s;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1SVPROC __glewVertexAttrib1sv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2DPROC __glewVertexAttrib2d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2DVPROC __glewVertexAttrib2dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2FPROC __glewVertexAttrib2f;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2FVPROC __glewVertexAttrib2fv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2SPROC __glewVertexAttrib2s;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2SVPROC __glewVertexAttrib2sv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3DPROC __glewVertexAttrib3d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3DVPROC __glewVertexAttrib3dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3FPROC __glewVertexAttrib3f;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3FVPROC __glewVertexAttrib3fv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3SPROC __glewVertexAttrib3s;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3SVPROC __glewVertexAttrib3sv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NBVPROC __glewVertexAttrib4Nbv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NIVPROC __glewVertexAttrib4Niv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NSVPROC __glewVertexAttrib4Nsv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUBPROC __glewVertexAttrib4Nub;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUBVPROC __glewVertexAttrib4Nubv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUIVPROC __glewVertexAttrib4Nuiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUSVPROC __glewVertexAttrib4Nusv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4BVPROC __glewVertexAttrib4bv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4DPROC __glewVertexAttrib4d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4DVPROC __glewVertexAttrib4dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4FPROC __glewVertexAttrib4f;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4FVPROC __glewVertexAttrib4fv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4IVPROC __glewVertexAttrib4iv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4SPROC __glewVertexAttrib4s;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4SVPROC __glewVertexAttrib4sv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4UBVPROC __glewVertexAttrib4ubv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4UIVPROC __glewVertexAttrib4uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4USVPROC __glewVertexAttrib4usv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer;

extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX2X3FVPROC __glewUniformMatrix2x3fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX2X4FVPROC __glewUniformMatrix2x4fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX3X2FVPROC __glewUniformMatrix3x2fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX3X4FVPROC __glewUniformMatrix3x4fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX4X2FVPROC __glewUniformMatrix4x2fv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX4X3FVPROC __glewUniformMatrix4x3fv;

extern __attribute__ ((visibility("default"))) PFNGLBEGINCONDITIONALRENDERPROC __glewBeginConditionalRender;
extern __attribute__ ((visibility("default"))) PFNGLBEGINTRANSFORMFEEDBACKPROC __glewBeginTransformFeedback;
extern __attribute__ ((visibility("default"))) PFNGLBINDFRAGDATALOCATIONPROC __glewBindFragDataLocation;
extern __attribute__ ((visibility("default"))) PFNGLCLAMPCOLORPROC __glewClampColor;
extern __attribute__ ((visibility("default"))) PFNGLCLEARBUFFERFIPROC __glewClearBufferfi;
extern __attribute__ ((visibility("default"))) PFNGLCLEARBUFFERFVPROC __glewClearBufferfv;
extern __attribute__ ((visibility("default"))) PFNGLCLEARBUFFERIVPROC __glewClearBufferiv;
extern __attribute__ ((visibility("default"))) PFNGLCLEARBUFFERUIVPROC __glewClearBufferuiv;
extern __attribute__ ((visibility("default"))) PFNGLCOLORMASKIPROC __glewColorMaski;
extern __attribute__ ((visibility("default"))) PFNGLDISABLEIPROC __glewDisablei;
extern __attribute__ ((visibility("default"))) PFNGLENABLEIPROC __glewEnablei;
extern __attribute__ ((visibility("default"))) PFNGLENDCONDITIONALRENDERPROC __glewEndConditionalRender;
extern __attribute__ ((visibility("default"))) PFNGLENDTRANSFORMFEEDBACKPROC __glewEndTransformFeedback;
extern __attribute__ ((visibility("default"))) PFNGLGETBOOLEANI_VPROC __glewGetBooleani_v;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGDATALOCATIONPROC __glewGetFragDataLocation;
extern __attribute__ ((visibility("default"))) PFNGLGETSTRINGIPROC __glewGetStringi;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXPARAMETERIIVPROC __glewGetTexParameterIiv;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXPARAMETERIUIVPROC __glewGetTexParameterIuiv;
extern __attribute__ ((visibility("default"))) PFNGLGETTRANSFORMFEEDBACKVARYINGPROC __glewGetTransformFeedbackVarying;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMUIVPROC __glewGetUniformuiv;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBIIVPROC __glewGetVertexAttribIiv;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBIUIVPROC __glewGetVertexAttribIuiv;
extern __attribute__ ((visibility("default"))) PFNGLISENABLEDIPROC __glewIsEnabledi;
extern __attribute__ ((visibility("default"))) PFNGLTEXPARAMETERIIVPROC __glewTexParameterIiv;
extern __attribute__ ((visibility("default"))) PFNGLTEXPARAMETERIUIVPROC __glewTexParameterIuiv;
extern __attribute__ ((visibility("default"))) PFNGLTRANSFORMFEEDBACKVARYINGSPROC __glewTransformFeedbackVaryings;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1UIPROC __glewUniform1ui;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1UIVPROC __glewUniform1uiv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2UIPROC __glewUniform2ui;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2UIVPROC __glewUniform2uiv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3UIPROC __glewUniform3ui;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3UIVPROC __glewUniform3uiv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4UIPROC __glewUniform4ui;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4UIVPROC __glewUniform4uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1IPROC __glewVertexAttribI1i;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1IVPROC __glewVertexAttribI1iv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1UIPROC __glewVertexAttribI1ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1UIVPROC __glewVertexAttribI1uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2IPROC __glewVertexAttribI2i;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2IVPROC __glewVertexAttribI2iv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2UIPROC __glewVertexAttribI2ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2UIVPROC __glewVertexAttribI2uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3IPROC __glewVertexAttribI3i;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3IVPROC __glewVertexAttribI3iv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3UIPROC __glewVertexAttribI3ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3UIVPROC __glewVertexAttribI3uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4BVPROC __glewVertexAttribI4bv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4IPROC __glewVertexAttribI4i;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4IVPROC __glewVertexAttribI4iv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4SVPROC __glewVertexAttribI4sv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4UBVPROC __glewVertexAttribI4ubv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4UIPROC __glewVertexAttribI4ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4UIVPROC __glewVertexAttribI4uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4USVPROC __glewVertexAttribI4usv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBIPOINTERPROC __glewVertexAttribIPointer;

extern __attribute__ ((visibility("default"))) PFNGLDRAWARRAYSINSTANCEDPROC __glewDrawArraysInstanced;
extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTSINSTANCEDPROC __glewDrawElementsInstanced;
extern __attribute__ ((visibility("default"))) PFNGLPRIMITIVERESTARTINDEXPROC __glewPrimitiveRestartIndex;
extern __attribute__ ((visibility("default"))) PFNGLTEXBUFFERPROC __glewTexBuffer;

extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTUREPROC __glewFramebufferTexture;
extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERPARAMETERI64VPROC __glewGetBufferParameteri64v;
extern __attribute__ ((visibility("default"))) PFNGLGETINTEGER64I_VPROC __glewGetInteger64i_v;

extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBDIVISORPROC __glewVertexAttribDivisor;

extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONSEPARATEIPROC __glewBlendEquationSeparatei;
extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONIPROC __glewBlendEquationi;
extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCSEPARATEIPROC __glewBlendFuncSeparatei;
extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCIPROC __glewBlendFunci;
extern __attribute__ ((visibility("default"))) PFNGLMINSAMPLESHADINGPROC __glewMinSampleShading;

extern __attribute__ ((visibility("default"))) PFNGLTBUFFERMASK3DFXPROC __glewTbufferMask3DFX;

extern __attribute__ ((visibility("default"))) PFNGLDEBUGMESSAGECALLBACKAMDPROC __glewDebugMessageCallbackAMD;
extern __attribute__ ((visibility("default"))) PFNGLDEBUGMESSAGEENABLEAMDPROC __glewDebugMessageEnableAMD;
extern __attribute__ ((visibility("default"))) PFNGLDEBUGMESSAGEINSERTAMDPROC __glewDebugMessageInsertAMD;
extern __attribute__ ((visibility("default"))) PFNGLGETDEBUGMESSAGELOGAMDPROC __glewGetDebugMessageLogAMD;

extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONINDEXEDAMDPROC __glewBlendEquationIndexedAMD;
extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONSEPARATEINDEXEDAMDPROC __glewBlendEquationSeparateIndexedAMD;
extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCINDEXEDAMDPROC __glewBlendFuncIndexedAMD;
extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCSEPARATEINDEXEDAMDPROC __glewBlendFuncSeparateIndexedAMD;

extern __attribute__ ((visibility("default"))) PFNGLDELETENAMESAMDPROC __glewDeleteNamesAMD;
extern __attribute__ ((visibility("default"))) PFNGLGENNAMESAMDPROC __glewGenNamesAMD;
extern __attribute__ ((visibility("default"))) PFNGLISNAMEAMDPROC __glewIsNameAMD;

extern __attribute__ ((visibility("default"))) PFNGLBEGINPERFMONITORAMDPROC __glewBeginPerfMonitorAMD;
extern __attribute__ ((visibility("default"))) PFNGLDELETEPERFMONITORSAMDPROC __glewDeletePerfMonitorsAMD;
extern __attribute__ ((visibility("default"))) PFNGLENDPERFMONITORAMDPROC __glewEndPerfMonitorAMD;
extern __attribute__ ((visibility("default"))) PFNGLGENPERFMONITORSAMDPROC __glewGenPerfMonitorsAMD;
extern __attribute__ ((visibility("default"))) PFNGLGETPERFMONITORCOUNTERDATAAMDPROC __glewGetPerfMonitorCounterDataAMD;
extern __attribute__ ((visibility("default"))) PFNGLGETPERFMONITORCOUNTERINFOAMDPROC __glewGetPerfMonitorCounterInfoAMD;
extern __attribute__ ((visibility("default"))) PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC __glewGetPerfMonitorCounterStringAMD;
extern __attribute__ ((visibility("default"))) PFNGLGETPERFMONITORCOUNTERSAMDPROC __glewGetPerfMonitorCountersAMD;
extern __attribute__ ((visibility("default"))) PFNGLGETPERFMONITORGROUPSTRINGAMDPROC __glewGetPerfMonitorGroupStringAMD;
extern __attribute__ ((visibility("default"))) PFNGLGETPERFMONITORGROUPSAMDPROC __glewGetPerfMonitorGroupsAMD;
extern __attribute__ ((visibility("default"))) PFNGLSELECTPERFMONITORCOUNTERSAMDPROC __glewSelectPerfMonitorCountersAMD;

extern __attribute__ ((visibility("default"))) PFNGLSETMULTISAMPLEFVAMDPROC __glewSetMultisamplefvAMD;

extern __attribute__ ((visibility("default"))) PFNGLTESSELLATIONFACTORAMDPROC __glewTessellationFactorAMD;
extern __attribute__ ((visibility("default"))) PFNGLTESSELLATIONMODEAMDPROC __glewTessellationModeAMD;

extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTARRAYAPPLEPROC __glewDrawElementArrayAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC __glewDrawRangeElementArrayAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLELEMENTPOINTERAPPLEPROC __glewElementPointerAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC __glewMultiDrawElementArrayAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC __glewMultiDrawRangeElementArrayAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLDELETEFENCESAPPLEPROC __glewDeleteFencesAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLFINISHFENCEAPPLEPROC __glewFinishFenceAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLFINISHOBJECTAPPLEPROC __glewFinishObjectAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLGENFENCESAPPLEPROC __glewGenFencesAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLISFENCEAPPLEPROC __glewIsFenceAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLSETFENCEAPPLEPROC __glewSetFenceAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLTESTFENCEAPPLEPROC __glewTestFenceAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLTESTOBJECTAPPLEPROC __glewTestObjectAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLBUFFERPARAMETERIAPPLEPROC __glewBufferParameteriAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC __glewFlushMappedBufferRangeAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLGETOBJECTPARAMETERIVAPPLEPROC __glewGetObjectParameterivAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLOBJECTPURGEABLEAPPLEPROC __glewObjectPurgeableAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLOBJECTUNPURGEABLEAPPLEPROC __glewObjectUnpurgeableAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC __glewGetTexParameterPointervAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLTEXTURERANGEAPPLEPROC __glewTextureRangeAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLBINDVERTEXARRAYAPPLEPROC __glewBindVertexArrayAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLDELETEVERTEXARRAYSAPPLEPROC __glewDeleteVertexArraysAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLGENVERTEXARRAYSAPPLEPROC __glewGenVertexArraysAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLISVERTEXARRAYAPPLEPROC __glewIsVertexArrayAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC __glewFlushVertexArrayRangeAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYPARAMETERIAPPLEPROC __glewVertexArrayParameteriAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYRANGEAPPLEPROC __glewVertexArrayRangeAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLDISABLEVERTEXATTRIBAPPLEPROC __glewDisableVertexAttribAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLENABLEVERTEXATTRIBAPPLEPROC __glewEnableVertexAttribAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLISVERTEXATTRIBENABLEDAPPLEPROC __glewIsVertexAttribEnabledAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLMAPVERTEXATTRIB1DAPPLEPROC __glewMapVertexAttrib1dAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLMAPVERTEXATTRIB1FAPPLEPROC __glewMapVertexAttrib1fAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLMAPVERTEXATTRIB2DAPPLEPROC __glewMapVertexAttrib2dAPPLE;
extern __attribute__ ((visibility("default"))) PFNGLMAPVERTEXATTRIB2FAPPLEPROC __glewMapVertexAttrib2fAPPLE;

extern __attribute__ ((visibility("default"))) PFNGLCLEARDEPTHFPROC __glewClearDepthf;
extern __attribute__ ((visibility("default"))) PFNGLDEPTHRANGEFPROC __glewDepthRangef;
extern __attribute__ ((visibility("default"))) PFNGLGETSHADERPRECISIONFORMATPROC __glewGetShaderPrecisionFormat;
extern __attribute__ ((visibility("default"))) PFNGLRELEASESHADERCOMPILERPROC __glewReleaseShaderCompiler;
extern __attribute__ ((visibility("default"))) PFNGLSHADERBINARYPROC __glewShaderBinary;

extern __attribute__ ((visibility("default"))) PFNGLBINDFRAGDATALOCATIONINDEXEDPROC __glewBindFragDataLocationIndexed;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGDATAINDEXPROC __glewGetFragDataIndex;

extern __attribute__ ((visibility("default"))) PFNGLCREATESYNCFROMCLEVENTARBPROC __glewCreateSyncFromCLeventARB;

extern __attribute__ ((visibility("default"))) PFNGLCLAMPCOLORARBPROC __glewClampColorARB;

extern __attribute__ ((visibility("default"))) PFNGLCOPYBUFFERSUBDATAPROC __glewCopyBufferSubData;

extern __attribute__ ((visibility("default"))) PFNGLDEBUGMESSAGECALLBACKARBPROC __glewDebugMessageCallbackARB;
extern __attribute__ ((visibility("default"))) PFNGLDEBUGMESSAGECONTROLARBPROC __glewDebugMessageControlARB;
extern __attribute__ ((visibility("default"))) PFNGLDEBUGMESSAGEINSERTARBPROC __glewDebugMessageInsertARB;
extern __attribute__ ((visibility("default"))) PFNGLGETDEBUGMESSAGELOGARBPROC __glewGetDebugMessageLogARB;

extern __attribute__ ((visibility("default"))) PFNGLDRAWBUFFERSARBPROC __glewDrawBuffersARB;

extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONSEPARATEIARBPROC __glewBlendEquationSeparateiARB;
extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONIARBPROC __glewBlendEquationiARB;
extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCSEPARATEIARBPROC __glewBlendFuncSeparateiARB;
extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCIARBPROC __glewBlendFunciARB;

extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTSBASEVERTEXPROC __glewDrawElementsBaseVertex;
extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC __glewDrawElementsInstancedBaseVertex;
extern __attribute__ ((visibility("default"))) PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC __glewDrawRangeElementsBaseVertex;
extern __attribute__ ((visibility("default"))) PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC __glewMultiDrawElementsBaseVertex;

extern __attribute__ ((visibility("default"))) PFNGLDRAWARRAYSINDIRECTPROC __glewDrawArraysIndirect;
extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTSINDIRECTPROC __glewDrawElementsIndirect;

extern __attribute__ ((visibility("default"))) PFNGLBINDFRAMEBUFFERPROC __glewBindFramebuffer;
extern __attribute__ ((visibility("default"))) PFNGLBINDRENDERBUFFERPROC __glewBindRenderbuffer;
extern __attribute__ ((visibility("default"))) PFNGLBLITFRAMEBUFFERPROC __glewBlitFramebuffer;
extern __attribute__ ((visibility("default"))) PFNGLCHECKFRAMEBUFFERSTATUSPROC __glewCheckFramebufferStatus;
extern __attribute__ ((visibility("default"))) PFNGLDELETEFRAMEBUFFERSPROC __glewDeleteFramebuffers;
extern __attribute__ ((visibility("default"))) PFNGLDELETERENDERBUFFERSPROC __glewDeleteRenderbuffers;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERRENDERBUFFERPROC __glewFramebufferRenderbuffer;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURE1DPROC __glewFramebufferTexture1D;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURE2DPROC __glewFramebufferTexture2D;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURE3DPROC __glewFramebufferTexture3D;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURELAYERPROC __glewFramebufferTextureLayer;
extern __attribute__ ((visibility("default"))) PFNGLGENFRAMEBUFFERSPROC __glewGenFramebuffers;
extern __attribute__ ((visibility("default"))) PFNGLGENRENDERBUFFERSPROC __glewGenRenderbuffers;
extern __attribute__ ((visibility("default"))) PFNGLGENERATEMIPMAPPROC __glewGenerateMipmap;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC __glewGetFramebufferAttachmentParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLGETRENDERBUFFERPARAMETERIVPROC __glewGetRenderbufferParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLISFRAMEBUFFERPROC __glewIsFramebuffer;
extern __attribute__ ((visibility("default"))) PFNGLISRENDERBUFFERPROC __glewIsRenderbuffer;
extern __attribute__ ((visibility("default"))) PFNGLRENDERBUFFERSTORAGEPROC __glewRenderbufferStorage;
extern __attribute__ ((visibility("default"))) PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC __glewRenderbufferStorageMultisample;

extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTUREARBPROC __glewFramebufferTextureARB;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTUREFACEARBPROC __glewFramebufferTextureFaceARB;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURELAYERARBPROC __glewFramebufferTextureLayerARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETERIARBPROC __glewProgramParameteriARB;

extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMBINARYPROC __glewGetProgramBinary;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMBINARYPROC __glewProgramBinary;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETERIPROC __glewProgramParameteri;

extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMDVPROC __glewGetUniformdv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1DEXTPROC __glewProgramUniform1dEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1DVEXTPROC __glewProgramUniform1dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2DEXTPROC __glewProgramUniform2dEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2DVEXTPROC __glewProgramUniform2dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3DEXTPROC __glewProgramUniform3dEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3DVEXTPROC __glewProgramUniform3dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4DEXTPROC __glewProgramUniform4dEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4DVEXTPROC __glewProgramUniform4dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC __glewProgramUniformMatrix2dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC __glewProgramUniformMatrix2x3dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC __glewProgramUniformMatrix2x4dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC __glewProgramUniformMatrix3dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC __glewProgramUniformMatrix3x2dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC __glewProgramUniformMatrix3x4dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC __glewProgramUniformMatrix4dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC __glewProgramUniformMatrix4x2dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC __glewProgramUniformMatrix4x3dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1DPROC __glewUniform1d;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1DVPROC __glewUniform1dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2DPROC __glewUniform2d;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2DVPROC __glewUniform2dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3DPROC __glewUniform3d;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3DVPROC __glewUniform3dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4DPROC __glewUniform4d;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4DVPROC __glewUniform4dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX2DVPROC __glewUniformMatrix2dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX2X3DVPROC __glewUniformMatrix2x3dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX2X4DVPROC __glewUniformMatrix2x4dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX3DVPROC __glewUniformMatrix3dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX3X2DVPROC __glewUniformMatrix3x2dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX3X4DVPROC __glewUniformMatrix3x4dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX4DVPROC __glewUniformMatrix4dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX4X2DVPROC __glewUniformMatrix4x2dv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX4X3DVPROC __glewUniformMatrix4x3dv;

extern __attribute__ ((visibility("default"))) PFNGLCOLORSUBTABLEPROC __glewColorSubTable;
extern __attribute__ ((visibility("default"))) PFNGLCOLORTABLEPROC __glewColorTable;
extern __attribute__ ((visibility("default"))) PFNGLCOLORTABLEPARAMETERFVPROC __glewColorTableParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLCOLORTABLEPARAMETERIVPROC __glewColorTableParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONFILTER1DPROC __glewConvolutionFilter1D;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONFILTER2DPROC __glewConvolutionFilter2D;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERFPROC __glewConvolutionParameterf;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERFVPROC __glewConvolutionParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERIPROC __glewConvolutionParameteri;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERIVPROC __glewConvolutionParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCOLORSUBTABLEPROC __glewCopyColorSubTable;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCOLORTABLEPROC __glewCopyColorTable;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCONVOLUTIONFILTER1DPROC __glewCopyConvolutionFilter1D;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCONVOLUTIONFILTER2DPROC __glewCopyConvolutionFilter2D;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEPROC __glewGetColorTable;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEPARAMETERFVPROC __glewGetColorTableParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEPARAMETERIVPROC __glewGetColorTableParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLGETCONVOLUTIONFILTERPROC __glewGetConvolutionFilter;
extern __attribute__ ((visibility("default"))) PFNGLGETCONVOLUTIONPARAMETERFVPROC __glewGetConvolutionParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLGETCONVOLUTIONPARAMETERIVPROC __glewGetConvolutionParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLGETHISTOGRAMPROC __glewGetHistogram;
extern __attribute__ ((visibility("default"))) PFNGLGETHISTOGRAMPARAMETERFVPROC __glewGetHistogramParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLGETHISTOGRAMPARAMETERIVPROC __glewGetHistogramParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLGETMINMAXPROC __glewGetMinmax;
extern __attribute__ ((visibility("default"))) PFNGLGETMINMAXPARAMETERFVPROC __glewGetMinmaxParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLGETMINMAXPARAMETERIVPROC __glewGetMinmaxParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLGETSEPARABLEFILTERPROC __glewGetSeparableFilter;
extern __attribute__ ((visibility("default"))) PFNGLHISTOGRAMPROC __glewHistogram;
extern __attribute__ ((visibility("default"))) PFNGLMINMAXPROC __glewMinmax;
extern __attribute__ ((visibility("default"))) PFNGLRESETHISTOGRAMPROC __glewResetHistogram;
extern __attribute__ ((visibility("default"))) PFNGLRESETMINMAXPROC __glewResetMinmax;
extern __attribute__ ((visibility("default"))) PFNGLSEPARABLEFILTER2DPROC __glewSeparableFilter2D;

extern __attribute__ ((visibility("default"))) PFNGLDRAWARRAYSINSTANCEDARBPROC __glewDrawArraysInstancedARB;
extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTSINSTANCEDARBPROC __glewDrawElementsInstancedARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBDIVISORARBPROC __glewVertexAttribDivisorARB;

extern __attribute__ ((visibility("default"))) PFNGLFLUSHMAPPEDBUFFERRANGEPROC __glewFlushMappedBufferRange;
extern __attribute__ ((visibility("default"))) PFNGLMAPBUFFERRANGEPROC __glewMapBufferRange;

extern __attribute__ ((visibility("default"))) PFNGLCURRENTPALETTEMATRIXARBPROC __glewCurrentPaletteMatrixARB;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXINDEXPOINTERARBPROC __glewMatrixIndexPointerARB;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXINDEXUBVARBPROC __glewMatrixIndexubvARB;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXINDEXUIVARBPROC __glewMatrixIndexuivARB;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXINDEXUSVARBPROC __glewMatrixIndexusvARB;

extern __attribute__ ((visibility("default"))) PFNGLSAMPLECOVERAGEARBPROC __glewSampleCoverageARB;

extern __attribute__ ((visibility("default"))) PFNGLACTIVETEXTUREARBPROC __glewActiveTextureARB;
extern __attribute__ ((visibility("default"))) PFNGLCLIENTACTIVETEXTUREARBPROC __glewClientActiveTextureARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1DARBPROC __glewMultiTexCoord1dARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1DVARBPROC __glewMultiTexCoord1dvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1FARBPROC __glewMultiTexCoord1fARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1FVARBPROC __glewMultiTexCoord1fvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1IARBPROC __glewMultiTexCoord1iARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1IVARBPROC __glewMultiTexCoord1ivARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1SARBPROC __glewMultiTexCoord1sARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1SVARBPROC __glewMultiTexCoord1svARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2DARBPROC __glewMultiTexCoord2dARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2DVARBPROC __glewMultiTexCoord2dvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2FARBPROC __glewMultiTexCoord2fARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2FVARBPROC __glewMultiTexCoord2fvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2IARBPROC __glewMultiTexCoord2iARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2IVARBPROC __glewMultiTexCoord2ivARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2SARBPROC __glewMultiTexCoord2sARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2SVARBPROC __glewMultiTexCoord2svARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3DARBPROC __glewMultiTexCoord3dARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3DVARBPROC __glewMultiTexCoord3dvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3FARBPROC __glewMultiTexCoord3fARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3FVARBPROC __glewMultiTexCoord3fvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3IARBPROC __glewMultiTexCoord3iARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3IVARBPROC __glewMultiTexCoord3ivARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3SARBPROC __glewMultiTexCoord3sARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3SVARBPROC __glewMultiTexCoord3svARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4DARBPROC __glewMultiTexCoord4dARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4DVARBPROC __glewMultiTexCoord4dvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4FARBPROC __glewMultiTexCoord4fARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4FVARBPROC __glewMultiTexCoord4fvARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4IARBPROC __glewMultiTexCoord4iARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4IVARBPROC __glewMultiTexCoord4ivARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4SARBPROC __glewMultiTexCoord4sARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4SVARBPROC __glewMultiTexCoord4svARB;

extern __attribute__ ((visibility("default"))) PFNGLBEGINQUERYARBPROC __glewBeginQueryARB;
extern __attribute__ ((visibility("default"))) PFNGLDELETEQUERIESARBPROC __glewDeleteQueriesARB;
extern __attribute__ ((visibility("default"))) PFNGLENDQUERYARBPROC __glewEndQueryARB;
extern __attribute__ ((visibility("default"))) PFNGLGENQUERIESARBPROC __glewGenQueriesARB;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTIVARBPROC __glewGetQueryObjectivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTUIVARBPROC __glewGetQueryObjectuivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYIVARBPROC __glewGetQueryivARB;
extern __attribute__ ((visibility("default"))) PFNGLISQUERYARBPROC __glewIsQueryARB;

extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERFARBPROC __glewPointParameterfARB;
extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERFVARBPROC __glewPointParameterfvARB;

extern __attribute__ ((visibility("default"))) PFNGLPROVOKINGVERTEXPROC __glewProvokingVertex;

extern __attribute__ ((visibility("default"))) PFNGLGETNCOLORTABLEARBPROC __glewGetnColorTableARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC __glewGetnCompressedTexImageARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNCONVOLUTIONFILTERARBPROC __glewGetnConvolutionFilterARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNHISTOGRAMARBPROC __glewGetnHistogramARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNMAPDVARBPROC __glewGetnMapdvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNMAPFVARBPROC __glewGetnMapfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNMAPIVARBPROC __glewGetnMapivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNMINMAXARBPROC __glewGetnMinmaxARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNPIXELMAPFVARBPROC __glewGetnPixelMapfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNPIXELMAPUIVARBPROC __glewGetnPixelMapuivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNPIXELMAPUSVARBPROC __glewGetnPixelMapusvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNPOLYGONSTIPPLEARBPROC __glewGetnPolygonStippleARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNSEPARABLEFILTERARBPROC __glewGetnSeparableFilterARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNTEXIMAGEARBPROC __glewGetnTexImageARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNUNIFORMDVARBPROC __glewGetnUniformdvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNUNIFORMFVARBPROC __glewGetnUniformfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNUNIFORMIVARBPROC __glewGetnUniformivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNUNIFORMUIVARBPROC __glewGetnUniformuivARB;
extern __attribute__ ((visibility("default"))) PFNGLREADNPIXELSARBPROC __glewReadnPixelsARB;

extern __attribute__ ((visibility("default"))) PFNGLMINSAMPLESHADINGARBPROC __glewMinSampleShadingARB;

extern __attribute__ ((visibility("default"))) PFNGLBINDSAMPLERPROC __glewBindSampler;
extern __attribute__ ((visibility("default"))) PFNGLDELETESAMPLERSPROC __glewDeleteSamplers;
extern __attribute__ ((visibility("default"))) PFNGLGENSAMPLERSPROC __glewGenSamplers;
extern __attribute__ ((visibility("default"))) PFNGLGETSAMPLERPARAMETERIIVPROC __glewGetSamplerParameterIiv;
extern __attribute__ ((visibility("default"))) PFNGLGETSAMPLERPARAMETERIUIVPROC __glewGetSamplerParameterIuiv;
extern __attribute__ ((visibility("default"))) PFNGLGETSAMPLERPARAMETERFVPROC __glewGetSamplerParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLGETSAMPLERPARAMETERIVPROC __glewGetSamplerParameteriv;
extern __attribute__ ((visibility("default"))) PFNGLISSAMPLERPROC __glewIsSampler;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLERPARAMETERIIVPROC __glewSamplerParameterIiv;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLERPARAMETERIUIVPROC __glewSamplerParameterIuiv;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLERPARAMETERFPROC __glewSamplerParameterf;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLERPARAMETERFVPROC __glewSamplerParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLERPARAMETERIPROC __glewSamplerParameteri;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLERPARAMETERIVPROC __glewSamplerParameteriv;

extern __attribute__ ((visibility("default"))) PFNGLACTIVESHADERPROGRAMPROC __glewActiveShaderProgram;
extern __attribute__ ((visibility("default"))) PFNGLBINDPROGRAMPIPELINEPROC __glewBindProgramPipeline;
extern __attribute__ ((visibility("default"))) PFNGLCREATESHADERPROGRAMVPROC __glewCreateShaderProgramv;
extern __attribute__ ((visibility("default"))) PFNGLDELETEPROGRAMPIPELINESPROC __glewDeleteProgramPipelines;
extern __attribute__ ((visibility("default"))) PFNGLGENPROGRAMPIPELINESPROC __glewGenProgramPipelines;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMPIPELINEINFOLOGPROC __glewGetProgramPipelineInfoLog;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMPIPELINEIVPROC __glewGetProgramPipelineiv;
extern __attribute__ ((visibility("default"))) PFNGLISPROGRAMPIPELINEPROC __glewIsProgramPipeline;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1DPROC __glewProgramUniform1d;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1DVPROC __glewProgramUniform1dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1FPROC __glewProgramUniform1f;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1FVPROC __glewProgramUniform1fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1IPROC __glewProgramUniform1i;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1IVPROC __glewProgramUniform1iv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1UIPROC __glewProgramUniform1ui;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1UIVPROC __glewProgramUniform1uiv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2DPROC __glewProgramUniform2d;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2DVPROC __glewProgramUniform2dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2FPROC __glewProgramUniform2f;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2FVPROC __glewProgramUniform2fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2IPROC __glewProgramUniform2i;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2IVPROC __glewProgramUniform2iv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2UIPROC __glewProgramUniform2ui;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2UIVPROC __glewProgramUniform2uiv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3DPROC __glewProgramUniform3d;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3DVPROC __glewProgramUniform3dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3FPROC __glewProgramUniform3f;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3FVPROC __glewProgramUniform3fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3IPROC __glewProgramUniform3i;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3IVPROC __glewProgramUniform3iv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3UIPROC __glewProgramUniform3ui;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3UIVPROC __glewProgramUniform3uiv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4DPROC __glewProgramUniform4d;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4DVPROC __glewProgramUniform4dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4FPROC __glewProgramUniform4f;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4FVPROC __glewProgramUniform4fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4IPROC __glewProgramUniform4i;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4IVPROC __glewProgramUniform4iv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4UIPROC __glewProgramUniform4ui;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4UIVPROC __glewProgramUniform4uiv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2DVPROC __glewProgramUniformMatrix2dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2FVPROC __glewProgramUniformMatrix2fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC __glewProgramUniformMatrix2x3dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC __glewProgramUniformMatrix2x3fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC __glewProgramUniformMatrix2x4dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC __glewProgramUniformMatrix2x4fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3DVPROC __glewProgramUniformMatrix3dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3FVPROC __glewProgramUniformMatrix3fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC __glewProgramUniformMatrix3x2dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC __glewProgramUniformMatrix3x2fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC __glewProgramUniformMatrix3x4dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC __glewProgramUniformMatrix3x4fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4DVPROC __glewProgramUniformMatrix4dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4FVPROC __glewProgramUniformMatrix4fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC __glewProgramUniformMatrix4x2dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC __glewProgramUniformMatrix4x2fv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC __glewProgramUniformMatrix4x3dv;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC __glewProgramUniformMatrix4x3fv;
extern __attribute__ ((visibility("default"))) PFNGLUSEPROGRAMSTAGESPROC __glewUseProgramStages;
extern __attribute__ ((visibility("default"))) PFNGLVALIDATEPROGRAMPIPELINEPROC __glewValidateProgramPipeline;

extern __attribute__ ((visibility("default"))) PFNGLATTACHOBJECTARBPROC __glewAttachObjectARB;
extern __attribute__ ((visibility("default"))) PFNGLCOMPILESHADERARBPROC __glewCompileShaderARB;
extern __attribute__ ((visibility("default"))) PFNGLCREATEPROGRAMOBJECTARBPROC __glewCreateProgramObjectARB;
extern __attribute__ ((visibility("default"))) PFNGLCREATESHADEROBJECTARBPROC __glewCreateShaderObjectARB;
extern __attribute__ ((visibility("default"))) PFNGLDELETEOBJECTARBPROC __glewDeleteObjectARB;
extern __attribute__ ((visibility("default"))) PFNGLDETACHOBJECTARBPROC __glewDetachObjectARB;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEUNIFORMARBPROC __glewGetActiveUniformARB;
extern __attribute__ ((visibility("default"))) PFNGLGETATTACHEDOBJECTSARBPROC __glewGetAttachedObjectsARB;
extern __attribute__ ((visibility("default"))) PFNGLGETHANDLEARBPROC __glewGetHandleARB;
extern __attribute__ ((visibility("default"))) PFNGLGETINFOLOGARBPROC __glewGetInfoLogARB;
extern __attribute__ ((visibility("default"))) PFNGLGETOBJECTPARAMETERFVARBPROC __glewGetObjectParameterfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETOBJECTPARAMETERIVARBPROC __glewGetObjectParameterivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETSHADERSOURCEARBPROC __glewGetShaderSourceARB;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMLOCATIONARBPROC __glewGetUniformLocationARB;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMFVARBPROC __glewGetUniformfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMIVARBPROC __glewGetUniformivARB;
extern __attribute__ ((visibility("default"))) PFNGLLINKPROGRAMARBPROC __glewLinkProgramARB;
extern __attribute__ ((visibility("default"))) PFNGLSHADERSOURCEARBPROC __glewShaderSourceARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1FARBPROC __glewUniform1fARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1FVARBPROC __glewUniform1fvARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1IARBPROC __glewUniform1iARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1IVARBPROC __glewUniform1ivARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2FARBPROC __glewUniform2fARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2FVARBPROC __glewUniform2fvARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2IARBPROC __glewUniform2iARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2IVARBPROC __glewUniform2ivARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3FARBPROC __glewUniform3fARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3FVARBPROC __glewUniform3fvARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3IARBPROC __glewUniform3iARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3IVARBPROC __glewUniform3ivARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4FARBPROC __glewUniform4fARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4FVARBPROC __glewUniform4fvARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4IARBPROC __glewUniform4iARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4IVARBPROC __glewUniform4ivARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX2FVARBPROC __glewUniformMatrix2fvARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX3FVARBPROC __glewUniformMatrix3fvARB;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMMATRIX4FVARBPROC __glewUniformMatrix4fvARB;
extern __attribute__ ((visibility("default"))) PFNGLUSEPROGRAMOBJECTARBPROC __glewUseProgramObjectARB;
extern __attribute__ ((visibility("default"))) PFNGLVALIDATEPROGRAMARBPROC __glewValidateProgramARB;

extern __attribute__ ((visibility("default"))) PFNGLGETACTIVESUBROUTINENAMEPROC __glewGetActiveSubroutineName;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC __glewGetActiveSubroutineUniformName;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC __glewGetActiveSubroutineUniformiv;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMSTAGEIVPROC __glewGetProgramStageiv;
extern __attribute__ ((visibility("default"))) PFNGLGETSUBROUTINEINDEXPROC __glewGetSubroutineIndex;
extern __attribute__ ((visibility("default"))) PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC __glewGetSubroutineUniformLocation;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMSUBROUTINEUIVPROC __glewGetUniformSubroutineuiv;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMSUBROUTINESUIVPROC __glewUniformSubroutinesuiv;

extern __attribute__ ((visibility("default"))) PFNGLCOMPILESHADERINCLUDEARBPROC __glewCompileShaderIncludeARB;
extern __attribute__ ((visibility("default"))) PFNGLDELETENAMEDSTRINGARBPROC __glewDeleteNamedStringARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDSTRINGARBPROC __glewGetNamedStringARB;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDSTRINGIVARBPROC __glewGetNamedStringivARB;
extern __attribute__ ((visibility("default"))) PFNGLISNAMEDSTRINGARBPROC __glewIsNamedStringARB;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDSTRINGARBPROC __glewNamedStringARB;

extern __attribute__ ((visibility("default"))) PFNGLCLIENTWAITSYNCPROC __glewClientWaitSync;
extern __attribute__ ((visibility("default"))) PFNGLDELETESYNCPROC __glewDeleteSync;
extern __attribute__ ((visibility("default"))) PFNGLFENCESYNCPROC __glewFenceSync;
extern __attribute__ ((visibility("default"))) PFNGLGETINTEGER64VPROC __glewGetInteger64v;
extern __attribute__ ((visibility("default"))) PFNGLGETSYNCIVPROC __glewGetSynciv;
extern __attribute__ ((visibility("default"))) PFNGLISSYNCPROC __glewIsSync;
extern __attribute__ ((visibility("default"))) PFNGLWAITSYNCPROC __glewWaitSync;

extern __attribute__ ((visibility("default"))) PFNGLPATCHPARAMETERFVPROC __glewPatchParameterfv;
extern __attribute__ ((visibility("default"))) PFNGLPATCHPARAMETERIPROC __glewPatchParameteri;

extern __attribute__ ((visibility("default"))) PFNGLTEXBUFFERARBPROC __glewTexBufferARB;

extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXIMAGE1DARBPROC __glewCompressedTexImage1DARB;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXIMAGE2DARBPROC __glewCompressedTexImage2DARB;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXIMAGE3DARBPROC __glewCompressedTexImage3DARB;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC __glewCompressedTexSubImage1DARB;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC __glewCompressedTexSubImage2DARB;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC __glewCompressedTexSubImage3DARB;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMPRESSEDTEXIMAGEARBPROC __glewGetCompressedTexImageARB;

extern __attribute__ ((visibility("default"))) PFNGLGETMULTISAMPLEFVPROC __glewGetMultisamplefv;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLEMASKIPROC __glewSampleMaski;
extern __attribute__ ((visibility("default"))) PFNGLTEXIMAGE2DMULTISAMPLEPROC __glewTexImage2DMultisample;
extern __attribute__ ((visibility("default"))) PFNGLTEXIMAGE3DMULTISAMPLEPROC __glewTexImage3DMultisample;

extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTI64VPROC __glewGetQueryObjecti64v;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTUI64VPROC __glewGetQueryObjectui64v;
extern __attribute__ ((visibility("default"))) PFNGLQUERYCOUNTERPROC __glewQueryCounter;

extern __attribute__ ((visibility("default"))) PFNGLBINDTRANSFORMFEEDBACKPROC __glewBindTransformFeedback;
extern __attribute__ ((visibility("default"))) PFNGLDELETETRANSFORMFEEDBACKSPROC __glewDeleteTransformFeedbacks;
extern __attribute__ ((visibility("default"))) PFNGLDRAWTRANSFORMFEEDBACKPROC __glewDrawTransformFeedback;
extern __attribute__ ((visibility("default"))) PFNGLGENTRANSFORMFEEDBACKSPROC __glewGenTransformFeedbacks;
extern __attribute__ ((visibility("default"))) PFNGLISTRANSFORMFEEDBACKPROC __glewIsTransformFeedback;
extern __attribute__ ((visibility("default"))) PFNGLPAUSETRANSFORMFEEDBACKPROC __glewPauseTransformFeedback;
extern __attribute__ ((visibility("default"))) PFNGLRESUMETRANSFORMFEEDBACKPROC __glewResumeTransformFeedback;

extern __attribute__ ((visibility("default"))) PFNGLBEGINQUERYINDEXEDPROC __glewBeginQueryIndexed;
extern __attribute__ ((visibility("default"))) PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC __glewDrawTransformFeedbackStream;
extern __attribute__ ((visibility("default"))) PFNGLENDQUERYINDEXEDPROC __glewEndQueryIndexed;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYINDEXEDIVPROC __glewGetQueryIndexediv;

extern __attribute__ ((visibility("default"))) PFNGLLOADTRANSPOSEMATRIXDARBPROC __glewLoadTransposeMatrixdARB;
extern __attribute__ ((visibility("default"))) PFNGLLOADTRANSPOSEMATRIXFARBPROC __glewLoadTransposeMatrixfARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTTRANSPOSEMATRIXDARBPROC __glewMultTransposeMatrixdARB;
extern __attribute__ ((visibility("default"))) PFNGLMULTTRANSPOSEMATRIXFARBPROC __glewMultTransposeMatrixfARB;

extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERBASEPROC __glewBindBufferBase;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERRANGEPROC __glewBindBufferRange;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC __glewGetActiveUniformBlockName;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEUNIFORMBLOCKIVPROC __glewGetActiveUniformBlockiv;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEUNIFORMNAMEPROC __glewGetActiveUniformName;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEUNIFORMSIVPROC __glewGetActiveUniformsiv;
extern __attribute__ ((visibility("default"))) PFNGLGETINTEGERI_VPROC __glewGetIntegeri_v;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMBLOCKINDEXPROC __glewGetUniformBlockIndex;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMINDICESPROC __glewGetUniformIndices;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMBLOCKBINDINGPROC __glewUniformBlockBinding;

extern __attribute__ ((visibility("default"))) PFNGLBINDVERTEXARRAYPROC __glewBindVertexArray;
extern __attribute__ ((visibility("default"))) PFNGLDELETEVERTEXARRAYSPROC __glewDeleteVertexArrays;
extern __attribute__ ((visibility("default"))) PFNGLGENVERTEXARRAYSPROC __glewGenVertexArrays;
extern __attribute__ ((visibility("default"))) PFNGLISVERTEXARRAYPROC __glewIsVertexArray;

extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBLDVPROC __glewGetVertexAttribLdv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1DPROC __glewVertexAttribL1d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1DVPROC __glewVertexAttribL1dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2DPROC __glewVertexAttribL2d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2DVPROC __glewVertexAttribL2dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3DPROC __glewVertexAttribL3d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3DVPROC __glewVertexAttribL3dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4DPROC __glewVertexAttribL4d;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4DVPROC __glewVertexAttribL4dv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBLPOINTERPROC __glewVertexAttribLPointer;

extern __attribute__ ((visibility("default"))) PFNGLVERTEXBLENDARBPROC __glewVertexBlendARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTPOINTERARBPROC __glewWeightPointerARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTBVARBPROC __glewWeightbvARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTDVARBPROC __glewWeightdvARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTFVARBPROC __glewWeightfvARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTIVARBPROC __glewWeightivARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTSVARBPROC __glewWeightsvARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTUBVARBPROC __glewWeightubvARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTUIVARBPROC __glewWeightuivARB;
extern __attribute__ ((visibility("default"))) PFNGLWEIGHTUSVARBPROC __glewWeightusvARB;

extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERARBPROC __glewBindBufferARB;
extern __attribute__ ((visibility("default"))) PFNGLBUFFERDATAARBPROC __glewBufferDataARB;
extern __attribute__ ((visibility("default"))) PFNGLBUFFERSUBDATAARBPROC __glewBufferSubDataARB;
extern __attribute__ ((visibility("default"))) PFNGLDELETEBUFFERSARBPROC __glewDeleteBuffersARB;
extern __attribute__ ((visibility("default"))) PFNGLGENBUFFERSARBPROC __glewGenBuffersARB;
extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERPARAMETERIVARBPROC __glewGetBufferParameterivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERPOINTERVARBPROC __glewGetBufferPointervARB;
extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERSUBDATAARBPROC __glewGetBufferSubDataARB;
extern __attribute__ ((visibility("default"))) PFNGLISBUFFERARBPROC __glewIsBufferARB;
extern __attribute__ ((visibility("default"))) PFNGLMAPBUFFERARBPROC __glewMapBufferARB;
extern __attribute__ ((visibility("default"))) PFNGLUNMAPBUFFERARBPROC __glewUnmapBufferARB;

extern __attribute__ ((visibility("default"))) PFNGLBINDPROGRAMARBPROC __glewBindProgramARB;
extern __attribute__ ((visibility("default"))) PFNGLDELETEPROGRAMSARBPROC __glewDeleteProgramsARB;
extern __attribute__ ((visibility("default"))) PFNGLDISABLEVERTEXATTRIBARRAYARBPROC __glewDisableVertexAttribArrayARB;
extern __attribute__ ((visibility("default"))) PFNGLENABLEVERTEXATTRIBARRAYARBPROC __glewEnableVertexAttribArrayARB;
extern __attribute__ ((visibility("default"))) PFNGLGENPROGRAMSARBPROC __glewGenProgramsARB;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMENVPARAMETERDVARBPROC __glewGetProgramEnvParameterdvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMENVPARAMETERFVARBPROC __glewGetProgramEnvParameterfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC __glewGetProgramLocalParameterdvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC __glewGetProgramLocalParameterfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMSTRINGARBPROC __glewGetProgramStringARB;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMIVARBPROC __glewGetProgramivARB;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBPOINTERVARBPROC __glewGetVertexAttribPointervARB;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBDVARBPROC __glewGetVertexAttribdvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBFVARBPROC __glewGetVertexAttribfvARB;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBIVARBPROC __glewGetVertexAttribivARB;
extern __attribute__ ((visibility("default"))) PFNGLISPROGRAMARBPROC __glewIsProgramARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETER4DARBPROC __glewProgramEnvParameter4dARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETER4DVARBPROC __glewProgramEnvParameter4dvARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETER4FARBPROC __glewProgramEnvParameter4fARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETER4FVARBPROC __glewProgramEnvParameter4fvARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETER4DARBPROC __glewProgramLocalParameter4dARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETER4DVARBPROC __glewProgramLocalParameter4dvARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETER4FARBPROC __glewProgramLocalParameter4fARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETER4FVARBPROC __glewProgramLocalParameter4fvARB;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMSTRINGARBPROC __glewProgramStringARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1DARBPROC __glewVertexAttrib1dARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1DVARBPROC __glewVertexAttrib1dvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1FARBPROC __glewVertexAttrib1fARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1FVARBPROC __glewVertexAttrib1fvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1SARBPROC __glewVertexAttrib1sARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1SVARBPROC __glewVertexAttrib1svARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2DARBPROC __glewVertexAttrib2dARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2DVARBPROC __glewVertexAttrib2dvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2FARBPROC __glewVertexAttrib2fARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2FVARBPROC __glewVertexAttrib2fvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2SARBPROC __glewVertexAttrib2sARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2SVARBPROC __glewVertexAttrib2svARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3DARBPROC __glewVertexAttrib3dARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3DVARBPROC __glewVertexAttrib3dvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3FARBPROC __glewVertexAttrib3fARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3FVARBPROC __glewVertexAttrib3fvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3SARBPROC __glewVertexAttrib3sARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3SVARBPROC __glewVertexAttrib3svARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NBVARBPROC __glewVertexAttrib4NbvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NIVARBPROC __glewVertexAttrib4NivARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NSVARBPROC __glewVertexAttrib4NsvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUBARBPROC __glewVertexAttrib4NubARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUBVARBPROC __glewVertexAttrib4NubvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUIVARBPROC __glewVertexAttrib4NuivARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4NUSVARBPROC __glewVertexAttrib4NusvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4BVARBPROC __glewVertexAttrib4bvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4DARBPROC __glewVertexAttrib4dARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4DVARBPROC __glewVertexAttrib4dvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4FARBPROC __glewVertexAttrib4fARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4FVARBPROC __glewVertexAttrib4fvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4IVARBPROC __glewVertexAttrib4ivARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4SARBPROC __glewVertexAttrib4sARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4SVARBPROC __glewVertexAttrib4svARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4UBVARBPROC __glewVertexAttrib4ubvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4UIVARBPROC __glewVertexAttrib4uivARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4USVARBPROC __glewVertexAttrib4usvARB;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBPOINTERARBPROC __glewVertexAttribPointerARB;

extern __attribute__ ((visibility("default"))) PFNGLBINDATTRIBLOCATIONARBPROC __glewBindAttribLocationARB;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEATTRIBARBPROC __glewGetActiveAttribARB;
extern __attribute__ ((visibility("default"))) PFNGLGETATTRIBLOCATIONARBPROC __glewGetAttribLocationARB;

extern __attribute__ ((visibility("default"))) PFNGLCOLORP3UIPROC __glewColorP3ui;
extern __attribute__ ((visibility("default"))) PFNGLCOLORP3UIVPROC __glewColorP3uiv;
extern __attribute__ ((visibility("default"))) PFNGLCOLORP4UIPROC __glewColorP4ui;
extern __attribute__ ((visibility("default"))) PFNGLCOLORP4UIVPROC __glewColorP4uiv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP1UIPROC __glewMultiTexCoordP1ui;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP1UIVPROC __glewMultiTexCoordP1uiv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP2UIPROC __glewMultiTexCoordP2ui;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP2UIVPROC __glewMultiTexCoordP2uiv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP3UIPROC __glewMultiTexCoordP3ui;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP3UIVPROC __glewMultiTexCoordP3uiv;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP4UIPROC __glewMultiTexCoordP4ui;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDP4UIVPROC __glewMultiTexCoordP4uiv;
extern __attribute__ ((visibility("default"))) PFNGLNORMALP3UIPROC __glewNormalP3ui;
extern __attribute__ ((visibility("default"))) PFNGLNORMALP3UIVPROC __glewNormalP3uiv;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLORP3UIPROC __glewSecondaryColorP3ui;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLORP3UIVPROC __glewSecondaryColorP3uiv;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP1UIPROC __glewTexCoordP1ui;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP1UIVPROC __glewTexCoordP1uiv;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP2UIPROC __glewTexCoordP2ui;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP2UIVPROC __glewTexCoordP2uiv;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP3UIPROC __glewTexCoordP3ui;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP3UIVPROC __glewTexCoordP3uiv;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP4UIPROC __glewTexCoordP4ui;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDP4UIVPROC __glewTexCoordP4uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP1UIPROC __glewVertexAttribP1ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP1UIVPROC __glewVertexAttribP1uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP2UIPROC __glewVertexAttribP2ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP2UIVPROC __glewVertexAttribP2uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP3UIPROC __glewVertexAttribP3ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP3UIVPROC __glewVertexAttribP3uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP4UIPROC __glewVertexAttribP4ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBP4UIVPROC __glewVertexAttribP4uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXP2UIPROC __glewVertexP2ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXP2UIVPROC __glewVertexP2uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXP3UIPROC __glewVertexP3ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXP3UIVPROC __glewVertexP3uiv;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXP4UIPROC __glewVertexP4ui;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXP4UIVPROC __glewVertexP4uiv;

extern __attribute__ ((visibility("default"))) PFNGLDEPTHRANGEARRAYVPROC __glewDepthRangeArrayv;
extern __attribute__ ((visibility("default"))) PFNGLDEPTHRANGEINDEXEDPROC __glewDepthRangeIndexed;
extern __attribute__ ((visibility("default"))) PFNGLGETDOUBLEI_VPROC __glewGetDoublei_v;
extern __attribute__ ((visibility("default"))) PFNGLGETFLOATI_VPROC __glewGetFloati_v;
extern __attribute__ ((visibility("default"))) PFNGLSCISSORARRAYVPROC __glewScissorArrayv;
extern __attribute__ ((visibility("default"))) PFNGLSCISSORINDEXEDPROC __glewScissorIndexed;
extern __attribute__ ((visibility("default"))) PFNGLSCISSORINDEXEDVPROC __glewScissorIndexedv;
extern __attribute__ ((visibility("default"))) PFNGLVIEWPORTARRAYVPROC __glewViewportArrayv;
extern __attribute__ ((visibility("default"))) PFNGLVIEWPORTINDEXEDFPROC __glewViewportIndexedf;
extern __attribute__ ((visibility("default"))) PFNGLVIEWPORTINDEXEDFVPROC __glewViewportIndexedfv;

extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2DARBPROC __glewWindowPos2dARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2DVARBPROC __glewWindowPos2dvARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2FARBPROC __glewWindowPos2fARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2FVARBPROC __glewWindowPos2fvARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2IARBPROC __glewWindowPos2iARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2IVARBPROC __glewWindowPos2ivARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2SARBPROC __glewWindowPos2sARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2SVARBPROC __glewWindowPos2svARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3DARBPROC __glewWindowPos3dARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3DVARBPROC __glewWindowPos3dvARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3FARBPROC __glewWindowPos3fARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3FVARBPROC __glewWindowPos3fvARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3IARBPROC __glewWindowPos3iARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3IVARBPROC __glewWindowPos3ivARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3SARBPROC __glewWindowPos3sARB;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3SVARBPROC __glewWindowPos3svARB;

extern __attribute__ ((visibility("default"))) PFNGLDRAWBUFFERSATIPROC __glewDrawBuffersATI;

extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTARRAYATIPROC __glewDrawElementArrayATI;
extern __attribute__ ((visibility("default"))) PFNGLDRAWRANGEELEMENTARRAYATIPROC __glewDrawRangeElementArrayATI;
extern __attribute__ ((visibility("default"))) PFNGLELEMENTPOINTERATIPROC __glewElementPointerATI;

extern __attribute__ ((visibility("default"))) PFNGLGETTEXBUMPPARAMETERFVATIPROC __glewGetTexBumpParameterfvATI;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXBUMPPARAMETERIVATIPROC __glewGetTexBumpParameterivATI;
extern __attribute__ ((visibility("default"))) PFNGLTEXBUMPPARAMETERFVATIPROC __glewTexBumpParameterfvATI;
extern __attribute__ ((visibility("default"))) PFNGLTEXBUMPPARAMETERIVATIPROC __glewTexBumpParameterivATI;

extern __attribute__ ((visibility("default"))) PFNGLALPHAFRAGMENTOP1ATIPROC __glewAlphaFragmentOp1ATI;
extern __attribute__ ((visibility("default"))) PFNGLALPHAFRAGMENTOP2ATIPROC __glewAlphaFragmentOp2ATI;
extern __attribute__ ((visibility("default"))) PFNGLALPHAFRAGMENTOP3ATIPROC __glewAlphaFragmentOp3ATI;
extern __attribute__ ((visibility("default"))) PFNGLBEGINFRAGMENTSHADERATIPROC __glewBeginFragmentShaderATI;
extern __attribute__ ((visibility("default"))) PFNGLBINDFRAGMENTSHADERATIPROC __glewBindFragmentShaderATI;
extern __attribute__ ((visibility("default"))) PFNGLCOLORFRAGMENTOP1ATIPROC __glewColorFragmentOp1ATI;
extern __attribute__ ((visibility("default"))) PFNGLCOLORFRAGMENTOP2ATIPROC __glewColorFragmentOp2ATI;
extern __attribute__ ((visibility("default"))) PFNGLCOLORFRAGMENTOP3ATIPROC __glewColorFragmentOp3ATI;
extern __attribute__ ((visibility("default"))) PFNGLDELETEFRAGMENTSHADERATIPROC __glewDeleteFragmentShaderATI;
extern __attribute__ ((visibility("default"))) PFNGLENDFRAGMENTSHADERATIPROC __glewEndFragmentShaderATI;
extern __attribute__ ((visibility("default"))) PFNGLGENFRAGMENTSHADERSATIPROC __glewGenFragmentShadersATI;
extern __attribute__ ((visibility("default"))) PFNGLPASSTEXCOORDATIPROC __glewPassTexCoordATI;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLEMAPATIPROC __glewSampleMapATI;
extern __attribute__ ((visibility("default"))) PFNGLSETFRAGMENTSHADERCONSTANTATIPROC __glewSetFragmentShaderConstantATI;

extern __attribute__ ((visibility("default"))) PFNGLMAPOBJECTBUFFERATIPROC __glewMapObjectBufferATI;
extern __attribute__ ((visibility("default"))) PFNGLUNMAPOBJECTBUFFERATIPROC __glewUnmapObjectBufferATI;

extern __attribute__ ((visibility("default"))) PFNGLPNTRIANGLESFATIPROC __glewPNTrianglesfATI;
extern __attribute__ ((visibility("default"))) PFNGLPNTRIANGLESIATIPROC __glewPNTrianglesiATI;

extern __attribute__ ((visibility("default"))) PFNGLSTENCILFUNCSEPARATEATIPROC __glewStencilFuncSeparateATI;
extern __attribute__ ((visibility("default"))) PFNGLSTENCILOPSEPARATEATIPROC __glewStencilOpSeparateATI;

extern __attribute__ ((visibility("default"))) PFNGLARRAYOBJECTATIPROC __glewArrayObjectATI;
extern __attribute__ ((visibility("default"))) PFNGLFREEOBJECTBUFFERATIPROC __glewFreeObjectBufferATI;
extern __attribute__ ((visibility("default"))) PFNGLGETARRAYOBJECTFVATIPROC __glewGetArrayObjectfvATI;
extern __attribute__ ((visibility("default"))) PFNGLGETARRAYOBJECTIVATIPROC __glewGetArrayObjectivATI;
extern __attribute__ ((visibility("default"))) PFNGLGETOBJECTBUFFERFVATIPROC __glewGetObjectBufferfvATI;
extern __attribute__ ((visibility("default"))) PFNGLGETOBJECTBUFFERIVATIPROC __glewGetObjectBufferivATI;
extern __attribute__ ((visibility("default"))) PFNGLGETVARIANTARRAYOBJECTFVATIPROC __glewGetVariantArrayObjectfvATI;
extern __attribute__ ((visibility("default"))) PFNGLGETVARIANTARRAYOBJECTIVATIPROC __glewGetVariantArrayObjectivATI;
extern __attribute__ ((visibility("default"))) PFNGLISOBJECTBUFFERATIPROC __glewIsObjectBufferATI;
extern __attribute__ ((visibility("default"))) PFNGLNEWOBJECTBUFFERATIPROC __glewNewObjectBufferATI;
extern __attribute__ ((visibility("default"))) PFNGLUPDATEOBJECTBUFFERATIPROC __glewUpdateObjectBufferATI;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTARRAYOBJECTATIPROC __glewVariantArrayObjectATI;

extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC __glewGetVertexAttribArrayObjectfvATI;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC __glewGetVertexAttribArrayObjectivATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBARRAYOBJECTATIPROC __glewVertexAttribArrayObjectATI;

extern __attribute__ ((visibility("default"))) PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC __glewClientActiveVertexStreamATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3BATIPROC __glewNormalStream3bATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3BVATIPROC __glewNormalStream3bvATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3DATIPROC __glewNormalStream3dATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3DVATIPROC __glewNormalStream3dvATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3FATIPROC __glewNormalStream3fATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3FVATIPROC __glewNormalStream3fvATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3IATIPROC __glewNormalStream3iATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3IVATIPROC __glewNormalStream3ivATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3SATIPROC __glewNormalStream3sATI;
extern __attribute__ ((visibility("default"))) PFNGLNORMALSTREAM3SVATIPROC __glewNormalStream3svATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXBLENDENVFATIPROC __glewVertexBlendEnvfATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXBLENDENVIATIPROC __glewVertexBlendEnviATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2DATIPROC __glewVertexStream2dATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2DVATIPROC __glewVertexStream2dvATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2FATIPROC __glewVertexStream2fATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2FVATIPROC __glewVertexStream2fvATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2IATIPROC __glewVertexStream2iATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2IVATIPROC __glewVertexStream2ivATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2SATIPROC __glewVertexStream2sATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM2SVATIPROC __glewVertexStream2svATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3DATIPROC __glewVertexStream3dATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3DVATIPROC __glewVertexStream3dvATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3FATIPROC __glewVertexStream3fATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3FVATIPROC __glewVertexStream3fvATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3IATIPROC __glewVertexStream3iATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3IVATIPROC __glewVertexStream3ivATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3SATIPROC __glewVertexStream3sATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM3SVATIPROC __glewVertexStream3svATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4DATIPROC __glewVertexStream4dATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4DVATIPROC __glewVertexStream4dvATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4FATIPROC __glewVertexStream4fATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4FVATIPROC __glewVertexStream4fvATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4IATIPROC __glewVertexStream4iATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4IVATIPROC __glewVertexStream4ivATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4SATIPROC __glewVertexStream4sATI;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXSTREAM4SVATIPROC __glewVertexStream4svATI;

extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMBUFFERSIZEEXTPROC __glewGetUniformBufferSizeEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMOFFSETEXTPROC __glewGetUniformOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMBUFFEREXTPROC __glewUniformBufferEXT;

extern __attribute__ ((visibility("default"))) PFNGLBLENDCOLOREXTPROC __glewBlendColorEXT;

extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONSEPARATEEXTPROC __glewBlendEquationSeparateEXT;

extern __attribute__ ((visibility("default"))) PFNGLBLENDFUNCSEPARATEEXTPROC __glewBlendFuncSeparateEXT;

extern __attribute__ ((visibility("default"))) PFNGLBLENDEQUATIONEXTPROC __glewBlendEquationEXT;

extern __attribute__ ((visibility("default"))) PFNGLCOLORSUBTABLEEXTPROC __glewColorSubTableEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCOLORSUBTABLEEXTPROC __glewCopyColorSubTableEXT;

extern __attribute__ ((visibility("default"))) PFNGLLOCKARRAYSEXTPROC __glewLockArraysEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNLOCKARRAYSEXTPROC __glewUnlockArraysEXT;

extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONFILTER1DEXTPROC __glewConvolutionFilter1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONFILTER2DEXTPROC __glewConvolutionFilter2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERFEXTPROC __glewConvolutionParameterfEXT;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERFVEXTPROC __glewConvolutionParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERIEXTPROC __glewConvolutionParameteriEXT;
extern __attribute__ ((visibility("default"))) PFNGLCONVOLUTIONPARAMETERIVEXTPROC __glewConvolutionParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC __glewCopyConvolutionFilter1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC __glewCopyConvolutionFilter2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCONVOLUTIONFILTEREXTPROC __glewGetConvolutionFilterEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC __glewGetConvolutionParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC __glewGetConvolutionParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETSEPARABLEFILTEREXTPROC __glewGetSeparableFilterEXT;
extern __attribute__ ((visibility("default"))) PFNGLSEPARABLEFILTER2DEXTPROC __glewSeparableFilter2DEXT;

extern __attribute__ ((visibility("default"))) PFNGLBINORMALPOINTEREXTPROC __glewBinormalPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLTANGENTPOINTEREXTPROC __glewTangentPointerEXT;

extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXIMAGE1DEXTPROC __glewCopyTexImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXIMAGE2DEXTPROC __glewCopyTexImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXSUBIMAGE1DEXTPROC __glewCopyTexSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXSUBIMAGE2DEXTPROC __glewCopyTexSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXSUBIMAGE3DEXTPROC __glewCopyTexSubImage3DEXT;

extern __attribute__ ((visibility("default"))) PFNGLCULLPARAMETERDVEXTPROC __glewCullParameterdvEXT;
extern __attribute__ ((visibility("default"))) PFNGLCULLPARAMETERFVEXTPROC __glewCullParameterfvEXT;

extern __attribute__ ((visibility("default"))) PFNGLDEPTHBOUNDSEXTPROC __glewDepthBoundsEXT;

extern __attribute__ ((visibility("default"))) PFNGLBINDMULTITEXTUREEXTPROC __glewBindMultiTextureEXT;
extern __attribute__ ((visibility("default"))) PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC __glewCheckNamedFramebufferStatusEXT;
extern __attribute__ ((visibility("default"))) PFNGLCLIENTATTRIBDEFAULTEXTPROC __glewClientAttribDefaultEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC __glewCompressedMultiTexImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC __glewCompressedMultiTexImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC __glewCompressedMultiTexImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC __glewCompressedMultiTexSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC __glewCompressedMultiTexSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC __glewCompressedMultiTexSubImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC __glewCompressedTextureImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC __glewCompressedTextureImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC __glewCompressedTextureImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC __glewCompressedTextureSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC __glewCompressedTextureSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC __glewCompressedTextureSubImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYMULTITEXIMAGE1DEXTPROC __glewCopyMultiTexImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYMULTITEXIMAGE2DEXTPROC __glewCopyMultiTexImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC __glewCopyMultiTexSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC __glewCopyMultiTexSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC __glewCopyMultiTexSubImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXTUREIMAGE1DEXTPROC __glewCopyTextureImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXTUREIMAGE2DEXTPROC __glewCopyTextureImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC __glewCopyTextureSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC __glewCopyTextureSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC __glewCopyTextureSubImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC __glewDisableClientStateIndexedEXT;
extern __attribute__ ((visibility("default"))) PFNGLDISABLECLIENTSTATEIEXTPROC __glewDisableClientStateiEXT;
extern __attribute__ ((visibility("default"))) PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC __glewDisableVertexArrayAttribEXT;
extern __attribute__ ((visibility("default"))) PFNGLDISABLEVERTEXARRAYEXTPROC __glewDisableVertexArrayEXT;
extern __attribute__ ((visibility("default"))) PFNGLENABLECLIENTSTATEINDEXEDEXTPROC __glewEnableClientStateIndexedEXT;
extern __attribute__ ((visibility("default"))) PFNGLENABLECLIENTSTATEIEXTPROC __glewEnableClientStateiEXT;
extern __attribute__ ((visibility("default"))) PFNGLENABLEVERTEXARRAYATTRIBEXTPROC __glewEnableVertexArrayAttribEXT;
extern __attribute__ ((visibility("default"))) PFNGLENABLEVERTEXARRAYEXTPROC __glewEnableVertexArrayEXT;
extern __attribute__ ((visibility("default"))) PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC __glewFlushMappedNamedBufferRangeEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC __glewFramebufferDrawBufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC __glewFramebufferDrawBuffersEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERREADBUFFEREXTPROC __glewFramebufferReadBufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENERATEMULTITEXMIPMAPEXTPROC __glewGenerateMultiTexMipmapEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENERATETEXTUREMIPMAPEXTPROC __glewGenerateTextureMipmapEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC __glewGetCompressedMultiTexImageEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC __glewGetCompressedTextureImageEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETDOUBLEINDEXEDVEXTPROC __glewGetDoubleIndexedvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETDOUBLEI_VEXTPROC __glewGetDoublei_vEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFLOATINDEXEDVEXTPROC __glewGetFloatIndexedvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFLOATI_VEXTPROC __glewGetFloati_vEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC __glewGetFramebufferParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXENVFVEXTPROC __glewGetMultiTexEnvfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXENVIVEXTPROC __glewGetMultiTexEnvivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXGENDVEXTPROC __glewGetMultiTexGendvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXGENFVEXTPROC __glewGetMultiTexGenfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXGENIVEXTPROC __glewGetMultiTexGenivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXIMAGEEXTPROC __glewGetMultiTexImageEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC __glewGetMultiTexLevelParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC __glewGetMultiTexLevelParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXPARAMETERIIVEXTPROC __glewGetMultiTexParameterIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXPARAMETERIUIVEXTPROC __glewGetMultiTexParameterIuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXPARAMETERFVEXTPROC __glewGetMultiTexParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMULTITEXPARAMETERIVEXTPROC __glewGetMultiTexParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC __glewGetNamedBufferParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDBUFFERPOINTERVEXTPROC __glewGetNamedBufferPointervEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDBUFFERSUBDATAEXTPROC __glewGetNamedBufferSubDataEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetNamedFramebufferAttachmentParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC __glewGetNamedProgramLocalParameterIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC __glewGetNamedProgramLocalParameterIuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC __glewGetNamedProgramLocalParameterdvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC __glewGetNamedProgramLocalParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDPROGRAMSTRINGEXTPROC __glewGetNamedProgramStringEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDPROGRAMIVEXTPROC __glewGetNamedProgramivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC __glewGetNamedRenderbufferParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETPOINTERINDEXEDVEXTPROC __glewGetPointerIndexedvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETPOINTERI_VEXTPROC __glewGetPointeri_vEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXTUREIMAGEEXTPROC __glewGetTextureImageEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC __glewGetTextureLevelParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC __glewGetTextureLevelParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXTUREPARAMETERIIVEXTPROC __glewGetTextureParameterIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXTUREPARAMETERIUIVEXTPROC __glewGetTextureParameterIuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXTUREPARAMETERFVEXTPROC __glewGetTextureParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXTUREPARAMETERIVEXTPROC __glewGetTextureParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC __glewGetVertexArrayIntegeri_vEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXARRAYINTEGERVEXTPROC __glewGetVertexArrayIntegervEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC __glewGetVertexArrayPointeri_vEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXARRAYPOINTERVEXTPROC __glewGetVertexArrayPointervEXT;
extern __attribute__ ((visibility("default"))) PFNGLMAPNAMEDBUFFEREXTPROC __glewMapNamedBufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLMAPNAMEDBUFFERRANGEEXTPROC __glewMapNamedBufferRangeEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXFRUSTUMEXTPROC __glewMatrixFrustumEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXLOADIDENTITYEXTPROC __glewMatrixLoadIdentityEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXLOADTRANSPOSEDEXTPROC __glewMatrixLoadTransposedEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXLOADTRANSPOSEFEXTPROC __glewMatrixLoadTransposefEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXLOADDEXTPROC __glewMatrixLoaddEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXLOADFEXTPROC __glewMatrixLoadfEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXMULTTRANSPOSEDEXTPROC __glewMatrixMultTransposedEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXMULTTRANSPOSEFEXTPROC __glewMatrixMultTransposefEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXMULTDEXTPROC __glewMatrixMultdEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXMULTFEXTPROC __glewMatrixMultfEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXORTHOEXTPROC __glewMatrixOrthoEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXPOPEXTPROC __glewMatrixPopEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXPUSHEXTPROC __glewMatrixPushEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXROTATEDEXTPROC __glewMatrixRotatedEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXROTATEFEXTPROC __glewMatrixRotatefEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXSCALEDEXTPROC __glewMatrixScaledEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXSCALEFEXTPROC __glewMatrixScalefEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXTRANSLATEDEXTPROC __glewMatrixTranslatedEXT;
extern __attribute__ ((visibility("default"))) PFNGLMATRIXTRANSLATEFEXTPROC __glewMatrixTranslatefEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXBUFFEREXTPROC __glewMultiTexBufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORDPOINTEREXTPROC __glewMultiTexCoordPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXENVFEXTPROC __glewMultiTexEnvfEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXENVFVEXTPROC __glewMultiTexEnvfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXENVIEXTPROC __glewMultiTexEnviEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXENVIVEXTPROC __glewMultiTexEnvivEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXGENDEXTPROC __glewMultiTexGendEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXGENDVEXTPROC __glewMultiTexGendvEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXGENFEXTPROC __glewMultiTexGenfEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXGENFVEXTPROC __glewMultiTexGenfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXGENIEXTPROC __glewMultiTexGeniEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXGENIVEXTPROC __glewMultiTexGenivEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXIMAGE1DEXTPROC __glewMultiTexImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXIMAGE2DEXTPROC __glewMultiTexImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXIMAGE3DEXTPROC __glewMultiTexImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXPARAMETERIIVEXTPROC __glewMultiTexParameterIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXPARAMETERIUIVEXTPROC __glewMultiTexParameterIuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXPARAMETERFEXTPROC __glewMultiTexParameterfEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXPARAMETERFVEXTPROC __glewMultiTexParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXPARAMETERIEXTPROC __glewMultiTexParameteriEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXPARAMETERIVEXTPROC __glewMultiTexParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXRENDERBUFFEREXTPROC __glewMultiTexRenderbufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXSUBIMAGE1DEXTPROC __glewMultiTexSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXSUBIMAGE2DEXTPROC __glewMultiTexSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXSUBIMAGE3DEXTPROC __glewMultiTexSubImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDBUFFERDATAEXTPROC __glewNamedBufferDataEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDBUFFERSUBDATAEXTPROC __glewNamedBufferSubDataEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC __glewNamedCopyBufferSubDataEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC __glewNamedFramebufferRenderbufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC __glewNamedFramebufferTexture1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC __glewNamedFramebufferTexture2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC __glewNamedFramebufferTexture3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC __glewNamedFramebufferTextureEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC __glewNamedFramebufferTextureFaceEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC __glewNamedFramebufferTextureLayerEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC __glewNamedProgramLocalParameter4dEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC __glewNamedProgramLocalParameter4dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC __glewNamedProgramLocalParameter4fEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC __glewNamedProgramLocalParameter4fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC __glewNamedProgramLocalParameterI4iEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC __glewNamedProgramLocalParameterI4ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC __glewNamedProgramLocalParameterI4uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC __glewNamedProgramLocalParameterI4uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC __glewNamedProgramLocalParameters4fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC __glewNamedProgramLocalParametersI4ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC __glewNamedProgramLocalParametersI4uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDPROGRAMSTRINGEXTPROC __glewNamedProgramStringEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC __glewNamedRenderbufferStorageEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC __glewNamedRenderbufferStorageMultisampleCoverageEXT;
extern __attribute__ ((visibility("default"))) PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewNamedRenderbufferStorageMultisampleEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1FEXTPROC __glewProgramUniform1fEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1FVEXTPROC __glewProgramUniform1fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1IEXTPROC __glewProgramUniform1iEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1IVEXTPROC __glewProgramUniform1ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1UIEXTPROC __glewProgramUniform1uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1UIVEXTPROC __glewProgramUniform1uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2FEXTPROC __glewProgramUniform2fEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2FVEXTPROC __glewProgramUniform2fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2IEXTPROC __glewProgramUniform2iEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2IVEXTPROC __glewProgramUniform2ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2UIEXTPROC __glewProgramUniform2uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2UIVEXTPROC __glewProgramUniform2uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3FEXTPROC __glewProgramUniform3fEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3FVEXTPROC __glewProgramUniform3fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3IEXTPROC __glewProgramUniform3iEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3IVEXTPROC __glewProgramUniform3ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3UIEXTPROC __glewProgramUniform3uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3UIVEXTPROC __glewProgramUniform3uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4FEXTPROC __glewProgramUniform4fEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4FVEXTPROC __glewProgramUniform4fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4IEXTPROC __glewProgramUniform4iEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4IVEXTPROC __glewProgramUniform4ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4UIEXTPROC __glewProgramUniform4uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4UIVEXTPROC __glewProgramUniform4uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC __glewProgramUniformMatrix2fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC __glewProgramUniformMatrix2x3fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC __glewProgramUniformMatrix2x4fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC __glewProgramUniformMatrix3fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC __glewProgramUniformMatrix3x2fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC __glewProgramUniformMatrix3x4fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC __glewProgramUniformMatrix4fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC __glewProgramUniformMatrix4x2fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC __glewProgramUniformMatrix4x3fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC __glewPushClientAttribDefaultEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREBUFFEREXTPROC __glewTextureBufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREIMAGE1DEXTPROC __glewTextureImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREIMAGE2DEXTPROC __glewTextureImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREIMAGE3DEXTPROC __glewTextureImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREPARAMETERIIVEXTPROC __glewTextureParameterIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREPARAMETERIUIVEXTPROC __glewTextureParameterIuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREPARAMETERFEXTPROC __glewTextureParameterfEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREPARAMETERFVEXTPROC __glewTextureParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREPARAMETERIEXTPROC __glewTextureParameteriEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREPARAMETERIVEXTPROC __glewTextureParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTURERENDERBUFFEREXTPROC __glewTextureRenderbufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTURESUBIMAGE1DEXTPROC __glewTextureSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTURESUBIMAGE2DEXTPROC __glewTextureSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTURESUBIMAGE3DEXTPROC __glewTextureSubImage3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNMAPNAMEDBUFFEREXTPROC __glewUnmapNamedBufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYCOLOROFFSETEXTPROC __glewVertexArrayColorOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC __glewVertexArrayEdgeFlagOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC __glewVertexArrayFogCoordOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYINDEXOFFSETEXTPROC __glewVertexArrayIndexOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC __glewVertexArrayMultiTexCoordOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYNORMALOFFSETEXTPROC __glewVertexArrayNormalOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC __glewVertexArraySecondaryColorOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC __glewVertexArrayTexCoordOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC __glewVertexArrayVertexAttribIOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC __glewVertexArrayVertexAttribOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC __glewVertexArrayVertexOffsetEXT;

extern __attribute__ ((visibility("default"))) PFNGLCOLORMASKINDEXEDEXTPROC __glewColorMaskIndexedEXT;
extern __attribute__ ((visibility("default"))) PFNGLDISABLEINDEXEDEXTPROC __glewDisableIndexedEXT;
extern __attribute__ ((visibility("default"))) PFNGLENABLEINDEXEDEXTPROC __glewEnableIndexedEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETBOOLEANINDEXEDVEXTPROC __glewGetBooleanIndexedvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETINTEGERINDEXEDVEXTPROC __glewGetIntegerIndexedvEXT;
extern __attribute__ ((visibility("default"))) PFNGLISENABLEDINDEXEDEXTPROC __glewIsEnabledIndexedEXT;

extern __attribute__ ((visibility("default"))) PFNGLDRAWARRAYSINSTANCEDEXTPROC __glewDrawArraysInstancedEXT;
extern __attribute__ ((visibility("default"))) PFNGLDRAWELEMENTSINSTANCEDEXTPROC __glewDrawElementsInstancedEXT;

extern __attribute__ ((visibility("default"))) PFNGLDRAWRANGEELEMENTSEXTPROC __glewDrawRangeElementsEXT;

extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDPOINTEREXTPROC __glewFogCoordPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDDEXTPROC __glewFogCoorddEXT;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDDVEXTPROC __glewFogCoorddvEXT;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDFEXTPROC __glewFogCoordfEXT;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDFVEXTPROC __glewFogCoordfvEXT;

extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTCOLORMATERIALEXTPROC __glewFragmentColorMaterialEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELFEXTPROC __glewFragmentLightModelfEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELFVEXTPROC __glewFragmentLightModelfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELIEXTPROC __glewFragmentLightModeliEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELIVEXTPROC __glewFragmentLightModelivEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTFEXTPROC __glewFragmentLightfEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTFVEXTPROC __glewFragmentLightfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTIEXTPROC __glewFragmentLightiEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTIVEXTPROC __glewFragmentLightivEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALFEXTPROC __glewFragmentMaterialfEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALFVEXTPROC __glewFragmentMaterialfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALIEXTPROC __glewFragmentMaterialiEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALIVEXTPROC __glewFragmentMaterialivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTLIGHTFVEXTPROC __glewGetFragmentLightfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTLIGHTIVEXTPROC __glewGetFragmentLightivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTMATERIALFVEXTPROC __glewGetFragmentMaterialfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTMATERIALIVEXTPROC __glewGetFragmentMaterialivEXT;
extern __attribute__ ((visibility("default"))) PFNGLLIGHTENVIEXTPROC __glewLightEnviEXT;

extern __attribute__ ((visibility("default"))) PFNGLBLITFRAMEBUFFEREXTPROC __glewBlitFramebufferEXT;

extern __attribute__ ((visibility("default"))) PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewRenderbufferStorageMultisampleEXT;

extern __attribute__ ((visibility("default"))) PFNGLBINDFRAMEBUFFEREXTPROC __glewBindFramebufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDRENDERBUFFEREXTPROC __glewBindRenderbufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC __glewCheckFramebufferStatusEXT;
extern __attribute__ ((visibility("default"))) PFNGLDELETEFRAMEBUFFERSEXTPROC __glewDeleteFramebuffersEXT;
extern __attribute__ ((visibility("default"))) PFNGLDELETERENDERBUFFERSEXTPROC __glewDeleteRenderbuffersEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC __glewFramebufferRenderbufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURE1DEXTPROC __glewFramebufferTexture1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURE2DEXTPROC __glewFramebufferTexture2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURE3DEXTPROC __glewFramebufferTexture3DEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENFRAMEBUFFERSEXTPROC __glewGenFramebuffersEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENRENDERBUFFERSEXTPROC __glewGenRenderbuffersEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENERATEMIPMAPEXTPROC __glewGenerateMipmapEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetFramebufferAttachmentParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC __glewGetRenderbufferParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLISFRAMEBUFFEREXTPROC __glewIsFramebufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLISRENDERBUFFEREXTPROC __glewIsRenderbufferEXT;
extern __attribute__ ((visibility("default"))) PFNGLRENDERBUFFERSTORAGEEXTPROC __glewRenderbufferStorageEXT;

extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTUREEXTPROC __glewFramebufferTextureEXT;
extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC __glewFramebufferTextureFaceEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETERIEXTPROC __glewProgramParameteriEXT;

extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETERS4FVEXTPROC __glewProgramEnvParameters4fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC __glewProgramLocalParameters4fvEXT;

extern __attribute__ ((visibility("default"))) PFNGLBINDFRAGDATALOCATIONEXTPROC __glewBindFragDataLocationEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGDATALOCATIONEXTPROC __glewGetFragDataLocationEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMUIVEXTPROC __glewGetUniformuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBIIVEXTPROC __glewGetVertexAttribIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBIUIVEXTPROC __glewGetVertexAttribIuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1UIEXTPROC __glewUniform1uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1UIVEXTPROC __glewUniform1uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2UIEXTPROC __glewUniform2uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2UIVEXTPROC __glewUniform2uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3UIEXTPROC __glewUniform3uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3UIVEXTPROC __glewUniform3uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4UIEXTPROC __glewUniform4uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4UIVEXTPROC __glewUniform4uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1IEXTPROC __glewVertexAttribI1iEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1IVEXTPROC __glewVertexAttribI1ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1UIEXTPROC __glewVertexAttribI1uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI1UIVEXTPROC __glewVertexAttribI1uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2IEXTPROC __glewVertexAttribI2iEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2IVEXTPROC __glewVertexAttribI2ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2UIEXTPROC __glewVertexAttribI2uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI2UIVEXTPROC __glewVertexAttribI2uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3IEXTPROC __glewVertexAttribI3iEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3IVEXTPROC __glewVertexAttribI3ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3UIEXTPROC __glewVertexAttribI3uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI3UIVEXTPROC __glewVertexAttribI3uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4BVEXTPROC __glewVertexAttribI4bvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4IEXTPROC __glewVertexAttribI4iEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4IVEXTPROC __glewVertexAttribI4ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4SVEXTPROC __glewVertexAttribI4svEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4UBVEXTPROC __glewVertexAttribI4ubvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4UIEXTPROC __glewVertexAttribI4uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4UIVEXTPROC __glewVertexAttribI4uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBI4USVEXTPROC __glewVertexAttribI4usvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBIPOINTEREXTPROC __glewVertexAttribIPointerEXT;

extern __attribute__ ((visibility("default"))) PFNGLGETHISTOGRAMEXTPROC __glewGetHistogramEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETHISTOGRAMPARAMETERFVEXTPROC __glewGetHistogramParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETHISTOGRAMPARAMETERIVEXTPROC __glewGetHistogramParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMINMAXEXTPROC __glewGetMinmaxEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMINMAXPARAMETERFVEXTPROC __glewGetMinmaxParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETMINMAXPARAMETERIVEXTPROC __glewGetMinmaxParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLHISTOGRAMEXTPROC __glewHistogramEXT;
extern __attribute__ ((visibility("default"))) PFNGLMINMAXEXTPROC __glewMinmaxEXT;
extern __attribute__ ((visibility("default"))) PFNGLRESETHISTOGRAMEXTPROC __glewResetHistogramEXT;
extern __attribute__ ((visibility("default"))) PFNGLRESETMINMAXEXTPROC __glewResetMinmaxEXT;

extern __attribute__ ((visibility("default"))) PFNGLINDEXFUNCEXTPROC __glewIndexFuncEXT;

extern __attribute__ ((visibility("default"))) PFNGLINDEXMATERIALEXTPROC __glewIndexMaterialEXT;

extern __attribute__ ((visibility("default"))) PFNGLAPPLYTEXTUREEXTPROC __glewApplyTextureEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTURELIGHTEXTPROC __glewTextureLightEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREMATERIALEXTPROC __glewTextureMaterialEXT;

extern __attribute__ ((visibility("default"))) PFNGLMULTIDRAWARRAYSEXTPROC __glewMultiDrawArraysEXT;
extern __attribute__ ((visibility("default"))) PFNGLMULTIDRAWELEMENTSEXTPROC __glewMultiDrawElementsEXT;

extern __attribute__ ((visibility("default"))) PFNGLSAMPLEMASKEXTPROC __glewSampleMaskEXT;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLEPATTERNEXTPROC __glewSamplePatternEXT;

extern __attribute__ ((visibility("default"))) PFNGLCOLORTABLEEXTPROC __glewColorTableEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEEXTPROC __glewGetColorTableEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEPARAMETERFVEXTPROC __glewGetColorTableParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEPARAMETERIVEXTPROC __glewGetColorTableParameterivEXT;

extern __attribute__ ((visibility("default"))) PFNGLGETPIXELTRANSFORMPARAMETERFVEXTPROC __glewGetPixelTransformParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETPIXELTRANSFORMPARAMETERIVEXTPROC __glewGetPixelTransformParameterivEXT;
extern __attribute__ ((visibility("default"))) PFNGLPIXELTRANSFORMPARAMETERFEXTPROC __glewPixelTransformParameterfEXT;
extern __attribute__ ((visibility("default"))) PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC __glewPixelTransformParameterfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLPIXELTRANSFORMPARAMETERIEXTPROC __glewPixelTransformParameteriEXT;
extern __attribute__ ((visibility("default"))) PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC __glewPixelTransformParameterivEXT;

extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERFEXTPROC __glewPointParameterfEXT;
extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERFVEXTPROC __glewPointParameterfvEXT;

extern __attribute__ ((visibility("default"))) PFNGLPOLYGONOFFSETEXTPROC __glewPolygonOffsetEXT;

extern __attribute__ ((visibility("default"))) PFNGLPROVOKINGVERTEXEXTPROC __glewProvokingVertexEXT;

extern __attribute__ ((visibility("default"))) PFNGLBEGINSCENEEXTPROC __glewBeginSceneEXT;
extern __attribute__ ((visibility("default"))) PFNGLENDSCENEEXTPROC __glewEndSceneEXT;

extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3BEXTPROC __glewSecondaryColor3bEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3BVEXTPROC __glewSecondaryColor3bvEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3DEXTPROC __glewSecondaryColor3dEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3DVEXTPROC __glewSecondaryColor3dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3FEXTPROC __glewSecondaryColor3fEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3FVEXTPROC __glewSecondaryColor3fvEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3IEXTPROC __glewSecondaryColor3iEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3IVEXTPROC __glewSecondaryColor3ivEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3SEXTPROC __glewSecondaryColor3sEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3SVEXTPROC __glewSecondaryColor3svEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UBEXTPROC __glewSecondaryColor3ubEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UBVEXTPROC __glewSecondaryColor3ubvEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UIEXTPROC __glewSecondaryColor3uiEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3UIVEXTPROC __glewSecondaryColor3uivEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3USEXTPROC __glewSecondaryColor3usEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3USVEXTPROC __glewSecondaryColor3usvEXT;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLORPOINTEREXTPROC __glewSecondaryColorPointerEXT;

extern __attribute__ ((visibility("default"))) PFNGLACTIVEPROGRAMEXTPROC __glewActiveProgramEXT;
extern __attribute__ ((visibility("default"))) PFNGLCREATESHADERPROGRAMEXTPROC __glewCreateShaderProgramEXT;
extern __attribute__ ((visibility("default"))) PFNGLUSESHADERPROGRAMEXTPROC __glewUseShaderProgramEXT;

extern __attribute__ ((visibility("default"))) PFNGLBINDIMAGETEXTUREEXTPROC __glewBindImageTextureEXT;
extern __attribute__ ((visibility("default"))) PFNGLMEMORYBARRIEREXTPROC __glewMemoryBarrierEXT;

extern __attribute__ ((visibility("default"))) PFNGLACTIVESTENCILFACEEXTPROC __glewActiveStencilFaceEXT;

extern __attribute__ ((visibility("default"))) PFNGLTEXSUBIMAGE1DEXTPROC __glewTexSubImage1DEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXSUBIMAGE2DEXTPROC __glewTexSubImage2DEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXSUBIMAGE3DEXTPROC __glewTexSubImage3DEXT;

extern __attribute__ ((visibility("default"))) PFNGLTEXIMAGE3DEXTPROC __glewTexImage3DEXT;

extern __attribute__ ((visibility("default"))) PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC __glewFramebufferTextureLayerEXT;

extern __attribute__ ((visibility("default"))) PFNGLTEXBUFFEREXTPROC __glewTexBufferEXT;

extern __attribute__ ((visibility("default"))) PFNGLCLEARCOLORIIEXTPROC __glewClearColorIiEXT;
extern __attribute__ ((visibility("default"))) PFNGLCLEARCOLORIUIEXTPROC __glewClearColorIuiEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXPARAMETERIIVEXTPROC __glewGetTexParameterIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTEXPARAMETERIUIVEXTPROC __glewGetTexParameterIuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXPARAMETERIIVEXTPROC __glewTexParameterIivEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXPARAMETERIUIVEXTPROC __glewTexParameterIuivEXT;

extern __attribute__ ((visibility("default"))) PFNGLARETEXTURESRESIDENTEXTPROC __glewAreTexturesResidentEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDTEXTUREEXTPROC __glewBindTextureEXT;
extern __attribute__ ((visibility("default"))) PFNGLDELETETEXTURESEXTPROC __glewDeleteTexturesEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENTEXTURESEXTPROC __glewGenTexturesEXT;
extern __attribute__ ((visibility("default"))) PFNGLISTEXTUREEXTPROC __glewIsTextureEXT;
extern __attribute__ ((visibility("default"))) PFNGLPRIORITIZETEXTURESEXTPROC __glewPrioritizeTexturesEXT;

extern __attribute__ ((visibility("default"))) PFNGLTEXTURENORMALEXTPROC __glewTextureNormalEXT;

extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTI64VEXTPROC __glewGetQueryObjecti64vEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETQUERYOBJECTUI64VEXTPROC __glewGetQueryObjectui64vEXT;

extern __attribute__ ((visibility("default"))) PFNGLBEGINTRANSFORMFEEDBACKEXTPROC __glewBeginTransformFeedbackEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERBASEEXTPROC __glewBindBufferBaseEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFEROFFSETEXTPROC __glewBindBufferOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERRANGEEXTPROC __glewBindBufferRangeEXT;
extern __attribute__ ((visibility("default"))) PFNGLENDTRANSFORMFEEDBACKEXTPROC __glewEndTransformFeedbackEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC __glewGetTransformFeedbackVaryingEXT;
extern __attribute__ ((visibility("default"))) PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC __glewTransformFeedbackVaryingsEXT;

extern __attribute__ ((visibility("default"))) PFNGLARRAYELEMENTEXTPROC __glewArrayElementEXT;
extern __attribute__ ((visibility("default"))) PFNGLCOLORPOINTEREXTPROC __glewColorPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLDRAWARRAYSEXTPROC __glewDrawArraysEXT;
extern __attribute__ ((visibility("default"))) PFNGLEDGEFLAGPOINTEREXTPROC __glewEdgeFlagPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLINDEXPOINTEREXTPROC __glewIndexPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLNORMALPOINTEREXTPROC __glewNormalPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDPOINTEREXTPROC __glewTexCoordPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXPOINTEREXTPROC __glewVertexPointerEXT;

extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBLDVEXTPROC __glewGetVertexAttribLdvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC __glewVertexArrayVertexAttribLOffsetEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1DEXTPROC __glewVertexAttribL1dEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1DVEXTPROC __glewVertexAttribL1dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2DEXTPROC __glewVertexAttribL2dEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2DVEXTPROC __glewVertexAttribL2dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3DEXTPROC __glewVertexAttribL3dEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3DVEXTPROC __glewVertexAttribL3dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4DEXTPROC __glewVertexAttribL4dEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4DVEXTPROC __glewVertexAttribL4dvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBLPOINTEREXTPROC __glewVertexAttribLPointerEXT;

extern __attribute__ ((visibility("default"))) PFNGLBEGINVERTEXSHADEREXTPROC __glewBeginVertexShaderEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDLIGHTPARAMETEREXTPROC __glewBindLightParameterEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDMATERIALPARAMETEREXTPROC __glewBindMaterialParameterEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDPARAMETEREXTPROC __glewBindParameterEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDTEXGENPARAMETEREXTPROC __glewBindTexGenParameterEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDTEXTUREUNITPARAMETEREXTPROC __glewBindTextureUnitParameterEXT;
extern __attribute__ ((visibility("default"))) PFNGLBINDVERTEXSHADEREXTPROC __glewBindVertexShaderEXT;
extern __attribute__ ((visibility("default"))) PFNGLDELETEVERTEXSHADEREXTPROC __glewDeleteVertexShaderEXT;
extern __attribute__ ((visibility("default"))) PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC __glewDisableVariantClientStateEXT;
extern __attribute__ ((visibility("default"))) PFNGLENABLEVARIANTCLIENTSTATEEXTPROC __glewEnableVariantClientStateEXT;
extern __attribute__ ((visibility("default"))) PFNGLENDVERTEXSHADEREXTPROC __glewEndVertexShaderEXT;
extern __attribute__ ((visibility("default"))) PFNGLEXTRACTCOMPONENTEXTPROC __glewExtractComponentEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENSYMBOLSEXTPROC __glewGenSymbolsEXT;
extern __attribute__ ((visibility("default"))) PFNGLGENVERTEXSHADERSEXTPROC __glewGenVertexShadersEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETINVARIANTBOOLEANVEXTPROC __glewGetInvariantBooleanvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETINVARIANTFLOATVEXTPROC __glewGetInvariantFloatvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETINVARIANTINTEGERVEXTPROC __glewGetInvariantIntegervEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC __glewGetLocalConstantBooleanvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETLOCALCONSTANTFLOATVEXTPROC __glewGetLocalConstantFloatvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETLOCALCONSTANTINTEGERVEXTPROC __glewGetLocalConstantIntegervEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVARIANTBOOLEANVEXTPROC __glewGetVariantBooleanvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVARIANTFLOATVEXTPROC __glewGetVariantFloatvEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVARIANTINTEGERVEXTPROC __glewGetVariantIntegervEXT;
extern __attribute__ ((visibility("default"))) PFNGLGETVARIANTPOINTERVEXTPROC __glewGetVariantPointervEXT;
extern __attribute__ ((visibility("default"))) PFNGLINSERTCOMPONENTEXTPROC __glewInsertComponentEXT;
extern __attribute__ ((visibility("default"))) PFNGLISVARIANTENABLEDEXTPROC __glewIsVariantEnabledEXT;
extern __attribute__ ((visibility("default"))) PFNGLSETINVARIANTEXTPROC __glewSetInvariantEXT;
extern __attribute__ ((visibility("default"))) PFNGLSETLOCALCONSTANTEXTPROC __glewSetLocalConstantEXT;
extern __attribute__ ((visibility("default"))) PFNGLSHADEROP1EXTPROC __glewShaderOp1EXT;
extern __attribute__ ((visibility("default"))) PFNGLSHADEROP2EXTPROC __glewShaderOp2EXT;
extern __attribute__ ((visibility("default"))) PFNGLSHADEROP3EXTPROC __glewShaderOp3EXT;
extern __attribute__ ((visibility("default"))) PFNGLSWIZZLEEXTPROC __glewSwizzleEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTPOINTEREXTPROC __glewVariantPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTBVEXTPROC __glewVariantbvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTDVEXTPROC __glewVariantdvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTFVEXTPROC __glewVariantfvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTIVEXTPROC __glewVariantivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTSVEXTPROC __glewVariantsvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTUBVEXTPROC __glewVariantubvEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTUIVEXTPROC __glewVariantuivEXT;
extern __attribute__ ((visibility("default"))) PFNGLVARIANTUSVEXTPROC __glewVariantusvEXT;
extern __attribute__ ((visibility("default"))) PFNGLWRITEMASKEXTPROC __glewWriteMaskEXT;

extern __attribute__ ((visibility("default"))) PFNGLVERTEXWEIGHTPOINTEREXTPROC __glewVertexWeightPointerEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXWEIGHTFEXTPROC __glewVertexWeightfEXT;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXWEIGHTFVEXTPROC __glewVertexWeightfvEXT;

extern __attribute__ ((visibility("default"))) PFNGLIMPORTSYNCEXTPROC __glewImportSyncEXT;

extern __attribute__ ((visibility("default"))) PFNGLFRAMETERMINATORGREMEDYPROC __glewFrameTerminatorGREMEDY;

extern __attribute__ ((visibility("default"))) PFNGLSTRINGMARKERGREMEDYPROC __glewStringMarkerGREMEDY;

extern __attribute__ ((visibility("default"))) PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC __glewGetImageTransformParameterfvHP;
extern __attribute__ ((visibility("default"))) PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC __glewGetImageTransformParameterivHP;
extern __attribute__ ((visibility("default"))) PFNGLIMAGETRANSFORMPARAMETERFHPPROC __glewImageTransformParameterfHP;
extern __attribute__ ((visibility("default"))) PFNGLIMAGETRANSFORMPARAMETERFVHPPROC __glewImageTransformParameterfvHP;
extern __attribute__ ((visibility("default"))) PFNGLIMAGETRANSFORMPARAMETERIHPPROC __glewImageTransformParameteriHP;
extern __attribute__ ((visibility("default"))) PFNGLIMAGETRANSFORMPARAMETERIVHPPROC __glewImageTransformParameterivHP;

extern __attribute__ ((visibility("default"))) PFNGLMULTIMODEDRAWARRAYSIBMPROC __glewMultiModeDrawArraysIBM;
extern __attribute__ ((visibility("default"))) PFNGLMULTIMODEDRAWELEMENTSIBMPROC __glewMultiModeDrawElementsIBM;

extern __attribute__ ((visibility("default"))) PFNGLCOLORPOINTERLISTIBMPROC __glewColorPointerListIBM;
extern __attribute__ ((visibility("default"))) PFNGLEDGEFLAGPOINTERLISTIBMPROC __glewEdgeFlagPointerListIBM;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDPOINTERLISTIBMPROC __glewFogCoordPointerListIBM;
extern __attribute__ ((visibility("default"))) PFNGLINDEXPOINTERLISTIBMPROC __glewIndexPointerListIBM;
extern __attribute__ ((visibility("default"))) PFNGLNORMALPOINTERLISTIBMPROC __glewNormalPointerListIBM;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLORPOINTERLISTIBMPROC __glewSecondaryColorPointerListIBM;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDPOINTERLISTIBMPROC __glewTexCoordPointerListIBM;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXPOINTERLISTIBMPROC __glewVertexPointerListIBM;

extern __attribute__ ((visibility("default"))) PFNGLCOLORPOINTERVINTELPROC __glewColorPointervINTEL;
extern __attribute__ ((visibility("default"))) PFNGLNORMALPOINTERVINTELPROC __glewNormalPointervINTEL;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDPOINTERVINTELPROC __glewTexCoordPointervINTEL;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXPOINTERVINTELPROC __glewVertexPointervINTEL;

extern __attribute__ ((visibility("default"))) PFNGLTEXSCISSORFUNCINTELPROC __glewTexScissorFuncINTEL;
extern __attribute__ ((visibility("default"))) PFNGLTEXSCISSORINTELPROC __glewTexScissorINTEL;

extern __attribute__ ((visibility("default"))) PFNGLBUFFERREGIONENABLEDEXTPROC __glewBufferRegionEnabledEXT;
extern __attribute__ ((visibility("default"))) PFNGLDELETEBUFFERREGIONEXTPROC __glewDeleteBufferRegionEXT;
extern __attribute__ ((visibility("default"))) PFNGLDRAWBUFFERREGIONEXTPROC __glewDrawBufferRegionEXT;
extern __attribute__ ((visibility("default"))) PFNGLNEWBUFFERREGIONEXTPROC __glewNewBufferRegionEXT;
extern __attribute__ ((visibility("default"))) PFNGLREADBUFFERREGIONEXTPROC __glewReadBufferRegionEXT;

extern __attribute__ ((visibility("default"))) PFNGLRESIZEBUFFERSMESAPROC __glewResizeBuffersMESA;

extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2DMESAPROC __glewWindowPos2dMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2DVMESAPROC __glewWindowPos2dvMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2FMESAPROC __glewWindowPos2fMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2FVMESAPROC __glewWindowPos2fvMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2IMESAPROC __glewWindowPos2iMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2IVMESAPROC __glewWindowPos2ivMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2SMESAPROC __glewWindowPos2sMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS2SVMESAPROC __glewWindowPos2svMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3DMESAPROC __glewWindowPos3dMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3DVMESAPROC __glewWindowPos3dvMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3FMESAPROC __glewWindowPos3fMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3FVMESAPROC __glewWindowPos3fvMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3IMESAPROC __glewWindowPos3iMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3IVMESAPROC __glewWindowPos3ivMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3SMESAPROC __glewWindowPos3sMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS3SVMESAPROC __glewWindowPos3svMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4DMESAPROC __glewWindowPos4dMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4DVMESAPROC __glewWindowPos4dvMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4FMESAPROC __glewWindowPos4fMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4FVMESAPROC __glewWindowPos4fvMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4IMESAPROC __glewWindowPos4iMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4IVMESAPROC __glewWindowPos4ivMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4SMESAPROC __glewWindowPos4sMESA;
extern __attribute__ ((visibility("default"))) PFNGLWINDOWPOS4SVMESAPROC __glewWindowPos4svMESA;

extern __attribute__ ((visibility("default"))) PFNGLBEGINCONDITIONALRENDERNVPROC __glewBeginConditionalRenderNV;
extern __attribute__ ((visibility("default"))) PFNGLENDCONDITIONALRENDERNVPROC __glewEndConditionalRenderNV;

extern __attribute__ ((visibility("default"))) PFNGLCOPYIMAGESUBDATANVPROC __glewCopyImageSubDataNV;

extern __attribute__ ((visibility("default"))) PFNGLCLEARDEPTHDNVPROC __glewClearDepthdNV;
extern __attribute__ ((visibility("default"))) PFNGLDEPTHBOUNDSDNVPROC __glewDepthBoundsdNV;
extern __attribute__ ((visibility("default"))) PFNGLDEPTHRANGEDNVPROC __glewDepthRangedNV;

extern __attribute__ ((visibility("default"))) PFNGLEVALMAPSNVPROC __glewEvalMapsNV;
extern __attribute__ ((visibility("default"))) PFNGLGETMAPATTRIBPARAMETERFVNVPROC __glewGetMapAttribParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETMAPATTRIBPARAMETERIVNVPROC __glewGetMapAttribParameterivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETMAPCONTROLPOINTSNVPROC __glewGetMapControlPointsNV;
extern __attribute__ ((visibility("default"))) PFNGLGETMAPPARAMETERFVNVPROC __glewGetMapParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETMAPPARAMETERIVNVPROC __glewGetMapParameterivNV;
extern __attribute__ ((visibility("default"))) PFNGLMAPCONTROLPOINTSNVPROC __glewMapControlPointsNV;
extern __attribute__ ((visibility("default"))) PFNGLMAPPARAMETERFVNVPROC __glewMapParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLMAPPARAMETERIVNVPROC __glewMapParameterivNV;

extern __attribute__ ((visibility("default"))) PFNGLGETMULTISAMPLEFVNVPROC __glewGetMultisamplefvNV;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLEMASKINDEXEDNVPROC __glewSampleMaskIndexedNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXRENDERBUFFERNVPROC __glewTexRenderbufferNV;

extern __attribute__ ((visibility("default"))) PFNGLDELETEFENCESNVPROC __glewDeleteFencesNV;
extern __attribute__ ((visibility("default"))) PFNGLFINISHFENCENVPROC __glewFinishFenceNV;
extern __attribute__ ((visibility("default"))) PFNGLGENFENCESNVPROC __glewGenFencesNV;
extern __attribute__ ((visibility("default"))) PFNGLGETFENCEIVNVPROC __glewGetFenceivNV;
extern __attribute__ ((visibility("default"))) PFNGLISFENCENVPROC __glewIsFenceNV;
extern __attribute__ ((visibility("default"))) PFNGLSETFENCENVPROC __glewSetFenceNV;
extern __attribute__ ((visibility("default"))) PFNGLTESTFENCENVPROC __glewTestFenceNV;

extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC __glewGetProgramNamedParameterdvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC __glewGetProgramNamedParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMNAMEDPARAMETER4DNVPROC __glewProgramNamedParameter4dNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC __glewProgramNamedParameter4dvNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMNAMEDPARAMETER4FNVPROC __glewProgramNamedParameter4fNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC __glewProgramNamedParameter4fvNV;

extern __attribute__ ((visibility("default"))) PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC __glewRenderbufferStorageMultisampleCoverageNV;

extern __attribute__ ((visibility("default"))) PFNGLPROGRAMVERTEXLIMITNVPROC __glewProgramVertexLimitNV;

extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETERI4INVPROC __glewProgramEnvParameterI4iNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETERI4IVNVPROC __glewProgramEnvParameterI4ivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETERI4UINVPROC __glewProgramEnvParameterI4uiNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETERI4UIVNVPROC __glewProgramEnvParameterI4uivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETERSI4IVNVPROC __glewProgramEnvParametersI4ivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC __glewProgramEnvParametersI4uivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETERI4INVPROC __glewProgramLocalParameterI4iNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC __glewProgramLocalParameterI4ivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETERI4UINVPROC __glewProgramLocalParameterI4uiNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC __glewProgramLocalParameterI4uivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC __glewProgramLocalParametersI4ivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC __glewProgramLocalParametersI4uivNV;

extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMI64VNVPROC __glewGetUniformi64vNV;
extern __attribute__ ((visibility("default"))) PFNGLGETUNIFORMUI64VNVPROC __glewGetUniformui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1I64NVPROC __glewProgramUniform1i64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1I64VNVPROC __glewProgramUniform1i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1UI64NVPROC __glewProgramUniform1ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM1UI64VNVPROC __glewProgramUniform1ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2I64NVPROC __glewProgramUniform2i64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2I64VNVPROC __glewProgramUniform2i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2UI64NVPROC __glewProgramUniform2ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM2UI64VNVPROC __glewProgramUniform2ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3I64NVPROC __glewProgramUniform3i64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3I64VNVPROC __glewProgramUniform3i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3UI64NVPROC __glewProgramUniform3ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM3UI64VNVPROC __glewProgramUniform3ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4I64NVPROC __glewProgramUniform4i64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4I64VNVPROC __glewProgramUniform4i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4UI64NVPROC __glewProgramUniform4ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORM4UI64VNVPROC __glewProgramUniform4ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1I64NVPROC __glewUniform1i64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1I64VNVPROC __glewUniform1i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1UI64NVPROC __glewUniform1ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM1UI64VNVPROC __glewUniform1ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2I64NVPROC __glewUniform2i64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2I64VNVPROC __glewUniform2i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2UI64NVPROC __glewUniform2ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM2UI64VNVPROC __glewUniform2ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3I64NVPROC __glewUniform3i64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3I64VNVPROC __glewUniform3i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3UI64NVPROC __glewUniform3ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM3UI64VNVPROC __glewUniform3ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4I64NVPROC __glewUniform4i64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4I64VNVPROC __glewUniform4i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4UI64NVPROC __glewUniform4ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORM4UI64VNVPROC __glewUniform4ui64vNV;

extern __attribute__ ((visibility("default"))) PFNGLCOLOR3HNVPROC __glewColor3hNV;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR3HVNVPROC __glewColor3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4HNVPROC __glewColor4hNV;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4HVNVPROC __glewColor4hvNV;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDHNVPROC __glewFogCoordhNV;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDHVNVPROC __glewFogCoordhvNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1HNVPROC __glewMultiTexCoord1hNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD1HVNVPROC __glewMultiTexCoord1hvNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2HNVPROC __glewMultiTexCoord2hNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD2HVNVPROC __glewMultiTexCoord2hvNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3HNVPROC __glewMultiTexCoord3hNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD3HVNVPROC __glewMultiTexCoord3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4HNVPROC __glewMultiTexCoord4hNV;
extern __attribute__ ((visibility("default"))) PFNGLMULTITEXCOORD4HVNVPROC __glewMultiTexCoord4hvNV;
extern __attribute__ ((visibility("default"))) PFNGLNORMAL3HNVPROC __glewNormal3hNV;
extern __attribute__ ((visibility("default"))) PFNGLNORMAL3HVNVPROC __glewNormal3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3HNVPROC __glewSecondaryColor3hNV;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLOR3HVNVPROC __glewSecondaryColor3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD1HNVPROC __glewTexCoord1hNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD1HVNVPROC __glewTexCoord1hvNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2HNVPROC __glewTexCoord2hNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2HVNVPROC __glewTexCoord2hvNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD3HNVPROC __glewTexCoord3hNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD3HVNVPROC __glewTexCoord3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD4HNVPROC __glewTexCoord4hNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD4HVNVPROC __glewTexCoord4hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEX2HNVPROC __glewVertex2hNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEX2HVNVPROC __glewVertex2hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEX3HNVPROC __glewVertex3hNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEX3HVNVPROC __glewVertex3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEX4HNVPROC __glewVertex4hNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEX4HVNVPROC __glewVertex4hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1HNVPROC __glewVertexAttrib1hNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1HVNVPROC __glewVertexAttrib1hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2HNVPROC __glewVertexAttrib2hNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2HVNVPROC __glewVertexAttrib2hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3HNVPROC __glewVertexAttrib3hNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3HVNVPROC __glewVertexAttrib3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4HNVPROC __glewVertexAttrib4hNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4HVNVPROC __glewVertexAttrib4hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS1HVNVPROC __glewVertexAttribs1hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS2HVNVPROC __glewVertexAttribs2hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS3HVNVPROC __glewVertexAttribs3hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS4HVNVPROC __glewVertexAttribs4hvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXWEIGHTHNVPROC __glewVertexWeighthNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXWEIGHTHVNVPROC __glewVertexWeighthvNV;

extern __attribute__ ((visibility("default"))) PFNGLBEGINOCCLUSIONQUERYNVPROC __glewBeginOcclusionQueryNV;
extern __attribute__ ((visibility("default"))) PFNGLDELETEOCCLUSIONQUERIESNVPROC __glewDeleteOcclusionQueriesNV;
extern __attribute__ ((visibility("default"))) PFNGLENDOCCLUSIONQUERYNVPROC __glewEndOcclusionQueryNV;
extern __attribute__ ((visibility("default"))) PFNGLGENOCCLUSIONQUERIESNVPROC __glewGenOcclusionQueriesNV;
extern __attribute__ ((visibility("default"))) PFNGLGETOCCLUSIONQUERYIVNVPROC __glewGetOcclusionQueryivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETOCCLUSIONQUERYUIVNVPROC __glewGetOcclusionQueryuivNV;
extern __attribute__ ((visibility("default"))) PFNGLISOCCLUSIONQUERYNVPROC __glewIsOcclusionQueryNV;

extern __attribute__ ((visibility("default"))) PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC __glewProgramBufferParametersIivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC __glewProgramBufferParametersIuivNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC __glewProgramBufferParametersfvNV;

extern __attribute__ ((visibility("default"))) PFNGLFLUSHPIXELDATARANGENVPROC __glewFlushPixelDataRangeNV;
extern __attribute__ ((visibility("default"))) PFNGLPIXELDATARANGENVPROC __glewPixelDataRangeNV;

extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERINVPROC __glewPointParameteriNV;
extern __attribute__ ((visibility("default"))) PFNGLPOINTPARAMETERIVNVPROC __glewPointParameterivNV;

extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOI64VNVPROC __glewGetVideoi64vNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOIVNVPROC __glewGetVideoivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOUI64VNVPROC __glewGetVideoui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOUIVNVPROC __glewGetVideouivNV;
extern __attribute__ ((visibility("default"))) PFNGLPRESENTFRAMEDUALFILLNVPROC __glewPresentFrameDualFillNV;
extern __attribute__ ((visibility("default"))) PFNGLPRESENTFRAMEKEYEDNVPROC __glewPresentFrameKeyedNV;

extern __attribute__ ((visibility("default"))) PFNGLPRIMITIVERESTARTINDEXNVPROC __glewPrimitiveRestartIndexNV;
extern __attribute__ ((visibility("default"))) PFNGLPRIMITIVERESTARTNVPROC __glewPrimitiveRestartNV;

extern __attribute__ ((visibility("default"))) PFNGLCOMBINERINPUTNVPROC __glewCombinerInputNV;
extern __attribute__ ((visibility("default"))) PFNGLCOMBINEROUTPUTNVPROC __glewCombinerOutputNV;
extern __attribute__ ((visibility("default"))) PFNGLCOMBINERPARAMETERFNVPROC __glewCombinerParameterfNV;
extern __attribute__ ((visibility("default"))) PFNGLCOMBINERPARAMETERFVNVPROC __glewCombinerParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLCOMBINERPARAMETERINVPROC __glewCombinerParameteriNV;
extern __attribute__ ((visibility("default"))) PFNGLCOMBINERPARAMETERIVNVPROC __glewCombinerParameterivNV;
extern __attribute__ ((visibility("default"))) PFNGLFINALCOMBINERINPUTNVPROC __glewFinalCombinerInputNV;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC __glewGetCombinerInputParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC __glewGetCombinerInputParameterivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC __glewGetCombinerOutputParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC __glewGetCombinerOutputParameterivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC __glewGetFinalCombinerInputParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC __glewGetFinalCombinerInputParameterivNV;

extern __attribute__ ((visibility("default"))) PFNGLCOMBINERSTAGEPARAMETERFVNVPROC __glewCombinerStageParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC __glewGetCombinerStageParameterfvNV;

extern __attribute__ ((visibility("default"))) PFNGLGETBUFFERPARAMETERUI64VNVPROC __glewGetBufferParameterui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLGETINTEGERUI64VNVPROC __glewGetIntegerui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC __glewGetNamedBufferParameterui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLISBUFFERRESIDENTNVPROC __glewIsBufferResidentNV;
extern __attribute__ ((visibility("default"))) PFNGLISNAMEDBUFFERRESIDENTNVPROC __glewIsNamedBufferResidentNV;
extern __attribute__ ((visibility("default"))) PFNGLMAKEBUFFERNONRESIDENTNVPROC __glewMakeBufferNonResidentNV;
extern __attribute__ ((visibility("default"))) PFNGLMAKEBUFFERRESIDENTNVPROC __glewMakeBufferResidentNV;
extern __attribute__ ((visibility("default"))) PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC __glewMakeNamedBufferNonResidentNV;
extern __attribute__ ((visibility("default"))) PFNGLMAKENAMEDBUFFERRESIDENTNVPROC __glewMakeNamedBufferResidentNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMUI64NVPROC __glewProgramUniformui64NV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMUNIFORMUI64VNVPROC __glewProgramUniformui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMUI64NVPROC __glewUniformui64NV;
extern __attribute__ ((visibility("default"))) PFNGLUNIFORMUI64VNVPROC __glewUniformui64vNV;

extern __attribute__ ((visibility("default"))) PFNGLTEXTUREBARRIERNVPROC __glewTextureBarrierNV;

extern __attribute__ ((visibility("default"))) PFNGLTEXIMAGE2DMULTISAMPLECOVERAGENVPROC __glewTexImage2DMultisampleCoverageNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXIMAGE3DMULTISAMPLECOVERAGENVPROC __glewTexImage3DMultisampleCoverageNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREIMAGE2DMULTISAMPLECOVERAGENVPROC __glewTextureImage2DMultisampleCoverageNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREIMAGE2DMULTISAMPLENVPROC __glewTextureImage2DMultisampleNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREIMAGE3DMULTISAMPLECOVERAGENVPROC __glewTextureImage3DMultisampleCoverageNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXTUREIMAGE3DMULTISAMPLENVPROC __glewTextureImage3DMultisampleNV;

extern __attribute__ ((visibility("default"))) PFNGLACTIVEVARYINGNVPROC __glewActiveVaryingNV;
extern __attribute__ ((visibility("default"))) PFNGLBEGINTRANSFORMFEEDBACKNVPROC __glewBeginTransformFeedbackNV;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERBASENVPROC __glewBindBufferBaseNV;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFEROFFSETNVPROC __glewBindBufferOffsetNV;
extern __attribute__ ((visibility("default"))) PFNGLBINDBUFFERRANGENVPROC __glewBindBufferRangeNV;
extern __attribute__ ((visibility("default"))) PFNGLENDTRANSFORMFEEDBACKNVPROC __glewEndTransformFeedbackNV;
extern __attribute__ ((visibility("default"))) PFNGLGETACTIVEVARYINGNVPROC __glewGetActiveVaryingNV;
extern __attribute__ ((visibility("default"))) PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC __glewGetTransformFeedbackVaryingNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVARYINGLOCATIONNVPROC __glewGetVaryingLocationNV;
extern __attribute__ ((visibility("default"))) PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC __glewTransformFeedbackAttribsNV;
extern __attribute__ ((visibility("default"))) PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC __glewTransformFeedbackVaryingsNV;

extern __attribute__ ((visibility("default"))) PFNGLBINDTRANSFORMFEEDBACKNVPROC __glewBindTransformFeedbackNV;
extern __attribute__ ((visibility("default"))) PFNGLDELETETRANSFORMFEEDBACKSNVPROC __glewDeleteTransformFeedbacksNV;
extern __attribute__ ((visibility("default"))) PFNGLDRAWTRANSFORMFEEDBACKNVPROC __glewDrawTransformFeedbackNV;
extern __attribute__ ((visibility("default"))) PFNGLGENTRANSFORMFEEDBACKSNVPROC __glewGenTransformFeedbacksNV;
extern __attribute__ ((visibility("default"))) PFNGLISTRANSFORMFEEDBACKNVPROC __glewIsTransformFeedbackNV;
extern __attribute__ ((visibility("default"))) PFNGLPAUSETRANSFORMFEEDBACKNVPROC __glewPauseTransformFeedbackNV;
extern __attribute__ ((visibility("default"))) PFNGLRESUMETRANSFORMFEEDBACKNVPROC __glewResumeTransformFeedbackNV;

extern __attribute__ ((visibility("default"))) PFNGLVDPAUFININVPROC __glewVDPAUFiniNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUGETSURFACEIVNVPROC __glewVDPAUGetSurfaceivNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUINITNVPROC __glewVDPAUInitNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUISSURFACENVPROC __glewVDPAUIsSurfaceNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUMAPSURFACESNVPROC __glewVDPAUMapSurfacesNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUREGISTEROUTPUTSURFACENVPROC __glewVDPAURegisterOutputSurfaceNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUREGISTERVIDEOSURFACENVPROC __glewVDPAURegisterVideoSurfaceNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUSURFACEACCESSNVPROC __glewVDPAUSurfaceAccessNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUUNMAPSURFACESNVPROC __glewVDPAUUnmapSurfacesNV;
extern __attribute__ ((visibility("default"))) PFNGLVDPAUUNREGISTERSURFACENVPROC __glewVDPAUUnregisterSurfaceNV;

extern __attribute__ ((visibility("default"))) PFNGLFLUSHVERTEXARRAYRANGENVPROC __glewFlushVertexArrayRangeNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXARRAYRANGENVPROC __glewVertexArrayRangeNV;

extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBLI64VNVPROC __glewGetVertexAttribLi64vNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBLUI64VNVPROC __glewGetVertexAttribLui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1I64NVPROC __glewVertexAttribL1i64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1I64VNVPROC __glewVertexAttribL1i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1UI64NVPROC __glewVertexAttribL1ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL1UI64VNVPROC __glewVertexAttribL1ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2I64NVPROC __glewVertexAttribL2i64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2I64VNVPROC __glewVertexAttribL2i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2UI64NVPROC __glewVertexAttribL2ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL2UI64VNVPROC __glewVertexAttribL2ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3I64NVPROC __glewVertexAttribL3i64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3I64VNVPROC __glewVertexAttribL3i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3UI64NVPROC __glewVertexAttribL3ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL3UI64VNVPROC __glewVertexAttribL3ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4I64NVPROC __glewVertexAttribL4i64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4I64VNVPROC __glewVertexAttribL4i64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4UI64NVPROC __glewVertexAttribL4ui64NV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBL4UI64VNVPROC __glewVertexAttribL4ui64vNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBLFORMATNVPROC __glewVertexAttribLFormatNV;

extern __attribute__ ((visibility("default"))) PFNGLBUFFERADDRESSRANGENVPROC __glewBufferAddressRangeNV;
extern __attribute__ ((visibility("default"))) PFNGLCOLORFORMATNVPROC __glewColorFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLEDGEFLAGFORMATNVPROC __glewEdgeFlagFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLFOGCOORDFORMATNVPROC __glewFogCoordFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLGETINTEGERUI64I_VNVPROC __glewGetIntegerui64i_vNV;
extern __attribute__ ((visibility("default"))) PFNGLINDEXFORMATNVPROC __glewIndexFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLNORMALFORMATNVPROC __glewNormalFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLSECONDARYCOLORFORMATNVPROC __glewSecondaryColorFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORDFORMATNVPROC __glewTexCoordFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBFORMATNVPROC __glewVertexAttribFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBIFORMATNVPROC __glewVertexAttribIFormatNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXFORMATNVPROC __glewVertexFormatNV;

extern __attribute__ ((visibility("default"))) PFNGLAREPROGRAMSRESIDENTNVPROC __glewAreProgramsResidentNV;
extern __attribute__ ((visibility("default"))) PFNGLBINDPROGRAMNVPROC __glewBindProgramNV;
extern __attribute__ ((visibility("default"))) PFNGLDELETEPROGRAMSNVPROC __glewDeleteProgramsNV;
extern __attribute__ ((visibility("default"))) PFNGLEXECUTEPROGRAMNVPROC __glewExecuteProgramNV;
extern __attribute__ ((visibility("default"))) PFNGLGENPROGRAMSNVPROC __glewGenProgramsNV;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMPARAMETERDVNVPROC __glewGetProgramParameterdvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMPARAMETERFVNVPROC __glewGetProgramParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMSTRINGNVPROC __glewGetProgramStringNV;
extern __attribute__ ((visibility("default"))) PFNGLGETPROGRAMIVNVPROC __glewGetProgramivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETTRACKMATRIXIVNVPROC __glewGetTrackMatrixivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBPOINTERVNVPROC __glewGetVertexAttribPointervNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBDVNVPROC __glewGetVertexAttribdvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBFVNVPROC __glewGetVertexAttribfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVERTEXATTRIBIVNVPROC __glewGetVertexAttribivNV;
extern __attribute__ ((visibility("default"))) PFNGLISPROGRAMNVPROC __glewIsProgramNV;
extern __attribute__ ((visibility("default"))) PFNGLLOADPROGRAMNVPROC __glewLoadProgramNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETER4DNVPROC __glewProgramParameter4dNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETER4DVNVPROC __glewProgramParameter4dvNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETER4FNVPROC __glewProgramParameter4fNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETER4FVNVPROC __glewProgramParameter4fvNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETERS4DVNVPROC __glewProgramParameters4dvNV;
extern __attribute__ ((visibility("default"))) PFNGLPROGRAMPARAMETERS4FVNVPROC __glewProgramParameters4fvNV;
extern __attribute__ ((visibility("default"))) PFNGLREQUESTRESIDENTPROGRAMSNVPROC __glewRequestResidentProgramsNV;
extern __attribute__ ((visibility("default"))) PFNGLTRACKMATRIXNVPROC __glewTrackMatrixNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1DNVPROC __glewVertexAttrib1dNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1DVNVPROC __glewVertexAttrib1dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1FNVPROC __glewVertexAttrib1fNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1FVNVPROC __glewVertexAttrib1fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1SNVPROC __glewVertexAttrib1sNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB1SVNVPROC __glewVertexAttrib1svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2DNVPROC __glewVertexAttrib2dNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2DVNVPROC __glewVertexAttrib2dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2FNVPROC __glewVertexAttrib2fNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2FVNVPROC __glewVertexAttrib2fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2SNVPROC __glewVertexAttrib2sNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB2SVNVPROC __glewVertexAttrib2svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3DNVPROC __glewVertexAttrib3dNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3DVNVPROC __glewVertexAttrib3dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3FNVPROC __glewVertexAttrib3fNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3FVNVPROC __glewVertexAttrib3fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3SNVPROC __glewVertexAttrib3sNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB3SVNVPROC __glewVertexAttrib3svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4DNVPROC __glewVertexAttrib4dNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4DVNVPROC __glewVertexAttrib4dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4FNVPROC __glewVertexAttrib4fNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4FVNVPROC __glewVertexAttrib4fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4SNVPROC __glewVertexAttrib4sNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4SVNVPROC __glewVertexAttrib4svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4UBNVPROC __glewVertexAttrib4ubNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIB4UBVNVPROC __glewVertexAttrib4ubvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBPOINTERNVPROC __glewVertexAttribPointerNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS1DVNVPROC __glewVertexAttribs1dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS1FVNVPROC __glewVertexAttribs1fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS1SVNVPROC __glewVertexAttribs1svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS2DVNVPROC __glewVertexAttribs2dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS2FVNVPROC __glewVertexAttribs2fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS2SVNVPROC __glewVertexAttribs2svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS3DVNVPROC __glewVertexAttribs3dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS3FVNVPROC __glewVertexAttribs3fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS3SVNVPROC __glewVertexAttribs3svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS4DVNVPROC __glewVertexAttribs4dvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS4FVNVPROC __glewVertexAttribs4fvNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS4SVNVPROC __glewVertexAttribs4svNV;
extern __attribute__ ((visibility("default"))) PFNGLVERTEXATTRIBS4UBVNVPROC __glewVertexAttribs4ubvNV;

extern __attribute__ ((visibility("default"))) PFNGLBEGINVIDEOCAPTURENVPROC __glewBeginVideoCaptureNV;
extern __attribute__ ((visibility("default"))) PFNGLBINDVIDEOCAPTURESTREAMBUFFERNVPROC __glewBindVideoCaptureStreamBufferNV;
extern __attribute__ ((visibility("default"))) PFNGLBINDVIDEOCAPTURESTREAMTEXTURENVPROC __glewBindVideoCaptureStreamTextureNV;
extern __attribute__ ((visibility("default"))) PFNGLENDVIDEOCAPTURENVPROC __glewEndVideoCaptureNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOCAPTURESTREAMDVNVPROC __glewGetVideoCaptureStreamdvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOCAPTURESTREAMFVNVPROC __glewGetVideoCaptureStreamfvNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOCAPTURESTREAMIVNVPROC __glewGetVideoCaptureStreamivNV;
extern __attribute__ ((visibility("default"))) PFNGLGETVIDEOCAPTUREIVNVPROC __glewGetVideoCaptureivNV;
extern __attribute__ ((visibility("default"))) PFNGLVIDEOCAPTURENVPROC __glewVideoCaptureNV;
extern __attribute__ ((visibility("default"))) PFNGLVIDEOCAPTURESTREAMPARAMETERDVNVPROC __glewVideoCaptureStreamParameterdvNV;
extern __attribute__ ((visibility("default"))) PFNGLVIDEOCAPTURESTREAMPARAMETERFVNVPROC __glewVideoCaptureStreamParameterfvNV;
extern __attribute__ ((visibility("default"))) PFNGLVIDEOCAPTURESTREAMPARAMETERIVNVPROC __glewVideoCaptureStreamParameterivNV;

extern __attribute__ ((visibility("default"))) PFNGLCLEARDEPTHFOESPROC __glewClearDepthfOES;
extern __attribute__ ((visibility("default"))) PFNGLCLIPPLANEFOESPROC __glewClipPlanefOES;
extern __attribute__ ((visibility("default"))) PFNGLDEPTHRANGEFOESPROC __glewDepthRangefOES;
extern __attribute__ ((visibility("default"))) PFNGLFRUSTUMFOESPROC __glewFrustumfOES;
extern __attribute__ ((visibility("default"))) PFNGLGETCLIPPLANEFOESPROC __glewGetClipPlanefOES;
extern __attribute__ ((visibility("default"))) PFNGLORTHOFOESPROC __glewOrthofOES;

extern __attribute__ ((visibility("default"))) PFNGLDETAILTEXFUNCSGISPROC __glewDetailTexFuncSGIS;
extern __attribute__ ((visibility("default"))) PFNGLGETDETAILTEXFUNCSGISPROC __glewGetDetailTexFuncSGIS;

extern __attribute__ ((visibility("default"))) PFNGLFOGFUNCSGISPROC __glewFogFuncSGIS;
extern __attribute__ ((visibility("default"))) PFNGLGETFOGFUNCSGISPROC __glewGetFogFuncSGIS;

extern __attribute__ ((visibility("default"))) PFNGLSAMPLEMASKSGISPROC __glewSampleMaskSGIS;
extern __attribute__ ((visibility("default"))) PFNGLSAMPLEPATTERNSGISPROC __glewSamplePatternSGIS;

extern __attribute__ ((visibility("default"))) PFNGLGETSHARPENTEXFUNCSGISPROC __glewGetSharpenTexFuncSGIS;
extern __attribute__ ((visibility("default"))) PFNGLSHARPENTEXFUNCSGISPROC __glewSharpenTexFuncSGIS;

extern __attribute__ ((visibility("default"))) PFNGLTEXIMAGE4DSGISPROC __glewTexImage4DSGIS;
extern __attribute__ ((visibility("default"))) PFNGLTEXSUBIMAGE4DSGISPROC __glewTexSubImage4DSGIS;

extern __attribute__ ((visibility("default"))) PFNGLGETTEXFILTERFUNCSGISPROC __glewGetTexFilterFuncSGIS;
extern __attribute__ ((visibility("default"))) PFNGLTEXFILTERFUNCSGISPROC __glewTexFilterFuncSGIS;

extern __attribute__ ((visibility("default"))) PFNGLASYNCMARKERSGIXPROC __glewAsyncMarkerSGIX;
extern __attribute__ ((visibility("default"))) PFNGLDELETEASYNCMARKERSSGIXPROC __glewDeleteAsyncMarkersSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFINISHASYNCSGIXPROC __glewFinishAsyncSGIX;
extern __attribute__ ((visibility("default"))) PFNGLGENASYNCMARKERSSGIXPROC __glewGenAsyncMarkersSGIX;
extern __attribute__ ((visibility("default"))) PFNGLISASYNCMARKERSGIXPROC __glewIsAsyncMarkerSGIX;
extern __attribute__ ((visibility("default"))) PFNGLPOLLASYNCSGIXPROC __glewPollAsyncSGIX;

extern __attribute__ ((visibility("default"))) PFNGLFLUSHRASTERSGIXPROC __glewFlushRasterSGIX;

extern __attribute__ ((visibility("default"))) PFNGLTEXTUREFOGSGIXPROC __glewTextureFogSGIX;

extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTCOLORMATERIALSGIXPROC __glewFragmentColorMaterialSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELFSGIXPROC __glewFragmentLightModelfSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELFVSGIXPROC __glewFragmentLightModelfvSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELISGIXPROC __glewFragmentLightModeliSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTMODELIVSGIXPROC __glewFragmentLightModelivSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTFSGIXPROC __glewFragmentLightfSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTFVSGIXPROC __glewFragmentLightfvSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTISGIXPROC __glewFragmentLightiSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTLIGHTIVSGIXPROC __glewFragmentLightivSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALFSGIXPROC __glewFragmentMaterialfSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALFVSGIXPROC __glewFragmentMaterialfvSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALISGIXPROC __glewFragmentMaterialiSGIX;
extern __attribute__ ((visibility("default"))) PFNGLFRAGMENTMATERIALIVSGIXPROC __glewFragmentMaterialivSGIX;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTLIGHTFVSGIXPROC __glewGetFragmentLightfvSGIX;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTLIGHTIVSGIXPROC __glewGetFragmentLightivSGIX;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTMATERIALFVSGIXPROC __glewGetFragmentMaterialfvSGIX;
extern __attribute__ ((visibility("default"))) PFNGLGETFRAGMENTMATERIALIVSGIXPROC __glewGetFragmentMaterialivSGIX;

extern __attribute__ ((visibility("default"))) PFNGLFRAMEZOOMSGIXPROC __glewFrameZoomSGIX;

extern __attribute__ ((visibility("default"))) PFNGLPIXELTEXGENSGIXPROC __glewPixelTexGenSGIX;

extern __attribute__ ((visibility("default"))) PFNGLREFERENCEPLANESGIXPROC __glewReferencePlaneSGIX;

extern __attribute__ ((visibility("default"))) PFNGLSPRITEPARAMETERFSGIXPROC __glewSpriteParameterfSGIX;
extern __attribute__ ((visibility("default"))) PFNGLSPRITEPARAMETERFVSGIXPROC __glewSpriteParameterfvSGIX;
extern __attribute__ ((visibility("default"))) PFNGLSPRITEPARAMETERISGIXPROC __glewSpriteParameteriSGIX;
extern __attribute__ ((visibility("default"))) PFNGLSPRITEPARAMETERIVSGIXPROC __glewSpriteParameterivSGIX;

extern __attribute__ ((visibility("default"))) PFNGLTAGSAMPLEBUFFERSGIXPROC __glewTagSampleBufferSGIX;

extern __attribute__ ((visibility("default"))) PFNGLCOLORTABLEPARAMETERFVSGIPROC __glewColorTableParameterfvSGI;
extern __attribute__ ((visibility("default"))) PFNGLCOLORTABLEPARAMETERIVSGIPROC __glewColorTableParameterivSGI;
extern __attribute__ ((visibility("default"))) PFNGLCOLORTABLESGIPROC __glewColorTableSGI;
extern __attribute__ ((visibility("default"))) PFNGLCOPYCOLORTABLESGIPROC __glewCopyColorTableSGI;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEPARAMETERFVSGIPROC __glewGetColorTableParameterfvSGI;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLEPARAMETERIVSGIPROC __glewGetColorTableParameterivSGI;
extern __attribute__ ((visibility("default"))) PFNGLGETCOLORTABLESGIPROC __glewGetColorTableSGI;

extern __attribute__ ((visibility("default"))) PFNGLFINISHTEXTURESUNXPROC __glewFinishTextureSUNX;

extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORBSUNPROC __glewGlobalAlphaFactorbSUN;
extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORDSUNPROC __glewGlobalAlphaFactordSUN;
extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORFSUNPROC __glewGlobalAlphaFactorfSUN;
extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORISUNPROC __glewGlobalAlphaFactoriSUN;
extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORSSUNPROC __glewGlobalAlphaFactorsSUN;
extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORUBSUNPROC __glewGlobalAlphaFactorubSUN;
extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORUISUNPROC __glewGlobalAlphaFactoruiSUN;
extern __attribute__ ((visibility("default"))) PFNGLGLOBALALPHAFACTORUSSUNPROC __glewGlobalAlphaFactorusSUN;

extern __attribute__ ((visibility("default"))) PFNGLREADVIDEOPIXELSSUNPROC __glewReadVideoPixelsSUN;

extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEPOINTERSUNPROC __glewReplacementCodePointerSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUBSUNPROC __glewReplacementCodeubSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUBVSUNPROC __glewReplacementCodeubvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUISUNPROC __glewReplacementCodeuiSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUIVSUNPROC __glewReplacementCodeuivSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUSSUNPROC __glewReplacementCodeusSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUSVSUNPROC __glewReplacementCodeusvSUN;

extern __attribute__ ((visibility("default"))) PFNGLCOLOR3FVERTEX3FSUNPROC __glewColor3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR3FVERTEX3FVSUNPROC __glewColor3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewColor4fNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewColor4fNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4UBVERTEX2FSUNPROC __glewColor4ubVertex2fSUN;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4UBVERTEX2FVSUNPROC __glewColor4ubVertex2fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4UBVERTEX3FSUNPROC __glewColor4ubVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLCOLOR4UBVERTEX3FVSUNPROC __glewColor4ubVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLNORMAL3FVERTEX3FSUNPROC __glewNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLNORMAL3FVERTEX3FVSUNPROC __glewNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC __glewReplacementCodeuiColor3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC __glewReplacementCodeuiColor3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiColor4fNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiColor4fNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC __glewReplacementCodeuiColor4ubVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC __glewReplacementCodeuiColor4ubVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC __glewReplacementCodeuiVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC __glewReplacementCodeuiVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC __glewTexCoord2fColor3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC __glewTexCoord2fColor3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewTexCoord2fColor4fNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewTexCoord2fColor4fNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC __glewTexCoord2fColor4ubVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC __glewTexCoord2fColor4ubVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC __glewTexCoord2fNormal3fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC __glewTexCoord2fNormal3fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FVERTEX3FSUNPROC __glewTexCoord2fVertex3fSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD2FVERTEX3FVSUNPROC __glewTexCoord2fVertex3fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC __glewTexCoord4fColor4fNormal3fVertex4fSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC __glewTexCoord4fColor4fNormal3fVertex4fvSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD4FVERTEX4FSUNPROC __glewTexCoord4fVertex4fSUN;
extern __attribute__ ((visibility("default"))) PFNGLTEXCOORD4FVERTEX4FVSUNPROC __glewTexCoord4fVertex4fvSUN;

extern __attribute__ ((visibility("default"))) PFNGLADDSWAPHINTRECTWINPROC __glewAddSwapHintRectWIN;

#endif
