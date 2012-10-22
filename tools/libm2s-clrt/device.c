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

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include <m2s-clrt.h>

/* stack size and alignment of stack */
#define STACK_SIZE 0x2000

extern struct _cl_platform_id *m2s_platform;
extern struct _cl_device_id *m2s_device;




/*
 * Private Functions
 */

void barrier(int data)
{
	struct clrt_workgroup_data_t *workgroup_data;
	struct fiber_t *sleep;
	struct fiber_t *resume;

	workgroup_data = get_workgroup_data();

	sleep = workgroup_data->workitems + workgroup_data->cur_item;
	workgroup_data->cur_item = (workgroup_data->cur_item + 1) % workgroup_data->num_items;

	resume = workgroup_data->workitems + workgroup_data->cur_item;
	switch_fiber(sleep, resume);
}

static clrt_barrier_t barrier_addr = barrier;

/* set the return address of a work item to point to this funciton. */
void exit_work_item(void)
{
	struct clrt_workgroup_data_t *workgroup_data;

	workgroup_data = get_workgroup_data();
	workgroup_data->num_done++;
	exit_fiber(&workgroup_data->main_ctx);
}  


/* Blocking call to exceute a work group.
 * This code is function is run from within a core-assigned runtime thread */
void launch_work_group(
	struct _cl_kernel *kernel, 
	int dims, 
	const size_t *group_start, 
	const size_t *global, 
	const size_t *local, 
	struct clrt_workgroup_data_t *workgroup_data)
{
	size_t i;
	size_t j;
	size_t k;
	size_t local_size[3];
	size_t group_global[3];

	assert(workgroup_data->num_items > 0);
	
	/* we want to safely assume that we have all three dimensions.
	 * but the arrays passed in may only have dims elements,
	 * so we copy them and fill in the rest of the dimensions as having size 1. */
	for (i = 0; i < 3; i++)
	{
		if (i < dims)
		{
			local_size[i] = local[i];
			group_global[i] = group_start[i];
		}
		else
		{
			local_size[i] = 1;
			group_global[i] = 0;
		}
	}

	/* initialize stuff that changes per work group */
	for (i = 0; i < local_size[2]; i++)
		for (j = 0; j < local_size[1]; j++)
			for (k = 0; k < local_size[0]; k++)
			{
				size_t id;
				int x;
				struct clrt_workitem_data_t *workitem_data;

				id = i * local_size[1] * local_size[0] + j * local_size[0] + k;	
				workitem_data = workgroup_data->workitem_data[id];
				/* set the global id */
				workitem_data->global_id[0] = (int32_t) (group_global[0] + k);
				workitem_data->global_id[1] = (int32_t) (group_global[1] + j);
				workitem_data->global_id[2] = (int32_t) (group_global[2] + i);

				/* set group global start id and group id */
				for (x = 0; x < 3; x++)
				{
					workitem_data->group_global[x] = (int32_t) group_global[x];
					workitem_data->group_id[x] = (int32_t) (group_global[x] / local_size[x]);
				}
				
			}

	workgroup_data->num_done = 0;
	/* make new contexts so that they start at the beginning of their functions again  */
	for (i = 0; i < workgroup_data->num_items; i++)
		make_fiber_ex(workgroup_data->workitems + i, kernel->function, exit_work_item, kernel->stack_param_words, workgroup_data->stack_params);

	while (workgroup_data->num_items > workgroup_data->num_done)
		for (workgroup_data->cur_item = 0; workgroup_data->cur_item < workgroup_data->num_items; workgroup_data->cur_item++)
			switch_fiber_cl(&workgroup_data->main_ctx, workgroup_data->workitems + workgroup_data->cur_item, kernel->register_params);
}


/* Check to see whether the device has been assigned work
 * Assume that the calling thread owns device->lock */
struct clrt_execution_t *has_work(struct _cl_device_id *device, int *old_count)
{
	while (device->num_kernels == *old_count)
		pthread_cond_wait(&device->ready, &device->lock);

	(*old_count)++;
	return device->exec;
}


/* Get the next workgroup in an NDRange */
int get_next(struct clrt_execution_t *exec)
{
	int val;

	pthread_mutex_lock(&exec->mutex);
	val = exec->next_group++;
	pthread_mutex_unlock(&exec->mutex);
	return val;
}


void init_workitem(
	struct clrt_workitem_data_t *workitem_data, 
	int dims, 
	const size_t *global, 
	const size_t *local, 
	struct clrt_workgroup_data_t *workgroup_data)
{
	int i;

	memset(workitem_data, 0, sizeof (struct clrt_workitem_data_t));
	for (i = 0; i < 4; i++)
	{
		workitem_data->global_size[i] = 1;
		workitem_data->local_size[i] = 1;
	}

	workitem_data->workgroup_data = (int32_t) workgroup_data;
	/* this probably won't work with 64-bit. */
	workitem_data->barrier_func = (int32_t) &barrier_addr;
	workitem_data->work_dim = dims;
	assert(dims > 0);
	
	for (i = 0; i < dims; i++)
	{
		workitem_data->global_size[i] = global[i];
		workitem_data->local_size[i] = local[i];
	}
} 


void init_workgroup(
	struct clrt_workgroup_data_t *workgroup, 
	struct _cl_kernel *kernel, 
	int dims, 
	const size_t *global, 
	const size_t *local)
{
	int i;

	workgroup->num_items = 1;
	for (i = 0; i < dims; i++)
		workgroup->num_items *= local[i];

	workgroup->num_done = 0;
	workgroup->cur_ctx = NULL;
	workgroup->workitems = (struct fiber_t *) malloc(sizeof (struct fiber_t) * workgroup->num_items);
	if (!workgroup->workitems)
		fatal("%s: out of memory", __FUNCTION__);

	workgroup->workitem_data = (struct clrt_workitem_data_t **) malloc(sizeof (struct clrt_workitem_data_t *) * workgroup->num_items);
	if (!workgroup->workitem_data)
		fatal("%s: out of memory", __FUNCTION__);

	if (posix_memalign((void **)&workgroup->aligned_stacks, STACK_SIZE, STACK_SIZE * workgroup->num_items))
		fatal("%s: aligned memory allocation failure", __FUNCTION__);

	for (i = 0; i < workgroup->num_items; i++)
	{
		struct fiber_t *ctx;

		/* properly initialize the stack and workgroup */
		ctx = workgroup->workitems + i;
		ctx->stack_bottom = workgroup->aligned_stacks + (i * STACK_SIZE);
		ctx->stack_size = STACK_SIZE - sizeof (struct clrt_workitem_data_t);
		workgroup->workitem_data[i] = (struct clrt_workitem_data_t *) ((char *) ctx->stack_bottom + ctx->stack_size);
		init_workitem(workgroup->workitem_data[i], dims, global, local, workgroup);
	}

	/* set up params with local memory pointers sperate from those of other threads */
	workgroup->stack_params = (size_t *) malloc(sizeof (size_t) * kernel->stack_param_words);
	if (!workgroup->stack_params)
		fatal("%s: out of memory", __FUNCTION__);

	memcpy(workgroup->stack_params, kernel->stack_params, sizeof (size_t) * kernel->stack_param_words);
	for (i = 0; i < kernel->num_params; i++)
		if (kernel->param_info[i].mem_type == CLRT_MEM_LOCAL)
		{
			int offset = kernel->param_info[i].stack_offset;
			workgroup->stack_params[offset] = (size_t) clrt_buffer_allocate(kernel->stack_params[offset]);
		}
} 


void destroy_workgroup(struct clrt_workgroup_data_t *workgroup, struct _cl_kernel *kernel)
{
	int i;

	free(workgroup->workitems);
	free(workgroup->workitem_data);
	free(workgroup->aligned_stacks);
	for (i = 0; i < kernel->num_params; i++)
		if (kernel->param_info[i].mem_type == CLRT_MEM_LOCAL)
		{
			int offset;

			offset = kernel->param_info[i].stack_offset;
			clrt_buffer_free((void *) workgroup->stack_params[offset]);
		}
	free(workgroup->stack_params);
}


/* Each core on every device has a thread that runs this procedure
 * It polls for workgroups and launches them on its core */
void *clrt_device_core_proc(void *ptr)
{
	int count = 0;
	struct _cl_device_id *device = (struct _cl_device_id *) ptr;
	struct clrt_execution_t *exec = NULL;

	pthread_mutex_lock(&device->lock);
	while ((exec = has_work(device, &count)))
	{
		int num;
		struct clrt_workgroup_data_t workgroup_data;

		pthread_mutex_unlock(&device->lock);

		init_workgroup(&workgroup_data, exec->kernel, exec->dims, exec->global, exec->local);

		while ((num = get_next(exec)) < exec->num_groups)
			launch_work_group(exec->kernel, exec->dims, exec->group_starts + 3 * num, exec->global, exec->local, &workgroup_data);

		destroy_workgroup(&workgroup_data, exec->kernel);
		pthread_mutex_lock(&device->lock);

		device->num_done++;
		if (device->num_done == device->num_cores);
			pthread_cond_signal(&device->done);
	}

	pthread_mutex_unlock(&device->lock);
	return NULL;
}


/* Used for clGet*Info functions, does standard checking */
cl_int populateParameter(const void *value, size_t actual, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	if (param_value && actual > param_value_size)
		return CL_INVALID_VALUE;
	if (param_value_size_ret)
		*param_value_size_ret = actual;
	if (param_value)
		memcpy(param_value, value, actual);
	return CL_SUCCESS;
}

int dataIsZero(const char *data, size_t size)
{
	int i;
	for (i = 0; i < size; i++)
		if (data[i])
			return 0;
	return 1;
}

size_t getPropertiesCount(const void *properties, size_t prop_size)
{
	size_t size = 1; // There is always a 0 at the end.
	const char *prop = (const char *)properties;
	while (!dataIsZero(prop + size * prop_size, prop_size))
		size++;
	return size;
}

void copyProperties(void *dest, const void *src, size_t size, size_t numObjs)
{
	memcpy(dest, src, size * numObjs);
}


/* Determine the number of CPUs present */
cl_uint getComputeUnitCount(cl_device_id device)
{
	if (device != m2s_device)
		return 0;
	else
		return 4; /* TODO: Fix */
}


/*
 * Public Functions
 */

cl_int clGetDeviceIDs(
	cl_platform_id platform,
	cl_device_type device_type,
	cl_uint num_entries,
	cl_device_id *devices,
	cl_uint *num_devices)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tplatform = %p", platform);
	m2s_clrt_debug("\tdevice_type = 0x%x", (int) device_type);
	m2s_clrt_debug("\tnum_entries = %u", num_entries);
	m2s_clrt_debug("\tdevices = %p", devices);
	m2s_clrt_debug("\tnum_devices = %p", num_devices);

	if (platform != m2s_platform)
		return CL_INVALID_PLATFORM;
	if (device_type == CL_DEVICE_TYPE_GPU || device_type == CL_DEVICE_TYPE_ACCELERATOR)
		return CL_DEVICE_NOT_FOUND;
	if (device_type != CL_DEVICE_TYPE_CPU && device_type != CL_DEVICE_TYPE_DEFAULT && device_type != CL_DEVICE_TYPE_ALL)
		return CL_INVALID_DEVICE_TYPE;


	if (!m2s_device)
	{
		int i;
		m2s_device = (struct _cl_device_id *) malloc(sizeof (struct _cl_device_id));
		if (!m2s_device)
			fatal("%s: out of memory", __FUNCTION__);

		m2s_device->num_cores = getComputeUnitCount(m2s_device);
		m2s_device->num_kernels = 0;
		m2s_device->num_done = 0;
		m2s_device->exec = NULL;

		pthread_mutex_init(&m2s_device->lock, NULL);
		pthread_cond_init(&m2s_device->ready, NULL);
		pthread_cond_init(&m2s_device->done, NULL);

		m2s_device->threads = (pthread_t *) malloc(sizeof (pthread_t) * m2s_device->num_cores);
		if(!m2s_device->threads)
			fatal("%s: out of memory", __FUNCTION__);

		for (i = 0; i < m2s_device->num_cores; i++)
		{
			cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(i, &cpuset);
			assert(pthread_create(m2s_device->threads + i, NULL, clrt_device_core_proc, m2s_device) == 0);
			pthread_setaffinity_np(m2s_device->threads[i], sizeof cpuset, &cpuset);
		}
	}

	/* If a device array is passed in, it must have a corresponding length and vice-versa
	 * The client must also want either a count of the number of devices or the devices themselves */
	if ((!num_entries && devices) || (num_entries && !devices) || (!num_devices && !devices))
		return CL_INVALID_VALUE;

	/* Client wants to know device count */
	else if (!num_entries && num_devices)
	{
		*num_devices = 1;
		return CL_SUCCESS;
	}

	/* Client wants list of devices populated */
	else
	{
		if (num_devices)
			*num_devices = 1;
		devices[0] = m2s_device;

		return CL_SUCCESS;
	}
}


const char *DEVICE_EXTENSIONS = "cl_khr_fp64";

cl_int clGetDeviceInfo(
	cl_device_id device,
	cl_device_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (device != m2s_device)
		return CL_INVALID_DEVICE;

	switch (param_name)
	{
		case CL_DEVICE_ADDRESS_BITS:
		{
			cl_int bits = 8 * sizeof (void *);
			return populateParameter(
				&bits, 
				sizeof bits, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_AVAILABLE:
		{
			cl_bool available = CL_TRUE;
			return populateParameter(
				&available, 
				sizeof available, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_COMPILER_AVAILABLE:
		{
			cl_bool compiler = CL_FALSE;
			return populateParameter(
				&compiler, 
				sizeof compiler, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		/* 0x1032 reserved for CL_DEVICE_DOUBLE_FP_CONFIG */
		case 0x1032:
		{
			cl_device_fp_config fp_config = 
				CL_FP_DENORM | 
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;

			return populateParameter(
				&fp_config, 
				sizeof fp_config, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_ENDIAN_LITTLE:
		{
			cl_bool little = CL_TRUE;
			return populateParameter(
				&little, 
				sizeof little, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
		{
			cl_bool correction = CL_FALSE;
			return populateParameter(
				&correction, 
				sizeof correction, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_EXECUTION_CAPABILITIES:
		{
			cl_device_exec_capabilities capable = CL_EXEC_KERNEL;
			return populateParameter(
				&capable, 
				sizeof capable, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_EXTENSIONS:
			return populateParameter(
				DEVICE_EXTENSIONS, 
				strlen(DEVICE_EXTENSIONS) + 1, 
				param_value_size, 
				param_value, 
				param_value_size_ret);

		case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
		{	
			cl_int size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
		{
			cl_device_mem_cache_type cache_type = CL_READ_WRITE_CACHE;
			return populateParameter(
				&cache_type, 
				sizeof cache_type, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
		{
			cl_uint line_size = 0;
			return populateParameter(
				&line_size, 
				sizeof line_size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_GLOBAL_MEM_SIZE:
		{	
			cl_int size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

//		case CL_DEVICE_HALF_FP_CONFIG:
//			return CL_SUCCESS;

		case CL_DEVICE_HOST_UNIFIED_MEMORY:
		{
			cl_bool unified = CL_TRUE;
			return populateParameter(
				&unified, 
				sizeof unified, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_IMAGE_SUPPORT:
		{
			cl_bool images = CL_FALSE;
			return populateParameter(
				&images, 
				sizeof images, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
		{	
			cl_int size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_IMAGE2D_MAX_WIDTH:
		{	
			cl_int size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_IMAGE3D_MAX_DEPTH:
		{	
			cl_int size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
		{	
			cl_int size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_IMAGE3D_MAX_WIDTH:
		{	
			cl_int size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_LOCAL_MEM_SIZE:
		{
			cl_ulong local = INT_MAX;
			return populateParameter(
				&local, 
				sizeof local, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_LOCAL_MEM_TYPE:
		{
			cl_device_local_mem_type mem_type = CL_GLOBAL;
			return populateParameter(
				&mem_type, 
				sizeof mem_type, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_CLOCK_FREQUENCY:
		{	
			cl_int mhz = 0;
			return populateParameter(
				&mhz, 
				sizeof mhz, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_COMPUTE_UNITS:
		{	
			cl_int units = getComputeUnitCount(device);
			return populateParameter(
				&units, 
				sizeof units, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_CONSTANT_ARGS:
		{
			cl_uint consts = 0;
			return populateParameter(
				&consts, 
				sizeof consts, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
		{
			cl_ulong size = 0;
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
		{
			cl_ulong size = INT_MAX;	
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_PARAMETER_SIZE:
		{
			size_t size = sizeof (cl_long16);
			return populateParameter(
				&size, 
				sizeof size, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_READ_IMAGE_ARGS:
		{
			cl_uint images = 0;
			return populateParameter(
				&images, 
				sizeof images, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_SAMPLERS:
		{	
			cl_int samplers = 0;
			return populateParameter(
				&samplers, 
				sizeof samplers, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_MAX_WORK_GROUP_SIZE:
		{
			size_t work_group_size = 1024;
			return populateParameter(&work_group_size, 
			                         sizeof work_group_size, 
			                         param_value_size, 
			                         param_value, 
			                         param_value_size_ret);
		}

		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
		{
			cl_uint dimensions = 3;
			return populateParameter(&dimensions, 
			                         sizeof dimensions, 
			                         param_value_size, 
			                         param_value, 
			                         param_value_size_ret);
		}

		case CL_DEVICE_MAX_WORK_ITEM_SIZES:
		{
			size_t max_work_items[] = {1024, 1024, 1024};
			return populateParameter(&max_work_items,
			                         sizeof max_work_items,
			                         param_value_size,
			                         param_value,
			                         param_value_size_ret);
		}

		case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
		{
			cl_uint images = 0;
			return populateParameter(&images, sizeof images, param_value_size, param_value, param_value_size_ret);
		}

		case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
		{
			cl_uint base = sizeof (cl_float4);
			return populateParameter(&base,
						 sizeof base,
						 param_value_size,
						 param_value,
						 param_value_size_ret);
		}

		case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
		{
			cl_uint align = sizeof (cl_float4);
			return populateParameter(&align,
						 sizeof align,
						 param_value_size,
						 param_value,
						 param_value_size_ret);
		}

		case CL_DEVICE_NAME:
			return populateParameter(
				DEVICE_NAME, 
				strlen(DEVICE_NAME) + 1, 
				param_value_size, 
				param_value, 
				param_value_size_ret);

		case CL_DEVICE_OPENCL_C_VERSION:
			return populateParameter(
				VERSION, 
				strlen(VERSION) + 1, 
				param_value_size, 
				param_value, 
				param_value_size_ret);

		case CL_DEVICE_PLATFORM:
			return populateParameter(
				&m2s_platform, 
				sizeof m2s_platform, 
				param_value_size, 
				param_value, 
				param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
		{
			cl_uint width = 16;
			return populateParameter(
				&width, 
				sizeof width, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
		{
			cl_uint width = 16 / sizeof (cl_short);
			return populateParameter(
				&width, 
				sizeof width, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
		{
			cl_uint width = 16 / sizeof (cl_int);
			return populateParameter(
				&width, 
				sizeof width, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
		{
			cl_uint width = 16 / sizeof (cl_long);
			return populateParameter(
				&width, 
				sizeof width, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
		{
			cl_uint width = 16 / sizeof (cl_float);
			return populateParameter(
				&width, 
				sizeof width, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
		{
			cl_uint width = 0;
			return populateParameter(
				&width, 
				sizeof width, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
		{
			cl_uint width = 0;
			return populateParameter(
				&width, 
				sizeof width, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PROFILE:
		{
			return populateParameter(
				FULL_PROFILE, 
				strlen(FULL_PROFILE) + 1, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
		{
			size_t resolution = 0;
			return populateParameter(
				&resolution, 
				sizeof resolution, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

/* #define CL_DEVICE_QUEUE_PROPERTIES                  0x102A */
		case CL_DEVICE_QUEUE_PROPERTIES:
		{
			cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
			return populateParameter(
				&props, 
				sizeof props, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_SINGLE_FP_CONFIG:
		{
			cl_device_fp_config fp_config = 
				CL_FP_DENORM | 
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;

			return populateParameter(
				&fp_config, 
				sizeof fp_config, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_TYPE:
		{
			cl_device_type device_type = CL_DEVICE_TYPE_CPU;
			return populateParameter(
				&device_type, 
				sizeof device_type, 
				param_value_size, 
				param_value, 
				param_value_size_ret);

		}

		case CL_DEVICE_VENDOR:
		return populateParameter(
				VENDOR, 
				strlen(VENDOR) + 1, 
				param_value_size, 
				param_value, 
				param_value_size_ret);

		case CL_DEVICE_VENDOR_ID:
		{
			cl_uint vendor_id = 0;
			return populateParameter(	
				&vendor_id, 
				sizeof vendor_id, 
				param_value_size, 
				param_value, 
				param_value_size_ret);
		}

		case CL_DEVICE_VERSION:
			return populateParameter(
				DEVICE_VERSION, 
				strlen(DEVICE_VERSION) + 1, 
				param_value_size, 
				param_value, 
				param_value_size_ret);

		case CL_DRIVER_VERSION:
			return populateParameter(
					DRIVER_VERSION, 
					strlen(DRIVER_VERSION) + 1, 
					param_value_size, 
					param_value, 
					param_value_size_ret);

		default:
			EVG_OPENCL_ARG_NOT_SUPPORTED(param_name)
			return CL_INVALID_VALUE;
	}

	return 0;
}

