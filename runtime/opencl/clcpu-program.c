#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>
#include <stdio.h>

#include "clcpu.h"
#include "clcpu-program.h"
#include "debug.h"

#define MAX_SSE_REG_PARAMS 4
#define SSE_REG_SIZE_IN_WORDS (16 / sizeof (size_t))
#define MEMORY_ALIGN 16


int clcpu_type_size(enum clcpu_param_type_t param_type)
{
	switch (param_type)
	{
	case CLCPU_TYPE_CHAR:
		return 1;

	case CLCPU_TYPE_SHORT:
	case CLCPU_TYPE_CHAR2:
		return 2;

	case CLCPU_TYPE_CHAR3:
		return 3;
	
	case CLCPU_TYPE_CHAR4:
	case CLCPU_TYPE_SHORT2:
	case CLCPU_TYPE_INT:
	case CLCPU_TYPE_FLOAT:
	case CLCPU_TYPE_POINTER:
		return 4;

	case CLCPU_TYPE_SHORT3:
		return 6;

	case CLCPU_TYPE_CHAR8:
	case CLCPU_TYPE_SHORT4:
	case CLCPU_TYPE_INT2:
	case CLCPU_TYPE_FLOAT2:
	case CLCPU_TYPE_LONG:
	case CLCPU_TYPE_DOUBLE:
		return 8;

	case CLCPU_TYPE_INT3:
	case CLCPU_TYPE_FLOAT3:
		return 12;

	case CLCPU_TYPE_CHAR16:
	case CLCPU_TYPE_SHORT8:
	case CLCPU_TYPE_INT4:
	case CLCPU_TYPE_FLOAT4:
	case CLCPU_TYPE_LONG2:
	case CLCPU_TYPE_DOUBLE2:
		return 16;

	case CLCPU_TYPE_LONG3:
	case CLCPU_TYPE_DOUBLE3:
		return 24;

	case CLCPU_TYPE_SHORT16:
	case CLCPU_TYPE_INT8:
	case CLCPU_TYPE_FLOAT8:
	case CLCPU_TYPE_LONG4:
	case CLCPU_TYPE_DOUBLE4:
		return 32;

	case CLCPU_TYPE_INT16:
	case CLCPU_TYPE_FLOAT16:
	case CLCPU_TYPE_LONG8:
	case CLCPU_TYPE_DOUBLE8:
		return 64;

	case CLCPU_TYPE_LONG16:
	case CLCPU_TYPE_DOUBLE16:
		return 128;

	default:
		return 0;
	}

}


int stack_words(enum clcpu_param_type_t param_type)
{
	int size;
	int rem;

	/* round up size to the nearest sizeof (size_t)-bytes. */
	size = clcpu_type_size(param_type);
	rem = size % sizeof (size_t);
	if (rem)
		rem = sizeof (size_t) - rem;
	return (size + rem) / sizeof (size_t);
}


int is_vector_type(enum clcpu_param_type_t param_type)
{
	switch (param_type)
	{
	case CLCPU_TYPE_CHAR:
	case CLCPU_TYPE_SHORT:
	case CLCPU_TYPE_INT:
	case CLCPU_TYPE_LONG:
	case CLCPU_TYPE_POINTER:
	case CLCPU_TYPE_FLOAT:
	case CLCPU_TYPE_DOUBLE:
		return 0;
	default:
		return 1;
	}
}

cl_bool clcpu_device_type_is_valid_binary(size_t length, const unsigned char *binary)
{
	const Elf32_Ehdr *h = (const Elf32_Ehdr *)binary;
	return h->e_machine == 0x7d2;
}

void *get_function_info(void *handle, const char *name, size_t **metadata)
{
	void *addr;
	char *fullname;
	char *metaname;

	fullname = (char *) malloc(strlen(name) + 100);
	if (!fullname)
		fatal("%s: out of memory", __FUNCTION__);
	metaname = (char *) malloc(strlen(name) + 100);
	if (!metaname)
		fatal("%s: out of memory", __FUNCTION__);

	sprintf(fullname, "__OpenCL_%s_kernel", name);
	sprintf(metaname, "__OpenCL_%s_metadata", name);

	*metadata = (size_t *)dlsym(handle, metaname);
	addr = dlsym(handle, fullname);

	free(fullname);
	free(metaname);
	return addr;
}

void *clcpu_device_type_create_kernel(void *handle, const char *kernel_name, cl_int *errcode_ret)
{
	int i;
	int stride;
	int num_reg;
	int stack_offset;
	int remainder;

	struct clcpu_kernel_t *kernel = malloc(sizeof *kernel);
	kernel->function = get_function_info(handle, kernel_name, &kernel->metadata);

	if (!kernel->function)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_KERNEL_NAME;
		return NULL;
	}

	kernel->local_reserved_bytes = kernel->metadata[1];
	kernel->num_params = (kernel->metadata[0] - 44) / 24;
	kernel->param_info = malloc(sizeof kernel->param_info[0] * kernel->num_params);
	if (!kernel->param_info)
		fatal("%s: out of memory", __FUNCTION__);

	memset(kernel->param_info, 0, sizeof kernel->param_info[0] * kernel->num_params);

	if(posix_memalign((void **)&kernel->register_params, MEMORY_ALIGN, sizeof kernel->register_params[0] * MAX_SSE_REG_PARAMS))
		fatal("%s: could not allocate aligned memory", __FUNCTION__);

	stride = 8; 
	num_reg = 0;
	stack_offset = 0;
	
	for (i = 0; i < kernel->num_params; i++)
	{
		struct clcpu_parameter_t *param_info;

		param_info = kernel->param_info + i;
		param_info->param_type = kernel->metadata[8 + stride * i];
		param_info->mem_type = kernel->metadata[8  + stride * i + 1];
		param_info->is_set = 0;
		param_info->size = stack_words(kernel->param_info[i].param_type);
		param_info->is_stack = 1;

		if (is_vector_type(param_info->param_type) && param_info->size <= (MAX_SSE_REG_PARAMS - num_reg) * SSE_REG_SIZE_IN_WORDS)
		{
			param_info->is_stack = 0;
			param_info->reg_offset = num_reg;
			num_reg += param_info->size / SSE_REG_SIZE_IN_WORDS;
	
			/* for double3, long3.	*/
			if (param_info->size % SSE_REG_SIZE_IN_WORDS != 0)
				num_reg++;
		}
		else
		{
			int align_size;
			int remainder;

			align_size = param_info->size;
			if (align_size > SSE_REG_SIZE_IN_WORDS)
				align_size = SSE_REG_SIZE_IN_WORDS;

			remainder = stack_offset % align_size;
			if (remainder)
				stack_offset += align_size - remainder;

			param_info->stack_offset = stack_offset;
			stack_offset += param_info->size;
		}

		/**
		printf("Param %d.  Type: %d.  Mem: %d. Size: %d. Offset: %d.\n", 
                       i, 
                       kernel->param_info[i].param_type, 
                       kernel->param_info[i].mem_type,
		       kernel->param_info[i].size,
		       kernel->param_info[i].offset);
		*/
	}

	remainder = stack_offset % SSE_REG_SIZE_IN_WORDS;
	if (!remainder)
		kernel->stack_param_words = stack_offset;
	else
		kernel->stack_param_words = stack_offset + SSE_REG_SIZE_IN_WORDS - remainder;

	kernel->stack_params = (size_t *) malloc(sizeof (size_t) * kernel->stack_param_words);
	if (!kernel->stack_params)
		fatal("%s: out of memory", __FUNCTION__);

	memset(kernel->stack_params, 0, sizeof (size_t) * kernel->stack_param_words);
	return kernel;
}

cl_int clcpu_device_check_kernel(void *k)
{
	int i;
	struct clcpu_kernel_t *kernel = k;
	for (i = 0; i < kernel->num_params; i++)
		if (kernel->param_info[i].is_set == 0)
			return CL_INVALID_VALUE;
	return CL_SUCCESS;
}

void clcpu_device_kernel_destroy(void *k)
{
	struct clcpu_kernel_t *kernel = k;
	free(kernel->param_info);
	free(kernel->stack_params);
	free(kernel->register_params);
	memset(kernel, 0, sizeof *kernel);
	free(kernel);
}

cl_int clcpu_device_type_set_kernel_arg(void *k, cl_uint arg_index, size_t arg_size, const void *arg_value)
{
	struct clcpu_kernel_t *kernel = k;
	struct clcpu_parameter_t *param_info;

	assert(arg_index >= 0 && arg_index < kernel->num_params);
	param_info = kernel->param_info + arg_index;
	assert(param_info->size * sizeof (size_t) >= arg_size || !arg_value);

	assert((!arg_value) == (param_info->mem_type == CLCPU_MEM_LOCAL));

	/* local memory */
	if (!arg_value)
		kernel->stack_params[param_info->stack_offset] = arg_size;
	else if (param_info->mem_type == CLCPU_MEM_GLOBAL || param_info->mem_type == CLCPU_MEM_CONSTANT)
	{
		void *addr = clrt_get_address_of_buffer_object(*(cl_mem *) arg_value);
		if (!addr)
			return CL_INVALID_MEM_OBJECT;
		memcpy(kernel->stack_params + param_info->stack_offset, &addr, sizeof addr);
	}
	/* only works on Little-endian machines */
	else if (param_info->is_stack)
		memcpy(kernel->stack_params + param_info->stack_offset, arg_value, arg_size); 
	else
		memcpy(kernel->register_params + param_info->reg_offset, arg_value, arg_size);

	kernel->param_info[arg_index].is_set = 1;
	return 0;
}

