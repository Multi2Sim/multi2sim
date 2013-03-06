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
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

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



/*
 * Kernel List
 */


struct list_t *opencl_si_kernel_list;

void opencl_si_kernel_list_init(void)
{
	/* Already initialized */
	if (opencl_si_kernel_list)
		return;

	/* Initialize and add one empty element */
	opencl_si_kernel_list = list_create();
	list_add(opencl_si_kernel_list, NULL);
}


void opencl_si_kernel_list_done(void)
{
	struct opencl_si_kernel_t *kernel;
	int index;

	/* Not initialized */
	if (!opencl_si_kernel_list)
		return;

	/* Free list of Southern Islands kernels */
	LIST_FOR_EACH(opencl_si_kernel_list, index)
	{
		kernel = list_get(opencl_si_kernel_list, index);
		if (kernel)
			opencl_si_kernel_free(kernel);
	}
	list_free(opencl_si_kernel_list);
}




/*
 * Argument
 */

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
	/* Specific fields per type */
	switch (arg->type)
	{
	case opencl_si_arg_value:

		if (arg->value.value_ptr)
			free(arg->value.value_ptr);
		break;

	default:
		break;
	}

	/* Rest */
	free(arg->name);
	free(arg);
}


/* Infer argument size from its data type */
int opencl_si_arg_get_data_size(enum opencl_si_arg_data_type_t data_type)
{
	switch (data_type)
	{

	case opencl_si_arg_i8:
	case opencl_si_arg_u8:
	case opencl_si_arg_struct:
	case opencl_si_arg_union:
	case opencl_si_arg_event:
	case opencl_si_arg_opaque:

		return 1;

	case opencl_si_arg_i16:
	case opencl_si_arg_u16:

		return 2;

	case opencl_si_arg_i32:
	case opencl_si_arg_u32:
	case opencl_si_arg_float:

		return 4;

	case opencl_si_arg_i64:
	case opencl_si_arg_u64:
	case opencl_si_arg_double:

		return 8;

	default:

		panic("%s: invalid data type (%d)",
				__FUNCTION__, data_type);
		return 0;
	}
}




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


static struct str_map_t opencl_si_arg_dimension_map =
{
	2,
	{
		{ "2D", 2 },
		{ "3D", 3 }
	}
};


static struct str_map_t opencl_si_arg_access_type_map =
{
	3,
	{
		{ "RO", opencl_si_arg_read_only },
		{ "WO", opencl_si_arg_write_only },
		{ "RW", opencl_si_arg_read_write }
	}
};


static struct str_map_t opencl_si_arg_data_type_map =
{
	16,
	{
		{ "i1", opencl_si_arg_i1 },
		{ "i8", opencl_si_arg_i8 },
		{ "i16", opencl_si_arg_i16 },
		{ "i32", opencl_si_arg_i32 },
		{ "i64", opencl_si_arg_i64 },
		{ "u1", opencl_si_arg_u1 },
		{ "u8", opencl_si_arg_u8 },
		{ "u16", opencl_si_arg_u16 },
		{ "u32", opencl_si_arg_u32 },
		{ "u64", opencl_si_arg_u64 },
		{ "float", opencl_si_arg_float },
		{ "double", opencl_si_arg_double },
		{ "struct", opencl_si_arg_struct },
		{ "union", opencl_si_arg_union },
		{ "event", opencl_si_arg_event },
		{ "opaque", opencl_si_arg_opaque }
	}
};


static struct str_map_t opencl_si_arg_scope_map =
{
	10,
	{
		{ "g", opencl_si_arg_global },
		{ "p", opencl_si_arg_emu_private },
		{ "l", opencl_si_arg_emu_local },
		{ "uav", opencl_si_arg_uav },
		{ "c", opencl_si_arg_emu_constant },
		{ "r", opencl_si_arg_emu_gds },
		{ "hl", opencl_si_arg_hw_local },
		{ "hp", opencl_si_arg_hw_private },
		{ "hc", opencl_si_arg_hw_constant },
		{ "hr", opencl_si_arg_hw_gds }
	}
};


static void opencl_si_kernel_load_metadata_v3(struct opencl_si_kernel_t *kernel)
{
	struct elf_buffer_t *buffer = &kernel->metadata_buffer;
	struct opencl_si_arg_t *arg;
	struct list_t *token_list;

	char line[MAX_STRING_SIZE];
	char *token;
	int err;

	for (;;)
	{
		/* Read the next line */
		elf_buffer_read_line(buffer, line, sizeof line);
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
			arg = opencl_si_arg_create(opencl_si_arg_value, token);

			/* Token 2 - Data type */
			token = str_token_list_shift(token_list);
			arg->value.data_type = str_map_string_err(&opencl_si_arg_data_type_map,
					token, &err);
			if (err)
				fatal("%s: invalid data type '%s'.\n%s",
					__FUNCTION__, token, opencl_err_si_kernel_metadata);

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
				opencl_si_arg_get_data_size(arg->value.data_type);

			/* Debug */
			opencl_debug("\targument '%s' - value stored in constant "
				"buffer %d at offset %d\n",
				arg->name, arg->value.constant_buffer_num,
				arg->value.constant_offset);

			/* Add argument */
			list_add(kernel->arg_list, arg);
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
			arg = opencl_si_arg_create(opencl_si_arg_pointer, token);

			/* Token 2 - Data type */
			token = str_token_list_shift(token_list);
			arg->pointer.data_type = str_map_string_err(&opencl_si_arg_data_type_map,
					token, &err);
			if (err)
				fatal("%s: invalid data type '%s'.\n%s",
					__FUNCTION__, token, opencl_err_si_kernel_metadata);

			/* Token 3 - Number of elements
			 * Arrays of pointers not supported, only "1" allowed. */
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
			arg->pointer.scope = str_map_string_err(&opencl_si_arg_scope_map,
					token, &err);
			if (err)
				fatal("%s: invalid scope '%s'.\n%s",
					__FUNCTION__, token, opencl_err_si_kernel_metadata);

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
			arg->pointer.access_type = str_map_string_err(&opencl_si_arg_access_type_map,
					token, &err);
			if (err)
				fatal("%s: invalid access type '%s'.\n%s",
					__FUNCTION__, token, opencl_err_si_kernel_metadata);

			/* Token 10 - ??? */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect(kernel, token_list, "0");

			/* Token 11 - ??? */
			token = str_token_list_shift(token_list);
			opencl_si_kernel_expect(kernel, token_list, "0");

			/* Data size inferred here is always 4, the size of a pointer. */
			arg->size = 4;

			/* Debug */
			opencl_debug("\targument '%s' - Pointer stored in constant "
				"buffer %d at offset %d\n",
				arg->name, arg->pointer.constant_buffer_num,
				arg->pointer.constant_offset);

			/* Add argument */
			list_add(kernel->arg_list, arg);
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
			arg = opencl_si_arg_create(opencl_si_arg_image, token);

			/* Token 2 - Dimension */
			token = str_token_list_shift(token_list);
			arg->image.dimension = str_map_string_err(&opencl_si_arg_dimension_map,
					token, &err);
			if (err)
				fatal("%s: invalid image dimensions '%s'.\n%s",
					__FUNCTION__, token, opencl_err_si_kernel_metadata);

			/* Token 3 - Access type */
			token = str_token_list_shift(token_list);
			arg->image.access_type = str_map_string_err(&opencl_si_arg_access_type_map,
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
			list_add(kernel->arg_list, arg);
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
			arg = opencl_si_arg_create(opencl_si_arg_sampler, token);

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
			list_add(kernel->arg_list, arg);
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


struct opencl_si_kernel_t *opencl_si_kernel_create(struct opencl_si_program_t *program,
		char *name)
{
	struct opencl_si_kernel_t *kernel;
	struct elf_file_t *elf_file;
	char symbol_name[MAX_STRING_SIZE];

	struct elf_symbol_t *metadata_symbol;
	struct elf_symbol_t *header_symbol;
	struct elf_symbol_t *kernel_symbol;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_si_kernel_t));
	kernel->name = xstrdup(name);
	kernel->program = program;
	kernel->arg_list = list_create();

	/* Insert in kernel list */
	opencl_si_kernel_list_init();
	list_add(opencl_si_kernel_list, kernel);
	kernel->id = list_count(opencl_si_kernel_list) - 1;

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
	kernel->bin_file = si_bin_file_create(kernel->kernel_buffer.ptr,
		kernel->kernel_buffer.size, symbol_name);

	/* Load metadata */
	opencl_si_kernel_load_metadata(kernel);

	/* Return */
	return kernel;
}


void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel)
{
	int index;

	/* Free argument list */
	LIST_FOR_EACH(kernel->arg_list, index)
		opencl_si_arg_free(list_get(kernel->arg_list, index));
	list_free(kernel->arg_list);

	/* Rest */
	si_bin_file_free(kernel->bin_file);
	free(kernel->name);
	free(kernel);
}
