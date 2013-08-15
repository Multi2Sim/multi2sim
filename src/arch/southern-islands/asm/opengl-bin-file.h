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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_OPENGL_BIN_FILE_H
#define ARCH_SOUTHERN_ISLANDS_EMU_OPENGL_BIN_FILE_H

#include <stdint.h>
#include <stdbool.h>
#include <lib/util/elf-format.h>
#include <src/arch/southern-islands/asm/bin-file.h>

/* Common */
#define MAX_USER_ELEMENTS 16
#define MAX_SEMANTICS_MAPPINGS 16
#define MAX_NUM_SAMPLER  32
#define MAX_NUM_RESOURCE 256
#define MAX_NUM_UAV      1024
#define MAX_CONSTANTS    256

#define MAX_UAV      12      /* mask uav supported */
#define MAX_TRANSFORM_FEEDBACK_BUFFERS 4  /* maximum number of transform feedback buffers supported */

#define MAX_UAV_RESOURCES 1024 /* The maximum number of UAVs supported */
#define MAX_UAV_MASKS (((MAX_UAV_RESOURCES) + 31) / 32)   /* The maximum of UAV masks */

#define MAX_TEXTURE_IMAGE_UNITS        32 /* The maximum number of texture image units supported by GLL */
#define MAX_UAV_IMAGE_UNITS            8  /* The maximum number of image (UAV) supported by GLL */
#define NUM_GLOBAL_RETURN_BUFFERS      8  /* max global return buffers */
#define MAX_TEXTURE_RESOURCES          160 /* The maximus number of texture resource */
#define PS_MAX_OUTPUTS                 8   /* Max Render Targets */

/* SI specific */
#define SC_SI_VS_MAX_INPUTS       32
#define SC_SI_VS_MAX_OUTPUTS      32
#define SC_SI_PS_MAX_INPUTS       32
#define SC_SI_PS_MAX_OUTPUTS      8
#define SC_SI_NUM_INTBUF          2
#define SC_SI_NUM_USER_ELEMENT    16

/* Max PS export patch code size in DWORDs */
#define SC_SI_MAX_EXPORT_CODE_SIZE 6
#define SC_SI_NUM_EXPORT_FMT  0xa

/* Number of SO streams */
#define SC_SI_SO_MAX_STREAMS  4
/* Number of SO buffers */
#define SC_SI_SO_MAX_BUFFERS  4

/* Forward declaration */
struct list_t;
struct si_opengl_shader_binary_t;

/* Shader types */
enum si_opengl_shader_binary_kind_t
{
	SI_OPENGL_SHADER_VERTEX = 0,
	SI_OPENGL_SHADER_GEOMETRY,
	SI_OPENGL_SHADER_CONTROL, /* aka HULL shader */
	SI_OPENGL_SHADER_EVALUATION, /* aka Domain shader */
	SI_OPENGL_SHADER_FRAGMENT = 4,
	SI_OPENGL_SHADER_COMPUTE,
	SI_OPENGL_SHADER_INVALID
};

enum si_opengl_bin_input_swizzle_type_t
{
	SI_OPENGL_SWIZZLE_X,	/* Swizzle the X component into this component */
	SI_OPENGL_SWIZZLE_Y,	/* Swizzle the Y component into this component */
	SI_OPENGL_SWIZZLE_Z,	/* Swizzle the Z component into this component */
	SI_OPENGL_SWIZZLE_W,	/* Swizzle the W component into this component */
	SI_OPENGL_SWIZZLE_0,	/* Swizzle constant 0 into this component */
	SI_OPENGL_SWIZZLE_1	/* Swizzle constant 1 into this component */
};

enum si_opengl_bin_input_type_t
{
	SI_OPENGL_INPUT_ATTRIB,	/* generic attribute */
	SI_OPENGL_INPUT_COLOR,	/* primary color */
	SI_OPENGL_INPUT_SECONDARYCOLOR,	/* secondary color */
	SI_OPENGL_INPUT_TEXCOORD,	/* texture coordinate */
	SI_OPENGL_INPUT_TEXID,	/* texture unit id */
	SI_OPENGL_INPUT_BUFFERID,	/* buffer unit id */
	SI_OPENGL_INPUT_CONSTANTBUFFERID,	/* constant buffer unit id */
	SI_OPENGL_INPUT_TEXTURERESOURCEID	/* texture resource id	 */
};

enum si_opengl_bin_output_type_t
{
	SI_OPENGL_OUTPUT_POS,	/* Position */
	SI_OPENGL_OUTPUT_POINTSIZE,	/* Point size */
	SI_OPENGL_OUTPUT_COLOR,	/* Primary color, offset 0 is front, offset 1 is back */
	SI_OPENGL_OUTPUT_SECONDARYCOLOR,	/* Secondary color, offset 0 is front, offset 1 is back */
	SI_OPENGL_OUTPUT_GENERIC,	/* Texture coordinate and user define varyings for pre R5xx asics, but on R6xx above, generic contains colors */
	SI_OPENGL_OUTPUT_DEPTH,	/* Fragment depth */
	SI_OPENGL_OUTPUT_CLIPDISTANCE,	/* Clip distance */
	SI_OPENGL_OUTPUT_PRIMITIVEID,	/* PrimitiveID */
	SI_OPENGL_OUTPUT_LAYER,	/* Layer */
	SI_OPENGL_OUTPUT_VIEWPORTINDEX,	/* viewportindex */
	SI_OPENGL_OUTPUT_STENCIL,	/* Fragment stencil ref value */
	SI_OPENGL_OUTPUT_STENCIL_VALUE,	/* Fragment stencil operation value */
	SI_OPENGL_OUTPUT_SAMPLEMASK,	/* Output sample coverage mask (FS only) */
	SI_OPENGL_OUTPUT_STREAMID	/* Ouput stream id */
};

/* Symbol types */
enum si_opengl_bin_symbol_type_t
{
	SI_OPENGL_SYMBOL_UNIFORM,                      /* GLSL uniform */
	SI_OPENGL_SYMBOL_BINDABLE_UNIFORM,             /* GLSL bindable uniform */
	SI_OPENGL_SYMBOL_UNIFORM_BLOCK,                /* GLSL uniform block */
	SI_OPENGL_SYMBOL_ATTRIB_POSITION,              /* Vertex position */
	SI_OPENGL_SYMBOL_ATTRIB_NORMAL,                /* Vertex normal */
	SI_OPENGL_SYMBOL_ATTRIB_PRIMARY_COLOR,         /* Vertex primary color */
	SI_OPENGL_SYMBOL_ATTRIB_SECONDARY_COLOR,       /* Vertex secondary color */
	SI_OPENGL_SYMBOL_ATTRIB_FOGCOORD,              /* Vertex fog coord */
	SI_OPENGL_SYMBOL_ATTRIB_EDGEFLAG,              /* Vertex edge flag */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD0,             /* Vertex texture coord 0 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD1,             /* Vertex texture coord 1 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD2,             /* Vertex texture coord 2 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD3,             /* Vertex texture coord 3 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD4,             /* Vertex texture coord 4 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD5,             /* Vertex texture coord 5 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD6,             /* Vertex texture coord 6 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD7,             /* Vertex texture coord 7 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD8,             /* Vertex texture coord 8 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD9,             /* Vertex texture coord 9 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD10,            /* Vertex texture coord 10 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD11,            /* Vertex texture coord 11 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD12,            /* Vertex texture coord 12 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD13,            /* Vertex texture coord 13 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD14,            /* Vertex texture coord 14 */
	SI_OPENGL_SYMBOL_ATTRIB_TEXCOORD15,            /* Vertex texture coord 15 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC0,              /* Generic attrib 0 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC1,              /* Generic attrib 1 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC2,              /* Generic attrib 2 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC3,              /* Generic attrib 3 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC4,              /* Generic attrib 4 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC5,              /* Generic attrib 5 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC6,              /* Generic attrib 6 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC7,              /* Generic attrib 7 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC8,              /* Generic attrib 8 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC9,              /* Generic attrib 9 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC10,             /* Generic attrib 10 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC11,             /* Generic attrib 11 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC12,             /* Generic attrib 12 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC13,             /* Generic attrib 13 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC14,             /* Generic attrib 14 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC15,             /* Generic attrib 15 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC16,             /* Generic attrib 16 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC17,             /* Generic attrib 17 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC18,             /* Generic attrib 18 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC19,             /* Generic attrib 19 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC20,             /* Generic attrib 20 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC21,             /* Generic attrib 21 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC22,             /* Generic attrib 22 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC23,             /* Generic attrib 23 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC24,             /* Generic attrib 24 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC25,             /* Generic attrib 25 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC26,             /* Generic attrib 26 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC27,             /* Generic attrib 27 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC28,             /* Generic attrib 28 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC29,             /* Generic attrib 29 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC30,             /* Generic attrib 30 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC31,             /* Generic attrib 31 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC32,             /* Generic attrib 32 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC33,             /* Generic attrib 33 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC34,             /* Generic attrib 34 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC35,             /* Generic attrib 35 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC36,             /* Generic attrib 36 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC37,             /* Generic attrib 37 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC38,             /* Generic attrib 38 */
	SI_OPENGL_SYMBOL_ATTRIB_GENERIC39,             /* Generic attrib 39 */
	SI_OPENGL_SYMBOL_FSVARYINGOUT,                 /* Varying out */
	SI_OPENGL_SYMBOL_ATTRIB_VERTEXID,              /* VertexID */
	SI_OPENGL_SYMBOL_ATTRIB_INSTANCEID,            /* InstanceID */
	SI_OPENGL_SYMBOL_SUBROUTINE_UNIFORM,           /* GLSL Subroutine Uniform */
	SI_OPENGL_SYMBOL_SUBROUTINE_FUNCTION,          /* GLSL Subroutine Function */
	SI_OPENGL_SYMBOL_SEPARATE_VARYING_IN_BASE,     /* base varying in symbol of separate mode */
	SI_OPENGL_SYMBOL_SEPARATE_VARYING_OUT_BASE,    /* base varying out symbol of sparate mode */
	SI_OPENGL_SYMBOL_SEPARATE_VARYING_IN_DERIVED,  /* derived varying in symbol of separate mode */
	SI_OPENGL_SYMBOL_SEPARATE_VARYING_OUT_DERIVED, /* derived varying out symbol of sparate mode */
	SI_OPENGL_SYMBOL_NONSEPARATE_VARYING_IN,       /* varying in of nonseparate mode */
	SI_OPENGL_SYMBOL_NONSEPARATE_VARYING_OUT,      /* varying out of nonseparate mode */
	SI_OPENGL_SYMBOL_SHADER_STORAGE_BUFFER,        /* shader storage buffer */
	SI_OPENGL_SYMBOL_ATTRIB_INPUTVALID,            /* input valide bits for sparse texture */
};


/* Symbol data types */
enum si_opengl_bin_symbol_datatype_t
{
	SI_OPENGL_SYMBOL_DATATYPE_VOID,                   /* void */
	SI_OPENGL_SYMBOL_DATATYPE_BOOL,                   /* bool */
	SI_OPENGL_SYMBOL_DATATYPE_INT,                    /* int */
	SI_OPENGL_SYMBOL_DATATYPE_UINT,                   /* unsigned int */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT,                  /* float */
	SI_OPENGL_SYMBOL_DATATYPE_DOUBLE,                 /* double */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16,                /* float16 */
	SI_OPENGL_SYMBOL_DATATYPE_DVEC2,                  /* dvec2 */
	SI_OPENGL_SYMBOL_DATATYPE_DVEC3,                  /* dvec3 */
	SI_OPENGL_SYMBOL_DATATYPE_DVEC4,                  /* dvec4 */
	SI_OPENGL_SYMBOL_DATATYPE_VEC2,                   /* vec2 */
	SI_OPENGL_SYMBOL_DATATYPE_VEC3,                   /* vec3 */
	SI_OPENGL_SYMBOL_DATATYPE_VEC4,                   /* vec4 */
	SI_OPENGL_SYMBOL_DATATYPE_BVEC2,                  /* bvec2 */
	SI_OPENGL_SYMBOL_DATATYPE_BVEC3,                  /* bvec3 */
	SI_OPENGL_SYMBOL_DATATYPE_BVEC4,                  /* bvec4 */
	SI_OPENGL_SYMBOL_DATATYPE_IVEC2,                  /* ivec2 */
	SI_OPENGL_SYMBOL_DATATYPE_IVEC3,                  /* ivec3 */
	SI_OPENGL_SYMBOL_DATATYPE_IVEC4,                  /* ivec4 */
	SI_OPENGL_SYMBOL_DATATYPE_UVEC2,                  /* unsigned ivec2 */
	SI_OPENGL_SYMBOL_DATATYPE_UVEC3,                  /* unsigned ivec3 */
	SI_OPENGL_SYMBOL_DATATYPE_UVEC4,                  /* unsigned ivec4 */
	SI_OPENGL_SYMBOL_DATATYPE_F16VEC2,                /* f16vec2 */
	SI_OPENGL_SYMBOL_DATATYPE_F16VEC3,                /* f16vec3 */
	SI_OPENGL_SYMBOL_DATATYPE_F16VEC4,                /* f16vec4 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT2,                   /* mat2 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT2X3,                 /* mat2x3 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT2X4,                 /* mat2x4 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT3X2,                 /* mat3x2 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT3,                   /* mat3 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT3X4,                 /* mat3x4 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT4X2,                 /* mat4x2 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT4X3,                 /* mat4x3 */
	SI_OPENGL_SYMBOL_DATATYPE_MAT4,                   /* mat4 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT2,                  /* dmat2 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT2X3,                /* dmat2x3 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT2X4,                /* dmat2x4 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT3X2,                /* dmat3x2 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT3,                  /* dmat3 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT3X4,                /* dmat3x4 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT4X2,                /* dmat4x2 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT4X3,                /* dmat4x3 */
	SI_OPENGL_SYMBOL_DATATYPE_DMAT4,                  /* dmat4 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT2,                /* f16mat2 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT2X3,              /* f16mat2x3 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT2X4,              /* f16mat2x4 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT3X2,              /* f16mat3x2 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT3,                /* f16mat3 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT3X4,              /* f16mat3x4 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT4X2,              /* f16mat4x2 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT4X3,              /* f16mat4x3 */
	SI_OPENGL_SYMBOL_DATATYPE_F16MAT4,                /* f16mat4 */

	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_1D,             /* sampler1D */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D,             /* sampler2D */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D_RECT,        /* sampler2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_EXTERNAL,       /* samplerExternalOES */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_3D,             /* sampler3D */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_CUBE,           /* samplerCube */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_1D_ARRAY,       /* sampler1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D_ARRAY,       /* sampler2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_CUBE_ARRAY,        /*samplerCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_BUFFER,         /* samplerBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_RENDERBUFFER,   /* samplerRenderbuffer */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D_MS,          /* sampler2DMS */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D_MS_ARRAY,    /* sampler2DMSArray */

	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_1D_SHADOW,      /* sampler1DShadow */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D_SHADOW,      /* sampler2DShadow */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D_RECT_SHADOW, /* sampler2DRectShadow */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_CUBE_SHADOW,    /* samplerCubeShadow */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_1D_ARRAY_SHADOW,/* sampler1DArrayShadow */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_2D_ARRAY_SHADOW,/* sampler2DArrayShadow */
	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_CUBE_ARRAY_SHADOW, /*samplerCubeArrayShadow */
	                                                  ///
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_1D,         /* isampler1D */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_2D,         /* isampler2D */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_2D_RECT,    /* isampler2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_3D,         /* isampler3D */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_CUBE,       /* isamplerCube */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_1D_ARRAY,   /* isampler1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_2D_ARRAY,   /* isampler2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_CUBE_ARRAY, /* isamplerCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_BUFFER,     /* isamplerBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_RENDERBUFFER,   /* isamplerRenderbuffer */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_2D_MS,          /* isampler2DMS */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_2D_MS_ARRAY,    /* isampler2DMSArray */

	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_1D,         /* usampler1D */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D,         /* usampler2D */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_RECT,    /* usampler2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_3D,         /* usampler3D */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE,       /* usamplerCube */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_1D_ARRAY,   /* usampler1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_ARRAY,   /* usampler2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE_ARRAY, /* usamplerCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_BUFFER,     /* usamplerBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_RENDERBUFFER,   /* usamplerRenderbuffer */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_MS,          /* usampler2DMS */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_MS_ARRAY,    /* usampler2DMSArray */

	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D,             /* f16sampler1D */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D,             /* f16sampler2D */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_RECT,        /* f16sampler2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_3D,             /* f16sampler3D */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE,           /* f16samplerCube */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_ARRAY,       /* f16sampler1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_ARRAY,       /* f16sampler2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_ARRAY,        /*f16samplerCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_BUFFER,         /* f16samplerBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_MS,          /* f16sampler2DMS */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_MS_ARRAY,    /* f16sampler2DMSArray */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_SHADOW,      /* f16sampler1DShadow */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_SHADOW,      /* f16sampler2DShadow */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_RECT_SHADOW, /* f16sampler2DRectShadow */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_SHADOW,    /* f16samplerCubeShadow */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_ARRAY_SHADOW,/* f16sampler1DArrayShadow */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_ARRAY_SHADOW,/* f16sampler2DArrayShadow */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_ARRAY_SHADOW,/* f16samplerCubeArrayShadow */

	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_1D,                        /* image1D */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_2D,                        /* image2D */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_3D,                        /* image3D */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_2D_RECT,                   /* image2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_CUBEMAP,                   /* imageCube */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_BUFFER,                    /* imageBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_1D_ARRAY,                  /* image1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_2D_ARRAY,                  /* image2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_CUBEMAP_ARRAY,             /* imageCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_2D_MS,                     /* image2DMultisample */
	SI_OPENGL_SYMBOL_DATATYPE_IMAGE_2D_MS_ARRAY,               /* image2DMultisampleArray */

	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_1D,                    /* iimage1D */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_2D,                    /* iimage2D */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_3D,                    /* iimage3D */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_2D_RECT,               /* iimage2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_CUBEMAP,               /* iimageCube */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_BUFFER,                /* iimageBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_1D_ARRAY,              /* iimage1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_2D_ARRAY,              /* iimage2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_CUBEMAP_ARRAY,         /* iimageCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_2D_MS,                 /* iimage2DMultisample */
	SI_OPENGL_SYMBOL_DATATYPE_INT_IMAGE_2D_MS_ARRAY,           /* iimage2DMultisampleArray */

	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_1D,           /* uimage1D */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D,           /* uimage2D */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_3D,           /* uimage3D */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_RECT,      /* uimage2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_CUBEMAP,      /* uimageCube */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_BUFFER,       /* uimageBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_1D_ARRAY,     /* uimage1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_ARRAY,     /* uimage2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_CUBEMAP_ARRAY,/* uimageCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_MS,        /* uimage2DMultisample */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_MS_ARRAY,  /* uimage2DMultisampleArray */

	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_1D,                /* f16image1D */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D,                /* f16image2D */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_3D,                /* f16image3D */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_RECT,           /* f16image2DRect */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_CUBEMAP,           /* f16imageCube */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_BUFFER,            /* f16imageBuffer */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_1D_ARRAY,          /* f16image1DArray */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_ARRAY,          /* f16image2DArray */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_CUBEMAP_ARRAY,     /* f16imageCubeArray */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_MS,             /* f16image2DMultisample */
	SI_OPENGL_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_MS_ARRAY,       /* f16image2DMultisampleArray */

	SI_OPENGL_SYMBOL_DATATYPE_SAMPLER_BUFFER_AMD,                 /* used for tessellation (from vbo) */
	SI_OPENGL_SYMBOL_DATATYPE_INT_SAMPLER_BUFFER_AMD,             /* used for vertex shader tesssellation */
	SI_OPENGL_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_BUFFER_AMD,    /* used for vertex shader tesssellation */

	SI_OPENGL_SYMBOL_DATATYPE_ATOMIC_COUNTER,          /* atomic counter */

	SI_OPENGL_SYMBOL_DATATYPE_STRUCT,                 /* struct, used for bindable uniform, it is also used for type check in separate program mode */
	SI_OPENGL_SYMBOL_DATATYPE_INTERFACE,              /* interface block, used for type check in separate progarm mode */
	SI_OPENGL_SYMBOL_DATATYPE_FIRST = SI_OPENGL_SYMBOL_DATATYPE_VOID,
	SI_OPENGL_SYMBOL_DATATYPE_LAST  = SI_OPENGL_SYMBOL_DATATYPE_INTERFACE
};

enum si_opengl_fetch_shader_type_t
{
	SI_OPENGL_FETCH_FIXED_FUNCTION,   /* Fetch shader executes all vertex fetches and start address is in a fixed function register. */
	SI_OPENGL_FETCH_SUB_IMMEDIATE,    /* SI+: Fetch shader performs all fetches, vertex buffer refs are stored as immediate user data */
	SI_OPENGL_FETCH_SUB_FLAT_TABLE,   /* SI+: Fetch shader performs all fetches, vertex buffer refs are stored in a flat table */
	SI_OPENGL_FETCH_SUB_PER_ATTRIB    /* SI+: Fetch shader performs a single attribute fetch per execution, vertex buffer refs stored in a flat table */
};



struct si_opengl_bin_shader_stats_t
{
	uint32_t uNumInst;
	uint32_t uNumTempReg;
	uint32_t uNumClauseTempReg;
	uint32_t uGPRPoolSize;
	uint32_t uNumFetchReg;
	uint32_t uNumExportReg;
	uint32_t uNumInterp;
	uint32_t uLoopNestingDepth;
	uint32_t uNumALUInst;
	uint32_t uNumTfetchInst;
	uint32_t uNumVfetchInst;
	uint32_t uNumMemExportInst;
	uint32_t uNumCflowInst;
	uint32_t uNumBoolConstReg;
	uint32_t uNumALUConstReg;
	uint32_t uNumIntConstReg;
	uint32_t uNumInternalALUConst;
	uint32_t uNumInternalTfetch;
}__attribute__((packed));

struct si_opengl_bin_shader_dep_t
{
	uint32_t MaxIntrlFConstants;
	uint32_t MaxIntrlIConstants;
	uint32_t MaxIntrlBConstants;

	uint32_t NumIntrlFConstants;
	uint32_t NumIntrlIConstants;
	uint32_t NumIntrlBConstants;

	uint32_t X32XIntrlFConstants;
	uint32_t X32XIntrlIConstants;
	uint32_t X32XIntrlBConstants;

	uint32_t MaxDefFConstants;
	uint32_t MaxDefIConstants;
	uint32_t MaxDefBConstants;

	uint32_t NumDefFConstants;
	uint32_t NumDefIConstants;
	uint32_t NumDefBConstants;

	uint32_t X32XDefFConstants;
	uint32_t X32XDefIConstants;
	uint32_t X32XDefBConstants;
}__attribute__((packed));

struct si_opengl_bin_constant_usage_t
{
	uint32_t maskBits[(MAX_CONSTANTS + 31) / 32];
}__attribute__((packed));


#define SI_OPENGL_BIN_SHADER_COMMON \
	uint32_t uSizeInBytes;	/* size of structure */            \
	struct si_opengl_bin_shader_stats_t stats;             /* hw-neutral stats */             \
	struct si_opengl_bin_shader_dep_t dep;	/* hw-neutral dependency */        \
	uint32_t X32XpPvtData;      /* SC-private data */              \
	uint32_t u32PvtDataSizeInBytes; /* size of private data */     \
	struct si_opengl_bin_constant_usage_t fConstantUsage;      /* float const usage */          \
	struct si_opengl_bin_constant_usage_t bConstantUsage;      /* int const usage */            \
	struct si_opengl_bin_constant_usage_t iConstantUsage;    \
	uint32_t uShaderType;        /* IL shader type */              \
	uint32_t eInstSet;          /* Instruction set */              \
	uint32_t texResourceUsage[(MAX_NUM_RESOURCE + 31) / 32];\
	uint32_t fetch4ResourceUsage[(MAX_NUM_RESOURCE + 31) / 32]; \
	uint32_t uavResourceUsage[(MAX_NUM_UAV + 31) / 32];     \
	uint32_t texSamplerUsage;   \
	uint32_t constBufUsage;     \
	uint32_t texSamplerResourceMapping[MAX_NUM_SAMPLER][(MAX_NUM_RESOURCE+31)/32]; \
	uint32_t NumConstOpportunities;               \
	uint32_t ResourcesAffectAlphaOutput[(MAX_NUM_RESOURCE+31)/32];        \

#define SI_OPENGL_BIN_SHADER_SI_COMMON \
	uint32_t X32XhShaderMemHandle;   \
	uint32_t X32XhConstBufferMemHandle[SC_SI_NUM_INTBUF]; \
	uint32_t CodeLenInByte;\
	uint32_t u32UserElementCount; /*Number of user data descriptors  */\
	struct si_bin_enc_user_element_t pUserElement[SC_SI_NUM_USER_ELEMENT]; /*User data descriptors */\
	/* Common HW shader info about registers and execution modes*/ \
	uint32_t   u32NumVgprs;  \
	uint32_t   u32NumSgprs;  \
	uint32_t   u32FloatMode; \
	bool     bIeeeMode;  \
	bool     bUsesPrimId; \
	bool     bUsesVertexId; \
	uint32_t   scratchSize;/* Scratch size in DWORDs for a single thread*/ \

#define SI_OPENGL_BIN_SHADER_SI_BASE \
	SI_OPENGL_BIN_SHADER_COMMON \
	SI_OPENGL_BIN_SHADER_SI_COMMON \

/* SPI_SHADER_PGM_RSRC2_VS */
struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t
{
	unsigned int unknown1	: 8;
	unsigned int unknown2	: 8;
	unsigned int unknown3	: 8;
	unsigned int user_sgpr	: 7;
	unsigned int unknown4	: 1;
};

/* PA_CL_VS_OUT_CNTL */
struct si_opengl_bin_pa_cl_vs_out_cntl_t
{
	unsigned int unknown1	: 8;
	unsigned int unknown2	: 8;
	unsigned int unknown3	: 8;
	unsigned int unknown4	: 8;	
};

/* SPI_VS_OUT_CONFIG */
struct si_opengl_spi_vs_out_config_t
{
	unsigned int vs_export_count		: 8;
	unsigned int vs_half_pack		: 8;
	unsigned int vs_export_fog 		: 8;
	unsigned int vs_out_fog_vec_addr 	: 8;
};

/* SPI_SHADER_POS_FORMAT */
struct si_opengl_spi_shader_pos_format
{
	unsigned int pos0_export_format 	: 8;
	unsigned int pos1_export_format 	: 8;
	unsigned int pos2_export_format 	: 8;
	unsigned int pos3_export_format 	: 8;	
};

/*
 *  Vertex shader input declaration to be used for semantic mapping with FS.
 */
struct si_opengl_bin_vs_semantic_mapping_in_t
{
	unsigned int usage: 8;  /* semantic usage. IL_IMPORTUSAGE. */
	unsigned int usageIdx  : 8;  /* semantic index. Opaque to SC. */
	unsigned int dataVgpr  : 8;  /* first VGPR to contain fetch result */
	unsigned int dataSize  : 2;  /* (fetch_size - 1), size in elements */
	unsigned int reserved  : 6;
}__attribute__((packed));

/*
 *  Vertex shader output declaration to be used for semantic mapping.
 *  The paramIdx is the index of the export parameter SC uses in the shader.
 */
struct si_opengl_bin_vs_semantic_mapping_out_t
{
	uint32_t usage     : 8;      /* semantic usage. IL_IMPORTUSAGE. */
	uint32_t usageIdx  : 8;      /* semantic index. Opaque to SC. */
	uint32_t paramIdx  : 8;      /* attribute export parameter index (0-31) */
	uint32_t reserved  : 8;
}__attribute__((packed));

/*
 *  Pixel shader input declaration to be used for semantic mapping.
 */
struct si_opengl_bin_fs_semantic_mapping_in_t
{
	uint32_t usage                  : 8;  // semantic usage. IL_IMPORTUSAGE.
	uint32_t usageIdx               : 8;  // semantic index. Opaque to SC.
	uint32_t inputIdx               : 8;  // PS input index.
	uint32_t defaultVal : 2;  // default val if VS doesn't export.
	uint32_t flatShade              : 1;  // set if interp mode is constant.
	uint32_t reserved               : 5;
}__attribute__((packed));

/*
 *  Pixel shader export patch .
 */
struct si_opengl_bin_fs_export_patch_into_t
{
	// Patch offset (in DWORDs) to start of an export sequence
	uint32_t patchOffset;
	// Conversion code snippet for each of the export formats
	uint32_t patchCode[SC_SI_NUM_EXPORT_FMT][SC_SI_MAX_EXPORT_CODE_SIZE];
}__attribute__((packed));


/*
 *  Flags to guide shader compilation.
 */
struct si_opengl_bin_compile_guide_t
{
	/* To make this structure consistent with SC_COMPILE_FLAGS, we add some reserved bits. */
	uint32_t psPrimId           : 1; /* PS needs primID input */
	uint32_t useHsOffChip       : 1; /* HS can use off-ship LDS */
	uint32_t clampScratchAccess : 1; /* add code to clamp scratch assesses */
	uint32_t streamOutEnable    : 1; /* enable writes to stream-out buffers */
	uint32_t reserved1          : 4;
	uint32_t useGsOnChip        : 1; /* ES and GS can use on-chip LDS. (CI+) */
	uint32_t reserved2          : 23;
}__attribute__((packed));

/* Inputs contains the index of vertex attribute array used in a shader program */
struct si_opengl_bin_enc_inputs_t
{
	unsigned int attributeIndex;
};

/* Encoding dictionary entry */
struct si_opengl_bin_enc_dict_entry_t
{
	/* Info */
	int num_vgpr_used;
	int num_sgpr_used;
	int lds_size_used;
	int stack_size_used;

	unsigned int userElementCount;
	struct si_bin_enc_user_element_t userElements[MAX_USER_ELEMENTS];

	unsigned int semanticMappingInCount;
	struct si_opengl_bin_vs_semantic_mapping_in_t semanticsMappingIn[SC_SI_VS_MAX_INPUTS];
	unsigned int semanticMappingOutCount;
	struct si_opengl_bin_vs_semantic_mapping_out_t semanticsMappingOut[SC_SI_VS_MAX_OUTPUTS];

	struct list_t *input_list; /* Equivalent to arg_list in OpenCL, elements of type si_input_t */

	/* FIXME: currently only designed for Vertex Shader */
	struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t *shader_pgm_rsrc2_vs;
};

/* FIXME: size doesn't match, should be 2124 bytes */
/* Vertex shader metadata stored in .text section */
struct si_opengl_bin_vertex_shader_metadata_t  
{
	/* SI Shader base structure, same for all shaders */
	SI_OPENGL_BIN_SHADER_SI_BASE

	/* Shader specific structures */
	/* Input semantics */
	uint32_t numVsInSemantics;
	struct si_opengl_bin_vs_semantic_mapping_in_t vsInSemantics[SC_SI_VS_MAX_INPUTS];

	/* Output semantics */
	uint32_t numVsOutSemantics;
	struct si_opengl_bin_vs_semantic_mapping_out_t vsOutSemantics[SC_SI_VS_MAX_OUTPUTS];

	/* LS/ES/VS specific shader resources */
	union
	{
		uint32_t spiShaderPgmRsrc2Ls;
		uint32_t spiShaderPgmRsrc2Es;
		uint32_t spiShaderPgmRsrc2Vs;
	};

	/* SC-provided values for certain VS-specific registers */
	uint32_t paClVsOutCntl;
	uint32_t spiVsOutConfig;
	uint32_t spiShaderPosFormat;
	uint32_t vgtStrmoutConfig;

	/* Number of SPI-generated VGPRs referenced by the vertex shader */
	uint32_t vgprCompCnt;

	/* Exported vertex size in DWORDs, can be used to program ESGS ring size */
	uint32_t exportVertexSize;

	bool useEdgeFlags;  
	bool  remapClipDistance; /*true if clip distance[4-7] is remapped to [0-3]  */
	uint32_t hwShaderStage;    /* hardware stage which this shader actually in when execution */
	struct si_opengl_bin_compile_guide_t compileFlags;     /* compile flag */
	uint32_t gsMode;                            /* gs mode */

	bool isOnChipGs;
	uint32_t targetLdsSize;

};

/* FIXME: size doesn't match, should be 3436 bytes */
/* Fragment shader metadata stored in .text section */
struct si_opengl_bin_fragment_shader_metadata_t
{
	/* SI Shader base structure, same for all shaders */
	SI_OPENGL_BIN_SHADER_SI_BASE

	// Input semantics
	uint32_t numPsInSemantics;
	struct si_opengl_bin_fs_semantic_mapping_in_t psInSemantics[SC_SI_PS_MAX_INPUTS];

	// Mapping of a PS interpolant to a texture coordinate index (0xffffffff if that interpolant is not a texture coordinate)
	uint32_t psInTexCoordIndex[SC_SI_PS_MAX_INPUTS];

	// PS specific shader resources
	uint32_t spiShaderPgmRsrc2Ps;

	uint32_t spiShaderZFormat;
	uint32_t spiPsInControl;
	uint32_t spiPsInputAddr;
	uint32_t spiPsInputEna;

	uint32_t spiBarycCntl;  // XXX mbalci: this would come from SCL(so SC) too; waiting for new interface change to be ported.

	uint32_t dbShaderControl;
	uint32_t cbShaderMask;

	// Size of each export patch fragment
	uint32_t exportPatchCodeSize;
	// Number of RT exports
	uint32_t numPsExports;
	// true if dual blending is enable
	bool dualBlending;
	// Export patch info per RT
	struct si_opengl_bin_fs_export_patch_into_t exportPatchInfo[SC_SI_PS_MAX_OUTPUTS];
	uint32_t defaultExportFmt;

};

/* texture resource and sampler binding */
struct si_opengl_texture_resource_bound_t
{
	uint32_t resourceId;         /* resource id */
	uint32_t samplerMask;        /* samplers bind to resource id */
};

/* FIXME: size doesn't match binary */
/* Info descriptor for .info section */
enum si_opengl_bin_info_max_offset
{ 
	MAX_PROGRAMINFO_OFFSET = 0x0ffff 
};

struct si_opengl_bin_info_t
{
	/* generaic shader resource information */

	/* texture */
	uint32_t fetch4Mask;                    /* Fetch4 mask */
	uint32_t textureMask;                   /* Texture unit masks */
	uint32_t textureSamplerMask;            /* texture sampler mask */
	uint32_t textureResourceMask[MAX_TEXTURE_RESOURCES/32]; /* texture resource mask */
	uint32_t bufferMask[MAX_TEXTURE_RESOURCES/32];  /* texture buffer mask */
	uint32_t textureResourceBoundCount;     /* the size of texture resource bound array */
	/* scratch */
	uint32_t maxScratchRegsNeeded;          /* scratch registers needed */
	/* constant buffer */
	uint32_t constantBufferMask;            /* constant buffer mask */
	/* uav */
	uint32_t uavOpIsUsed;                   /* mask for uav used delete!! */
	bool   uavInCB;                       /* UAV is in CB */
	uint32_t uavResourceCount;              /* size of uav resource mask array */
	/* atomic counter */
	uint32_t uavAtomicOpIsUsed;             /* mask for atomic op used */
	/* subroutine */
	int16_t  maxUniformLocation;             /* max explicit uniform location assigned by application */
	int16_t  maxSubroutineIndex;             /* max explicit Subroutine index assigned by application */
	int16_t  maxSubroutineUniformLocation;   /* max explicit Subroutine uniform location assigned by application */
	/* / union for per shader stage parameters */
	union
	{
		/// Vexter Shader,  Tessellation Evaluation Shader and Geometry Shader parameter
		struct
		{
			// VS input mask
			uint32_t inputStreamMask;                /* input stream mask (phsyical id) */
			bool   usesVertexID;                   /* tells whether this program uses VertexID */
			// transform feedback
			uint32_t streamOutStrideInDWORDs0;       /* streamout stride0 */
			uint32_t streamOutStrideInDWORDs1;       /* streamout stride1 */
			uint32_t streamOutStrideInDWORDs2;       /* streamout stride2 */
			uint32_t streamOutStrideInDWORDs3;       /* streamout stride3 */
			int8_t   streamOutBufferMapping[MAX_TRANSFORM_FEEDBACK_BUFFERS];   /* streamout buffer config */
			// vertex shader tessellation
			uint8_t  tessPrimType;                            /* tessellation shader primitive type (sclTessPrimType) */
			// viewport array index
			unsigned int   outputViewportArrayIndex :8;             /* true if output viewport array index */
			// svp members
			uint8_t  frontColorOutputReg;                     /* front color output register number */
			uint8_t  frontSecondaryColorOutputReg;            /* front secondary color output register number */
			uint8_t  backColorOutputReg;                      /* back color output register number */
			uint8_t  backSecondaryColorOutputReg;             /* back secondary color output register number */
			uint8_t  aaStippleTexCoord;                       /* Bitfield representing which texture cood will be used for aastipple patch */
			enum si_opengl_fetch_shader_type_t fsTypeForPassThruVS :8;   /* Fetch shader type (SVP PassThruVS) */
			uint8_t  fsReturnAddrRegForPassThruVS;            /* Fetch shader subroutine start SGPR (SVP PassThruVS) */
			uint8_t  fsInputStreamTableRegForPassThruVS;      /* Fetch shader input stream table start SGPR (SVP PassThruVS) */
			int32_t  fsAttribValidMaskReg;                    /* VPGR which Fetch shader should populate, if sparse buffers are used. */
		};
		/// Fragment Shader Parameters
		struct
		{
			uint32_t texKillPresent;                         /* Program uses texkill */
			int32_t  pointCoordReg;                          /* register number of gl_PointCoord which is an input of FS */
			uint8_t  outputColorMap[PS_MAX_OUTPUTS];  /* fragment shader color output map (from virtual to physical) */
			bool   useFlatInterpMode;                      /* if flat has been used on a varying */
			bool   forcePerSampleShading;                  /* true if the FS is required to run in per sample frequency */
			bool   uncached_writes;                        /* uncached writes */
			bool   outputDepth;                            /* true if fragment shader output depth */
			uint32_t usingDualBlendMask;                     /* indicates using an index = 1 for dual blending, in glsl layout */
		};
		/// Compute Shader Parameters
		struct
		{
			uint32_t numSharedGPRUser;    /* shared GPR */
			uint32_t numSharedGPRTotal;   /* shared GPR total including ones used by SC. */

			uint32_t numThreadPerGroup;   /* threads per group */
			uint32_t numThreadPerGroupX;  /* dimension x of NumThreadPerGroup */
			uint32_t numThreadPerGroupY;  /* dimension y of NumThreadPerGroup */
			uint32_t numThreadPerGroupZ;  /* dimension z of NumThreadPerGroup */
			uint32_t totalNumThreadGroup; /* total number of thread groups */
			uint32_t NumWavefrontPerSIMD; /* wavefronts per simd */
			bool   eCsSetupMode;        /* compute slow/fast mode */
			bool   IsMaxNumWavePerSIMD; /* Is this the max active num wavefronts per simd */
			bool   SetBufferForNumGroup;/* Need to set up buffer for info on number of thread groups? */
		};
		/// Fetch Shader Parameters
		struct
		{
			bool   usesVertexCache;      /* vertex cache used? (fetch shader only) */
		};
	};
	/* dynamic array, offset fields is valid in ELF package, int64_t is to keep the struct size fixed in all operation system. */
	/* texture resource bound array */
	union
	{
		struct si_opengl_texture_resource_bound_t *_textureResourceBound; /* texture resoruce and sampler bounding */
		int64_t textureResourceBoundOffset;                 /* resource binding array offset */
	};
	/* / uav resource mask array */
	union
	{
		uint32_t *uavResourceMask;                          /* UAV resource mask */
		int64_t   uavResourceMaskOffset;                    /* UAV resource mask array offset */
	};
	/* / uav return buffer */
	union
	{
		uint32_t *_uavRtnBufStride;       /* save stride of uav return buffer for each UAV */
		int64_t   uavRtnBufStrideOffset;                    /* uav return buffer stride array offset */
	};
	/* / uav dynamic resource map */
	union
	{
		uint32_t *_uavDynamicResouceMap;  /* save fetch const offset of each UAV */
		int64_t   uavDynamicResouceMapOffset;               /* uav dynamic resource map offset */
	};
	
	enum si_opengl_bin_info_max_offset max_valid_offset;
};

/* ARB program parameter */
struct si_opengl_bin_arb_program_parameter_t
{
	uint32_t instructions;              /* Number of instructions */
	uint32_t nativeInstructions;        /* Number of native instructions */
	uint32_t aluInstructions;           /* Number of alu instructions */
	uint32_t nativeAluInstructions;     /* Number of native alu instructions */
	uint32_t texInstructions;           /* Number of tex instructions */
	uint32_t nativeTexInstructions;     /* Number of native tex instructions */
	uint32_t texIndirections;           /* Number of tex indirections */
	uint32_t nativeTexIndirections;     /* Number of native tex indirections */
	uint32_t temporaries;               /* Number of temporaries */
	uint32_t nativeTemporaries;         /* Number of native temporaries */
	uint32_t parameters;                /* Number of parameters */
	uint32_t nativeParameters;          /* Number of native parameters */
	uint32_t attribs;                   /* Number of attribs */
	uint32_t nativeAttribs;             /* Number of native attribs */
	uint32_t addressRegisters;          /* Number of address registers */
	uint32_t nativeAddressRegisters;    /* Number of native address registers */
	bool   underNativeLimits;         /* Is shader under native limits? */
};

/* Structure in .usageinfo section */
struct si_opengl_bin_usageinfo_t
{
	struct si_opengl_bin_arb_program_parameter_t   arbProgramParameter;   /* ARB program parameters */
	uint16_t layoutParamMask;                            /* layout parameters mask, see type gllShaderLayoutType for detail info */
	bool usesPrimitiveID;                              /* Does this program use PrimitiveID */
	bool usesClipDistance;                             /* vp outputs clip distance(s) */

	uint32_t texImageMask;                               /* orignial teximage unit usage mask (without SC optimization and it isn't include patched texture stage) */
	uint32_t usesTexImageMask;                           /* teximge unit usage returned by SC */
	uint32_t usesShadowMask;                             /* shadow texture unit usage mask */
	uint32_t uavImageMask;                               /* original uav image usage mask */
	uint32_t uavImageMaskShaderStorageBuffer;            /* uav used by shader storage buffer */
	uint32_t usesBufMask;                                /* vertex tessellation buffer mask */

	uint32_t imageTypeAndFormatSize;                     /* size of array imageType and imageFormat */
	uint32_t textureTypeSize;                            /* size of array textureType */
	//
	/// per shader stage parameter
	//
	union
	{
		//
		/// Fragment Shader Parameters
		//
		struct
		{
			/* fs input usage */
			uint32_t usesTexCoordMask;                   /* texcoord unit usage mask */
			bool   usesFogCoord;                       /* fogcoord usage */
			bool   usePrimaryColor;                    /* true if primary color is used */
			bool   useSecondaryColor;                  /* true if secondary color is used */
			/* aa stipple */
			int8_t   aaStippleTexCoord;                  /* the texture coord used by SCL to simulate aa/stipple */
			int8_t   aaStippleTexUnit;                   /* the texture unit used by aa/stipple texture image */
			int8_t   aaStippleTexVirtUnit;               /* default logic unit for aa stipple texture unit */
			/* bitmap */
			int8_t   bitmapTexCoord;                     /* the texture coord used for bitmap drawing */
			int8_t   bitmapTexUnit;                      /* the texture unit used for bitmap drawing */
			int8_t   bitmapTexVirtUnit;                  /* default logic unit for bitmap drawing */
			/* misc fields */
			bool   needSampleInfoCBs;                  /* whether the FP needs the 2 constant buffers for the SAMPLEINFO, SAMPLEPOS and EVAL_SAMPLE_INDEX. */
			bool   earlyFragTest;                      /* true if early frag test is enabled */
			uint8_t  conservativeZ;                      /* 0:defult, 1:less, 2:greater */
		};
		//
		/// Geometry Shader Parametes
		//
		struct
		{
			uint16_t maxGSVertices[4];                   /* max gs output vertices */
			uint16_t gsInvocations;                      /* gs invocation number */
			uint8_t  inputTopology;                      /* gs input topology */
			uint8_t  outputTopology[4];                  /* gs output topology */
		};
		//
		/// Tessellation Evaluation Shader Parameters
		//
		struct
		{
			uint32_t tessOutputVertices;                 /* output control point number */
		};
		//
		/// Tessellation Control Shader Parameters
		//
		struct
		{
			uint32_t tessGenMode;                        /* domain mode */
			uint32_t tessGenSpacing;                     /* partition mode */
			uint32_t tessGenVertexOrder;                 /* output primitive mode */
			bool   tessGenPointMode;                   /* If point mode when tessellated */
		};
		//
		/// Vertex Shader Parametes
		//
		struct
		{
			bool   positionInvariant;                  /* Has the ARB_position_invariant option been requested */
			bool   enableAnotherSetAttribAlias;        /* if it is cg generated program or nv_vertex_program */
			uint32_t lsStride;                           /* The number of input-control-points per patch declared in HS */
			/* SI+ fetch shader parameters */
			int8_t   fsType;                             /* Fetch shader type (immediate vs flat), SI only */
			int8_t   fsReturnAddrReg;                    /* Fetch shader subroutine return address SGPR, SI only */
			int8_t   fsInputStreamTableReg;              /* Fetch shader input stream table start SGPR (either first data element or pointer depending on FS type), SI only */
			int8_t   fsVertexIdChannel;                  /* Fetch shader channel(R,G,B,A) to compute the vertexID with */
		};
		//
		/* Compute Shader Parameters */
		//
		struct
		{
			uint32_t  workDimension;                     /* Work dimension (1, 2 or 3 right now) */
			uint32_t  workSizeX;                         /* Work size in the X dimension */
			uint32_t  workSizeY;                         /* Work size in the Y dimension */
			uint32_t  workSizeZ;                         /* Work size in the Z dimension */
		};

	};

	enum si_opengl_bin_info_max_offset maxOffset;         /* max valid value for dynamic array offset */
	//
	/// texture type array
	//
	union
	{
		uint32_t* textureType;                         /* teximage unit usage */
		int64_t   textureTypeOffset;                   /* texture type array offset in elf section */
	};
	//
	/// uav image type array
	//
	union
	{
		uint32_t* imageType;                            /* UAV image usage */
		int64_t   imageTypeOffset;                      /* uav image type array offset in elf section */
	};
	//
	/// uav image format array
	//
	union
	{
		uint32_t* imageFormat;                          /* uav image format array */
		int64_t   imageFormatOffset;                    /* uav image format array offset in elf section */
	};

};

struct si_opengl_bin_vertex_shader_t
{
	/* Parent shader binary it belongs to */
	struct si_opengl_shader_binary_t *parent;

	/* Shader info extracted from sections in parent shader binary */
	struct si_opengl_bin_vertex_shader_metadata_t *meta;
	struct list_t *inputs; /* Elements with type struct si_opengl_bin_input_t */
	struct list_t *outputs; /* Elements with type struct si_opengl_bin_output_t*/
	struct si_opengl_bin_info_t *info;
	struct si_opengl_bin_usageinfo_t *usageinfo;
};

struct si_opengl_bin_fragment_shader_t
{
	/* Parent binary it belongs to */
	struct si_opengl_shader_binary_t *parent;

	/* Shader info extracted from sections in parent shader binary */
	struct si_opengl_bin_fragment_shader_metadata_t *meta;
	struct list_t *inputs; /* Elements with type struct si_opengl_bin_input_t */
	struct list_t *outputs; /* Elements with type struct si_opengl_bin_output_t*/
	struct si_opengl_bin_info_t *info;
	struct si_opengl_bin_usageinfo_t *usageinfo;
};

/* Input descriptor for .inputs section */
struct si_opengl_bin_input_t
{
	enum si_opengl_bin_input_type_t type;
	unsigned int voffset;
	unsigned int poffset;
	// bool isFloat16;  FIXME: has to comment this as the size will be 29 instead of 28 bytes  
	enum si_opengl_bin_input_swizzle_type_t swizzles[4];
}__attribute__((packed));

/* FIXME: size of this structure doesn't match the binary */
/* Output descriptor for .outputs section */
struct si_opengl_bin_output_t 
{
	union
	{
		enum si_opengl_bin_output_type_t type;          /* Semantic type */
		enum si_opengl_bin_symbol_datatype_t data_type;      /* Data type */
	};
	unsigned int voffset;           /* Virtual offset */
	unsigned int poffset;           /* Physical offset */
	unsigned int array_size;     /* Array size */
	char* name;           /* Name of the output */
}__attribute__((packed));;

typedef void (*si_opengl_shader_free_func_t)(void *);
struct si_opengl_shader_binary_t
{
	/* Type of shader */
	enum si_opengl_shader_binary_kind_t shader_kind;

	/* ELF-formatted shader, it's embedded in the .internal 
	 * section of a shader binary */
	struct elf_file_t *shader_elf;

	/* Pointer to ISA */
	struct elf_buffer_t *shader_isa;

	/* Encoding dictionary */
	struct si_opengl_bin_enc_dict_entry_t *shader_enc_dict;

	void *shader; /* Shader with type si_opengl_bin_xxxxx_shader_t */
	si_opengl_shader_free_func_t free_func;  /* Callback function to free shader */
};

struct si_opengl_program_binary_t
{
	/* Name of the associated shader binary file */
	char *name;

	/* Associated ELF-format shader binary */
	struct elf_file_t *binary;

	/* List of shaders in shader binary, elements with type si_opengl_shader_binary_t */
	struct list_t *shaders;
};

struct si_opengl_program_binary_t *si_opengl_program_binary_create(void *buffer_ptr, int size, char *name);
void si_opengl_program_binary_free(struct si_opengl_program_binary_t *program_bin);

struct si_opengl_shader_binary_t *si_opengl_shader_binary_create(void *buffer, int size, char* name);
void si_opengl_shader_binary_free(struct si_opengl_shader_binary_t *shdr);

struct si_bin_enc_user_element_t *si_opengl_bin_enc_user_element_create();
void si_opengl_bin_enc_user_element_free(struct si_bin_enc_user_element_t *user_elem);




#endif

