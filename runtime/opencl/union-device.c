#include "union-device.h"
#include "mhandle.h"
#include "x86-device.h"
#include "union-kernel.h"

struct opencl_union_device_t *opencl_union_device_create(struct opencl_device_t *parent, cl_uint num_devices, cl_device_id *devices)
{
	cl_uint i;
	struct opencl_union_device_t *u;
	u = xcalloc(1, sizeof (struct opencl_union_device_t));
	u->parent = parent;
	u->devices = list_create_with_size(num_devices);
	for (i = 0; i < num_devices; i++)
		list_set(u->devices, i, devices[i]);

	*(parent) = *(devices[0]); // just copy over the parameters from someone - we'll do a better job later.
	parent->arch_device_mem_alloc_func =
			(opencl_arch_device_mem_alloc_func_t)
			opencl_x86_device_mem_alloc;
	parent->arch_device_mem_free_func =
			(opencl_arch_device_mem_free_func_t)
			opencl_x86_device_mem_free;
	parent->arch_device_mem_read_func =
			(opencl_arch_device_mem_read_func_t)
			opencl_x86_device_mem_read;
	parent->arch_device_mem_write_func =
			(opencl_arch_device_mem_write_func_t)
			opencl_x86_device_mem_write;
	parent->arch_device_mem_copy_func =
			(opencl_arch_device_mem_copy_func_t)
			opencl_x86_device_mem_copy;

	parent->arch_kernel_run_func =
			(opencl_arch_kernel_run_func_t)
			opencl_union_kernel_run;

	return u;
}

