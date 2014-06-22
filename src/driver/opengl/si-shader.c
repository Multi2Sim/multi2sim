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
#include <arch/southern-islands/asm/opengl-bin-file.h> 
#include <arch/southern-islands/emu/ndrange.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <../runtime/include/GL/gl.h>
#include <memory/memory.h>
#include <memory/mmu.h>

 
#include "opengl.h"
#include "si-shader.h"
#include "si-program.h"

/*
 * Private Functions
 */

/* Buffer descriptor will be loaded into 4 successive SGPRs */
static void opengl_si_create_buffer_desc(unsigned int base_addr,
	unsigned int size, int num_elems, 
	SIInputDataType data_type,
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

	case SIInputByte:
	case SIInputUbyte:

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

	case SIInputShort:
	case SIInputUshort:

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

	case SIInputInt:
	case SIInputUint:

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

	case SIInputHfloat:
	case SIInputFloat:

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

	case SIInputInt2101010Rev:
	case SIInputUint2101010Rev:

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
	buffer_desc->stride = elem_size;
	buffer_desc->num_format = num_format;
	buffer_desc->data_format = data_format;
	buffer_desc->elem_size = elem_size;
	buffer_desc->num_records = size;

	/* Return */
	return;
}

static void opengl_si_shader_set_inputs(struct opengl_si_shader_t *shdr)
{
	int i;
	int input_count;
	SIInput *input;
	struct opengl_si_enc_dict_vertex_shader_t *enc_vs;
	struct opengl_si_enc_dict_pixel_shader_t *enc_ps;

	switch(shdr->shader_kind)
	{
	case OPENGL_SI_SHADER_VERTEX:
		/* Create input and add to input list */
		enc_vs = (struct opengl_si_enc_dict_vertex_shader_t *)shdr->bin->enc_dict;
		input_count = enc_vs->meta->numVsInSemantics;
		for (i = 0; i < input_count; ++i)
		{
			input = new(SIInput);
			SIInputSetUsageIndex(input, enc_vs->meta->vsInSemantics[i].usageIdx);
			list_insert(shdr->input_list, i, input);
		}
		break;
	case OPENGL_SI_SHADER_PIXEL:
		/* Create input and add to input list */
		enc_ps = (struct opengl_si_enc_dict_pixel_shader_t *)shdr->bin->enc_dict;
		input_count = enc_ps->meta->numPsInSemantics;
		for (i = 0; i < input_count; ++i)
		{
			input = new(SIInput);
			SIInputSetUsageIndex(input, enc_ps->meta->psInSemantics[i].usageIdx);
			list_insert(shdr->input_list, i, input);
		}
		break;	
	default:
		break;
	}
}

static void opengl_si_shader_init_from_program(struct opengl_si_program_t *program, 
	struct list_t *shdr_lst, 
	unsigned int shader_id)
{
	struct list_t *shdr_bin_lst;
	struct opengl_si_shader_t *shdr;
	struct opengl_si_program_binary_t *prog_bin;
	struct opengl_si_shader_binary_t *shdr_bin;
	int i;

	/* Get shader object */
	shdr = list_get(shdr_lst, shader_id);

	/* Initialize */
	shdr->program = program;
	prog_bin = program->program_bin;
	shdr_bin_lst = prog_bin->shader_bins;
	
	/* 
	 * Initialized program contains a list of shader binary,
	 * Shaders created by driver should be initialized with
	 * the same type of shader binary from the list.
	 */
	LIST_FOR_EACH(shdr_bin_lst, i)
	{
		shdr_bin = list_get(shdr_bin_lst, i);
		if (shdr_bin->shader_kind == shdr->shader_kind)
		{
			shdr->bin = shdr_bin;
			shdr_bin->parent = shdr;
			opengl_si_shader_set_inputs(shdr);
		}
	}
}

/*
 * Public Functions
 */

struct opengl_si_shader_t *opengl_si_shader_create(
	struct list_t *shdr_lst, struct opengl_si_program_t *program,
	unsigned int shader_id, unsigned int shader_kind)
{
	struct opengl_si_shader_t *shdr;

	/* Allocate */
	shdr = xcalloc(1, sizeof(struct opengl_si_shader_t));

	/* Setup shader type for later mapping */
	switch(shader_kind)
	{
	case GL_VERTEX_SHADER:
		shdr->shader_kind = OPENGL_SI_SHADER_VERTEX;
		break;
	case GL_FRAGMENT_SHADER:
		shdr->shader_kind = OPENGL_SI_SHADER_PIXEL;
		break;
	case GL_GEOMETRY_SHADER:
		shdr->shader_kind = OPENGL_SI_SHADER_GEOMETRY;
		break;
	case GL_TESS_CONTROL_SHADER:
		shdr->shader_kind = OPENGL_SI_SHADER_HULL;
		break;
	case GL_TESS_EVALUATION_SHADER:
		shdr->shader_kind = OPENGL_SI_SHADER_DOMAIN;
		break;
	default:
		shdr->shader_kind = OPENGL_SI_SHADER_INVALID;
		break;
	}

	/* Program it belongs to */
	shdr->program = program;
	
	/* Create input list, element will be created at opengl_si_shader_binary_set_inputs() */
	shdr->input_list = list_create();

	/* Add to shader list, shader id is the index */
	list_insert(shdr_lst, shader_id, shdr);

	opengl_si_shader_init_from_program(program, shdr_lst, shader_id);

	/* Return */
	return shdr;

}

void opengl_si_shader_free(struct opengl_si_shader_t *shdr)
{
	SIInput *input;
	int index;

	/* Free inputs */
	LIST_FOR_EACH(shdr->input_list, index)
	{
		input = list_get(shdr->input_list, index);
		delete(input);
	}
	/* Free list */
	list_free(shdr->input_list);

	/* Free */
	free(shdr);
}

void opengl_si_shader_create_ndrange_constant_buffers(SINDRange *ndrange, 
	MMU *gpu_mmu)
{
	SIEmu *emu = ndrange->emu;

	if (gpu_mmu)
	{
		/* Allocate starting from nearest page boundary */
		if (emu->video_mem_top % gpu_mmu->page_mask)
		{
			emu->video_mem_top += gpu_mmu->page_size -
				(emu->video_mem_top & gpu_mmu->page_mask);
		}
	}

	/* Map new pages */
	mem_map(emu->video_mem, emu->video_mem_top, SI_EMU_TOTAL_CONST_BUF_SIZE,
		mem_access_read | mem_access_write);

	opengl_debug("\t%u bytes of device memory allocated at " 
		"0x%x for SI constant buffers\n", SI_EMU_TOTAL_CONST_BUF_SIZE,
		emu->video_mem_top);


	/* Create constant buffers */
	ndrange->cb_start = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_TOTAL_CONST_BUF_SIZE;
}

void opengl_si_shader_setup_ndrange_constant_buffers(
	SINDRange *ndrange)
{
	struct si_buffer_desc_t buffer_desc;

	unsigned int zero = 0;

	float f;

	/* Constant buffer 0 */
	opengl_si_create_buffer_desc(ndrange->cb_start, SI_EMU_CONST_BUF_SIZE, 
		1, SIInputInt, &buffer_desc);

	SINDRangeInsertBufferIntoConstantBufferTable(ndrange, &buffer_desc, 0);

	/* Constant buffer 1 */
	opengl_si_create_buffer_desc(
		ndrange->cb_start + 1*SI_EMU_CONST_BUF_SIZE, 
		SI_EMU_CONST_BUF_SIZE, 1, SIInputInt, &buffer_desc);

	SINDRangeInsertBufferIntoConstantBufferTable(ndrange, &buffer_desc, 1);

	/* Initialize constant buffer 0 */

	/* CB0 bytes 0:15 */

	/* Global work size for the {x,y,z} dimensions */
	SINDRangeConstantBufferWrite(ndrange, 0, 0, 
		&ndrange->global_size3[0], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 4, 
		&ndrange->global_size3[1], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 8, 
		&ndrange->global_size3[2], 4);

	/* Number of work dimensions */
	SINDRangeConstantBufferWrite(ndrange, 0, 12, &ndrange->work_dim, 4);

	/* CB0 bytes 16:31 */

	/* Local work size for the {x,y,z} dimensions */
	SINDRangeConstantBufferWrite(ndrange, 0, 16, 
		&ndrange->local_size3[0], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 20, 
		&ndrange->local_size3[1], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 24, 
		&ndrange->local_size3[2], 4);

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 28, &zero, 4);

	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	SINDRangeConstantBufferWrite(ndrange, 0, 32, 
		&ndrange->group_count3[0], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 36, 
		&ndrange->group_count3[1], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 40, 
		&ndrange->group_count3[2], 4);

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 44, &zero, 4);

	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is
	 * not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 56, &zero, 4);

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 60, &zero, 4);

	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is
	 * not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	SINDRangeConstantBufferWrite(ndrange, 0, 72, &zero, 4);

	/* FIXME Pointer to location in global buffer where math library
	 * tables start. */

	/* CB0 bytes 80:95 */

	/* 0.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	SINDRangeConstantBufferWrite(ndrange, 0, 80, &f, 4);

	/* 0.5 as IEEE-32bit float - required for math library. */
	f = 0.5f;
	SINDRangeConstantBufferWrite(ndrange, 0, 84, &f, 4);

	/* 1.0 as IEEE-32bit float - required for math library. */
	f = 1.0f;
	SINDRangeConstantBufferWrite(ndrange, 0, 88, &f, 4);

	/* 2.0 as IEEE-32bit float - required for math library. */
	f = 2.0f;
	SINDRangeConstantBufferWrite(ndrange, 0, 92, &f, 4);

	/* CB0 bytes 96:111 */

	/* Global offset for the {x,y,z} dimension of the work_item spawn */
	SINDRangeConstantBufferWrite(ndrange, 0, 96, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 100, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 104, &zero, 4);

	/* Global single dimension flat offset: x * y * z */
	SINDRangeConstantBufferWrite(ndrange, 0, 108, &zero, 4);

	/* CB0 bytes 112:127 */

	/* Group offset for the {x,y,z} dimensions of the work_item spawn */
	SINDRangeConstantBufferWrite(ndrange, 0, 112, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 116, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 120, &zero, 4);

	/* Group single dimension flat offset, x * y * z */
	SINDRangeConstantBufferWrite(ndrange, 0, 124, &zero, 4);

	/* CB0 bytes 128:143 */

	/* FIXME Offset in the global buffer where data segment exists */
	/* FIXME Offset in buffer for printf support */
	/* FIXME Size of the printf buffer */
}

void opengl_si_shader_setup_ndrange_inputs(struct opengl_si_shader_t *shdr,
		SINDRange *ndrange)
{
	SIInput *input;
	struct si_buffer_desc_t buffer_desc;

	int index;

	/* Shader inputs */
	LIST_FOR_EACH(shdr->input_list, index)
	{
		input = list_get(shdr->input_list, index);
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

		opengl_debug("\tinput[%d] usage_index = %d, device_ptr = %x, size = %d, num_elems = %d, type = %x\n", 
			index, input->usage_index, input->device_ptr, input->size, input->num_elems, input->type);

		/* Add to Vertex Buffer table */
		SINDRangeInsertBufferIntoVertexBufferTable(
			ndrange, &buffer_desc,
			input->usage_index);
	}

}

void opengl_si_shader_debug_ndrange_state(struct opengl_si_shader_t *shader, 
	SINDRange *ndrange)
{

}

void opengl_si_shader_create_ndrange_tables(SINDRange *ndrange, MMU *gpu_mmu)
{
	SIEmu *emu = ndrange->emu;
	unsigned int size_of_tables;

	size_of_tables = SI_EMU_CONST_BUF_TABLE_SIZE + 
		SI_EMU_RESOURCE_TABLE_SIZE + 
		SI_EMU_VERTEX_BUFFER_TABLE_SIZE  +
		SI_EMU_UAV_TABLE_SIZE;

	if (gpu_mmu)
	{
		/* Allocate starting from nearest page boundary */
		if (emu->video_mem_top % gpu_mmu->page_mask)
		{
			emu->video_mem_top += gpu_mmu->page_size -
				(emu->video_mem_top & gpu_mmu->page_mask);
		}

	}
	/* Map new pages */
	mem_map(emu->video_mem, emu->video_mem_top, size_of_tables,
		mem_access_read | mem_access_write);

	opengl_debug("\t%u bytes of device memory allocated at " 
		"0x%x for SI internal tables\n", size_of_tables,
		emu->video_mem_top);

	/* Setup internal tables */
	ndrange->const_buf_table = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_CONST_BUF_TABLE_SIZE;
	ndrange->resource_table = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_RESOURCE_TABLE_SIZE;
	ndrange->uav_table = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_UAV_TABLE_SIZE;
	ndrange->vertex_buffer_table = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_VERTEX_BUFFER_TABLE_SIZE;

	return;
}
