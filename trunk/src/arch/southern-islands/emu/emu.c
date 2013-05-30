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

#include <arch/common/arch.h>
#include <arch/southern-islands/asm/bin-file.h>
#include <arch/southern-islands/timing/gpu.h>
#include <driver/opencl/opencl.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "ndrange.h"
#include "opengl-bin-file.h"
#include "wavefront.h"
#include "work-group.h"


/*
 * Global variables
 */


struct si_emu_t *si_emu;

long long si_emu_max_cycles = 0;
long long si_emu_max_inst = 0;
int si_emu_max_kernels = 0;

char *si_emu_opengl_binary_name = "";
char *si_emu_report_file_name = "";
FILE *si_emu_report_file = NULL;

int si_emu_wavefront_size = 64;

int si_emu_num_mapped_const_buffers = 2;  /* CB0, CB1 by default */



/*
 * GPU Kernel (Functional Simulator) Public Functions
 */


/* Initialize GPU kernel */
void si_emu_init(void)
{
	/* Open report file */
	if (*si_emu_report_file_name)
	{
		si_emu_report_file = file_open_for_write(
			si_emu_report_file_name);
		if (!si_emu_report_file)
		{
			fatal("%s: cannot open report for Southern Islands "
				"emulator", si_emu_report_file_name);
		}
	}

	/* Initialize */
	si_emu = xcalloc(1, sizeof(struct si_emu_t));
	si_emu->video_mem = mem_create();
	si_emu->video_mem->safe = 0;
	si_emu->video_mem_top = 0;
	si_emu->waiting_work_groups = list_create();
	si_emu->running_work_groups = list_create();

	/* Set global memory to video memory by default */
	si_emu->global_mem = si_emu->video_mem; 

	/* Initialize disassembler (decoding tables...) */
	si_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	si_isa_init();
}


/* Finalize GPU kernel */
void si_emu_done()
{
	/* GPU report */
	if (si_emu_report_file)
		fclose(si_emu_report_file);

	/* Finalize disassembler */
	si_disasm_done();

	/* Finalize ISA */
	si_isa_done();

	/* Free emulator memory */
	mem_free(si_emu->video_mem);

	/* Free the work-group queues */
	list_free(si_emu->waiting_work_groups);
	list_free(si_emu->running_work_groups);

	/* Free the emulator */
	memset(si_emu, 0, sizeof(struct si_emu_t));
	free(si_emu);
}


void si_emu_dump(FILE *f)
{
}


void si_emu_dump_summary(FILE *f)
{
	fprintf(f, "NDRangeCount = %d\n", si_emu->ndrange_count);
	fprintf(f, "WorkGroupCount = %lld\n", si_emu->work_group_count);
	fprintf(f, "BranchInstructions = %lld\n", si_emu->branch_inst_count);
	fprintf(f, "LDSInstructions = %lld\n", si_emu->lds_inst_count);
	fprintf(f, "ScalarALUInstructions = %lld\n", 
		si_emu->scalar_alu_inst_count);
	fprintf(f, "ScalarMemInstructions = %lld\n", 
		si_emu->scalar_mem_inst_count);
	fprintf(f, "VectorALUInstructions = %lld\n", 
		si_emu->vector_alu_inst_count);
	fprintf(f, "VectorMemInstructions = %lld\n", 
		si_emu->vector_mem_inst_count);
}


/* GPU disassembler tool */
void si_emu_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;

	struct si_bin_file_t *amd_bin;

	char kernel_name[MAX_STRING_SIZE];

	int i;

	/* Initialize disassembler */
	si_disasm_init();

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
		if (str_prefix(symbol->name, "__OpenCL_") && 
			str_suffix(symbol->name, "_kernel"))
		{
			/* Decode internal ELF */
			str_substr(kernel_name, sizeof(kernel_name), 
				symbol->name, 9, strlen(symbol->name) - 16);
			amd_bin = si_bin_file_create(
				section->buffer.ptr + symbol->value, 
				symbol->size, kernel_name);

			/* Get kernel name */
			printf("**\n** Disassembly for '__kernel %s'\n**\n\n", 
				kernel_name);
			si_disasm_buffer(&amd_bin->
				enc_dict_entry_southern_islands->
				sec_text_buffer, stdout);
			printf("\n\n\n");

			/* Free internal ELF */
			si_bin_file_free(amd_bin);
		}
	}

	/* Free external ELF */
	elf_file_free(elf_file);
	si_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}

/* GPU OpenGL disassembler tool */
void si_emu_opengl_disasm(char *path, int opengl_shader_index)
{
	struct si_opengl_program_binary_t *si_program_bin;
	struct si_opengl_shader_binary_t *si_shader;
	void *file_buffer;
	int file_size;

	/* Initialize disassembler */
	si_disasm_init();

	/* Load file into memory buffer */
	file_buffer = read_buffer(path, &file_size);
	if(!file_buffer)
		fatal("%s:Invalid file!", path);

	/* Analyze the file and initialize structure */	
	si_program_bin = si_opengl_program_binary_create(file_buffer, file_size, path);
	free_buffer(file_buffer);

	// /* Basic info of the shader binary */
	printf("This shader binary contains %d shaders\n\n", 
		list_count(si_program_bin->shaders));
	if (opengl_shader_index > list_count(si_program_bin->shaders) || 
		opengl_shader_index <= 0 )
	{
		fatal("Shader index out of range! Please choose <index> "
			"from 1 ~ %d", list_count(si_program_bin->shaders));
	}

	/* Disassemble */
	si_shader = list_get(si_program_bin->shaders, 
		opengl_shader_index - 1 );
	printf("**\n** Disassembly for shader %d\n**\n\n", opengl_shader_index);
	si_disasm_buffer(si_shader->shader_isa, stdout);
	printf("\n\n\n");

	/* Free */
	si_opengl_program_binary_free(si_program_bin);
	si_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}

/* Run one iteration of the Southern Islands GPU emulation loop.
 * Return TRUE if still running. */
int si_emu_run(void)
{
	struct si_ndrange_t *ndrange;
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;

	int wavefront_id;
	long work_group_id;

	if (!list_count(si_emu->running_work_groups) &&
		list_count(si_emu->waiting_work_groups))
	{
		work_group_id = (long)list_dequeue(si_emu->waiting_work_groups);
		list_enqueue(si_emu->running_work_groups, (void*)work_group_id);
	}

	/* For efficiency when no Southern Islands emulation is selected, 
	 * exit here if the list of existing ND-Ranges is empty. */
	if (!list_count(si_emu->running_work_groups))
		return FALSE;

	assert(si_emu->ndrange);
	ndrange = si_emu->ndrange;

	/* Instantiate the next work-group */
	work_group_id = (long)list_bottom(si_emu->running_work_groups);
	work_group = si_work_group_create(work_group_id, ndrange);

	/* Execute the work-group to completion */
	while (!work_group->finished_emu)
	{
		SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
		{
			wavefront = work_group->wavefronts[wavefront_id];

			if (wavefront->finished || wavefront->at_barrier)
				continue;

			/* Execute instruction in wavefront */
			si_wavefront_execute(wavefront);
		}
	}

	/* Remove work group from running list */
	list_dequeue(si_emu->running_work_groups);

	/* Free work group */
	si_work_group_free(work_group);

	/* If there is not more work groups to run, let driver know */
	if (!list_count(si_emu->waiting_work_groups))
	{
		opencl_si_request_work();
	}

	/* Still emulating */
	return TRUE;
}

