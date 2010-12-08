/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#ifndef GPUKERNEL_LOCAL_H
#define GPUKERNEL_LOCAL_H

#include <gpukernel.h>
#include <stdint.h>



/* OpenCL API Implementation */


/* OpenCL objects */

enum opencl_obj_enum {
	OPENCL_OBJ_PLATFORM = 1,
	OPENCL_OBJ_DEVICE,
	OPENCL_OBJ_CONTEXT,
	OPENCL_OBJ_COMMAND_QUEUE
};

extern struct lnlist_t *opencl_object_list;

void opencl_object_add(void *object);
void opencl_object_remove(void *object);
void *opencl_object_get(enum opencl_obj_enum type, uint32_t id);
void *opencl_object_get_type(enum opencl_obj_enum type);
uint32_t opencl_assign_object_id(enum opencl_obj_enum type);




/* OpenCL platform */

struct opencl_platform_t
{
	uint32_t id;
};

extern struct opencl_platform_t *opencl_platform;

struct opencl_platform_t *opencl_platform_create();
void opencl_platform_free(struct opencl_platform_t *platform);




/* OpenCL devices */

struct opencl_device_t
{
	uint32_t id;
};

struct opencl_device_t *opencl_device_create();
void opencl_device_free(struct opencl_device_t *device);




/* OpenCL contexts */

struct opencl_context_t
{
	uint32_t id;
	uint32_t device_id;  /* ID of associated device (only one allowed so far) */
};

struct opencl_context_t *opencl_context_create();
void opencl_context_free(struct opencl_context_t *context);




/* OpenCL command queue */

struct opencl_command_queue_t
{
	uint32_t id;
	uint32_t device_id;
	uint32_t context_id;
};

struct opencl_command_queue_t *opencl_command_queue_create();
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue);


#endif

