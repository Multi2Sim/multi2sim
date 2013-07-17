/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef RUNTIME_CUDA_DEVICE_H
#define RUNTIME_CUDA_DEVICE_H


extern struct list_t *device_list;

struct cuda_device_t
{
	/* CUdevice is also an ID */
	CUdevice device;

	char *name;

	int attributes[CU_DEVICE_ATTRIBUTE_MAX];
};

struct cuda_device_t *cuda_device_create(int device_type);
void cuda_device_free(struct cuda_device_t *device);


#endif

