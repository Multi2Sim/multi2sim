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

#ifndef SOUTHERN_ISLANDS_OPENCL_CONTEXT_H
#define SOUTHERN_ISLANDS_OPENCL_CONTEXT_H

#include <arch/southern-islands/emu/emu.h>

struct si_opencl_context_t
{
	uint32_t id;
	int ref_count;

	uint32_t platform_id;
	uint32_t device_id;
};

struct si_opencl_context_t *si_opencl_context_create(void);
void si_opencl_context_free(struct si_opencl_context_t *context);

uint32_t si_opencl_context_get_info(struct si_opencl_context_t *context,
	uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);
void si_opencl_context_set_properties(struct si_opencl_context_t *context,
	struct mem_t *mem, uint32_t addr);

#endif
