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
#include <string.h>

#include "context.h"
#include "debug.h"
#include "device.h"
#include "list.h"
#include "mhandle.h"
#include "object.h"
#include "opencl.h"
#include "platform.h"




/*
 * Public Functions
 */


struct opencl_context_t *opencl_context_create(void)
{
	struct opencl_context_t *context;

	/* Initialize */
	context = xcalloc(1, sizeof(struct opencl_context_t));
	context->device_list = list_create();
	
	/* Register OpenCL object */
	opencl_object_create(context, OPENCL_OBJECT_CONTEXT,
		(opencl_object_free_func_t) opencl_context_free);

	/* Return */
	return context;
}


void opencl_context_free(struct opencl_context_t *context)
{
	list_free(context->device_list);
	free(context);
}


int opencl_context_has_device(struct opencl_context_t *context,
	struct opencl_device_t *device)
{
	int i;

	/* Search */
	LIST_FOR_EACH(context->device_list, i)
	{
		if (list_get(context->device_list, i) == device)
			return 1;
	}

	/* Not found */
	return 0;
}




/*
 * OpenCL API Functions
 */

cl_context clCreateContext(
	const cl_context_properties *properties,
	cl_uint num_devices,
	const cl_device_id *devices,
	void (*pfn_notify)(const char *, const void *, size_t , void *),
	void *user_data,
	cl_int *errcode_ret)
{
	int i;
	struct _cl_context *context;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tproperties = %p", properties);
	opencl_debug("\tnum_devices = %u", num_devices);
	opencl_debug("\tdevices = %p", devices);
	opencl_debug("\tcallback = %p", pfn_notify);
	opencl_debug("\tuser_data = %p", user_data);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Supported combinations of arguments */
	OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);
	OPENCL_ARG_NOT_SUPPORTED_NEQ((int) pfn_notify, 0);
	OPENCL_ARG_NOT_SUPPORTED_NEQ((int) user_data, 0);

	/* Platform not initialized */
	if (!opencl_platform)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_PLATFORM;		
		return NULL;
	}

	/* No device given */
	if (!devices || !num_devices)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}	

	/* Check devices given */
	for (i = 0; i < num_devices; i++)
	{
		if (!opencl_device_verify(devices[i]))
		{
			if (errcode_ret)
				*errcode_ret = CL_INVALID_DEVICE;
			return NULL;
		}
		opencl_debug("\tdevice %d: %s", i, devices[i]->name);
	}

	/* Create context */
	context = opencl_context_create();

	/* Add devices */
	for (i = 0; i < num_devices; i++)
		list_add(context->device_list, devices[i]);

	/* Initialize context properties if given */
	if (properties)
	{
		context->prop_count = getPropertiesCount(properties, sizeof (cl_context_properties));
		context->props = xmalloc(sizeof (cl_context_properties) * context->prop_count);
		copyProperties(context->props, properties, sizeof (cl_context_properties), context->prop_count);
	}

	/* Success */
	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;

	/* Return context */
	return context;
}


cl_context clCreateContextFromType(
	const cl_context_properties *properties,
	cl_device_type device_type,
	void (*pfn_notify)(const char *, const void *, size_t , void *),
	void *user_data,
	cl_int *errcode_ret)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tproperties = %p", properties);
	opencl_debug("\tdevice_type = 0x%x", (int) device_type);
	opencl_debug("\tcallback = %p", pfn_notify);
	opencl_debug("\tuser_data = %p", user_data);
	opencl_debug("\terrcode_ret = %p", errcode_ret);


	cl_uint num_devices;
	clGetDeviceIDs(opencl_platform, device_type, 0, NULL, &num_devices);

	if (num_devices == 0)
	{
		if (errcode_ret)
			*errcode_ret = CL_DEVICE_NOT_FOUND;
		return NULL;
	}

	cl_device_id *devices = xmalloc(sizeof devices[0] * num_devices);
	
	clGetDeviceIDs(opencl_platform, device_type, num_devices, devices, NULL);

	cl_context context = clCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
	free(devices);
	return context;
}


cl_int clRetainContext(
	cl_context context)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);

	return opencl_object_retain(context, OPENCL_OBJECT_CONTEXT, CL_INVALID_CONTEXT);
}


cl_int clReleaseContext(
	cl_context context)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);

	return opencl_object_release(context, OPENCL_OBJECT_CONTEXT, CL_INVALID_CONTEXT);
}


cl_int clGetContextInfo(
	cl_context context,
	cl_context_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (!opencl_object_verify(context, OPENCL_OBJECT_CONTEXT))
		return CL_INVALID_CONTEXT;

	switch (param_name)
	{

	case CL_CONTEXT_REFERENCE_COUNT:
	{
		cl_int count = opencl_object_find(context, OPENCL_OBJECT_CONTEXT)->ref_count;
		return opencl_set_param(&count, sizeof count, param_value_size,
			param_value, param_value_size_ret);
	}

	case CL_CONTEXT_DEVICES:
	{
		int i;
		int count = list_count(context->device_list);
		cl_device_id *devices = xcalloc(count, sizeof (cl_device_id));
		LIST_FOR_EACH(context->device_list, i)
			devices[i] = list_get(context->device_list, i);

		cl_int status = opencl_set_param(devices, sizeof (cl_device_id) *
			count, param_value_size, param_value, param_value_size_ret);
		free(devices);
		return status;
	}

/*	
	case CL_CONTEXT_PROPERTIES:
		if (context->props)
			return opencl_set_param(context->props, sizeof (cl_context_properties)
				* context->prop_count, param_value_size, param_value, param_value_size_ret);
		return opencl_set_param(NULL, 0, param_value_size, param_value, param_value_size_ret);
		FIXME - argument 'NULL' is not valid above.
*/

	default:
		return CL_INVALID_VALUE;
	}	
	return 0;
}

