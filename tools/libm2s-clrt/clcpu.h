#ifndef __CLCPU_H__
#define __CLCPU_H__

#include "device-interface.h"
#include "fibers.h"

struct clrt_device_type_t *clcpu_create_device_type(void);


cl_int clcpu_device_type_init_devices(
	cl_uint num_entries, 
	cl_device_id *devices, 
	cl_uint *num_devices);

cl_bool clcpu_device_type_is_valid_binary(
	size_t length,
	const unsigned char *binary);


void *clcpu_device_type_create_kernel(
	void *handle, 
	const char *kernel_name, 
	cl_int *errcode_ret);

cl_int clcpu_device_type_set_kernel_arg(
	void *kernel, 
	cl_uint arg_index, 
	size_t arg_size, 
	const void *arg_value);

void clcpu_device_exceute_ndrange(
	void *device,
	void *kernel, 
	cl_uint work_dim, 
	const size_t *global_work_offset, 
	const size_t *global_work_size, 
	const size_t *local_work_size);










#endif

