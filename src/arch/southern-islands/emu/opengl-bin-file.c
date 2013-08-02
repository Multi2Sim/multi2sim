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
#include "opengl-bin-file.h"


/* Forward declaration */
static struct si_opengl_bin_si_vertex_shader_metadata_t *si_opengl_bin_si_vertex_shader_metadata_init_from_section(struct elf_section_t *section);
static void si_opengl_bin_si_vertex_shader_metadata_free(struct si_opengl_bin_si_vertex_shader_metadata_t *vs);
static struct si_opengl_bin_si_vertex_shader_t *si_opengl_bin_si_vertex_shader_create(struct si_opengl_shader_binary_t *parent);
static void si_opengl_bin_si_vertex_shader_free(struct si_opengl_bin_si_vertex_shader_t *vs);
static void si_opengl_bin_si_vertex_shader_init(struct si_opengl_bin_si_vertex_shader_t *vs);

/*
 * Private Functions
 */

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
		shdr->shader = si_opengl_bin_si_vertex_shader_create(shdr);
		shdr->free_func = (si_opengl_shader_free_func_t) &si_opengl_bin_si_vertex_shader_free;
		si_opengl_bin_si_vertex_shader_init(shdr->shader);
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

/* Structure in .text section */
static struct si_opengl_bin_si_vertex_shader_metadata_t *si_opengl_bin_si_vertex_shader_metadata_create()
{
	struct si_opengl_bin_si_vertex_shader_metadata_t *vs;

	/* Allocate */
	vs = xcalloc(1, sizeof(struct si_opengl_bin_si_vertex_shader_metadata_t));

	/* Return */
	return vs;
}

static void si_opengl_bin_si_vertex_shader_metadata_free(struct si_opengl_bin_si_vertex_shader_metadata_t *vs)
{
	/* Free */
	free(vs);
}

static struct si_opengl_bin_si_vertex_shader_metadata_t *si_opengl_bin_si_vertex_shader_metadata_init_from_section(struct elf_section_t *section)
{
	struct si_opengl_bin_si_vertex_shader_metadata_t *vs_meta;

	/* Create and memcpy */
	assert(sizeof(struct si_opengl_bin_si_vertex_shader_metadata_t) < section->buffer.size);
	vs_meta = si_opengl_bin_si_vertex_shader_metadata_create();
	memcpy(vs_meta, section->buffer.ptr, sizeof(struct si_opengl_bin_si_vertex_shader_metadata_t));

	/* Return */
	return vs_meta;
}

/* Structure in .inputs section */
static struct si_opengl_bin_si_input_t *si_opengl_bin_si_input_create()
{
	struct si_opengl_bin_si_input_t *input;

	/* Allocate */
	input = xcalloc(1, sizeof(struct si_opengl_bin_si_input_t));

	/* Return */
	return input;
}

static void si_opengl_bin_si_input_free(struct si_opengl_bin_si_input_t *input)
{
	free(input);
}

static void si_opengl_bin_si_inputs_init_from_section(struct list_t *lst, struct elf_section_t *section)
{
	struct si_opengl_bin_si_input_t *input;
	int input_count;
	int i;

	/* Calculate # of input */
	if (section->buffer.size % sizeof(struct si_opengl_bin_si_input_t))
		fatal("Section size must be multiples of input structure.");
	else
	{
		input_count = section->buffer.size / sizeof(struct si_opengl_bin_si_input_t);
		for (i = 0; i < input_count; ++i)
		{
			input = si_opengl_bin_si_input_create();
			memcpy(input, section->buffer.ptr + i * sizeof(struct si_opengl_bin_si_input_t), 
				sizeof(struct si_opengl_bin_si_input_t));
			list_add(lst, input);
		}
	}
}

/* Structure in .outputs section */
static struct si_opengl_bin_si_output_t *si_opengl_bin_si_output_create()
{
	struct si_opengl_bin_si_output_t *output;

	/* Allocate */
	output = xcalloc(1, sizeof(struct si_opengl_bin_si_output_t));

	/* Return */
	return output;
}

static void si_opengl_bin_si_output_free(struct si_opengl_bin_si_output_t *output)
{
	free(output->name);
	free(output);
}

static void si_opengl_bin_si_outputs_init_from_section(struct list_t *lst, struct elf_section_t *section)
{
	struct si_opengl_bin_si_output_t *output;
	struct si_opengl_bin_si_output_t* output_ptr;	
	char *outname;
	char *bin_ptr;
	unsigned int name_offset;
	size_t len;
	int output_count;
	int i;

	output_count = section->header->sh_entsize;
	bin_ptr = (char *) section->buffer.ptr;
	name_offset = sizeof(struct si_opengl_bin_si_output_t) - sizeof(char*);
	for (i = 0; i < output_count; ++i)
	{
		output_ptr = (struct si_opengl_bin_si_output_t *)bin_ptr;

		output = si_opengl_bin_si_output_create();
		outname = &bin_ptr[name_offset];
		if(*outname != '\0')
		{
			len = strlen(outname)+1;
			output->name = xstrdup(outname);
			output->data_type = output_ptr->data_type;
			output->array_size = output_ptr->array_size;
			bin_ptr += (name_offset + len);
		}
		else
		{
			bin_ptr += (name_offset + 1);
			output->name = NULL;
			output->type = (enum si_opengl_bin_output_type_t) output_ptr->type;
			output->poffset = output_ptr->poffset;
			output->array_size = output_ptr->array_size;
		}
		list_add(lst, output);
	}
}

static struct si_opengl_bin_si_vertex_shader_t *si_opengl_bin_si_vertex_shader_create(struct si_opengl_shader_binary_t *parent)
{
	struct si_opengl_bin_si_vertex_shader_t *vs;

	/* Allocate */
	vs = xcalloc(1, sizeof(struct si_opengl_bin_si_vertex_shader_t));
	vs->parent = parent;
	parent->shader = vs;
	vs->meta = si_opengl_bin_si_vertex_shader_metadata_create();
	vs->inputs = list_create();
	vs->outputs = list_create();

	/* Return */
	return vs;
}

static void si_opengl_bin_si_vertex_shader_free(struct si_opengl_bin_si_vertex_shader_t *vs)
{
	struct si_opengl_bin_si_input_t *input;
	struct si_opengl_bin_si_output_t *output;
	int i;

	si_opengl_bin_si_vertex_shader_metadata_free(vs->meta);
	LIST_FOR_EACH(vs->inputs, i)
	{
		input = list_get(vs->inputs, i);
		si_opengl_bin_si_input_free(input);
	}
	LIST_FOR_EACH(vs->outputs, i)
	{
		output = list_get(vs->outputs, i);
		si_opengl_bin_si_output_free(output);
	}
}

static void si_opengl_bin_si_vertex_shader_init(struct si_opengl_bin_si_vertex_shader_t *vs)
{
	struct si_opengl_shader_binary_t *parent;
	struct elf_file_t *shader_elf;
	struct elf_section_t *section;
	int i;

	/* Get parent */
	parent = vs->parent;
	assert(parent);
	assert(parent->shader_kind == SI_OPENGL_SHADER_VERTEX);

	shader_elf = parent->shader_elf;

	/* Initialize from sections */
	LIST_FOR_EACH(shader_elf->section_list, i)
	{
		section = list_get(shader_elf->section_list, i);
		if (!strcmp(section->name, ".text"))
			si_opengl_bin_si_vertex_shader_metadata_init_from_section(section);
		else if (!strcmp(section->name, ".inputs"))
			si_opengl_bin_si_inputs_init_from_section(vs->inputs, section);
		else if (!strcmp(section->name, ".outputs"))
			si_opengl_bin_si_outputs_init_from_section(vs->outputs, section);
	}
}

static struct si_opengl_bin_enc_dict_entry_t *si_opengl_bin_enc_dict_entry_create()
{
	struct si_opengl_bin_enc_dict_entry_t *enc_dict;

	/* Allocate */
	enc_dict = xcalloc(1, sizeof(struct si_opengl_bin_enc_dict_entry_t));
	enc_dict->input_list = list_create();

	/* Initialize */
	enc_dict->shader_pgm_rsrc2_vs = si_opengl_bin_spi_shader_pgm_rsrc2_vs_create();

	/* Return */	
	return enc_dict;
}

static void si_opengl_bin_enc_dict_entry_free(struct si_opengl_bin_enc_dict_entry_t *enc_dict)
{
	int i;

	si_opengl_bin_spi_shader_pgm_rsrc2_vs_free(enc_dict->shader_pgm_rsrc2_vs);
	LIST_FOR_EACH(enc_dict->input_list, i)
		si_input_free(list_get(enc_dict->input_list, i));
	list_free(enc_dict->input_list);
	free(enc_dict);
}

static void si_opengl_enc_dict_set_userElements(struct si_opengl_shader_binary_t *shdr, struct si_opengl_bin_enc_dict_entry_t *enc_dict)
{
	/* FIXME: should get this info from binary! */
	switch(shdr->shader_kind)
	{
	case SI_OPENGL_SHADER_VERTEX:
		/* Set userElementCount */
		enc_dict->userElementCount = 0x2;
		/* Fetch shader */
		enc_dict->userElements[0].dataClass = SUB_PTR_FETCH_SHADER;
		enc_dict->userElements[0].apiSlot = 0x0; /* */
		enc_dict->userElements[0].startUserReg = 0x00000002; /* s2, s3*/
		enc_dict->userElements[0].userRegCount = 0x00000002;
		/* Vertex Buffer Table */
		enc_dict->userElements[1].dataClass = PTR_VERTEX_BUFFER_TABLE;
		enc_dict->userElements[1].apiSlot = 0x0; /* ? */
		enc_dict->userElements[1].startUserReg = 0x00000004;	/* s4, s5 */
		enc_dict->userElements[1].userRegCount = 0x00000002;
		break;
	default:
		break;
	}
}

static void si_opengl_enc_dict_set_semanticMappings(struct si_opengl_shader_binary_t *shdr, struct si_opengl_bin_enc_dict_entry_t *enc_dict)
{
	/* FIXME: should get this info from binary! */
	switch(shdr->shader_kind)
	{
	case SI_OPENGL_SHADER_VERTEX:
		/* Semantic mapping */
		enc_dict->semanticsMapping[0].count = 0x0;
		enc_dict->semanticsMapping[0].usageIndex = 0x0;
		enc_dict->semanticsMapping[0].startUserReg = 0x8;
		enc_dict->semanticsMapping[0].userRegCount = 0x4;
		enc_dict->semanticsMapping[1].count = 0x0;
		enc_dict->semanticsMapping[1].usageIndex = 0x1;
		enc_dict->semanticsMapping[1].startUserReg = 0x4;
		enc_dict->semanticsMapping[1].userRegCount = 0x4;
		break;
	default:
		break;
	}
}

static void si_opengl_enc_dict_set_inputs(struct si_opengl_shader_binary_t *shdr, struct si_opengl_bin_enc_dict_entry_t *enc_dict)
{
	// struct elf_section_t *section;
	int i;

	/* FIXME: should get this info from binary! */
	int input_count = 2;
	struct si_input_t *input;

	/* Create input and add to input list */
	for (i = 0; i < input_count; ++i)
	{
		input = si_input_create();
		si_input_set_usage_index(input, i);
		list_insert(enc_dict->input_list, i, input);
	}

	switch(shdr->shader_kind)
	{
	case SI_OPENGL_SHADER_VERTEX:
		/* Inputs */
		break;
	default:
		break;
	}
}

static void si_opengl_shader_binary_set_enc_dict(struct si_opengl_shader_binary_t *shdr)
{
	struct si_opengl_bin_enc_dict_entry_t *enc_dict;

	/* Allocate */
	shdr->shader_enc_dict = si_opengl_bin_enc_dict_entry_create();
	enc_dict = shdr->shader_enc_dict;

	/* Initialize */
	si_opengl_enc_dict_set_userElements(shdr, enc_dict);
	si_opengl_enc_dict_set_semanticMappings(shdr, enc_dict);
	si_opengl_enc_dict_set_inputs(shdr, enc_dict);
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
		fatal("Shader binary is not ELF formatted, please upgrade your AMD driver!");
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

struct si_bin_enc_user_element_t *si_opengl_bin_enc_user_element_create()
{
	struct si_bin_enc_user_element_t *user_elem;

	/* Initialize */
	user_elem = xcalloc(1, sizeof(struct si_bin_enc_user_element_t));
	
	/* Return */
	return user_elem;
}

void si_opengl_bin_enc_user_element_free(struct si_bin_enc_user_element_t *user_elem)
{
	free(user_elem);
}
