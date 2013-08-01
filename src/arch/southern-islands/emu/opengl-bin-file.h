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
};

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
};

struct si_opengl_bin_constant_usage_t
{
    uint32_t maskBits[(MAX_CONSTANTS + 31) / 32];
};


#define SI_OPENGL_BIN_SHADER_COMMON
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

#define SI_OPENGL_BIN_SHADER_SI_COMMON
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

#define SI_OPENGL_BIN_SHADER_SI_BASE
	SI_OPENGL_BIN_SHADER_COMMON 
	SI_OPENGL_BIN_SHADER_SI_COMMON

struct si_opengl_bin_input_t
{
	enum si_opengl_bin_input_type_t type;
	unsigned int voffset;
	unsigned int poffset;
	bool isFloat16;
	enum si_opengl_bin_input_swizzle_type_t swizzles[4];
};

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
	unsigned int usage: 8;  // semantic usage. IL_IMPORTUSAGE.
	unsigned int usageIdx  : 8;  // semantic index. Opaque to SC.
	unsigned int dataVgpr  : 8;  // first VGPR to contain fetch result
	unsigned int dataSize  : 2;  // (fetch_size - 1), size in elements
	unsigned int reserved  : 6;
};

/*
 *  Vertex shader output declaration to be used for semantic mapping.
 *  The paramIdx is the index of the export parameter SC uses in the shader.
 */
struct si_opengl_bin_vs_semantic_mapping_out_t
{
	uint32_t usage     : 8;      // semantic usage. IL_IMPORTUSAGE.
	uint32_t usageIdx  : 8;      // semantic index. Opaque to SC.
	uint32_t paramIdx  : 8;      // attribute export parameter index (0-31)
	uint32_t reserved  : 8;
};


/*
 *  Flags to guide shader compilation.
 */
struct si_opengl_bin_compile_guide_t
{
	/* To make this structure consistent with SC_COMPILE_FLAGS, we add some reserved bits. */
	uint32_t psPrimId           : 1; // PS needs primID input
	uint32_t useHsOffChip       : 1; // HS can use off-ship LDS
	uint32_t clampScratchAccess : 1; // add code to clamp scratch assesses
	uint32_t streamOutEnable    : 1; // enable writes to stream-out buffers
	uint32_t reserved1          : 4;
	uint32_t useGsOnChip        : 1; // ES and GS can use on-chip LDS. (CI+)
	uint32_t reserved2          : 23;
} ;

/* FIXME: Totally unverified */
struct si_opengl_bin_enc_semantic_mapping_t
{
	unsigned int count;
	unsigned int usageIndex;
	unsigned int startUserReg;
	unsigned int userRegCount;
};

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

	unsigned int semanticMappingCount;
	struct si_opengl_bin_enc_semantic_mapping_t semanticsMapping[MAX_SEMANTICS_MAPPINGS];

	struct list_t *input_list; /* Equivalent to arg_list in OpenCL, elements of type si_input_t */

	/* FIXME: currently only designed for Vertex Shader */
	struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t *shader_pgm_rsrc2_vs;
};

/* Vertex shader */
struct si_opengl_bin_si_vertex_shader_t  
{
	SI_OPENGL_BIN_SHADER_SI_BASE

	/* Input semantics */
	uint32_t numVsInSemantics;
	struct si_opengl_bin_vs_semantic_mapping_in_t vsInSemantics[SC_SI_VS_MAX_INPUTS];

	// Output semantics
	uint32_t numVsOutSemantics;
	struct si_opengl_bin_vs_semantic_mapping_out_t vsOutSemantics[SC_SI_VS_MAX_OUTPUTS];

	// LS/ES/VS specific shader resources
	union
	{
	    uint32_t spiShaderPgmRsrc2Ls;
	    uint32_t spiShaderPgmRsrc2Es;
	    uint32_t spiShaderPgmRsrc2Vs;
	};

	// SC-provided values for certain VS-specific registers
	uint32_t paClVsOutCntl;
	uint32_t spiVsOutConfig;
	uint32_t spiShaderPosFormat;
	uint32_t vgtStrmoutConfig;

	// Number of SPI-generated VGPRs referenced by the vertex shader
	uint32_t vgprCompCnt;

	// Exported vertex size in DWORDs, can be used to program ESGS ring size
	uint32_t exportVertexSize;

	bool useEdgeFlags;  
	bool  remapClipDistance; //true if clip distance[4-7] is remapped to [0-3] 
	uint32_t hwShaderStage;    // hardware stage which this shader actually in when execution
	struct si_opengl_bin_compile_guide_t compileFlags;     // compile flag
	uint32_t gsMode;                            // gs mode

	bool isOnChipGs;
	uint32_t targetLdsSize;

};


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

