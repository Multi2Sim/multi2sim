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

namespace SI
{

// Common 
#define MAX_USER_ELEMENTS 16
#define MAX_SEMANTICS_MAPPINGS 16
#define MAX_NUM_SAMPLER  32
#define MAX_NUM_RESOURCE 256
#define MAX_NUM_UAV      1024
#define MAX_CONSTANTS    256

#define MAX_UAV      12      // mask uav supported
#define MAX_TRANSFORM_FEEDBACK_BUFFERS 4  // maximum number of transform feedback buffers supported

#define MAX_UAV_RESOURCES 1024 // The maximum number of UAVs supported
#define MAX_UAV_MASKS (((MAX_UAV_RESOURCES) + 31) / 32)   // The maximum of UAV masks

#define MAX_TEXTURE_IMAGE_UNITS        32 // The maximum number of texture image units supported by GLL
#define MAX_UAV_IMAGE_UNITS            8  // The maximum number of image (UAV) supported by GLL
#define NUM_GLOBAL_RETURN_BUFFERS      8  // max global return buffers
#define MAX_TEXTURE_RESOURCES          160 // The maximus number of texture resource
#define PS_MAX_OUTPUTS                 8   // Max Render Targets

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

// Shader Types
enum OpenGLSiShaderBinaryKind
{
	OpenGLSiShaderVertex = 0,
	OpenGLSiShaderGeometry,
	OpenGLSiShaderHull, // Aka Tess Control Shader
	OpenGLSiShaderDomain, // Aka Tess Evaluastion Shader
	OpenGLSiShaderPixel = 4, // Aka Fragment Shader
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
	OpenGLSiInputAttrib,	// Generic Attribute
	OpenGLSiInputColor,	// Primary Color
	OpenGLSiInputSecondarycolor,	// Secondary Color
	OpenGLSiInputTexcoord,	// Texture Coordinate
	OpenGLSiInputTexid,	// Texture Unit Id
	OpenGLSiInputBufferid,	// Buffer Unit Id
	OpenGLSiInputConstantbufferid,	// Constant Buffer Unit Id
	OpenGLSiInputTextureresourceid	// Texture Resource Id	
};

enum OpenGLSiBinOutputType
{
	OpenGLSiOutputPos,	// Position
	OpenGLSiOutputPointsize,	// Point Size
	OpenGLSiOutputColor,	// Primary Color, Offset 0 Is Front, Offset 1 Is Back
	OpenGLSiOutputSecondarycolor,	// Secondary Color, Offset 0 Is Front, Offset 1 Is Back
	OpenGLSiOutputGeneric,	// Texture Coordinate And User Define Varyings For Pre R5xx Asics, But On R6xx Above, Generic Contains Colors
	OpenGLSiOutputDepth,	// Fragment Depth
	OpenGLSiOutputClipdistance,	// Clip Distance
	OpenGLSiOutputPrimitiveid,	// Primitiveid
	OpenGLSiOutputLayer,	// Layer
	OpenGLSiOutputViewportindex,	// Viewportindex
	OpenGLSiOutputStencil,	// Fragment Stencil Ref Value
	OpenGLSiOutputStencilValue,	// Fragment Stencil Operation Value
	OpenGLSiOutputSamplemask,	// Output Sample Coverage Mask (fs Only)
	OpenGLSiOutputStreamid	// Ouput Stream Id
};

// Symbol Types
enum OpenGLSiBinSymbolType
{
	OpenGLSiSymbolUniform,                      // Glsl Uniform
	OpenGLSiSymbolBindableUniform,             // Glsl Bindable Uniform
	OpenGLSiSymbolUniformBlock,                // Glsl Uniform Block
	OpenGLSiSymbolAttribPosition,              // Vertex Position
	OpenGLSiSymbolAttribNormal,                // Vertex Normal
	OpenGLSiSymbolAttribPrimaryColor,         // Vertex Primary Color
	OpenGLSiSymbolAttribSecondaryColor,       // Vertex Secondary Color
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
	OpenGLSiSymbolFsvaryingout,                 // Varying Out
	OpenGLSiSymbolAttribVertexid,              // Vertexid
	OpenGLSiSymbolAttribInstanceid,            // Instanceid
	OpenGLSiSymbolSubroutineUniform,           // Glsl Subroutine Uniform
	OpenGLSiSymbolSubroutineFunction,          // Glsl Subroutine Function
	OpenGLSiSymbolSeparateVaryingInBase,     // Base Varying In Symbol Of Separate Mode
	OpenGLSiSymbolSeparateVaryingOutBase,    // Base Varying Out Symbol Of Sparate Mode
	OpenGLSiSymbolSeparateVaryingInDerived,  // Derived Varying In Symbol Of Separate Mode
	OpenGLSiSymbolSeparateVaryingOutDerived, // Derived Varying Out Symbol Of Sparate Mode
	OpenGLSiSymbolNonseparateVaryingIn,       // Varying In Of Nonseparate Mode
	OpenGLSiSymbolNonseparateVaryingOut,      // Varying Out Of Nonseparate Mode
	OpenGLSiSymbolShaderStorageBuffer,        // Shader Storage Buffer
	OpenGLSiSymbolAttribInputvalid,            // Input Valide Bits For Sparse Texture
};


// Symbol Data Types
enum OpenGLSiBinSymbolDatatype
{
	OpenGLSiSymbolDatatypeVoid,                   // Void
	OpenGLSiSymbolDatatypeBool,                   // Bool
	OpenGLSiSymbolDatatypeInt,                    // Int
	OpenGLSiSymbolDatatypeUint,                   // Unsigned Int
	OpenGLSiSymbolDatatypeFloat,                  // Float
	OpenGLSiSymbolDatatypeDouble,                 // Double
	OpenGLSiSymbolDatatypeFloat16,                // Float16
	OpenGLSiSymbolDatatypeDvec2,                  // Dvec2
	OpenGLSiSymbolDatatypeDvec3,                  // Dvec3
	OpenGLSiSymbolDatatypeDvec4,                  // Dvec4
	OpenGLSiSymbolDatatypeVec2,                   // Vec2
	OpenGLSiSymbolDatatypeVec3,                   // Vec3
	OpenGLSiSymbolDatatypeVec4,                   // Vec4
	OpenGLSiSymbolDatatypeBvec2,                  // Bvec2
	OpenGLSiSymbolDatatypeBvec3,                  // Bvec3
	OpenGLSiSymbolDatatypeBvec4,                  // Bvec4
	OpenGLSiSymbolDatatypeIvec2,                  // Ivec2
	OpenGLSiSymbolDatatypeIvec3,                  // Ivec3
	OpenGLSiSymbolDatatypeIvec4,                  // Ivec4
	OpenGLSiSymbolDatatypeUvec2,                  // Unsigned Ivec2
	OpenGLSiSymbolDatatypeUvec3,                  // Unsigned Ivec3
	OpenGLSiSymbolDatatypeUvec4,                  // Unsigned Ivec4
	OpenGLSiSymbolDatatypeF16vec2,                // F16vec2
	OpenGLSiSymbolDatatypeF16vec3,                // F16vec3
	OpenGLSiSymbolDatatypeF16vec4,                // F16vec4
	OpenGLSiSymbolDatatypeMat2,                   // Mat2
	OpenGLSiSymbolDatatypeMat2x3,                 // Mat2x3
	OpenGLSiSymbolDatatypeMat2x4,                 // Mat2x4
	OpenGLSiSymbolDatatypeMat3x2,                 // Mat3x2
	OpenGLSiSymbolDatatypeMat3,                   // Mat3
	OpenGLSiSymbolDatatypeMat3x4,                 // Mat3x4
	OpenGLSiSymbolDatatypeMat4x2,                 // Mat4x2
	OpenGLSiSymbolDatatypeMat4x3,                 // Mat4x3
	OpenGLSiSymbolDatatypeMat4,                   // Mat4
	OpenGLSiSymbolDatatypeDmat2,                  // Dmat2
	OpenGLSiSymbolDatatypeDmat2x3,                // Dmat2x3
	OpenGLSiSymbolDatatypeDmat2x4,                // Dmat2x4
	OpenGLSiSymbolDatatypeDmat3x2,                // Dmat3x2
	OpenGLSiSymbolDatatypeDmat3,                  // Dmat3
	OpenGLSiSymbolDatatypeDmat3x4,                // Dmat3x4
	OpenGLSiSymbolDatatypeDmat4x2,                // Dmat4x2
	OpenGLSiSymbolDatatypeDmat4x3,                // Dmat4x3
	OpenGLSiSymbolDatatypeDmat4,                  // Dmat4
	OpenGLSiSymbolDatatypeF16mat2,                // F16mat2
	OpenGLSiSymbolDatatypeF16mat2x3,              // F16mat2x3
	OpenGLSiSymbolDatatypeF16mat2x4,              // F16mat2x4
	OpenGLSiSymbolDatatypeF16mat3x2,              // F16mat3x2
	OpenGLSiSymbolDatatypeF16mat3,                // F16mat3
	OpenGLSiSymbolDatatypeF16mat3x4,              // F16mat3x4
	OpenGLSiSymbolDatatypeF16mat4x2,              // F16mat4x2
	OpenGLSiSymbolDatatypeF16mat4x3,              // F16mat4x3
	OpenGLSiSymbolDatatypeF16mat4,                // F16mat4

	OpenGLSiSymbolDatatypeSampler1d,             // Sampler1d
	OpenGLSiSymbolDatatypeSampler2d,             // Sampler2d
	OpenGLSiSymbolDatatypeSampler2dRect,        // Sampler2drect
	OpenGLSiSymbolDatatypeSamplerExternal,       // Samplerexternaloes
	OpenGLSiSymbolDatatypeSampler3d,             // Sampler3d
	OpenGLSiSymbolDatatypeSamplerCube,           // Samplercube
	OpenGLSiSymbolDatatypeSampler1dArray,       // Sampler1darray
	OpenGLSiSymbolDatatypeSampler2dArray,       // Sampler2darray
	OpenGLSiSymbolDatatypeSamplerCubeArray,        //samplercubearray
	OpenGLSiSymbolDatatypeSamplerBuffer,         // Samplerbuffer
	OpenGLSiSymbolDatatypeSamplerRenderbuffer,   // Samplerrenderbuffer
	OpenGLSiSymbolDatatypeSampler2dMs,          // Sampler2dms
	OpenGLSiSymbolDatatypeSampler2dMsArray,    // Sampler2dmsarray

	OpenGLSiSymbolDatatypeSampler1dShadow,      // Sampler1dshadow
	OpenGLSiSymbolDatatypeSampler2dShadow,      // Sampler2dshadow
	OpenGLSiSymbolDatatypeSampler2dRectShadow, // Sampler2drectshadow
	OpenGLSiSymbolDatatypeSamplerCubeShadow,    // Samplercubeshadow
	OpenGLSiSymbolDatatypeSampler1dArrayShadow,// Sampler1darrayshadow
	OpenGLSiSymbolDatatypeSampler2dArrayShadow,// Sampler2darrayshadow
	OpenGLSiSymbolDatatypeSamplerCubeArrayShadow, //samplercubearrayshadow
	                                                  ///
	OpenGLSiSymbolDatatypeIntSampler1d,         // Isampler1d
	OpenGLSiSymbolDatatypeIntSampler2d,         // Isampler2d
	OpenGLSiSymbolDatatypeIntSampler2dRect,    // Isampler2drect
	OpenGLSiSymbolDatatypeIntSampler3d,         // Isampler3d
	OpenGLSiSymbolDatatypeIntSamplerCube,       // Isamplercube
	OpenGLSiSymbolDatatypeIntSampler1dArray,   // Isampler1darray
	OpenGLSiSymbolDatatypeIntSampler2dArray,   // Isampler2darray
	OpenGLSiSymbolDatatypeIntSamplerCubeArray, // Isamplercubearray
	OpenGLSiSymbolDatatypeIntSamplerBuffer,     // Isamplerbuffer
	OpenGLSiSymbolDatatypeIntSamplerRenderbuffer,   // Isamplerrenderbuffer
	OpenGLSiSymbolDatatypeIntSampler2dMs,          // Isampler2dms
	OpenGLSiSymbolDatatypeIntSampler2dMsArray,    // Isampler2dmsarray

	OpenGLSiSymbolDatatypeUnsignedIntSampler1d,         // Usampler1d
	OpenGLSiSymbolDatatypeUnsignedIntSampler2d,         // Usampler2d
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dRect,    // Usampler2drect
	OpenGLSiSymbolDatatypeUnsignedIntSampler3d,         // Usampler3d
	OpenGLSiSymbolDatatypeUnsignedIntSamplerCube,       // Usamplercube
	OpenGLSiSymbolDatatypeUnsignedIntSampler1dArray,   // Usampler1darray
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dArray,   // Usampler2darray
	OpenGLSiSymbolDatatypeUnsignedIntSamplerCubeArray, // Usamplercubearray
	OpenGLSiSymbolDatatypeUnsignedIntSamplerBuffer,     // Usamplerbuffer
	OpenGLSiSymbolDatatypeUnsignedIntSamplerRenderbuffer,   // Usamplerrenderbuffer
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dMs,          // Usampler2dms
	OpenGLSiSymbolDatatypeUnsignedIntSampler2dMsArray,    // Usampler2dmsarray

	OpenGLSiSymbolDatatypeFloat16Sampler1d,             // F16sampler1d
	OpenGLSiSymbolDatatypeFloat16Sampler2d,             // F16sampler2d
	OpenGLSiSymbolDatatypeFloat16Sampler2dRect,        // F16sampler2drect
	OpenGLSiSymbolDatatypeFloat16Sampler3d,             // F16sampler3d
	OpenGLSiSymbolDatatypeFloat16SamplerCube,           // F16samplercube
	OpenGLSiSymbolDatatypeFloat16Sampler1dArray,       // F16sampler1darray
	OpenGLSiSymbolDatatypeFloat16Sampler2dArray,       // F16sampler2darray
	OpenGLSiSymbolDatatypeFloat16SamplerCubeArray,        //f16samplercubearray
	OpenGLSiSymbolDatatypeFloat16SamplerBuffer,         // F16samplerbuffer
	OpenGLSiSymbolDatatypeFloat16Sampler2dMs,          // F16sampler2dms
	OpenGLSiSymbolDatatypeFloat16Sampler2dMsArray,    // F16sampler2dmsarray
	OpenGLSiSymbolDatatypeFloat16Sampler1dShadow,      // F16sampler1dshadow
	OpenGLSiSymbolDatatypeFloat16Sampler2dShadow,      // F16sampler2dshadow
	OpenGLSiSymbolDatatypeFloat16Sampler2dRectShadow, // F16sampler2drectshadow
	OpenGLSiSymbolDatatypeFloat16SamplerCubeShadow,    // F16samplercubeshadow
	OpenGLSiSymbolDatatypeFloat16Sampler1dArrayShadow,// F16sampler1darrayshadow
	OpenGLSiSymbolDatatypeFloat16Sampler2dArrayShadow,// F16sampler2darrayshadow
	OpenGLSiSymbolDatatypeFloat16SamplerCubeArrayShadow,// F16samplercubearrayshadow

	OpenGLSiSymbolDatatypeImage1d,                        // Image1d
	OpenGLSiSymbolDatatypeImage2d,                        // Image2d
	OpenGLSiSymbolDatatypeImage3d,                        // Image3d
	OpenGLSiSymbolDatatypeImage2dRect,                   // Image2drect
	OpenGLSiSymbolDatatypeImageCubemap,                   // Imagecube
	OpenGLSiSymbolDatatypeImageBuffer,                    // Imagebuffer
	OpenGLSiSymbolDatatypeImage1dArray,                  // Image1darray
	OpenGLSiSymbolDatatypeImage2dArray,                  // Image2darray
	OpenGLSiSymbolDatatypeImageCubemapArray,             // Imagecubearray
	OpenGLSiSymbolDatatypeImage2dMs,                     // Image2dmultisample
	OpenGLSiSymbolDatatypeImage2dMsArray,               // Image2dmultisamplearray

	OpenGLSiSymbolDatatypeIntImage1d,                    // Iimage1d
	OpenGLSiSymbolDatatypeIntImage2d,                    // Iimage2d
	OpenGLSiSymbolDatatypeIntImage3d,                    // Iimage3d
	OpenGLSiSymbolDatatypeIntImage2dRect,               // Iimage2drect
	OpenGLSiSymbolDatatypeIntImageCubemap,               // Iimagecube
	OpenGLSiSymbolDatatypeIntImageBuffer,                // Iimagebuffer
	OpenGLSiSymbolDatatypeIntImage1dArray,              // Iimage1darray
	OpenGLSiSymbolDatatypeIntImage2dArray,              // Iimage2darray
	OpenGLSiSymbolDatatypeIntImageCubemapArray,         // Iimagecubearray
	OpenGLSiSymbolDatatypeIntImage2dMs,                 // Iimage2dmultisample
	OpenGLSiSymbolDatatypeIntImage2dMsArray,           // Iimage2dmultisamplearray

	OpenGLSiSymbolDatatypeUnsignedIntImage1d,           // Uimage1d
	OpenGLSiSymbolDatatypeUnsignedIntImage2d,           // Uimage2d
	OpenGLSiSymbolDatatypeUnsignedIntImage3d,           // Uimage3d
	OpenGLSiSymbolDatatypeUnsignedIntImage2dRect,      // Uimage2drect
	OpenGLSiSymbolDatatypeUnsignedIntImageCubemap,      // Uimagecube
	OpenGLSiSymbolDatatypeUnsignedIntImageBuffer,       // Uimagebuffer
	OpenGLSiSymbolDatatypeUnsignedIntImage1dArray,     // Uimage1darray
	OpenGLSiSymbolDatatypeUnsignedIntImage2dArray,     // Uimage2darray
	OpenGLSiSymbolDatatypeUnsignedIntImageCubemapArray,// Uimagecubearray
	OpenGLSiSymbolDatatypeUnsignedIntImage2dMs,        // Uimage2dmultisample
	OpenGLSiSymbolDatatypeUnsignedIntImage2dMsArray,  // Uimage2dmultisamplearray

	OpenGLSiSymbolDatatypeFloat16Image1d,                // F16image1d
	OpenGLSiSymbolDatatypeFloat16Image2d,                // F16image2d
	OpenGLSiSymbolDatatypeFloat16Image3d,                // F16image3d
	OpenGLSiSymbolDatatypeFloat16Image2dRect,           // F16image2drect
	OpenGLSiSymbolDatatypeFloat16ImageCubemap,           // F16imagecube
	OpenGLSiSymbolDatatypeFloat16ImageBuffer,            // F16imagebuffer
	OpenGLSiSymbolDatatypeFloat16Image1dArray,          // F16image1darray
	OpenGLSiSymbolDatatypeFloat16Image2dArray,          // F16image2darray
	OpenGLSiSymbolDatatypeFloat16ImageCubemapArray,     // F16imagecubearray
	OpenGLSiSymbolDatatypeFloat16Image2dMs,             // F16image2dmultisample
	OpenGLSiSymbolDatatypeFloat16Image2dMsArray,       // F16image2dmultisamplearray

	OpenGLSiSymbolDatatypeSamplerBufferAmd,                 // Used For Tessellation (from Vbo)
	OpenGLSiSymbolDatatypeIntSamplerBufferAmd,             // Used For Vertex Shader Tesssellation
	OpenGLSiSymbolDatatypeUnsignedIntSamplerBufferAmd,    // Used For Vertex Shader Tesssellation

	OpenGLSiSymbolDatatypeAtomicCounter,          // Atomic Counter

	OpenGLSiSymbolDatatypeStruct,                 // Struct, Used For Bindable Uniform, It Is Also Used For Type Check In Separate Program Mode
	OpenGLSiSymbolDatatypeInterface,              // Interface Block, Used For Type Check In Separate Progarm Mode
	OpenGLSiSymbolDatatypeFirst = OpenGLSiSymbolDatatypeVoid,
	OpenGLSiSymbolDatatypeLast  = OpenGLSiSymbolDatatypeInterface
};

enum OpenGLSiFetchShaderType
{
	OpenGLSiFetchFixedFunction,   // Fetch Shader Executes All Vertex Fetches And Start Address Is In A Fixed Function Register.
	OpenGLSiFetchSubImmediate,    // Si+: Fetch Shader Performs All Fetches, Vertex Buffer Refs Are Stored As Immediate User Data
	OpenGLSiFetchSubFlatTable,   // Si+: Fetch Shader Performs All Fetches, Vertex Buffer Refs Are Stored In A Flat Table
	OpenGLSiFetchSubPerAttrib    // Si+: Fetch Shader Performs A Single Attribute Fetch Per Execution, Vertex Buffer Refs Stored In A Flat Table
};

} // namepace SI

#endif
