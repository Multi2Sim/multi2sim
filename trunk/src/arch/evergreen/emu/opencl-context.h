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

#ifndef ARCH_EVERGREEN_EMU_OPENCL_CONTEXT_H
#define ARCH_EVERGREEN_EMU_OPENCL_CONTEXT_H


struct evg_opencl_context_t
{
	unsigned int id;
	int ref_count;

	unsigned int platform_id;
	unsigned int device_id;
};

struct evg_opencl_context_t *evg_opencl_context_create(void);
void evg_opencl_context_free(struct evg_opencl_context_t *context);

struct mem_t;
unsigned int evg_opencl_context_get_info(struct evg_opencl_context_t *context,
	unsigned int name, struct mem_t *mem, unsigned int addr, unsigned int size);
void evg_opencl_context_set_properties(struct evg_opencl_context_t *context,
	struct mem_t *mem, unsigned int addr);


#endif

