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

#include <assert.h>

#include "debug.h"
#include "elf-format.h"
#include "list.h"
#include "mem.h"
#include "mhandle.h"
#include "object.h"
#include "si-kernel.h"
#include "si-program.h"
#include "string.h"


/*
 * Southern Islands Kernel Argument
 */

static struct str_map_t opencl_si_arg_type_map =
{
	4,
	{
		{ "value", opencl_si_arg_value },
		{ "pointer", opencl_si_arg_pointer },
		{ "image", opencl_si_arg_image },
		{ "sampler", opencl_si_arg_sampler }
	}
};


struct opencl_si_arg_t *opencl_si_arg_create(enum opencl_si_arg_type_t type,
		char *name)
{
	struct opencl_si_arg_t *arg;

	/* Initialize */
	arg = xcalloc(1, sizeof(struct opencl_si_arg_t));
	arg->type = type;
	arg->name = xstrdup(name);

	/* Return */
	return arg;
}


void opencl_si_arg_free(struct opencl_si_arg_t *arg)
{
	free(arg->name);
	free(arg);
}


void opencl_si_arg_debug(struct opencl_si_arg_t *arg)
{
	opencl_debug("\ttype = %s", str_map_value(&opencl_si_arg_type_map,
			arg->type));
	opencl_debug("\tname = '%s'", arg->name);
}




/*
 * Southern Islands Kernel
 */

static void opencl_si_kernel_metadata_line(struct opencl_si_kernel_t *kernel,
	char *line)
{
	struct list_t *token_list;
	struct opencl_si_arg_t *arg;

	char *token;

	/* Split line in tokens */
	token_list = str_token_list_create(line, ":;");
	token = str_token_list_first(token_list);

	/* Version */
	if (!strcmp(token, "version"))
	{
		int version;
		char *version_str;

		str_token_list_shift(token_list);
		version_str = str_token_list_first(token_list);
		version = atoi(version_str);
		if (version != 3)
			fatal("%s: unsupported metadata version: %s",
					__FUNCTION__, line);
	}

	/* Pointer argument */
	else if (!strcmp(token, "pointer"))
	{
		/* Token 1 - Name */
		str_token_list_shift(token_list);
		token = str_token_list_first(token_list);
		arg = opencl_si_arg_create(opencl_si_arg_pointer, token);

		/* Add argument */
		list_add(kernel->arg_list, arg);
	}

	/* Value argument */
	else if (!strcmp(token, "value"))
	{
		/* Token 1 - Name */
		str_token_list_shift(token_list);
		token = str_token_list_first(token_list);
		arg = opencl_si_arg_create(opencl_si_arg_value, token);
		str_token_list_shift(token_list);

		/* Add argument */
		list_add(kernel->arg_list, arg);
	}

	/* Free token list */
	str_token_list_free(token_list);
}


struct opencl_si_kernel_t *opencl_si_kernel_create( 
	struct opencl_kernel_t *parent, struct opencl_si_program_t *program,
	char *func_name)
{
	struct opencl_si_kernel_t *kernel;
	struct elf_symbol_t *symbol;
	struct elf_buffer_t elf_buffer;

	char symbol_name[MAX_STRING_SIZE];
	char line[MAX_STRING_SIZE];

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_si_kernel_t));
	kernel->parent = parent;
	kernel->program = program;
	kernel->arg_list = list_create();

	/* Get kernel metadata from binary */
	snprintf(symbol_name, sizeof symbol_name, "__OpenCL_%s_metadata", 
		func_name);
	symbol = elf_symbol_get_by_name(program->elf_file, symbol_name);
	if (!symbol)
		fatal("%s: %s: kernel function not found in binary",
				__FUNCTION__, func_name);
	if (!elf_symbol_read_content(program->elf_file, symbol, &elf_buffer))
		fatal("%s: unexpected error in metadata symbol",
				__FUNCTION__);

	/* Process metadata */
	while (elf_buffer.pos < elf_buffer.size)
	{
		elf_buffer_read_line(&elf_buffer, line, sizeof line);
		opencl_si_kernel_metadata_line(kernel, line);
	}

	/* Create kernel object in driver */
	kernel->id = syscall(OPENCL_SYSCALL_CODE, opencl_abi_si_kernel_create,
			program->id, func_name);

	/* Dump debug info and return */
	opencl_si_kernel_debug(kernel);
	return kernel;
}


void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel)
{
	int index;

	/* Free arguments */
	LIST_FOR_EACH(kernel->arg_list, index)
		opencl_si_arg_free(list_get(kernel->arg_list, index));
	list_free(kernel->arg_list);

	/* Free kernel */
	free(kernel);
}


void opencl_si_kernel_debug(struct opencl_si_kernel_t *kernel)
{
	struct opencl_si_arg_t *arg;
	int index;

	LIST_FOR_EACH(kernel->arg_list, index)
	{
		opencl_debug("Argument %d", index);
		arg = list_get(kernel->arg_list, index);
		opencl_si_arg_debug(arg);
	}
}


int opencl_si_kernel_set_arg(struct opencl_si_kernel_t *kernel, int arg_index,
	unsigned int arg_size, void *arg_value)
{
	struct opencl_si_arg_t *arg;
	struct opencl_mem_t *mem;

	/* Check valid argument index */
	arg = list_get(kernel->arg_list, arg_index);
	if (!arg)
		fatal("%s: invalid argument index (%d)\n",
				__FUNCTION__, arg_index);

	/* Perform ABI call depending on the argument type */
	switch (arg->type)
	{

	case opencl_si_arg_value:

		/* ABI call */
		syscall(OPENCL_SYSCALL_CODE, opencl_abi_si_kernel_set_arg_value,
				kernel->id, arg_index, arg_value, arg_size);
		break;

	case opencl_si_arg_pointer:

		/* If an argument value is given, it should be a 'cl_mem' 
		 * object.  We need to obtain the 'device_ptr' of the 
		 * allocated memory associated with it. */
		if (arg_value)
		{
			mem = * (struct opencl_mem_t **) arg_value;
			if (!opencl_object_verify(mem, OPENCL_OBJECT_MEM))
				fatal("%s: argument %d is not a cl_mem object",
						__FUNCTION__, arg_index);
			if (arg_size != 4)
			{
				fatal("%s: cl_mem argument %d expects size 4 "
					"(%d given)", __FUNCTION__, arg_index,
					arg_size);
			}
			arg_value = mem->device_ptr;
			arg_size = mem->size;
		}

		/* ABI call */
		syscall(OPENCL_SYSCALL_CODE, 
			opencl_abi_si_kernel_set_arg_pointer, kernel->id, 
			arg_index, arg_value, arg_size);
		break;

	default:
		fatal("%s: argument type not supported (%d)",
				__FUNCTION__, arg->type);
	}

	/* Success */
	return 0;
}


void opencl_si_kernel_run(struct opencl_si_kernel_t *kernel, int work_dim,
	unsigned int *global_work_offset, unsigned int *global_work_size,
	unsigned int *local_work_size, unsigned int *group_id_offset)
{
	/* ABI call */
	syscall(OPENCL_SYSCALL_CODE, opencl_abi_si_ndrange_initialize, 
		kernel->id, work_dim, global_work_offset, global_work_size, 
		local_work_size);
}

