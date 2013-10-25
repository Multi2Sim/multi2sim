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
#include <lib/util/misc.h>
#include <arch/southern-islands/emu/work-item.h>
#include <driver/opengl/si-pa.h>
#include <driver/glut/frame-buffer.h>

#include "sx.h"

/*
 * Private Functions
 */

static void SISXPositionCreate(SISX *self)
{
	int i;
	for (i = 0; i < SI_POS_COUNT; ++i)
		self->pos[i] = list_create();
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
		self->param[i] = list_create();
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

static void SISXMRTCreate(SISX *self)
{
	int i;

	for (i = 0; i < SI_MRT_COUNT; ++i)
	{
		self->mrt[i] = 	xcalloc(1, sizeof(SISXMRT));
	}
}

static void SISXMRTDestroy(SISX *self)
{
	int i;
	SISXMRT *mrt;

	for (i = 0; i < SI_MRT_COUNT; ++i)
	{
		mrt = self->mrt[i];
		if (mrt)
		{
			if (mrt->buffer)
				free(mrt->buffer);
			free(mrt);
		}
	}
}

static void SISXMRTReset(SISX *self)
{
	SISXMRTDestroy(self);
	SISXMRTCreate(self);
}

/* FIXME: make it static */
void SISXMRTSetPixel(SISXMRT *mrt, int x, int y, int color)
{
	/* Invalid color */
	if ((unsigned int) color > 0xffffff)
	{
		warning("%s: invalid pixel color", __FUNCTION__);
		return;
	}

	/* Invalid X coordinate */
	if (!IN_RANGE(x, 0, mrt->width - 1))
	{
		warning("%s: invalid X coordinate", __FUNCTION__);
		return;
	}

	/* Invalid Y coordinate */
	if (!IN_RANGE(y, 0, mrt->height - 1))
	{
		warning("%s: invalid Y coordinate", __FUNCTION__);
		return;
	}

	/* Set pixel */
	mrt->buffer[y * mrt->width + x] = color;
}

/*
 * Public Functions
 */

uint16_t Float32to16(float value)
{
	union Bits v, s;
	v.f = value;
	uint32_t sign = v.si & F_signN;
	v.si ^= sign;
	sign >>= F_shiftSign; // logical F_shift
	s.si = F_mulN;
	s.si = s.f * v.f; // correct subnormals
	v.si ^= (s.si ^ v.si) & -(F_minN > v.si);
	v.si ^= (F_infN ^ v.si) & -((F_infN > v.si) & (v.si > F_maxN));
	v.si ^= (F_nanN ^ v.si) & -((F_nanN > v.si) & (v.si > F_infN));
	v.ui >>= F_shift; // logical F_shift
	v.si ^= ((v.si - F_maxD) ^ v.si) & -(v.si > F_maxC);
	v.si ^= ((v.si - F_minD) ^ v.si) & -(v.si > F_subC);
	return v.ui | sign;
}

float Float16to32(uint16_t value)
{
	union Bits v;
	v.ui = value;
	int32_t sign = v.si & F_signC;
	v.si ^= sign;
	sign <<= F_shiftSign;
	v.si ^= ((v.si + F_minD) ^ v.si) & -(v.si > F_subC);
	v.si ^= ((v.si + F_maxD) ^ v.si) & -(v.si > F_maxC);
	union Bits s;
	s.si = F_mulC;
	s.f *= v.si;
	int32_t mask = -(F_norC > v.si);
	v.si <<= F_shift;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;
	return v.f;
}


void SISXCreate(SISX *self, SIEmu *emu)
{
	/* Initialize */
	self->emu = emu;
	SISXPositionCreate(self);
	SISXParamCreate(self);
	SISXMRTCreate(self);
}

void SISXDestroy(SISX *self)
{
	/* Free */
	SISXPositionDestroy(self);
	SISXParamDestroy(self);
	SISXMRTDestroy(self);
}

void SISXReset(SISX *self)
{
	/* Reset */
	SISXPosReset(self);
	SISXParamReset(self);
	SISXMRTReset(self);
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

void SISXExportMRT(SISX *self, unsigned int target, SIWorkItem *work_item, unsigned int compr_en, 
	SIInstReg x, SIInstReg y, SIInstReg z, SIInstReg w)
{
	// SISXMRT *mrt;
	union hfpack rg;
	union hfpack ba;
	float r;
	float g;
	float b;
	int color;

	// mrt = self->mrt[target];

	/* Calculate color */
	if (compr_en)
	{
		rg.as_uint32 = x.as_int;
		ba.as_uint32 = y.as_int;
		r = Float16to32(rg.as_f16f16.s0f);
		g = Float16to32(rg.as_f16f16.s1f);
		b = Float16to32(ba.as_f16f16.s0f);
	}
	else
	{
		r = x.as_float;
		g = y.as_float;
		b = z.as_float;
	}

	/* Map to range [0-255] */
	r *= 255;
	g *= 255;
	b *= 255;

	/* Compress to a 4 bytes int */
	color = (((int)(r) << 16) + ((int)(g) << 8) + (int)(b));
	// printf("%08x [%d, %d, %d]\n", color, (int)r, (int)g, (int)b);

	/* Export to MRT target */
	// SISXMRTSetPixel(mrt, work_item->vreg[16].as_int, work_item->vreg[17].as_int, color);

	/* FIXME */
	// printf("t%d:  [%d, %d]: r %f,  g %f, b %f\n", work_item->id, work_item->vreg[16].as_int, work_item->vreg[17].as_int, r, g, b);
	glut_frame_buffer_pixel(work_item->vreg[16].as_int, work_item->vreg[17].as_int, color);
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

void SISXMRTResize(SISXMRT *mrt, unsigned int width, unsigned int height)
{
	/* Invalid size */
	if (width < 1 || height < 1)
		fatal("%s: invalid size (width = %d, height = %d)\n",
			__FUNCTION__, width, height);

	/* If same size, just clear it. */
	if (mrt->width == width && mrt->height == height)
	{
		memset(mrt->buffer, 0, mrt->width *
			mrt->height * sizeof(int));
		return;
	}

	/* Free previous buffer */
	if (mrt->buffer)
		free(mrt->buffer);

	/* Store new size */
	mrt->buffer = xcalloc(width * height, sizeof(int));
	mrt->width = width;
	mrt->height = height;
}

void SISXMRTResizeAll(SISX *self, unsigned int width, unsigned int height)
{
	int i;

	for (i = 0; i < SI_MRT_COUNT; ++i)
		SISXMRTResize(self->mrt[i], width, height);
}

