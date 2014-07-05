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

#include <cstring>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Asm.h"
#include "ShaderBinary.h"


namespace SI
{

misc::StringMap EncDictInputTypeMap =
{
	{ "generic attribute",                          OpenGLSiInputAttrib },
	{ "primary color",                              OpenGLSiInputColor },
	{ "secondary color",                            OpenGLSiInputSecondarycolor },
	{ "texture coordinate",                         OpenGLSiInputTexcoord },
	{ "texture unit id",                            OpenGLSiInputTexid },
	{ "buffer unit id",                             OpenGLSiInputBufferid },
	{ "constant buffer unit id",                    OpenGLSiInputConstantbufferid },
	{ "texture resource id",                        OpenGLSiInputTextureresourceid }
};

misc::StringMap EncDictInputSwizzleTypeMap =
{
	{"x",                                           OpenGLSiSwizzleX },
	{"y",                                           OpenGLSiSwizzleY },
	{"z",                                           OpenGLSiSwizzleZ },
	{"w",                                           OpenGLSiSwizzleW },
	{"0",                                           OpenGLSiSwizzle0 },
	{"1",                                           OpenGLSiSwizzle1 }
};

misc::StringMap EncDictOutputTypeMap =
{
	{ "pos",                                        OpenGLSiOutputPos },
	{ "point size",                                 OpenGLSiOutputPointsize },
	{ "color",                                      OpenGLSiOutputColor },
	{ "secondary color",                            OpenGLSiOutputSecondarycolor },
	{ "generic",                                    OpenGLSiOutputGeneric },
	{ "depth",                                      OpenGLSiOutputDepth },
	{ "clip distance",                              OpenGLSiOutputClipdistance },
	{ "primitive id",                               OpenGLSiOutputPrimitiveid },
	{ "layer",                                      OpenGLSiOutputLayer },
	{ "viewport index",                             OpenGLSiOutputViewportindex },
	{ "stencil ref",                                OpenGLSiOutputStencil },
	{ "stencil value",                              OpenGLSiOutputStencilValue },
	{ "sample mask",                                OpenGLSiOutputSamplemask },
	{ "stream id",                                  OpenGLSiOutputStreamid}
};

misc::StringMap EncDictSymbolDataTypeMap =
{
	{ "VOID",                                       OpenGLSiSymbolDatatypeVoid },
	{ "BOOL",                                       OpenGLSiSymbolDatatypeBool },
	{ "INT",                                        OpenGLSiSymbolDatatypeInt },
	{ "UINT",                                       OpenGLSiSymbolDatatypeUint },
	{ "FLOAT",                                      OpenGLSiSymbolDatatypeFloat },
	{ "DOUBLE",                                     OpenGLSiSymbolDatatypeDouble },
	{ "FLOAT16",                                    OpenGLSiSymbolDatatypeFloat16 },
	{ "DVEC2",                                      OpenGLSiSymbolDatatypeDvec2 },
	{ "DVEC3",                                      OpenGLSiSymbolDatatypeDvec3 },
	{ "DVEC4",                                      OpenGLSiSymbolDatatypeDvec4 },
	{ "VEC2",                                       OpenGLSiSymbolDatatypeVec2 },
	{ "VEC3",                                       OpenGLSiSymbolDatatypeVec3 },
	{ "VEC4",                                       OpenGLSiSymbolDatatypeVec4 },
	{ "BVEC2",                                      OpenGLSiSymbolDatatypeBvec2 },
	{ "BVEC3",                                      OpenGLSiSymbolDatatypeBvec3 },
	{ "BVEC4",                                      OpenGLSiSymbolDatatypeBvec4 },
	{ "IVEC2",                                      OpenGLSiSymbolDatatypeIvec2 },
	{ "IVEC3",                                      OpenGLSiSymbolDatatypeIvec3 },
	{ "IVEC4",                                      OpenGLSiSymbolDatatypeIvec4 },
	{ "UVEC2",                                      OpenGLSiSymbolDatatypeUvec2 },
	{ "UVEC3",                                      OpenGLSiSymbolDatatypeUvec3 },
	{ "UVEC4",                                      OpenGLSiSymbolDatatypeUvec4 },
	{ "F16VEC2",                                    OpenGLSiSymbolDatatypeF16vec2 },
	{ "F16VEC3",                                    OpenGLSiSymbolDatatypeF16vec3 },
	{ "F16VEC4",                                    OpenGLSiSymbolDatatypeF16vec4 },
	{ "MAT2",                                       OpenGLSiSymbolDatatypeMat2 },
	{ "MAT2X3",                                     OpenGLSiSymbolDatatypeMat2x3 },
	{ "MAT2X4",                                     OpenGLSiSymbolDatatypeMat2x4 },
	{ "MAT3X2",                                     OpenGLSiSymbolDatatypeMat3x2 },
	{ "MAT3",                                       OpenGLSiSymbolDatatypeMat3 },
	{ "MAT3X4",                                     OpenGLSiSymbolDatatypeMat3x4 },
	{ "MAT4X2",                                     OpenGLSiSymbolDatatypeMat4x2 },
	{ "MAT4X3",                                     OpenGLSiSymbolDatatypeMat4x3 },
	{ "MAT4",                                       OpenGLSiSymbolDatatypeMat4 },
	{ "DMAT2",                                      OpenGLSiSymbolDatatypeDmat2 },
	{ "DMAT2X3",                                    OpenGLSiSymbolDatatypeDmat2x3 },
	{ "DMAT2X4",                                    OpenGLSiSymbolDatatypeDmat2x4 },
	{ "DMAT3X2",                                    OpenGLSiSymbolDatatypeDmat3x2 },
	{ "DMAT3",                                      OpenGLSiSymbolDatatypeDmat3 },
	{ "DMAT3X4",                                    OpenGLSiSymbolDatatypeDmat3x4 },
	{ "DMAT4X2",                                    OpenGLSiSymbolDatatypeDmat4x2 },
	{ "DMAT4X3",                                    OpenGLSiSymbolDatatypeDmat4x3 },
	{ "DMAT4",                                      OpenGLSiSymbolDatatypeDmat4 },
	{ "F16MAT2",                                    OpenGLSiSymbolDatatypeF16mat2 },
	{ "F16MAT2X3",                                  OpenGLSiSymbolDatatypeF16mat2x3 },
	{ "F16MAT2X4",                                  OpenGLSiSymbolDatatypeF16mat2x4 },
	{ "F16MAT3X2",                                  OpenGLSiSymbolDatatypeF16mat3x2 },
	{ "F16MAT3",                                    OpenGLSiSymbolDatatypeF16mat3 },
	{ "F16MAT3X4",                                  OpenGLSiSymbolDatatypeF16mat3x4 },
	{ "F16MAT4X2",                                  OpenGLSiSymbolDatatypeF16mat4x2 },
	{ "F16MAT4X3",                                  OpenGLSiSymbolDatatypeF16mat4x3 },
	{ "F16MAT4",                                    OpenGLSiSymbolDatatypeF16mat4 },
	{ "SAMPLER_1D",                                 OpenGLSiSymbolDatatypeSampler1d },
	{ "SAMPLER_2D",                                 OpenGLSiSymbolDatatypeSampler2d },
	{ "SAMPLER_2D_RECT",                            OpenGLSiSymbolDatatypeSampler2dRect },
	{ "SAMPLER_EXTERNAL",                           OpenGLSiSymbolDatatypeSamplerExternal },
	{ "SAMPLER_3D",                                 OpenGLSiSymbolDatatypeSampler3d },
	{ "SAMPLER_CUBE",                               OpenGLSiSymbolDatatypeSamplerCube },
	{ "SAMPLER_1D_ARRAY",                           OpenGLSiSymbolDatatypeSampler1dArray },
	{ "SAMPLER_2D_ARRAY",                           OpenGLSiSymbolDatatypeSampler2dArray },
	{ "SAMPLER_CUBE_ARRAY",                         OpenGLSiSymbolDatatypeSamplerCubeArray },
	{ "SAMPLER_BUFFER",                             OpenGLSiSymbolDatatypeSamplerBuffer },
	{ "SAMPLER_RENDERBUFFER",                       OpenGLSiSymbolDatatypeSamplerRenderbuffer },
	{ "SAMPLER_2D_MS",                              OpenGLSiSymbolDatatypeSampler2dMs },
	{ "SAMPLER_2D_MS_ARRAY",                        OpenGLSiSymbolDatatypeSampler2dMsArray },
	{ "SAMPLER_1D_SHADOW",                          OpenGLSiSymbolDatatypeSampler1dShadow },
	{ "SAMPLER_2D_SHADOW",                          OpenGLSiSymbolDatatypeSampler2dShadow },
	{ "SAMPLER_2D_RECT_SHADOW",                     OpenGLSiSymbolDatatypeSampler2dRectShadow },
	{ "SAMPLER_CUBE_SHADOW",                        OpenGLSiSymbolDatatypeSamplerCubeShadow },
	{ "SAMPLER_1D_ARRAY_SHADOW",                    OpenGLSiSymbolDatatypeSampler1dArrayShadow },
	{ "SAMPLER_2D_ARRAY_SHADOW",                    OpenGLSiSymbolDatatypeSampler2dArrayShadow },
	{ "SAMPLER_CUBE_ARRAY_SHADOW",                  OpenGLSiSymbolDatatypeSamplerCubeArrayShadow },
	{ "INT_SAMPLER_1D",                             OpenGLSiSymbolDatatypeIntSampler1d },
	{ "INT_SAMPLER_2D",                             OpenGLSiSymbolDatatypeIntSampler2d },
	{ "INT_SAMPLER_2D_RECT",                        OpenGLSiSymbolDatatypeIntSampler2dRect },
	{ "INT_SAMPLER_3D",                             OpenGLSiSymbolDatatypeIntSampler3d },
	{ "INT_SAMPLER_CUBE",                           OpenGLSiSymbolDatatypeIntSamplerCube },
	{ "INT_SAMPLER_1D_ARRAY",                       OpenGLSiSymbolDatatypeIntSampler1dArray },
	{ "INT_SAMPLER_2D_ARRAY",                       OpenGLSiSymbolDatatypeIntSampler2dArray },
	{ "INT_SAMPLER_CUBE_ARRAY",                     OpenGLSiSymbolDatatypeIntSamplerCubeArray },
	{ "INT_SAMPLER_BUFFER",                         OpenGLSiSymbolDatatypeIntSamplerBuffer },
	{ "INT_SAMPLER_RENDERBUFFER",                   OpenGLSiSymbolDatatypeIntSamplerRenderbuffer },
	{ "INT_SAMPLER_2D_MS",                          OpenGLSiSymbolDatatypeIntSampler2dMs },
	{ "INT_SAMPLER_2D_MS_ARRAY",                    OpenGLSiSymbolDatatypeIntSampler2dMsArray },
	{ "UNSIGNED_INT_SAMPLER_1D",                    OpenGLSiSymbolDatatypeUnsignedIntSampler1d },
	{ "UNSIGNED_INT_SAMPLER_2D",                    OpenGLSiSymbolDatatypeUnsignedIntSampler2d },
	{ "UNSIGNED_INT_SAMPLER_2D_RECT",               OpenGLSiSymbolDatatypeUnsignedIntSampler2dRect },
	{ "UNSIGNED_INT_SAMPLER_3D",                    OpenGLSiSymbolDatatypeUnsignedIntSampler3d },
	{ "UNSIGNED_INT_SAMPLER_CUBE",                  OpenGLSiSymbolDatatypeUnsignedIntSamplerCube },
	{ "UNSIGNED_INT_SAMPLER_1D_ARRAY",              OpenGLSiSymbolDatatypeUnsignedIntSampler1dArray },
	{ "UNSIGNED_INT_SAMPLER_2D_ARRAY",              OpenGLSiSymbolDatatypeUnsignedIntSampler2dArray },
	{ "UNSIGNED_INT_SAMPLER_CUBE_ARRAY",            OpenGLSiSymbolDatatypeUnsignedIntSamplerCubeArray },
	{ "UNSIGNED_INT_SAMPLER_BUFFER",                OpenGLSiSymbolDatatypeUnsignedIntSamplerBuffer },
	{ "UNSIGNED_INT_SAMPLER_RENDERBUFFER",          OpenGLSiSymbolDatatypeUnsignedIntSamplerRenderbuffer },
	{ "UNSIGNED_INT_SAMPLER_2D_MS",                 OpenGLSiSymbolDatatypeUnsignedIntSampler2dMs },
	{ "UNSIGNED_INT_SAMPLER_2D_MS_ARRAY",           OpenGLSiSymbolDatatypeUnsignedIntSampler2dMsArray },
	{ "FLOAT16_SAMPLER_1D",                         OpenGLSiSymbolDatatypeFloat16Sampler1d },
	{ "FLOAT16_SAMPLER_2D",                         OpenGLSiSymbolDatatypeFloat16Sampler2d },
	{ "FLOAT16_SAMPLER_2D_RECT",                    OpenGLSiSymbolDatatypeFloat16Sampler2dRect },
	{ "FLOAT16_SAMPLER_3D",                         OpenGLSiSymbolDatatypeFloat16Sampler3d },
	{ "FLOAT16_SAMPLER_CUBE",                       OpenGLSiSymbolDatatypeFloat16SamplerCube },
	{ "FLOAT16_SAMPLER_1D_ARRAY",                   OpenGLSiSymbolDatatypeFloat16Sampler1dArray },
	{ "FLOAT16_SAMPLER_2D_ARRAY",                   OpenGLSiSymbolDatatypeFloat16Sampler2dArray },
	{ "FLOAT16_SAMPLER_CUBE_ARRAY",                 OpenGLSiSymbolDatatypeFloat16SamplerCubeArray },
	{ "FLOAT16_SAMPLER_BUFFER",                     OpenGLSiSymbolDatatypeFloat16SamplerBuffer },
	{ "FLOAT16_SAMPLER_2D_MS",                      OpenGLSiSymbolDatatypeFloat16Sampler2dMs },
	{ "FLOAT16_SAMPLER_2D_MS_ARRAY",                OpenGLSiSymbolDatatypeFloat16Sampler2dMsArray },
	{ "FLOAT16_SAMPLER_1D_SHADOW",                  OpenGLSiSymbolDatatypeFloat16Sampler1dShadow },
	{ "FLOAT16_SAMPLER_2D_SHADOW",                  OpenGLSiSymbolDatatypeFloat16Sampler2dShadow },
	{ "FLOAT16_SAMPLER_2D_RECT_SHADOW",             OpenGLSiSymbolDatatypeFloat16Sampler2dRectShadow },
	{ "FLOAT16_SAMPLER_CUBE_SHADOW",                OpenGLSiSymbolDatatypeFloat16SamplerCubeShadow },
	{ "FLOAT16_SAMPLER_1D_ARRAY_SHADOW",            OpenGLSiSymbolDatatypeFloat16Sampler1dArrayShadow },
	{ "FLOAT16_SAMPLER_2D_ARRAY_SHADOW",            OpenGLSiSymbolDatatypeFloat16Sampler2dArrayShadow },
	{ "FLOAT16_SAMPLER_CUBE_ARRAY_SHADOW",          OpenGLSiSymbolDatatypeFloat16SamplerCubeArrayShadow },
	{ "IMAGE_1D",                                   OpenGLSiSymbolDatatypeImage1d },
	{ "IMAGE_2D",                                   OpenGLSiSymbolDatatypeImage2d },
	{ "IMAGE_3D",                                   OpenGLSiSymbolDatatypeImage3d },
	{ "IMAGE_2D_RECT",                              OpenGLSiSymbolDatatypeImage2dRect },
	{ "IMAGE_CUBEMAP",                              OpenGLSiSymbolDatatypeImageCubemap },
	{ "IMAGE_BUFFER",                               OpenGLSiSymbolDatatypeImageBuffer },
	{ "IMAGE_1D_ARRAY",                             OpenGLSiSymbolDatatypeImage1dArray },
	{ "IMAGE_2D_ARRAY",                             OpenGLSiSymbolDatatypeImage2dArray },
	{ "IMAGE_CUBEMAP_ARRAY",                        OpenGLSiSymbolDatatypeImageCubemapArray },
	{ "IMAGE_2D_MS",                                OpenGLSiSymbolDatatypeImage2dMs },
	{ "IMAGE_2D_MS_ARRAY",                          OpenGLSiSymbolDatatypeImage2dMsArray },
	{ "INT_IMAGE_1D",                               OpenGLSiSymbolDatatypeIntImage1d },
	{ "INT_IMAGE_2D",                               OpenGLSiSymbolDatatypeIntImage2d },
	{ "INT_IMAGE_3D",                               OpenGLSiSymbolDatatypeIntImage3d },
	{ "INT_IMAGE_2D_RECT",                          OpenGLSiSymbolDatatypeIntImage2dRect },
	{ "INT_IMAGE_CUBEMAP",                          OpenGLSiSymbolDatatypeIntImageCubemap },
	{ "INT_IMAGE_BUFFER",                           OpenGLSiSymbolDatatypeIntImageBuffer },
	{ "INT_IMAGE_1D_ARRAY",                         OpenGLSiSymbolDatatypeIntImage1dArray },
	{ "INT_IMAGE_2D_ARRAY",                         OpenGLSiSymbolDatatypeIntImage2dArray },
	{ "INT_IMAGE_CUBEMAP_ARRAY",                    OpenGLSiSymbolDatatypeIntImageCubemapArray },
	{ "INT_IMAGE_2D_MS",                            OpenGLSiSymbolDatatypeIntImage2dMs },
	{ "INT_IMAGE_2D_MS_ARRAY",                      OpenGLSiSymbolDatatypeIntImage2dMsArray },
	{ "UNSIGNED_INT_IMAGE_1D",                      OpenGLSiSymbolDatatypeUnsignedIntImage1d },
	{ "UNSIGNED_INT_IMAGE_2D",                      OpenGLSiSymbolDatatypeUnsignedIntImage2d },
	{ "UNSIGNED_INT_IMAGE_3D",                      OpenGLSiSymbolDatatypeUnsignedIntImage3d },
	{ "UNSIGNED_INT_IMAGE_2D_RECT",                 OpenGLSiSymbolDatatypeUnsignedIntImage2dRect },
	{ "UNSIGNED_INT_IMAGE_CUBEMAP",                 OpenGLSiSymbolDatatypeUnsignedIntImageCubemap },
	{ "UNSIGNED_INT_IMAGE_BUFFER",                  OpenGLSiSymbolDatatypeUnsignedIntImageBuffer },
	{ "UNSIGNED_INT_IMAGE_1D_ARRAY",                OpenGLSiSymbolDatatypeUnsignedIntImage1dArray },
	{ "UNSIGNED_INT_IMAGE_2D_ARRAY",                OpenGLSiSymbolDatatypeUnsignedIntImage2dArray },
	{ "UNSIGNED_INT_IMAGE_CUBEMAP_ARRAY",           OpenGLSiSymbolDatatypeUnsignedIntImageCubemapArray },
	{ "UNSIGNED_INT_IMAGE_2D_MS",                   OpenGLSiSymbolDatatypeUnsignedIntImage2dMs },
	{ "UNSIGNED_INT_IMAGE_2D_MS_ARRAY",             OpenGLSiSymbolDatatypeUnsignedIntImage2dMsArray },
	{ "FLOAT16_IMAGE_1D",                           OpenGLSiSymbolDatatypeFloat16Image1d },
	{ "FLOAT16_IMAGE_2D",                           OpenGLSiSymbolDatatypeFloat16Image2d },
	{ "FLOAT16_IMAGE_3D",                           OpenGLSiSymbolDatatypeFloat16Image3d },
	{ "FLOAT16_IMAGE_2D_RECT",                      OpenGLSiSymbolDatatypeFloat16Image2dRect },
	{ "FLOAT16_IMAGE_CUBEMAP",                      OpenGLSiSymbolDatatypeFloat16ImageCubemap },
	{ "FLOAT16_IMAGE_BUFFER",                       OpenGLSiSymbolDatatypeFloat16ImageBuffer },
	{ "FLOAT16_IMAGE_1D_ARRAY",                     OpenGLSiSymbolDatatypeFloat16Image1dArray },
	{ "FLOAT16_IMAGE_2D_ARRAY",                     OpenGLSiSymbolDatatypeFloat16Image2dArray },
	{ "FLOAT16_IMAGE_CUBEMAP_ARRAY",                OpenGLSiSymbolDatatypeFloat16ImageCubemapArray },
	{ "FLOAT16_IMAGE_2D_MS",                        OpenGLSiSymbolDatatypeFloat16Image2dMs },
	{ "FLOAT16_IMAGE_2D_MS_ARRAY",                  OpenGLSiSymbolDatatypeFloat16Image2dMsArray },
	{ "SAMPLER_BUFFER_AMD",                         OpenGLSiSymbolDatatypeSamplerBufferAmd },
	{ "INT_SAMPLER_BUFFER_AMD",                     OpenGLSiSymbolDatatypeIntSamplerBufferAmd },
	{ "UNSIGNED_INT_SAMPLER_BUFFER_AMD",            OpenGLSiSymbolDatatypeUnsignedIntSamplerBufferAmd },
	{ "ATOMIC_COUNTER",                             OpenGLSiSymbolDatatypeAtomicCounter },
	{ "STRUCT",                                     OpenGLSiSymbolDatatypeStruct },
	{ "INTERFACE",                                  OpenGLSiSymbolDatatypeInterface }
};

misc::StringMap EncDictSemanticInputTypeMap =
{
	{ "generic",0 }
};

misc::StringMap EncDictSemanticOutputTypeMap =
{
	{ "generic",0 },
	{ "unknown 1",1 },
	{ "unknown 2",2 },
	{ "unknown 3",3 },
	{ "unknown 4",4 },
	{ "unknown 5",5 },
	{ "unknown 6",6 }
};

misc::StringMap EncDictUserElementsTypeMap = 
{
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
	{"unknown 0", 29}
};

misc::StringMap EncDictSymbolTypeMap =
{
	{ "GLSL uniform",                               OpenGLSiSymbolUniform },
	{ "GLSL bindable uniform",                      OpenGLSiSymbolBindableUniform },
	{ "GLSL uniform block",                         OpenGLSiSymbolUniformBlock },
	{ "Vertex position",                            OpenGLSiSymbolAttribPosition },
	{ "Vertex normal",                              OpenGLSiSymbolAttribNormal },
	{ "Vertex primary color",                       OpenGLSiSymbolAttribPrimaryColor },
	{ "Vertex secondary color",                     OpenGLSiSymbolAttribSecondaryColor },
	{ "Vertex fog coord",                           OpenGLSiSymbolAttribFogcoord },
	{ "Vertex edge flag",                           OpenGLSiSymbolAttribEdgeflag },
	{ "Vertex texture coord 0",                     OpenGLSiSymbolAttribTexcoord0 },
	{ "Vertex texture coord 1",                     OpenGLSiSymbolAttribTexcoord1 },
	{ "Vertex texture coord 2",                     OpenGLSiSymbolAttribTexcoord2 },
	{ "Vertex texture coord 3",                     OpenGLSiSymbolAttribTexcoord3 },
	{ "Vertex texture coord 4",                     OpenGLSiSymbolAttribTexcoord4 },
	{ "Vertex texture coord 5",                     OpenGLSiSymbolAttribTexcoord5 },
	{ "Vertex texture coord 6",                     OpenGLSiSymbolAttribTexcoord6 },
	{ "Vertex texture coord 7",                     OpenGLSiSymbolAttribTexcoord7 },
	{ "Vertex texture coord 8",                     OpenGLSiSymbolAttribTexcoord8 },
	{ "Vertex texture coord 9",                     OpenGLSiSymbolAttribTexcoord9 },
	{ "Vertex texture coord 10",                    OpenGLSiSymbolAttribTexcoord10 },
	{ "Vertex texture coord 11",                    OpenGLSiSymbolAttribTexcoord11 },
	{ "Vertex texture coord 12",                    OpenGLSiSymbolAttribTexcoord12 },
	{ "Vertex texture coord 13",                    OpenGLSiSymbolAttribTexcoord13 },
	{ "Vertex texture coord 14",                    OpenGLSiSymbolAttribTexcoord14 },
	{ "Vertex texture coord 15",                    OpenGLSiSymbolAttribTexcoord15 },
	{ "Generic attrib 0",                           OpenGLSiSymbolAttribGeneric0 },
	{ "Generic attrib 1",                           OpenGLSiSymbolAttribGeneric1 },
	{ "Generic attrib 2",                           OpenGLSiSymbolAttribGeneric2 },
	{ "Generic attrib 3",                           OpenGLSiSymbolAttribGeneric3 },
	{ "Generic attrib 4",                           OpenGLSiSymbolAttribGeneric4 },
	{ "Generic attrib 5",                           OpenGLSiSymbolAttribGeneric5 },
	{ "Generic attrib 6",                           OpenGLSiSymbolAttribGeneric6 },
	{ "Generic attrib 7",                           OpenGLSiSymbolAttribGeneric7 },
	{ "Generic attrib 8",                           OpenGLSiSymbolAttribGeneric8 },
	{ "Generic attrib 9",                           OpenGLSiSymbolAttribGeneric9 },
	{ "Generic attrib 10",                          OpenGLSiSymbolAttribGeneric10 },
	{ "Generic attrib 11",                          OpenGLSiSymbolAttribGeneric11 },
	{ "Generic attrib 12",                          OpenGLSiSymbolAttribGeneric12 },
	{ "Generic attrib 13",                          OpenGLSiSymbolAttribGeneric13 },
	{ "Generic attrib 14",                          OpenGLSiSymbolAttribGeneric14 },
	{ "Generic attrib 15",                          OpenGLSiSymbolAttribGeneric15 },
	{ "Generic attrib 16",                          OpenGLSiSymbolAttribGeneric16 },
	{ "Generic attrib 17",                          OpenGLSiSymbolAttribGeneric17 },
	{ "Generic attrib 18",                          OpenGLSiSymbolAttribGeneric18 },
	{ "Generic attrib 19",                          OpenGLSiSymbolAttribGeneric19 },
	{ "Generic attrib 20",                          OpenGLSiSymbolAttribGeneric20 },
	{ "Generic attrib 21",                          OpenGLSiSymbolAttribGeneric21 },
	{ "Generic attrib 22",                          OpenGLSiSymbolAttribGeneric22 },
	{ "Generic attrib 23",                          OpenGLSiSymbolAttribGeneric23 },
	{ "Generic attrib 24",                          OpenGLSiSymbolAttribGeneric24 },
	{ "Generic attrib 25",                          OpenGLSiSymbolAttribGeneric25 },
	{ "Generic attrib 26",                          OpenGLSiSymbolAttribGeneric26 },
	{ "Generic attrib 27",                          OpenGLSiSymbolAttribGeneric27 },
	{ "Generic attrib 28",                          OpenGLSiSymbolAttribGeneric28 },
	{ "Generic attrib 29",                          OpenGLSiSymbolAttribGeneric29 },
	{ "Generic attrib 30",                          OpenGLSiSymbolAttribGeneric30 },
	{ "Generic attrib 31",                          OpenGLSiSymbolAttribGeneric31 },
	{ "Generic attrib 32",                          OpenGLSiSymbolAttribGeneric32 },
	{ "Generic attrib 33",                          OpenGLSiSymbolAttribGeneric33 },
	{ "Generic attrib 34",                          OpenGLSiSymbolAttribGeneric34 },
	{ "Generic attrib 35",                          OpenGLSiSymbolAttribGeneric35 },
	{ "Generic attrib 36",                          OpenGLSiSymbolAttribGeneric36 },
	{ "Generic attrib 37",                          OpenGLSiSymbolAttribGeneric37 },
	{ "Generic attrib 38",                          OpenGLSiSymbolAttribGeneric38 },
	{ "Generic attrib 39",                          OpenGLSiSymbolAttribGeneric39 },
	{ "Varying out",                                OpenGLSiSymbolFsvaryingout },
	{ "VertexID",                                   OpenGLSiSymbolAttribVertexid },
	{ "InstanceID",                                 OpenGLSiSymbolAttribInstanceid },
	{ "GLSL Subroutine Uniform",                    OpenGLSiSymbolSubroutineUniform },
	{ "GLSL Subroutine Function",                   OpenGLSiSymbolSubroutineFunction },
	{ "base varying in symbol of separate mode",    OpenGLSiSymbolSeparateVaryingInBase },
	{ "base varying out symbol of sparate mode",    OpenGLSiSymbolSeparateVaryingOutBase },
	{ "derived varying in symbol of separate mode", OpenGLSiSymbolSeparateVaryingInDerived },
	{ "derived varying out symbol of sparate mode", OpenGLSiSymbolSeparateVaryingOutDerived },
	{ "varying in of nonseparate mode",             OpenGLSiSymbolNonseparateVaryingIn },
	{ "varying out of nonseparate mode",            OpenGLSiSymbolNonseparateVaryingOut },
	{ "shader storage buffer",                      OpenGLSiSymbolShaderStorageBuffer },
	{ "input valid bits for sparse texture",        OpenGLSiSymbolAttribInputvalid }
};


OpenGLSiBinInput::OpenGLSiBinInput(const char *buffer)
{
	OpenGLSiBinInput *input_ptr = (OpenGLSiBinInput *)buffer;
	type = input_ptr->type;
	if (type < OpenGLSiInputAttrib || type > OpenGLSiInputTextureresourceid)
		throw Asm::Error(misc::fmt("Invalid input type (%d)", type));
	voffset = input_ptr->voffset;
	poffset = input_ptr->poffset;
	for (int i = 0; i < 4; ++i)
		swizzles[i] = input_ptr->swizzles[i];
}


OpenGLSiBinOutput::OpenGLSiBinOutput(const char *buffer)
{
	OpenGLSiBinOutput *output_ptr = (OpenGLSiBinOutput *)buffer;

	type = output_ptr->type;
	if (type < OpenGLSiOutputPos || type > OpenGLSiOutputStreamid )
		throw Asm::Error(misc::fmt("Invalid output type (%d)", type));
	voffset = output_ptr->voffset;
	poffset = output_ptr->poffset;
	data_type = output_ptr->data_type;
	if (data_type < OpenGLSiSymbolDatatypeFirst || data_type > OpenGLSiSymbolDatatypeLast)
		throw Asm::Error(misc::fmt("Invalid output datatype (%d)",
				data_type));
	// Make a copy of name
	name = new char[strlen(output_ptr->name) + 1];
	strcpy(this->name, output_ptr->name);
}


OpenGLSiBinOutput::~OpenGLSiBinOutput()
{
	// Free name
	delete [] name;
}


OpenGLSiBinInfo::OpenGLSiBinInfo(const char *buffer)
{
	OpenGLSiBinInfo *info_ptr = (OpenGLSiBinInfo *)buffer;

	fetch4Mask                   = info_ptr->fetch4Mask;
	textureMask                  = info_ptr->textureMask;
	textureSamplerMask           = info_ptr->textureSamplerMask;
	for (int i= 0; i < MAX_TEXTURE_RESOURCES/32; ++i)
	{
		textureResourceMask[i]   = info_ptr->textureResourceMask[i];
		bufferMask[i]            = info_ptr->bufferMask[i];
	}
	textureResourceBoundCount    = info_ptr->textureResourceBoundCount;
	maxScratchRegsNeeded         = info_ptr->maxScratchRegsNeeded;
	constantBufferMask           = info_ptr->constantBufferMask;
	uavOpIsUsed                  = info_ptr->uavOpIsUsed;
	uavInCB                      = info_ptr->uavInCB;
	uavResourceCount             = info_ptr->uavResourceCount;
	uavAtomicOpIsUsed            = info_ptr->uavAtomicOpIsUsed;
	maxUniformLocation           = info_ptr->maxUniformLocation;
	maxSubroutineIndex           = info_ptr->maxSubroutineIndex;
	maxSubroutineUniformLocation = info_ptr->maxSubroutineUniformLocation;
	shaderinfo                   = info_ptr->shaderinfo;
	textureResourceBound         = info_ptr->textureResourceBound;
	uavResourceMaskUnion         = info_ptr->uavResourceMaskUnion;
	uavRtnBufStrideUnion         = info_ptr->uavRtnBufStrideUnion;
	uavDynamicResouceMapUnion    = info_ptr->uavDynamicResouceMapUnion;
	max_valid_offset             = info_ptr->max_valid_offset;
}

OpenGLSiBinUsageinfo::OpenGLSiBinUsageinfo(const char *buffer)
{
	OpenGLSiBinUsageinfo *usageinfo_ptr = (OpenGLSiBinUsageinfo *)buffer;

	arbProgramParameter             = usageinfo_ptr->arbProgramParameter;
	layoutParamMask                 = usageinfo_ptr->layoutParamMask;
	usesPrimitiveID                 = usageinfo_ptr->usesPrimitiveID;
	usesClipDistance                = usageinfo_ptr->usesClipDistance;

	texImageMask                    = usageinfo_ptr->texImageMask;
	usesTexImageMask                = usageinfo_ptr->usesTexImageMask;
	usesShadowMask                  = usageinfo_ptr->usesShadowMask;
	uavImageMask                    = usageinfo_ptr->uavImageMask;
	uavImageMaskShaderStorageBuffer = usageinfo_ptr->uavImageMaskShaderStorageBuffer;
	usesBufMask                     = usageinfo_ptr->usesBufMask;

	imageTypeAndFormatSize          = usageinfo_ptr->imageTypeAndFormatSize;
	textureTypeSize                 = usageinfo_ptr->textureTypeSize;

	shaderusageinfo                 = usageinfo_ptr->shaderusageinfo;

	maxOffset                       = usageinfo_ptr->maxOffset;

	textureTypeUnion                = usageinfo_ptr->textureTypeUnion;
	imageTypeUnion                  = usageinfo_ptr->imageTypeUnion;
	imageFormatUnion                = usageinfo_ptr->imageFormatUnion;
}

OpenGLSiBinSymbol::OpenGLSiBinSymbol(const char *buffer)
{
	OpenGLSiBinSymbol *symbol_ptr = (OpenGLSiBinSymbol *)buffer;

	type                = symbol_ptr->type;
	dataType            = symbol_ptr->dataType;

	qualifierUnion      = symbol_ptr->qualifierUnion;

	vvalue              = symbol_ptr->vvalue;
	vcbindex            = symbol_ptr->vcbindex;
	vswizzle            = symbol_ptr->vswizzle;
	gvalue              = symbol_ptr->gvalue;
	gcbindex            = symbol_ptr->gcbindex;
	gswizzle            = symbol_ptr->gswizzle;
	fvalue              = symbol_ptr->fvalue;
	fcbindex            = symbol_ptr->fcbindex;
	fswizzle            = symbol_ptr->fswizzle;
	hvalue              = symbol_ptr->hvalue;
	hcbindex            = symbol_ptr->hcbindex;
	hswizzle            = symbol_ptr->hswizzle;
	dvalue              = symbol_ptr->dvalue;
	dcbindex            = symbol_ptr->dcbindex;
	dswizzle            = symbol_ptr->dswizzle;
	cvalue              = symbol_ptr->cvalue;
	ccbindex            = symbol_ptr->ccbindex;
	cswizzle            = symbol_ptr->cswizzle;
	size                = symbol_ptr->size;
	count               = symbol_ptr->count;

	isArray             = symbol_ptr->isArray;
	matrixStride        = symbol_ptr->matrixStride;
	subSize             = symbol_ptr->subSize;
	uniformBinding      = symbol_ptr->uniformBinding;
	layoutLocation      = symbol_ptr->layoutLocation;
	layoutIndex         = symbol_ptr->layoutIndex;
	uniformOffset       = symbol_ptr->uniformOffset;
	resourceIndex       = symbol_ptr->resourceIndex;

	subroutineTypeID    = symbol_ptr->subroutineTypeID;
	topLevelArraySize   = symbol_ptr->topLevelArraySize;
	topLevelArrayStride = symbol_ptr->topLevelArrayStride;

	name                = new char[strlen(symbol_ptr->name) + 1];
	strcpy(name, symbol_ptr->name);
	baseName            = new char[strlen(symbol_ptr->baseName) + 1];
	strcpy(baseName, symbol_ptr->baseName);
	uniformBlockName    = new char[strlen(symbol_ptr->uniformBlockName) + 1];
	strcpy(uniformBlockName, symbol_ptr->uniformBlockName);
	mangledName         = new char[strlen(symbol_ptr->mangledName) + 1];	
	strcpy(mangledName, symbol_ptr->mangledName);
}

OpenGLSiBinSymbol::~OpenGLSiBinSymbol()
{
	delete [] name;
	delete [] baseName;
	delete [] uniformBlockName;
	delete [] mangledName;
}

OpenGLSiBinVertexShaderMetadata::OpenGLSiBinVertexShaderMetadata(const char *buffer)
{
	OpenGLSiBinVertexShaderMetadata *meta_ptr = (OpenGLSiBinVertexShaderMetadata *)buffer;

	// Same for all metadata
	uSizeInBytes                    = meta_ptr->uSizeInBytes;
	stats                           = meta_ptr->stats;
	dep                             = meta_ptr->dep;
	X32XpPvtData                    = meta_ptr->X32XpPvtData;
	u32PvtDataSizeInBytes           = meta_ptr->u32PvtDataSizeInBytes;
	fConstantUsage                  = meta_ptr->fConstantUsage;
	bConstantUsage                  = meta_ptr->bConstantUsage;
	iConstantUsage                  = meta_ptr->iConstantUsage;
	uShaderType                     = meta_ptr->uShaderType;
	eInstSet                        = meta_ptr->eInstSet;
	for (int i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
	{
		texResourceUsage[i]         = meta_ptr->texResourceUsage[i];
		fetch4ResourceUsage[i]      = meta_ptr->fetch4ResourceUsage[i];
	}
	for (int j = 0; j < (MAX_NUM_UAV + 31) / 32; ++j)
		uavResourceUsage[j]         = meta_ptr->uavResourceUsage[j];

	texSamplerUsage                 = meta_ptr->texSamplerUsage;
	constBufUsage                   = meta_ptr->constBufUsage;
	for (int i = 0; i < MAX_NUM_SAMPLER; ++i)
		for (int j = 0; j < (MAX_NUM_RESOURCE + 31) / 32; ++j)
		texSamplerResourceMapping[i][j] = meta_ptr->texSamplerResourceMapping[i][j];
	
	NumConstOpportunities           = meta_ptr->NumConstOpportunities;
	for (int i = 0; i < (MAX_NUM_RESOURCE + 31) /32; ++i)
		ResourcesAffectAlphaOutput[i]   = meta_ptr->ResourcesAffectAlphaOutput[i];
	X32XhShaderMemHandle            = meta_ptr->X32XhShaderMemHandle;
	for (int i = 0; i < SC_SI_NUM_INTBUF; ++i)
		X32XhConstBufferMemHandle[i]    = meta_ptr->X32XhConstBufferMemHandle[i];
	CodeLenInByte                   = meta_ptr->CodeLenInByte;
	u32UserElementCount             = meta_ptr->u32UserElementCount;
	for (int i = 0; i < SC_SI_NUM_USER_ELEMENT; ++i)
		pUserElement[i]             = meta_ptr->pUserElement[i];

	u32NumVgprs                     = meta_ptr->u32NumVgprs;
	u32NumSgprs                     = meta_ptr->u32NumSgprs;
	u32FloatMode                    = meta_ptr->u32FloatMode;
	bIeeeMode                       = meta_ptr->bIeeeMode;
	bUsesPrimId                     = meta_ptr->bUsesPrimId;
	bUsesVertexId                   = meta_ptr->bUsesVertexId;
	scratchSize                     = meta_ptr->scratchSize;

	// Unique for vertex shader metadata
	numVsInSemantics = meta_ptr->numVsInSemantics;
	for (int i = 0; i < SC_SI_VS_MAX_INPUTS; ++i)
		vsInSemantics[i] = meta_ptr->vsInSemantics[i];

	numVsOutSemantics = meta_ptr->numVsOutSemantics;
	for (int i = 0; i < SC_SI_VS_MAX_OUTPUTS; ++i)
		vsOutSemantics[i] = meta_ptr->vsOutSemantics[i];

	spiShaderPgmRsrcUnion = meta_ptr->spiShaderPgmRsrcUnion;

	paClVsOutCntl = meta_ptr->paClVsOutCntl;
	OpenGLSiSpiVsOutConfigUnion = meta_ptr->OpenGLSiSpiVsOutConfigUnion;
	OpenGLSiSpiShaderPosFormatUnion = meta_ptr->OpenGLSiSpiShaderPosFormatUnion;

	vgtStrmoutConfig = meta_ptr->vgtStrmoutConfig;

	vgprCompCnt = meta_ptr->vgprCompCnt;

	exportVertexSize = meta_ptr->exportVertexSize;

	useEdgeFlags = meta_ptr->useEdgeFlags;
	remapClipDistance = meta_ptr->remapClipDistance;
	hwShaderStage = meta_ptr->hwShaderStage;
	compileFlags = meta_ptr->compileFlags;
	gsMode = meta_ptr->gsMode;
}

OpenGLSiBinPixelShaderMetadata::OpenGLSiBinPixelShaderMetadata(const char *buffer)
{
	OpenGLSiBinPixelShaderMetadata *meta_ptr = (OpenGLSiBinPixelShaderMetadata *)buffer;

	// Same for all metadata
	uSizeInBytes                    = meta_ptr->uSizeInBytes;
	stats                           = meta_ptr->stats;
	dep                             = meta_ptr->dep;
	X32XpPvtData                    = meta_ptr->X32XpPvtData;
	u32PvtDataSizeInBytes           = meta_ptr->u32PvtDataSizeInBytes;
	fConstantUsage                  = meta_ptr->fConstantUsage;
	bConstantUsage                  = meta_ptr->bConstantUsage;
	iConstantUsage                  = meta_ptr->iConstantUsage;
	uShaderType                     = meta_ptr->uShaderType;
	eInstSet                        = meta_ptr->eInstSet;
	for (int i = 0; i < (MAX_NUM_RESOURCE + 31) / 32; ++i)
	{
		texResourceUsage[i]         = meta_ptr->texResourceUsage[i];
		fetch4ResourceUsage[i]      = meta_ptr->fetch4ResourceUsage[i];
	}
	for (int j = 0; j < (MAX_NUM_UAV + 31) / 32; ++j)
		uavResourceUsage[j]         = meta_ptr->uavResourceUsage[j];

	texSamplerUsage                 = meta_ptr->texSamplerUsage;
	constBufUsage                   = meta_ptr->constBufUsage;
	for (int i = 0; i < MAX_NUM_SAMPLER; ++i)
		for (int j = 0; j < (MAX_NUM_RESOURCE + 31) / 32; ++j)
		texSamplerResourceMapping[i][j] = meta_ptr->texSamplerResourceMapping[i][j];
	
	NumConstOpportunities           = meta_ptr->NumConstOpportunities;
	for (int i = 0; i < (MAX_NUM_RESOURCE + 31) /32; ++i)
		ResourcesAffectAlphaOutput[i]   = meta_ptr->ResourcesAffectAlphaOutput[i];
	X32XhShaderMemHandle            = meta_ptr->X32XhShaderMemHandle;
	for (int i = 0; i < SC_SI_NUM_INTBUF; ++i)
		X32XhConstBufferMemHandle[i]    = meta_ptr->X32XhConstBufferMemHandle[i];
	CodeLenInByte                   = meta_ptr->CodeLenInByte;
	u32UserElementCount             = meta_ptr->u32UserElementCount;
	for (int i = 0; i < SC_SI_NUM_USER_ELEMENT; ++i)
		pUserElement[i]             = meta_ptr->pUserElement[i];

	u32NumVgprs                     = meta_ptr->u32NumVgprs;
	u32NumSgprs                     = meta_ptr->u32NumSgprs;
	u32FloatMode                    = meta_ptr->u32FloatMode;
	bIeeeMode                       = meta_ptr->bIeeeMode;
	bUsesPrimId                     = meta_ptr->bUsesPrimId;
	bUsesVertexId                   = meta_ptr->bUsesVertexId;
	scratchSize                     = meta_ptr->scratchSize;

	// Unique for Pixel Shader
	numPsInSemantics = meta_ptr->numPsInSemantics;
	for (int i = 0; i < SC_SI_PS_MAX_INPUTS; ++i)
	{
		psInSemantics[i] = meta_ptr->psInSemantics[SC_SI_PS_MAX_INPUTS];
		psInTexCoordIndex[i] = meta_ptr->psInTexCoordIndex[i];		
	}	

	spiShaderPgmRsrc2PsUnion = meta_ptr->spiShaderPgmRsrc2PsUnion;

	spiShaderZFormat = meta_ptr->spiShaderZFormat;
	spiPsInControl = meta_ptr->spiPsInControl;
	spiPsInputAddr = meta_ptr->spiPsInputAddr;
	spiPsInputEna = meta_ptr->spiPsInputEna;

	spiBarycCntl = meta_ptr->spiBarycCntl;

	dbShaderControl = meta_ptr->dbShaderControl;
	cbShaderMask = meta_ptr->cbShaderMask;

	exportPatchCodeSize = meta_ptr->exportPatchCodeSize;
	numPsExports = meta_ptr->numPsExports;
	dualBlending = meta_ptr->dualBlending;
	for (int i = 0; i < SC_SI_PS_MAX_OUTPUTS; ++i)
		exportPatchInfo[i] = meta_ptr->exportPatchInfo[i];
	defaultExportFmt = meta_ptr->defaultExportFmt;
}

OpenGLSiShaderBinaryCommon::OpenGLSiShaderBinaryCommon(const char *buffer, unsigned int size)
	: ELFReader::File(buffer, size)
{
	int num_section = getNumSections();
	ELFReader::Section *section;

	for (int i = 0; i < num_section; ++i)
	{
		section = getSection(i);
		if (section->getName() == ".inputs")
		{
			DecodeInputs(section);
			continue;
		}
		if (section->getName() == ".outputs")
		{
			DecodeOutputs(section);
			continue;
		}
		if (section->getName() == ".info")
		{
			DecodeInfo(section);
			continue;
		}
		if (section->getName() == ".usageinfo")
		{
			DecodeUsageInfo(section);
			continue;
		}
		if (section->getName() == ".symbols")
		{
			DecodeSymbols(section);
			continue;
		}
	}
}

void OpenGLSiShaderBinaryCommon::DecodeInputs(ELFReader::Section *section)
{
	assert(section->getName() != ".inputs");

	// Calculate number of inputs based on section size and structure size
	if (section->getSize() % sizeof(OpenGLSiBinInput))
		throw Asm::Error(misc::fmt("Section size (%d) must be "
				"multiples of input structure (%d).",
				(int) section->getSize(),
				(int) sizeof(OpenGLSiBinInput)));
	else
	{
		int input_count = section->getSize() / sizeof(OpenGLSiBinInput);
		for (int i = 0; i < input_count; ++i)
			inputs.push_back(std::unique_ptr<OpenGLSiBinInput>(new 
				OpenGLSiBinInput(section->getBuffer() + i * sizeof(OpenGLSiBinInput))));
	}
}

void OpenGLSiShaderBinaryCommon::DecodeOutputs(ELFReader::Section *section)
{
	assert(section->getName() != ".outputs");

	const char *output_ptr = section->getBuffer();
	do
	{
		outputs.push_back(std::unique_ptr<OpenGLSiBinOutput>(new
			OpenGLSiBinOutput(output_ptr)));
		output_ptr = outputs.back()->getPtrNextOutput(output_ptr);
	} while (output_ptr < section->getBuffer() + section->getSize());
}

void OpenGLSiShaderBinaryCommon::DecodeInfo(ELFReader::Section *section)
{
	assert(section->getName() != ".info");

	// Size must match
	if (section->getSize() != sizeof(OpenGLSiBinInfo))
		throw Asm::Error(misc::fmt("Section size (%d) != "
				"sizeof(OpenGLSiBinInfo) (%d)",
				(int) section->getSize(),
				(int) sizeof(OpenGLSiBinInfo)));
	else
		info.reset(new OpenGLSiBinInfo(section->getBuffer()));
}

void OpenGLSiShaderBinaryCommon::DecodeUsageInfo(ELFReader::Section *section)
{
	assert(section->getName() != ".usageinfo");

	// Size must match
	if (section->getSize() != sizeof(OpenGLSiBinUsageinfo))
		throw Asm::Error(misc::fmt("Section size (%d) != "
				"sizeof(OpenGLSiBinUsageinfo) (%d)",
				(int) section->getSize(),
				(int) sizeof(OpenGLSiBinUsageinfo)));
	else
		usageinfo.reset(new OpenGLSiBinUsageinfo(section->getBuffer()));

}

void OpenGLSiShaderBinaryCommon::DecodeSymbols(ELFReader::Section *section)
{
	assert(section->getName() != ".symbols");

	// First 4 bypes in .symbol section is the count of symbols
	int symbol_count = *(int *)section->getBuffer();
	const char *symbols_ptr = section->getBuffer() + 4;

	for (int i = 0; i < symbol_count; ++i)
	{
		symbols.push_back(std::unique_ptr<OpenGLSiBinSymbol>(new 
			OpenGLSiBinSymbol(symbols_ptr)));
		symbols_ptr = symbols.back()->getPtrNextSymbol(symbols_ptr);
	}
}

// Vertex shader binary constructor
OpenGLSiShaderBinaryVertex::OpenGLSiShaderBinaryVertex(const char *buffer, unsigned size)
	: OpenGLSiShaderBinaryCommon(buffer, size)
{
	int num_section = getNumSections();
	ELFReader::Section *section;

	for (int i = 0; i < num_section; ++i)
	{
		section = getSection(i);
		if (section->getName() == ".text")
			meta.reset(new OpenGLSiBinVertexShaderMetadata(section->getBuffer()));
		break;
	}
}

// Fragment shader binary constructor
OpenGLSiShaderBinaryPixel::OpenGLSiShaderBinaryPixel(const char *buffer, unsigned size)
	: OpenGLSiShaderBinaryCommon(buffer, size)
{
	int num_section = getNumSections();
	ELFReader::Section *section;

	for (int i = 0; i < num_section; ++i)
	{
		section = getSection(i);
		if (section->getName() == ".text")
			meta.reset(new OpenGLSiBinPixelShaderMetadata(section->getBuffer()));
		break;
	}
}

// OpenGL ELF program binary, get by glGetProgramBinary
OpenGLSiProgramBinary::OpenGLSiProgramBinary(const char *buffer, unsigned buffer_size)
	: ELFReader::File(buffer, buffer_size)
{
	for (auto &symbol : this->getSymbols())
	{
		ELFReader::Section *section = symbol->getSection();
		const char *elf_bin_buf   = section->getBuffer() + symbol->getValue();
		unsigned    elf_bin_size  = section->getSize();

		if (symbol->getName() == "__Shader_V_AsicID_23_ElfBinary_0_")
			vertex_shader.reset(new OpenGLSiShaderBinaryVertex(elf_bin_buf, elf_bin_size));
		else if (symbol->getName() == "__Shader_F_AsicID_23_ElfBinary_0_")
			pixel_shader.reset(new OpenGLSiShaderBinaryPixel(elf_bin_buf, elf_bin_size));
		else
			return;
	}
}

} // namespace SI
