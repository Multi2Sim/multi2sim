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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_MEM_H
#define ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_MEM_H


struct si_opencl_mem_t
{
	unsigned int id;
	int ref_count;

	unsigned int type;  /* 0 buffer, 1 2D image, 2 3D image */

	unsigned int size;

	/* Used for images only */
	unsigned int height;
	unsigned int width;
	unsigned int depth;
	unsigned int num_pixels;
	unsigned int pixel_size;
	unsigned int num_channels_per_pixel;

	/* Used for mapping only */
	unsigned int map_flags;
	unsigned int map_offset;
	unsigned int map_cb;

	unsigned int flags;
	unsigned int host_ptr;

	unsigned int device_ptr;  /* Position assigned in device global memory */
};

struct si_opencl_mem_t *si_opencl_mem_create(void);
void si_opencl_mem_free(struct si_opencl_mem_t *mem);

#endif
