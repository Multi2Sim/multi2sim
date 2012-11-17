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

#include "opengl-bin-file.h"




/*
 * Private Functions
 */


/* Find ELF magic bytes in buffer */
static void *search_elf_magic(void *ptr_buffer)
{
	const char ELF_magic[4]= {0x7F, 0x45, 0x4C, 0x46};

	if (!ptr_buffer)
		fatal("Invalid buffer!");

	while (ptr_buffer)
	{
		if (!memcmp(ptr_buffer, ELF_magic, sizeof(ELF_magic)))
			return ptr_buffer;
		ptr_buffer++;
	}
	return NULL;
}


/* Find all ELF magic locations in a buffer & store the location offset in a list */
static struct list_t *elf_file_list_create(void *ptr_buffer, size_t buf_size)
{
	const char ELF_magic[4] = { 0x7F, 0x45, 0x4C, 0x46};
	struct list_t *elf_file_list;
	void *ptr_copy;

	int *offset;
	int i;

	elf_file_list = list_create();

	ptr_copy = ptr_buffer;
	for (i = 0; i < buf_size - sizeof(ELF_magic); i++)
	{
		if (memcmp(ptr_copy, ELF_magic, sizeof(ELF_magic)) == 0)
		{
			offset = xcalloc(1, sizeof(int));
			*offset = (int)(ptr_copy - ptr_buffer);
			list_add(elf_file_list, offset);			
		}
		ptr_copy++;
	}

	return elf_file_list;
}


static void elf_file_list_free(struct list_t * elf_file_list)
{
	while (list_count(elf_file_list))
		free(list_remove_at(elf_file_list, 0));
	list_free(elf_file_list);
}


/* Given a buffer and buffer size, find out whether the buffer content is an external ELF or not. 
 * Return 0 as Yes and -1 as No */
static int is_external_elf(void *ptr_buffer, int buf_size)
{
	struct elf_file_t *elf_file;
	int i;
	struct elf_section_t *elf_section;

	elf_file = elf_file_create_from_buffer(ptr_buffer, buf_size, NULL);

	if (list_count(elf_file->section_list) != 5)
	{
		elf_file_free(elf_file);
		return -1;
	}
	else
	{
		for (i = 0; i < list_count(elf_file->section_list); i++)
		{
			elf_section = list_get(elf_file->section_list, i);
			if(!strcmp(elf_section->name,"binary"))
			{
				elf_file_free(elf_file);
				return 0;		
			}
		}
		elf_file_free(elf_file);
		return -1;
	}
	
	return -1;
}


/* Given a buffer and buffer size, create list that contains offsets of external ELF magic bytes in the buffer */
static struct list_t *external_elf_file_list_create(void *ptr_buffer, size_t buf_size)
{
	struct list_t *elf_file_list_external;
	int i;
	int elf_file_size;
	int *offset;

	elf_file_list_external = elf_file_list_create(ptr_buffer, buf_size);

	/* Remove internal ELF from list */
	for (i = 0; i < list_count(elf_file_list_external); i++)
	{
		offset = list_get(elf_file_list_external, i);
		elf_file_size = buf_size - *offset;

		if (is_external_elf(ptr_buffer + *offset, elf_file_size) != 0)
			free(list_remove_at(elf_file_list_external, i));
	}

	return elf_file_list_external;
}


static void external_elf_file_list_free(struct list_t *elf_external_file_list)
{
	while(list_count(elf_external_file_list))
		free(list_remove_at(elf_external_file_list,0));
	elf_file_list_free(elf_external_file_list);
}


/* Given a buffer and buffer size, a list contains all external ELF magic bytes offset and an index, and the file name loaded into the buffer
 * Create a corresponding ELF file object and return it */
static struct elf_file_t *external_elf_file_create(void *ptr_buffer, int buf_size, struct list_t *elf_offset_list, int elf_file_index, char *name)
{
	struct elf_file_t *external_elf_file;
	int *external_elf_offset;
	void *external_elf_base;
	int external_elf_size;

	/* Create external shader object */
	external_elf_offset = list_get(elf_offset_list,elf_file_index);
	external_elf_base = ptr_buffer + *external_elf_offset;
	external_elf_size = buf_size - *external_elf_offset;

	external_elf_file = elf_file_create_from_buffer(external_elf_base, external_elf_size, name);

	return external_elf_file;
}


static void external_elf_file_free(struct elf_file_t *external_elf_file)
{
	elf_file_free(external_elf_file);
}


/* Create internal ELF file object from an external ELF file object */
static struct elf_file_t *internal_elf_file_create(struct elf_file_t *external_elf_file)
{
	struct elf_section_t *external_section;
	void *section_buf;

	struct elf_file_t *internal_elf_file;
	int internal_elf_offset;
	void *internal_elf_base;
	int internal_elf_size;

	int i;

	/* Create internal shader object */
	for (i = 0; i < list_count(external_elf_file->section_list); i++)
	{
		/* Get section */
		external_section = list_get(external_elf_file->section_list, i);

		/* Internal ELF in the section with name 'binary' */
		if (!strcmp(external_section->name,"binary"))
		{

			elf_buffer_seek(&external_elf_file->buffer, external_section->header->sh_offset);
			section_buf = xcalloc(1, external_section->header->sh_size);
			memcpy(section_buf, external_elf_file->buffer.ptr + external_elf_file->buffer.pos, external_section->header->sh_size);

			internal_elf_offset = (int)(search_elf_magic(section_buf) - section_buf);
			internal_elf_base = section_buf + internal_elf_offset;
			internal_elf_size = external_section->header->sh_size - internal_elf_offset;

			internal_elf_file = elf_file_create_from_buffer(internal_elf_base, internal_elf_size, external_section->name);

			free(section_buf);
			break;
		}
	}

	return internal_elf_file;
}


static void internal_elf_file_free(struct elf_file_t *internal_elf_file)
{
	elf_file_free(internal_elf_file);
}


/* Get offset of ISAs based on the type of shader */
static int get_isa_offset(struct evg_opengl_shader_t *opengl_shader)
{
	int isa_offset;

	switch (opengl_shader->shader_kind)
	{
		case EVG_OPENGL_SHADER_VERTEX:
		{
			isa_offset = 4204;
			break;
		}
		case EVG_OPENGL_SHADER_FRAGMENT:
		{
			isa_offset = 2844;
			break;
		}
		case EVG_OPENGL_SHADER_GEOMETRY:
		{
			isa_offset = 3912;
			break;
		}
		default:
			isa_offset = 0;
	}

	return isa_offset;
}


/* Set 'isa_buffer' element for a shader object, the shader object must have the 'shader_kind' element set first */
static int amd_opengl_shader_set_isa_buffer(struct evg_opengl_shader_t *opengl_shader)
{
	struct elf_section_t *internal_section;
	int isa_offset;

	int i;

	if (!&opengl_shader->isa_buffer)
		fatal("%s: shader error", __FUNCTION__);	

		/* Initialize ISA buffer */
		if (opengl_shader->internal_elf_file)
		{
			for (i = 0; i < list_count(opengl_shader->internal_elf_file->section_list); i++)
			{
				internal_section = list_get(opengl_shader->internal_elf_file->section_list, i);

				/* ISAs in '.text' section */
				if (!strcmp(internal_section->name,".text"))
				{					
					isa_offset = get_isa_offset(opengl_shader);
					elf_buffer_seek(&opengl_shader->internal_elf_file->buffer, internal_section->header->sh_offset);
					
					opengl_shader->isa_buffer.ptr  = opengl_shader->internal_elf_file->buffer.ptr + opengl_shader->internal_elf_file->buffer.pos + isa_offset;
					opengl_shader->isa_buffer.size = opengl_shader->internal_elf_file->buffer.size - isa_offset;
					opengl_shader->isa_buffer.pos  = 0;
					return 1;
				}
			}

		}
		else
			fatal("No internal ELF for this shader!");

		return 0;
}


/* FIXME: NEED TO CONFIRM */
static int amd_opengl_shader_set_shader_kind(struct evg_opengl_shader_t *opengl_shader)
{
	struct elf_file_t *internal_elf_file;
	struct elf_file_t *external_elf_file;

	internal_elf_file = opengl_shader->internal_elf_file;
	external_elf_file = opengl_shader->external_elf_file;

	if (internal_elf_file->header->e_flags == 0x1 && external_elf_file->header->e_flags == 0x1)
	{
		opengl_shader->shader_kind = EVG_OPENGL_SHADER_FRAGMENT;
		return 1;
	}
	else if (internal_elf_file->header->e_flags == 0x2 && external_elf_file->header->e_flags == 0x2)
	{
		opengl_shader->shader_kind = EVG_OPENGL_SHADER_VERTEX;
		return 1;
	}
	else if (internal_elf_file->header->e_flags == 0x3 && external_elf_file->header->e_flags == 0x3)
	{
		opengl_shader->shader_kind = EVG_OPENGL_SHADER_GEOMETRY;
		return 1;
	}
	else if (internal_elf_file->header->e_flags == 0x5 && external_elf_file->header->e_flags == 0x5)
	{
		opengl_shader->shader_kind = EVG_OPENGL_SHADER_CONTROL;
		return 1;
	}
	else if (internal_elf_file->header->e_flags == 0x6 && external_elf_file->header->e_flags == 0x6)
	{
		opengl_shader->shader_kind = EVG_OPENGL_SHADER_EVALUATION;
		return 1;
	}
	else
	{
		fatal("Unknown shader kind!\n");
	}

	return 1;
}


/* Given a buffer and buffer size, a list contains all external ELF magic bytes offset and an index, and the file name loaded into the buffer
 * Create an corresponding evg_opengl_shader_t object and return it */
static struct evg_opengl_shader_t *amd_opengl_shader_create_from_buffer(void *ptr, int size, struct list_t *elf_offset_list, int elf_file_index, char *name)
{
	struct evg_opengl_shader_t *opengl_shader;

	/* Create shader object */
	opengl_shader = xcalloc(1,sizeof(struct evg_opengl_shader_t));

	opengl_shader->external_elf_file = external_elf_file_create(ptr, size, elf_offset_list, elf_file_index, name);
	opengl_shader->internal_elf_file = internal_elf_file_create(opengl_shader->external_elf_file);
	if(!amd_opengl_shader_set_shader_kind(opengl_shader))
		fatal("Set Shader kind fail!");
	if(!amd_opengl_shader_set_isa_buffer(opengl_shader))
		fatal("Set ISA info fail!");

	return opengl_shader;
}


static void amd_opengl_shader_free(struct evg_opengl_shader_t *opengl_shader)
{
	external_elf_file_free(opengl_shader->external_elf_file);
	internal_elf_file_free(opengl_shader->internal_elf_file);
	free(opengl_shader);
}




/*
 * Public functions
 */

/* Create evg_opengl_bin_file_t object based on a file buffer, the corresponding buffer size and the name of the file */
struct evg_opengl_bin_file_t *evg_opengl_bin_file_create(void *ptr, int size, char *name)
{
	struct evg_opengl_bin_file_t *bin_file;
	struct evg_opengl_shader_t *opengl_shader;

	struct list_t *elf_file_list;

	int i;

	/* Get binary file name */
	bin_file = xcalloc(1, sizeof(struct evg_opengl_bin_file_t));
	bin_file->name = xstrdup(name);

	/* Create elf file list contain all external ELF offset in the buffer */
	elf_file_list = external_elf_file_list_create(ptr, size);

	/* Initialize shader list */
	bin_file->shader_list = list_create();

	/* Find external/internal ELF files associated with a shader and insert into shader list */
	for (i = 0; i < list_count(elf_file_list); ++i)
	{
		/* Initialize shader */
		opengl_shader = amd_opengl_shader_create_from_buffer(ptr, size, elf_file_list, i, name);
		
		/* Add shader to shader list */
		list_add(bin_file->shader_list, opengl_shader);
	}


	/* Free elf file list */
	external_elf_file_list_free(elf_file_list);

	return bin_file;
}

void evg_opengl_bin_file_free(struct evg_opengl_bin_file_t *bin_file)
{
	/* Free shader list */
	while (list_count(bin_file->shader_list))
		amd_opengl_shader_free(list_remove_at(bin_file->shader_list, 0));
	list_free(bin_file->shader_list);

	/* Free rest */
	free(bin_file->name);
	free(bin_file);
}
