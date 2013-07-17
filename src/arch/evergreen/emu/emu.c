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

#include <arch/evergreen/emu/opengl-bin-file.h>
#include <driver/opencl-old/evergreen/bin-file.h>
#include <driver/opencl-old/evergreen/device.h>
#include <driver/opencl-old/evergreen/platform.h>
#include <driver/opencl-old/evergreen/repo.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "ndrange.h"
#include "work-group.h"



/*
 * Class 'EvgEmu'
 */

CLASS_IMPLEMENTATION(EvgEmu);

void EvgEmuCreate(EvgEmu *self)
{
	/* Parent */
	EmuCreate(asEmu(self), "Evergreen");

	/* Memories */
	self->const_mem = mem_create();
	self->const_mem->safe = 0;
	self->global_mem = mem_create();
	self->global_mem->safe = 0;

	/* Initialize OpenCL objects */
	self->opencl_repo = evg_opencl_repo_create();
	self->opencl_platform = evg_opencl_platform_create(self);
	self->opencl_device = evg_opencl_device_create(self);

	/* Virtual functions */
	asObject(self)->Dump = EvgEmuDump;
	asEmu(self)->DumpSummary = EvgEmuDumpSummary;
	asEmu(self)->Run = EvgEmuRun;
}


void EvgEmuDestroy(EvgEmu *self)
{
	/* Free ND-Ranges */
	while (self->ndrange_list_count)
		evg_ndrange_free(self->ndrange_list_head);

	/* Free OpenCL objects */
	evg_opencl_repo_free_all_objects(self->opencl_repo);
	evg_opencl_repo_free(self->opencl_repo);

	/* Finalize GPU kernel */
	mem_free(self->const_mem);
	mem_free(self->global_mem);
}


void EvgEmuDump(Object *self, FILE *f)
{
	/* Call parent */
	EmuDump(self, f);
}


void EvgEmuDumpSummary(Emu *self, FILE *f)
{
	EvgEmu *emu = asEvgEmu(evg_emu);

	/* Call parent */
	EmuDumpSummary(self, f);

	/* More statistics */
	fprintf(f, "NDRangeCount = %d\n", emu->ndrange_count);
}


int EvgEmuRun(Emu *self)
{
	EvgEmu *emu = asEvgEmu(self);

	struct evg_ndrange_t *ndrange;
	struct evg_ndrange_t *ndrange_next;

	struct evg_work_group_t *work_group;
	struct evg_work_group_t *work_group_next;

	struct evg_wavefront_t *wavefront;
	struct evg_wavefront_t *wavefront_next;

	/* Exit if there are no ND-Ranges to emulate */
	if (!emu->ndrange_list_count)
		return FALSE;

	/* Start any ND-Range in state 'pending' */
	while ((ndrange = emu->pending_ndrange_list_head))
	{
		/* Set all ready work-groups to running */
		while ((work_group = ndrange->pending_list_head))
		{
			evg_work_group_clear_status(work_group, evg_work_group_pending);
			evg_work_group_set_status(work_group, evg_work_group_running);
		}

		/* Set is in state 'running' */
		evg_ndrange_clear_status(ndrange, evg_ndrange_pending);
		evg_ndrange_set_status(ndrange, evg_ndrange_running);
	}

	/* Run one instruction of each wavefront in each work-group of each
	 * ND-Range that is in status 'running'. */
	for (ndrange = emu->running_ndrange_list_head; ndrange; ndrange = ndrange_next)
	{
		/* Save next ND-Range in state 'running'. This is done because the state
		 * might change during the execution of the ND-Range. */
		ndrange_next = ndrange->running_ndrange_list_next;

		/* Execute an instruction from each work-group */
		for (work_group = ndrange->running_list_head; work_group; work_group = work_group_next)
		{
			/* Save next running work-group */
			work_group_next = work_group->running_list_next;

			/* Run an instruction from each wavefront */
			for (wavefront = work_group->running_list_head; wavefront; wavefront = wavefront_next)
			{
				/* Save next running wavefront */
				wavefront_next = wavefront->running_list_next;

				/* Execute instruction in wavefront */
				evg_wavefront_execute(wavefront);
			}
		}
	}

	/* Free ND-Ranges that finished */
	while ((ndrange = emu->finished_ndrange_list_head))
	{
		/* Dump ND-Range report */
		evg_ndrange_dump(ndrange, evg_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (evg_emu_max_kernels && emu->ndrange_count >= evg_emu_max_kernels)
			esim_finish = esim_finish_evg_max_kernels;

		/* Extract from list of finished ND-Ranges and free */
		evg_ndrange_free(ndrange);
	}

	/* Still emulating */
	return TRUE;
}



/*
 * Non-Class Stuff
 */


EvgEmu *evg_emu;

long long evg_emu_max_cycles;
long long evg_emu_max_inst;
int evg_emu_max_kernels;

char *evg_emu_opencl_binary_name = "";
char *evg_emu_report_file_name = "";
FILE *evg_emu_report_file = NULL;

int evg_emu_wavefront_size = 64;


void evg_emu_init(void)
{
	/* Classes */
	CLASS_REGISTER(EvgEmu);

	/* Open report file */
	if (*evg_emu_report_file_name)
	{
		evg_emu_report_file = file_open_for_write(evg_emu_report_file_name);
		if (!evg_emu_report_file)
			fatal("%s: cannot open report for Evergreen emulator",
				evg_emu_report_file_name);
	}

	/* Create emulator */
	evg_emu = new(EvgEmu);

	/* Initialize disassembler (decoding tables...) */
	evg_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	evg_isa_init();
}


void evg_emu_done()
{
	/* GPU report */
	if (evg_emu_report_file)
		fclose(evg_emu_report_file);

	/* Finalize disassembler */
	evg_disasm_done();

	/* Finalize ISA */
	evg_isa_done();

	/* Free emulator */
	delete(evg_emu);
}


void evg_emu_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;

	struct evg_bin_file_t *amd_bin;

	char kernel_name[MAX_STRING_SIZE];

	int i;

	/* Initialize disassembler */
	evg_disasm_init();

	/* Decode external ELF */
	elf_file = elf_file_create_from_path(path);
	for (i = 0; i < list_count(elf_file->symbol_table); i++)
	{
		/* Get symbol and section */
		symbol = list_get(elf_file->symbol_table, i);
		section = list_get(elf_file->section_list, symbol->section);
		if (!section)
			continue;

		/* If symbol is '__OpenCL_XXX_kernel', it points to internal ELF */
		if (str_prefix(symbol->name, "__OpenCL_") && str_suffix(symbol->name, "_kernel"))
		{
			/* Decode internal ELF */
			str_substr(kernel_name, sizeof(kernel_name), symbol->name, 9, strlen(symbol->name) - 16);
			amd_bin = evg_bin_file_create(section->buffer.ptr + symbol->value, symbol->size, kernel_name);

			/* Get kernel name */
			printf("**\n** Disassembly for '__kernel %s'\n**\n\n", kernel_name);
			evg_disasm_buffer(&amd_bin->enc_dict_entry_evergreen->sec_text_buffer, stdout);
			printf("\n\n\n");

			/* Free internal ELF */
			evg_bin_file_free(amd_bin);
		}
	}

	/* Free external ELF */
	elf_file_free(elf_file);
	evg_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}


void evg_emu_opengl_disasm(char *path, int opengl_shader_index)
{
	void *file_buffer;
	int file_size;

	struct evg_opengl_bin_file_t *amd_opengl_bin;
	struct evg_opengl_shader_t *amd_opengl_shader;

	/* Initialize disassembler */
	evg_disasm_init();

	/* Load file into memory buffer */
	file_buffer = read_buffer(path, &file_size);
	if(!file_buffer)
		fatal("%s:Invalid file!", path);

	/* Analyze the file and initialize structure */	
	amd_opengl_bin = evg_opengl_bin_file_create(file_buffer, file_size, path);

	free_buffer(file_buffer);

	/* Basic info of the shader binary */
	printf("This shader binary contains %d shaders\n\n", list_count(amd_opengl_bin->shader_list));
	if (opengl_shader_index > list_count(amd_opengl_bin->shader_list) || opengl_shader_index <= 0 )
	{
		fatal("Shader index out of range! Please choose <index> from 1 ~ %d", list_count(amd_opengl_bin->shader_list));
	}

	/* Disaseemble */
	amd_opengl_shader = list_get(amd_opengl_bin->shader_list, opengl_shader_index -1 );
	printf("**\n** Disassembly for shader %d\n**\n\n", opengl_shader_index);
	evg_disasm_buffer(&amd_opengl_shader->isa_buffer, stdout);
	printf("\n\n\n");

	/* Free */
	evg_opengl_bin_file_free(amd_opengl_bin);
	evg_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}

