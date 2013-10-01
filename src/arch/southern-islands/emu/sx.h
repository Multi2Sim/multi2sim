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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_SHADER_EXPORT_H
#define ARCH_SOUTHERN_ISLANDS_EMU_SHADER_EXPORT_H


#include <lib/class/class.h>

#include "emu.h"

#define SI_POS_COUNT 4
#define SI_PARAM_COUNT 32

struct list_t;
typedef enum 
{
	SISXCL = 0,	/* OpenCL is the default */
	SISXVS,
	SISXGS,
	SISXHS,
	SISXDS,
	SISXPS,
	SISXCS
} SISXStage;

typedef struct 
{
	float brctrc_i;
	float brctrc_j;
} SISXPSInitMeta;

CLASS_BEGIN(SISX, Object)

	/* Emulator */
	SIEmu *emu;

	/* Lists contain data from export instructions */
	struct list_t *pos[SI_POS_COUNT];
	struct list_t *param[SI_PARAM_COUNT];

	/* Stage indicate which OpenGL stage in use */
	SISXStage stage;

	/* List contain metadata to initialize Pixel Shader */
	struct list_t *ps_init_meta;

CLASS_END(SISX)

void SISXCreate(SISX *self, SIEmu *emu);
void SISXDestroy(SISX *self);
void SISXReset(SISX *self);

void SISXStageSet(SISX *self, SISXStage stage);

void SISXExportPosition(SISX *self, unsigned int target, unsigned int id, 
	float x, float y, float z, float w);
void SISXExportParam(SISX *self, unsigned int target, unsigned int id, 
	float x, float y, float z, float w);
void SISXPSInitMetaAdd(SISX *self, float brctrc_i, float brctrc_j);

#endif
