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

#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/emu/sx.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "si-pa.h"
#include "si-sc.h"
#include "si-shader.h"
#include "si-spi.h"

/*
 * Private Functions
 */

/* Basically make a copy of pixel info list, but with less data */
static struct list_t *opengl_spi_meta_list_create_from_pixel_info_list(struct list_t *pixel_info_list)
{
	struct list_t *meta_list;
	struct si_sx_ps_init_meta_t *meta;
	struct opengl_sc_pixel_info_t *pixel_info;
	int i;

	meta_list = list_create();

	LIST_FOR_EACH(pixel_info_list, i)
	{
		pixel_info = list_get(pixel_info_list, i);
		meta = SISXPSInitMetaCreate(pixel_info->pos[0], pixel_info->pos[1], 
			pixel_info->brctrc_i, pixel_info->brctrc_j);
		list_add(meta_list, meta);
	}

	/* Return */
	return meta_list;
}

/* Prepare P0 P10 P20 so we can load to LDS for interpolation in Pixel Shader */
static struct list_t *SISpiPSInitLDSListCreate(struct list_t **param)
{
	struct list_t *param_lst;
	struct list_t *lds_repo;
	int attribute_count = 0;
	struct si_sx_ps_init_lds_t *ps_lds;
	float *p_0;
	float *p_1;
	float *p_2;
	int i, j;

	/* Count parameter slots in use */
	for (i = 0; i < SI_PARAM_COUNT; ++i)
	{
		param_lst = param[i];
		if (list_count(param_lst))
			attribute_count += 1;
	}

	/* Create LDS list */
	lds_repo = list_create();

	/* 
	 * LDS data size is always attribute_count * 12dwords 
	 * The layout in LDS is:
	 * P0: X Y Z W, P10: X Y Z W, P20: X Y Z W ------ Attribute 0, LDS base
	 * P0: X Y Z W, P10: X Y Z W, P20: X Y Z W ------ Attribute 1, LDS base + 12 dwords
	 */
	for (i = 0; i < attribute_count; ++i)
	{
		param_lst = param[i];
		ps_lds = SISXPSInitLDSCreate(attribute_count);
		for (j = 0; j < list_count(param_lst) / 3; ++j)
		{

			p_0 = list_get(param_lst, 3 * j);
			p_1 = list_get(param_lst, 3 * j + 1);
			p_2 = list_get(param_lst, 3 * j + 2);

			/* P0, Attribute i */
			ps_lds->data[12 * i + 0] = p_0[0];
			ps_lds->data[12 * i + 1] = p_0[1];
			ps_lds->data[12 * i + 2] = p_0[2];
			ps_lds->data[12 * i + 3] = p_0[3];

			/* P10, Attribute i */
			ps_lds->data[12 * i + 4] = p_1[0] - p_0[0];
			ps_lds->data[12 * i + 5] = p_1[1] - p_0[1];
			ps_lds->data[12 * i + 6] = p_1[2] - p_0[2];
			ps_lds->data[12 * i + 7] = p_1[3] - p_0[3];

			/* P20, Attribute i */
			ps_lds->data[12 * i + 8] = p_2[0] - p_0[0];
			ps_lds->data[12 * i + 9] = p_2[1] - p_0[1];
			ps_lds->data[12 * i + 10] = p_2[2] - p_0[2];
			ps_lds->data[12 * i + 11] = p_2[3] - p_0[3];
		}
		list_add(lds_repo, ps_lds);
	}

	/* Return */
	return lds_repo;
}

static void SISpiPSInitLDSListFree(struct list_t *lds_repo)
{
	struct si_sx_ps_init_lds_t *ps_lds;
	int i;

	LIST_FOR_EACH(lds_repo, i)
	{
		ps_lds = list_get(lds_repo, i);
		SISXPSInitLDSDestroy(ps_lds);
	}
	list_free(lds_repo);
}

/* FIXME: currently only handle pos[0] */
static struct list_t *SISpiPSInitMetaListsCreate(struct list_t **pos_lsts, struct opengl_pa_viewport_t *vwpt, struct opengl_depth_buffer_t *db)
{
	struct list_t *pos_sub_lst;
	struct list_t *pxl_info_lst;
	struct list_t *meta_lsts;
	struct list_t *meta_sub_lst;
	struct opengl_pa_primitive_t *prmtv;
	struct opengl_pa_triangle_t *triangle;	
	int i;

	pos_sub_lst = pos_lsts[0];
	meta_lsts = list_create();

	/* Primitive has a list inside */
	prmtv = opengl_pa_primitives_create(OPENGL_PA_TRIANGLES, pos_sub_lst, vwpt);
	LIST_FOR_EACH(prmtv->list, i)
	{
		triangle = list_get(prmtv->list, i);

		/* Rasterizer creates one pixel info list per triangle */
		pxl_info_lst = opengl_sc_rast_triangle_gen_pixel_info(triangle, db);

		/* Create one meta info list per triangle */
		meta_sub_lst = opengl_spi_meta_list_create_from_pixel_info_list(pxl_info_lst);

		/* Discard pixel info list */
		opengl_sc_rast_triangle_done(pxl_info_lst);

		/* Add sub lists to main list if not empty */
		if (list_count(meta_sub_lst))
			list_add(meta_lsts, meta_sub_lst);
		else
			list_free(meta_sub_lst);

	}
	/* Free */
	opengl_pa_primitives_free(prmtv);

	/* Return */
	return meta_lsts;
}

static void SISpiPSInitMetaListsFree(struct list_t *meta_list_repo)
{
	struct list_t *meta_list;
	struct si_sx_ps_init_meta_t *meta;
	int i;
	int j;

	LIST_FOR_EACH(meta_list_repo, i)
	{
		meta_list = list_get(meta_list_repo, i);
		LIST_FOR_EACH(meta_list, j)
		{
			meta = list_get(meta_list, j);
			SISXPSInitMetaDestroy(meta);
		}
		list_free(meta_list);
	}
	list_free(meta_list_repo);
}

/*
 * Public Functions
 */

struct list_t *SISpiPSNDRangesCreate(SISX *sx, enum opengl_pa_primitive_mode_t mode, 
	struct opengl_pa_viewport_t *vwpt, struct opengl_depth_buffer_t *db,
	struct opengl_si_shader_t *pixel_shader)
{
	SINDRange *ndrange;
	struct list_t *ndrange_repo;
	struct list_t *lds_repo;
	struct list_t *meta_list_repo; /* This repo contains several sub lists */
	struct list_t *meta_list; /* Elements have type si_sx_ps_init_meta_t */
	struct si_sx_ps_init_lds_t *lds;
	struct si_sx_ps_init_t *ps_init;

	unsigned int global_work_size[3];
	unsigned int local_work_size[3];
	int work_dim;

	unsigned int work_group_start[3];
	unsigned int work_group_count[3];
	unsigned int total_num_groups;
	long work_group_id;

	struct elf_buffer_t *elf_buffer;
	int user_element_count;
	struct SIBinaryUserElement *user_elements;
	struct opengl_si_enc_dict_pixel_shader_t *ps_enc;

	int i, j;
	int w_i, w_j, w_k;

	ndrange_repo = list_create();

	/* FIXME: Currently only accept triangle */
	assert(mode == OPENGL_PA_TRIANGLES);

	/* Create LDS data repository for NDRange */
	lds_repo = SISpiPSInitLDSListCreate(sx->param);

	/* Create meta data repository for NDRange */
	meta_list_repo = SISpiPSInitMetaListsCreate(sx->pos, vwpt, db);

	/* Should have same amount of elements, otherwise discard */
	if(list_count(meta_list_repo) != list_count(lds_repo))
	{
		SISpiPSInitLDSListFree(lds_repo);
		SISpiPSInitMetaListsFree(meta_list_repo);
		return ndrange_repo;
	}

	/* If meta data is empty, return an empty NDRange repo */
	if (!list_count((struct list_t *)list_get(meta_list_repo, 0)))
	{
		/* Free prvious created repo */
		SISpiPSInitLDSListFree(lds_repo);
		SISpiPSInitMetaListsFree(meta_list_repo);
		return ndrange_repo;		
	}

	for (i = 0; i < list_count(meta_list_repo); ++i)
	{
		/* Create init data for each Ndrange */
		lds = list_dequeue(lds_repo);
		meta_list = list_dequeue(meta_list_repo);
		ps_init = SISXPSInitCreate(lds, meta_list);

		/* Create NDRange */
		ndrange = new(SINDRange, sx->emu);

		/* Setup init data */
		SINDRangeSetupPSInitData(ndrange, ps_init);

		/* Setup NDRange stage */
		SINDRangeSetupStage(ndrange, STAGE_PS);

		/* Set dimensions */
		global_work_size[0] = list_count(meta_list);
		local_work_size[0] = global_work_size[0];
		work_group_count[0] = global_work_size[0] / local_work_size[0];
		work_dim = 1;

		/* Unused dimensions */
		for (j = work_dim; j < 3; j++)
		{
			global_work_size[j] = 1;
			local_work_size[j] = 1;
			work_group_count[j] = global_work_size[j] / local_work_size[j];
		}

		SINDRangeSetupSize(ndrange, global_work_size, local_work_size, work_dim);

		/* Setup workgroup */
		for (j = 0; j < 3; j++)
		{
			work_group_start[j] = 0;
		}
		total_num_groups = work_group_count[2] * work_group_count[1] * 
			work_group_count[0];
		assert(total_num_groups <= 16 -
			list_count(ndrange->waiting_work_groups));

		/* Receive work groups (add them to the waiting queue) */
		for (w_i = work_group_start[2]; w_i < work_group_start[2] + work_group_count[2]; w_i++)
		{
			for (w_j = work_group_start[1]; w_j < work_group_start[1] + work_group_count[1]; w_j++)
			{
				for (w_k = work_group_start[0]; w_k < work_group_start[0] + work_group_count[0]; w_k++)
				{
					work_group_id = (w_i * work_group_count[1] * 
						work_group_count[0]) + (w_j * 
						work_group_count[0]) + w_k;

					list_enqueue(ndrange->waiting_work_groups, 
						(void*)work_group_id);
				}
			}
		}
		
		/* Instruction buffer */
		elf_buffer = pixel_shader->bin->isa;
		if (!elf_buffer->size)
			fatal("%s: cannot load shader code", __FUNCTION__);
		SINDRangeSetupInstMem(ndrange, elf_buffer->ptr, 
			elf_buffer->size, 0);

		/* Some metadata*/
		ps_enc = pixel_shader->bin->enc_dict;
		ndrange->num_sgpr_used = ps_enc->meta->u32NumSgprs;
		ndrange->num_vgpr_used = ps_enc->meta->u32NumVgprs;
		ndrange->wg_id_sgpr = ps_enc->meta->spiShaderPgmRsrc2Ps.user_sgpr;
		
		/* Copy user elements from shader to ND-Range */
		user_element_count = ps_enc->meta->u32UserElementCount;
		user_elements = ps_enc->meta->pUserElement;
		ndrange->userElementCount = user_element_count;
		for (i = 0; i < user_element_count; i++)
		{
			ndrange->userElements[i] = user_elements[i];
		}

		/* Add to NDrange list */
		list_add(ndrange_repo, ndrange);
	}

	/* Free */
	assert(!list_count(lds_repo)); /* Should be empty as all elments are consumed by NDRanges */
	list_free(lds_repo);

	assert(!list_count(meta_list_repo)); /* Should be empty as all elments are consumed by NDRanges */
	list_free(meta_list_repo);

	/* Return */
	return ndrange_repo;
}

void SISpiPSNDRangesDestroy(struct list_t *ndrange_list)
{
	assert(!list_count(ndrange_list));
	list_free(ndrange_list);
}