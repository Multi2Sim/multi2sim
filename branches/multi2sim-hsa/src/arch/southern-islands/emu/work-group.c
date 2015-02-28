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


#include <driver/opengl/si-spi.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <memory/memory.h>

#include "isa.h"
#include "ndrange.h"
#include "sx.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"

/*
 * Private Functions
 */


/* Parameter cache is LDS, which has parameter data from shader export module */
static void SIWorkGroupInitParamCache(SIWorkGroup *self)
{
	struct si_sx_ps_init_lds_t *lds;

	lds = self->ndrange->ps_init_data->lds;
	assert(lds);

	/* 
	 * Currently, NDRange is created per primitive(triangle) for Pixel Shader
	 * So just load lds data to the beginning of LDS module
	 */	 
	mem_write(self->lds_module, 0x0, lds->size, lds->data);
}


/*
 * Public Functions
 */

void SIWorkGroupCreate(SIWorkGroup *self, unsigned int id, SINDRange *ndrange)
{
	struct SIBinaryUserElement *user_elements;

	SIWavefront *wavefront;
	SIWorkItem *work_item;
	SIEmu *emu = ndrange->emu;
	struct si_sx_ps_init_meta_t *ps_init_meta;

	int i;
	int lid;
	int lidx, lidy, lidz;
	int tid;
	int user_element_count;
	int wavefront_id;
	int wavefront_offset;
	int work_item_id;
	int work_item_gidx_start;
	int work_item_gidy_start;
	int work_item_gidz_start;

	/* Number of work-items in work-group */
	unsigned int work_items_per_group = ndrange->local_size3[0] * 
		ndrange->local_size3[1] * ndrange->local_size3[2];
	assert(work_items_per_group > 0);

	/* Number of wavefronts in work-group */
	unsigned int wavefronts_per_group = (work_items_per_group + 
		(si_emu_wavefront_size - 1)) / si_emu_wavefront_size;
	assert(wavefronts_per_group > 0);

	/* Initialize */
	self->id = id;
	self->ndrange = ndrange;

	/* Create LDS */
	self->lds_module = mem_create();
	self->lds_module->safe = 0;

	/* Allocate pointers for work-items (will actually be created when
	 * wavefronts are created) */
	self->work_items = xcalloc(si_emu_wavefront_size * 
		wavefronts_per_group, sizeof(void *));
	self->wavefronts = xcalloc(wavefronts_per_group, sizeof(void *));
	self->wavefront_count = wavefronts_per_group;

	/* Allocate wavefronts and work-items */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(self, wavefront_id)
	{
		self->wavefronts[wavefront_id] = new(SIWavefront,
			self->id * wavefronts_per_group + wavefront_id,
			self);

		wavefront = self->wavefronts[wavefront_id];

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			wavefront_offset = wavefront_id * si_emu_wavefront_size;

			self->work_items[wavefront_offset+work_item_id] = 
				wavefront->work_items[work_item_id];
			self->work_items[wavefront_offset+work_item_id]->
				work_group = self;
		}
	}

	/* Initialize work-group and work-item metadata */
	self->id_3d[0] = id % ndrange->group_count3[0];
	self->id_3d[1] = (id / ndrange->group_count3[0]) % 
		ndrange->group_count3[1];
	self->id_3d[2] = id / (ndrange->group_count3[0] * 
		ndrange->group_count3[1]);

	/* Number of work-items in work-group */
	self->work_item_count = ndrange->local_size3[2] *
		ndrange->local_size3[1] * ndrange->local_size3[0];

	/* Global ID of work-item (0,0,0) within the work group */
	work_item_gidx_start = self->id_3d[0] * ndrange->local_size3[0];
	work_item_gidy_start = self->id_3d[1] * ndrange->local_size3[1];
	work_item_gidz_start = self->id_3d[2] * ndrange->local_size3[2];

	/* For NDRange created for Pixel Shader, we need to initialize LDS */
	switch(self->ndrange->stage)
	{
	case STAGE_PS:
	{
		/* Initialize LDS(Parameter Cache) */
		SIWorkGroupInitParamCache(self);
		break;
	}
	default:
		break;
	}

	/* Initialize work-item metadata */
	lid = 0;
	tid = self->id * work_items_per_group;
	for (lidz = 0; lidz < ndrange->local_size3[2]; lidz++)
	{
		for (lidy = 0; lidy < ndrange->local_size3[1]; lidy++)
		{
			for (lidx = 0; lidx < ndrange->local_size3[0]; lidx++)
			{
				work_item = self->work_items[lid];

				/* Global IDs */
				work_item->id_3d[0] = work_item_gidx_start + 
					lidx;
				work_item->id_3d[1] = work_item_gidy_start + 
					lidy;
				work_item->id_3d[2] = work_item_gidz_start + 
					lidz;
				work_item->id = tid;

				/* Local IDs */
				work_item->id_in_work_group_3d[0] = lidx;
				work_item->id_in_work_group_3d[1] = lidy;
				work_item->id_in_work_group_3d[2] = lidz;
				work_item->id_in_work_group = lid;


				/* First, last, and number of work-items 
				 * in wavefront */
				wavefront = work_item->wavefront;
				if (!wavefront->work_item_count)
				{
					wavefront->work_item_id_first = tid;
				}
				wavefront->work_item_id_last = tid;
				wavefront->work_item_count++;

				/* Next work-item */
				tid++;
				lid++;

				/* Initialize the execution mask */
				if (work_item->id_in_wavefront < 32)
				{
					wavefront->sreg[SI_EXEC].as_uint |= 
						1 << work_item->id_in_wavefront;
				}
				else if (work_item->id_in_wavefront < 64)
				{
					wavefront->sreg[SI_EXEC + 1].as_uint |= 
						1 << work_item->id_in_wavefront;
				}
				else 
				{
					fatal("%s: invalid work-item id (%d)",
						__FUNCTION__, 
						work_item->id_in_wavefront);
				}
			}
		}
	}

	/* Intialize wavefront state */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(self, wavefront_id)
	{
		wavefront = self->wavefronts[wavefront_id];

		/* Set PC */
		wavefront->pc = 0;

		switch(self->ndrange->stage)
		{

			case STAGE_CL:
			{
				/* Save work-group IDs in scalar registers */
				wavefront->sreg[ndrange->wg_id_sgpr].as_int =
					wavefront->work_group->id_3d[0];
				wavefront->sreg[ndrange->wg_id_sgpr + 1].as_int =
					wavefront->work_group->id_3d[1];
				wavefront->sreg[ndrange->wg_id_sgpr + 2].as_int =
					wavefront->work_group->id_3d[2];
				break;
			}
			case STAGE_PS:
			{
				/* Currently hard coded as only store 1 primitive data in LDS with offset 0x0 */
				wavefront->sreg[ndrange->wg_id_sgpr].as_int =0x0000000; 
			}
			default:
			{
				/* Save work-group IDs in scalar registers */
				wavefront->sreg[ndrange->wg_id_sgpr].as_int =
					wavefront->work_group->id_3d[0];
				wavefront->sreg[ndrange->wg_id_sgpr + 1].as_int =
					wavefront->work_group->id_3d[1];
				wavefront->sreg[ndrange->wg_id_sgpr + 2].as_int =
					wavefront->work_group->id_3d[2];
				break;				
			}
		}

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			/* Store work-item IDs in vector registers */
			work_item = wavefront->work_items[work_item_id];
			switch(self->ndrange->stage)
			{
				/* OpenCL convention */
				case STAGE_CL:
				{
					/* V0 */
					work_item->vreg[0].as_int = 
						work_item->id_in_work_group_3d[0];  
					/* V1 */
					work_item->vreg[1].as_int = 
						work_item->id_in_work_group_3d[1]; 
					/* V2 */
					work_item->vreg[2].as_int = 
						work_item->id_in_work_group_3d[2];
					break;					
				}
				/* Vertex shader initialization convention */
				case STAGE_VS:
				{
					/* VSes load VGPR0 with the thread's vertex index */
					work_item->vreg[0].as_int = work_item->id;
					break;
				}
				/* Pixel shader initialization convention */
				case STAGE_PS:
				{
					ps_init_meta = list_get(self->ndrange->ps_init_data->meta_list, work_item->id);

					/* PSes load barycentric coordinates to VGPRs */
					if (ps_init_meta)
					{
						work_item->vreg[0].as_float = ps_init_meta->brctrc_i; /* I pers sample */
						work_item->vreg[1].as_float = ps_init_meta->brctrc_j; /* J pers sample */
						work_item->vreg[2].as_float = 0.0f; /* I pers center */
						work_item->vreg[3].as_float = 0.0f; /* J pers center */
						work_item->vreg[4].as_float = 0.0f; /* I pers centroid */
						work_item->vreg[5].as_float = 0.0f; /* J pers centroid */
						work_item->vreg[6].as_float = 0.0f ; /* I/W */
						work_item->vreg[7].as_float = 0.0f; /* J/W */
						work_item->vreg[8].as_float = 0.0f; /* 1/W */
						work_item->vreg[9].as_float = 0.0f; /* I linear sample */
						work_item->vreg[10].as_float = 0.0f; /* J linear sample */
						work_item->vreg[11].as_float = 0.0f; /* I linear center */
						work_item->vreg[12].as_float = 0.0f; /* J linear center */
						work_item->vreg[13].as_float = 0.0f; /* I linear centroid */
						work_item->vreg[14].as_float = 0.0f; /* J linear centroid */
						work_item->vreg[15].as_float = 0.0f; /* Line stipple */
						work_item->vreg[16].as_int = ps_init_meta->x; /* X float */
						work_item->vreg[17].as_int = ps_init_meta->y; /* Y float */
						work_item->vreg[18].as_float = 0.0f; /* Z float */
						work_item->vreg[19].as_float = 0.0f; /* W float */
						work_item->vreg[20].as_float = 0.0f; /* Facedness */
						work_item->vreg[21].as_float = 0.0f; /* RTA, ISN, PT */
						work_item->vreg[22].as_float = 0.0f; /* Sample mask */
						work_item->vreg[23].as_float = 0.0f; /* X/Y fixed */
					}
					break;
				}
				/* Default is OpenCL convention */
				default:
				{
					/* V0 */
					work_item->vreg[0].as_int = 
						work_item->id_in_work_group_3d[0];  
					/* V1 */
					work_item->vreg[1].as_int = 
						work_item->id_in_work_group_3d[1]; 
					/* V2 */
					work_item->vreg[2].as_int = 
						work_item->id_in_work_group_3d[2];
					break;
				}
			}
		}

		/* Initialize sreg pointers to internal data structures */
		user_element_count = ndrange->userElementCount;
		user_elements = ndrange->userElements;
		for (i = 0; i < user_element_count; i++)
		{
			if (user_elements[i].dataClass == SIBinaryUserDataConstBuffer)
			{
				/* Store CB pointer in sregs */
				SIWavefrontInitSRegWithConstantBuffer(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount,
					user_elements[i].apiSlot);
			}
			else if (user_elements[i].dataClass == SIBinaryUserDataUAV)
			{
				/* Store UAV pointer in sregs */
				SIWavefrontInitSRegWithUAV(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount,
					user_elements[i].apiSlot);
			}
			else if (user_elements[i].dataClass ==
				SIBinaryUserDataConstBufferTable)
			{
				/* Store CB table in sregs */
				SIWavefrontInitSRegWithConstantBufferTable(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == SIBinaryUserDataUAVTable)
			{
				/* Store UAV table in sregs */
				SIWavefrontInitSRegWithUAVTable(
					wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			/* FIXME: PTR_VERTEX_BUFFER_TABLE doesn't match binary */
			else if (user_elements[i].dataClass == 21)
			{
				/* Store VB table in sregs */
				SIWavefrontInitSRegWithBufferTable(
					wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			/* FIXME: SUB_PTR_FETCH_SHADER doesn't match binary */
			else if (user_elements[i].dataClass == 16)
			{
				/* Store Fetch Shader pointer in sregs */
				SIWavefrontInitSRegWithFetchShader(
					wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == SIBinaryUserDataSampler)
			{
				/* Store sampler in sregs */
				assert(0);
			}
			else if (user_elements[i].dataClass ==
				SIBinaryUserDataPtrResourceTable)
			{
				/* Store resource table in sregs */
				assert(0);
			}
			else if (user_elements[i].dataClass ==
				SIBinaryUserDataInternalGlobalTable)
			{
				fatal("%s: PTR_INTERNAL_GLOBAL_TABLE not "
					"supported", __FUNCTION__);
			}
			else
			{
				fatal("%s: Unimplemented User Element: "
					"dataClass:%d", __FUNCTION__,
					user_elements[i].dataClass);
			}
		}
	}

	/* Statistics */
	emu->work_group_count++;
}


void SIWorkGroupDestroy(SIWorkGroup *self)
{
	SIWavefront *wavefront;

	int wavefront_id;
	int work_item_id; 

	/* Free wavefronts and work-items */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(self, wavefront_id)
	{
		wavefront = self->wavefronts[wavefront_id];

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
			delete(wavefront->work_items[work_item_id]);

		delete(wavefront->scalar_work_item);
		delete(wavefront);
	}
	free(self->wavefronts);
	free(self->work_items);

	/* Free LDS memory module */
	mem_free(self->lds_module);
}

