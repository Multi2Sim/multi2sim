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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WORK_ITEM_OLD_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WORK_ITEM_OLD_H

#include <arch/southern-islands/asm/Wrapper.h>
#include <lib/class/class.h>


/*
 * Class 'SIWorkItem'
 */

#define SI_MAX_LDS_ACCESSES_PER_INST  2

/* Structure describing a memory access definition */
struct si_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	unsigned int addr;
	int size;
};


CLASS_BEGIN(SIWorkItem, Object)
	
	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	SIWavefront *wavefront;
	SIWorkGroup *work_group;

	/* Work-item state */
	SIInstReg vreg[256];  /* Vector general purpose registers */

	/* Last global memory access */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Last LDS access */
	int lds_access_count;  /* Number of LDS access by last instruction */
	unsigned int lds_access_addr[SI_MAX_LDS_ACCESSES_PER_INST];
	unsigned int lds_access_size[SI_MAX_LDS_ACCESSES_PER_INST];
	int lds_access_type[SI_MAX_LDS_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */

CLASS_END(SIWorkItem)

#define SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(WAVEFRONT, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = 0; \
		(WORK_ITEM_ID) < si_emu_wavefront_size; \
		(WORK_ITEM_ID)++)

void SIWorkItemCreate(SIWorkItem *self, int id, SIWavefront *wavefront);
void SIWorkItemDestroy(SIWorkItem *self);

/* FIXME
 * Some older compilers need the 'union' type to be not only declared but 
 * also defined to allow for the declaration below. This forces us to 
 * #include 'asm.h' from this file.  To avoid this extra dependence, 
 * 'union' types below could be replaced by 'unsigned int', and then all 
 * callers updated accordingly. */
unsigned int SIWorkItemReadSReg(SIWorkItem *self, int sreg);
void SIWorkItemWriteSReg(SIWorkItem *self, int sreg, 
	unsigned int value);
unsigned int SIWorkItemReadVReg(SIWorkItem *self, int vreg);
void SIWorkItemWriteVReg(SIWorkItem *self, int vreg, 
	unsigned int value);
unsigned int SIWorkItemReadReg(SIWorkItem *self, int reg);
void SIWorkItemWriteBitmaskSReg(SIWorkItem *self, int sreg, 
	unsigned int value);
int SIWorkItemReadBitmaskSReg(SIWorkItem *self, int sreg);

struct si_buffer_desc_t;
struct si_mem_ptr_t;
void SIWorkItemReadBufferResource(SIWorkItem *self, 
	struct si_buffer_desc_t *buf_desc, int sreg);
void SIWorkItemReadMemPtr(SIWorkItem *self, 
	struct si_mem_ptr_t *mem_ptr, int sreg);

#endif
