#include "union-device.h"
#include "mhandle.h"
#include "x86-device.h"
#include "union-kernel.h"
#include "union-program.h"


struct opencl_union_device_t *opencl_union_device_create(struct opencl_device_t *parent, struct list_t *devices)
{
	cl_uint i;
	struct opencl_union_device_t *u;
	struct opencl_device_t *tmp;

	u = xcalloc(1, sizeof (struct opencl_union_device_t));
	u->parent = parent;
	int num_devices = list_count(devices);
	u->devices = list_create();
	for (i = 0; i < num_devices; i++)
		list_add(u->devices, list_get(devices, i));

	opencl_debug("[%s] union device contains:", __FUNCTION__);
	for (i = 0; i < num_devices; i++)
	{
		tmp = list_get(devices, i);
		opencl_debug("[%s] %s = %p", __FUNCTION__, tmp->name, tmp);
	}

	*(parent) = *(struct opencl_device_t *)list_get(devices, 0); // just copy over the parameters from someone - we'll do a better job later.
	parent->name = "Multi2Sim Union Device";
	parent->type = CL_DEVICE_TYPE_ACCELERATOR;

	parent->arch_device_free_func =
			(opencl_arch_device_free_func_t)
			opencl_union_device_free;
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
	parent->arch_device_preferred_workgroups_func = NULL;

	/* Call-back functions for architecture-specific program */
	parent->arch_program_create_func =
			(opencl_arch_program_create_func_t)
			opencl_union_program_create;
	parent->arch_program_free_func =
			(opencl_arch_program_free_func_t)
			opencl_union_program_free;
	parent->arch_program_valid_binary_func =
			opencl_union_program_valid_binary;

	/* Call-back functions for architecture-specific kernel */
	parent->arch_kernel_create_func =
			(opencl_arch_kernel_create_func_t)
			opencl_union_kernel_create;
	parent->arch_kernel_free_func =
			(opencl_arch_kernel_free_func_t)
			opencl_union_kernel_free;
	parent->arch_kernel_set_arg_func =
			(opencl_arch_kernel_set_arg_func_t)
			opencl_union_kernel_set_arg;

	/* Call-back functions for architecture-specific ND-Range */
	parent->arch_ndrange_create_func =
			(opencl_arch_ndrange_create_func_t)
			opencl_union_ndrange_create;
	parent->arch_ndrange_free_func =
			(opencl_arch_ndrange_free_func_t)
			opencl_union_ndrange_free;
	parent->arch_ndrange_init_func =
			(opencl_arch_ndrange_init_func_t)
			opencl_union_ndrange_init;
	parent->arch_ndrange_run_func =
			(opencl_arch_ndrange_run_func_t)
			opencl_union_ndrange_run;
	parent->arch_ndrange_run_partial_func =
			(opencl_arch_ndrange_run_partial_func_t)
			opencl_union_ndrange_run_partial;

	return u;
}


void opencl_union_device_free(struct opencl_union_device_t *device)
{
	list_free(device->devices);
	free(device);
}
