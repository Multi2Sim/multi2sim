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

#ifndef SOUTHERN_ISLANDS_OPENCL_MEM_H
#define SOUTHERN_ISLANDS_OPENCL_MEM_H

#include <arch/southern-islands/emu/emu.h>

struct si_opencl_mem_t
{
	uint32_t id;
	int ref_count;

	uint32_t type;  /* 0 buffer, 1 2D image, 2 3D image */

	uint32_t size;

	/* Used for images only */
	uint32_t height;
	uint32_t width;
	uint32_t depth;
	uint32_t num_pixels;
	uint32_t pixel_size;
	uint32_t num_channels_per_pixel;

	/* Used for mapping only */
	uint32_t map_flags;
	uint32_t map_offset;
	uint32_t map_cb;

	uint32_t flags;
	uint32_t host_ptr;

	uint32_t device_ptr;  /* Position assigned in device global memory */
};

struct si_opencl_mem_t *si_opencl_mem_create(void);
void si_opencl_mem_free(struct si_opencl_mem_t *mem);

#endif
