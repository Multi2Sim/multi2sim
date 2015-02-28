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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_SHADER_BINARY_H
#define ARCH_SOUTHERN_ISLANDS_ASM_SHADER_BINARY_H

#include <cstring>
#include <vector>
#include <memory>
#include <lib/cpp/ELFReader.h>
#include "Wrapper.h"

// Forward declaration
namespace SI
{
	class Shader;
}  // namespace SI

namespace SI
{

// ---------------------------------defines----------------------------------
// Common 
#define MAX_USER_ELEMENTS 16
#define MAX_SEMANTICS_MAPPINGS 16
#define MAX_NUM_SAMPLER  32
#define MAX_NUM_RESOURCE 256
#define MAX_NUM_UAV      1024
#define MAX_CONSTANTS    256

#define MAX_UAV      12                                 // mask uav supported
#define MAX_TRANSFORM_FEEDBACK_BUFFERS 4                // maximum number of transform feedback buffers supported

#define MAX_UAV_RESOURCES 1024                          // The maximum number of UAVs supported
#define MAX_UAV_MASKS (((MAX_UAV_RESOURCES) + 31) / 32) // The maximum of UAV masks

#define MAX_TEXTURE_IMAGE_UNITS        32               // The maximum number of texture image units supported by GLL
#define MAX_UAV_IMAGE_UNITS            8                // The maximum number of image (UAV) supported by GLL
#define NUM_GLOBAL_RETURN_BUFFERS      8                // max global return buffers
#define MAX_TEXTURE_RESOURCES          160              // The maximus number of texture resource
#define PS_MAX_OUTPUTS                 8                // Max Render Targets

// SI specific
#define SC_SI_VS_MAX_INPUTS       32
#define SC_SI_VS_MAX_OUTPUTS      32
#define SC_SI_PS_MAX_INPUTS       32
#define SC_SI_PS_MAX_OUTPUTS      8
#define SC_SI_NUM_INTBUF          2
#define SC_SI_NUM_USER_ELEMENT    16

// Max PS export patch code size in DWORDs
#define SC_SI_MAX_EXPORT_CODE_SIZE 6
#define SC_SI_NUM_EXPORT_FMT  0xa

// Number of SO streams
#define SC_SI_SO_MAX_STREAMS  4
// Number of SO buffers
#define SC_SI_SO_MAX_BUFFERS  4


// ---------------------------------Enums----------------------------------
// Shader Types
enum OpenGLSiShaderStage
{
	OpenGLSiShaderVertex = 0,
	OpenGLSiShaderGeometry,
	OpenGLSiShaderHull,       // Aka Tess Control Shader
	OpenGLSiShaderDomain,     // Aka Tess Evaluastion Shader
	OpenGLSiShaderPixel  = 4, // Aka Fragment Shader
	OpenGLSiShaderCompute,
	OpenGLSiShaderInvalid
};

enum OpenGLSiBinInputSwizzleType
{
	OpenGLSiSwizzleX,	// Swizzle The X Component Into This Component
	OpenGLSiSwizzleY,	// Swizzle The Y Component Into This Component
	OpenGLSiSwizzleZ,	// Swizzle The Z Component Into This Component
	OpenGLSiSwizzleW,	// Swizzle The W Component Into This Component
	OpenGLSiSwizzle0,	// Swizzle Constant 0 Into This Component
	OpenGLSiSwizzle1	// Swizzle Constant 1 Into This Component
};

enum OpenGLSiBinInputType
{
	OpenGLSiInputAttrib,           // Generic Attribute
	OpenGLSiInputColor,            // Primary Color
	OpenGLSiInputSecondarycolor,   // Secondary Color
	OpenGLSiInputTexcoord,         // Texture Coordinate
	OpenGLSiInputTexid,            // Texture Unit Id
	OpenGLSiInputBufferid,         // Buffer Unit Id
	OpenGLSiInputConstantbufferid, // Constant Buffer Unit Id
	OpenGLSiInputTextureresourceid // Texture Resource Id	
};

enum OpenGLSiBinOutputType
{
	OpenGLSiOutputPos,            // Position
	OpenGLSiOutputPointsize,      // Point Size
	OpenGLSiOutputColor,          // Primary Color, Offset 0 Is Front, Offset 1 Is Back
	OpenGLSiOutputSecondarycolor, // Secondary Color, Offset 0 Is Front, Offset 1 Is Back
	OpenGLSiOutputGeneric,        // Texture Coordinate And User Define Varyings For Pre R5xx Asics, But On R6xx Above, Generic Contains Colors
	OpenGLSiOutputDepth,          // Fragment Depth
	OpenGLSiOutputClipdistance,   // Clip Distance
	OpenGLSiOutputPrimitiveid,    // Primitiveid
	OpenGLSiOutputLayer,          // Layer
	OpenGLSiOutputViewportindex,  // Viewportindex
	OpenGLSiOutputStencil,        // Fragment Stencil Ref Value
	OpenGLSiOutputStencilValue,   // Fragment Stencil Operation Value
	OpenGLSiOutputSamplemask,     // Output Sample Coverage Mask (fs Only)
	OpenGLSiOutputStreamid        // Ouput Stream Id
};

// Symbol Types
enum OpenGLSiBinSymbolType
{
	OpenGLSiSymbolUniform,                     // Glsl Uniform
	OpenGLSiSymbolBindableUniform,             // Glsl Bindable Uniform
	OpenGLSiSymbolUniformBlock,                // Glsl Uniform Block
	OpenGLSiSymbolAttribPosition,              // Vertex Position
	OpenGLSiSymbolAttribNormal,                // Vertex Normal
	OpenGLSiSymbolAttribPrimaryColor,          // Vertex Primary Color
	OpenGLSiSymbolAttribSecondaryColor,        // Vertex Secondary Color
	OpenGLSiSymbolAttribFogcoord,              // Vertex Fog Coord
	OpenGLSiSymbolAttribEdgeflag,              // Vertex Edge Flag
	OpenGLSiSymbolAttribTexcoord0,             // Vertex Texture Coord 0
	OpenGLSiSymbolAttribTexcoord1,             // Vertex Texture Coord 1
	OpenGLSiSymbolAttribTexcoord2,             // Vertex Texture Coord 2
	OpenGLSiSymbolAttribTexcoord3,             // Vertex Texture Coord 3
	OpenGLSiSymbolAttribTexcoord4,             // Vertex Texture Coord 4
	OpenGLSiSymbolAttribTexcoord5,             // Vertex Texture Coord 5
	OpenGLSiSymbolAttribTexcoord6,             // Vertex Texture Coord 6
	OpenGLSiSymbolAttribTexcoord7,             // Vertex Texture Coord 7
	OpenGLSiSymbolAttribTexcoord8,             // Vertex Texture Coord 8
	OpenGLSiSymbolAttribTexcoord9,             // Vertex Texture Coord 9
	OpenGLSiSymbolAttribTexcoord10,            // Vertex Texture Coord 10
	OpenGLSiSymbolAttribTexcoord11,            // Vertex Texture Coord 11
	OpenGLSiSymbolAttribTexcoord12,            // Vertex Texture Coord 12
	OpenGLSiSymbolAttribTexcoord13,            // Vertex Texture Coord 13
	OpenGLSiSymbolAttribTexcoord14,            // Vertex Texture Coord 14
	OpenGLSiSymbolAttribTexcoord15,            // Vertex Texture Coord 15
	OpenGLSiSymbolAttribGeneric0,              // Generic Attrib 0
	OpenGLSiSymbolAttribGeneric1,              // Generic Attrib 1
	OpenGLSiSymbolAttribGeneric2,              // Generic Attrib 2
	OpenGLSiSymbolAttribGeneric3,              // Generic Attrib 3
	OpenGLSiSymbolAttribGeneric4,              // Generic Attrib 4
	OpenGLSiSymbolAttribGeneric5,              // Generic Attrib 5
	OpenGLSiSymbolAttribGeneric6,              // Generic Attrib 6
	OpenGLSiSymbolAttribGeneric7,              // Generic Attrib 7
	OpenGLSiSymbolAttribGeneric8,              // Generic Attrib 8
	OpenGLSiSymbolAttribGeneric9,              // Generic Attrib 9
	OpenGLSiSymbolAttribGeneric10,             // Generic Attrib 10
	OpenGLSiSymbolAttribGeneric11,             // Generic Attrib 11
	OpenGLSiSymbolAttribGeneric12,             // Generic Attrib 12
	OpenGLSiSymbolAttribGeneric13,             // Generic Attrib 13
	OpenGLSiSymbolAttribGeneric14,             // Generic Attrib 14
	OpenGLSiSymbolAttribGeneric15,             // Generic Attrib 15
	OpenGLSiSymbolAttribGeneric16,             // Generic Attrib 16
	OpenGLSiSymbolAttribGeneric17,             // Generic Attrib 17
	OpenGLSiSymbolAttribGeneric18,             // Generic Attrib 18
	OpenGLSiSymbolAttribGeneric19,             // Generic Attrib 19
	OpenGLSiSymbolAttribGeneric20,             // Generic Attrib 20
	OpenGLSiSymbolAttribGeneric21,             // Generic Attrib 21
	OpenGLSiSymbolAttribGeneric22,             // Generic Attrib 22
	OpenGLSiSymbolAttribGeneric23,             // Generic Attrib 23
	OpenGLSiSymbolAttribGeneric24,             // Generic Attrib 24
	OpenGLSiSymbolAttribGeneric25,             // Generic Attrib 25
	OpenGLSiSymbolAttribGeneric26,             // Generic Attrib 26
	OpenGLSiSymbolAttribGeneric27,             // Generic Attrib 27
	OpenGLSiSymbolAttribGeneric28,             // Generic Attrib 28
	OpenGLSiSymbolAttribGeneric29,             // Generic Attrib 29
	OpenGLSiSymbolAttribGeneric30,             // Generic Attrib 30
	OpenGLSiSymbolAttribGeneric31,             // Generic Attrib 31
	OpenGLSiSymbolAttribGeneric32,             // Generic Attrib 32
	OpenGLSiSymbolAttribGeneric33,             // Generic Attrib 33
	OpenGLSiSymbolAttribGeneric34,             // Generic Attrib 34
	OpenGLSiSymbolAttribGeneric35,             // Generic Attrib 35
	OpenGLSiSymbolAttribGeneric36,             // Generic Attrib 36
	OpenGLSiSymbolAttribGeneric37,             // Generic Attrib 37
	OpenGLSiSymbolAttribGeneric38,             // Generic Attrib 38
	OpenGLSiSymbolAttribGeneric39,             // Generic Attrib 39
	OpenGLSiSymbolFsvaryingout,                // Varying Out
	OpenGLSiSymbolAttribVertexid,              // Vertexid
	OpenGLSiSymbolAttribInstanceid,            // Instanceid
	OpenGLSiSymbolSubroutineUniform,           // Glsl Subroutine Uniform
	OpenGLSiSymbolSubroutineFunction,          // Glsl Subroutine Function
	OpenGLSiSymbolSeparateVaryingInBase,       // Base Varying In Symbol Of Separate Mode
	OpenGLSiSymbolSeparateVaryingOutBase,      // Base Varying Out Symbol Of Sparate Mode
	OpenGLSiSymbolSeparateVaryingInDerived,    // Derived Varying In Symbol Of Separate Mode
	OpenGLSiSymbolSeparateVaryingOutDerived,   // Derived Varying Out Symbol Of Sparate Mode
	OpenGLSiSymbolNonseparateVaryingIn,        // Varying In Of Nonseparate Mode
	OpenGLSiSymbolNonseparateVaryingOut,       // Varying Out Of Nonseparate Mode
	OpenGLSiSymbolShaderStorageBuffer,         // Shader Storage Buffer
	OpenGLSiSymbolAttribInputvalid,            // Input Valide Bits For Sparse Texture
};


// Symbol Data Types
enum OpenGLSiBinSymbolDatatype
{
	OpenGLSiSymbolDatatypeVoid,                           // Void
	OpenGLSiSymbolDatatypeBool,                           // Bool
	OpenGLSiSymbolDatatypeInt,                            // Int
	OpenGLSiSymbolDatatypeUint,                           // Unsigned Int
	OpenGLSiSymbolDatatypeFloat,                          // Float
	OpenGLSiSymbolDatatypeDouble,                         // Double
	OpenGLSiSymbolDatatypeFloat16,                        // Float16
	OpenGLSiSymbolDatatypeDvec2,                          // Dvec2
	OpenGLSiSymbolDatatypeDvec3,                          // Dvec3
	OpenGLSiSymbolDatatypeDvec4,                          // Dvec4
	OpenGLSiSymbolDatatypeVec2,                           // Vec2
	OpenGLSiSymbolDatatypeVec3,                           // Vec3
	OpenGLSiSymbolDatatypeVec4,                           // Vec4
	OpenGLSiSymbolDatatypeBvec2,                          // Bvec2
	OpenGLSiSymbolDatatypeBvec3,                          // Bvec3
	OpenGLSiSymbolDatatypeBvec4,                          // Bvec4
	OpenGLSiSymbolDatatypeIvec2,                          // Ivec2
	OpenGLSiSymbolDatatypeIvec3,                          // Ivec3
	OpenGLSiSymbolDatatypeIvec4,                          // Ivec4
	OpenGLSiSymbolDatatypeUvec2,                          // Unsigned Ivec2
	OpenGLSiSymbolDatatypeUvec3,                          // Unsigned Ivec3
	OpenGLSiSymbolDatatypeUvec4,                          // Unsigned Ivec4
	OpenGLSiSymbolDatatypeF16vec2,                        // F16vec2
	OpenGLSiSymbolDatatypeF16vec3,                        // F16vec3
	OpenGLSiSymbolDatatypeF16vec4,                        // F16vec4
	OpenGLSiSymbolDatatypeMat2,                           // Mat2
	OpenGLSiSymbolDatatypeMat2x3,                         // Mat2x3
	OpenGLSiSymbolDatatypeMat2x4,                         // Mat2x4
	OpenGLSiSymbolDatatypeMat3x2,                         // Mat3x2
	OpenGLSiSymbolDatatypeMat3,                           // Mat3
	OpenGLSiSymbolDatatypeMat3x4,                         // Mat3x4
	OpenGLSiSymbolDatatypeMat4x2,                         // Mat4x2
	OpenGLSiSymbolDatatypeMat4x3,                         // Mat4x3
	OpenGLSiSymbolDatatypeMat4,                           // Mat4
	OpenGLSiSymbolDatatypeDmat2,                          // Dmat2
	OpenGLSiSymbolDatatypeDmat2x3,                        // Dmat2x3
	OpenGLSiSymbolDatatypeDmat2x4,                        // Dmat2x4
	OpenGLSiSymbolDatatypeDmat3x2,                        // Dmat3x2
	OpenGLSiSymbolDatatypeDmat3,                          // Dmat3
	OpenGLSiSymbolDatatypeDmat3x4,                        // Dmat3x4
	OpenGLSiSymbolDatatypeDmat4x2,                        // Dmat4x2
	OpenGLSiSymbolDatatypeDmat4x3,                        // Dmat4x3
	OpenGLSiSymbolDatatypeDmat4,                          // Dmat4
	OpenGLSiSymbolDatatypeF16mat2,                        // F16mat2
	OpenGLSiSymbolDatatypeF16mat2x3,                      // F16mat2x3
	OpenGLSiSymbolDatatypeF16mat2x4,                      // F16mat2x4
	OpenGLSiSymbolDatatypeF16mat3x2,                      // F16mat3x2
	OpenGLSiSymbolDatatypeF16mat3,                        // F16mat3
	OpenGLSiSymbolDatatypeF16mat3x4,                      // F16mat3x4
	OpenGLSiSymbolDatatypeF16mat4x2,                      // F16mat4x2
	OpenGLSiSymbolDatatypeF16mat4x3,                      // F16mat4x3
	OpenGLSiSymbolDatatypeF16mat4,                        // F16mat4

	OpenGLSiSymbolDatatypeSampler1d,                      // Sampler1d
	OpenGLSiSymbolDatatypeSampler2d,                      // Sampler2d
	OpenGLSiSymbolDatatypeSampler2dRect,                  // Sampler2drect
	OpenGLSiSymbolDatatypeSamplerExternal,                // Samplerexternaloes
	OpenGLSiSymbolDatatypeSampler3d,                      // Sampler3d
	OpenGLSiSymbolDatatypeSamplerCube,                    // Samplercube
	OpenGLSiSymbolDatatypeSampler1dArray,                 // Sampler1darray
	OpenGLSiSymbolDatatypeSampler2dArray,                 // Sampler2darray
	OpenGLSiSymbolDatatypeSamplerCubeArray,               // Samplercubearray
	OpenGLSiSymbolDatatypeSamplerBuffer,                  // Samplerbuffer
	OpenGLSiSymbolDatatypeSamplerRenderbuffer,            // Samplerrenderbuffer
	OpenGLSiSymbolDatatypeSampler2dMs,                    // Sampler2dms
	OpenGLSiSymbolDatatypeSampler2dMsArray,               // Sampler2dmsarray

	OpenGLSiSymbolDatatypeSampler1dShadow,                // Sampler1dshadow
	OpenGLSiSymbolDatatypeSampler2dShadow,                // Sampler2dshadow
	OpenGLSiSymbolDatatypeSampler2dRectShadow,            // Sampler2drectshadow
	OpenGLSiSymbolDatatypeSamplerCubeShadow,              // Samplercubeshadow
	OpenGLSiSymbolDatatypeSampler1dArrayShadow,           // Sampler1darrayshadow
	OpenGLSiSymbolDatatypeSampler2dArrayShadow,           // Sampler2darrayshadow
	OpenGLSiSymbolDatatypeSamplerCubeArrayShadow,         // Samplercubearrayshadow

	OpenGLSiSymbolDatatypeIntSampler1d,                   // Isampler1d
	OpenGLSiSymbolDatatypeIntSampler2d,                   // Isampler2d
	OpenGLSiSymbolDatatypeIntSampler2dRect,               // Isampler2drect
	OpenGLSiSymbolDatatypeIntSampler3d,                   // Isampler3d
	OpenGLSiSymbolDatatypeIntSamplerCube,                 // Isamplercube
	OpenGLSiSymbolDatatypeIntSampler1dArray,              // Isampler1darray
	OpenGLSiSymbolDatatypeIntSampler2dArray,              // Isampler2darray
	OpenGLSiSymbolDatatypeIntSamplerCubeArray,            // Isamplercubearray
	OpenGLSiSymbolDatatypeIntSamplerBuffer,               // Isamplerbuffer
	OpenGLSiSymbolDatatypeIntSamplerRenderbuffer,         // Isamplerrenderbuffer
	OpenGLSiSymbolDatatypeIntSampler2dMs,                 // Isampler2dms
	OpenGLSiSymbolDatatypeIntSampler2dMsArray,            // Isampler2dmsarray

	OpenGLSiSymbolDatatypeUnsignedIntSampler1d,           // Usampler1d
	OpenGLSiSymbolDatatypeUnsignedIntSampler2d,           // Usampler2d
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dRect,       // Usampler2drect
	OpenGLSiSymbolDatatypeUnsignedIntSampler3d,           // Usampler3d
	OpenGLSiSymbolDatatypeUnsignedIntSamplerCube,         // Usamplercube
	OpenGLSiSymbolDatatypeUnsignedIntSampler1dArray,      // Usampler1darray
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dArray,      // Usampler2darray
	OpenGLSiSymbolDatatypeUnsignedIntSamplerCubeArray,    // Usamplercubearray
	OpenGLSiSymbolDatatypeUnsignedIntSamplerBuffer,       // Usamplerbuffer
	OpenGLSiSymbolDatatypeUnsignedIntSamplerRenderbuffer, // Usamplerrenderbuffer
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dMs,         // Usampler2dms
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dMsArray,    // Usampler2dmsarray

	OpenGLSiSymbolDatatypeFloat16Sampler1d,               // F16sampler1d
	OpenGLSiSymbolDatatypeFloat16Sampler2d,               // F16sampler2d
	OpenGLSiSymbolDatatypeFloat16Sampler2dRect,           // F16sampler2drect
	OpenGLSiSymbolDatatypeFloat16Sampler3d,               // F16sampler3d
	OpenGLSiSymbolDatatypeFloat16SamplerCube,             // F16samplercube
	OpenGLSiSymbolDatatypeFloat16Sampler1dArray,          // F16sampler1darray
	OpenGLSiSymbolDatatypeFloat16Sampler2dArray,          // F16sampler2darray
	OpenGLSiSymbolDatatypeFloat16SamplerCubeArray,        // f16samplercubearray
	OpenGLSiSymbolDatatypeFloat16SamplerBuffer,           // F16samplerbuffer
	OpenGLSiSymbolDatatypeFloat16Sampler2dMs,             // F16sampler2dms
	OpenGLSiSymbolDatatypeFloat16Sampler2dMsArray,        // F16sampler2dmsarray
	OpenGLSiSymbolDatatypeFloat16Sampler1dShadow,         // F16sampler1dshadow
	OpenGLSiSymbolDatatypeFloat16Sampler2dShadow,         // F16sampler2dshadow
	OpenGLSiSymbolDatatypeFloat16Sampler2dRectShadow,     // F16sampler2drectshadow
	OpenGLSiSymbolDatatypeFloat16SamplerCubeShadow,       // F16samplercubeshadow
	OpenGLSiSymbolDatatypeFloat16Sampler1dArrayShadow,    // F16sampler1darrayshadow
	OpenGLSiSymbolDatatypeFloat16Sampler2dArrayShadow,    // F16sampler2darrayshadow
	OpenGLSiSymbolDatatypeFloat16SamplerCubeArrayShadow,  // F16samplercubearrayshadow

	OpenGLSiSymbolDatatypeImage1d,                        // Image1d
	OpenGLSiSymbolDatatypeImage2d,                        // Image2d
	OpenGLSiSymbolDatatypeImage3d,                        // Image3d
	OpenGLSiSymbolDatatypeImage2dRect,                    // Image2drect
	OpenGLSiSymbolDatatypeImageCubemap,                   // Imagecube
	OpenGLSiSymbolDatatypeImageBuffer,                    // Imagebuffer
	OpenGLSiSymbolDatatypeImage1dArray,                   // Image1darray
	OpenGLSiSymbolDatatypeImage2dArray,                   // Image2darray
	OpenGLSiSymbolDatatypeImageCubemapArray,              // Imagecubearray
	OpenGLSiSymbolDatatypeImage2dMs,                      // Image2dmultisample
	OpenGLSiSymbolDatatypeImage2dMsArray,                 // Image2dmultisamplearray

	OpenGLSiSymbolDatatypeIntImage1d,                     // Iimage1d
	OpenGLSiSymbolDatatypeIntImage2d,                     // Iimage2d
	OpenGLSiSymbolDatatypeIntImage3d,                     // Iimage3d
	OpenGLSiSymbolDatatypeIntImage2dRect,                 // Iimage2drect
	OpenGLSiSymbolDatatypeIntImageCubemap,                // Iimagecube
	OpenGLSiSymbolDatatypeIntImageBuffer,                 // Iimagebuffer
	OpenGLSiSymbolDatatypeIntImage1dArray,                // Iimage1darray
	OpenGLSiSymbolDatatypeIntImage2dArray,                // Iimage2darray
	OpenGLSiSymbolDatatypeIntImageCubemapArray,           // Iimagecubearray
	OpenGLSiSymbolDatatypeIntImage2dMs,                   // Iimage2dmultisample
	OpenGLSiSymbolDatatypeIntImage2dMsArray,              // Iimage2dmultisamplearray

	OpenGLSiSymbolDatatypeUnsignedIntImage1d,             // Uimage1d
	OpenGLSiSymbolDatatypeUnsignedIntImage2d,             // Uimage2d
	OpenGLSiSymbolDatatypeUnsignedIntImage3d,             // Uimage3d
	OpenGLSiSymbolDatatypeUnsignedIntImage2dRect,         // Uimage2drect
	OpenGLSiSymbolDatatypeUnsignedIntImageCubemap,        // Uimagecube
	OpenGLSiSymbolDatatypeUnsignedIntImageBuffer,         // Uimagebuffer
	OpenGLSiSymbolDatatypeUnsignedIntImage1dArray,        // Uimage1darray
	OpenGLSiSymbolDatatypeUnsignedIntImage2dArray,        // Uimage2darray
	OpenGLSiSymbolDatatypeUnsignedIntImageCubemapArray,   // Uimagecubearray
	OpenGLSiSymbolDatatypeUnsignedIntImage2dMs,           // Uimage2dmultisample
	OpenGLSiSymbolDatatypeUnsignedIntImage2dMsArray,      // Uimage2dmultisamplearray

	OpenGLSiSymbolDatatypeFloat16Image1d,                 // F16image1d
	OpenGLSiSymbolDatatypeFloat16Image2d,                 // F16image2d
	OpenGLSiSymbolDatatypeFloat16Image3d,                 // F16image3d
	OpenGLSiSymbolDatatypeFloat16Image2dRect,             // F16image2drect
	OpenGLSiSymbolDatatypeFloat16ImageCubemap,            // F16imagecube
	OpenGLSiSymbolDatatypeFloat16ImageBuffer,             // F16imagebuffer
	OpenGLSiSymbolDatatypeFloat16Image1dArray,            // F16image1darray
	OpenGLSiSymbolDatatypeFloat16Image2dArray,            // F16image2darray
	OpenGLSiSymbolDatatypeFloat16ImageCubemapArray,       // F16imagecubearray
	OpenGLSiSymbolDatatypeFloat16Image2dMs,               // F16image2dmultisample
	OpenGLSiSymbolDatatypeFloat16Image2dMsArray,          // F16image2dmultisamplearray

	OpenGLSiSymbolDatatypeSamplerBufferAmd,               // Used For Tessellation (from Vbo)
	OpenGLSiSymbolDatatypeIntSamplerBufferAmd,            // Used For Vertex Shader Tesssellation
	OpenGLSiSymbolDatatypeUnsignedIntSamplerBufferAmd,    // Used For Vertex Shader Tesssellation

	OpenGLSiSymbolDatatypeAtomicCounter,                  // Atomic Counter

	OpenGLSiSymbolDatatypeStruct,                         // Struct, Used For Bindable Uniform, It Is Also Used For Type Check In Separate Program Mode
	OpenGLSiSymbolDatatypeInterface,                      // Interface Block, Used For Type Check In Separate Progarm Mode
	OpenGLSiSymbolDatatypeFirst = OpenGLSiSymbolDatatypeVoid,
	OpenGLSiSymbolDatatypeLast  = OpenGLSiSymbolDatatypeInterface
};

enum OpenGLSiFetchShaderType
{
	OpenGLSiFetchFixedFunction, // Fetch Shader Executes All Vertex Fetches And Start Address Is In A Fixed Function Register.
	OpenGLSiFetchSubImmediate,  // Si+: Fetch Shader Performs All Fetches, Vertex Buffer Refs Are Stored As Immediate User Data
	OpenGLSiFetchSubFlatTable,  // Si+: Fetch Shader Performs All Fetches, Vertex Buffer Refs Are Stored In A Flat Table
	OpenGLSiFetchSubPerAttrib   // Si+: Fetch Shader Performs A Single Attribute Fetch Per Execution, Vertex Buffer Refs Stored In A Flat Table
};

//
/// Swizzle types
//
enum SIElfSwizzle 
{
	OpenGLSiElfSwizzleX,
	OpenGLSiElfSwizzleY,
	OpenGLSiElfSwizzleZ,
	OpenGLSiElfSwizzleW,
	OpenGLSiElfSwizzleUnknown
};


//
/// Symbol data precision qualifier
//
enum SIElfSymbolDataPrecisionQualifier 
{
	OpenGLSiElfSymbolDatapqLow,
	OpenGLSiElfSymbolDatapqMedium,
	OpenGLSiElfSymbolDatapqHigh,
	OpenGLSiElfSymbolDatapqDouble
};

//
/// Style for layout qualifier
//
enum SIElfLayoutStyleQualifier
{
	OpenGLSiElfLayoutStyleNone,
	OpenGLSiElfLayoutShared,
	OpenGLSiElfLayoutPacked,
	OpenGLSiElfLayoutStd140,
};

//
/// Major for layout qualifier
//
enum SIElfLayoutMajorQualifier
{
	OpenGLSiElfLayoutMajorNone,
	OpenGLSiElfLayoutRowMajor,
	OpenGLSiElfLayoutColumnMajor,
};

//
/// invariant qualifier
///
enum SIElfInvariantQualifier
{
	OpenGLSiElfInvariantOff,
	OpenGLSiElfInvariantOn,
};

//
/// precise qualifier
//
enum SIElfPreciseQualifier
{
	OpenGLSiElfPreciseOff,
	OpenGLSiElfPreciseOn,
};

//
/// storage qualifier
///
enum SIElfStorageQualifier
{
	OpenGLSiElfStorageDefault,
	OpenGLSiElfStorageCentroid,
	OpenGLSiElfStorageSample,
};

//
/// interpolation qualifier
///
enum SIElfInterpolationQualifier
{
	OpenGLSiElfInterpolationSmooth,
	OpenGLSiElfInterpolationFlat,
	OpenGLSiElfInterpolationNoperspective,
};

//
/// patch qualifier
//
enum SIElfPatchQualifier
{
	OpenGLSiElfPerPatchOff,
	OpenGLSiElfPerPatchOn,
};

// ---------------------------------Structures----------------------------------
struct OpenGLSiBinShaderStats
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

struct OpenGLSiBinShaderDep
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

struct OpenGLSiBinConstantUsage
{
	uint32_t maskBits[(MAX_CONSTANTS + 31) / 32];
}__attribute__((packed));

#define OPENGL_SI_BIN_SHADER_COMMON                                                \
	uint32_t uSizeInBytes;                     /* size of structure */             \
	OpenGLSiBinShaderStats stats;              /* hw-neutral stats */              \
	OpenGLSiBinShaderDep dep;                  /* hw-neutral dependency */         \
	uint32_t X32XpPvtData;                     /* SC-private data */               \
	uint32_t u32PvtDataSizeInBytes;            /* size of private data */          \
	OpenGLSiBinConstantUsage fConstantUsage;   /* float const usage */             \
	OpenGLSiBinConstantUsage bConstantUsage;   /* int const usage */               \
	OpenGLSiBinConstantUsage iConstantUsage;                                       \
	uint32_t uShaderType;                      /* IL shader type */                \
	uint32_t eInstSet;                         /* Instruction set */               \
	uint32_t texResourceUsage[(MAX_NUM_RESOURCE + 31) / 32];                       \
	uint32_t fetch4ResourceUsage[(MAX_NUM_RESOURCE + 31) / 32];                    \
	uint32_t uavResourceUsage[(MAX_NUM_UAV + 31) / 32];                            \
	uint32_t texSamplerUsage;                                                      \
	uint32_t constBufUsage;                                                        \
	uint32_t texSamplerResourceMapping[MAX_NUM_SAMPLER][(MAX_NUM_RESOURCE+31)/32]; \
	uint32_t NumConstOpportunities;                                                \
	uint32_t ResourcesAffectAlphaOutput[(MAX_NUM_RESOURCE+31)/32];                 \

#define OPENGL_SI_BIN_SHADER_SI_COMMON                                                                               \
	uint32_t X32XhShaderMemHandle;                                                                                   \
	uint32_t X32XhConstBufferMemHandle[SC_SI_NUM_INTBUF];                                                            \
	uint32_t CodeLenInByte;                                                                                          \
	uint32_t u32UserElementCount; /* Number of user data descriptors  */                                             \
	struct   SIBinaryUserElement pUserElement[SC_SI_NUM_USER_ELEMENT]; /* User data descriptors */                   \
	/* Common HW shader info about registers and execution modes*/                                                   \
	uint32_t u32NumVgprs;                                                                                            \
	uint32_t u32NumSgprs;                                                                                            \
	uint32_t u32FloatMode;                                                                                           \
	bool     bIeeeMode;                                                                                              \
	bool     bUsesPrimId;                                                                                            \
	bool     bUsesVertexId;                                                                                          \
	uint32_t scratchSize; /* Scratch size in DWORDs for a single thread*/                                            \

#define OPENGL_SI_BIN_SHADER_SI_BASE \
	OPENGL_SI_BIN_SHADER_COMMON \
	OPENGL_SI_BIN_SHADER_SI_COMMON \

/* SPI_SHADER_PGM_RSRC2_VS */
struct OpenGLSiBinSpiShaderPgmRsrc2Vs
{
	unsigned scratch_en              : 1;
	unsigned user_sgpr               : 5;
	unsigned trap_present            : 1;
	unsigned offchip_lds_pointer_en  : 1;
	unsigned streamout_offset0       : 1;
	unsigned streamout_offset1       : 1;
	unsigned streamout_offset2       : 1;
	unsigned streamout_offset3       : 1;
	unsigned streamout_en            : 1;
	unsigned reserved                : 19;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_PS */
struct OpenGLSiBinSpiShaderPgmRsrc2Ps
{
	unsigned scratch_en              : 1;
	unsigned user_sgpr               : 5;
	unsigned trap_present            : 1;
	unsigned unknown_en              : 1;
	unsigned reserved                : 24;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_GS */
struct OpenGLSiBinSpiShaderPgmRsrc2Gs
{
	unsigned scratch_en              : 1;
	unsigned user_sgpr               : 5;
	unsigned trap_present            : 1;
	unsigned reserved                : 25;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_ES */
struct OpenGLSiBinSpiShaderPgmRsrc2Es
{
	unsigned scratch_en              : 1;
	unsigned user_sgpr               : 5;
	unsigned trap_present            : 1;
	unsigned offchip_lds_pointer_en  : 1;
	unsigned reserved                : 24;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_HS */
struct OpenGLSiBinSpiShaderPgmRsrc2Hs
{
	unsigned scratch_en              : 1;
	unsigned user_sgpr               : 5;
	unsigned trap_present            : 1;
	unsigned offchip_lds_pointer_en  : 1;
	unsigned unknown_en              : 1;
	unsigned reserved                : 23;
}__attribute__((packed));

/* SPI_SHADER_PGM_RSRC2_LS */
struct OpenGLSiBinSpiShaderPgmRsrc2Ls
{
	unsigned scratch_en              : 1;
	unsigned user_sgpr               : 5;
	unsigned trap_present            : 1;
	unsigned reserved                : 25;
}__attribute__((packed));

/* PA_CL_VS_OUT_CNTL */
struct OpenGLSiBinPaClVsOutCntl
{
	unsigned int unknown1            : 8;
	unsigned int unknown2            : 8;
	unsigned int unknown3            : 8;
	unsigned int unknown4            : 8;
}__attribute__((packed));

/* SPI_VS_OUT_CONFIG */
struct OpenGLSiSpiVsOutConfig
{
	unsigned int vs_export_count     : 8;
	unsigned int vs_half_pack        : 8;
	unsigned int vs_export_fog       : 8;
	unsigned int vs_out_fog_vec_addr : 8;
}__attribute__((packed));

/* SPI_SHADER_POS_FORMAT */
struct OpenGLSiSpiShaderPosFormat
{
	unsigned int pos0_export_format  : 8;
	unsigned int pos1_export_format  : 8;
	unsigned int pos2_export_format  : 8;
	unsigned int pos3_export_format  : 8;
}__attribute__((packed));

//
//  Vertex shader input declaration to be used for semantic mapping with FS.
//
struct OpenGLSiBinVsSemanticMappingIn
{
	unsigned int usage               : 8;  // semantic usage. IL_IMPORTUSAGE.
	unsigned int usageIdx            : 8;  // semantic index. Opaque to SC.
	unsigned int dataVgpr            : 8;  // first VGPR to contain fetch result
	unsigned int dataSize            : 2;  // (fetch_size - 1), size in elements
	unsigned int reserved            : 6;
}__attribute__((packed));

//
//  Vertex shader output declaration to be used for semantic mapping.
//  The paramIdx is the index of the export parameter SC uses in the shader.
//
struct OpenGLSiBinVsSemanticMappingOut
{
	uint32_t usage                   : 8;  // semantic usage. IL_IMPORTUSAGE.
	uint32_t usageIdx                : 8;  // semantic index. Opaque to SC.
	uint32_t paramIdx                : 8;  // attribute export parameter index (0-31)
	uint32_t reserved                : 8;
}__attribute__((packed));

//
//  Pixel shader input declaration to be used for semantic mapping.
//
struct OpenGLSiBinPsSemanticMappingIn
{
	uint32_t usage                   : 8;  // semantic usage. IL_IMPORTUSAGE.
	uint32_t usageIdx                : 8;  // semantic index. Opaque to SC.
	uint32_t inputIdx                : 8;  // PS input index.
	uint32_t defaultVal              : 2;  // default val if VS doesn't export.
	uint32_t flatShade               : 1;  // set if interp mode is constant.
	uint32_t reserved                : 5;
}__attribute__((packed));

//
//  Pixel shader export patch .
//
struct OpenGLSiBinFsExportPatchInto
{
	// Patch offset (in DWORDs) to start of an export sequence
	uint32_t patchOffset;
	// Conversion code snippet for each of the export formats
	uint32_t patchCode[SC_SI_NUM_EXPORT_FMT][SC_SI_MAX_EXPORT_CODE_SIZE];
}__attribute__((packed));


//
//  Flags to guide shader compilation.
//
struct OpenGLSiBinCompileGuide
{
	// To make this structure consistent with SC_COMPILE_FLAGS, we add some reserved bits.
	uint32_t psPrimId                : 1;  // PS needs primID input
	uint32_t useHsOffChip            : 1;  // HS can use off-ship LDS
	uint32_t clampScratchAccess      : 1;  // add code to clamp scratch assesses
	uint32_t streamOutEnable         : 1;  // enable writes to stream-out buffers
	uint32_t reserved1               : 4;
	uint32_t useGsOnChip             : 1;  // ES and GS can use on-chip LDS. (CI+)
	uint32_t reserved2               : 23;
}__attribute__((packed));

// ---------------------------------.text section -----------------------------------
// Vertex shader metadata stored in .text section
class OpenGLSiBinVertexShaderMetadata
{
	// SI Shader base structure, same for all shaders
	OPENGL_SI_BIN_SHADER_SI_BASE

	// Shader specific structures
	// Input semantics
	uint32_t numVsInSemantics;
	OpenGLSiBinVsSemanticMappingIn vsInSemantics[SC_SI_VS_MAX_INPUTS];

	// Output semantics
	uint32_t numVsOutSemantics;
	OpenGLSiBinVsSemanticMappingOut vsOutSemantics[SC_SI_VS_MAX_OUTPUTS];

	// LS/ES/VS specific shader resources
	union 
	{
		uint32_t spiShaderPgmRsrc2Ls;
		uint32_t spiShaderPgmRsrc2Es;
		OpenGLSiBinSpiShaderPgmRsrc2Vs spiShaderPgmRsrc2Vs;
	} spiShaderPgmRsrcUnion;

	// SC-provided values for certain VS-specific registers
	uint32_t paClVsOutCntl;
	union
	{
		OpenGLSiSpiVsOutConfig spiVsOutConfig;
		uint32_t spiVsOutConfigAsUint;
	} OpenGLSiSpiVsOutConfigUnion;
	union
	{
		OpenGLSiSpiShaderPosFormat spiShaderPosFormat;
		uint32_t spiShaderPosFormatAsUint;
	} OpenGLSiSpiShaderPosFormatUnion;

	// FIXME: should be a struct
	uint32_t vgtStrmoutConfig;

	// Number of SPI-generated VGPRs referenced by the vertex shader
	uint32_t vgprCompCnt;

	// Exported vertex size in DWORDs, can be used to program ESGS ring size
	uint32_t exportVertexSize;

	bool useEdgeFlags;  
	bool  remapClipDistance; //true if clip distance[4-7] is remapped to [0-3] 
	uint32_t hwShaderStage;    // hardware stage which this shader actually in when execution
	OpenGLSiBinCompileGuide compileFlags;     // compile flag
	uint32_t gsMode;                            // gs mode

	// FIXME: size doesn't match
	// bool isOnChipGs; 
	// uint32_t targetLdsSize;
public:
	explicit OpenGLSiBinVertexShaderMetadata(const char *buffer);
};

// Fragment shader metadata stored in .text section
class OpenGLSiBinPixelShaderMetadata
{
	// SI Shader base structure, same for all shaders
	OPENGL_SI_BIN_SHADER_SI_BASE

	// Input semantics
	uint32_t numPsInSemantics;
	OpenGLSiBinPsSemanticMappingIn psInSemantics[SC_SI_PS_MAX_INPUTS];

	// Mapping of a PS interpolant to a texture coordinate index (0xffffffff if that interpolant is not a texture coordinate)
	uint32_t psInTexCoordIndex[SC_SI_PS_MAX_INPUTS];

	// PS specific shader resources
	union
	{
		uint32_t spiShaderPgmRsrc2PsAsUint;
		OpenGLSiBinSpiShaderPgmRsrc2Ps spiShaderPgmRsrc2Ps;		
	} spiShaderPgmRsrc2PsUnion;

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
	OpenGLSiBinFsExportPatchInto exportPatchInfo[SC_SI_PS_MAX_OUTPUTS];
	uint32_t defaultExportFmt;
public:
	explicit OpenGLSiBinPixelShaderMetadata(const char *buffer);
};

// texture resource and sampler binding
struct OpenGLSiTextureResourceBound
{
	uint32_t resourceId;         // resource id
	uint32_t samplerMask;        // samplers bind to resource id
};


// ----------------------.info section----------------------------------------

// Info descriptor for .info section
enum OpenGLSiBinInfoMaxOffset
{ 
	MAX_PROGRAMINFO_OFFSET = 0x0ffff 
};

struct SIVSInfo
{
	                                                                 // VS input mask
	uint32_t inputStreamMask;                                        // input stream mask (phsyical id)
	uint16_t   usesVertexID;                                         // tells whether this program uses VertexID
	                                                                 // transform feedback
	uint32_t streamOutStrideInDWORDs0;                               // streamout stride0
	uint32_t streamOutStrideInDWORDs1;                               // streamout stride1
	uint32_t streamOutStrideInDWORDs2;                               // streamout stride2
	uint32_t streamOutStrideInDWORDs3;                               // streamout stride3
	int8_t   streamOutBufferMapping[MAX_TRANSFORM_FEEDBACK_BUFFERS]; // streamout buffer config
	                                                                 // vertex shader tessellation
	uint8_t  tessPrimType;                                           // tessellation shader primitive type (sclTessPrimType)
	                                                                 // viewport array index
	unsigned outputViewportArrayIndex :8;                            // true if output viewport array index
	                                                                 // svp members
	uint8_t  frontColorOutputReg;                                    // front color output register number
	uint8_t  frontSecondaryColorOutputReg;                           // front secondary color output register number
	uint8_t  backColorOutputReg;                                     // back color output register number
	uint8_t  backSecondaryColorOutputReg;                            // back secondary color output register number
	uint8_t  aaStippleTexCoord;                                      // Bitfield representing which texture cood will be used for aastipple patch
	OpenGLSiFetchShaderType fsTypeForPassThruVS :8;                  // Fetch shader type (SVP PassThruVS)
	uint8_t  fsReturnAddrRegForPassThruVS;                           // Fetch shader subroutine start SGPR (SVP PassThruVS)
	uint8_t  fsInputStreamTableRegForPassThruVS;                     // Fetch shader input stream table start SGPR (SVP PassThruVS)
	int32_t  fsAttribValidMaskReg;                                   // VPGR which Fetch shader should populate, if sparse buffers are used.
} __attribute__((packed));

struct SIPSInfo
{
	uint32_t texKillPresent;                                         // Program uses texkill
	int32_t  pointCoordReg;                                          // register number of gl_PointCoord which is an input of FS
	uint8_t  outputColorMap[PS_MAX_OUTPUTS];                         // pixel shader color output map (from virtual to physical)
	uint16_t useFlatInterpMode;                                      // if flat has been used on a varying
	uint16_t forcePerSampleShading;                                  // true if the FS is required to run in per sample frequency
	uint16_t uncached_writes;                                        // uncached writes
	uint16_t outputDepth;                                            // true if pixel shader output depth
	uint32_t usingDualBlendMask;                                     // indicates using an index = 1 for dual blending, in glsl layout
} __attribute__((packed));

struct SICSInfo
{
	uint32_t numSharedGPRUser;                                       // shared GPR
	uint32_t numSharedGPRTotal;                                      // shared GPR total including ones used by SC.

	uint32_t numThreadPerGroup;                                      // threads per group
	uint32_t numThreadPerGroupX;                                     // dimension x of NumThreadPerGroup
	uint32_t numThreadPerGroupY;                                     // dimension y of NumThreadPerGroup
	uint32_t numThreadPerGroupZ;                                     // dimension z of NumThreadPerGroup
	uint32_t totalNumThreadGroup;                                    // total number of thread groups
	uint32_t NumWavefrontPerSIMD;                                    // wavefronts per simd
	uint16_t eCsSetupMode;                                           // compute slow/fast mode
	uint16_t IsMaxNumWavePerSIMD;                                    // Is this the max active num wavefronts per simd
	uint16_t SetBufferForNumGroup;                                   // Need to set up buffer for info on number of thread groups?
} __attribute__((packed));

struct SIFSInfo
{
	uint16_t   usesVertexCache;                                      // vertex cache used? (fetch shader only)
} __attribute__((packed));


class OpenGLSiBinInfo
{
	// generaic shader resource information
	// texture
	uint32_t fetch4Mask;                                    // Fetch4 mask
	uint32_t textureMask;                                   // Texture unit masks
	uint32_t textureSamplerMask;                            // texture sampler mask
	uint32_t textureResourceMask[MAX_TEXTURE_RESOURCES/32]; // texture resource mask
	uint32_t bufferMask[MAX_TEXTURE_RESOURCES/32];          // texture buffer mask
	uint32_t textureResourceBoundCount;                     // the size of texture resource bound array
	// scratch
	uint32_t maxScratchRegsNeeded;                          // scratch registers needed
	// constant buffer
	uint32_t constantBufferMask;                            // constant buffer mask
	// uav
	uint32_t uavOpIsUsed;                                   // mask for uav used delete!!
	uint16_t uavInCB;                                       // UAV is in CB
	uint32_t uavResourceCount;                              // size of uav resource mask array
	// atomic counter
	uint32_t uavAtomicOpIsUsed;                             // mask for atomic op used
	// subroutine
	int16_t  maxUniformLocation;                            // max explicit uniform location assigned by application
	int16_t  maxSubroutineIndex;                            // max explicit Subroutine index assigned by application
	int16_t  maxSubroutineUniformLocation;                  // max explicit Subroutine uniform location assigned by application
	// / union for per shader stage parameters
	union
	{
		/// Vexter Shader, Tessellation Evaluation Shader and Geometry Shader parameter
		SIVSInfo vs;
	    /// Fragment Shader Parameters
		SIPSInfo ps;
	    /// Compute Shader Parameters
		SICSInfo cs;
	    /// Fetch Shader Parameters
		SIFSInfo fs;
	} shaderinfo;
	// dynamic array, offset fields is valid in ELF package, int64_t is to keep the struct size fixed in all operation system.
	// texture resource bound array
	union
	{
		OpenGLSiTextureResourceBound *_textureResourceBound;    // texture resoruce and sampler bounding
		int64_t textureResourceBoundOffset;                     // resource binding array offset
	} textureResourceBound;
	/// uav resource mask array
	union
	{
		uint32_t *uavResourceMask;                              // UAV resource mask
		int64_t   uavResourceMaskOffset;                        // UAV resource mask array offset
	} uavResourceMaskUnion;
	/// uav return buffer
	union
	{
		uint32_t *_uavRtnBufStride;                             // save stride of uav return buffer for each UAV
		int64_t   uavRtnBufStrideOffset;                        // uav return buffer stride array offset
	} uavRtnBufStrideUnion;
	// / uav dynamic resource map
	union
	{
		uint32_t *_uavDynamicResouceMap;                        // save fetch const offset of each UAV
		int64_t   uavDynamicResouceMapOffset;                   // uav dynamic resource map offset
	} uavDynamicResouceMapUnion;
	
	OpenGLSiBinInfoMaxOffset max_valid_offset;

public:
	OpenGLSiBinInfo(const char *buffer);
}__attribute__((packed));

//--------------------------.usageinfo section--------------------------------
// ARB program parameter
struct OpenGLSiBinArbProgramParameter
{
	uint32_t instructions;              // Number of instructions
	uint32_t nativeInstructions;        // Number of native instructions
	uint32_t aluInstructions;           // Number of alu instructions
	uint32_t nativeAluInstructions;     // Number of native alu instructions
	uint32_t texInstructions;           // Number of tex instructions
	uint32_t nativeTexInstructions;     // Number of native tex instructions
	uint32_t texIndirections;           // Number of tex indirections
	uint32_t nativeTexIndirections;     // Number of native tex indirections
	uint32_t temporaries;               // Number of temporaries
	uint32_t nativeTemporaries;         // Number of native temporaries
	uint32_t parameters;                // Number of parameters
	uint32_t nativeParameters;          // Number of native parameters
	uint32_t attribs;                   // Number of attribs
	uint32_t nativeAttribs;             // Number of native attribs
	uint32_t addressRegisters;          // Number of address registers
	uint32_t nativeAddressRegisters;    // Number of native address registers
	bool   underNativeLimits;           // Is shader under native limits?
}__attribute__((packed));

// Structure in .usageinfo section
struct SIPSUsageinfo
{
	// fs input usage
	uint32_t usesTexCoordMask;              // texcoord unit usage mask
	uint16_t   usesFogCoord;                // fogcoord usage
	uint16_t   usePrimaryColor;             // true if primary color is used
	uint16_t   useSecondaryColor;           // true if secondary color is used
	// aa stipple
	int8_t   aaStippleTexCoord;             // the texture coord used by SCL to simulate aa/stipple
	int8_t   aaStippleTexUnit;              // the texture unit used by aa/stipple texture image
	int8_t   aaStippleTexVirtUnit;          // default logic unit for aa stipple texture unit
	// bitmap
	int8_t   bitmapTexCoord;                // the texture coord used for bitmap drawing
	int8_t   bitmapTexUnit;                 // the texture unit used for bitmap drawing
	int8_t   bitmapTexVirtUnit;             // default logic unit for bitmap drawing
	// misc fields
	uint16_t   needSampleInfoCBs;           // whether the FP needs the 2 constant buffers for the SAMPLEINFO, SAMPLEPOS and EVAL_SAMPLE_INDEX.
	uint16_t   earlyFragTest;               // true if early frag test is enabled
	uint8_t  conservativeZ;                 // 0:defult, 1:less, 2:greater
} __attribute__((packed));

struct SIGSUsageinfo
{
	uint16_t maxGSVertices[4];              // max gs output vertices
	uint16_t gsInvocations;                 // gs invocation number
	uint8_t  inputTopology;                 // gs input topology
	uint8_t  outputTopology[4];             // gs output topology
} __attribute__((packed));

struct SIHSUsageinfo
{
	uint32_t tessOutputVertices;            // output control point number
} __attribute__((packed));

struct SIDSUsageinfo
{
	uint32_t tessGenMode;                   // domain mode
	uint32_t tessGenSpacing;                // partition mode
	uint32_t tessGenVertexOrder;            // output primitive mode
	uint16_t   tessGenPointMode;            // If point mode when tessellated
} __attribute__((packed));

struct SIVSUsageinfo
{
	uint16_t   positionInvariant;           // Has the ARB_position_invariant option been requested
	uint16_t   enableAnotherSetAttribAlias; // if it is cg generated program or nv_vertex_program
	uint32_t lsStride;                      // The number of input-control-points per patch declared in HS
	// SI+ fetch shader parameters
	int8_t   fsType;                        // Fetch shader type (immediate vs flat), SI only
	int8_t   fsReturnAddrReg;               // Fetch shader subroutine return address SGPR, SI only
	int8_t   fsInputStreamTableReg;         // Fetch shader input stream table start SGPR (either first data element or pointer depending on FS type), SI only
	int8_t   fsVertexIdChannel;             // Fetch shader channel(R,G,B,A) to compute the vertexID with
} __attribute__((packed));

struct SICSUsageinfo
{
	uint32_t  workDimension;                // Work dimension (1, 2 or 3 right now)
	uint32_t  workSizeX;                    // Work size in the X dimension
	uint32_t  workSizeY;                    // Work size in the Y dimension
	uint32_t  workSizeZ;                    // Work size in the Z dimension
} __attribute__((packed));

// FIXME: usageinfo section is 156 bytes for PS but 152 for VS in binary
class OpenGLSiBinUsageinfo
{
	OpenGLSiBinArbProgramParameter arbProgramParameter; // ARB program parameters
	uint16_t layoutParamMask;                           // layout parameters mask, see type gllShaderLayoutType for detail info
	uint16_t usesPrimitiveID;                           // Does this program use PrimitiveID
	uint16_t usesClipDistance;                          // vp outputs clip distance(s)

	uint32_t texImageMask;                              // orignial teximage unit usage mask (without SC optimization and it isn't include patched texture stage)
	uint32_t usesTexImageMask;                          // teximge unit usage returned by SC
	uint32_t usesShadowMask;                            // shadow texture unit usage mask
	uint32_t uavImageMask;                              // original uav image usage mask
	uint32_t uavImageMaskShaderStorageBuffer;           // uav used by shader storage buffer
	uint32_t usesBufMask;                               // vertex tessellation buffer mask

	uint32_t imageTypeAndFormatSize;                    // size of array imageType and imageFormat
	uint32_t textureTypeSize;                           // size of array textureType
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
	} shaderusageinfo;

	OpenGLSiBinInfoMaxOffset maxOffset;      // max valid value for dynamic array offset
	//
	/// texture type array
	//
	union
	{
		uint32_t* textureType;                         // teximage unit usage
		int64_t   textureTypeOffset;                   // texture type array offset in elf section
	} textureTypeUnion;
	//
	/// uav image type array
	//
	union
	{
		uint32_t* imageType;                           // UAV image usage
		int64_t   imageTypeOffset;                     // uav image type array offset in elf section
	} imageTypeUnion;
	//
	/// uav image format array
	//
	union
	{
		uint32_t* imageFormat;                         // uav image format array
		int64_t   imageFormatOffset;                   // uav image format array offset in elf section
	} imageFormatUnion;
public:
	OpenGLSiBinUsageinfo(const char *buffer);

}__attribute__((packed));

//----------------------------.symbol section---------------------------------
// structure for .symbol section
class OpenGLSiBinSymbol
{
	OpenGLSiBinSymbolType     type;                          // Uniform, normal, texcoord, generic attrib, etc.
	OpenGLSiBinSymbolDatatype dataType;                      // float, vec2, mat4, etc.
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
			SIElfSymbolDataPrecisionQualifier dataPrecQual            : 4; // low, medium, high, double
			SIElfLayoutStyleQualifier         layoutStyleQualifier    : 4; // Style of layout qualifier
			SIElfLayoutMajorQualifier         layoutMajorQualifier    : 4; // Major of layout qualifier
			SIElfInvariantQualifier           invariantQualifier      : 2; // invariant qualifier
			SIElfPreciseQualifier             preciseQualifier        : 2; // precise qualifier
			SIElfStorageQualifier             storageQualifier        : 3; // storage qualifier
			SIElfInterpolationQualifier       interpQualifier         : 3; // interpolation qualifier
			SIElfPatchQualifier               patchQualifier          : 2; // patch qualifier
		};
		uint32_t  qualifier;	                                           // qualifier
	} qualifierUnion;

	int                  vvalue;              // The value of this symbol for the vertex shader, intended to be virtual constant offset
	int                  vcbindex;            // The index of the bindable uniform of this symbol for vertex shader
	SIElfSwizzle         vswizzle;            // Destination swizzle for vertex constants
	int                  gvalue;              // The value of this symbol for the geometry shader, intended to be virtual constant offset
	int                  gcbindex;            // The index of the bindable uniform of this symbol for geometry shader
	SIElfSwizzle         gswizzle;            // Destination swizzle for geometry constants
	int                  fvalue;              // The value of this symbol for the fragment shader
	int                  fcbindex;            // The index of the bindable uniform of this symbol for fragment shader
	SIElfSwizzle         fswizzle;            // Destination swizzle for fragment constants
	int                  hvalue;              // The value of this symbol for the hull shader
	int                  hcbindex;            // The index of the bindable uniform of this symbol for hull shader
	SIElfSwizzle         hswizzle;            // Destination swizzle for hull constants
	int                  dvalue;              // The value of this symbol for the domain shader
	int                  dcbindex;            // The index of the bindable uniform of this symbol for domain shader
	SIElfSwizzle         dswizzle;            // Destination swizzle for domain constants
	int                  cvalue;              // The value of this symbol for the compute shader
	int                  ccbindex;            // The index of the bindable uniform of this symbol for compute shader
	SIElfSwizzle         cswizzle;            // Destination swizzle for compute constants
	int                  size;                // register_count * 4 * sizeof(float), no packing, with padding. This is for bindable uniform (GetUniformBufferSizeEXT)
	int                  count;               // if this symbol is part of an array, the number of remaining elements (including this one), 1 otherwise
	                                          // If anything is put between count and name, code in Uniform/VertexInterface ScanSymbols needs to be updated
	bool                 isArray;             // TRUE if the symbol is arrayed, FALSE otherwise
	uint32_t             matrixStride;        // stride of columns of column major matrix or rows of row major matrix
	uint32_t             subSize;             // the number of subroutine functions, all dynamic array and string must be added at the end of the sturct.
	uint32_t             uniformBinding;      // Binding (versatile usages for uniform block, sampler and image, atomic counter, array buffer)
	int16_t              layoutLocation;      // layout location (versatile usages for uniform variables, including opaque types, and subroutine uniforms)
	int16_t              layoutIndex;         // layout Index for subroutine function
	uint32_t             uniformOffset;       // Offset (currently used for atomic counter)
	uint32_t             resourceIndex;       // Resource index (currently for storing GDS offset for atomic counter in DWs)

	uint32_t             subroutineTypeID;    // subroutine type id
	uint32_t             topLevelArraySize;   // TOP_LEVEL_ARRAY_SIZE
	uint32_t             topLevelArrayStride; // TOP_LEVEL_ARRAY_STRIDE
	char*                name;                // NULL-terminated string which is the name of this symbol
	char*                baseName;            // If this symbol is part of an array, NULL-terminated string which is the unsubscripted name of the array
	char*                uniformBlockName;    // Name of uniform block
	char*                mangledName;         // Available if this sysmbol is struct or interface block, it is used to check type.
public:
	OpenGLSiBinSymbol(const char *buffer);
	~OpenGLSiBinSymbol();

	/// Getters
	///
	/// Get pointer to next symbol
	const char *getPtrNextSymbol(const char *buffer) {
		OpenGLSiBinSymbol *symbol_ptr = (OpenGLSiBinSymbol *)buffer;
		return (const char *)(&symbol_ptr->mangledName + 
			strlen(symbol_ptr->mangledName) + 1);
	}

	/// Get name
	const char *getName() const { return name; }

	/// Get type
	OpenGLSiBinSymbolType getType() const { return type; }

};

//---------------------------.inputs section ---------------------------------
// Input descriptor for .inputs section
class OpenGLSiBinInput
{
	OpenGLSiBinInputType type;
	uint32_t voffset;
	uint32_t poffset;
	// bool isFloat16;  FIXME: has to comment this as the size will be 29 instead of 28 bytes  
	OpenGLSiBinInputSwizzleType swizzles[4];
public:
	OpenGLSiBinInput(const char *buffer);
}__attribute__((packed));


//---------------------------.outputs section --------------------------------
// Output descriptor for .outputs section
class OpenGLSiBinOutput 
{
	OpenGLSiBinOutputType type;          // Semantic type
	uint32_t voffset;                    // Virtual offset
	uint32_t poffset;                    // Physical offset
	OpenGLSiBinSymbolDatatype data_type; // Data type	
	// uint32_t array_size;                 // Array size FIXME: otherwise size doesn't match
	char* name;                          // Name of the output
public:
	OpenGLSiBinOutput(const char *buffer);
	~OpenGLSiBinOutput(); // name element is a dedicated copy, needs to free

	/// Getters
	/// Return pointer to the beginning of next output object in .outputs section
	const char *getPtrNextOutput(const char *buffer) { 
		OpenGLSiBinOutput *output_ptr = (OpenGLSiBinOutput *)buffer;
		return (const char *)(&output_ptr->name + strlen(output_ptr->name) + 1);
	}
}__attribute__((packed));

// --------------------- Shader binary ---------------------------------------
class OpenGLSiShaderBinaryCommon : public ELFReader::File
{
protected:
	// Same for all shaders
	std::vector<std::unique_ptr<OpenGLSiBinInput>>   inputs;
	std::vector<std::unique_ptr<OpenGLSiBinOutput>>  outputs;
	std::unique_ptr<OpenGLSiBinInfo>                 info;
	std::unique_ptr<OpenGLSiBinUsageinfo>            usageinfo;
	std::vector<std::unique_ptr<OpenGLSiBinSymbol>>  symbols;

	// Decode ELF binary and store information in above structures/containers
	void DecodeInputs(ELFReader::Section *section);
	void DecodeOutputs(ELFReader::Section *section);
	void DecodeInfo(ELFReader::Section *section);
	void DecodeUsageInfo(ELFReader::Section *section);
	void DecodeSymbols(ELFReader::Section *section);

public:
	OpenGLSiShaderBinaryCommon(const char *buffer, unsigned int size);

	std::vector<std::unique_ptr<OpenGLSiBinSymbol>> &getSymbols() { return symbols; }
};

// Vertex Shader ELF binary with decoded information
class OpenGLSiShaderBinaryVertex : public OpenGLSiShaderBinaryCommon
{
	// metadata is stored in .text section
	std::unique_ptr<OpenGLSiBinVertexShaderMetadata> meta;
public:
	OpenGLSiShaderBinaryVertex(const char *buffer, unsigned size);
};

// Pixel Shader ELF binary with decoded information
class OpenGLSiShaderBinaryPixel : public OpenGLSiShaderBinaryCommon
{
	// metadata is stored in .text section
	std::unique_ptr<OpenGLSiBinPixelShaderMetadata> meta;
public:
	OpenGLSiShaderBinaryPixel(const char *buffer, unsigned size);
};

// Binary get by glGetProgramBinary. It should be ELF formatted in latest AMD driver
class OpenGLSiProgramBinary : public ELFReader::File
{
	std::unique_ptr<OpenGLSiShaderBinaryVertex> vertex_shader;
	std::unique_ptr<OpenGLSiShaderBinaryPixel> pixel_shader;

public:
	OpenGLSiProgramBinary(const char *buffer, unsigned buffer_size);

	/// Getter
	///
	/// Get associated vertex shader
	OpenGLSiShaderBinaryVertex *getVertexShader() const { 
		return vertex_shader.get();
	}

	/// Get associated pixel shader
	OpenGLSiShaderBinaryPixel *getPixelShader() const {
		return pixel_shader.get();
	}
};

} // namepace SI

#endif
