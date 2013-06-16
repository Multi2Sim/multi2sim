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

#include <unistd.h>
#include <stdio.h>

#include <arch/southern-islands/emu/opengl-bin-file.h> 
#include <arch/southern-islands/emu/ndrange.h> 
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <../runtime/include/GL/gl.h>
 
#include "si-shader.h"
#include "si-program.h"

struct list_t *opengl_si_hader_list;

void opengl_si_shader_list_init(void)
{
	/* Already initialized */
	if (opengl_si_hader_list)
		return;

	/* Initialize and add one empty element */
	opengl_si_hader_list = list_create();
	list_add(opengl_si_hader_list, NULL);
}

void opengl_si_shader_list_done(void)
{
	int index;
	struct opengl_si_shader_t *shdr;

	/* Not initialized */
	if (!opengl_si_hader_list)
		return;

	/* Free list of Southern Islands programs */
	LIST_FOR_EACH(opengl_si_hader_list, index)
	{
		shdr = list_get(opengl_si_hader_list, index);
		if (shdr)
			opengl_si_shader_free(shdr);
	}
	list_free(opengl_si_hader_list);
}

/*
 * Public Functions
 */

struct opengl_si_shader_t *opengl_si_shader_create(
	unsigned int shader_id, unsigned int shader_kind)
{
	struct opengl_si_shader_t *shdr;

	opengl_si_shader_list_init();

	/* Allocate */
	shdr = xcalloc(1, sizeof(struct opengl_si_shader_t));

	/* Setup shader type for later mapping */
	switch(shader_kind)
	{
	
	case GL_VERTEX_SHADER:
		shdr->shader_kind = SI_OPENGL_SHADER_VERTEX;
		break;
	case GL_FRAGMENT_SHADER:
		shdr->shader_kind = SI_OPENGL_SHADER_FRAGMENT;
		break;
	case GL_GEOMETRY_SHADER:
		shdr->shader_kind = SI_OPENGL_SHADER_GEOMETRY;
		break;
	case GL_TESS_CONTROL_SHADER:
		shdr->shader_kind = SI_OPENGL_SHADER_CONTROL;
		break;
	case GL_TESS_EVALUATION_SHADER:
		shdr->shader_kind = SI_OPENGL_SHADER_CONTROL;
		break;
	default:
		shdr->shader_kind = SI_OPENGL_SHADER_INVALID;
		break;
	}

	/* Add to shader list, shader id is the index */
	list_insert(opengl_si_hader_list, shader_id, shdr);

	/* Return */
	return shdr;

}

void opengl_si_shader_free(struct opengl_si_shader_t *shdr)
{
	/* Free */
	free(shdr);
}

void opengl_si_shader_init( struct opengl_si_program_t *program, unsigned int shader_id)
{
	struct list_t *shaders_list;
	struct opengl_si_shader_t *shdr;
	struct si_opengl_program_binary_t *prog_bin;
	struct si_opengl_shader_binary_t *shdr_bin;
	int i;

	/* Get shader object */
	shdr = list_get(opengl_si_hader_list, shader_id);

	/* Initialize */
	shdr->program = program;
	prog_bin = program->program_bin;
	shaders_list = prog_bin->shaders;
	
	/* FIXME: is it true that an OpenGL program binary can only contain 1 shader of each kind */
	LIST_FOR_EACH(shaders_list, i)
	{
		shdr_bin = list_get(shaders_list, i);
		if (shdr_bin->shader_kind == shdr->shader_kind)
			shdr->shader_bin = shdr_bin;
	}
}



void opengl_si_shader_setup_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange)
{
	
}

void opengl_si_shader_debug_ndrange_state(struct opengl_si_shader_t *shader, 
	struct si_ndrange_t *ndrange)
{

}

void opengl_si_shader_create_ndrange_tables(struct si_ndrange_t *ndrange)
{
	/* Setup internal tables */
	ndrange->const_buf_table = si_emu->video_mem_top;
	si_emu->video_mem_top += SI_EMU_CONST_BUF_TABLE_SIZE;
	ndrange->resource_table = si_emu->video_mem_top;
	si_emu->video_mem_top += SI_EMU_RESOURCE_TABLE_SIZE;
	ndrange->uav_table = si_emu->video_mem_top;
	si_emu->video_mem_top += SI_EMU_UAV_TABLE_SIZE;
	ndrange->vertex_buffer_table = si_emu->video_mem_top;
	si_emu->video_mem_top += SI_EMU_VERTEX_BUFFER_TABLE_SIZE;

	return;
}

void opengl_si_shader_create_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange)
{

}
