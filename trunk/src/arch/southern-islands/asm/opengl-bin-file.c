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
#include <src/driver/opengl/si-shader.h>
#include "opengl-bin-file.h"


/* 
 * Forward declaration
 */

/* Vertex shader */
static struct opengl_si_enc_dict_vertex_shader_t *opengl_si_enc_dict_vertex_shader_create(struct opengl_si_shader_binary_t *parent);
static void opengl_si_bin_vertex_shader_free(struct opengl_si_enc_dict_vertex_shader_t *vs);
static void opengl_si_bin_vertex_shader_init(struct opengl_si_enc_dict_vertex_shader_t *vs);

/* Pixel shader */
static struct opengl_si_enc_dict_pixel_shader_t *opengl_si_enc_dict_pixel_shader_create(struct opengl_si_shader_binary_t *parent);
static void opengl_si_bin_pixel_shader_free(struct opengl_si_enc_dict_pixel_shader_t *ps);
static void opengl_si_bin_pixel_shader_init(struct opengl_si_enc_dict_pixel_shader_t *ps);


/*
 * Private Functions
 */

/*static struct str_map_t enc_dict_input_type_map =
{
	8, {
		{ "generic attribute",	OPENGL_SI_INPUT_ATTRIB },
		{ "primary color",	OPENGL_SI_INPUT_COLOR },
		{ "secondary color",	OPENGL_SI_INPUT_SECONDARYCOLOR },
		{ "texture coordinate",	OPENGL_SI_INPUT_TEXCOORD },
		{ "texture unit id",	OPENGL_SI_INPUT_TEXID },
		{ "buffer unit id",	OPENGL_SI_INPUT_BUFFERID },
		{ "constant buffer unit id",	OPENGL_SI_INPUT_CONSTANTBUFFERID },
		{ "texture resource id",	OPENGL_SI_INPUT_TEXTURERESOURCEID },
	}
};
*/
static struct str_map_t enc_dict_semantic_input_type_map =
{
	1, {
		{ "generic",	0 },
	}
};

static struct str_map_t enc_dict_semantic_output_type_map =
{
	7, {
		{ "generic",	0 },
		{ "unknown 1",	1 },
		{ "unknown 2",	2 },
		{ "unknown 3",	3 },
		{ "unknown 4",	4 },
		{ "unknown 5",	5 },
		{ "unknown 6",	6 },
	}
};

static struct str_map_t enc_dict_user_elements_type_map = 
{
	30, {
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
		{"unknown 0", 29},
	}
};

static int opengl_si_shader_binary_get_isa_offset(struct opengl_si_shader_binary_t *shdr)
{
	int isa_offset;

	switch (shdr->shader_kind)
	{
		case OPENGL_SI_SHADER_VERTEX:
		{
			isa_offset = 2124;
			break;
		}
		case OPENGL_SI_SHADER_PIXEL:
		{
			isa_offset = 3436;
			break;
		}
		case OPENGL_SI_SHADER_GEOMETRY:
		{
			isa_offset = 3976;
			break;
		}
		default:
			isa_offset = 0;
	}

	return isa_offset;	
}

/* Dummy callback function */
static void opengl_si_bin_shader_free(void *shader)
{
}

static void opengl_si_shader_binary_set(struct opengl_si_shader_binary_t *shdr)
{
	switch(shdr->elf->header->e_flags)
	{
	case 0x0:
		shdr->shader_kind = OPENGL_SI_SHADER_VERTEX;
		shdr->enc_dict = opengl_si_enc_dict_vertex_shader_create(shdr);
		shdr->free_func = (opengl_si_shader_bin_free_func_t) &opengl_si_bin_vertex_shader_free;
		opengl_si_bin_vertex_shader_init(shdr->enc_dict);
		break;
	case 0x4:
		shdr->shader_kind = OPENGL_SI_SHADER_PIXEL;
		shdr->enc_dict = opengl_si_enc_dict_pixel_shader_create(shdr);
		shdr->free_func = (opengl_si_shader_bin_free_func_t) &opengl_si_bin_pixel_shader_free;
		opengl_si_bin_pixel_shader_init(shdr->enc_dict);		
		break;
	default:
		shdr->shader_kind = OPENGL_SI_SHADER_INVALID;
		shdr->free_func = (opengl_si_shader_bin_free_func_t) &opengl_si_bin_shader_free;
		break;
	}
}

static void opengl_si_shader_binary_set_isa(struct opengl_si_shader_binary_t *shdr)
{
	struct elf_section_t *section;
	int offset;
	int i;

	/* The ISA is in .text section */
	offset = opengl_si_shader_binary_get_isa_offset(shdr);
	LIST_FOR_EACH(shdr->elf->section_list, i)
	{
		section = list_get(shdr->elf->section_list, i);
		if (!strcmp(section->name, ".text"))
		{
			shdr->isa = xcalloc(1, sizeof(struct elf_buffer_t));
			shdr->isa->ptr = section->buffer.ptr + offset;
			shdr->isa->size = section->buffer.size - offset;
			shdr->isa->pos = 0;
		}
	}
}

/* Structure in .text section for VS */
static struct opengl_si_bin_vertex_shader_metadata_t *opengl_si_bin_vertex_shader_metadata_create()
{
	struct opengl_si_bin_vertex_shader_metadata_t *vs_meta;

	/* Allocate */
	vs_meta = xcalloc(1, sizeof(struct opengl_si_bin_vertex_shader_metadata_t));

	/* Return */
	return vs_meta;
}

static void opengl_si_bin_vertex_shader_metadata_free(struct opengl_si_bin_vertex_shader_metadata_t *vs_meta)
{
	/* Free */
	free(vs_meta);
}

static struct opengl_si_bin_vertex_shader_metadata_t *opengl_si_bin_vertex_shader_metadata_init_from_section(
	struct opengl_si_bin_vertex_shader_metadata_t *vs_meta, 
	struct elf_section_t *section)
{
	/* Make sure section is correct */
	assert(!strcmp(section->name, ".text"));

	/* Create and memcpy */
	assert(sizeof(struct opengl_si_bin_vertex_shader_metadata_t) < section->buffer.size);
	memcpy(vs_meta, section->buffer.ptr, sizeof(struct opengl_si_bin_vertex_shader_metadata_t));

	/* Return */
	return vs_meta;
}

/* Structure in .text section for FS */
static struct opengl_si_bin_pixel_shader_metadata_t *opengl_si_bin_pixel_shader_metadata_create()
{
	struct opengl_si_bin_pixel_shader_metadata_t *fs_meta;

	/* Allocate */
	fs_meta = xcalloc(1, sizeof(struct opengl_si_bin_pixel_shader_metadata_t));

	/* Return */
	return fs_meta;
}

static void opengl_si_bin_pixel_shader_metadata_free(struct opengl_si_bin_pixel_shader_metadata_t *fs_meta)
{
	/* Free */
	free(fs_meta);
}

static struct opengl_si_bin_pixel_shader_metadata_t *opengl_si_bin_pixel_shader_metadata_init_from_section(
	struct opengl_si_bin_pixel_shader_metadata_t *fs_meta, 
	struct elf_section_t *section)
{
	/* Make sure section is correct */
	assert(!strcmp(section->name, ".text"));

	/* Create and memcpy */
	/* FIXME: size doesn't match */
	// assert(sizeof(struct opengl_si_bin_pixel_shader_metadata_t) < section->buffer.size);
	memcpy(fs_meta, section->buffer.ptr, sizeof(struct opengl_si_bin_pixel_shader_metadata_t));

	/* Return */
	return fs_meta;
}

/* Structure in .inputs section */
static struct opengl_si_bin_input_t *opengl_si_bin_input_create()
{
	struct opengl_si_bin_input_t *input;

	/* Allocate */
	input = xcalloc(1, sizeof(struct opengl_si_bin_input_t));

	/* Return */
	return input;
}

static void opengl_si_bin_input_free(struct opengl_si_bin_input_t *input)
{
	free(input);
}

static void opengl_si_bin_inputs_init_from_section(struct list_t *lst, struct elf_section_t *section)
{
	struct opengl_si_bin_input_t *input;
	int input_count;
	int i;

	assert(!strcmp(section->name, ".inputs"));

	/* Calculate # of input */
	if (section->buffer.size % sizeof(struct opengl_si_bin_input_t))
		fatal("Section size must be multiples of input structure.");
	else
	{
		input_count = section->buffer.size / sizeof(struct opengl_si_bin_input_t);
		for (i = 0; i < input_count; ++i)
		{
			input = opengl_si_bin_input_create();
			memcpy(input, section->buffer.ptr + i * sizeof(struct opengl_si_bin_input_t), 
				sizeof(struct opengl_si_bin_input_t));
			list_add(lst, input);
		}
	}
}

/* Structure in .outputs section */
static struct opengl_si_bin_output_t *opengl_si_bin_output_create()
{
	struct opengl_si_bin_output_t *output;

	/* Allocate */
	output = xcalloc(1, sizeof(struct opengl_si_bin_output_t));

	/* Return */
	return output;
}

static void opengl_si_bin_output_free(struct opengl_si_bin_output_t *output)
{
	free(output->name);
	free(output);
}

static void opengl_si_bin_outputs_init_from_section(struct list_t *lst, struct elf_section_t *section)
{
	struct opengl_si_bin_output_t *output;
	struct opengl_si_bin_output_t* output_ptr;	
	char *outname;
	char *bin_ptr;
	unsigned int name_offset;
	size_t len;
	int output_count;
	int i;

	/* Make sure section is correct */
	assert(!strcmp(section->name, ".outputs"));

	output_count = section->header->sh_entsize;
	bin_ptr = (char *) section->buffer.ptr;
	name_offset = sizeof(struct opengl_si_bin_output_t) - sizeof(char*);
	for (i = 0; i < output_count; ++i)
	{
		output_ptr = (struct opengl_si_bin_output_t *)bin_ptr;

		output = opengl_si_bin_output_create();
		outname = &bin_ptr[name_offset];
		if(*outname != '\0')
		{
			len = strlen(outname) + 1;
			output->name = xstrdup(outname);
			output->data_type = output_ptr->data_type;
			output->array_size = output_ptr->array_size;
			bin_ptr += (name_offset + len);
		}
		else
		{
			bin_ptr += (name_offset + 1);
			output->name = NULL;
			output->type = (enum opengl_si_bin_output_type_t) output_ptr->type;
			output->poffset = output_ptr->poffset;
			output->array_size = output_ptr->array_size;
		}
		list_add(lst, output);
	}
}

/* Structure in .info section */
static struct opengl_si_bin_info_t *opengl_si_si_bin_info_create()
{
	struct opengl_si_bin_info_t *info;

	/* Allocate */
	info = xcalloc(1, sizeof(struct opengl_si_bin_info_t));

	/* Return */	
	return info;
}

static void opengl_si_si_bin_info_free(struct opengl_si_bin_info_t *info)
{
	free(info);
}

static void opengl_si_si_bin_info_init_with_section(struct opengl_si_bin_info_t *info, struct elf_section_t *section)
{
	assert(!strcmp(section->name, ".info"));

	if (section->buffer.size != sizeof(struct opengl_si_bin_info_t))
		fatal("Section size(%d) doesn't match info structure(%d).",
			section->buffer.size, (int)sizeof(struct opengl_si_bin_info_t));
	else
		memcpy(info, section->buffer.ptr, sizeof(struct opengl_si_bin_info_t));
}

/* Structure in .usageinfo section */
static struct opengl_si_bin_usageinfo_t *opengl_si_bin_usageinfo_create()
{
	struct opengl_si_bin_usageinfo_t *usageinfo;

	/* Allocate */
	usageinfo = xcalloc(1, sizeof(struct opengl_si_bin_usageinfo_t));

	/* Return */
	return usageinfo;
}

static void opengl_si_bin_usageinfo_free(struct opengl_si_bin_usageinfo_t *usageinfo)
{
	free(usageinfo);
}

static void opengl_si_si_bin_usageinfo_init_with_section(struct opengl_si_bin_usageinfo_t *usageinfo, struct elf_section_t *section)
{
	assert(!strcmp(section->name, ".usageinfo"));

	/* FIXME: size doesn't match */
	// if (section->buffer.size != sizeof(struct opengl_si_bin_usageinfo_t))
	// 	fatal("Section size(%d) doesn't match usageinfo structure(%d).",
	// 		section->buffer.size, sizeof(struct opengl_si_bin_usageinfo_t));
	// else
		memcpy(usageinfo, section->buffer.ptr, sizeof(struct opengl_si_bin_usageinfo_t));
}


static struct opengl_si_enc_dict_vertex_shader_t *opengl_si_enc_dict_vertex_shader_create(struct opengl_si_shader_binary_t *parent)
{
	struct opengl_si_enc_dict_vertex_shader_t *vs;

	/* Allocate */
	vs = xcalloc(1, sizeof(struct opengl_si_enc_dict_vertex_shader_t));
	vs->parent = parent;
	parent->enc_dict = vs;
	vs->meta = opengl_si_bin_vertex_shader_metadata_create();
	vs->inputs = list_create();
	vs->outputs = list_create();
	vs->info = opengl_si_si_bin_info_create();
	vs->usageinfo = opengl_si_bin_usageinfo_create();

	/* Return */
	return vs;
}

static void opengl_si_bin_vertex_shader_free(struct opengl_si_enc_dict_vertex_shader_t *vs)
{
	struct opengl_si_bin_input_t *input;
	struct opengl_si_bin_output_t *output;
	int i;

	opengl_si_bin_vertex_shader_metadata_free(vs->meta);
	LIST_FOR_EACH(vs->inputs, i)
	{
		input = list_get(vs->inputs, i);
		opengl_si_bin_input_free(input);
	}
	list_free(vs->inputs);
	LIST_FOR_EACH(vs->outputs, i)
	{
		output = list_get(vs->outputs, i);
		opengl_si_bin_output_free(output);
	}
	list_free(vs->outputs);
	opengl_si_si_bin_info_free(vs->info);
	opengl_si_bin_usageinfo_free(vs->usageinfo);

	free(vs);

}

static void opengl_si_bin_vertex_shader_init(struct opengl_si_enc_dict_vertex_shader_t *vs)
{
	struct opengl_si_shader_binary_t *parent;
	struct elf_file_t *elf;
	struct elf_section_t *section;
	int i;

	/* Get parent */
	parent = vs->parent;
	assert(parent);
	assert(parent->shader_kind == OPENGL_SI_SHADER_VERTEX);

	elf = parent->elf;

	/* Initialize from sections */
	LIST_FOR_EACH(elf->section_list, i)
	{
		section = list_get(elf->section_list, i);
		if (!strcmp(section->name, ".text"))
			opengl_si_bin_vertex_shader_metadata_init_from_section(vs->meta, section);
		else if (!strcmp(section->name, ".inputs"))
			opengl_si_bin_inputs_init_from_section(vs->inputs, section);
		else if (!strcmp(section->name, ".outputs"))
			opengl_si_bin_outputs_init_from_section(vs->outputs, section);
		else if (!strcmp(section->name, ".info"))
			opengl_si_si_bin_info_init_with_section(vs->info, section);
		else if(!strcmp(section->name, ".usageinfo"))
			opengl_si_si_bin_usageinfo_init_with_section(vs->usageinfo, section);
	}
}

static struct opengl_si_enc_dict_pixel_shader_t *opengl_si_enc_dict_pixel_shader_create(struct opengl_si_shader_binary_t *parent)
{
	struct opengl_si_enc_dict_pixel_shader_t *ps;

	/* Allocate */
	ps = xcalloc(1, sizeof(struct opengl_si_enc_dict_pixel_shader_t));
	ps->parent = parent;
	parent->enc_dict = ps;
	ps->meta = opengl_si_bin_pixel_shader_metadata_create();
	ps->inputs = list_create();
	ps->outputs = list_create();
	ps->info = opengl_si_si_bin_info_create();
	ps->usageinfo = opengl_si_bin_usageinfo_create();

	/* Return */
	return ps;
}

static void opengl_si_bin_pixel_shader_free(struct opengl_si_enc_dict_pixel_shader_t *ps)
{
	struct opengl_si_bin_input_t *input;
	struct opengl_si_bin_output_t *output;
	int i;

	opengl_si_bin_pixel_shader_metadata_free(ps->meta);
	LIST_FOR_EACH(ps->inputs, i)
	{
		input = list_get(ps->inputs, i);
		opengl_si_bin_input_free(input);
	}
	list_free(ps->inputs);
	LIST_FOR_EACH(ps->outputs, i)
	{
		output = list_get(ps->outputs, i);
		opengl_si_bin_output_free(output);
	}
	list_free(ps->outputs);
	opengl_si_si_bin_info_free(ps->info);
	opengl_si_bin_usageinfo_free(ps->usageinfo);

	free(ps);

}

static void opengl_si_bin_pixel_shader_init(struct opengl_si_enc_dict_pixel_shader_t *ps)
{
	struct opengl_si_shader_binary_t *parent;
	struct elf_file_t *elf;
	struct elf_section_t *section;
	int i;

	/* Get parent */
	parent = ps->parent;
	assert(parent);
	assert(parent->shader_kind == OPENGL_SI_SHADER_PIXEL);

	elf = parent->elf;

	/* Initialize from sections */
	LIST_FOR_EACH(elf->section_list, i)
	{
		section = list_get(elf->section_list, i);
		if (!strcmp(section->name, ".text"))
			opengl_si_bin_pixel_shader_metadata_init_from_section(ps->meta, section);
		else if (!strcmp(section->name, ".inputs"))
			opengl_si_bin_inputs_init_from_section(ps->inputs, section);
		else if (!strcmp(section->name, ".outputs"))
			opengl_si_bin_outputs_init_from_section(ps->outputs, section);
		else if (!strcmp(section->name, ".info"))
			opengl_si_si_bin_info_init_with_section(ps->info, section);
		else if(!strcmp(section->name, ".usageinfo"))
			opengl_si_si_bin_usageinfo_init_with_section(ps->usageinfo, section);
	}

}

static struct list_t *opengl_si_shader_bin_list_create(struct elf_file_t *binary)
{
	struct list_t *lst;
	struct opengl_si_shader_binary_t *shader;
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
			shader = opengl_si_shader_binary_create(section->buffer.ptr + symbol->value, 
				symbol->size, 
				symbol->name);
			list_add(lst, shader);
		}
	}

	/* Return */
	return lst;
}

static void opengl_si_shader_bin_list_free(struct list_t *shaders)
{
	struct opengl_si_shader_binary_t *shdr;
	int i;

	/* Free */
	LIST_FOR_EACH(shaders, i)
	{
		shdr = list_get(shaders, i);
		opengl_si_shader_binary_free(shdr);
	}

	list_free(shaders);
}

/*
 * Public functions
 */

struct opengl_si_program_binary_t *opengl_si_program_binary_create(void *buffer_ptr, int size, char *name)
{
	struct opengl_si_program_binary_t *program_bin;

	/* Allocate */
	program_bin = xcalloc(1, sizeof(struct opengl_si_program_binary_t));

	/* Initialize */
	program_bin->name = xstrdup(name);
	program_bin->binary = elf_file_create_from_buffer(buffer_ptr, size, name);
	if (!program_bin->binary)
		fatal("Shader binary is not ELF formatted, please upgrade your AMD driver!");
	program_bin->shader_bins = opengl_si_shader_bin_list_create(program_bin->binary);

	/* Return */	
	return program_bin;
}

void opengl_si_program_binary_free(struct opengl_si_program_binary_t *program_bin)
{
	/* Free shader binary */
	free(program_bin->name);
	elf_file_free(program_bin->binary);
	opengl_si_shader_bin_list_free(program_bin->shader_bins);

	free(program_bin);
}

struct opengl_si_shader_binary_t *opengl_si_shader_binary_create(void *buffer, int size, char* name)
{
	struct opengl_si_shader_binary_t *shdr;

	/* Allocate */
	shdr = xcalloc(1, sizeof(struct opengl_si_shader_binary_t));

	/* Initialize */
	shdr->elf = elf_file_create_from_buffer(buffer, size, name);
	if (shdr->elf)
	{
		opengl_si_shader_binary_set(shdr);
		opengl_si_shader_binary_set_isa(shdr);
	}

	/* Return */
	return shdr;
}

void opengl_si_shader_binary_free(struct opengl_si_shader_binary_t *shdr)
{
	shdr->free_func(shdr->enc_dict);
	elf_file_free(shdr->elf);
	free(shdr->isa);
	free(shdr);
}

struct SIBinaryUserElement *opengl_si_bin_enc_user_element_create()
{
	struct SIBinaryUserElement *user_elem;

	/* Initialize */
	user_elem = xcalloc(1, sizeof(struct SIBinaryUserElement));
	
	/* Return */
	return user_elem;
}

void opengl_si_bin_enc_user_element_free(struct SIBinaryUserElement *user_elem)
{
	free(user_elem);
}

void opengl_si_shader_binary_debug_meta(struct opengl_si_shader_binary_t *shdr_bin)
{
	int i;
	struct opengl_si_enc_dict_vertex_shader_t *enc_vs;
	struct opengl_si_bin_vertex_shader_metadata_t *meta_vs;

	switch(shdr_bin->shader_kind)
	{

	case OPENGL_SI_SHADER_VERTEX:
	{
		enc_vs = (struct opengl_si_enc_dict_vertex_shader_t *)shdr_bin->enc_dict;
		meta_vs = enc_vs->meta;
		printf("-----------------------VS Data -------------------------\n");
		printf("Input Semantic Mappings\n");
		for (i = 0; i < meta_vs->numVsInSemantics; ++i)
		{
			printf(" [%d] %s, usageIdx %d, v[%d:%d]\n", i, 
				str_map_value(&enc_dict_semantic_input_type_map, meta_vs->vsInSemantics[i].usage), 
				meta_vs->vsInSemantics[i].usageIdx, meta_vs->vsInSemantics[i].dataVgpr, 
				meta_vs->vsInSemantics[i].dataVgpr + 3);
		}
		printf("Output Semantic Mappings\n");
		for (i = 0; i < meta_vs->numVsOutSemantics; ++i)
		{
			printf(" [%d] %s, usageIdx %d, paramIdx %d\n", i, 
				str_map_value(&enc_dict_semantic_output_type_map, meta_vs->vsOutSemantics[i].usage), 
				meta_vs->vsOutSemantics[i].usageIdx, meta_vs->vsOutSemantics[i].paramIdx);
		}
		printf("\n");
		printf("codeLenInByte\t= %d;Bytes\n", meta_vs->CodeLenInByte);
		printf("\n");
		printf("userElementCount\t= %d\n", meta_vs->u32UserElementCount);
		for (i = 0; i < meta_vs->u32UserElementCount; ++i)
		{
			printf(" userElements[%d]\t= %s, %d, s[%d,%d]\n", i, 
				str_map_value(&enc_dict_user_elements_type_map, meta_vs->pUserElement[i].dataClass), 
				meta_vs->pUserElement[i].apiSlot, meta_vs->pUserElement[i].startUserReg, 
				meta_vs->pUserElement[i].startUserReg + meta_vs->pUserElement[i].userRegCount  - 1);
		}
		printf("NumVgprs\t\t= %d\n", meta_vs->u32NumVgprs);
		printf("NumSgprs\t\t= %d\n", meta_vs->u32NumSgprs);
		break;
	}
	default:
		break;
	}
}

