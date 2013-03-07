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
#include <arch/southern-islands/emu/isa.h>
#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/emu/wavefront.h>
#include <arch/southern-islands/emu/work-group.h>
#include <arch/southern-islands/emu/work-item.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <mem-system/memory.h>

#include "kernel.h"
#include "mem.h"
#include "opencl.h"
#include "program.h"
#include "repo.h"


static enum si_opencl_kernel_arg_mem_type_t 
	si_opencl_kernel_arg_get_mem_type(char *type_str);
static enum si_opencl_kernel_arg_data_type_t 
	si_opencl_kernel_arg_get_data_type(char *type_str);
static void si_opencl_kernel_load_metadata_8xx(
	struct si_opencl_kernel_t *kernel);
static void si_opencl_kernel_load_metadata_v2(
	struct si_opencl_kernel_t *kernel);
static void si_opencl_kernel_load_metadata_v3(
	struct si_opencl_kernel_t *kernel);
static unsigned int si_opencl_kernel_arg_get_data_size(
	enum si_opencl_kernel_arg_data_type_t data_type);
static enum si_opencl_kernel_arg_access_type_t 
	si_opencl_kernel_get_access_type(char *access_str);



struct si_opencl_kernel_t *si_opencl_kernel_create(char *name)
{
	struct si_opencl_kernel_t *kernel;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct si_opencl_kernel_t) + 
		strlen(name) + 1);
	strcpy(kernel->name, name);
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
	{
		si_opencl_kernel_arg_free((struct si_opencl_kernel_arg_t *) 
			list_get(kernel->arg_list, i));
	}
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
	arg = xcalloc(1, sizeof(struct si_opencl_kernel_arg_t) + 
		strlen(name) + 1);
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

static void si_opencl_kernel_load_metadata_8xx(
	struct si_opencl_kernel_t *kernel)
{
	char line[MAX_STRING_SIZE];
	char *line_ptrs[MAX_STRING_SIZE];

	int token_count;
	int metadata_version[3];

	struct elf_buffer_t *buffer;

	/* Open as text file */
	buffer = &kernel->metadata_buffer;
	elf_buffer_seek(buffer, 0);
	si_opencl_debug("Kernel Metadata:\n");
	for (;;)
	{
		/* Read line containing kernel name */
		elf_buffer_read_line(buffer, line, MAX_STRING_SIZE);
		if (!line[0])
			break;

		si_opencl_debug("\t%s\n", line);

		line_ptrs[0] = strtok(line, ":;\n");
		for (token_count = 1; (line_ptrs[token_count] = 
			strtok(NULL, ":\n")); token_count++);

		if (!strcmp(line_ptrs[0], ";ARGSTART"))
		{
			fatal("%s: Expected ARGSTART, instead: %s", 
				__FUNCTION__, line);
		}
		SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(2);

		/* Read line containing metadata version */
		elf_buffer_read_line(buffer, line, MAX_STRING_SIZE);

		line_ptrs[0] = strtok(line, ":;\n");
		for (token_count = 1; (line_ptrs[token_count] = 
			strtok(NULL, ":\n")); token_count++);

		if (!strcmp(line_ptrs[0], ";version"))
		{
			fatal("%s: Expected version, instead: %s", 
				__FUNCTION__, line);
		}
		SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(4);

		metadata_version[0] = atoi(line_ptrs[1]);
		metadata_version[1] = atoi(line_ptrs[2]);
		metadata_version[2] = atoi(line_ptrs[3]);

		if (metadata_version[0] == 2)
		{
			si_opencl_kernel_load_metadata_v2(kernel);
		}
		else if (metadata_version[0] == 3)
		{
			si_opencl_kernel_load_metadata_v3(kernel);
		}
		else
		{
			fatal("%s: Unsupported metadata major version "
				"(%d)\n", __FUNCTION__, metadata_version[0]);
		}
	}
}

static void si_opencl_kernel_load_metadata_v2(
	struct si_opencl_kernel_t *kernel)
{
	char line[MAX_STRING_SIZE];
	char *line_ptrs[MAX_STRING_SIZE];
	int token_count;
	struct si_opencl_kernel_arg_t *arg;
	struct elf_buffer_t *buffer;

	buffer = &kernel->metadata_buffer;

	for (;;)
	{
		/* Read the next line */
		elf_buffer_read_line(buffer, line, MAX_STRING_SIZE);
		if (!line[0])
			break;

		si_opencl_debug("\t%s\n", line);

		line_ptrs[0] = strtok(line, ":;\n");
		for (token_count = 1; (line_ptrs[token_count] = 
			strtok(NULL, ":\n")); token_count++);

		/* Stop when ARGEND is found */
		if (!strcmp(line_ptrs[0], "ARGEND"))
		{
			break;
		}

		/*
		 * Kernel argument metadata 
		 */

		/* Image */
		if (!strcmp(line_ptrs[0], "image"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(7);

			/* Create input image argument */
			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_IMAGE;

			/* Set the dimension */
			if (!strcmp(line_ptrs[2], "2D"))
			{
				arg->image.dimension = 2;
					
			}
			else if (!strcmp(line_ptrs[2], "3D"))
			{
				arg->image.dimension = 3;
			}
			else
			{
				fatal("%s: Invalid number of dimensions for "
					"OpenCL Image (%s)\n%s", __FUNCTION__,
					line_ptrs[2], 
					si_err_opencl_param_note);
			}
			arg->image.access_type = 
				si_opencl_kernel_get_access_type(line_ptrs[3]);
			arg->image.uav = atoi(line_ptrs[4]);
			arg->image.constant_buffer_num = 
				atoi(line_ptrs[5]);
			arg->image.constant_offset = 
				atoi(line_ptrs[6]);

			arg->size = 4;  /* pointer value is stored */

			list_add(kernel->arg_list, arg);

			continue;
		} 

		/* Value */
		if (!strcmp(line_ptrs[0], "value"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(6);
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");
			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_VALUE;
			arg->value.data_type = 
				si_opencl_kernel_arg_get_data_type(
					line_ptrs[2]);
			arg->value.num_elems = atoi(line_ptrs[3]);
			assert(arg->value.num_elems > 0);
			arg->value.constant_buffer_num = 
				atoi(line_ptrs[4]);
			arg->value.constant_offset = 
				atoi(line_ptrs[5]);

			arg->size = arg->value.num_elems * 
				si_opencl_kernel_arg_get_data_size(
					arg->value.data_type);

			list_add(kernel->arg_list, arg);


			continue;
		}

		/* Pointer */
		if (!strcmp(line_ptrs[0], "pointer"))
		{
			/* APP SDK 2.5 supplies 9 tokens, 2.6 supplies 
			 * 10 tokens. Metadata version 3:1:104 (as specified 
			 * in entry 'version') uses 12 items. */
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(9);

			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_POINTER;
			arg->pointer.data_type = 
				si_opencl_kernel_arg_get_data_type(
					line_ptrs[2]);
			arg->pointer.num_elems = atoi(line_ptrs[3]);
			/* Arrays of pointers not supported */
			assert(arg->pointer.num_elems == 1);
			arg->pointer.constant_buffer_num = 
				atoi(line_ptrs[4]);
			arg->pointer.constant_offset = 
				atoi(line_ptrs[5]);
			arg->pointer.mem_type = 
				si_opencl_kernel_arg_get_mem_type(
					line_ptrs[6]);
			arg->pointer.buffer_num = 
				atoi(line_ptrs[7]);
			arg->pointer.alignment = 
				atoi(line_ptrs[8]);

			arg->size = 4;  /* pointer value is stored */

			list_add(kernel->arg_list, arg);

			continue;
		}

		/* Entry 'sampler'. */
		if (!strcmp(line_ptrs[0], "sampler"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(5);

			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_SAMPLER;

			arg->sampler.id = atoi(line_ptrs[2]);
			arg->sampler.location = atoi(line_ptrs[3]);
			arg->sampler.value = atoi(line_ptrs[4]);

			continue;
		}

		/*
		 * Non-kernel argument metadata 
		 */

		/* Memory 
		 * Used to let the GPU know how much local and private memory 
		 * is required for a kernel, where it should be allocated, 
		 * as well as other information. */
		if (!strcmp(line_ptrs[0], "memory"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			if (!strcmp(line_ptrs[1], "private"))
			{
				fatal("kernel '%s': emulated private memory "
					"not supported", kernel->name);
			}
			else if (!strcmp(line_ptrs[1], "hwprivate"))
			{
				kernel->mem_size_private = atoi(line_ptrs[2]);
			}
			else if (!strcmp(line_ptrs[1], "local"))
			{
				fatal("kernel '%s': emulated local memory not"
					" supported", kernel->name);
			}
			else if (!strcmp(line_ptrs[1], "hwlocal"))
			{
				kernel->mem_size_local = atoi(line_ptrs[2]);
			}
			else
			{
				fatal("kernel '%s': unknown memory metadata "
					"address space entry '%s'", 
					kernel->name, line_ptrs[1]);
			}

			continue;
		}

		/* Function 
		 * Used for multi-kernel compilation units. */
		if (!strcmp(line_ptrs[0], "function"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(1, "1");
			kernel->func_uniqueid = atoi(line_ptrs[2]);
			continue;
		}

		/* Device 
		 * Device that the kernel was compiled for. */
		if (!strcmp(line_ptrs[0], "device"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(2);
			continue;
		}

		/* Crash when uninterpreted entries appear */
		fatal("kernel '%s': unknown metadata entry '%s'",
			kernel->name, line_ptrs[0]);
	}
}

static void si_opencl_kernel_load_metadata_v3(
	struct si_opencl_kernel_t *kernel)
{
	char line[MAX_STRING_SIZE];
	char *line_ptrs[MAX_STRING_SIZE];
	int token_count;
	struct si_opencl_kernel_arg_t *arg;
	struct elf_buffer_t *buffer;

	buffer = &kernel->metadata_buffer;

	for (;;)
	{
		/* Read the next line */
		elf_buffer_read_line(buffer, line, MAX_STRING_SIZE);
		if (!line[0])
			break;

		si_opencl_debug("\t%s\n", line);

		line_ptrs[0] = strtok(line, ":;\n");
		for (token_count = 1; (line_ptrs[token_count] = 
			strtok(NULL, ":\n")); token_count++);

		/* Stop when ARGEND is found */
		if (!strcmp(line_ptrs[0], "ARGEND"))
		{
			break;
		}

		/*
		 * Kernel argument metadata 
		 */

		/* Image */
		if (!strcmp(line_ptrs[0], "image"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(7);

			/* Create input image argument */
			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_IMAGE;

			/* Set the dimension */
			if (!strcmp(line_ptrs[2], "2D"))
			{
				arg->image.dimension = 2;
					
			}
			else if (!strcmp(line_ptrs[2], "3D"))
			{
				arg->image.dimension = 3;
			}
			else
			{
				fatal("%s: Invalid number of dimensions for "
					"OpenCL Image (%s)\n%s", __FUNCTION__,
					line_ptrs[2], 
					si_err_opencl_param_note);
			}
			
			arg->image.access_type = 
				si_opencl_kernel_get_access_type(line_ptrs[3]);
			arg->image.uav = atoi(line_ptrs[4]);
			arg->image.constant_buffer_num = 
				atoi(line_ptrs[5]);
			arg->image.constant_offset = 
				atoi(line_ptrs[6]);

			list_add(kernel->arg_list, arg);

			continue;
		} 

		/* Value */
		if (!strcmp(line_ptrs[0], "value"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(6);
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");
			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_VALUE;
			arg->value.data_type = 
				si_opencl_kernel_arg_get_data_type(
					line_ptrs[2]);
			arg->value.num_elems = atoi(line_ptrs[3]);
			assert(arg->value.num_elems > 0);
			arg->value.constant_buffer_num = 
				atoi(line_ptrs[4]);
			arg->value.constant_offset = 
				atoi(line_ptrs[5]);

			arg->size = arg->value.num_elems * 
				si_opencl_kernel_arg_get_data_size(
					arg->value.data_type);

			si_opencl_debug("Arg %s: value stored in constant "
				"buffer %d at offset %d\n", 
				arg->name, arg->value.constant_buffer_num, 
				arg->value.constant_offset);

			list_add(kernel->arg_list, arg);

			continue;
		}

		/* Pointer */
		if (!strcmp(line_ptrs[0], "pointer"))
		{
			/* APP SDK 2.5 supplies 9 tokens, 2.6 supplies 
			 * 10 tokens. Metadata version 3:1:104 (as specified 
			 * in entry 'version') uses 12 items. */
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(12);

			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_POINTER;
			arg->pointer.data_type = 
				si_opencl_kernel_arg_get_data_type(
					line_ptrs[2]);
			arg->pointer.num_elems = atoi(line_ptrs[3]);
			/* Arrays of pointers not supported */
			assert(arg->pointer.num_elems == 1);
			arg->pointer.constant_buffer_num = 
				atoi(line_ptrs[4]);
			arg->pointer.constant_offset = 
				atoi(line_ptrs[5]);
			arg->pointer.mem_type = 
				si_opencl_kernel_arg_get_mem_type(
					line_ptrs[6]);
			arg->pointer.buffer_num = 
				atoi(line_ptrs[7]);
			arg->pointer.alignment = 
				atoi(line_ptrs[8]);
			arg->pointer.access_type = 
				si_opencl_kernel_get_access_type(
					line_ptrs[9]);

			arg->size = 4;  /* pointer value is stored */

			/* Not sure what these do, so make sure they are 0 */
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(10, "0");
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(11, "0");

			list_add(kernel->arg_list, arg);

			si_opencl_debug("Arg %s: pointer stored in constant "
				"buffer %d at offset %d\n", 
				arg->name, arg->pointer.constant_buffer_num, 
				arg->pointer.constant_offset);

			continue;
		}

		/* Entry 'sampler'. */
		if (!strcmp(line_ptrs[0], "sampler"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(5);

			arg = si_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = SI_OPENCL_KERNEL_ARG_KIND_SAMPLER;

			arg->sampler.id = atoi(line_ptrs[2]);
			arg->sampler.location = atoi(line_ptrs[3]);
			arg->sampler.value = atoi(line_ptrs[4]);

			continue;
		}

		/*
		 * Non-kernel argument metadata 
		 */

		/* Memory 
		 * Used to let the GPU know how much local and private memory 
		 * is required for a kernel, where it should be allocated, 
		 * as well as other information. */
		if (!strcmp(line_ptrs[0], "memory"))
		{
			if (!strcmp(line_ptrs[1], "hwprivate"))
			{
				/* FIXME Add support for private memory by
				 * adding space in global memory */
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2, 
					"0");
			}
			else if (!strcmp(line_ptrs[1], "hwregion"))
			{
				SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2,
					"0");
			}
			else if (!strcmp(line_ptrs[1], "hwlocal"))
			{
				SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
				kernel->mem_size_local = atoi(line_ptrs[2]);
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
			{
				SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED(1);
			}

			continue;
		}

		/* Function 
		 * Used for multi-kernel compilation units. */
		if (!strcmp(line_ptrs[0], "function"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			SI_OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(1, "1");
			kernel->func_uniqueid = atoi(line_ptrs[2]);
			continue;
		}

		/* Reflection 
		 * Format: reflection:<arg_id>:<type>
		 * Observed first in version 3:1:104 of metadata.
		 * This entry specifies the type of the argument, as 
		 * specified in the OpenCL kernel function header. It is 
		 * currently ignored, since this information is extracted from
		 * the argument descriptions in 'value' and 'pointer' entries.
		 */
		if (!strcmp(line_ptrs[0], "reflection"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			continue;
		}

		/* Privateid
		 * Format: privateid:<id>
		 * Observed first in version 3:1:104 of metadata. 
		 * Not sure what this entry is for.
		 */
		if (!strcmp(line_ptrs[0], "privateid"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(2);
			continue;
		}

		/* Constarg
		 * Format: constarg:<arg_id>:<arg_name>
		 * Observed first in version 3:1:104 of metadata. 
		 * It shows up when an argument is declared as 
		 * '__global const'. Entry ignored here. */
		if (!strcmp(line_ptrs[0], "constarg"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			continue;
		}

		/* Device 
		 * Device that the kernel was compiled for. */
		if (!strcmp(line_ptrs[0], "device"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(2);
			continue;
		}

		/* Uniqueid 
		 * A mapping between a kernel and its unique ID */
		if (!strcmp(line_ptrs[0], "uniqueid"))
		{
			SI_OPENCL_KERNEL_METADATA_TOKEN_COUNT(2);
			continue;
		}

		/* Crash when uninterpreted entries appear */
		fatal("kernel '%s': unknown metadata entry '%s'",
			kernel->name, line_ptrs[0]);
	}
}


/* Extract and analyze information from the program binary associated 
 * with 'kernel_name' */
void si_opencl_kernel_load(struct si_opencl_kernel_t *kernel, char *kernel_name)
{
	struct si_opencl_program_t *program;
	char symbol_name[MAX_STRING_SIZE];
	char name[MAX_STRING_SIZE];

	/* Get the program */
	program = si_opencl_repo_get_object(si_emu->opencl_repo,
		si_opencl_object_program, kernel->program_id);

	/* Read 'metadata' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_metadata", 
		kernel_name);
	si_opencl_program_read_symbol(program, symbol_name, 
		&kernel->metadata_buffer);
	
	/* Read 'kernel' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_kernel", 
		kernel_name);
	si_opencl_program_read_symbol(program, symbol_name, 
		&kernel->kernel_buffer);
	
	/* Read 'header' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_header", 
		kernel_name);
	si_opencl_program_read_symbol(program, symbol_name, 
		&kernel->header_buffer);

	/* Create and parse kernel binary (internal ELF).
	 * The internal ELF is contained in the buffer pointer to by 
	 * the 'kernel' symbol. */
	snprintf(name, sizeof(name), "clKernel<%s>.InternalELF", kernel_name);
	kernel->bin_file = si_bin_file_create(kernel->kernel_buffer.ptr, 
		kernel->kernel_buffer.size, name);
	
	/* Analyze 'metadata' file */
	/* XXX Need to know driver version to choose proper function */
	si_opencl_kernel_load_metadata_8xx(kernel);
}


void si_opencl_kernel_setup_ndrange_state(struct si_opencl_kernel_t *kernel,
		struct si_ndrange_t *ndrange)
{
	struct si_wavefront_t *wavefront;
	struct si_work_item_t *work_item;
	struct si_bin_enc_user_element_t *user_elements;

	int work_item_id;
	int wavefront_id;
	int i;

	unsigned int user_sgpr;
	unsigned int user_element_count;
	unsigned int zero = 0;

	float f;

	/* Save local IDs in registers */
	SI_FOREACH_WORK_ITEM_IN_NDRANGE(ndrange, work_item_id)
	{
		work_item = ndrange->work_items[work_item_id];
		work_item->vreg[0].as_int = work_item->id_in_work_group_3d[0];  /* V0 */
		work_item->vreg[1].as_int = work_item->id_in_work_group_3d[1];  /* V1 */
		work_item->vreg[2].as_int = work_item->id_in_work_group_3d[2];  /* V2 */
	}

	/* Initialize the wavefronts */
	SI_FOREACH_WAVEFRONT_IN_NDRANGE(ndrange, wavefront_id)
	{
		/* Get wavefront */
		wavefront = ndrange->wavefronts[wavefront_id];

		/* Save work-group IDs in registers */
		user_sgpr = kernel->bin_file->
			enc_dict_entry_southern_islands->
			compute_pgm_rsrc2->user_sgpr;
		wavefront->sreg[user_sgpr].as_int =
			wavefront->work_group->id_3d[0];
		wavefront->sreg[user_sgpr + 1].as_int =
			wavefront->work_group->id_3d[1];
		wavefront->sreg[user_sgpr + 2].as_int =
			wavefront->work_group->id_3d[2];

		/* Initialize sreg pointers to internal data structures */
		user_element_count = kernel->bin_file->enc_dict_entry_southern_islands->
			userElementCount;
		user_elements = kernel->bin_file->enc_dict_entry_southern_islands->
			userElements;
		for (i = 0; i < user_element_count; i++)
		{
			if (user_elements[i].dataClass == IMM_CONST_BUFFER)
			{
				/* Store CB pointer in sregs */
				si_wavefront_init_sreg_with_cb(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount,
					user_elements[i].apiSlot);
			}
			else if (user_elements[i].dataClass == IMM_UAV)
			{
				/* Store UAV pointer in sregs */
				si_wavefront_init_sreg_with_uav(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount,
					user_elements[i].apiSlot);
			}
			else if (user_elements[i].dataClass ==
				PTR_CONST_BUFFER_TABLE)
			{
				/* Store CB table in sregs */
				si_wavefront_init_sreg_with_cb_table(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == PTR_UAV_TABLE)
			{
				/* Store UAV table in sregs */
				si_wavefront_init_sreg_with_uav_table(
					wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == IMM_SAMPLER)
			{
				/* Store sampler in sregs */
				assert(0);
			}
			else if (user_elements[i].dataClass ==
				PTR_RESOURCE_TABLE)
			{
				/* Store resource table in sregs */
				assert(0);
			}
			else if (user_elements[i].dataClass ==
				PTR_INTERNAL_GLOBAL_TABLE)
			{
				fatal("%s: PTR_INTERNAL_GLOBAL_TABLE not "
					"supported", __FUNCTION__);
			}
			else
			{
				fatal("%s: Unimplemented User Element: "
					"dataClass:%d", __FUNCTION__,
					user_elements[i].dataClass);
			}
		}

		/* Initialize the execution mask */
		wavefront->sreg[SI_EXEC].as_int = 0xffffffff;
		wavefront->sreg[SI_EXEC + 1].as_int = 0xffffffff;
		wavefront->sreg[SI_EXECZ].as_int = 0;
	}

	/* CB0 bytes 0:15 */

	/* Global work size for the {x,y,z} dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 0, 
		&ndrange->global_size3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 4, 
		&ndrange->global_size3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 8, 
		&ndrange->global_size3[2], 4);

	/* Number of work dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 12, &ndrange->work_dim, 4);

	/* CB0 bytes 16:31 */

	/* Local work size for the {x,y,z} dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 16, 
		&ndrange->local_size3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 20, 
		&ndrange->local_size3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 24, 
		&ndrange->local_size3[2], 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 28, &zero, 4);

	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	si_ndrange_const_buf_write(ndrange, 0, 32, 
		&ndrange->group_count3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 36, 
		&ndrange->group_count3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 40, 
		&ndrange->group_count3[2], 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 44, &zero, 4);

	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is
	 * not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 56, &zero, 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 60, &zero, 4);

	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is
	 * not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	si_ndrange_const_buf_write(ndrange, 0, 72, &zero, 4);

	/* FIXME Pointer to location in global buffer where math library
	 * tables start. */

	/* CB0 bytes 80:95 */

	/* 0.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	si_ndrange_const_buf_write(ndrange, 0, 80, &f, 4);

	/* 0.5 as IEEE-32bit float - required for math library. */
	f = 0.5f;
	si_ndrange_const_buf_write(ndrange, 0, 84, &f, 4);

	/* 1.0 as IEEE-32bit float - required for math library. */
	f = 1.0f;
	si_ndrange_const_buf_write(ndrange, 0, 88, &f, 4);

	/* 2.0 as IEEE-32bit float - required for math library. */
	f = 2.0f;
	si_ndrange_const_buf_write(ndrange, 0, 92, &f, 4);

	/* CB0 bytes 96:111 */

	/* Global offset for the {x,y,z} dimension of the work_item spawn */
	si_ndrange_const_buf_write(ndrange, 0, 96, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 100, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 104, &zero, 4);

	/* Global single dimension flat offset: x * y * z */
	si_ndrange_const_buf_write(ndrange, 0, 108, &zero, 4);

	/* CB0 bytes 112:127 */

	/* Group offset for the {x,y,z} dimensions of the work_item spawn */
	si_ndrange_const_buf_write(ndrange, 0, 112, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 116, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 120, &zero, 4);

	/* Group single dimension flat offset, x * y * z */
	si_ndrange_const_buf_write(ndrange, 0, 124, &zero, 4);

	/* CB0 bytes 128:143 */

	/* FIXME Offset in the global buffer where data segment exists */
	/* FIXME Offset in buffer for printf support */
	/* FIXME Size of the printf buffer */
}


void si_opencl_kernel_setup_ndrange_args(struct si_opencl_kernel_t *kernel,
		struct si_ndrange_t *ndrange)
{
	struct si_opencl_kernel_arg_t *arg;
	struct si_opencl_mem_t *mem_obj;

	int i;

	/* Initial top of local memory is determined by the static local memory
	 * specified in the kernel binary. Number of vector and scalar registers
	 * used by the kernel recorded as well. */
	ndrange->local_mem_top = kernel->mem_size_local;
	ndrange->num_sgpr_used = kernel->bin_file->
			enc_dict_entry_southern_islands->num_sgpr_used;
	ndrange->num_vgpr_used = kernel->bin_file->
			enc_dict_entry_southern_islands->num_vgpr_used;

	/* Kernel arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
		{
			fatal("kernel '%s': argument '%s' has not been "
				"assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);
		}

		/* Process argument depending on its type */
		switch (arg->kind)
		{

		case SI_OPENCL_KERNEL_ARG_KIND_VALUE:
		{
			/* Value copied directly into device constant
			 * memory */
			assert(arg->size);
			si_ndrange_const_buf_write(ndrange,
				arg->value.constant_buffer_num,
				arg->value.constant_offset,
				arg->value.value,
				arg->size);
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_POINTER:
		{
			if (arg->pointer.mem_type ==
				SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL)
			{
				/* Pointer in __local scope.
				 * Argument value is always NULL, just assign
				 * space for it. */
				si_ndrange_const_buf_write(ndrange, 
					arg->pointer.constant_buffer_num,
					arg->pointer.constant_offset,
					&ndrange->local_mem_top, 4);

				si_opencl_debug("    arg %d: %d bytes reserved"
					" in local memory at 0x%x\n", i,
					arg->size, ndrange->local_mem_top);

				ndrange->local_mem_top += arg->size;
			}
			else
			{
				/* XXX Need to figure out what value goes in
				 * CB1 and what value goes in
				 * buf_desc.base_addr. For now, putting UAV
				 * offset in CB1 and setting base_addr to 0 */
				mem_obj = si_opencl_repo_get_object(
						si_emu->opencl_repo,
						si_opencl_object_mem,
						arg->pointer.mem_obj_id);
				si_ndrange_const_buf_write(ndrange, 
					arg->pointer.constant_buffer_num,
					arg->pointer.constant_offset,
					&mem_obj->device_ptr, 4);
			}
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_IMAGE:
		{
			assert(0);
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_SAMPLER:
		{
			assert(0);
			break;
		}

		default:
		{
			fatal("%s: argument type not reconized",
				__FUNCTION__);
		}

		}
	}
}

void si_kernel_create_buffer_desc(int num_elems, 
	enum si_opencl_kernel_arg_data_type_t data_type,
	struct si_buffer_desc_t *buffer_desc)
{
	int num_format;
	int data_format;
	int elem_size;

	/* Zero-out the buffer resource descriptor */
	assert(sizeof(*buffer_desc) == 16);
	memset(buffer_desc, 0, sizeof(*buffer_desc));

	num_format = SI_BUF_DESC_NUM_FMT_INVALID;
	data_format = SI_BUF_DESC_DATA_FMT_INVALID;

	if (data_type == SI_OPENCL_KERNEL_ARG_DATA_TYPE_I8)
	{
		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		if (num_elems == 1)
		{
			data_format = SI_BUF_DESC_DATA_FMT_8;
		}
		else if (num_elems == 2)
		{
			data_format = SI_BUF_DESC_DATA_FMT_8_8;
		}
		else if (num_elems == 4)
		{
			data_format = SI_BUF_DESC_DATA_FMT_8_8_8_8;
		}
		elem_size = 1 * num_elems;
	}
	else if (data_type == SI_OPENCL_KERNEL_ARG_DATA_TYPE_I16)
	{
		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		if (num_elems == 1)
		{
			data_format = SI_BUF_DESC_DATA_FMT_16;
		}
		else if (num_elems == 2)
		{
			data_format = SI_BUF_DESC_DATA_FMT_16_16;
		}
		else if (num_elems == 4)
		{
			data_format = SI_BUF_DESC_DATA_FMT_16_16_16_16;
		}
		elem_size = 2 * num_elems;
	}
	else if (data_type == SI_OPENCL_KERNEL_ARG_DATA_TYPE_I32)
	{
		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		if (num_elems == 1)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32;
		}
		else if (num_elems == 2)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32_32;
		}
		else if (num_elems == 3)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32;
		}
		else if (num_elems == 4)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32_32;
		}
		elem_size = 4 * num_elems;
	}
	else if (data_type == SI_OPENCL_KERNEL_ARG_DATA_TYPE_FLOAT)
	{
		num_format = SI_BUF_DESC_NUM_FMT_FLOAT;
		if (num_elems == 1)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32;
		}
		else if (num_elems == 2)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32_32;
		}
		else if (num_elems == 3)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32;
		}
		else if (num_elems == 4)
		{
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32_32;
		}
		elem_size = 4 * num_elems;
	}
	assert(data_format != SI_BUF_DESC_DATA_FMT_INVALID);
	assert(num_format != SI_BUF_DESC_NUM_FMT_INVALID);

	/* FIXME For now, storing the device_ptr in the UAV table and
	 * setting the descriptor base address to zero */
	buffer_desc->base_addr = 0;
	buffer_desc->num_format = num_format;
	buffer_desc->data_format = data_format;
	buffer_desc->elem_size = elem_size;

	return;
}


#if 0
void si_opencl_kernel_debug_ndrange_state(struct si_opencl_kernel_t *kernel,
		struct si_ndrange_t *ndrange)
{
	struct si_buffer_desc_t buf_desc;
	struct si_opencl_kernel_arg_t *arg;
	int i;

	si_opencl_debug("\n");
	si_opencl_debug("================ Initialization Summary ================"
		"\n");
	si_opencl_debug("\n");

	/* Dump address ranges */
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\t| Memory Space  |    Start   |     End    |\n");
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\t| UAV table     | %10u | %10u |\n",
		SI_EMU_UAV_TABLE_START,
		SI_EMU_UAV_TABLE_START + SI_EMU_UAV_TABLE_SIZE-1);
	si_opencl_debug("\t| CB table      | %10u | %10u |\n",
		SI_EMU_CONSTANT_BUFFER_TABLE_START,
		SI_EMU_CONSTANT_BUFFER_TABLE_START +
		SI_EMU_CONSTANT_BUFFER_TABLE_SIZE-1);
	si_opencl_debug("\t| RSRC table    | %10u | %10u |\n",
		SI_EMU_RESOURCE_TABLE_START,
		SI_EMU_RESOURCE_TABLE_START +
		SI_EMU_RESOURCE_TABLE_SIZE - 1);
	si_opencl_debug("\t| Constant mem  | %10u | %10u |\n",
		SI_EMU_CONSTANT_MEMORY_START,
		SI_EMU_CONSTANT_MEMORY_START+SI_EMU_CONSTANT_MEMORY_SIZE - 1);
	si_opencl_debug("\t| Global mem    | %10u | %10u |\n",
		SI_EMU_GLOBAL_MEMORY_START, 0xFFFFFFFFU);
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\n");

	/* Dump SREG initialization */
	unsigned int userElementCount =
		kernel->bin_file->enc_dict_entry_southern_islands->
		userElementCount;
	struct si_bin_enc_user_element_t* userElements =
		kernel->bin_file->enc_dict_entry_southern_islands->
		userElements;
	si_opencl_debug("Scalar register initialization prior to execution:\n");
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\t|  Registers  |   Initialization Value    |\n");
	si_opencl_debug("\t-------------------------------------------\n");
	for (int i = 0; i < userElementCount; i++)
	{
		if (userElements[i].dataClass == IMM_CONST_BUFFER)
		{

			if (userElements[i].userRegCount > 1)
			{
				/* FIXME Replace CB calculation with value
				 * from CB table once it's implemented */
				si_opencl_debug("\t| SREG[%2d:%2d] |  CB%1d "
					"desc   (%10u)  |\n",
					userElements[i].startUserReg,
					userElements[i].startUserReg +
					userElements[i].userRegCount - 1,
					userElements[i].apiSlot,
					SI_EMU_CALC_CB_ADDR(
						userElements[i].apiSlot));
			}
			else
			{
				/* FIXME Replace CB calculation with value
				 * from CB table once it's implemented */
				si_opencl_debug("\t| SREG[%2d]    |  CB%1d "
					"desc   (%10u)  |\n",
					userElements[i].startUserReg,
					userElements[i].apiSlot,
					SI_EMU_CALC_CB_ADDR(
						userElements[i].apiSlot));
			}
		}
		else if (userElements[i].dataClass == IMM_UAV)
		{
			si_opencl_debug("\t| SREG[%2d:%2d] |  UAV%-2d "
				"desc (%10u)  |\n",
				userElements[i].startUserReg,
				userElements[i].startUserReg +
				userElements[i].userRegCount - 1,
				userElements[i].apiSlot,
				si_emu_get_uav_base_addr(
					userElements[i].apiSlot));
		}
		else if (userElements[i].dataClass ==
			PTR_CONST_BUFFER_TABLE)
		{
			si_opencl_debug("\t| SREG[%2d:%2d] |  CONSTANT BUFFER "
				"TABLE    |\n",
				userElements[i].startUserReg,
				userElements[i].startUserReg +
				userElements[i].userRegCount - 1);
		}
		else if (userElements[i].dataClass == PTR_UAV_TABLE)
		{
			si_opencl_debug("\t| SREG[%2d:%2d] |  UAV "
				"TABLE                |\n",
				userElements[i].startUserReg,
				userElements[i].startUserReg +
				userElements[i].userRegCount - 1);
		}
		else
		{
			assert(0);
		}
	}
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\n");

	/* Dump constant buffer 1 (argument mapping) */
	si_opencl_debug("Constant buffer 1 initialization (kernel arguments):\n");
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\t| CB1 Idx | Arg # |   Size   |    Name    |\n");
	si_opencl_debug("\t-------------------------------------------\n");
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
		{
			fatal("kernel '%s': argument '%s' has not been "
				"assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);
		}

		/* Process argument depending on its type */
		switch (arg->kind)
		{

		case SI_OPENCL_KERNEL_ARG_KIND_VALUE:
		{
			/* Value copied directly into device constant
			 * memory */
			assert(arg->size);
			si_opencl_debug("\t| CB1[%2d] | %5d | %8d | %-10s |\n",
				arg->pointer.constant_offset/4, i, arg->size,
				arg->name);

			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_POINTER:
		{
			if (arg->pointer.mem_type !=
				SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL)
			{
				si_opencl_debug("\t| CB1[%2d] | %5d | %8d | %-10s"
					" |\n", arg->pointer.constant_offset/4,
					i, arg->size,
					arg->name);
			}
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_IMAGE:
		{
			assert(0);
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_SAMPLER:
		{
			assert(0);
			break;
		}

		default:
		{
			fatal("%s: argument type not reconized",
				__FUNCTION__);
		}

		}
	}
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\n");

	/* Dump constant buffers */
	si_opencl_debug("Constant buffer mappings into global memory:\n");
	si_opencl_debug("\t--------------------------------------\n");
	si_opencl_debug("\t|  CB  |        Address Range        |\n");
	si_opencl_debug("\t--------------------------------------\n");
	for (i = 0; i < si_emu_num_mapped_const_buffers; i++)
	{
		si_opencl_debug("\t| CB%-2d |   [%10u:%10u]   |\n", i,
			SI_EMU_CALC_CB_ADDR(i),
			SI_EMU_CALC_CB_ADDR(i)+SI_EMU_CONSTANT_BUFFER_SIZE-1);
	}
	si_opencl_debug("\t--------------------------------------\n");
	si_opencl_debug("\n");

	/* Dump UAVs */
	si_opencl_debug("Initialized UAVs:\n");
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\t|  UAV  |    Address   |    Arg Name      |\n");
	si_opencl_debug("\t-------------------------------------------\n");
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
		{
			fatal("kernel '%s': argument '%s' has not been "
				"assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);
		}

		/* Process argument depending on its type */
		if (arg->kind == SI_OPENCL_KERNEL_ARG_KIND_POINTER &&
			arg->pointer.mem_type !=
			SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL)
		{
			buf_desc = si_emu_get_uav_table_entry(
				arg->pointer.buffer_num);

			si_opencl_debug("\t| UAV%-2d | %10u   |    %-12s  |\n",
				arg->pointer.buffer_num,
				(unsigned int)buf_desc.base_addr,
				arg->name);
		}
	}
	si_opencl_debug("\t-------------------------------------------\n");
	si_opencl_debug("\n");
	si_opencl_debug("========================================================"
		"\n");
}
#endif


unsigned int si_opencl_kernel_get_work_group_info(
	struct si_opencl_kernel_t *kernel, unsigned int name,
	struct mem_t *mem, unsigned int addr, unsigned int size)
{
	unsigned int size_ret = 0;
	void *info = NULL;

	unsigned int max_work_group_size = 256;  /* FIXME */

	unsigned int compile_work_group_size[3];
	compile_work_group_size[0] = 1;  /* FIXME */
	compile_work_group_size[1] = 1;  /* FIXME */
	compile_work_group_size[2] = 1;  /* FIXME */

	int sixtyfour = 64; /* FIXME change to wavefront size */

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
		info = &kernel->mem_size_local;
		size_ret = 4;
		break;
	}
	
	case 0x11b3:  /* CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE */
	{
		info = &sixtyfour;
		size_ret = 4;
		break;
	}
	case 0x11b4:  /* CL_KERNEL_PRIVATE_MEM_SIZE */
	{
		info = &kernel->mem_size_private;
		size_ret = 4;
		break;
	}
	default:
		fatal("%s: invalid or not implemented value for 'name' "
			"(0x%x)", __FUNCTION__, name);
			
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}

static enum si_opencl_kernel_arg_data_type_t 
	si_opencl_kernel_arg_get_data_type(char *type_str)
{
	enum si_opencl_kernel_arg_data_type_t data_type = 0;

	if (!strcmp(type_str, "i1"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_I1;
		fatal("%s: Boolean arguments not supported", __FUNCTION__);
	}
	else if (!strcmp(type_str, "i8"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_I8;
	}
	else if (!strcmp(type_str, "i16"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_I16;
	}
	else if (!strcmp(type_str, "i32"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_I32;
	}
	else if (!strcmp(type_str, "i64"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_I64;
	}
	else if (!strcmp(type_str, "float"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_FLOAT;
	}
	else if (!strcmp(type_str, "double"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_DOUBLE;
	}
	else if (!strcmp(type_str, "struct"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_STRUCT;
	}
	else if (!strcmp(type_str, "union"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_UNION;
	}
	else if (!strcmp(type_str, "event"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_EVENT;
	}
	else if (!strcmp(type_str, "opaque"))
	{
		data_type = SI_OPENCL_KERNEL_ARG_DATA_TYPE_OPAQUE;
	}
	else
	{
		fatal("%s: Unsupported metadata data type (%s)", 
			__FUNCTION__, type_str);
	}

	return data_type;
}

static enum si_opencl_kernel_arg_mem_type_t 
	si_opencl_kernel_arg_get_mem_type(char *type_str)
{
	enum si_opencl_kernel_arg_mem_type_t mem_type = 0;

	if (!strcmp(type_str, "g"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_GLOBAL;
	}
	else if (!strcmp(type_str, "p"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_PRIVATE;
	}
	else if (!strcmp(type_str, "l"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_LOCAL;
	}
	else if (!strcmp(type_str, "uav"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_UAV;
	}
	else if (!strcmp(type_str, "c"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_CONSTANT;
	}
	else if (!strcmp(type_str, "r"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_GDS;
	}
	else if (!strcmp(type_str, "hl"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL;
	}
	else if (!strcmp(type_str, "hp"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_PRIVATE;
	}
	else if (!strcmp(type_str, "hc"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_CONSTANT;
	}
	else if (!strcmp(type_str, "hr"))
	{
		mem_type = SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_GDS;
	}
	else
	{
		fatal("%s: Unsupported metadata mem type (%s)", 
			__FUNCTION__, type_str);
	}

	return mem_type;
}

static unsigned int si_opencl_kernel_arg_get_data_size(
	enum si_opencl_kernel_arg_data_type_t data_type)
{
	unsigned int elem_size = 0;

	switch (data_type)
	{
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_I16:
		elem_size = 2;
		break;
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_I32:
		elem_size = 4;
		break;
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_I64:
		elem_size = 8;
		break;
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_FLOAT:
		elem_size = 4;
		break;
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_DOUBLE:
		elem_size = 8;
		break;

	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_I8:
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_STRUCT:
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_UNION:
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_EVENT:
	case SI_OPENCL_KERNEL_ARG_DATA_TYPE_OPAQUE:
		elem_size = 1;
		break;

	default:
		fatal("%s: Unsupported data type: (%d)", __FUNCTION__, 
			data_type);
		break;
	}
	assert(elem_size);

	return elem_size;
}

static enum si_opencl_kernel_arg_access_type_t 
	si_opencl_kernel_get_access_type(char *access_str)
{
	enum si_opencl_kernel_arg_access_type_t access_type;

	/* Set the access type */
	if (!strcmp(access_str, "RO"))
	{
		access_type = SI_OPENCL_KERNEL_ARG_READ_ONLY;
	}
	else if (!strcmp(access_str, "WO"))
	{
		access_type = SI_OPENCL_KERNEL_ARG_WRITE_ONLY;
	}
	else if (!strcmp(access_str, "RW"))
	{
		access_type = SI_OPENCL_KERNEL_ARG_READ_WRITE;
	}
	else
	{
		fatal("%s: Unrecognized memory access type: %s\n", 
			__FUNCTION__, access_str);
	}

	return access_type;
}
