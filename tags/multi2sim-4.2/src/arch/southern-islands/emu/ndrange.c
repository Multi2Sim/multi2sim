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
#include <mem-system/memory.h>

#include "ndrange.h"
#include "work-item.h"



/*
 * Public Functions
 */

struct si_ndrange_t *si_ndrange_create()
{
	struct si_ndrange_t *ndrange;

	/* Initialize */
	ndrange = xcalloc(1, sizeof(struct si_ndrange_t));
	ndrange->id = si_emu->ndrange_count++;

	/* Instruction histogram */
	if (si_emu_report_file)
		ndrange->inst_histogram = xcalloc(SI_INST_COUNT, 
			sizeof(unsigned int));

	/* Return */
	return ndrange;
}


void si_ndrange_free(struct si_ndrange_t *ndrange)
{
	/* Free instruction histogram */
	if (ndrange->inst_histogram)
		free(ndrange->inst_histogram);

	/* Free instruction buffer */
	if (ndrange->inst_buffer)
		free(ndrange->inst_buffer);

	/* Free ndrange */
	memset(ndrange, 0, sizeof(struct si_ndrange_t));
	free(ndrange);
}


void si_ndrange_setup_size(struct si_ndrange_t *ndrange,
	unsigned int *global_size, unsigned int *local_size, int work_dim)
{
	int i;

	/* Default value */
	ndrange->global_size3[1] = 1;
	ndrange->global_size3[2] = 1;
	ndrange->local_size3[1] = 1;
	ndrange->local_size3[2] = 1;
	ndrange->work_dim = work_dim;

	/* Global work sizes */
	for (i = 0; i < work_dim; i++)
	{
		ndrange->global_size3[i] = global_size[i];
	}
	ndrange->global_size = ndrange->global_size3[0] *
		ndrange->global_size3[1] * ndrange->global_size3[2];

	/* Local work sizes */
	for (i = 0; i < work_dim; i++)
	{
		ndrange->local_size3[i] = local_size[i];
		if (ndrange->local_size3[i] < 1)
		{
			fatal("%s: local work size must be greater than 0",
					__FUNCTION__);
		}
	}
	ndrange->local_size = ndrange->local_size3[0] * 
		ndrange->local_size3[1] * ndrange->local_size3[2];

	/* Check valid global/local sizes */
	if (ndrange->global_size3[0] < 1 || ndrange->global_size3[1] < 1
			|| ndrange->global_size3[2] < 1)
		fatal("%s: invalid global size", __FUNCTION__);
	if (ndrange->local_size3[0] < 1 || ndrange->local_size3[1] < 1
			|| ndrange->local_size3[2] < 1)
		fatal("%s: invalid local size", __FUNCTION__);

	/* Check divisibility of global by local sizes */
	if ((ndrange->global_size3[0] % ndrange->local_size3[0])
		|| (ndrange->global_size3[1] % ndrange->local_size3[1])
		|| (ndrange->global_size3[2] % ndrange->local_size3[2]))
	{
		fatal("%s: global work sizes must be multiples of local sizes",
				__FUNCTION__);
	}

	/* Calculate number of groups */
	for (i = 0; i < 3; i++)
	{
		ndrange->group_count3[i] = ndrange->global_size3[i] / 
			ndrange->local_size3[i];
	}
	ndrange->group_count = ndrange->group_count3[0] * 
		ndrange->group_count3[1] * ndrange->group_count3[2];
}


void si_ndrange_setup_inst_mem(struct si_ndrange_t *ndrange, void *buf, 
	int size, unsigned int pc)
{
	/* Sanity */
	if (ndrange->inst_buffer || ndrange->inst_buffer_size)
		panic("%s: instruction buffer already set up", __FUNCTION__);
	if (!size || pc >= size)
		panic("%s: invalid value for size/pc", __FUNCTION__);

	/* Allocate memory buffer */
	assert(size);
	ndrange->inst_buffer = xmalloc(size);
	ndrange->inst_buffer_size = size;
	memcpy(ndrange->inst_buffer, buf, size);
}

void si_ndrange_setup_fs_mem(struct si_ndrange_t *ndrange, void *buf, 
	int size, unsigned int pc)
{
	void *buffer;

	/* Sanity */
	if (ndrange->fs_buffer_initialized || ndrange->fs_buffer_size)
		panic("%s: fetch shader buffer already set up", __FUNCTION__);
	if (!size || pc >= size)
		panic("%s: invalid value for size/pc", __FUNCTION__);

	/* Append fetch shader to the end of instuction memory */
	assert(size);
	buffer = xcalloc(1, ndrange->inst_buffer_size + size);
	memcpy(buffer, ndrange->inst_buffer, ndrange->inst_buffer_size);
	memcpy(buffer + ndrange->inst_buffer_size, buf, size);
	ndrange->fs_buffer_size = size;
	ndrange->fs_buffer_initialized = 1;
	free(ndrange->inst_buffer);
	ndrange->inst_buffer = buffer;
}

void si_ndrange_insert_buffer_into_uav_table(struct si_ndrange_t *ndrange,
	struct si_buffer_desc_t *buf_desc, unsigned int uav)
{
	assert(uav < SI_EMU_MAX_NUM_UAVS);
	assert(sizeof(*buf_desc) <= SI_EMU_UAV_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the UAV table */
	mem_write(si_emu->global_mem, ndrange->uav_table +
		uav*SI_EMU_UAV_TABLE_ENTRY_SIZE, sizeof(*buf_desc),
		buf_desc);

	ndrange->uav_table_entries[uav].valid = 1;
	ndrange->uav_table_entries[uav].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
	ndrange->uav_table_entries[uav].size = sizeof(*buf_desc);
}

void si_ndrange_insert_buffer_into_vertex_buffer_table(struct si_ndrange_t *ndrange,
	struct si_buffer_desc_t *buf_desc, unsigned int vertex_buffer)
{
	assert(vertex_buffer < SI_EMU_MAX_NUM_VERTEX_BUFFERS);
	assert(sizeof(*buf_desc) <= SI_EMU_VERTEX_BUFFER_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the Vertex Buffer table */
	mem_write(si_emu->global_mem, ndrange->vertex_buffer_table +
		vertex_buffer*SI_EMU_VERTEX_BUFFER_TABLE_ENTRY_SIZE, sizeof(*buf_desc),
		buf_desc);

	ndrange->vertex_buffer_table_entries[vertex_buffer].valid = 1;
	ndrange->vertex_buffer_table_entries[vertex_buffer].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
	ndrange->vertex_buffer_table_entries[vertex_buffer].size = sizeof(*buf_desc);
}


void si_ndrange_insert_buffer_into_const_buf_table(struct si_ndrange_t *ndrange,
	struct si_buffer_desc_t *buf_desc, unsigned int const_buf_num)
{
	assert(const_buf_num < SI_EMU_MAX_NUM_CONST_BUFS);
	assert(sizeof(*buf_desc) <= SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the constant buffer table */
	mem_write(si_emu->global_mem, ndrange->const_buf_table +
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(*buf_desc), buf_desc);

	ndrange->const_buf_table_entries[const_buf_num].valid = 1;
	ndrange->const_buf_table_entries[const_buf_num].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
	ndrange->uav_table_entries[const_buf_num].size = sizeof(*buf_desc);
}

void si_ndrange_insert_image_into_uav_table(struct si_ndrange_t *ndrange,
	struct si_image_desc_t *image_desc, unsigned int uav)
{
	assert(uav < SI_EMU_MAX_NUM_UAVS);
	assert(sizeof(*image_desc) <= SI_EMU_UAV_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the UAV table */
	mem_write(si_emu->global_mem, ndrange->uav_table +
		uav*SI_EMU_UAV_TABLE_ENTRY_SIZE, sizeof(*image_desc),
		image_desc);

	ndrange->uav_table_entries[uav].valid = 1;
	ndrange->uav_table_entries[uav].kind = SI_TABLE_ENTRY_KIND_IMAGE_DESC;
	ndrange->uav_table_entries[uav].size = sizeof(*image_desc);
}

void si_ndrange_const_buf_write(struct si_ndrange_t *ndrange, 
	int const_buf_num, int offset, void *pvalue, unsigned int size)
{
	unsigned int addr;

	struct si_buffer_desc_t buffer_desc;

	/* Sanity check */
	assert(const_buf_num < 2);
	if (const_buf_num == 0)
	{
		assert(offset + size < SI_EMU_CONST_BUF_0_SIZE);
	}
	else if (const_buf_num == 1)
	{
		assert(offset + size < SI_EMU_CONST_BUF_1_SIZE);
	}

	mem_read(si_emu->global_mem, ndrange->const_buf_table + 
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(buffer_desc), &buffer_desc);

	addr = (unsigned int)buffer_desc.base_addr;
	addr += offset;

	/* Write */
	mem_write(si_emu->global_mem, addr, size, pvalue);
}

void si_ndrange_const_buf_read(struct si_ndrange_t *ndrange, int const_buf_num, 	int offset, void *pvalue, unsigned int size)
{
	unsigned int addr;

	struct si_buffer_desc_t buffer_desc;

	/* Sanity check */
	assert(const_buf_num < 2);
	if (const_buf_num == 0)
	{
		assert(offset + size < SI_EMU_CONST_BUF_0_SIZE);
	}
	else if (const_buf_num == 1)
	{
		assert(offset + size < SI_EMU_CONST_BUF_1_SIZE);
	}

	mem_read(si_emu->global_mem, ndrange->const_buf_table + 
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(buffer_desc), &buffer_desc);

	addr = (unsigned int)buffer_desc.base_addr;
	addr += offset;

	/* Read */
	mem_read(si_emu->global_mem, addr, size, pvalue);
}
