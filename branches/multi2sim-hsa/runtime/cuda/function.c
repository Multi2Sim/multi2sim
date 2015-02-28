/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include "../include/cuda.h"
#include "debug.h"
#include "elf-format.h"
#include "list.h"
#include "function.h"
#include "mhandle.h"
#include "module.h"


CUfunction cuda_function_create(CUmodule module, const char *function_name)
{
	CUfunction function;

	struct elf_file_t *dev_func_bin;
	struct elf_section_t *sec;
	char sec_name[1024];
	int sec_index;
	struct elf_section_t *text_sec;
	struct elf_section_t *shared_sec = NULL;

	int i;
	unsigned char inst_buf_byte;

	/* Create function */
	function = xcalloc(1, sizeof(struct CUfunc_st));

	/* Initialize identification */
	function->id = list_count(function_list);
	function->name = xstrdup(function_name);

	/* Get cubin */
	dev_func_bin = module->elf_file;

	/* Look for .text.device_function_name section */
	snprintf(sec_name, sizeof sec_name, ".text.%s", function_name);
	for (i = 0; i < list_count(dev_func_bin->section_list); ++i)
	{
		sec = list_get(dev_func_bin->section_list, i);

		if (!strncmp(sec->name, sec_name, sizeof sec_name))
		{
			sec_index = i;
			break;
		}
	}
	if (i == list_count(dev_func_bin->section_list))
		fatal("%s: cannot get .text.device_function_name section.! This is \n"
				"probably because the cubin filename set in M2S_CUDA_BINARY \n"
				"is not the device function identifier. Please check the \n"
				"cubin file.", __FUNCTION__);

	/* Get .text.device_function_name section */
	text_sec = list_get(dev_func_bin->section_list, sec_index);

	/* Get instruction binary */
	function->inst_buf_size = text_sec->header->sh_size;
	function->inst_buf = xcalloc(1, function->inst_buf_size);
	for (i = 0; i < function->inst_buf_size; ++i)
	{
		elf_buffer_seek(&(dev_func_bin->buffer), text_sec->header->sh_offset +
				i);
		elf_buffer_read(&(dev_func_bin->buffer), &inst_buf_byte, 1);
		if (i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3)
		{
			function->inst_buf[i / 8] |= (unsigned long long)(inst_buf_byte) <<
					(i * 8 + 32);
		}
		else
		{
			function->inst_buf[i / 8] |= (unsigned long long)(inst_buf_byte) <<
					(i * 8 - 32);
		}
	}

	/* Look for .nv.shared.device_function_name section */
	snprintf(sec_name, sizeof sec_name, ".nv.shared.%s", function_name);
	for (i = 0; i < list_count(dev_func_bin->section_list); ++i)
	{
		sec = list_get(dev_func_bin->section_list, i);

		if (!strncmp(sec->name, sec_name, sizeof sec_name))
		{
			sec_index = i;
			break;
		}
	}

	/* Get .nv.shared.device_function_name section if any */
	if (i < list_count(dev_func_bin->section_list))
		shared_sec = list_get(dev_func_bin->section_list, sec_index);

	/* Initialize attributes */
	function->maxThreadsPerBlock = 1024;  // Hard-coded for Fermi
	function->sharedSizeBytes = shared_sec ? shared_sec->header->sh_size : 0;
	function->constSizeBytes = 0;
	function->localSizeBytes = 0;
	function->numRegs = text_sec->header->sh_info >> 24;
	function->ptxVersion = (dev_func_bin->header->e_flags >> 16) & 0xff;
	function->binaryVersion = dev_func_bin->header->e_flags & 0xff;
	function->cache_config = CU_FUNC_CACHE_PREFER_NONE;

	/* Add to function list */
	list_add(function_list, function);

	return function;
}

void cuda_function_free(CUfunction function)
{
	list_remove(function_list, function);

	free(function->inst_buf);
	free(function->name);

	free(function);
}

