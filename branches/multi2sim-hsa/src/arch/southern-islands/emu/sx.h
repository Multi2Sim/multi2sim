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
#include <stdint.h>
#include <src/arch/southern-islands/asm/Wrapper.h>
#include "emu.h"

#define SI_POS_COUNT 4
#define SI_PARAM_COUNT 32
#define SI_MRT_COUNT 8

/* 
 * Float32 <-> Float16
 * Reference: http://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion 
 */

union Bits
{
	float f;
	int32_t si;
	uint32_t ui;
};

#define F_shift 13
#define F_shiftSign 16

#define F_infN 0x7F800000 // flt32 infinity
#define F_maxN 0x477FE000 // max flt16 normal as a flt32
#define F_minN 0x38800000 // min flt16 normal as a flt32
#define F_signN 0x80000000 // flt32 sign bit

#define F_infC (F_infN >> F_shift)
#define F_nanN ((F_infC + 1) << F_shift) // minimum flt16 nan as a flt32
#define F_maxC (F_maxN >> F_shift)
#define F_minC (F_minN >> F_shift)
#define F_signC (F_signN >> F_shiftSign) // flt16 sign bit

#define F_mulN 0x52000000 // (1 << 23) / F_minN
#define F_mulC 0x33800000 // F_minN / (1 << (23 - F_shift))

#define F_subC 0x003FF // max flt32 subnormal down shifted
#define F_norC 0x00400 // min flt32 normal down shifted

#define F_maxD (F_infC - F_maxC - 1)
#define F_minD (F_minC - F_subC - 1)

union hfpack
{
	uint32_t as_uint32;
	struct
	{
		uint16_t s1f;
		uint16_t s0f;
	} as_f16f16;
};

struct si_sx_ps_init_meta_t
{
	int x;
	int y;
	float brctrc_i;
	float brctrc_j;
};

struct si_sx_ps_init_lds_t
{
	float *data;
	int size; /* Size in bytes */
};

/* Elements are created in SPI module */
struct si_sx_ps_init_t
{
	struct list_t *meta_list; 
	struct si_sx_ps_init_lds_t *lds;
};

typedef struct
{
	int *buffer;

	int width;
	int height;	
} SISXMRT;

CLASS_BEGIN(SISX, Object)

	/* Emulator */
	SIEmu *emu;

	/* Lists contain data from export instructions */
	struct list_t *pos[SI_POS_COUNT];
	struct list_t *param[SI_PARAM_COUNT];
	SISXMRT *mrt[SI_MRT_COUNT];

CLASS_END(SISX)

uint16_t Float32to16(float value);
float Float16to32(uint16_t value);

void SISXCreate(SISX *self, SIEmu *emu);
void SISXDestroy(SISX *self);
void SISXReset(SISX *self);

/* Only used by workitems in machine.c */
void SISXExportPosition(SISX *self, unsigned int target, unsigned int id, 
	float x, float y, float z, float w);
void SISXExportParam(SISX *self, unsigned int target, unsigned int id, 
	float x, float y, float z, float w);
void SISXExportMRT(SISX *self, unsigned int target, SIWorkItem *work_item, unsigned int compr_en, 
	SIInstReg x, SIInstReg y, SIInstReg z, SIInstReg w);

struct si_sx_ps_init_lds_t *SISXPSInitLDSCreate(unsigned int attribute_count);
void SISXPSInitLDSDestroy(struct si_sx_ps_init_lds_t *ps_lds);

struct si_sx_ps_init_meta_t *SISXPSInitMetaCreate(int x, int y, float brctrc_i, float brctrc_j);
void SISXPSInitMetaDestroy(struct si_sx_ps_init_meta_t *meta);

struct si_sx_ps_init_t *SISXPSInitCreate(struct si_sx_ps_init_lds_t *lds, struct list_t *meta_list);
void SISXPSInitDestroy(struct si_sx_ps_init_t *ps_init);

void SISXMRTResize(SISXMRT *mrt, unsigned int width, unsigned int height);
void SISXMRTResizeAll(SISX *self, unsigned int width, unsigned int height);

#endif

