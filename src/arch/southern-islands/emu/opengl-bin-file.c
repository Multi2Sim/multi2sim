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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "opengl-bin-file.h"




/*
 * Private Functions
 */

typedef enum _E_SC_USER_DATA_CLASS
{
	IMM_RESOURCE,               // immediate resource descriptor
	IMM_SAMPLER,                // immediate sampler descriptor
	IMM_CONST_BUFFER,           // immediate const buffer descriptor
	IMM_VERTEX_BUFFER,          // immediate vertex buffer descriptor
	IMM_UAV,                    // immediate UAV descriptor
	IMM_ALU_FLOAT_CONST,        // immediate float const (scalar or vector)
	IMM_ALU_BOOL32_CONST,       // 32 immediate bools packed into a single UINT
	IMM_GDS_COUNTER_RANGE,      // immediate UINT with GDS address range for counters
	IMM_GDS_MEMORY_RANGE,       // immediate UINT with GDS address range for storage
	IMM_GWS_BASE,               // immediate UINT with GWS resource base offset
	IMM_WORK_ITEM_RANGE,        // immediate HSAIL work item range
	IMM_WORK_GROUP_RANGE,       // immediate HSAIL work group range
	IMM_DISPATCH_ID,            // immediate HSAIL dispatch ID
	IMM_SCRATCH_BUFFER,         // immediate HSAIL scratch buffer descriptor
	IMM_HEAP_BUFFER,            // immediate HSAIL heap buffer descriptor
	IMM_KERNEL_ARG,             // immediate HSAIL kernel argument
	IMM_CONTEXT_BASE,           // immediate HSAIL context base-address
	IMM_LDS_ESGS_SIZE,          // immediate LDS ESGS size used in on-chip GS
	SUB_PTR_FETCH_SHADER,       // fetch shader subroutine pointer
	PTR_RESOURCE_TABLE,         // flat/chunked resource table pointer
	PTR_CONST_BUFFER_TABLE,     // flat/chunked const buffer table pointer
	PTR_INTERNAL_RESOURCE_TABLE,// flat/chunked internal resource table pointer
	PTR_SAMPLER_TABLE,          // flat/chunked sampler table pointer
	PTR_UAV_TABLE,              // flat/chunked UAV resource table pointer
	PTR_INTERNAL_GLOBAL_TABLE,  // internal driver table pointer
	PTR_VERTEX_BUFFER_TABLE,    // flat/chunked vertex buffer table pointer
	PTR_SO_BUFFER_TABLE,        // flat/chunked stream-out buffer table pointer
	PTR_EXTENDED_USER_DATA,     // extended user data in video memory
	PTR_INDIRECT_RESOURCE,      // pointer to resource indirection table
	PTR_INDIRECT_INTERNAL_RESOURCE,// pointer to internal resource indirection table
	PTR_INDIRECT_UAV,           // pointer to UAV indirection table
	E_SC_USER_DATA_CLASS_LAST
} E_SC_USER_DATA_CLASS;


static int si_opengl_shader_binary_get_isa_offset(struct si_opengl_shader_binary_t *shdr)
{
	int isa_offset;

	switch (shdr->shader_kind)
	{
		case SI_OPENGL_SHADER_VERTEX:
		{
			isa_offset = 2124;
			break;
		}
		case SI_OPENGL_SHADER_FRAGMENT:
		{
			isa_offset = 3436;
			break;
		}
		case SI_OPENGL_SHADER_GEOMETRY:
		{
			isa_offset = 3976;
			break;
		}
		default:
			isa_offset = 0;
	}

	return isa_offset;	
}

static void si_opengl_shader_binary_set_type(struct si_opengl_shader_binary_t *shdr)
{
	switch(shdr->shader_elf->header->e_flags)
	{
	case 0x0:
		shdr->shader_kind = SI_OPENGL_SHADER_VERTEX;
		break;
	case 0x4:
		shdr->shader_kind = SI_OPENGL_SHADER_FRAGMENT;
		break;
	default:
		shdr->shader_kind = SI_OPENGL_SHADER_INVALID;
		break;
	}
}

static void si_opengl_shader_binary_set_isa(struct si_opengl_shader_binary_t *shdr)
{
	struct elf_section_t *section;
	int offset;
	int i;

	/* The ISA is in .text section */
	offset = si_opengl_shader_binary_get_isa_offset(shdr);
	LIST_FOR_EACH(shdr->shader_elf->section_list, i)
	{
		section = list_get(shdr->shader_elf->section_list, i);
		if (!strcmp(section->name, ".text"))
		{
			shdr->shader_isa = xcalloc(1, sizeof(struct elf_buffer_t));
			shdr->shader_isa->ptr = section->buffer.ptr + offset;
			shdr->shader_isa->size = section->buffer.size - offset;
			shdr->shader_isa->pos = 0;
		}
	}

}

static struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t *si_opengl_bin_spi_shader_pgm_rsrc2_vs_create()
{
	struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t *pgm_rsrc2;

	/* Allocate */
	pgm_rsrc2 = xcalloc(1, sizeof(struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t));

	/* Return */	
	return pgm_rsrc2;
}

static void si_opengl_bin_spi_shader_pgm_rsrc2_vs_free(struct si_opengl_bin_spi_shader_pgm_rsrc2_vs_t *pgm_rsrc2)
{
	free(pgm_rsrc2);
}

static struct si_opengl_bin_enc_dict_entry_t *si_opengl_bin_enc_dict_entry_create()
{
	struct si_opengl_bin_enc_dict_entry_t *enc_dict;

	/* Allocate */
	enc_dict = xcalloc(1, sizeof(struct si_opengl_bin_enc_dict_entry_t));

	/* Initialize */
	enc_dict->shader_pgm_rsrc2_vs = si_opengl_bin_spi_shader_pgm_rsrc2_vs_create();

	/* Return */	
	return enc_dict;
}

static void si_opengl_bin_enc_dict_entry_free(struct si_opengl_bin_enc_dict_entry_t *enc_dict)
{
	si_opengl_bin_spi_shader_pgm_rsrc2_vs_free(enc_dict->shader_pgm_rsrc2_vs);
	free(enc_dict);
}

static void si_opengl_shader_binary_set_enc_dict(struct si_opengl_shader_binary_t *shdr)
{
	struct si_opengl_bin_enc_dict_entry_t *enc_dict;

	/* Allocate */
	shdr->shader_enc_dict = si_opengl_bin_enc_dict_entry_create();
	enc_dict = shdr->shader_enc_dict;

	/* Initialize */
	/* FIXME: should get this info from binary! */
	switch(shdr->shader_kind)
	{
	case SI_OPENGL_SHADER_VERTEX:
		/* Fetch shader */
		enc_dict->userElements[0].dataClass = SUB_PTR_FETCH_SHADER;
		enc_dict->userElements[0].apiSlot = 0x0; /* */
		enc_dict->userElements[0].startUserReg = 0x00000002; /* s2, s3*/
		enc_dict->userElements[0].userRegCount = 0x00000002;
		/* Vertex Buffer Table */
		enc_dict->userElements[0].dataClass = PTR_VERTEX_BUFFER_TABLE;
		enc_dict->userElements[0].apiSlot = 0x0; /* ? */
		enc_dict->userElements[0].startUserReg = 0x00000004;	/* s4, s5 */
		enc_dict->userElements[0].userRegCount = 0x00000002;
		break;
	default:
		break;
	}
}

struct si_opengl_bin_enc_user_element_t *si_opengl_bin_enc_user_element_create()
{
	struct si_opengl_bin_enc_user_element_t *user_elem;

	/* Initialize */
	user_elem = xcalloc(1, sizeof(struct si_opengl_bin_enc_user_element_t));
	
	/* Return */
	return user_elem;
}

void si_opengl_bin_enc_user_element_free(struct si_opengl_bin_enc_user_element_t *user_elem)
{
	free(user_elem);
}

struct si_opengl_shader_binary_t *si_opengl_shader_binary_create(void *buffer, int size, char* name)
{
	struct si_opengl_shader_binary_t *shdr;

	/* Allocate */
	shdr = xcalloc(1, sizeof(struct si_opengl_shader_binary_t));

	/* Initialize */
	shdr->shader_elf = elf_file_create_from_buffer(buffer, size, name);
	if (shdr->shader_elf)
	{
		si_opengl_shader_binary_set_type(shdr);
		si_opengl_shader_binary_set_isa(shdr);
		/* FIXME: encoding dictionary currently use fixed settings */
		si_opengl_shader_binary_set_enc_dict(shdr);
	}

	/* Return */
	return shdr;
}

void si_opengl_shader_binary_free(struct si_opengl_shader_binary_t *shdr)
{
	si_opengl_bin_enc_dict_entry_free(shdr->shader_enc_dict);
	elf_file_free(shdr->shader_elf);
	free(shdr->shader_isa);
	free(shdr);
}

static struct list_t *si_opengl_shaders_list_create(struct elf_file_t *binary)
{
	struct list_t *lst;
	struct si_opengl_shader_binary_t *shader;
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
			shader = si_opengl_shader_binary_create(section->buffer.ptr + symbol->value, 
				symbol->size, 
				symbol->name);
			list_add(lst, shader);
		}
	}

	/* Return */
	return lst;
}

static void si_opengl_shaders_list_free(struct list_t *shaders)
{
	struct si_opengl_shader_binary_t *shdr;
	int i;

	/* Free */
	LIST_FOR_EACH(shaders, i)
	{
		shdr = list_get(shaders, i);
		si_opengl_shader_binary_free(shdr);
	}

	list_free(shaders);
}

/*
 * Public functions
 */

struct si_opengl_program_binary_t *si_opengl_program_binary_create(void *buffer_ptr, int size, char *name)
{
	struct si_opengl_program_binary_t *program_bin;

	/* Allocate */
	program_bin = xcalloc(1, sizeof(struct si_opengl_program_binary_t));

	/* Initialize */
	program_bin->name = xstrdup(name);
	program_bin->binary = elf_file_create_from_buffer(buffer_ptr, size, name);
	if (!program_bin->binary)
		fatal("Shader binary is generated by old AMD driver, please upgrade!");
	program_bin->shaders = si_opengl_shaders_list_create(program_bin->binary);

	/* Return */	
	return program_bin;
}

void si_opengl_program_binary_free(struct si_opengl_program_binary_t *program_bin)
{
	/* Free shader binary */
	free(program_bin->name);
	elf_file_free(program_bin->binary);
	si_opengl_shaders_list_free(program_bin->shaders);

	free(program_bin);
}

