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


#include <arch/southern-islands/asm/input.h>
#include <arch/southern-islands/emu/opengl-bin-file.h> 
#include <arch/southern-islands/emu/ndrange.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <../runtime/include/GL/gl.h>
 
#include "opengl.h"
#include "si-shader.h"
#include "si-program.h"

struct list_t *opengl_si_shader_list;

/*
 * Private Functions
 */

static void opengl_si_create_buffer_desc(unsigned int base_addr,
	unsigned int size, int num_elems, 
	enum si_input_data_type_t data_type,
	struct si_buffer_desc_t *buffer_desc)
{
	int num_format;
	int data_format;
	int elem_size;

	/* Zero-out the buffer resource descriptor */
	assert(sizeof(struct si_buffer_desc_t) == 16);
	memset(buffer_desc, 0, sizeof(struct si_buffer_desc_t));

	num_format = SI_BUF_DESC_NUM_FMT_INVALID;
	data_format = SI_BUF_DESC_DATA_FMT_INVALID;

	switch (data_type)
	{

	case si_input_byte:
	case si_input_ubyte:

		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		switch (num_elems)
		{
		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_8;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_8_8;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_8_8_8_8;
			break;

		default:
			fatal("%s: invalid number of i8/u8 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 1 * num_elems;
		break;

	case si_input_short:
	case si_input_ushort:

		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		switch (num_elems)
		{

		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_16;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_16_16;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_16_16_16_16;
			break;

		default:
			fatal("%s: invalid number of i16/u16 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 2 * num_elems;
		break;

	case si_input_int:
	case si_input_uint:

		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		switch (num_elems)
		{

		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_32;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_32_32;
			break;

		case 3:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32_32;
			break;

		default:
			fatal("%s: invalid number of i32/u32 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 4 * num_elems;
		break;

	case si_input_hfloat:
	case si_input_float:

		num_format = SI_BUF_DESC_NUM_FMT_FLOAT;
		switch (num_elems)
		{
		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_32;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_32_32;
			break;

		case 3:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32_32;
			break;

		default:
			fatal("%s: invalid number of float elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 4 * num_elems;
		break;

	case si_input_int_2_10_10_10_rev:
	case si_input_uint_2_10_10_10_rev:

		num_format = SI_BUF_DESC_NUM_FMT_FLOAT;
		switch (num_elems)
		{
		case 1:
			data_format = 	SI_BUF_DESC_DATA_FMT_2_10_10_10;
			break;

		default:
			fatal("%s: invalid number of elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 4 * num_elems;
		break;


	default:
		fatal("%s: invalid data type for SI buffer (%d)",
			__FUNCTION__, data_type);
	}
	assert(num_format != SI_BUF_DESC_NUM_FMT_INVALID);
	assert(data_format != SI_BUF_DESC_DATA_FMT_INVALID);

	buffer_desc->base_addr = base_addr;
	buffer_desc->num_format = num_format;
	buffer_desc->data_format = data_format;
	buffer_desc->elem_size = elem_size;
	buffer_desc->num_records = size;

	return;

}

/*
 * Public Functions
 */

void opengl_si_shader_list_init(void)
{
	/* Already initialized */
	if (opengl_si_shader_list)
		return;

	/* Initialize and add one empty element */
	opengl_si_shader_list = list_create();
	list_add(opengl_si_shader_list, NULL);
}

void opengl_si_shader_list_done(void)
{
	int index;
	struct opengl_si_shader_t *shdr;

	/* Not initialized */
	if (!opengl_si_shader_list)
		return;

	/* Free list of Southern Islands programs */
	LIST_FOR_EACH(opengl_si_shader_list, index)
	{
		shdr = list_get(opengl_si_shader_list, index);
		if (shdr)
			opengl_si_shader_free(shdr);
	}
	list_free(opengl_si_shader_list);
}

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
	list_insert(opengl_si_shader_list, shader_id, shdr);

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
	shdr = list_get(opengl_si_shader_list, shader_id);

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

void opengl_si_shader_create_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange)
{
	/* Create constant buffer 0 */
	ndrange->cb0 = si_emu->video_mem_top;
	si_emu->video_mem_top += SI_EMU_CONST_BUF_0_SIZE;

	/* Create constant buffer 1 */
	ndrange->cb1 = si_emu->video_mem_top;
	si_emu->video_mem_top += SI_EMU_CONST_BUF_1_SIZE;
}

void opengl_si_shader_setup_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange)
{
	struct si_buffer_desc_t buffer_desc;

	unsigned int zero = 0;

	float f;

	opengl_si_create_buffer_desc(ndrange->cb0, SI_EMU_CONST_BUF_0_SIZE, 1,
		si_input_int, &buffer_desc);

	si_ndrange_insert_buffer_into_const_buf_table(ndrange, &buffer_desc, 0);

	opengl_si_create_buffer_desc(ndrange->cb1, SI_EMU_CONST_BUF_1_SIZE, 1,
		si_input_int, &buffer_desc);

	si_ndrange_insert_buffer_into_const_buf_table(ndrange, &buffer_desc, 1);

	/* Initialize constant buffers */

	/* CB0 bytes 0:15 */

	/* Global work size for the {x,y,z} dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 0, 
		&ndrange->global_size3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 4, 
		&ndrange->global_size3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 8, 
		&ndrange->global_size3[2], 4);

	/* Number of work dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 12, &ndrange->work_dim, 4);

	/* CB0 bytes 16:31 */

	/* Local work size for the {x,y,z} dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 16, 
		&ndrange->local_size3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 20, 
		&ndrange->local_size3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 24, 
		&ndrange->local_size3[2], 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 28, &zero, 4);

	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	si_ndrange_const_buf_write(ndrange, 0, 32, 
		&ndrange->group_count3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 36, 
		&ndrange->group_count3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 40, 
		&ndrange->group_count3[2], 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 44, &zero, 4);

	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is
	 * not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 56, &zero, 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 60, &zero, 4);

	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is
	 * not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	si_ndrange_const_buf_write(ndrange, 0, 72, &zero, 4);

	/* FIXME Pointer to location in global buffer where math library
	 * tables start. */

	/* CB0 bytes 80:95 */

	/* 0.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	si_ndrange_const_buf_write(ndrange, 0, 80, &f, 4);

	/* 0.5 as IEEE-32bit float - required for math library. */
	f = 0.5f;
	si_ndrange_const_buf_write(ndrange, 0, 84, &f, 4);

	/* 1.0 as IEEE-32bit float - required for math library. */
	f = 1.0f;
	si_ndrange_const_buf_write(ndrange, 0, 88, &f, 4);

	/* 2.0 as IEEE-32bit float - required for math library. */
	f = 2.0f;
	si_ndrange_const_buf_write(ndrange, 0, 92, &f, 4);

	/* CB0 bytes 96:111 */

	/* Global offset for the {x,y,z} dimension of the work_item spawn */
	si_ndrange_const_buf_write(ndrange, 0, 96, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 100, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 104, &zero, 4);

	/* Global single dimension flat offset: x * y * z */
	si_ndrange_const_buf_write(ndrange, 0, 108, &zero, 4);

	/* CB0 bytes 112:127 */

	/* Group offset for the {x,y,z} dimensions of the work_item spawn */
	si_ndrange_const_buf_write(ndrange, 0, 112, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 116, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 120, &zero, 4);

	/* Group single dimension flat offset, x * y * z */
	si_ndrange_const_buf_write(ndrange, 0, 124, &zero, 4);

	/* CB0 bytes 128:143 */

	/* FIXME Offset in the global buffer where data segment exists */
	/* FIXME Offset in buffer for printf support */
	/* FIXME Size of the printf buffer */
	
}

void opengl_si_shader_setup_ndrange_inputs(struct opengl_si_shader_t *shdr,
		struct si_ndrange_t *ndrange)
{
	struct si_input_t *input;
	struct si_buffer_desc_t buffer_desc;

	int index;

	/* Shader inputs */
	LIST_FOR_EACH(shdr->shader_bin->shader_enc_dict->input_list, index)
	{
		input = list_get(shdr->shader_bin->shader_enc_dict->input_list, index);
		assert(input);

		/* Check that input was set */
		if (!input->set)
			fatal("%s: shader %p: input #%d not set",
				__FUNCTION__, shdr, input->usage_index);

		/* Create descriptor for input */
		opengl_si_create_buffer_desc(
			input->device_ptr,
			input->size,
			input->num_elems,
			input->type, &buffer_desc);

		opengl_debug("\tinput created, device_ptr = %d, size = %d, num_elems = %d, type = %x\n", 
			input->device_ptr, input->size, input->num_elems, input->type);

		/* Add to Vertex Buffer table */
		si_ndrange_insert_buffer_into_vertex_buffer_table(
			ndrange, &buffer_desc,
			input->usage_index);
	}

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
