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

#include "Wrapper.h"


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

/* Shader types */
enum opengl_si_shader_binary_kind_t
{
	OPENGL_SI_SHADER_VERTEX = 0,
	OPENGL_SI_SHADER_GEOMETRY,
	OPENGL_SI_SHADER_HULL, /* aka Tess Control shader */
	OPENGL_SI_SHADER_DOMAIN, /* aka Tess Evaluastion shader */
	OPENGL_SI_SHADER_PIXEL = 4, /* aka Fragment shader */
	OPENGL_SI_SHADER_COMPUTE,
	OPENGL_SI_SHADER_INVALID
};

enum opengl_si_bin_input_swizzle_type_t
{
	OPENGL_SI_SWIZZLE_X,	/* Swizzle the X component into this component */
	OPENGL_SI_SWIZZLE_Y,	/* Swizzle the Y component into this component */
	OPENGL_SI_SWIZZLE_Z,	/* Swizzle the Z component into this component */
	OPENGL_SI_SWIZZLE_W,	/* Swizzle the W component into this component */
	OPENGL_SI_SWIZZLE_0,	/* Swizzle constant 0 into this component */
	OPENGL_SI_SWIZZLE_1	/* Swizzle constant 1 into this component */
};

enum opengl_si_bin_input_type_t
{
	OPENGL_SI_INPUT_ATTRIB,	/* generic attribute */
	OPENGL_SI_INPUT_COLOR,	/* primary color */
	OPENGL_SI_INPUT_SECONDARYCOLOR,	/* secondary color */
	OPENGL_SI_INPUT_TEXCOORD,	/* texture coordinate */
	OPENGL_SI_INPUT_TEXID,	/* texture unit id */
	OPENGL_SI_INPUT_BUFFERID,	/* buffer unit id */
	OPENGL_SI_INPUT_CONSTANTBUFFERID,	/* constant buffer unit id */
	OPENGL_SI_INPUT_TEXTURERESOURCEID	/* texture resource id	 */
};

enum opengl_si_bin_output_type_t
{
	OPENGL_SI_OUTPUT_POS,	/* Position */
	OPENGL_SI_OUTPUT_POINTSIZE,	/* Point size */
	OPENGL_SI_OUTPUT_COLOR,	/* Primary color, offset 0 is front, offset 1 is back */
	OPENGL_SI_OUTPUT_SECONDARYCOLOR,	/* Secondary color, offset 0 is front, offset 1 is back */
	OPENGL_SI_OUTPUT_GENERIC,	/* Texture coordinate and user define varyings for pre R5xx asics, but on R6xx above, generic contains colors */
	OPENGL_SI_OUTPUT_DEPTH,	/* Fragment depth */
	OPENGL_SI_OUTPUT_CLIPDISTANCE,	/* Clip distance */
	OPENGL_SI_OUTPUT_PRIMITIVEID,	/* PrimitiveID */
	OPENGL_SI_OUTPUT_LAYER,	/* Layer */
	OPENGL_SI_OUTPUT_VIEWPORTINDEX,	/* viewportindex */
	OPENGL_SI_OUTPUT_STENCIL,	/* Fragment stencil ref value */
	OPENGL_SI_OUTPUT_STENCIL_VALUE,	/* Fragment stencil operation value */
	OPENGL_SI_OUTPUT_SAMPLEMASK,	/* Output sample coverage mask (FS only) */
	OPENGL_SI_OUTPUT_STREAMID	/* Ouput stream id */
};

/* Symbol types */
enum opengl_si_bin_symbol_type_t
{
	OPENGL_SI_SYMBOL_UNIFORM,                      /* GLSL uniform */
	OPENGL_SI_SYMBOL_BINDABLE_UNIFORM,             /* GLSL bindable uniform */
	OPENGL_SI_SYMBOL_UNIFORM_BLOCK,                /* GLSL uniform block */
	OPENGL_SI_SYMBOL_ATTRIB_POSITION,              /* Vertex position */
	OPENGL_SI_SYMBOL_ATTRIB_NORMAL,                /* Vertex normal */
	OPENGL_SI_SYMBOL_ATTRIB_PRIMARY_COLOR,         /* Vertex primary color */
	OPENGL_SI_SYMBOL_ATTRIB_SECONDARY_COLOR,       /* Vertex secondary color */
	OPENGL_SI_SYMBOL_ATTRIB_FOGCOORD,              /* Vertex fog coord */
	OPENGL_SI_SYMBOL_ATTRIB_EDGEFLAG,              /* Vertex edge flag */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD0,             /* Vertex texture coord 0 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD1,             /* Vertex texture coord 1 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD2,             /* Vertex texture coord 2 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD3,             /* Vertex texture coord 3 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD4,             /* Vertex texture coord 4 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD5,             /* Vertex texture coord 5 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD6,             /* Vertex texture coord 6 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD7,             /* Vertex texture coord 7 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD8,             /* Vertex texture coord 8 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD9,             /* Vertex texture coord 9 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD10,            /* Vertex texture coord 10 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD11,            /* Vertex texture coord 11 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD12,            /* Vertex texture coord 12 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD13,            /* Vertex texture coord 13 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD14,            /* Vertex texture coord 14 */
	OPENGL_SI_SYMBOL_ATTRIB_TEXCOORD15,            /* Vertex texture coord 15 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC0,              /* Generic attrib 0 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC1,              /* Generic attrib 1 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC2,              /* Generic attrib 2 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC3,              /* Generic attrib 3 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC4,              /* Generic attrib 4 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC5,              /* Generic attrib 5 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC6,              /* Generic attrib 6 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC7,              /* Generic attrib 7 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC8,              /* Generic attrib 8 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC9,              /* Generic attrib 9 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC10,             /* Generic attrib 10 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC11,             /* Generic attrib 11 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC12,             /* Generic attrib 12 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC13,             /* Generic attrib 13 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC14,             /* Generic attrib 14 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC15,             /* Generic attrib 15 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC16,             /* Generic attrib 16 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC17,             /* Generic attrib 17 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC18,             /* Generic attrib 18 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC19,             /* Generic attrib 19 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC20,             /* Generic attrib 20 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC21,             /* Generic attrib 21 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC22,             /* Generic attrib 22 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC23,             /* Generic attrib 23 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC24,             /* Generic attrib 24 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC25,             /* Generic attrib 25 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC26,             /* Generic attrib 26 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC27,             /* Generic attrib 27 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC28,             /* Generic attrib 28 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC29,             /* Generic attrib 29 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC30,             /* Generic attrib 30 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC31,             /* Generic attrib 31 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC32,             /* Generic attrib 32 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC33,             /* Generic attrib 33 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC34,             /* Generic attrib 34 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC35,             /* Generic attrib 35 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC36,             /* Generic attrib 36 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC37,             /* Generic attrib 37 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC38,             /* Generic attrib 38 */
	OPENGL_SI_SYMBOL_ATTRIB_GENERIC39,             /* Generic attrib 39 */
	OPENGL_SI_SYMBOL_FSVARYINGOUT,                 /* Varying out */
	OPENGL_SI_SYMBOL_ATTRIB_VERTEXID,              /* VertexID */
	OPENGL_SI_SYMBOL_ATTRIB_INSTANCEID,            /* InstanceID */
	OPENGL_SI_SYMBOL_SUBROUTINE_UNIFORM,           /* GLSL Subroutine Uniform */
	OPENGL_SI_SYMBOL_SUBROUTINE_FUNCTION,          /* GLSL Subroutine Function */
	OPENGL_SI_SYMBOL_SEPARATE_VARYING_IN_BASE,     /* base varying in symbol of separate mode */
	OPENGL_SI_SYMBOL_SEPARATE_VARYING_OUT_BASE,    /* base varying out symbol of sparate mode */
	OPENGL_SI_SYMBOL_SEPARATE_VARYING_IN_DERIVED,  /* derived varying in symbol of separate mode */
	OPENGL_SI_SYMBOL_SEPARATE_VARYING_OUT_DERIVED, /* derived varying out symbol of sparate mode */
	OPENGL_SI_SYMBOL_NONSEPARATE_VARYING_IN,       /* varying in of nonseparate mode */
	OPENGL_SI_SYMBOL_NONSEPARATE_VARYING_OUT,      /* varying out of nonseparate mode */
	OPENGL_SI_SYMBOL_SHADER_STORAGE_BUFFER,        /* shader storage buffer */
	OPENGL_SI_SYMBOL_ATTRIB_INPUTVALID,            /* input valide bits for sparse texture */
};


/* Symbol data types */
enum opengl_si_bin_symbol_datatype_t
{
	OPENGL_SI_SYMBOL_DATATYPE_VOID,                   /* void */
	OPENGL_SI_SYMBOL_DATATYPE_BOOL,                   /* bool */
	OPENGL_SI_SYMBOL_DATATYPE_INT,                    /* int */
	OPENGL_SI_SYMBOL_DATATYPE_UINT,                   /* unsigned int */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT,                  /* float */
	OPENGL_SI_SYMBOL_DATATYPE_DOUBLE,                 /* double */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16,                /* float16 */
	OPENGL_SI_SYMBOL_DATATYPE_DVEC2,                  /* dvec2 */
	OPENGL_SI_SYMBOL_DATATYPE_DVEC3,                  /* dvec3 */
	OPENGL_SI_SYMBOL_DATATYPE_DVEC4,                  /* dvec4 */
	OPENGL_SI_SYMBOL_DATATYPE_VEC2,                   /* vec2 */
	OPENGL_SI_SYMBOL_DATATYPE_VEC3,                   /* vec3 */
	OPENGL_SI_SYMBOL_DATATYPE_VEC4,                   /* vec4 */
	OPENGL_SI_SYMBOL_DATATYPE_BVEC2,                  /* bvec2 */
	OPENGL_SI_SYMBOL_DATATYPE_BVEC3,                  /* bvec3 */
	OPENGL_SI_SYMBOL_DATATYPE_BVEC4,                  /* bvec4 */
	OPENGL_SI_SYMBOL_DATATYPE_IVEC2,                  /* ivec2 */
	OPENGL_SI_SYMBOL_DATATYPE_IVEC3,                  /* ivec3 */
	OPENGL_SI_SYMBOL_DATATYPE_IVEC4,                  /* ivec4 */
	OPENGL_SI_SYMBOL_DATATYPE_UVEC2,                  /* unsigned ivec2 */
	OPENGL_SI_SYMBOL_DATATYPE_UVEC3,                  /* unsigned ivec3 */
	OPENGL_SI_SYMBOL_DATATYPE_UVEC4,                  /* unsigned ivec4 */
	OPENGL_SI_SYMBOL_DATATYPE_F16VEC2,                /* f16vec2 */
	OPENGL_SI_SYMBOL_DATATYPE_F16VEC3,                /* f16vec3 */
	OPENGL_SI_SYMBOL_DATATYPE_F16VEC4,                /* f16vec4 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT2,                   /* mat2 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT2X3,                 /* mat2x3 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT2X4,                 /* mat2x4 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT3X2,                 /* mat3x2 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT3,                   /* mat3 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT3X4,                 /* mat3x4 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT4X2,                 /* mat4x2 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT4X3,                 /* mat4x3 */
	OPENGL_SI_SYMBOL_DATATYPE_MAT4,                   /* mat4 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT2,                  /* dmat2 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT2X3,                /* dmat2x3 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT2X4,                /* dmat2x4 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT3X2,                /* dmat3x2 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT3,                  /* dmat3 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT3X4,                /* dmat3x4 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT4X2,                /* dmat4x2 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT4X3,                /* dmat4x3 */
	OPENGL_SI_SYMBOL_DATATYPE_DMAT4,                  /* dmat4 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT2,                /* f16mat2 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT2X3,              /* f16mat2x3 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT2X4,              /* f16mat2x4 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT3X2,              /* f16mat3x2 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT3,                /* f16mat3 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT3X4,              /* f16mat3x4 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT4X2,              /* f16mat4x2 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT4X3,              /* f16mat4x3 */
	OPENGL_SI_SYMBOL_DATATYPE_F16MAT4,                /* f16mat4 */

	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D,             /* sampler1D */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D,             /* sampler2D */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_RECT,        /* sampler2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_EXTERNAL,       /* samplerExternalOES */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_3D,             /* sampler3D */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE,           /* samplerCube */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D_ARRAY,       /* sampler1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_ARRAY,       /* sampler2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE_ARRAY,        /*samplerCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_BUFFER,         /* samplerBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_RENDERBUFFER,   /* samplerRenderbuffer */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_MS,          /* sampler2DMS */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_MS_ARRAY,    /* sampler2DMSArray */

	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D_SHADOW,      /* sampler1DShadow */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_SHADOW,      /* sampler2DShadow */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_RECT_SHADOW, /* sampler2DRectShadow */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE_SHADOW,    /* samplerCubeShadow */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_1D_ARRAY_SHADOW,/* sampler1DArrayShadow */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_2D_ARRAY_SHADOW,/* sampler2DArrayShadow */
	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_CUBE_ARRAY_SHADOW, /*samplerCubeArrayShadow */
	                                                  ///
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_1D,         /* isampler1D */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D,         /* isampler2D */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_RECT,    /* isampler2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_3D,         /* isampler3D */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_CUBE,       /* isamplerCube */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_1D_ARRAY,   /* isampler1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_ARRAY,   /* isampler2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_CUBE_ARRAY, /* isamplerCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_BUFFER,     /* isamplerBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_RENDERBUFFER,   /* isamplerRenderbuffer */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_MS,          /* isampler2DMS */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_2D_MS_ARRAY,    /* isampler2DMSArray */

	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_1D,         /* usampler1D */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D,         /* usampler2D */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_RECT,    /* usampler2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_3D,         /* usampler3D */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE,       /* usamplerCube */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_1D_ARRAY,   /* usampler1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_ARRAY,   /* usampler2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE_ARRAY, /* usamplerCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_BUFFER,     /* usamplerBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_RENDERBUFFER,   /* usamplerRenderbuffer */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_MS,          /* usampler2DMS */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_MS_ARRAY,    /* usampler2DMSArray */

	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D,             /* f16sampler1D */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D,             /* f16sampler2D */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_RECT,        /* f16sampler2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_3D,             /* f16sampler3D */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE,           /* f16samplerCube */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_ARRAY,       /* f16sampler1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_ARRAY,       /* f16sampler2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_ARRAY,        /*f16samplerCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_BUFFER,         /* f16samplerBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_MS,          /* f16sampler2DMS */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_MS_ARRAY,    /* f16sampler2DMSArray */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_SHADOW,      /* f16sampler1DShadow */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_SHADOW,      /* f16sampler2DShadow */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_RECT_SHADOW, /* f16sampler2DRectShadow */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_SHADOW,    /* f16samplerCubeShadow */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_1D_ARRAY_SHADOW,/* f16sampler1DArrayShadow */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_2D_ARRAY_SHADOW,/* f16sampler2DArrayShadow */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_SAMPLER_CUBE_ARRAY_SHADOW,/* f16samplerCubeArrayShadow */

	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_1D,                        /* image1D */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D,                        /* image2D */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_3D,                        /* image3D */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_RECT,                   /* image2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_CUBEMAP,                   /* imageCube */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_BUFFER,                    /* imageBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_1D_ARRAY,                  /* image1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_ARRAY,                  /* image2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_CUBEMAP_ARRAY,             /* imageCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_MS,                     /* image2DMultisample */
	OPENGL_SI_SYMBOL_DATATYPE_IMAGE_2D_MS_ARRAY,               /* image2DMultisampleArray */

	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_1D,                    /* iimage1D */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D,                    /* iimage2D */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_3D,                    /* iimage3D */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_RECT,               /* iimage2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_CUBEMAP,               /* iimageCube */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_BUFFER,                /* iimageBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_1D_ARRAY,              /* iimage1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_ARRAY,              /* iimage2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_CUBEMAP_ARRAY,         /* iimageCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_MS,                 /* iimage2DMultisample */
	OPENGL_SI_SYMBOL_DATATYPE_INT_IMAGE_2D_MS_ARRAY,           /* iimage2DMultisampleArray */

	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_1D,           /* uimage1D */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D,           /* uimage2D */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_3D,           /* uimage3D */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_RECT,      /* uimage2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_CUBEMAP,      /* uimageCube */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_BUFFER,       /* uimageBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_1D_ARRAY,     /* uimage1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_ARRAY,     /* uimage2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_CUBEMAP_ARRAY,/* uimageCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_MS,        /* uimage2DMultisample */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_IMAGE_2D_MS_ARRAY,  /* uimage2DMultisampleArray */

	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_1D,                /* f16image1D */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D,                /* f16image2D */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_3D,                /* f16image3D */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_RECT,           /* f16image2DRect */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_CUBEMAP,           /* f16imageCube */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_BUFFER,            /* f16imageBuffer */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_1D_ARRAY,          /* f16image1DArray */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_ARRAY,          /* f16image2DArray */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_CUBEMAP_ARRAY,     /* f16imageCubeArray */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_MS,             /* f16image2DMultisample */
	OPENGL_SI_SYMBOL_DATATYPE_FLOAT16_IMAGE_2D_MS_ARRAY,       /* f16image2DMultisampleArray */

	OPENGL_SI_SYMBOL_DATATYPE_SAMPLER_BUFFER_AMD,                 /* used for tessellation (from vbo) */
	OPENGL_SI_SYMBOL_DATATYPE_INT_SAMPLER_BUFFER_AMD,             /* used for vertex shader tesssellation */
	OPENGL_SI_SYMBOL_DATATYPE_UNSIGNED_INT_SAMPLER_BUFFER_AMD,    /* used for vertex shader tesssellation */

	OPENGL_SI_SYMBOL_DATATYPE_ATOMIC_COUNTER,          /* atomic counter */

	OPENGL_SI_SYMBOL_DATATYPE_STRUCT,                 /* struct, used for bindable uniform, it is also used for type check in separate program mode */
	OPENGL_SI_SYMBOL_DATATYPE_INTERFACE,              /* interface block, used for type check in separate progarm mode */
	OPENGL_SI_SYMBOL_DATATYPE_FIRST = OPENGL_SI_SYMBOL_DATATYPE_VOID,
	OPENGL_SI_SYMBOL_DATATYPE_LAST  = OPENGL_SI_SYMBOL_DATATYPE_INTERFACE
};

enum opengl_si_fetch_shader_type_t
{
	OPENGL_SI_FETCH_FIXED_FUNCTION,   /* Fetch shader executes all vertex fetches and start address is in a fixed function register. */
	OPENGL_SI_FETCH_SUB_IMMEDIATE,    /* SI+: Fetch shader performs all fetches, vertex buffer refs are stored as immediate user data */
	OPENGL_SI_FETCH_SUB_FLAT_TABLE,   /* SI+: Fetch shader performs all fetches, vertex buffer refs are stored in a flat table */
	OPENGL_SI_FETCH_SUB_PER_ATTRIB    /* SI+: Fetch shader performs a single attribute fetch per execution, vertex buffer refs stored in a flat table */
};

//
/// Swizzle types
//
typedef enum {
	OPENGL_SI_ELF_SWIZZLE_X,
	OPENGL_SI_ELF_SWIZZLE_Y,
	OPENGL_SI_ELF_SWIZZLE_Z,
	OPENGL_SI_ELF_SWIZZLE_W,
	OPENGL_SI_ELF_SWIZZLE_UNKNOWN
} SIElfSwizzle;


//
/// Symbol data precision qualifier
//
typedef enum 
{
	OPENGL_SI_ELF_SYMBOL_DATAPQ_LOW,
	OPENGL_SI_ELF_SYMBOL_DATAPQ_MEDIUM,
	OPENGL_SI_ELF_SYMBOL_DATAPQ_HIGH,
	OPENGL_SI_ELF_SYMBOL_DATAPQ_DOUBLE
} SIElfSymbolDataPrecisionQualifier;

//
/// Style for layout qualifier
//
typedef enum
{
	OPENGL_SI_ELF_LAYOUT_STYLE_NONE,
	OPENGL_SI_ELF_LAYOUT_SHARED,
	OPENGL_SI_ELF_LAYOUT_PACKED,
	OPENGL_SI_ELF_LAYOUT_STD140,
} SIElfLayoutStyleQualifier;

//
/// Major for layout qualifier
//
typedef enum
{
	OPENGL_SI_ELF_LAYOUT_MAJOR_NONE,
	OPENGL_SI_ELF_LAYOUT_ROW_MAJOR,
	OPENGL_SI_ELF_LAYOUT_COLUMN_MAJOR,
} SIElfLayoutMajorQualifier;

//
/// invariant qualifier
///
typedef enum
{
	OPENGL_SI_ELF_INVARIANT_OFF,
	OPENGL_SI_ELF_INVARIANT_ON,
}SIElfInvariantQualifier;

//
/// precise qualifier
//
typedef enum
{
	OPENGL_SI_ELF_PRECISE_OFF,
	OPENGL_SI_ELF_PRECISE_ON,
}SIElfPreciseQualifier;

//
/// storage qualifier
///
typedef enum
{
	OPENGL_SI_ELF_STORAGE_DEFAULT,
	OPENGL_SI_ELF_STORAGE_CENTROID,
	OPENGL_SI_ELF_STORAGE_SAMPLE,
}SIElfStorageQualifier;

//
/// interpolation qualifier
///
typedef enum
{
	OPENGL_SI_ELF_INTERPOLATION_SMOOTH,
	OPENGL_SI_ELF_INTERPOLATION_FLAT,
	OPENGL_SI_ELF_INTERPOLATION_NOPERSPECTIVE,
}SIElfInterpolationQualifier;

//
/// patch qualifier
//
typedef enum
{
	OPENGL_SI_ELF_PER_PATCH_OFF,
	OPENGL_SI_ELF_PER_PATCH_ON,
}SIElfPatchQualifier;

struct opengl_si_bin_shader_stats_t
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

struct opengl_si_bin_shader_dep_t
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

struct opengl_si_bin_constant_usage_t
{
	uint32_t maskBits[(MAX_CONSTANTS + 31) / 32];
}__attribute__((packed));


#define OPENGL_SI_BIN_SHADER_COMMON \
	uint32_t uSizeInBytes;	/* size of structure */            \
	struct opengl_si_bin_shader_stats_t stats;             /* hw-neutral stats */             \
	struct opengl_si_bin_shader_dep_t dep;	/* hw-neutral dependency */        \
	uint32_t X32XpPvtData;      /* SC-private data */              \
	uint32_t u32PvtDataSizeInBytes; /* size of private data */     \
	struct opengl_si_bin_constant_usage_t fConstantUsage;      /* float const usage */          \
	struct opengl_si_bin_constant_usage_t bConstantUsage;      /* int const usage */            \
	struct opengl_si_bin_constant_usage_t iConstantUsage;    \
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

#define OPENGL_SI_BIN_SHADER_SI_COMMON \
	uint32_t X32XhShaderMemHandle;   \
	uint32_t X32XhConstBufferMemHandle[SC_SI_NUM_INTBUF]; \
	uint32_t CodeLenInByte;\
	uint32_t u32UserElementCount; /*Number of user data descriptors  */\
	struct SIBinaryUserElement pUserElement[SC_SI_NUM_USER_ELEMENT]; /*User data descriptors */\
	/* Common HW shader info about registers and execution modes*/ \
	uint32_t   u32NumVgprs;  \
	uint32_t   u32NumSgprs;  \
	uint32_t   u32FloatMode; \
	bool     bIeeeMode;  \
	bool     bUsesPrimId; \
	bool     bUsesVertexId; \
	uint32_t   scratchSize;/* Scratch size in DWORDs for a single thread*/ \

#define OPENGL_SI_BIN_SHADER_SI_BASE \
	OPENGL_SI_BIN_SHADER_COMMON \
	OPENGL_SI_BIN_SHADER_SI_COMMON \

struct opengl_si_bin_shader_common_t
{
	uint32_t uSizeInBytes;	/* size of structure */
	struct opengl_si_bin_shader_stats_t stats;             /* hw-neutral stats */
	struct opengl_si_bin_shader_dep_t dep;	/* hw-neutral dependency */
	uint32_t X32XpPvtData;      /* SC-private data */
	uint32_t u32PvtDataSizeInBytes; /* size of private data */
	struct opengl_si_bin_constant_usage_t fConstantUsage;      /* float const usage */
	struct opengl_si_bin_constant_usage_t bConstantUsage;      /* int const usage */
	struct opengl_si_bin_constant_usage_t iConstantUsage;   
	uint32_t uShaderType;        /* IL shader type */
	uint32_t eInstSet;          /* Instruction set */
	uint32_t texResourceUsage[(MAX_NUM_RESOURCE + 31) / 32];
	uint32_t fetch4ResourceUsage[(MAX_NUM_RESOURCE + 31) / 32];
	uint32_t uavResourceUsage[(MAX_NUM_UAV + 31) / 32];    
	uint32_t texSamplerUsage;  
	uint32_t constBufUsage;    
	uint32_t texSamplerResourceMapping[MAX_NUM_SAMPLER][(MAX_NUM_RESOURCE+31)/32];
	uint32_t NumConstOpportunities;              
	uint32_t ResourcesAffectAlphaOutput[(MAX_NUM_RESOURCE+31)/32];       
};

struct opengl_si_bin_shader_si_common_t
{
	uint32_t X32XhShaderMemHandle;   
	uint32_t X32XhConstBufferMemHandle[SC_SI_NUM_INTBUF]; 
	uint32_t CodeLenInByte;
	uint32_t u32UserElementCount; /*Number of user data descriptors  */
	struct SIBinaryUserElement pUserElement[SC_SI_NUM_USER_ELEMENT]; /*User data descriptors */
	/* Common HW shader info about registers and execution modes*/ 
	uint32_t   u32NumVgprs;  
	uint32_t   u32NumSgprs;  
	uint32_t   u32FloatMode; 
	bool     bIeeeMode;  
	bool     bUsesPrimId; 
	bool     bUsesVertexId; 
	uint32_t   scratchSize;/* Scratch size in DWORDs for a single thread*/ 
}__attribute__((packed));

struct opengl_si_bin_shader_base_t
{
	uint32_t uSizeInBytes;	/* size of structure */
	struct opengl_si_bin_shader_stats_t stats;             /* hw-neutral stats */
	struct opengl_si_bin_shader_dep_t dep;	/* hw-neutral dependency */
	uint32_t X32XpPvtData;      /* SC-private data */
	uint32_t u32PvtDataSizeInBytes; /* size of private data */
	struct opengl_si_bin_constant_usage_t fConstantUsage;      /* float const usage */
	struct opengl_si_bin_constant_usage_t bConstantUsage;      /* int const usage */
	struct opengl_si_bin_constant_usage_t iConstantUsage;   
	uint32_t uShaderType;        /* IL shader type */
	uint32_t eInstSet;          /* Instruction set */
	uint32_t texResourceUsage[(MAX_NUM_RESOURCE + 31) / 32];
	uint32_t fetch4ResourceUsage[(MAX_NUM_RESOURCE + 31) / 32];
	uint32_t uavResourceUsage[(MAX_NUM_UAV + 31) / 32];    
	uint32_t texSamplerUsage;  
	uint32_t constBufUsage;    
	uint32_t texSamplerResourceMapping[MAX_NUM_SAMPLER][(MAX_NUM_RESOURCE+31)/32];
	uint32_t NumConstOpportunities;              
	uint32_t ResourcesAffectAlphaOutput[(MAX_NUM_RESOURCE+31)/32];       

	uint32_t X32XhShaderMemHandle;   
	uint32_t X32XhConstBufferMemHandle[SC_SI_NUM_INTBUF]; 
	uint32_t CodeLenInByte;
	uint32_t u32UserElementCount; /*Number of user data descriptors  */
	struct SIBinaryUserElement pUserElement[SC_SI_NUM_USER_ELEMENT]; /*User data descriptors */
	/* Common HW shader info about registers and execution modes*/ 
	uint32_t   u32NumVgprs;  
	uint32_t   u32NumSgprs;  
	uint32_t   u32FloatMode; 
	bool     bIeeeMode;  
	bool     bUsesPrimId; 
	bool     bUsesVertexId; 
	uint32_t   scratchSize;/* Scratch size in DWORDs for a single thread*/ 
}__attribute__((packed));


/* SPI_SHADER_PGM_RSRC2_VS */
struct opengl_si_bin_spi_shader_pgm_rsrc2_vs_t
{
	unsigned int scratch_en		: 1;
	unsigned int user_sgpr		: 5;
	unsigned int trap_present		: 1;
	unsigned int offchip_lds_pointer_en	: 1;
	unsigned int streamout_offset0	: 1;
	unsigned int streamout_offset1	: 1;
	unsigned int streamout_offset2	: 1;
	unsigned int streamout_offset3	: 1;
	unsigned int streamout_en		: 1;
	unsigned int reserved			: 19;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_PS */
struct opengl_si_bin_spi_shader_pgm_rsrc2_ps_t
{
	unsigned int scratch_en		: 1;
	unsigned int user_sgpr		: 5;
	unsigned int trap_present		: 1;
	unsigned int unknown_en		: 1;
	unsigned int reserved 		: 24;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_GS */
struct opengl_si_bin_spi_shader_pgm_rsrc2_gs_t
{
	unsigned int scratch_en		: 1;
	unsigned int user_sgpr		: 5;
	unsigned int trap_present		: 1;
	unsigned int reserved 		: 25;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_ES */
struct opengl_si_bin_spi_shader_pgm_rsrc2_es_t
{
	unsigned int scratch_en		: 1;
	unsigned int user_sgpr		: 5;
	unsigned int trap_present		: 1;
	unsigned int offchip_lds_pointer_en	: 1;
	unsigned int reserved 		: 24;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_HS */
struct opengl_si_bin_spi_shader_pgm_rsrc2_hs_t
{
	unsigned int scratch_en		: 1;
	unsigned int user_sgpr		: 5;
	unsigned int trap_present		: 1;
	unsigned int offchip_lds_pointer_en	: 1;
	unsigned int unknown_en		: 1;	
	unsigned int reserved 		: 23;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_LS */
struct opengl_si_bin_spi_shader_pgm_rsrc2_ls_t
{
	unsigned int scratch_en		: 1;
	unsigned int user_sgpr		: 5;
	unsigned int trap_present		: 1;
	unsigned int reserved 		: 25;
}__attribute__((packed));

/* PA_CL_VS_OUT_CNTL */
struct opengl_si_bin_pa_cl_vs_out_cntl_t
{
	unsigned int unknown1	: 8;
	unsigned int unknown2	: 8;
	unsigned int unknown3	: 8;
	unsigned int unknown4	: 8;	
}__attribute__((packed));

/* SPI_VS_OUT_CONFIG */
struct opengl_si_spi_vs_out_config_t
{
	unsigned int vs_export_count		: 8;
	unsigned int vs_half_pack		: 8;
	unsigned int vs_export_fog 		: 8;
	unsigned int vs_out_fog_vec_addr 	: 8;
}__attribute__((packed));

/* SPI_SHADER_POS_FORMAT */
struct opengl_si_spi_shader_pos_format
{
	unsigned int pos0_export_format 	: 8;
	unsigned int pos1_export_format 	: 8;
	unsigned int pos2_export_format 	: 8;
	unsigned int pos3_export_format 	: 8;	
}__attribute__((packed));

/*
 *  Vertex shader input declaration to be used for semantic mapping with FS.
 */
struct opengl_si_bin_vs_semantic_mapping_in_t
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
struct opengl_si_bin_vs_semantic_mapping_out_t
{
	uint32_t usage     : 8;      /* semantic usage. IL_IMPORTUSAGE. */
	uint32_t usageIdx  : 8;      /* semantic index. Opaque to SC. */
	uint32_t paramIdx  : 8;      /* attribute export parameter index (0-31) */
	uint32_t reserved  : 8;
}__attribute__((packed));

/*
 *  Pixel shader input declaration to be used for semantic mapping.
 */
struct opengl_si_bin_ps_semantic_mapping_in_t
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
struct opengl_si_bin_fs_export_patch_into_t
{
	// Patch offset (in DWORDs) to start of an export sequence
	uint32_t patchOffset;
	// Conversion code snippet for each of the export formats
	uint32_t patchCode[SC_SI_NUM_EXPORT_FMT][SC_SI_MAX_EXPORT_CODE_SIZE];
}__attribute__((packed));


/*
 *  Flags to guide shader compilation.
 */
struct opengl_si_bin_compile_guide_t
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


/* Vertex shader metadata stored in .text section */
struct opengl_si_bin_vertex_shader_metadata_t  
{
	/* SI Shader base structure, same for all shaders */
	OPENGL_SI_BIN_SHADER_SI_BASE

	/* Shader specific structures */
	/* Input semantics */
	uint32_t numVsInSemantics;
	struct opengl_si_bin_vs_semantic_mapping_in_t vsInSemantics[SC_SI_VS_MAX_INPUTS];

	/* Output semantics */
	uint32_t numVsOutSemantics;
	struct opengl_si_bin_vs_semantic_mapping_out_t vsOutSemantics[SC_SI_VS_MAX_OUTPUTS];

	/* LS/ES/VS specific shader resources */
	union
	{
		uint32_t spiShaderPgmRsrc2Ls;
		uint32_t spiShaderPgmRsrc2Es;
		struct opengl_si_bin_spi_shader_pgm_rsrc2_vs_t spiShaderPgmRsrc2Vs;
	};

	/* SC-provided values for certain VS-specific registers */
	uint32_t paClVsOutCntl;
	union
	{
		struct opengl_si_spi_vs_out_config_t spiVsOutConfig;
		uint32_t spiVsOutConfigAsUint;
	};
	union
	{
		struct opengl_si_spi_shader_pos_format spiShaderPosFormat;
		uint32_t spiShaderPosFormatAsUint;
	};

	/* FIXME: should be a struct */
	uint32_t vgtStrmoutConfig;

	/* Number of SPI-generated VGPRs referenced by the vertex shader */
	uint32_t vgprCompCnt;

	/* Exported vertex size in DWORDs, can be used to program ESGS ring size */
	uint32_t exportVertexSize;

	bool useEdgeFlags;  
	bool  remapClipDistance; /*true if clip distance[4-7] is remapped to [0-3]  */
	uint32_t hwShaderStage;    /* hardware stage which this shader actually in when execution */
	struct opengl_si_bin_compile_guide_t compileFlags;     /* compile flag */
	uint32_t gsMode;                            /* gs mode */

	/* FIXME: value doesn't make sense and the size doesn't match if not commentted */
	// bool isOnChipGs; 
	// uint32_t targetLdsSize; 
};

/* Fragment shader metadata stored in .text section */
struct opengl_si_bin_pixel_shader_metadata_t
{
	/* SI Shader base structure, same for all shaders */
	OPENGL_SI_BIN_SHADER_SI_BASE

	// Input semantics
	uint32_t numPsInSemantics;
	struct opengl_si_bin_ps_semantic_mapping_in_t psInSemantics[SC_SI_PS_MAX_INPUTS];

	// Mapping of a PS interpolant to a texture coordinate index (0xffffffff if that interpolant is not a texture coordinate)
	uint32_t psInTexCoordIndex[SC_SI_PS_MAX_INPUTS];

	// PS specific shader resources
	union
	{
		uint32_t spiShaderPgmRsrc2PsAsUint;
		struct opengl_si_bin_spi_shader_pgm_rsrc2_ps_t spiShaderPgmRsrc2Ps;		
	};

	uint32_t spiShaderZFormat;
	uint32_t spiPsInControl;
	uint32_t spiPsInputAddr;
	uint32_t spiPsInputEna;

	uint32_t spiBarycCntl;  // XXX mbalci: this would come from SCL(so SC) too; waiting for new interface change to be ported.

	uint32_t dbShaderControl;
	uint32_t cbShaderMask;

	// Size of each export patch pixel
	uint32_t exportPatchCodeSize;
	// Number of RT exports
	uint32_t numPsExports;
	// true if dual blending is enable
	bool dualBlending;
	// Export patch info per RT
	struct opengl_si_bin_fs_export_patch_into_t exportPatchInfo[SC_SI_PS_MAX_OUTPUTS];
	uint32_t defaultExportFmt;
};

/* texture resource and sampler binding */
struct opengl_si_texture_resource_bound_t
{
	uint32_t resourceId;         /* resource id */
	uint32_t samplerMask;        /* samplers bind to resource id */
};

/* Info descriptor for .info section */
enum opengl_si_bin_info_max_offset
{ 
	MAX_PROGRAMINFO_OFFSET = 0x0ffff 
};

typedef struct 
{
	// VS input mask
	uint32_t inputStreamMask;                /* input stream mask (phsyical id) */
	uint16_t   usesVertexID;                   /* tells whether this program uses VertexID */
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
	enum opengl_si_fetch_shader_type_t fsTypeForPassThruVS :8;   /* Fetch shader type (SVP PassThruVS) */
	uint8_t  fsReturnAddrRegForPassThruVS;            /* Fetch shader subroutine start SGPR (SVP PassThruVS) */
	uint8_t  fsInputStreamTableRegForPassThruVS;      /* Fetch shader input stream table start SGPR (SVP PassThruVS) */
	int32_t  fsAttribValidMaskReg;                    /* VPGR which Fetch shader should populate, if sparse buffers are used. */
} __attribute__((packed)) SIVSInfo;

typedef struct
{
	uint32_t texKillPresent;                         /* Program uses texkill */
	int32_t  pointCoordReg;                          /* register number of gl_PointCoord which is an input of FS */
	uint8_t  outputColorMap[PS_MAX_OUTPUTS];  /* pixel shader color output map (from virtual to physical) */
	uint16_t   useFlatInterpMode;                      /* if flat has been used on a varying */
	uint16_t   forcePerSampleShading;                  /* true if the FS is required to run in per sample frequency */
	uint16_t   uncached_writes;                        /* uncached writes */
	uint16_t   outputDepth;                            /* true if pixel shader output depth */
	uint32_t usingDualBlendMask;                     /* indicates using an index = 1 for dual blending, in glsl layout */
} __attribute__((packed)) SIPSInfo;

typedef struct
{
	uint32_t numSharedGPRUser;    /* shared GPR */
	uint32_t numSharedGPRTotal;   /* shared GPR total including ones used by SC. */

	uint32_t numThreadPerGroup;   /* threads per group */
	uint32_t numThreadPerGroupX;  /* dimension x of NumThreadPerGroup */
	uint32_t numThreadPerGroupY;  /* dimension y of NumThreadPerGroup */
	uint32_t numThreadPerGroupZ;  /* dimension z of NumThreadPerGroup */
	uint32_t totalNumThreadGroup; /* total number of thread groups */
	uint32_t NumWavefrontPerSIMD; /* wavefronts per simd */
	uint16_t   eCsSetupMode;        /* compute slow/fast mode */
	uint16_t   IsMaxNumWavePerSIMD; /* Is this the max active num wavefronts per simd */
	uint16_t   SetBufferForNumGroup;/* Need to set up buffer for info on number of thread groups? */
} __attribute__((packed)) SICSInfo;

typedef struct
{
	uint16_t   usesVertexCache;      /* vertex cache used? (fetch shader only) */
} __attribute__((packed)) SIFSInfo;

struct opengl_si_bin_info_t
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
	uint16_t   uavInCB;                       /* UAV is in CB */
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
		SIVSInfo vs;
		/// Fragment Shader Parameters
		SIPSInfo ps;
		/// Compute Shader Parameters
		SICSInfo cs;
		/// Fetch Shader Parameters
		SIFSInfo fs;
	};
	/* dynamic array, offset fields is valid in ELF package, int64_t is to keep the struct size fixed in all operation system. */
	/* texture resource bound array */
	union
	{
		struct opengl_si_texture_resource_bound_t *_textureResourceBound; /* texture resoruce and sampler bounding */
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
	
	enum opengl_si_bin_info_max_offset max_valid_offset;
}__attribute__((packed));

/* ARB program parameter */
struct opengl_si_bin_arb_program_parameter_t
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
}__attribute__((packed));

/* Structure in .usageinfo section */
typedef struct
{
	/* fs input usage */
	uint32_t usesTexCoordMask;                   /* texcoord unit usage mask */
	uint16_t   usesFogCoord;                       /* fogcoord usage */
	uint16_t   usePrimaryColor;                    /* true if primary color is used */
	uint16_t   useSecondaryColor;                  /* true if secondary color is used */
	/* aa stipple */
	int8_t   aaStippleTexCoord;                  /* the texture coord used by SCL to simulate aa/stipple */
	int8_t   aaStippleTexUnit;                   /* the texture unit used by aa/stipple texture image */
	int8_t   aaStippleTexVirtUnit;               /* default logic unit for aa stipple texture unit */
	/* bitmap */
	int8_t   bitmapTexCoord;                     /* the texture coord used for bitmap drawing */
	int8_t   bitmapTexUnit;                      /* the texture unit used for bitmap drawing */
	int8_t   bitmapTexVirtUnit;                  /* default logic unit for bitmap drawing */
	/* misc fields */
	uint16_t   needSampleInfoCBs;                  /* whether the FP needs the 2 constant buffers for the SAMPLEINFO, SAMPLEPOS and EVAL_SAMPLE_INDEX. */
	uint16_t   earlyFragTest;                      /* true if early frag test is enabled */
	uint8_t  conservativeZ;                      /* 0:defult, 1:less, 2:greater */
} __attribute__((packed)) SIPSUsageinfo;

typedef struct
{
	uint16_t maxGSVertices[4];                   /* max gs output vertices */
	uint16_t gsInvocations;                      /* gs invocation number */
	uint8_t  inputTopology;                      /* gs input topology */
	uint8_t  outputTopology[4];                  /* gs output topology */
} __attribute__((packed)) SIGSUsageinfo;

typedef struct
{
	uint32_t tessOutputVertices;                 /* output control point number */
} __attribute__((packed)) SIHSUsageinfo;

typedef struct
{
	uint32_t tessGenMode;                        /* domain mode */
	uint32_t tessGenSpacing;                     /* partition mode */
	uint32_t tessGenVertexOrder;                 /* output primitive mode */
	uint16_t   tessGenPointMode;                   /* If point mode when tessellated */
} __attribute__((packed)) SIDSUsageinfo;

typedef struct
{
	uint16_t   positionInvariant;                  /* Has the ARB_position_invariant option been requested */
	uint16_t   enableAnotherSetAttribAlias;        /* if it is cg generated program or nv_vertex_program */
	uint32_t lsStride;                           /* The number of input-control-points per patch declared in HS */
	/* SI+ fetch shader parameters */
	int8_t   fsType;                             /* Fetch shader type (immediate vs flat), SI only */
	int8_t   fsReturnAddrReg;                    /* Fetch shader subroutine return address SGPR, SI only */
	int8_t   fsInputStreamTableReg;              /* Fetch shader input stream table start SGPR (either first data element or pointer depending on FS type), SI only */
	int8_t   fsVertexIdChannel;                  /* Fetch shader channel(R,G,B,A) to compute the vertexID with */
} __attribute__((packed)) SIVSUsageinfo;

typedef struct
{
	uint32_t  workDimension;                     /* Work dimension (1, 2 or 3 right now) */
	uint32_t  workSizeX;                         /* Work size in the X dimension */
	uint32_t  workSizeY;                         /* Work size in the Y dimension */
	uint32_t  workSizeZ;                         /* Work size in the Z dimension */
} __attribute__((packed)) SICSUsageinfo;

/* FIXME: usageinfo section is 156 bytes for PS but 152 for VS */
struct opengl_si_bin_usageinfo_t
{
	struct opengl_si_bin_arb_program_parameter_t   arbProgramParameter;   /* ARB program parameters */
	uint16_t layoutParamMask;                            /* layout parameters mask, see type gllShaderLayoutType for detail info */
	uint16_t usesPrimitiveID;                              /* Does this program use PrimitiveID */
	uint16_t usesClipDistance;                             /* vp outputs clip distance(s) */

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
		SIPSUsageinfo ps;
		//
		/// Geometry Shader Parametes
		//
		SIGSUsageinfo gs;
		//
		/// Tessellation Evaluation Shader Parameters
		//
		//
		SIHSUsageinfo hs;
		//
		/// Tessellation Control Shader Parameters
		//
		SIDSUsageinfo ds;
		//
		/// Vertex Shader Parametes
		//
		SIVSUsageinfo vs;
		//
		/// Compute Shader Parameters 
		//
		SICSUsageinfo cs;
	};

	enum opengl_si_bin_info_max_offset maxOffset;         /* max valid value for dynamic array offset */
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

}__attribute__((packed));


/* structure for .symbol section */
struct opengl_si_bin_symbol_t
{
	enum opengl_si_bin_symbol_type_t     type;     ///< Uniform, normal, texcoord, generic attrib, etc.
	enum opengl_si_bin_symbol_datatype_t dataType; ///< float, vec2, mat4, etc.
	//
	/// union of qualifier struct
	//
	union
	{
		//
		/// qualifier struct
		//
		struct
		{
		SIElfSymbolDataPrecisionQualifier      dataPrecQual            : 4;    ///< low, medium, high, double
		SIElfLayoutStyleQualifier              layoutStyleQualifier    : 4;    ///< Style of layout qualifier
		SIElfLayoutMajorQualifier              layoutMajorQualifier    : 4;    ///< Major of layout qualifier
		SIElfInvariantQualifier                invariantQualifier      : 2;    ///< invariant qualifier
		SIElfPreciseQualifier                  preciseQualifier        : 2;    ///< precise qualifier
		SIElfStorageQualifier                  storageQualifier        : 3;    ///< storage qualifier
		SIElfInterpolationQualifier            interpQualifier         : 3;    ///< interpolation qualifier
		SIElfPatchQualifier                    patchQualifier          : 2;    ///< patch qualifier
		};
		uint32_t  qualifier;	///< qualifier
	};

	int                  vvalue;   ///< The value of this symbol for the vertex shader, intended to be virtual constant offset
	int                  vcbindex; ///< The index of the bindable uniform of this symbol for vertex shader
	SIElfSwizzle        vswizzle; ///< Destination swizzle for vertex constants
	int                  gvalue;   ///< The value of this symbol for the geometry shader, intended to be virtual constant offset
	int                  gcbindex; ///< The index of the bindable uniform of this symbol for geometry shader
	SIElfSwizzle        gswizzle; ///< Destination swizzle for geometry constants
	int                  fvalue;   ///< The value of this symbol for the fragment shader
	int                  fcbindex; ///< The index of the bindable uniform of this symbol for fragment shader
	SIElfSwizzle        fswizzle; ///< Destination swizzle for fragment constants
	int                  hvalue;   ///< The value of this symbol for the hull shader
	int                  hcbindex; ///< The index of the bindable uniform of this symbol for hull shader
	SIElfSwizzle        hswizzle; ///< Destination swizzle for hull constants
	int                  dvalue;   ///< The value of this symbol for the domain shader
	int                  dcbindex; ///< The index of the bindable uniform of this symbol for domain shader
	SIElfSwizzle        dswizzle; ///< Destination swizzle for domain constants
	int                  cvalue;   ///< The value of this symbol for the compute shader
	int                  ccbindex; ///< The index of the bindable uniform of this symbol for compute shader
	SIElfSwizzle        cswizzle; ///< Destination swizzle for compute constants
	int                  size;     ///< register_count * 4 * sizeof(float), no packing, with padding. This is for bindable uniform (GetUniformBufferSizeEXT)
	int                  count;    ///< if this symbol is part of an array, the number of remaining elements (including this one), 1 otherwise
						   ///< If anything is put between count and name, code in Uniform/VertexInterface ScanSymbols needs to be updated
	bool                 isArray;         ///< TRUE if the symbol is arrayed, FALSE otherwise
	uint32_t               matrixStride; ///< stride of columns of column major matrix or rows of row major matrix
	uint32_t               subSize;      ///< the number of subroutine functions, all dynamic array and string must be added at the end of the sturct.
	uint32_t               uniformBinding;  ///< Binding (versatile usages for uniform block, sampler and image, atomic counter, array buffer)
	int16_t                layoutLocation;  ///< layout location (versatile usages for uniform variables, including opaque types, and subroutine uniforms)
	int16_t                layoutIndex;     ///< layout Index for subroutine function
	uint32_t               uniformOffset;   ///< Offset (currently used for atomic counter)
	uint32_t               resourceIndex;   ///< Resource index (currently for storing GDS offset for atomic counter in DWs)

	uint32_t               subroutineTypeID; ///< subroutine type id
	uint32_t               topLevelArraySize; ///< TOP_LEVEL_ARRAY_SIZE
	uint32_t               topLevelArrayStride; ///< TOP_LEVEL_ARRAY_STRIDE
	char*          name;     ///< NULL-terminated string which is the name of this symbol
	char*          baseName; ///< if this symbol is part of an array, NULL-terminated string which is the unsubscripted name of the array
	char*          uniformBlockName;         ///< Name of uniform block
	char*          mangledName;///< available if this sysmbol is struct or interface block, it is used to check type.
};


struct opengl_si_enc_dict_vertex_shader_t
{
	/* Parent shader binary it belongs to */
	struct opengl_si_shader_binary_t *parent;

	/* Shader info extracted from sections in parent shader binary */
	struct opengl_si_bin_vertex_shader_metadata_t *meta;
	struct list_t *inputs; /* Elements with type struct opengl_si_bin_input_t */
	struct list_t *outputs; /* Elements with type struct opengl_si_bin_output_t */
	struct opengl_si_bin_info_t *info;
	struct opengl_si_bin_usageinfo_t *usageinfo;
	struct list_t *symbols; /* Elements with type struct opengl_si_bin_symbols_t */
};

struct opengl_si_enc_dict_pixel_shader_t
{
	/* Parent binary it belongs to */
	struct opengl_si_shader_binary_t *parent;

	/* Shader info extracted from sections in parent shader binary */
	struct opengl_si_bin_pixel_shader_metadata_t *meta;
	struct list_t *inputs; /* Elements with type struct opengl_si_bin_input_t */
	struct list_t *outputs; /* Elements with type struct opengl_si_bin_output_t*/
	struct opengl_si_bin_info_t *info;
	struct opengl_si_bin_usageinfo_t *usageinfo;
	struct list_t *symbols; /* Elements with type struct opengl_si_bin_symbols_t */	
};

/* Input descriptor for .inputs section */
struct opengl_si_bin_input_t
{
	enum opengl_si_bin_input_type_t type;
	uint32_t voffset;
	uint32_t poffset;
	// bool isFloat16;  FIXME: has to comment this as the size will be 29 instead of 28 bytes  
	enum opengl_si_bin_input_swizzle_type_t swizzles[4];
}__attribute__((packed));

/* FIXME: size of this structure doesn't match the binary */
/* Output descriptor for .outputs section */
struct opengl_si_bin_output_t 
{
	enum opengl_si_bin_output_type_t type;          /* Semantic type */
	unsigned int voffset;           /* Virtual offset */
	unsigned int poffset;           /* Physical offset */
	enum opengl_si_bin_symbol_datatype_t data_type;      /* Data type */	
	// uint32_t array_size;     /* Array size */ FIXME: otherwise size doesn't match 
	char* name;           /* Name of the output */
}__attribute__((packed));;

typedef void (*opengl_si_shader_bin_free_func_t)(void *);
struct opengl_si_shader_binary_t
{
	/* Parent shader, defined in driver */
	struct opengl_si_shader_t *parent;

	/* Type of shader */
	enum opengl_si_shader_binary_kind_t shader_kind;

	/* ELF-formatted shader, it's embedded in the .internal 
	 * section of a shader binary */
	struct elf_file_t *elf;

	/* Pointer to ISA */
	struct elf_buffer_t *isa;

	/* 
	 * Element of type opengl_si_enc_dict_xxxxx_shader_t 
	 * it is decided by a flag in ELF header when creating
	 * on the fly
	 */
	void *enc_dict; 

	/* 
	 * Callback function to free shader binary,  as
	 * encoding dictionary has different structure 
	 * for different types of shader binary
	 */
	opengl_si_shader_bin_free_func_t free_func;  
};

struct opengl_si_program_binary_t
{
	/* Name of the associated shader binary file */
	char *name;

	/* Associated ELF-format shader binary */
	struct elf_file_t *binary;

	/* List of shaders in shader binary, elements with type opengl_si_shader_binary_t */
	struct list_t *shader_bins;
};

struct opengl_si_program_binary_t *opengl_si_program_binary_create(
		const char *buffer_ptr, int size, const char *name);
void opengl_si_program_binary_free(struct opengl_si_program_binary_t *program_bin);

struct opengl_si_shader_binary_t *opengl_si_shader_binary_create(void *buffer, int size, char* name);
void opengl_si_shader_binary_free(struct opengl_si_shader_binary_t *shdr);

struct SIBinaryUserElement *opengl_si_bin_enc_user_element_create();
void opengl_si_bin_enc_user_element_free(struct SIBinaryUserElement *user_elem);

int opengl_si_bin_symbol_get_location(struct list_t *symbol_list, char *name);

void opengl_si_shader_binary_debug_meta(struct opengl_si_shader_binary_t *shdr_bin);


#endif

