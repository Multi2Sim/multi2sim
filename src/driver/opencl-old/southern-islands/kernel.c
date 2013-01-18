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

#include <arch/southern-islands/asm/bin-file.h>
#include <arch/southern-islands/emu/emu.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <mem-system/memory.h>

#include "kernel.h"
#include "opencl.h"
#include "program.h"
#include "repo.h"


struct si_opencl_kernel_t *si_opencl_kernel_create()
{
	struct si_opencl_kernel_t *kernel;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct si_opencl_kernel_t));
	kernel->id = si_opencl_repo_new_object_id(si_emu->opencl_repo,
		si_opencl_object_kernel);
	kernel->ref_count = 1;
	kernel->arg_list = list_create();

	/* Create the UAV-to-physical-address lookup lists */
	kernel->constant_buffer_list = list_create();

	/* Return */
	si_opencl_repo_add_object(si_emu->opencl_repo, kernel);
	return kernel;
}


void si_opencl_kernel_free(struct si_opencl_kernel_t *kernel)
{
	int i;

	/* Free arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
		si_opencl_kernel_arg_free((struct si_opencl_kernel_arg_t *) list_get(kernel->arg_list, i));
	list_free(kernel->arg_list);

	/* AMD Binary (internal ELF) */
	if (kernel->bin_file)
		si_bin_file_free(kernel->bin_file);

	/* Free lists */
	list_free(kernel->constant_buffer_list);

	/* Free kernel */
	si_opencl_repo_remove_object(si_emu->opencl_repo, kernel);
	free(kernel);
}


struct si_opencl_kernel_arg_t *si_opencl_kernel_arg_create(char *name)
{
	struct si_opencl_kernel_arg_t *arg;

	/* Initialize */
	arg = xcalloc(1, sizeof(struct si_opencl_kernel_arg_t) + strlen(name) + 1);
	strcpy(arg->name, name);

	/* Return */
	return arg;
}


void si_opencl_kernel_arg_free(struct si_opencl_kernel_arg_t *arg)
{
	free(arg);
}

/* Analyze 'metadata' associated with kernel */

#define SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(_tc) \
	if (token_count != (_tc)) \
	fatal("%s: meta data entry '%s' expects %d tokens", \
	__FUNCTION__, line_ptrs[0], (_tc));
#define SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED(_idx) \
	fatal("%s: meta data entry '%s', token %d: value '%s' not supported.\n%s", \
	__FUNCTION__, line_ptrs[0], (_idx), line_ptrs[(_idx)], \
	si_err_opencl_kernel_metadata_note);
#define SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(_idx, _str) \
	if (strcmp(line_ptrs[(_idx)], (_str))) \
	SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED(_idx);

static char *si_err_opencl_kernel_metadata_note =
	"\tThe kernel binary loaded by your application is a valid ELF file. In this\n"
	"\tfile, a '.rodata' section contains specific information about the OpenCL\n"
	"\tkernel. However, this information is only partially supported by Multi2Sim.\n"
	"\tTo request support for this error, please email 'development@multi2sim.org'.\n";

static void si_opencl_kernel_load_metadata(struct si_opencl_kernel_t *kernel)
{
	char line[MAX_STRING_SIZE];
	char *line_ptrs[MAX_STRING_SIZE];
	int token_count;
	struct si_opencl_kernel_arg_t *arg;
	struct elf_buffer_t *buffer;

	/* Open as text file */
	buffer = &kernel->metadata_buffer;
	elf_buffer_seek(buffer, 0);
	si_opencl_debug("Kernel Metadata:\n");
	for (;;)
	{
		/* Read line from buffer */
		elf_buffer_read_line(buffer, line, MAX_STRING_SIZE);
		if (!line[0])
			break;
		si_opencl_debug("\t%s\n", line);

		/* Split line */
		line_ptrs[0] = strtok(line, ":;\n");
		for (token_count = 1; (line_ptrs[token_count] = strtok(NULL, ":\n")); token_count++);

		/* Ignored entries */
		if (!line_ptrs[0] ||
			!strcmp(line_ptrs[0], "ARGSTART") ||
			!strcmp(line_ptrs[0], "version") ||
			!strcmp(line_ptrs[0], "device") ||
			!strcmp(line_ptrs[0], "uniqueid") ||
			!strcmp(line_ptrs[0], "uavid") ||
			!strcmp(line_ptrs[0], "privateid") ||
			!strcmp(line_ptrs[0], "reflection") ||
			!strcmp(line_ptrs[0], "ARGEND"))
			continue;

		/* Image */
		if (!strcmp(line_ptrs[0], "image"))
		{
			/* Create input image argument */
			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_IMAGE;
			if (!strcmp(line_ptrs[2], "2D"))
			{
				/* Ignore dimensions for now */
			}
			else if (!strcmp(line_ptrs[2], "3D"))
			{
				/* Ignore dimensions for now */
			}
			else
			{
				fatal("%s: Invalid number of dimensions for OpenCL Image (%s)\n%s",
					__FUNCTION__, line_ptrs[2], si_err_opencl_param_note);
			}
			
			if (!strcmp(line_ptrs[3], "RO"))
			{
				arg->access_type = SI_OPENCL_KERNEL_ARG_READ_ONLY;
			}
			else if (!strcmp(line_ptrs[3], "WO"))
			{
				arg->access_type = SI_OPENCL_KERNEL_ARG_WRITE_ONLY;
			}
			else
			{
				fatal("%s: Invalid memory access type for OpenCL Image (%s)\n%s",
					__FUNCTION__, line_ptrs[3], si_err_opencl_param_note);
			}
			arg->uav = atoi(line_ptrs[4]);
			arg->mem_scope = SI_OPENCL_MEM_SCOPE_GLOBAL;

			list_add(kernel->arg_list, arg);

			continue;

		} 

		/* Memory */
		if (!strcmp(line_ptrs[0], "memory"))
		{
			if (!strcmp(line_ptrs[1], "hwprivate"))
			{
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2, "0");
			}
			else if (!strcmp(line_ptrs[1], "hwregion"))
			{
				SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2, "0");
			}
			else if (!strcmp(line_ptrs[1], "hwlocal"))
			{
				SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
				kernel->func_mem_local = atoi(line_ptrs[2]);
			}
			else if (!strcmp(line_ptrs[1], "datareqd"))
			{
				SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(2);
			}
			else if (!strcmp(line_ptrs[1], "uavprivate"))
			{
				SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			}
			else
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED(1);

			continue;
		}

		/* Entry 'value'. Format: value:<ArgName>:<DataType>:<Size>:<ConstNum>:<ConstOffset> */
		if (!strcmp(line_ptrs[0], "value"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(6);
			//SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(3, "1");
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");
			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_VALUE;
			list_add(kernel->arg_list, arg);

			continue;
		}

		/* Entry 'pointer'. Format: pointer:<name>:<type>:?:?:<addr>:?:?:<elem_size> */
		if (!strcmp(line_ptrs[0], "pointer"))
		{
			/* APP SDK 2.5 supplies 9 tokens, 2.6 supplies 10 tokens */
			/* Metadata version 3:1:104 (as specified in entry 'version') uses 12 items. */
			if (token_count != 9 && token_count != 10 && token_count != 12)
			{
				SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(10);
			}
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(3, "1");
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");

			/* We don't know what the two last entries are, so make sure that they are
			 * set to 0. If they're not 0, it probably means something important. */
			if (token_count == 12)
			{
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(10, "0");
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(11, "0");
			}

			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_POINTER;

			list_add(kernel->arg_list, arg);
			if (!strcmp(line_ptrs[6], "uav"))
			{
				arg->mem_scope = SI_OPENCL_MEM_SCOPE_GLOBAL;
				arg->uav = atoi(line_ptrs[7]);
			}
			else if (!strcmp(line_ptrs[6], "hl"))
			{
				arg->mem_scope = SI_OPENCL_MEM_SCOPE_LOCAL;
				arg->uav = atoi(line_ptrs[7]);
			}
			else if (!strcmp(line_ptrs[6], "hc"))
			{
				arg->mem_scope = SI_OPENCL_MEM_SCOPE_CONSTANT;
				arg->uav = atoi(line_ptrs[7]);
			}
			else
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED(6);

			continue;
		}

		/* Entry 'function'. Format: function:?:<uniqueid> */
		if (!strcmp(line_ptrs[0], "function"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(1, "1");
			kernel->func_uniqueid = atoi(line_ptrs[2]);
			continue;
		}

		/* Entry 'sampler'. Format: sampler:name:ID:location:value.
		 * 'location' is 1 for kernel defined samplers, 0 for kernel argument.
		 * 'value' is bitfield value of sampler (0 if a kernel argument) */
		if (!strcmp(line_ptrs[0], "sampler"))
		{
			/* As far as I can tell, the actual sampler data is stored 
			 * as a value, so adding it to the argument list is not required */
			continue;
		}

		/* Entry 'reflection'. Format: reflection:<arg_id>:<type>
		 * Observed first in version 3:1:104 of metadata.
		 * This entry specifies the type of the argument, as specified in the OpenCL
		 * kernel function header. It is currently ignored, since this information
		 * is extracted from the argument descriptions in 'value' and 'pointer' entries.
		 */
		if (!strcmp(line_ptrs[0], "reflection"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			continue;
		}

		/* Entry 'privateid'. Format: privateid:<id>
		 * Observed first in version 3:1:104 of metadata. Not sure what this entry is for.
		 */
		if (!strcmp(line_ptrs[0], "privateid"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(2);
			continue;
		}

		/* Entry 'constarg'. Format: constarg:<arg_id>:<arg_name>
		 * Observed first in version 3:1:104 of metadata. It shows up when an argument
		 * is declared as '__global const'. Entry ignored here. */
		if (!strcmp(line_ptrs[0], "constarg"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			continue;
		}

		/* Warn about uninterpreted entries */
		warning("kernel '%s': unknown meta data entry '%s'",
			kernel->name, line_ptrs[0]);
	}
}


/* Extract and analyze information from the program binary associated with 'kernel_name' */
void si_opencl_kernel_load(struct si_opencl_kernel_t *kernel, char *kernel_name)
{
	struct si_opencl_program_t *program;
	char symbol_name[MAX_STRING_SIZE];
	char name[MAX_STRING_SIZE];

	/* First */
	strncpy(kernel->name, kernel_name, MAX_STRING_SIZE);
	program = si_opencl_repo_get_object(si_emu->opencl_repo,
		si_opencl_object_program, kernel->program_id);

	/* Read 'metadata' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_metadata", kernel_name);
	si_opencl_program_read_symbol(program, symbol_name, &kernel->metadata_buffer);
	
	/* Read 'kernel' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_kernel", kernel_name);
	si_opencl_program_read_symbol(program, symbol_name, &kernel->kernel_buffer);
	
	/* Read 'header' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_header", kernel_name);
	si_opencl_program_read_symbol(program, symbol_name, &kernel->header_buffer);

	/* Create and parse kernel binary (internal ELF).
	 * The internal ELF is contained in the buffer pointer to by the 'kernel' symbol. */
	snprintf(name, sizeof(name), "clKernel<%s>.InternalELF", kernel_name);
	kernel->bin_file = si_bin_file_create(kernel->kernel_buffer.ptr, kernel->kernel_buffer.size, name);
	
	/* Analyze 'metadata' file */
	si_opencl_kernel_load_metadata(kernel);
}


unsigned int si_opencl_kernel_get_work_group_info(struct si_opencl_kernel_t *kernel, unsigned int name,
	struct mem_t *mem, unsigned int addr, unsigned int size)
{
	unsigned int size_ret = 0;
	void *info = NULL;

	unsigned long long local_mem_size = 0;
	unsigned int max_work_group_size = 256;  /* FIXME */

	unsigned int compile_work_group_size[3];
	compile_work_group_size[0] = 1;  /* FIXME */
	compile_work_group_size[1] = 1;  /* FIXME */
	compile_work_group_size[2] = 1;  /* FIXME */

	switch (name)
	{

	case 0x11b0:  /* CL_KERNEL_WORK_GROUP_SIZE */
		info = &max_work_group_size;
		size_ret = 4;
		break;

	case 0x11b1:  /* CL_KERNEL_COMPILE_WORK_GROUP_SIZE */
		info = compile_work_group_size;
		size_ret = 4 * 3;
		break;

	case 0x11b2:  /* CL_KERNEL_LOCAL_MEM_SIZE */
	{
		int i;
		struct si_opencl_kernel_arg_t *arg;

		/* Compute local memory usage */
		local_mem_size = kernel->func_mem_local;
		for (i = 0; i < list_count(kernel->arg_list); i++)
		{
			arg = list_get(kernel->arg_list, i);
			if (arg->mem_scope == SI_OPENCL_MEM_SCOPE_LOCAL)
				local_mem_size += arg->size;
		}

		/* Return it */
		info = &local_mem_size;
		size_ret = 8;
		break;
	}
	
	case 0x11b3:  /* CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE */
	case 0x11b4:  /* CL_KERNEL_PRIVATE_MEM_SIZE */
	default:
		fatal("%s: invalid or not implemented value for 'name' (0x%x)\n%s",
			__FUNCTION__, name, si_err_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}

