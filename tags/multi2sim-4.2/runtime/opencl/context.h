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

#ifndef RUNTIME_OPENCL_CONTEXT_H
#define RUNTIME_OPENCL_CONTEXT_H

#include "opencl.h"


/* Context Object */
struct _cl_context
{
	/* List of devices - elements of type opencl_device_t */
	struct list_t *device_list;

	/* Properties */
	size_t prop_count;
	cl_context_properties *props;
};


struct opencl_context_t *opencl_context_create(void);
void opencl_context_free(struct opencl_context_t *context);

/* Return TRUE if the device is associated with the context. */
int opencl_context_has_device(struct opencl_context_t *context,
	struct opencl_device_t *device);



#endif

