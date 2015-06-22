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


#include <arch/southern-islands/asm/arg.h>
#include <lib/class/list.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <memory/memory.h>

#include "ndrange.h"
#include "sx.h"
#include "work-item.h"



/*
 * Class 'SINDRange'
 */

void SINDRangeCreate(SINDRange *self, SIEmu *emu)
{
	/* Initialize */
	self->emu = emu;
	self->id = emu->ndrange_count++;
	self->stage = STAGE_CL;

	self->arg_list = new(List);

	self->waiting_work_groups = list_create();
	self->running_work_groups = list_create();
	self->completed_work_groups = list_create();

	/* Set by driver */
	self->last_work_group_sent = 0;
}


void SINDRangeDestroy(SINDRange *self)
{
	/* Free initialization data which is dequeue from SPI module */
	if (self->ps_init_data)
		SISXPSInitDestroy(self->ps_init_data);

	/* Free instruction buffer */
	if (self->inst_buffer)
		free(self->inst_buffer);

	/* Free the work-group queues */
	list_free(self->waiting_work_groups);
	list_free(self->running_work_groups);
	list_free(self->completed_work_groups);

	/* Free the argument list */
	ListDeleteObjects(self->arg_list);
	delete(self->arg_list);
}


void SINDRangeSetupSize(SINDRange *self, unsigned int *global_size, 
	unsigned int *local_size, int work_dim)
{
	int i;

	/* Default value */
	self->global_size3[1] = 1;
	self->global_size3[2] = 1;
	self->local_size3[1] = 1;
	self->local_size3[2] = 1;
	self->work_dim = work_dim;

	/* Global work sizes */
	for (i = 0; i < work_dim; i++)
	{
		self->global_size3[i] = global_size[i];
	}
	self->global_size = self->global_size3[0] *
		self->global_size3[1] * self->global_size3[2];

	/* Local work sizes */
	for (i = 0; i < work_dim; i++)
	{
		self->local_size3[i] = local_size[i];
		if (self->local_size3[i] < 1)
		{
			fatal("%s: local work size must be greater than 0",
					__FUNCTION__);
		}
	}
	self->local_size = self->local_size3[0] * 
		self->local_size3[1] * self->local_size3[2];

	/* Check valid global/local sizes */
	if (self->global_size3[0] < 1 || self->global_size3[1] < 1
			|| self->global_size3[2] < 1)
		fatal("%s: invalid global size", __FUNCTION__);
	if (self->local_size3[0] < 1 || self->local_size3[1] < 1
			|| self->local_size3[2] < 1)
		fatal("%s: invalid local size", __FUNCTION__);

	/* Check divisibility of global by local sizes */
	if ((self->global_size3[0] % self->local_size3[0])
		|| (self->global_size3[1] % self->local_size3[1])
		|| (self->global_size3[2] % self->local_size3[2]))
	{
		fatal("%s: global work sizes must be multiples of local sizes",
				__FUNCTION__);
	}

	/* Calculate number of groups */
	for (i = 0; i < 3; i++)
	{
		self->group_count3[i] = self->global_size3[i] / 
			self->local_size3[i];
	}
	self->group_count = self->group_count3[0] * 
		self->group_count3[1] * self->group_count3[2];
}


void SINDRangeSetupInstMem(SINDRange *self, const char *buf,
	int size, unsigned int pc)
{
	/* Sanity */
	if (self->inst_buffer || self->inst_buffer_size)
		panic("%s: instruction buffer already set up", __FUNCTION__);
	if (!size || pc >= size)
		panic("%s: invalid value for size/pc", __FUNCTION__);

	/* Allocate memory buffer */
	assert(size);
	self->inst_buffer = xmalloc(size);
	self->inst_buffer_size = size;
	memcpy(self->inst_buffer, buf, size);
}

void SINDRangeSetupFSMem(SINDRange *self, void *buf, 
	int size, unsigned int pc)
{
	void *buffer;

	/* Sanity */
	if (self->fs_buffer_initialized || self->fs_buffer_size)
		panic("%s: fetch shader buffer already set up", __FUNCTION__);
	if (!size || pc >= size)
		panic("%s: invalid value for size/pc", __FUNCTION__);

	/* Append fetch shader to the end of shader instuctions */
	assert(size);
	buffer = xcalloc(1, self->inst_buffer_size + size);
	memcpy(buffer, self->inst_buffer, self->inst_buffer_size);
	memcpy(buffer + self->inst_buffer_size, buf, size);
	free(self->inst_buffer);

	/* Update state of instruction buffer */
	self->fs_buffer_size = size;
	self->fs_buffer_ptr = self->inst_buffer_size;
	self->fs_buffer_initialized = 1;
	self->inst_buffer = buffer;
	self->inst_buffer_size +=  size;
}

void SINDRangeSetupStage(SINDRange *self,
	enum si_ndrange_stage_t stage)
{
	self->stage = stage;
}

void SINDRangeSetupPSInitData(SINDRange *self,
	struct si_sx_ps_init_t *ps_init_data)
{
	self->ps_init_data = ps_init_data;
}

void SINDRangeInsertBufferIntoUAVTable(SINDRange *self,
	struct si_buffer_desc_t *buf_desc, unsigned int uav)
{
	SIEmu *emu = self->emu;

	assert(uav < SI_EMU_MAX_NUM_UAVS);
	assert(sizeof(*buf_desc) <= SI_EMU_UAV_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the UAV table */
	mem_write(emu->global_mem, self->uav_table +
		uav*SI_EMU_UAV_TABLE_ENTRY_SIZE, sizeof(*buf_desc),
		buf_desc);

	self->uav_table_entries[uav].valid = 1;
	self->uav_table_entries[uav].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
	self->uav_table_entries[uav].size = sizeof(*buf_desc);
}

void SINDRangeInsertBufferIntoVertexBufferTable(SINDRange *self,
	struct si_buffer_desc_t *buf_desc, unsigned int vertex_buffer)
{
	SIEmu *emu = self->emu;

	assert(vertex_buffer < SI_EMU_MAX_NUM_VERTEX_BUFFERS);
	assert(sizeof(*buf_desc) <= SI_EMU_VERTEX_BUFFER_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the Vertex Buffer table */
	mem_write(emu->global_mem, self->vertex_buffer_table +
		vertex_buffer*SI_EMU_VERTEX_BUFFER_TABLE_ENTRY_SIZE, sizeof(*buf_desc),
		buf_desc);

	self->vertex_buffer_table_entries[vertex_buffer].valid = 1;
	self->vertex_buffer_table_entries[vertex_buffer].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
	self->vertex_buffer_table_entries[vertex_buffer].size = sizeof(*buf_desc);
}


void SINDRangeInsertBufferIntoConstantBufferTable(SINDRange *self,
	struct si_buffer_desc_t *buf_desc, unsigned int const_buf_num)
{
	SIEmu *emu = self->emu;

	assert(const_buf_num < SI_EMU_MAX_NUM_CONST_BUFS);
	assert(sizeof(*buf_desc) <= SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the constant buffer table */
	mem_write(emu->global_mem, self->const_buf_table +
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(*buf_desc), buf_desc);

	self->const_buf_table_entries[const_buf_num].valid = 1;
	self->const_buf_table_entries[const_buf_num].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
	self->uav_table_entries[const_buf_num].size = sizeof(*buf_desc);
}

void SINDRangeInsertImageIntoUAVTable(SINDRange *self,
	struct si_image_desc_t *image_desc, unsigned int uav)
{
	SIEmu *emu = self->emu;

	assert(uav < SI_EMU_MAX_NUM_UAVS);
	assert(sizeof(*image_desc) <= SI_EMU_UAV_TABLE_ENTRY_SIZE);

	/* Write the buffer resource descriptor into the UAV table */
	mem_write(emu->global_mem, self->uav_table +
		uav*SI_EMU_UAV_TABLE_ENTRY_SIZE, sizeof(*image_desc),
		image_desc);

	self->uav_table_entries[uav].valid = 1;
	self->uav_table_entries[uav].kind = SI_TABLE_ENTRY_KIND_IMAGE_DESC;
	self->uav_table_entries[uav].size = sizeof(*image_desc);
}

void SINDRangeConstantBufferWrite(SINDRange *self, 
	int const_buf_num, int offset, void *pvalue, unsigned int size)
{
	SIEmu *emu = self->emu;

	unsigned int addr;

	struct si_buffer_desc_t buffer_desc;

	/* Sanity check */
	assert(const_buf_num < 2);
	assert(offset + size < SI_EMU_CONST_BUF_SIZE);

	mem_read(emu->global_mem, self->const_buf_table +
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(buffer_desc), &buffer_desc);

	addr = (unsigned int)buffer_desc.base_addr;
	addr += offset;

	/* Write */
	mem_write(emu->global_mem, addr, size, pvalue);
}

void SINDRangeConstantBufferRead(SINDRange *self, int const_buf_num,
		int offset, void *pvalue, unsigned int size)
{
	SIEmu *emu = self->emu;

	unsigned int addr;

	struct si_buffer_desc_t buffer_desc;

	/* Sanity check */
	assert(const_buf_num < 2);
	assert(offset+size < SI_EMU_CONST_BUF_SIZE);

	mem_read(emu->global_mem, self->const_buf_table +
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(buffer_desc), &buffer_desc);

	addr = (unsigned int)buffer_desc.base_addr;
	addr += offset;

	/* Read */
	mem_read(emu->global_mem, addr, size, pvalue);
}
