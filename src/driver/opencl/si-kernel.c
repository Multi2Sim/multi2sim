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

struct opencl_si_arg_t *opencl_si_arg_create(void)
{
	struct opencl_si_arg_t *arg;

	/* Initialize */
	arg = xcalloc(1, sizeof(struct opencl_si_arg_t));

	/* Return */
	return arg;
}


void opencl_si_arg_free(struct opencl_si_arg_t *arg)
{
	free(arg);
}




/*
 * Kernel
 */

static void opencl_si_kernel_expect_head_token(struct opencl_si_kernel_t *kernel,
		struct list_t *token_list, char *head_token)
{
	char *token;

	token = str_token_list_first(token_list);
	if (strcmp(token, head_token))
		fatal("%s: token '%s' expected, '%s' found.\n%s",
				__FUNCTION__, head_token, token,
				opencl_err_si_kernel_metadata);
}


static void opencl_si_kernel_expect_token_count(struct opencl_si_kernel_t *kernel,
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
	opencl_si_kernel_expect_head_token(kernel, token_list, "ARGSTART");
	opencl_si_kernel_expect_token_count(kernel, token_list, 2);
	str_token_list_free(token_list);

	/* Second line contains version info. Example:
	 * ;version:3:1:104 */
	elf_buffer_read_line(elf_buffer, line, sizeof line);
	token_list = str_token_list_create(line, ";:");
	opencl_si_kernel_expect_head_token(kernel, token_list, "version");
	opencl_si_kernel_expect_token_count(kernel, token_list, 4);
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
	free(kernel->name);
	free(kernel);
}
