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

#include <gpukernel-local.h>
#include <assert.h>
#include <debug.h>
#include <lnlist.h>
#include <stdlib.h>


/* Initialize GPU kernel */
void gk_init()
{
	struct opencl_device_t *device;

	/* Debug categories */
	opencl_debug_category = debug_new_category();

	/* Create platform and device */
	opencl_object_list = lnlist_create();
	opencl_platform = opencl_platform_create();
	device = opencl_device_create();
}


/* Finalize GPU kernel */
void gk_done()
{
	void *object;
	uint32_t id;

	/* Release OpenCL objects */
	while (lnlist_count(opencl_object_list)) {
		lnlist_head(opencl_object_list);
		object = lnlist_get(opencl_object_list);
		id = * (uint32_t *) object;

		switch (id) {
		case OPENCL_OBJ_PLATFORM:
			opencl_platform_free((struct opencl_platform_t *) object);
			break;
		case OPENCL_OBJ_DEVICE:
			opencl_device_free((struct opencl_device_t *) object);
			break;
		case OPENCL_OBJ_CONTEXT:
			opencl_context_free((struct opencl_context_t *) object);
			break;
		case OPENCL_OBJ_COMMAND_QUEUE:
			opencl_command_queue_free((struct opencl_command_queue_t *) object);
			break;
		default:
			panic("gk_done: unknown OpenCL object");
		}
	}
	lnlist_free(opencl_object_list);
}

