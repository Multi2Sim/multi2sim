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
#include <arch/southern-islands/emu/work-item.h>
#include <driver/opengl/si-pa.h>

#include "sx.h"

/*
 * Private Functions
 */

static void SISXPositionCreate(SISX *self)
{
	int i;
	for (i = 0; i < SI_POS_COUNT; ++i)
	{
		self->pos[i] = list_create();
	}
}

static void SISXPositionDestroy(SISX *self)
{
	int i;
	int j;
	float *pos;

	/* Free */
	for (i = 0; i < SI_POS_COUNT; ++i)
	{
		LIST_FOR_EACH(self->pos[i], j)
		{
			pos = list_get(self->pos[i], j);
			free(pos);
		}
		list_free(self->pos[i]);
	}

}

static void SISXPosReset(SISX *self)
{
	SISXPositionDestroy(self);
	SISXPositionCreate(self);
}

static void SISXParamCreate(SISX *self)
{
	int i;
	for (i = 0; i < SI_PARAM_COUNT; ++i)
	{
		self->param[i] = list_create();
	}
}

static void SISXParamDestroy(SISX *self)
{
	int i;
	int j;
	float *param;

	/* Free */
	for (i = 0; i < SI_PARAM_COUNT; ++i)
	{
		LIST_FOR_EACH(self->param[i], j)
		{
			param = list_get(self->param[i], j);
			free(param);
		}
		list_free(self->param[i]);
	}

}

static void SISXParamReset(SISX *self)
{
	SISXParamDestroy(self);
	SISXParamCreate(self);
}


/*
 * Public Functions
 */

void SISXCreate(SISX *self, SIEmu *emu)
{
	/* Initialize */
	self->emu = emu;
	SISXPositionCreate(self);
	SISXParamCreate(self);
}

void SISXDestroy(SISX *self)
{
	/* Free */
	SISXPositionDestroy(self);
	SISXParamDestroy(self);
}

void SISXReset(SISX *self)
{
	/* Reset */
	SISXPosReset(self);
	SISXParamReset(self);
}

void SISXExportPosition(SISX *self, unsigned int target, unsigned int id, 
	float x, float y, float z, float w)
{
	struct list_t *pos_lst;
	float *pos;

	pos = xcalloc(1, 4 * sizeof(float));

	pos[0] = (float)x;
	pos[1] = (float)y;
	pos[2] = (float)z;
	pos[3] = (float)w;

	pos_lst = self->pos[target];
	list_insert(pos_lst, id, pos);
}

void SISXExportParam(SISX *self, unsigned int target, unsigned int id, 
	float x, float y, float z, float w)
{
	struct list_t *param_lst;
	float *param;

	param = xcalloc(1, 4 * sizeof(float));

	param[0] = (float)x;
	param[1] = (float)y;
	param[2] = (float)z;
	param[3] = (float)w;

	param_lst = self->param[target];
	list_insert(param_lst, id, param);
}

struct si_sx_ps_init_lds_t *SISXPSInitLDSCreate(unsigned int attribute_count)
{
	struct si_sx_ps_init_lds_t *ps_lds;

	/* Allocate */
	ps_lds = xcalloc(1, sizeof(struct si_sx_ps_init_lds_t));
	ps_lds->data = xcalloc(1, attribute_count * 3 * 4 * sizeof(float));
	ps_lds->size = attribute_count * 3 * 4 * sizeof(float);

	/* Return */
	return ps_lds;
}

void SISXPSInitLDSDestroy(struct si_sx_ps_init_lds_t *ps_lds)
{
	free(ps_lds->data);
	free(ps_lds);
}

struct si_sx_ps_init_meta_t *SISXPSInitMetaCreate(int x, int y, float brctrc_i, float brctrc_j)
{
	struct si_sx_ps_init_meta_t *meta;

	/* Allocate */
	meta = xcalloc(1, sizeof(struct si_sx_ps_init_meta_t));
	meta->x = x;
	meta->y = y;
	meta->brctrc_i = brctrc_i;
	meta->brctrc_j = brctrc_j;

	/* Return */
	return meta;
}

void SISXPSInitMetaDestroy(struct si_sx_ps_init_meta_t *meta)
{
	free(meta);
}

struct si_sx_ps_init_t *SISXPSInitCreate(struct si_sx_ps_init_lds_t *lds, struct list_t *meta_list)
{
	struct si_sx_ps_init_t *ps_init;

	/* Allocate */
	ps_init = xcalloc(1, sizeof(struct si_sx_ps_init_t));
	ps_init->meta_list = meta_list;
	ps_init->lds = lds;

	/* Return */
	return ps_init;
}

void SISXPSInitDestroy(struct si_sx_ps_init_t *ps_init)
{
	int i;
	struct si_sx_ps_init_meta_t *meta;

	/* Free LDS */
	SISXPSInitLDSDestroy(ps_init->lds);

	/* Free pixel info from pixel info list */
	LIST_FOR_EACH(ps_init->meta_list, i)
	{
		meta = list_get(ps_init->meta_list, i);
		SISXPSInitMetaDestroy(meta);
	}

	/* Free list */
	list_free(ps_init->meta_list);

	/* Free */
	free(ps_init);
}
