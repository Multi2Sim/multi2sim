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


#include <arch/southern-islands/asm/arg.h>
#include <arch/southern-islands/emu/isa.h>
#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/timing/compute-unit.h>
#include <arch/southern-islands/timing/gpu.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/timing/cpu.h>
#include <lib/class/list.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <memory/module.h>

#include "opencl.h"
#include "si-kernel.h"
#include "si-program.h"


static char *opencl_err_si_kernel_symbol =
	"\tThe ELF file analyzer is trying to find a name in the ELF symbol table.\n"
	"\tIf it is not found, it probably means that your application is requesting\n"
	"\texecution of a kernel function that is not present in the encoded binary.\n"
	"\tPlease, check the parameters passed to the 'clCreateKernel' function in\n"
	"\tyour application.\n"
	"\tThis could be also a symptom of compiling an OpenCL kernel source on a\n"
	"\tmachine with an installation of the AMD SDK (using 'm2s-clcc') but\n"
	"\twith an incorrect or missing installation of the GPU driver. In this case,\n"
	"\tthe tool will still compile the kernel into LLVM, but the ISA section will\n"
	"\tbe missing in the kernel binary.\n";

static char *opencl_err_si_kernel_metadata =
	"\tThe kernel binary loaded by your application is a valid ELF file. In this\n"
	"\tfile, a '.rodata' section contains specific information about the OpenCL\n"
	"\tkernel. However, this information is only partially supported by Multi2Sim.\n"
	"\tTo request support for this error, please email 'development@multi2sim.org'.\n";

static void opencl_si_create_buffer_desc(unsigned int base_addr,
	unsigned int size, int num_elems, 
	SIArgDataType data_type,
	struct si_buffer_desc_t *buffer_desc);



/*
 * Kernel
 */

static void opencl_si_kernel_expect(struct opencl_si_kernel_t *kernel,
		struct list_t *token_list, char *head_token)
{
	char *token;

	token = str_token_list_first(token_list);
	if (strcmp(token, head_token))
		fatal("%s: token '%s' expected, '%s' found.\n%s",
				__FUNCTION__, head_token, token,
				opencl_err_si_kernel_metadata);
}


static void opencl_si_kernel_expect_int(struct opencl_si_kernel_t *kernel,
		struct list_t *token_list)
{
	char *token;
	int err;

	token = str_token_list_first(token_list);
	str_to_int(token, &err);
	if (err)
		fatal("%s: integer number expected, '%s' found.\n%s",
				__FUNCTION__, token,
				opencl_err_si_kernel_metadata);
}


static void opencl_si_kernel_expect_count(struct opencl_si_kernel_t *kernel,
		struct list_t *token_list, int count)
{
	char *head_token;

	head_token = str_token_list_first(token_list);
	if (token_list->count != count)
		fatal("%s: %d tokens expected for '%s', %d found.\n%s",
				__FUNCTION__, count, head_token, token_list->count,
				opencl_err_si_kernel_metadata);
}


static void opencl_si_kernel_load_metadata_v3(struct opencl_si_kernel_t *kernel)
{
	struct elf_buffer_t *buffer = &kernel->metadata_buffer;
	SIArg *arg;
	struct list_t *token_list;

	char line[MAX_STRING_SIZE];
	char *token;
	int err;

	for (;;)
	{
		/* Read the next line */
		elf_buffer_read_line(buffer, line, sizeof line);
		opencl_debug("\t%s\n", line);
		token_list = str_token_list_create(line, ":;");

		/* Stop when ARGEND is found or line is empty */
		token = str_token_list_first(token_list);
		if (!token_list->count || !strcmp(token, "ARGEND"))
		{
			str_token_list_free(token_list);
			break;
		}


		/*
		 * Kernel argument metadata
		 */

		/* Value */
		if (!strcmp(token, "value"))
		{
			/* 6 tokens expected */
			opencl_si_kernel_expect_count(kernel, token_list, 6);

			/* Token 1 - Name */
			token = str_token_list_shift(token_list);
			arg = new(SIArg, SIArgTypeValue, token);

			/* Token 2 - Data type */
			token = str_token_list_shift(token_list);
			arg->value.data_type = str_map_string_err(&si_arg_data_type_map,
					token, &err);
			if (err)
				fatal("%s: invalid data type '%s'.\n%s",
					__FUNCTION__, token, 
					opencl_err_si_kernel_metadata);

			/* Token 3 - Number of elements */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->value.num_elems = atoi(token);
			assert(arg->value.num_elems > 0);

			/* Token 4 - Constant buffer */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			opencl_si_kernel_expect(kernel, token_list, "1");
			arg->value.constant_buffer_num = atoi(token);

			/* Token 5 - Conastant offset */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->value.constant_offset = atoi(token);

			/* Infer argument size from its type */
			arg->size = arg->value.num_elems *
				SIArgGetDataSize(
					arg->value.data_type);

			/* Debug */
			opencl_debug("\targument '%s' - value stored in "
				"constant buffer %d at offset %d\n",
				arg->name->text, arg->value.constant_buffer_num,
				arg->value.constant_offset);

			/* Add argument */
			ListAdd(kernel->arg_list, asObject(arg));
			str_token_list_free(token_list);
			continue;
		}

		/* Pointer */
		if (!strcmp(token, "pointer"))
		{
			/* APP SDK 2.5 supplies 9 tokens, 2.6 supplies
			 * 10 tokens. Metadata version 3:1:104 (as specified
			 * in entry 'version') uses 12 items. */
			opencl_si_kernel_expect_count(kernel, token_list, 12);

			/* Token 1 - Name */
			token = str_token_list_shift(token_list);
			arg = new(SIArg, SIArgTypePointer, token);

			/* Token 2 - Data type */
			token = str_token_list_shift(token_list);
			arg->pointer.data_type = str_map_string_err(
				&si_arg_data_type_map, token, &err);
			if (err)
				fatal("%s: invalid data type '%s'.\n%s",
					__FUNCTION__, token, 
					opencl_err_si_kernel_metadata);

			/* Token 3 - Number of elements
			 * Arrays of pointers not supported, 
			 * only "1" allowed. */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			opencl_si_kernel_expect(kernel, token_list, "1");
			arg->pointer.num_elems = atoi(token);

			/* Token 4 - Constant buffer */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->pointer.constant_buffer_num = atoi(token);

			/* Token 5 - Constant offset */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->pointer.constant_offset = atoi(token);

			/* Token 6 - Memory scope */
			token = str_token_list_shift(token_list);
			arg->pointer.scope = str_map_string_err(
				&si_arg_scope_map, token, &err);
			if (err)
				fatal("%s: invalid scope '%s'.\n%s",
					__FUNCTION__, token, 
					opencl_err_si_kernel_metadata);

			/* Token 7 - Buffer number */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->pointer.buffer_num = atoi(token);

			/* Token 8 - Alignment */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->pointer.alignment = atoi(token);

			/* Token 9 - Access type */
			token = str_token_list_shift(token_list);
			arg->pointer.access_type = str_map_string_err(
				&si_arg_access_type_map, token, &err);
			if (err)
				fatal("%s: invalid access type '%s'.\n%s",
					__FUNCTION__, token, 
					opencl_err_si_kernel_metadata);

			/* Token 10 - ??? */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect(kernel, token_list, "0");

			/* Token 11 - ??? */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect(kernel, token_list, "0");

			/* Data size inferred here is always 4, the size 
			 * of a pointer. */
			arg->size = 4;

			/* Debug */
			opencl_debug("\targument '%s' - Pointer stored in "
				"constant buffer %d at offset %d\n",
				arg->name->text, arg->pointer.constant_buffer_num,
				arg->pointer.constant_offset);

			/* Add argument */
			ListAdd(kernel->arg_list, asObject(arg));
			str_token_list_free(token_list);
			continue;
		}

		/* Image */
		if (!strcmp(token, "image"))
		{
			/* 7 tokens expected */
			opencl_si_kernel_expect_count(kernel, token_list, 7);

			/* Token 1 - Name */
			token = str_token_list_shift(token_list);
			arg = new(SIArg, SIArgTypeImage, token);

			/* Token 2 - Dimension */
			token = str_token_list_shift(token_list);
			arg->image.dimension = str_map_string_err(&si_arg_dimension_map,
					token, &err);
			if (err)
				fatal("%s: invalid image dimensions '%s'.\n%s",
					__FUNCTION__, token, opencl_err_si_kernel_metadata);

			/* Token 3 - Access type */
			token = str_token_list_shift(token_list);
			arg->image.access_type = str_map_string_err(&si_arg_access_type_map,
					token, &err);
			if (err)
				fatal("%s: invalid access type '%s'.\n%s",
					__FUNCTION__, token, opencl_err_si_kernel_metadata);

			/* Token 4 - UAV */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->image.uav = atoi(token);

			/* Token 5 - Constant buffer */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->image.constant_buffer_num = atoi(token);

			/* Token 6 - Constant offset */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->image.constant_offset = atoi(token);

			/* Add argument */
			ListAdd(kernel->arg_list, asObject(arg));
			str_token_list_free(token_list);
			continue;
		}

		/* Entry 'sampler'. */
		if (!strcmp(token, "sampler"))
		{
			/* 5 tokens expected */
			opencl_si_kernel_expect_count(kernel, token_list, 5);

			/* Token 1 - Name */
			token = str_token_list_shift(token_list);
			arg = new(SIArg, SIArgTypeSampler, token);

			/* Token 2 - ID */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->sampler.id = atoi(token);

			/* Token 3 - Location */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->sampler.location = atoi(token);

			/* Token 4 - Value */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			arg->sampler.value = atoi(token);

			/* Add argument */
			ListAdd(kernel->arg_list, asObject(arg));
			str_token_list_free(token_list);
			continue;
		}

		/*
		 * Non-kernel argument metadata
		 */

		/* Memory
		 * Used to let the GPU know how much local and private memory
		 * is required for a kernel, where it should be allocated,
		 * as well as other information. */
		if (!strcmp(token, "memory"))
		{
			/* Token 1 - Memory scope */
			token = str_token_list_shift(token_list);
			if (!strcmp(token, "hwprivate"))
			{
				/* FIXME Add support for private memory by
				 * adding space in global memory */

				/* Token 2 - ??? */
				token = str_token_list_shift(token_list);
				opencl_si_kernel_expect(kernel, token_list, "0");
			}
			else if (!strcmp(token, "hwregion"))
			{
				/* 2 more tokens expected */
				opencl_si_kernel_expect_count(kernel, token_list, 2);

				/* Token 2 - ??? */
				token = str_token_list_shift(token_list);
				opencl_si_kernel_expect(kernel, token_list, "0");
			}
			else if (!strcmp(token, "hwlocal"))
			{
				/* 2 more tokens expected */
				opencl_si_kernel_expect_count(kernel, token_list, 2);

				/* Token 2 - Size of local memory */
				token = str_token_list_shift(token_list);
				opencl_si_kernel_expect_int(kernel, token_list);
				kernel->mem_size_local = atoi(token);
			}
			else if (!strcmp(token, "datareqd"))
			{
				/* 1 more token expected */
				opencl_si_kernel_expect_count(kernel, token_list, 1);
			}
			else if (!strcmp(token, "uavprivate"))
			{
				/* 2 more tokens expected */
				opencl_si_kernel_expect_count(kernel, token_list, 2);
			}
			else
			{
				fatal("%s: not supported metadata '%s'.\n%s",
						__FUNCTION__, token, opencl_err_si_kernel_metadata);
			}

			/* Next */
			str_token_list_free(token_list);
			continue;
		}

		/* Function
		 * Used for multi-kernel compilation units. */
		if (!strcmp(token, "function"))
		{
			/* Expect 3 token */
			opencl_si_kernel_expect_count(kernel, token_list, 3);

			/* Token 1 - ??? */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect(kernel, token_list, "1");

			/* Token 2 - Function ID */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect_int(kernel, token_list);
			kernel->func_uniqueid = atoi(token);

			/* Next */
			str_token_list_free(token_list);
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
		if (!strcmp(token, "reflection"))
		{
			/* Expect 3 tokens */
			opencl_si_kernel_expect_count(kernel, token_list, 3);

			/* Next */
			str_token_list_free(token_list);
			continue;
		}

		/* Privateid
		 * Format: privateid:<id>
		 * Observed first in version 3:1:104 of metadata.
		 * Not sure what this entry is for.
		 */
		if (!strcmp(token, "privateid"))
		{
			/* Expect 2 tokens */
			opencl_si_kernel_expect_count(kernel, token_list, 2);

			/* Next */
			str_token_list_free(token_list);
			continue;
		}

		/* Constarg
		 * Format: constarg:<arg_id>:<arg_name>
		 * Observed first in version 3:1:104 of metadata.
		 * It shows up when an argument is declared as
		 * '__global const'. Entry ignored here. */
		if (!strcmp(token, "constarg"))
		{
			/* Expect 3 tokens */
			opencl_si_kernel_expect_count(kernel, token_list, 3);

			/* Next */
			str_token_list_free(token_list);
			continue;
		}

		/* Device
		 * Device that the kernel was compiled for. */
		if (!strcmp(token, "device"))
		{
			/* Expect 2 tokens */
			opencl_si_kernel_expect_count(kernel, token_list, 2);

			/* Next */
			str_token_list_free(token_list);
			continue;
		}

		/* Uniqueid
		 * A mapping between a kernel and its unique ID */
		if (!strcmp(token, "uniqueid"))
		{
			/* Expect 2 tokens */
			opencl_si_kernel_expect_count(kernel, token_list, 2);

			/* Next */
			str_token_list_free(token_list);
			continue;
		}

		/* Uavid 
		 * ID of a raw UAV */
		if (!strcmp(token, "uavid"))
		{
			/* Expect 2 tokens */
			opencl_si_kernel_expect_count(kernel, token_list, 2);

			/* Next */
			str_token_list_free(token_list);
			continue;
		}

		/* Crash when uninterpreted entries appear */
		fatal("kernel '%s': unknown metadata entry '%s'",
			kernel->name, token);
	}
}


static void opencl_si_kernel_load_metadata(struct opencl_si_kernel_t *kernel)
{
	struct elf_buffer_t *elf_buffer = &kernel->metadata_buffer;
	struct list_t *token_list;

	char line[MAX_STRING_SIZE];
	int version;

	/* First line example:
	 * ;ARGSTART:__OpenCL_opencl_mmul_kernel */
	elf_buffer_read_line(elf_buffer, line, sizeof line);
	token_list = str_token_list_create(line, ";:");
	opencl_si_kernel_expect(kernel, token_list, "ARGSTART");
	opencl_si_kernel_expect_count(kernel, token_list, 2);
	str_token_list_free(token_list);

	/* Second line contains version info. Example:
	 * ;version:3:1:104 */
	elf_buffer_read_line(elf_buffer, line, sizeof line);
	token_list = str_token_list_create(line, ";:");
	opencl_si_kernel_expect(kernel, token_list, "version");
	opencl_si_kernel_expect_count(kernel, token_list, 4);
	str_token_list_shift(token_list);
	version = atoi(str_token_list_first(token_list));
	str_token_list_free(token_list);

	/* Parse rest of the metadata based on version number */
	switch (version)
	{
	case 3:

		opencl_si_kernel_load_metadata_v3(kernel);
		break;

	default:

		fatal("%s: metadata version %d not supported.\n%s",
				__FUNCTION__, version, opencl_err_si_kernel_metadata);
	}
}


struct opencl_si_kernel_t *opencl_si_kernel_create(int id,
		struct opencl_si_program_t *program, char *name)
{
	struct opencl_si_kernel_t *kernel;
	struct elf_file_t *elf_file;
	char symbol_name[MAX_STRING_SIZE];

	struct elf_symbol_t *metadata_symbol;
	struct elf_symbol_t *header_symbol;
	struct elf_symbol_t *kernel_symbol;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_si_kernel_t));
	kernel->id = id;
	kernel->name = xstrdup(name);
	kernel->program = program;
	kernel->arg_list = new(List);

	/* Check that program has been built */
	elf_file = program->elf_file;
	if (!elf_file)
		fatal("%s: program not built", __FUNCTION__);

	/* Get symbols from program binary */
	snprintf(symbol_name, sizeof symbol_name, "__OpenCL_%s_metadata", name);
	metadata_symbol = elf_symbol_get_by_name(elf_file, symbol_name);
	snprintf(symbol_name, sizeof symbol_name, "__OpenCL_%s_header", name);
	header_symbol = elf_symbol_get_by_name(elf_file, symbol_name);
	snprintf(symbol_name, sizeof symbol_name, "__OpenCL_%s_kernel", name);
	kernel_symbol = elf_symbol_get_by_name(elf_file, symbol_name);
	if (!metadata_symbol || !header_symbol || !kernel_symbol)
		fatal("%s: invalid kernel function (ELF symbol '__OpenCL_%s_xxx missing')\n%s",
				__FUNCTION__, name, opencl_err_si_kernel_symbol);

	/* Read contents pointed to by symbols */
	elf_symbol_read_content(elf_file, metadata_symbol, &kernel->metadata_buffer);
	elf_symbol_read_content(elf_file, header_symbol, &kernel->header_buffer);
	elf_symbol_read_content(elf_file, kernel_symbol, &kernel->kernel_buffer);
	opencl_debug("\tmetadata symbol: offset=0x%x, size=%u\n",
			metadata_symbol->value, kernel->metadata_buffer.size);
	opencl_debug("\theader symbol: offset=0x%x, size=%u\n",
			header_symbol->value, kernel->header_buffer.size);
	opencl_debug("\tkernel symbol: offset=0x%x, size=%u\n",
			kernel_symbol->value, kernel->kernel_buffer.size);

	/* Create and parse kernel binary (internal ELF).
	 * The internal ELF is contained in the buffer pointer to by
	 * the 'kernel' symbol. */
	snprintf(symbol_name, sizeof symbol_name, "kernel<%s>.InternalELF", name);
	kernel->bin_file = SIBinaryCreate(kernel->kernel_buffer.ptr,
		kernel->kernel_buffer.size, symbol_name);

	/* Load metadata */
	opencl_si_kernel_load_metadata(kernel);

	/* Return */
	return kernel;
}


void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel)
{
	/* Free argument list */
	ListDeleteObjects(kernel->arg_list);
	delete(kernel->arg_list);

	/* Rest */
	SIBinaryFree(kernel->bin_file);
	free(kernel->name);
	free(kernel);
}


void opencl_si_kernel_create_ndrange_constant_buffers(SINDRange *ndrange,
	MMU *gpu_mmu)
{
	SIEmu *emu = ndrange->emu;

	if (gpu_mmu)
	{
		/* Allocate starting from nearest page boundary */
		if (emu->video_mem_top & gpu_mmu->page_mask)
		{
			emu->video_mem_top += gpu_mmu->page_size -
				(emu->video_mem_top & gpu_mmu->page_mask);
		}
	}

	/* Map new pages */
	mem_map(emu->video_mem, emu->video_mem_top, SI_EMU_TOTAL_CONST_BUF_SIZE,
		mem_access_read | mem_access_write);

	opencl_debug("\t%u bytes of device memory allocated at " 
		"0x%x for SI constant buffers\n", SI_EMU_TOTAL_CONST_BUF_SIZE,
		emu->video_mem_top);

	/* Create constant buffers */
	ndrange->cb_start = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_TOTAL_CONST_BUF_SIZE;
}

void opencl_si_kernel_setup_ndrange_constant_buffers(
	SINDRange *ndrange)
{
	struct si_buffer_desc_t buffer_desc;

	unsigned int zero = 0;

	float f;

	/* Constant buffer 0 */
	opencl_si_create_buffer_desc(ndrange->cb_start, SI_EMU_CONST_BUF_SIZE, 
		1, SIArgInt32, &buffer_desc);

	SINDRangeInsertBufferIntoConstantBufferTable(ndrange, &buffer_desc, 0);

	/* Constant buffer 1 */
	opencl_si_create_buffer_desc(
		ndrange->cb_start + 1*SI_EMU_CONST_BUF_SIZE, 
		SI_EMU_CONST_BUF_SIZE, 1, SIArgInt32, &buffer_desc);

	SINDRangeInsertBufferIntoConstantBufferTable(ndrange, &buffer_desc, 1);

	/* Initialize constant buffer 0 */

	/* CB0 bytes 0:15 */

	/* Global work size for the {x,y,z} dimensions */
	SINDRangeConstantBufferWrite(ndrange, 0, 0, 
		&ndrange->global_size3[0], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 4, 
		&ndrange->global_size3[1], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 8, 
		&ndrange->global_size3[2], 4);

	/* Number of work dimensions */
	SINDRangeConstantBufferWrite(ndrange, 0, 12, &ndrange->work_dim, 4);

	/* CB0 bytes 16:31 */

	/* Local work size for the {x,y,z} dimensions */
	SINDRangeConstantBufferWrite(ndrange, 0, 16, 
		&ndrange->local_size3[0], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 20, 
		&ndrange->local_size3[1], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 24, 
		&ndrange->local_size3[2], 4);

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 28, &zero, 4);

	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	SINDRangeConstantBufferWrite(ndrange, 0, 32, 
		&ndrange->group_count3[0], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 36, 
		&ndrange->group_count3[1], 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 40, 
		&ndrange->group_count3[2], 4);

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 44, &zero, 4);

	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is
	 * not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 56, &zero, 4);

	/* 0  */
	SINDRangeConstantBufferWrite(ndrange, 0, 60, &zero, 4);

	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is
	 * not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	SINDRangeConstantBufferWrite(ndrange, 0, 72, &zero, 4);

	/* FIXME Pointer to location in global buffer where math library
	 * tables start. */

	/* CB0 bytes 80:95 */

	/* 0.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	SINDRangeConstantBufferWrite(ndrange, 0, 80, &f, 4);

	/* 0.5 as IEEE-32bit float - required for math library. */
	f = 0.5f;
	SINDRangeConstantBufferWrite(ndrange, 0, 84, &f, 4);

	/* 1.0 as IEEE-32bit float - required for math library. */
	f = 1.0f;
	SINDRangeConstantBufferWrite(ndrange, 0, 88, &f, 4);

	/* 2.0 as IEEE-32bit float - required for math library. */
	f = 2.0f;
	SINDRangeConstantBufferWrite(ndrange, 0, 92, &f, 4);

	/* CB0 bytes 96:111 */

	/* Global offset for the {x,y,z} dimension of the work_item spawn */
	SINDRangeConstantBufferWrite(ndrange, 0, 96, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 100, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 104, &zero, 4);

	/* Global single dimension flat offset: x * y * z */
	SINDRangeConstantBufferWrite(ndrange, 0, 108, &zero, 4);

	/* CB0 bytes 112:127 */

	/* Group offset for the {x,y,z} dimensions of the work_item spawn */
	SINDRangeConstantBufferWrite(ndrange, 0, 112, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 116, &zero, 4);
	SINDRangeConstantBufferWrite(ndrange, 0, 120, &zero, 4);

	/* Group single dimension flat offset, x * y * z */
	SINDRangeConstantBufferWrite(ndrange, 0, 124, &zero, 4);

	/* CB0 bytes 128:143 */

	/* FIXME Offset in the global buffer where data segment exists */
	/* FIXME Offset in buffer for printf support */
	/* FIXME Size of the printf buffer */
}

void opencl_si_kernel_create_ndrange_tables(SINDRange *ndrange, MMU *gpu_mmu)
{
	SIEmu *emu = ndrange->emu;
	unsigned int size_of_tables;

	size_of_tables = SI_EMU_CONST_BUF_TABLE_SIZE + 
		SI_EMU_RESOURCE_TABLE_SIZE + SI_EMU_UAV_TABLE_SIZE;

	if (gpu_mmu)
	{
		/* Allocate starting from nearest page boundary */
		if (emu->video_mem_top & gpu_mmu->page_mask)
		{
			emu->video_mem_top += gpu_mmu->page_size -
				(emu->video_mem_top & gpu_mmu->page_mask);
		}

	}

	/* Map new pages */
	mem_map(emu->video_mem, emu->video_mem_top, size_of_tables,
		mem_access_read | mem_access_write);

	opencl_debug("\t%u bytes of device memory allocated at " 
		"0x%x for SI internal tables\n", size_of_tables,
		emu->video_mem_top);

	/* Setup internal tables */
	ndrange->const_buf_table = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_CONST_BUF_TABLE_SIZE;
	ndrange->resource_table = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_RESOURCE_TABLE_SIZE;
	ndrange->uav_table = emu->video_mem_top;
	emu->video_mem_top += SI_EMU_UAV_TABLE_SIZE;

	return;
}


void opencl_si_kernel_setup_ndrange_args(struct opencl_si_kernel_t *kernel,
		SINDRange *ndrange)
{
	SIArg *arg;
	struct opencl_si_constant_buffer_t *constant_buffer;
	struct si_buffer_desc_t buffer_desc;

	int index;
	int zero = 0;

	/* Initial top of local memory is determined by the static local memory
	 * specified in the kernel binary. Number of vector and scalar 
	 * registers used by the kernel recorded as well. */
	struct SIBinaryDictEntry *dict_entry =
			SIBinaryGetSIDictEntry(kernel->bin_file);
	ndrange->local_mem_top = kernel->mem_size_local;
	ndrange->num_sgpr_used = SIBinaryDictEntryGetNumSgpr(dict_entry);
	ndrange->num_vgpr_used = SIBinaryDictEntryGetNumVgpr(dict_entry);

	/* Kernel arguments */
	index = 0;
	ListForEach(ndrange->arg_list, arg, SIArg)
	{
		/* Check that argument was set */
		assert(arg);
		if (!arg->set)
			fatal("%s: kernel '%s': argument '%s' not set",
				__FUNCTION__, kernel->name, arg->name->text);

		opencl_debug("\targ[%d] = %s ", index, arg->name->text);

		/* Process argument depending on its type */
		switch (arg->type)
		{

		case SIArgTypeValue:

			/* Value copied directly into device constant
			 * memory */
			assert(arg->size);
			SINDRangeConstantBufferWrite(ndrange,
				arg->value.constant_buffer_num,
				arg->value.constant_offset,
				arg->value.value_ptr, arg->size);
			break;

		case SIArgTypePointer:

			switch (arg->pointer.scope)
			{

			/* Hardware local memory */
			case SIArgHwLocal:

				/* Pointer in __local scope.
				 * Argument value is always NULL, just assign
				 * space for it. */
				SINDRangeConstantBufferWrite(ndrange, 
					arg->pointer.constant_buffer_num,
					arg->pointer.constant_offset,
					&ndrange->local_mem_top, 4);

				opencl_debug("%u bytes at 0x%x", arg->size, 
					ndrange->local_mem_top);

				ndrange->local_mem_top += arg->size;

				break;

			/* UAV */
			case SIArgUAV:
			{
				opencl_debug("(0x%x)", arg->pointer.device_ptr);
				/* Create descriptor for argument */
				opencl_si_create_buffer_desc(
					arg->pointer.device_ptr,
					arg->size,
					arg->pointer.num_elems,
					arg->pointer.data_type, &buffer_desc);

				/* Add to UAV table */
				SINDRangeInsertBufferIntoUAVTable(
					ndrange, &buffer_desc,
					arg->pointer.buffer_num);

				/* Write 0 to CB1 */
				SINDRangeConstantBufferWrite(ndrange, 
					arg->pointer.constant_buffer_num,
					arg->pointer.constant_offset,
					&zero, 4);

				break;
			}

			/* Hardware constant memory */
			case SIArgHwConstant:
			{
				opencl_si_create_buffer_desc(
					arg->pointer.device_ptr,
					arg->size,
					arg->pointer.num_elems,
					arg->pointer.data_type, &buffer_desc);

				/* Data stored in hw constant memory 
				 * uses a 4-byte stride */
				buffer_desc.stride = 4;

				/* Add to Constant Buffer table */
				SINDRangeInsertBufferIntoConstantBufferTable(
					ndrange, &buffer_desc,
					arg->pointer.buffer_num);

				/* Write 0 to CB1 */
				SINDRangeConstantBufferWrite(ndrange, 
					arg->pointer.constant_buffer_num,
					arg->pointer.constant_offset,
					&zero, 4);

				break;
			}

			default:

				fatal("%s: not implemented memory scope",
						__FUNCTION__);
			}

			break;

		case SIArgTypeImage:

			fatal("%s: type 'image' not implemented", __FUNCTION__);
			break;

		case SIArgTypeSampler:

			fatal("%s: type 'sampler' not implemented", 
				__FUNCTION__);
			break;

		default:

			fatal("%s: argument type not recognized (%d)",
				__FUNCTION__, arg->type);

		}
		opencl_debug("\n");

		/* Next */
		index++;
	}

	/* Add program-wide constant buffers to the ND-range. 
	 * Program-wide constant buffers start at number 2. */
	for (index = 2; index < SI_EMU_MAX_CONST_BUFS; index++) 
	{
		constant_buffer = list_get(
			kernel->program->constant_buffer_list, index);

		if (!constant_buffer)
			break;

		opencl_si_create_buffer_desc(
			ndrange->cb_start + SI_EMU_CONST_BUF_SIZE*index,
			constant_buffer->size,
			4,
			SIArgFloat,
			&buffer_desc);

		/* Data stored in hw constant memory 
		 * uses a 16-byte stride */
		buffer_desc.stride = 16; // XXX Use or don't use?

		/* Add to Constant Buffer table */
		SINDRangeInsertBufferIntoConstantBufferTable(ndrange, 
			&buffer_desc, index);
	}
}

static void opencl_si_create_buffer_desc(unsigned int base_addr,
	unsigned int size, int num_elems, 
	SIArgDataType data_type,
	struct si_buffer_desc_t *buffer_desc)
{
	int num_format;
	int data_format;
	int elem_size;

	/* Zero-out the buffer resource descriptor */
	assert(sizeof(struct si_buffer_desc_t) == 16);
	memset(buffer_desc, 0, sizeof(struct si_buffer_desc_t));

	num_format = SI_BUF_DESC_NUM_FMT_INVALID;
	data_format = SI_BUF_DESC_DATA_FMT_INVALID;

	switch (data_type)
	{

	case SIArgInt8:
	case SIArgUInt8:

		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		switch (num_elems)
		{
		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_8;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_8_8;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_8_8_8_8;
			break;

		default:
			fatal("%s: invalid number of i8/u8 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 1 * num_elems;
		break;

	case SIArgInt16:
	case SIArgUInt16:

		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		switch (num_elems)
		{

		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_16;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_16_16;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_16_16_16_16;
			break;

		default:
			fatal("%s: invalid number of i16/u16 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 2 * num_elems;
		break;

	case SIArgInt32:
	case SIArgUInt32:

		num_format = SI_BUF_DESC_NUM_FMT_SINT;
		switch (num_elems)
		{

		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_32;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_32_32;
			break;

		case 3:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32_32;
			break;

		default:
			fatal("%s: invalid number of i32/u32 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 4 * num_elems;
		break;

	case SIArgFloat:

		num_format = SI_BUF_DESC_NUM_FMT_FLOAT;
		switch (num_elems)
		{
		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_32;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_32_32;
			break;

		case 3:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32;
			break;

		case 4:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32_32;
			break;

		default:
			fatal("%s: invalid number of float elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 4 * num_elems;
		break;

	case SIArgDouble:

		num_format = SI_BUF_DESC_NUM_FMT_FLOAT;
		switch (num_elems)
		{
		case 1:
			data_format = SI_BUF_DESC_DATA_FMT_32_32;
			break;

		case 2:
			data_format = SI_BUF_DESC_DATA_FMT_32_32_32_32;
			break;

		default:
			fatal("%s: invalid number of double elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 8 * num_elems;
		break;
	case SIArgStruct:

		num_format = SI_BUF_DESC_NUM_FMT_UINT;
		data_format = SI_BUF_DESC_DATA_FMT_8;
		elem_size = 1;
		break;

	default:
		fatal("%s: invalid data type for SI buffer (%d)",
			__FUNCTION__, data_type);
	}
	assert(num_format != SI_BUF_DESC_NUM_FMT_INVALID);
	assert(data_format != SI_BUF_DESC_DATA_FMT_INVALID);

	buffer_desc->base_addr = base_addr;
	buffer_desc->num_format = num_format;
	buffer_desc->data_format = data_format;
	assert(!(size % elem_size));  
	buffer_desc->elem_size = elem_size;
	buffer_desc->num_records = size/elem_size;

	return;
}

void opencl_si_kernel_debug_ndrange_state(struct opencl_si_kernel_t *kernel, 
	SINDRange *ndrange) 
{
	SIEmu *emu = ndrange->emu;

	int i;
	int index;

	struct si_buffer_desc_t buffer_desc;
	SIArg *arg;

        si_isa_debug("\n");
        si_isa_debug("================ Initialization Summary ================"
                "\n");
        si_isa_debug("Kernel: %s\n", kernel->name);
        si_isa_debug("ND-Range: %d\n", ndrange->id);
        si_isa_debug("\n");

        /* Table locations */
        si_isa_debug("NDRange table locations:\n");
        si_isa_debug("\t------------------------------------------------\n");
        si_isa_debug("\t|    Name            |    Address Range        |\n");
        si_isa_debug("\t------------------------------------------------\n");
        si_isa_debug("\t| Const Buffer table | [0x%8x:0x%8x] |\n",
                ndrange->const_buf_table,
                ndrange->const_buf_table +
                SI_EMU_CONST_BUF_TABLE_SIZE-1);
        si_isa_debug("\t| Resource table     | [0x%8x:0x%8x] |\n",
                ndrange->resource_table,
                ndrange->resource_table +
                SI_EMU_RESOURCE_TABLE_SIZE-1);
        si_isa_debug("\t| UAV table          | [0x%8x:0x%8x] |\n",
                ndrange->uav_table,
                ndrange->uav_table + SI_EMU_UAV_TABLE_SIZE - 1);
        si_isa_debug("\t------------------------------------------------\n");
        si_isa_debug("\n");

        /* SREG initialization */
	struct SIBinaryDictEntry *dict_entry =
			SIBinaryGetSIDictEntry(kernel->bin_file);
        unsigned int userElementCount = SIBinaryDictEntryGetNumUserElements(dict_entry);
	struct SIBinaryUserElement* userElements = SIBinaryDictEntryGetUserElements(dict_entry);
        si_isa_debug("Scalar register initialization prior to execution:\n");
        si_isa_debug("\t-------------------------------------------\n");
        si_isa_debug("\t|  Registers  |   Initialization Value    |\n");
        si_isa_debug("\t-------------------------------------------\n");
        for (int i = 0; i < userElementCount; i++)
        {
                if (userElements[i].dataClass == SIBinaryUserDataConstBuffer)
                {
                        /* Constant buffer descriptor */
                        if (userElements[i].userRegCount > 1)
                        {
                                si_isa_debug("\t| SREG[%2d:%2d] |  CB%1d "
                                        "Descriptor           |\n",
                                        userElements[i].startUserReg,
                                        userElements[i].startUserReg +
                                        userElements[i].userRegCount - 1,
                                        userElements[i].apiSlot);
                        }
                        else
                        {
                                si_isa_debug("\t| SREG[%2d]    |  CB%1d "
                                        "Descriptor         |\n",
                                        userElements[i].startUserReg,
                                        userElements[i].apiSlot);
                        }
                }

                else if (userElements[i].dataClass == SIBinaryUserDataUAV)
                {
                        /* UAV buffer descriptor */
                        si_isa_debug("\t| SREG[%2d:%2d] |  UAV%-2d "
                                "Descriptor         |\n",
                                userElements[i].startUserReg,
                                userElements[i].startUserReg +
                                userElements[i].userRegCount - 1,
                                userElements[i].apiSlot);
                }
                else if (userElements[i].dataClass ==
                        SIBinaryUserDataConstBufferTable)
                {
                        si_isa_debug("\t| SREG[%2d:%2d] |  Constant Buffer "
                                "Table    |\n",
                                userElements[i].startUserReg,
                                userElements[i].startUserReg +
                                userElements[i].userRegCount - 1);
                }
                else if (userElements[i].dataClass == SIBinaryUserDataUAVTable)
                {
                        si_isa_debug("\t| SREG[%2d:%2d] |  UAV "
                                "Table                |\n",
                                userElements[i].startUserReg,
                                userElements[i].startUserReg +
                                userElements[i].userRegCount - 1);
                }
                else
                {
                        assert(0);
                }
        }
        si_isa_debug("\t-------------------------------------------\n");
        si_isa_debug("\n");

        /* Dump constant buffer 1 (argument mapping) */
        si_isa_debug("Constant buffer 1 initialization (kernel arguments):\n");
        si_isa_debug("\t----------------------------------------------\n");
        si_isa_debug("\t| CB1 Idx | Arg | Uav |   Size   |    Name    |\n");
        si_isa_debug("\t----------------------------------------------\n");
        index = 0;
        ListForEach(ndrange->arg_list, arg, SIArg)
        {
                /* Check that argument was set */
                assert(arg);
                if (!arg->set)
                {
                        fatal("kernel '%s': argument '%s' has not been "
                                "assigned with 'clKernelSetArg'.",
                                kernel->name, arg->name->text);
                }

                /* Process argument depending on its type */
                switch (arg->type)
                {

		case SIArgTypeValue:
                {
                        /* Value copied directly into device constant 
                         * memory */
                        assert(arg->size);
                        si_isa_debug("\t| CB1[%2d] | %3d | %s | %8d | %-10s |\n",
                                arg->value.constant_offset/16, index,
				"n/a", arg->size, arg->name->text);

                        break;
                }

		case SIArgTypePointer:
                {
                        if (arg->pointer.scope != SIArgHwLocal)
                        {
                                si_isa_debug("\t| CB1[%2d] | %3d | %3d | %8d | %-10s |\n", 
					arg->pointer.constant_offset/16, index,
					arg->pointer.buffer_num, arg->size, 
					arg->name->text);
                        }

                        break;
                }

		case SIArgTypeImage:
                {
			warning("unexpected type in si isa debug file");
                        break;
                }

		case SIArgTypeSampler:
                {
			warning("unexpected type in si isa debug file");
                        break;
                }

                default:
                        fatal("%s: argument type not reconized",
                                __FUNCTION__);
                }

                /* Next */
                index++;
        }
        si_isa_debug("\t-------------------------------------------\n");
        si_isa_debug("\n");

        /* Initialized constant buffers */
	si_isa_debug("Initialized constant buffers:\n");
	si_isa_debug("\t-----------------------------------\n");
	si_isa_debug("\t|  CB   |      Address Range      |\n");
	si_isa_debug("\t-----------------------------------\n");
        for (i = 0; i < SI_EMU_MAX_NUM_CONST_BUFS; i++)
	{
		if (!ndrange->const_buf_table_entries[i].valid)
                	continue;

        	mem_read(emu->global_mem,
			ndrange->const_buf_table + 
			i*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
			sizeof(buffer_desc), &buffer_desc);

        	si_isa_debug("\t| CB%-2d  | [0x%8llx:0x%8llx] |\n",
			i, (long long unsigned int)buffer_desc.base_addr,
			(long long unsigned int)buffer_desc.base_addr + 
			(long long unsigned int)buffer_desc.num_records - 1);
	}
	si_isa_debug("\t-----------------------------------\n");
        si_isa_debug("\n");

        /* Initialized UAVs */
	si_isa_debug("Initialized UAVs:\n");
	si_isa_debug("\t-----------------------------------\n");
	si_isa_debug("\t|  UAV  |      Address Range      |\n");
	si_isa_debug("\t-----------------------------------\n");
        for (i = 0; i < SI_EMU_MAX_NUM_UAVS; i++)
	{
		if (!ndrange->uav_table_entries[i].valid)
                	continue;

        	mem_read(emu->global_mem,
			ndrange->uav_table + i*SI_EMU_UAV_TABLE_ENTRY_SIZE, 
			sizeof(buffer_desc), &buffer_desc);

        	si_isa_debug("\t| UAV%-2d | [0x%8x:0x%8x] |\n",
			i, (unsigned int)buffer_desc.base_addr,
			(unsigned int)buffer_desc.base_addr + 
			(unsigned int)buffer_desc.num_records - 1);
	}
	si_isa_debug("\t-----------------------------------\n");
        si_isa_debug("\n");
        si_isa_debug("========================================================"
                "\n");
}

void opencl_si_ndrange_setup_mmu(SINDRange *ndrange, MMU *cpu_mmu,
	int cpu_address_space_index, MMU *gpu_mmu, 
	unsigned int internal_tables_ptr, 
	unsigned int constant_buffers_ptr)
{
	SIArg *arg;

	/* With our SVM implementation, the ndrange ASID should match
	 * the cpu context ASID.  However, this is not a general requirement
	 * so we leave the option of passing the cpu ASID as a parameter */
	assert(ndrange->address_space_index == cpu_address_space_index);

	/* Map constant buffers to MMU */
	MMUCopyTranslation(gpu_mmu, ndrange->address_space_index, cpu_mmu, 
		cpu_address_space_index, constant_buffers_ptr, 
		SI_EMU_TOTAL_CONST_BUF_SIZE);
	opencl_debug("\tmapping constant buffers (addr 0x%x)\n", 
		constant_buffers_ptr);

	/* Map internal tables to MMU */
	unsigned int internal_tables_size = SI_EMU_CONST_BUF_TABLE_SIZE +
		SI_EMU_RESOURCE_TABLE_SIZE + SI_EMU_UAV_TABLE_SIZE;
	MMUCopyTranslation(gpu_mmu, ndrange->address_space_index, cpu_mmu, 
		cpu_address_space_index, internal_tables_ptr, 
		internal_tables_size);
	opencl_debug("\tmapping internal tables (addr 0x%x)\n", 
		internal_tables_ptr);

	/* Map memory objects to MMU */
	ListForEach(ndrange->arg_list, arg, SIArg)
	{
		/* Process argument depending on its type */
		assert(arg);
		switch (arg->type)
		{

		case SIArgTypePointer:
		{
			
			switch (arg->pointer.scope)
			{
			/* UAV */
			case SIArgUAV:
			{
				opencl_debug("\tmapping uav %d (addr 0x%x, "
					"size %u) to GPU asid %d\n",
					arg->pointer.buffer_num, 
					arg->pointer.device_ptr, arg->size,
					ndrange->address_space_index);
				MMUCopyTranslation(gpu_mmu, 
					ndrange->address_space_index, 
					cpu_mmu, cpu_address_space_index, 
					arg->pointer.device_ptr, arg->size);
				break;
			}
			/* Hardware constant memory */
			case SIArgHwConstant:
			{
				opencl_debug("\tmapping cb %d (addr 0x%x)\n",
					arg->pointer.constant_buffer_num, 
					arg->pointer.device_ptr);
				MMUCopyTranslation(gpu_mmu, 
					ndrange->address_space_index,
					cpu_mmu, cpu_address_space_index, 
					arg->pointer.device_ptr, arg->size);
				break;
			}
			default:
			{
				break;
			}
			}

		}
		default:
			break;

		}
	}
}

static void flush_callback(void *data)
{
	assert(data);

	X86Emu *x86_emu = (X86Emu *)data;
	X86EmuProcessEventsSchedule(x86_emu);
}

/* Go through list of arguments */
/* For each buffer found, check if rw or wo */
/* For each rw or wo buffer, create a list of pages to flush */
void opencl_si_kernel_flush_ndrange_buffers(SINDRange *ndrange, SIGpu *gpu,
	X86Emu *x86_emu)
{
	MMU *mmu = gpu->mmu;
	SIArg *arg;

	int num_pages;
	int i;

	unsigned int phys_page;

	/* Compute unit whose cache will be used for the flush */
	SIComputeUnit *compute_unit = gpu->compute_units[0];
	assert(compute_unit);

	/* Use vector cache for module that receives the flush request */
	struct mod_t *flush_mod = compute_unit->vector_cache;
	assert(flush_mod);

	ListForEach(ndrange->arg_list, arg, SIArg)
	{
		/* Check that argument was set */
		assert(arg);
		assert(arg->set);

		/* Process argument depending on its type */
		switch (arg->type)
		{

		case SIArgTypeValue:

			/* Ignore values */
			break;

		case SIArgTypePointer:

			switch (arg->pointer.scope)
			{

			/* Hardware local memory */
			case SIArgHwLocal:
			{
				/* Ignore local memory */
				break;
			}

			/* UAV */
			case SIArgUAV:
			{
				/* Flush UAVs if they are RW or WO */
				if ((arg->pointer.access_type == 
					SIArgWriteOnly) || 
					(arg->pointer.access_type == 
					SIArgReadWrite))
				{
					opencl_debug("\tflushing buffer %s: virtual address 0x%x, ndrange %d (asid %d)\n", 
						arg->name->text, 
						arg->pointer.device_ptr, 
						ndrange->id, 
						ndrange->address_space_index);

					assert(!(arg->pointer.device_ptr %
						mmu->page_size));

					num_pages = (arg->size + 
						mmu->page_size - 1) /
						mmu->page_size;

					/* Iterate through pages and generate
					 * an flush request for each one */
					for (i = 0; i < num_pages; i++)
					{
						phys_page = MMUTranslate(
							gpu->mmu,
							ndrange->
							address_space_index,
							arg->
							pointer.device_ptr + 
							i*mmu->page_size);

						ndrange->flushing--;

						mod_flush(flush_mod, phys_page,
							&ndrange->flushing,
							flush_callback,
							x86_emu);
					}
				}
				break;
			}

			/* Hardware constant memory */
			case SIArgHwConstant:
			{
				/* Ignore constant memory */
				break;
			}

			default:
			{
				fatal("%s: not implemented memory scope",
						__FUNCTION__);
			}

			}

			break;

		case SIArgTypeImage:

			/* Ignore images */
			break;

		case SIArgTypeSampler:

			/* Ignore samplers */
			break;

		default:

			fatal("%s: argument type not recognized (%d)",
				__FUNCTION__, arg->type);

		}
	}
}
