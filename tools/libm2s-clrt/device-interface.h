#ifndef __DEVICE_INTERFACE_H__
#define __DEVICE_INTERFACE_H__

#include <CL/cl.h>

/* same style as clGetDeviceIDs, except this time 
 * it is the framework querying a driver */
typedef cl_int (*clrt_device_type_init_devices_t)(
	cl_uint num_entries, 
	cl_device_id *devices, 
	cl_uint *num_devices);

/* check if a binary blob is a valid program */
typedef cl_bool (*clrt_device_type_is_valid_binary_t)(
	size_t length,
	const unsigned char *binary);

/* create a kernel object who's parameters can be set */
typedef void *(*clrt_device_type_create_kernel_t)(
	void *handle, 
	const char *kernel_name, 
	cl_int *errcode_ret);

/* set the argument of a kernel */
typedef cl_int (*clrt_device_type_set_kernel_arg_t)(
	void *kernel, 
	cl_uint arg_index, 
	size_t arg_size, 
	const void *arg_value);

/* execute on a device */
typedef void (*clrt_device_exceute_ndrange_t)(
	void *device,
	void *kernel, 
	cl_uint work_dim, 
	const size_t *global_work_offset, 
	const size_t *global_work_size, 
	const size_t *local_work_size);


struct clrt_device_type_t
{
	clrt_device_type_init_devices_t init_devices;	
	clrt_device_type_is_valid_binary_t valid_binary;
	clrt_device_type_create_kernel_t create_kernel;
	clrt_device_type_set_kernel_arg_t set_kernel_arg;
	clrt_device_exceute_ndrange_t execute_ndrange;
};

/* create a device type */
typedef struct clrt_device_type_t *(*clrt_device_type_create_t)(void);

#endif
