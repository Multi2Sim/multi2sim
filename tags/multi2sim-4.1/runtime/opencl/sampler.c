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

#include <unistd.h>
#include <stdio.h>

#include "debug.h"
#include "mhandle.h"
#include "sampler.h"



/*
 * Public Functions
 */

struct opencl_sampler_t *opencl_sampler_create(void)
{
	struct opencl_sampler_t *sampler;

	/* Initialize */
	sampler = xcalloc(1, sizeof(struct opencl_sampler_t));

	/* Return */
	return sampler;
}


void opencl_sampler_free(struct opencl_sampler_t *sampler)
{
	free(sampler);
}




/*
 * OpenCL API Functions
 */

cl_sampler clCreateSampler(
	cl_context context,
	cl_bool normalized_coords,
	cl_addressing_mode addressing_mode,
	cl_filter_mode filter_mode,
	cl_int *errcode_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clRetainSampler(
	cl_sampler sampler)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clReleaseSampler(
	cl_sampler sampler)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clGetSamplerInfo(
	cl_sampler sampler,
	cl_sampler_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}

