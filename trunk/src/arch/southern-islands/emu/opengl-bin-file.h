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

#include <lib/util/elf-format.h>
#include <src/arch/southern-islands/asm/bin-file.h>

#define MAX_USER_ELEMENTS 16
#define MAX_SEMANTICS_MAPPINGS 16

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

	struct list_t *input_list; /* - Equivalent to arg_list in OpenCL, elements of type si_input_t */

	/* FIXME: currently only designed for Vertex Shader */
	struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t *shader_pgm_rsrc2_vs;
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

