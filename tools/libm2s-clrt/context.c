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

#include <m2s-clrt.h>

extern struct _cl_pltaform_id *m2s_platform;
extern struct _cl_device_id *m2s_device;




/*
 * Private Functions
 */

void clrt_context_free(void *data)
{
	struct _cl_context *context; 

	context = (struct _cl_context *) data;
	free(context->devices);
	free(context);
}




/*
 * Public Functions
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
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tproperties = %p", properties);
	m2s_clrt_debug("\tnum_devices = %u", num_devices);
	m2s_clrt_debug("\tdevices = %p", devices);
	m2s_clrt_debug("\tcallback = %p", pfn_notify);
	m2s_clrt_debug("\tuser_data = %p", user_data);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ((int) properties, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ((int) pfn_notify, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ((int) user_data, 0);

	if (!m2s_platform)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_PLATFORM;		
		return NULL;
	}

	if (!devices || !num_devices)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	if (*devices != m2s_device)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_DEVICE;
		return NULL;
	}
	

	context = (struct _cl_context *) malloc(sizeof (struct _cl_context));
	if (!context)
		fatal("%s: out of memory", __FUNCTION__);
	clrt_object_create(context, CLRT_OBJECT_CONTEXT, clrt_context_free);


	context->num_devices = 1;
	context->devices = (struct _cl_device_id **) malloc(sizeof (struct _cl_device_id *) * context->num_devices);
	if (!context->devices)
		fatal("%s: out of memory", __FUNCTION__);

	for (i = 0; i < context->num_devices; i++)
		context->devices[i] = devices[i];

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;

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
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tproperties = %p", properties);
	m2s_clrt_debug("\tdevice_type = 0x%x", (int) device_type);
	m2s_clrt_debug("\tcallback = %p", pfn_notify);
	m2s_clrt_debug("\tuser_data = %p", user_data);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	if (device_type == CL_DEVICE_TYPE_GPU || device_type == CL_DEVICE_TYPE_ACCELERATOR)
	{
		if (errcode_ret)
			*errcode_ret = CL_DEVICE_NOT_FOUND;
		return NULL;
	}

	if (device_type != CL_DEVICE_TYPE_CPU && device_type != CL_DEVICE_TYPE_DEFAULT && device_type != CL_DEVICE_TYPE_ALL)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_DEVICE_TYPE;
	}

	return clCreateContext(properties, 1, &m2s_device, NULL, NULL, errcode_ret);
}


cl_int clRetainContext(
	cl_context context)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcontext = %p", context);

	return clrt_object_retain(context, CLRT_OBJECT_CONTEXT, CL_INVALID_CONTEXT);
}


cl_int clReleaseContext(
	cl_context context)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcontext = %p", context);

	return clrt_object_release(context, CLRT_OBJECT_CONTEXT, CL_INVALID_CONTEXT);
}


cl_int clGetContextInfo(
	cl_context context,
	cl_context_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


