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
#include <driver/opengl/si-pa.h>

#include "sx.h"

/*
 * Public Functions
 */

void SISXCreate(SISX *self, SIEmu *emu)
{
	int i;

	/* Initialize */
	self->emu = emu;
	for (i = 0; i < SI_POS_COUNT; ++i)
	{
		self->pos[i] = list_create();
	}
	for (int i = 0; i < SI_PARAM_COUNT; ++i)
	{
		self->param[i] = list_create();
	}
}

void SISXDestroy(SISX *self)
{
	int i;
	int j;
	float *pos;
	float *param;

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
	for (int i = 0; i < SI_PARAM_COUNT; ++i)
	{
		LIST_FOR_EACH(self->param[i], j)
		{
			param = list_get(self->param[i], j);
			free(param);
		}
		list_free(self->param[i]);
	}
}

void SISXExportPosition(SISX *self, unsigned int target, unsigned int id, 
	unsigned int v0, unsigned int v1, unsigned int v2, unsigned int v3)
{
	struct list_t *vtx_lst;
	float *pos;

	pos = xcalloc(1, 4 * sizeof(float));

	pos[0] = (float)v0;
	pos[1] = (float)v1;
	pos[2] = (float)v2;
	pos[3] = (float)v3;

	vtx_lst = self->pos[target];
	list_insert(vtx_lst, id, pos);
}

