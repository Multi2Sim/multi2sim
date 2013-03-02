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
#include <unistd.h>

#include <arch/common/arch.h>
#include <arch/common/arch-list.h>
#include <arch/southern-islands/asm/bin-file.h>
#include <arch/southern-islands/timing/gpu.h>
#include <arch/x86/emu/context.h>
#include <driver/opencl-old/southern-islands/device.h>
#include <driver/opencl-old/southern-islands/platform.h>
#include <driver/opencl-old/southern-islands/repo.h>
#include <driver/opencl-old/southern-islands/kernel.h>
#include <driver/opencl-old/southern-islands/mem.h>
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
struct arch_t *si_emu_arch;

long long si_emu_max_cycles = 0;
long long si_emu_max_inst = 0;
int si_emu_max_kernels = 0;

enum arch_sim_kind_t si_emu_sim_kind = arch_sim_kind_functional;

char *si_emu_opencl_binary_name = "";
char *si_emu_opengl_binary_name = "";
char *si_emu_report_file_name = "";
FILE *si_emu_report_file = NULL;

int si_emu_wavefront_size = 64;

int si_emu_num_mapped_const_buffers = 2;  /* CB0, CB1 by default */



/*
 * GPU Kernel (Functional Simulator) Public Functions
 */



/* Initialize GPU kernel */
void si_emu_init()
{
	/* Register architecture */
	si_emu_arch = arch_list_register("SouthernIslands", "si");
	si_emu_arch->sim_kind = si_emu_sim_kind;

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
	si_emu->timer = m2s_timer_create("Southern Islands GPU Timer");
	si_emu->global_mem = mem_create();
	si_emu->global_mem->safe = 0;

	si_emu->global_mem_top = SI_EMU_GLOBAL_MEMORY_START;

	/* Initialize disassembler (decoding tables...) */
	si_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	si_isa_init();

	/* Initialize OpenCL objects */
	si_emu->opencl_repo = si_opencl_repo_create();
	si_emu->opencl_platform = si_opencl_platform_create();
	si_emu->opencl_device = si_opencl_device_create();
}


/* Finalize GPU kernel */
void si_emu_done()
{
	/* GPU report */
	if (si_emu_report_file)
		fclose(si_emu_report_file);

	/* Free ND-Ranges */
	while (si_emu->ndrange_list_count)
		si_ndrange_free(si_emu->ndrange_list_head);

	/* Free OpenCL objects */
	si_opencl_repo_free_all_objects(si_emu->opencl_repo);
	si_opencl_repo_free(si_emu->opencl_repo);

	/* Finalize disassembler */
	si_disasm_done();

	/* Finalize ISA */
	si_isa_done();

	/* Finalize GPU kernel */
	mem_free(si_emu->global_mem);
	m2s_timer_free(si_emu->timer);
	free(si_emu);
}

void si_emu_dump_summary(FILE *f)
{
	double time_in_sec;
	double inst_per_sec;

	/* If there was no Southern Islands simulation, no summary */
	if (!si_emu->ndrange_count)
		return;

	/* Calculate statistics */
	time_in_sec = (double) m2s_timer_get_value(si_emu->timer) / 1.0e6;
	inst_per_sec = time_in_sec > 0.0 ? 
		(double) si_emu->inst_count / time_in_sec : 0.0;

	/* Print statistics */
	fprintf(f, "[ SouthernIslands ]\n");
	fprintf(f, "SimType = %s\n", si_emu_sim_kind == 
		arch_sim_kind_functional ?  "Functional" : "Detailed");
	fprintf(f, "Time = %.2f\n", time_in_sec);
	fprintf(f, "NDRangeCount = %d\n", si_emu->ndrange_count);
	fprintf(f, "Instructions = %lld\n", si_emu->inst_count);
	fprintf(f, "InstructionsPerSecond = %.0f\n", inst_per_sec);

	/* Detailed simulation */
	if (si_emu_sim_kind == arch_sim_kind_detailed)
		si_gpu_dump_summary(f);

	/* End */
	fprintf(f, "\n");
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
	void *file_buffer;
	int file_size;

	struct si_opengl_bin_file_t *amd_opengl_bin;
	struct si_opengl_shader_t *amd_opengl_shader;

	/* Initialize disassembler */
	si_disasm_init();

	/* Load file into memory buffer */
	file_buffer = read_buffer(path, &file_size);
	if(!file_buffer)
		fatal("%s:Invalid file!", path);

	/* Analyze the file and initialize structure */	
	amd_opengl_bin = si_opengl_bin_file_create(file_buffer, file_size, path);

	free_buffer(file_buffer);

	/* Basic info of the shader binary */
	printf("This shader binary contains %d shaders\n\n", 
		list_count(amd_opengl_bin->shader_list));
	if (opengl_shader_index > list_count(amd_opengl_bin->shader_list) || 
		opengl_shader_index <= 0 )
	{
		fatal("Shader index out of range! Please choose <index> "
			"from 1 ~ %d", list_count(amd_opengl_bin->shader_list));
	}

	/* Disassemble */
	amd_opengl_shader = list_get(amd_opengl_bin->shader_list, 
		opengl_shader_index - 1 );
	printf("**\n** Disassembly for shader %d\n**\n\n", opengl_shader_index);
	si_disasm_buffer(&amd_opengl_shader->isa_buffer, stdout);
	printf("\n\n\n");

	/* Free */
	si_opengl_bin_file_free(amd_opengl_bin);
	si_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}

/* Run one iteration of the Southern Islands GPU emulation loop.
 * Return FALSE if there is no more emulation to perform. */
int si_emu_run(void)
{
	struct si_ndrange_t *ndrange;
	struct si_ndrange_t *ndrange_next;

	struct si_work_group_t *work_group;
	struct si_work_group_t *work_group_next;

	struct si_wavefront_t *wavefront;
	struct si_wavefront_t *wavefront_next;

	/* For efficiency when no Southern Islands emulation is selected, 
	 * exit here if the list of existing ND-Ranges is empty. */
	if (!si_emu->ndrange_list_count)
		return 0;

	/* Start any ND-Range in state 'pending' */
	while ((ndrange = si_emu->pending_ndrange_list_head))
	{
		/* Set all ready work-groups to running */
		while ((work_group = ndrange->pending_list_head))
		{
			si_work_group_clear_status(work_group, 
				si_work_group_pending);
			si_work_group_set_status(work_group, 
				si_work_group_running);
		}

		/* Set is in state 'running' */
		si_ndrange_clear_status(ndrange, si_ndrange_pending);
		si_ndrange_set_status(ndrange, si_ndrange_running);
	}

	/* Run one instruction of each wavefront in each work-group of each
	 * ND-Range that is in status 'running'. */
	for (ndrange = si_emu->running_ndrange_list_head; ndrange; 
		ndrange = ndrange_next)
	{
		/* Save next ND-Range in state 'running'. This is done because 
		 * the state might change during the execution of the 
		 * ND-Range. */
		ndrange_next = ndrange->running_ndrange_list_next;

		/* Execute an instruction from each work-group */
		for (work_group = ndrange->running_list_head; work_group; 
			work_group = work_group_next)
		{
			/* Save next running work-group */
			work_group_next = work_group->running_list_next;

			/* Run an instruction from each wavefront */
			for (wavefront = work_group->running_list_head; 
				wavefront; wavefront = wavefront_next)
			{
				/* Save next running wavefront */
				wavefront_next = wavefront->running_list_next;

				/* Execute instruction in wavefront */
				si_wavefront_execute(wavefront);
			}
		}
	}

	/* Free ND-Ranges that finished */
	while ((ndrange = si_emu->finished_ndrange_list_head))
	{
		/* Dump ND-Range report */
		si_ndrange_dump(ndrange, si_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (si_emu_max_kernels && si_emu->ndrange_count >= 
			si_emu_max_kernels)
		{
			esim_finish = esim_finish_si_max_kernels;
		}

		/* Extract from list of finished ND-Ranges and free */
		si_ndrange_free(ndrange);
	}

	/* Return TRUE */
	return 1;
}

struct si_buffer_desc_t si_emu_create_buffer_desc(
	struct si_opencl_kernel_arg_t *arg)
{
	struct si_buffer_desc_t buf_desc;
	struct si_opencl_mem_t *mem_obj;

	enum si_opencl_kernel_arg_data_type_t data_type;

	int num_elems;	

	int num_format = -1;
	int data_format = -1;
	int elem_size;

	assert(sizeof(buf_desc) == 16);
	assert(arg);

	/* Zero-out the buffer resource descriptor */
	memset(&buf_desc, 0, 16);

	num_format = SI_BUF_DESC_DATA_FMT_INVALID;
	data_format = SI_BUF_DESC_DATA_FMT_INVALID;

	num_elems = arg->pointer.num_elems;
	data_type = arg->pointer.data_type;
	
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
	assert(num_format != -1); 
	assert(data_format != -1);

	assert(arg->pointer.mem_obj_id);
	mem_obj = si_opencl_repo_get_object(si_emu->opencl_repo,
		si_opencl_object_mem, arg->pointer.mem_obj_id);
	/* FIXME I believe that base address should be 0 unless there is an
	 * offset provided in one of the unknown metadata fields. 
	 * However, I'm not completely sure how the UAV mapping works
	 * (which should probably go in CB1), so we'll set the base
	 * address here, and store 0 in CB1. */
	buf_desc.base_addr = mem_obj->device_ptr;
	buf_desc.num_format = num_format;
	buf_desc.data_format = data_format;
	buf_desc.elem_size = elem_size;

	return buf_desc;
}

void si_emu_insert_into_uav_table(struct si_buffer_desc_t buf_desc,
	struct si_opencl_kernel_arg_t *arg)
{
	unsigned int buffer_num;

	assert(arg->kind == SI_OPENCL_KERNEL_ARG_KIND_POINTER);
	buffer_num = arg->pointer.buffer_num;

	assert(buffer_num < SI_EMU_MAX_NUM_UAVS);
	assert(SI_EMU_UAV_TABLE_START + buffer_num*32 <= 
		SI_EMU_UAV_TABLE_START + SI_EMU_UAV_TABLE_SIZE - 32);

	/* Write the buffer resource descriptor into the UAV at
	 * 'buffer_num' offset */
	mem_write(si_emu->global_mem, SI_EMU_UAV_TABLE_START + buffer_num*32,
		sizeof(buf_desc), &buf_desc);

	si_emu->valid_uav_list[buffer_num] = 1;
}

/* Store an entry in the UAV table */
void si_emu_set_uav_table_entry(int uav, unsigned int addr)
{
	struct si_buffer_desc_t buf_desc;
        memset(&buf_desc, 0, sizeof buf_desc);

	assert(sizeof(struct si_buffer_desc_t) == 16);
	assert(uav < SI_EMU_MAX_NUM_UAVS);  

	/* NOTE This may be just an address instead of a resource
	 * descriptor, but either way it should work */
	buf_desc.base_addr = addr;

        mem_write(si_emu->global_mem, SI_EMU_UAV_TABLE_START+uav*32, 16,
                &buf_desc);

	si_emu->valid_uav_list[uav] = 1;
}

/* Retrieve an entry in the UAV table */
struct si_buffer_desc_t si_emu_get_uav_table_entry(int uav)
{
	struct si_buffer_desc_t buf_desc;
        memset(&buf_desc, 0, sizeof buf_desc);

	assert(uav < SI_EMU_MAX_NUM_UAVS);  
	assert(si_emu->valid_uav_list[uav]);

        mem_read(si_emu->global_mem, SI_EMU_UAV_TABLE_START+uav*32, 16,
                &buf_desc);

	assert(buf_desc.base_addr >= SI_EMU_GLOBAL_MEMORY_START);

	return buf_desc;
}

/* Get base address for a UAV */
unsigned int si_emu_get_uav_base_addr(int uav)
{
	unsigned int base_addr; 

	struct si_buffer_desc_t buf_desc;
        memset(&buf_desc, 0, sizeof buf_desc);

	assert(uav < SI_EMU_MAX_NUM_UAVS);  
	assert(si_emu->valid_uav_list[uav]);

        mem_read(si_emu->global_mem, SI_EMU_UAV_TABLE_START+uav*32, 16,
                &buf_desc);

	assert(buf_desc.base_addr >= SI_EMU_GLOBAL_MEMORY_START);

	base_addr = (unsigned int)buf_desc.base_addr;

	return base_addr;
}
