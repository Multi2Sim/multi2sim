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

#include <evergreen-emu.h>
#include <mem-system.h>
#include <x86-emu.h>
#include <evergreen-timing.h>



/*
 * Global variables
 */


struct evg_emu_t *evg_emu;

long long evg_emu_max_cycles = 0;
long long evg_emu_max_inst = 0;
int evg_emu_max_kernels = 0;

enum evg_emu_kind_t evg_emu_kind = evg_emu_kind_functional;

char *evg_emu_opencl_binary_name = "";
char *evg_emu_report_file_name = "";
FILE *evg_emu_report_file = NULL;

int evg_emu_wavefront_size = 64;





/*
 * GPU Kernel (Functional Simulator) Public Functions
 */



/* Initialize GPU kernel */
void evg_emu_init()
{
	/* Open report file */
	if (*evg_emu_report_file_name)
	{
		evg_emu_report_file = file_open_for_write(evg_emu_report_file_name);
		if (!evg_emu_report_file)
			fatal("%s: cannot open report for Evergreen emulator",
				evg_emu_report_file_name);
	}

	/* Allocate */
	evg_emu = calloc(1, sizeof(struct evg_emu_t));
	if (!evg_emu)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	evg_emu->timer = m2s_timer_create("Evergreen GPU timer");
	evg_emu->const_mem = mem_create();
	evg_emu->const_mem->safe = 0;
	evg_emu->global_mem = mem_create();
	evg_emu->global_mem->safe = 0;

	/* Initialize disassembler (decoding tables...) */
	evg_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	evg_isa_init();

	/* Initialize OpenCL objects */
	evg_emu->opencl_repo = evg_opencl_repo_create();
	evg_emu->opencl_platform = evg_opencl_platform_create();
	evg_emu->opencl_device = evg_opencl_device_create();
}


/* Finalize GPU kernel */
void evg_emu_done()
{
	/* GPU report */
	if (evg_emu_report_file)
		fclose(evg_emu_report_file);

	/* Free ND-Ranges */
	while (evg_emu->ndrange_list_count)
		evg_ndrange_free(evg_emu->ndrange_list_head);

	/* Free OpenCL objects */
	evg_opencl_repo_free_all_objects(evg_emu->opencl_repo);
	evg_opencl_repo_free(evg_emu->opencl_repo);

	/* Finalize disassembler */
	evg_disasm_done();

	/* Finalize ISA */
	evg_isa_done();

	/* Finalize GPU kernel */
	mem_free(evg_emu->const_mem);
	mem_free(evg_emu->global_mem);
	m2s_timer_free(evg_emu->timer);
	free(evg_emu);
}


void evg_emu_dump_summary(FILE *f)
{
	double time_in_sec;
	double inst_per_sec;

	/* If there was no Evergreen simulation, no summary */
	if (!evg_emu->ndrange_count)
		return;

	/* Calculate statistics */
	time_in_sec = (double) m2s_timer_get_value(evg_emu->timer) / 1.0e6;
	inst_per_sec = time_in_sec > 0.0 ? (double) evg_emu->inst_count / time_in_sec : 0.0;

	/* Print statistics */
	fprintf(f, "[ Evergreen ]\n");
	fprintf(f, "SimType = %s\n", evg_emu_kind == evg_emu_kind_functional ?
			"Functional" : "Detailed");
	fprintf(f, "Time = %.2f\n", time_in_sec);
	fprintf(f, "NDRangeCount = %d\n", evg_emu->ndrange_count);
	fprintf(f, "Instructions = %lld\n", evg_emu->inst_count);
	fprintf(f, "InstructionsPerSecond = %.0f\n", inst_per_sec);

	/* Detailed simulation */
	if (evg_emu_kind == evg_emu_kind_detailed)
		evg_gpu_dump_summary(f);

	/* End */
	fprintf(f, "\n");
}


/* If 'fullpath' points to the original OpenCL library, redirect it to 'm2s-opencl.so'
 * in the same path. */
void evg_emu_libopencl_redirect(struct x86_ctx_t *ctx, char *full_path, int size)
{
	char fullpath_original[MAX_PATH_SIZE];
	char buf[MAX_PATH_SIZE];
	char *relpath, *filename;
	int length;
	FILE *f;

	/* Get path length */
	snprintf(fullpath_original, sizeof fullpath_original, "%s", full_path);
	length = strlen(full_path);
	relpath = rindex(full_path, '/');
	assert(relpath && *relpath == '/');
	filename = relpath + 1;

	/* Detect an attempt to open 'libm2s-opencl' and record it */
	if (!strcmp(filename, "libm2s-opencl.so"))
	{
		/* Attempt to open original location */
		f = fopen(full_path, "r");
		if (f)
		{
			fclose(f);
			ctx->libopencl_open_attempt = 0;
		}
		else
		{
			/* Attempt to open 'libm2s-openc.so' in current directory */
			if (!getcwd(buf, MAX_PATH_SIZE))
				fatal("%s: cannot get current directory", __FUNCTION__);
			sprintf(full_path, "%s/libm2s-opencl.so", buf);
			f = fopen(full_path, "r");
			if (f)
			{
				fclose(f);
				warning("path '%s' has been redirected to '%s'\n"
					"\tYour application is trying to access the Multi2Sim OpenCL library. A copy of\n"
					"\tthis library has been found in the current working directory, and this copy\n"
					"\twill be used in the linker. To avoid this message, please link your program\n"
					"\tstatically. See the Multi2Sim Guide for further details (www.multi2sim.org).\n",
					fullpath_original, full_path);
				ctx->libopencl_open_attempt = 0;
			}
			else
			{
				/* Attemps failed, record this. */
				ctx->libopencl_open_attempt = 1;
			}
		}
	}

	/* Translate libOpenCL -> libm2s-opencl */
	if (!strcmp(filename, "libOpenCL.so") || !strncmp(filename, "libOpenCL.so.", 13))
	{
		/* Translate name in same path */
		full_path[length - 13] = '\0';
		snprintf(buf, MAX_STRING_SIZE, "%s/libm2s-opencl.so", full_path);
		strncpy(full_path, buf, size);
		f = fopen(full_path, "r");

		/* If attempt failed, translate name into current working directory */
		if (!f)
		{
			if (!getcwd(buf, MAX_PATH_SIZE))
				fatal("%s: cannot get current directory", __FUNCTION__);
			sprintf(full_path, "%s/libm2s-opencl.so", buf);
			f = fopen(full_path, "r");
		}

		/* End of attempts */
		if (f)
		{
			fclose(f);
			warning("path '%s' has been redirected to '%s'\n"
				"\tYour application is trying to access the default OpenCL library, which is being\n"
				"\tredirected by Multi2Sim to its own provided library. Though this should work,\n"
				"\tthe safest way to simulate an OpenCL program is by linking it initially with\n"
				"\t'libm2s-opencl.so'. See the Multi2Sim Guide for further details (www.multi2sim.org).\n",
				fullpath_original, full_path);
			ctx->libopencl_open_attempt = 0;
		}
		else
		{
			ctx->libopencl_open_attempt = 1;
		}
	}
}


/* Dump a warning about failed attempts of context to access OpenCL library */
void evg_emu_libopencl_failed(int pid)
{
	warning("context %d finished after failing to access OpenCL library.\n"
		"\tMulti2Sim has detected several attempts to access 'libm2s-opencl.so' by your\n"
		"\tapplication's dynamic linker. Please, make sure that this file is available\n"
		"\teither in any shared library path, in the current working directory, or in\n"
		"\tany directory pointed by the environment variable LD_LIBRARY_PATH. See the\n"
		"\tMulti2Sim Guide for further details (www.multi2sim.org).\n",
		pid);
}


/* GPU disassembler tool */
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

/* GPU OpenGL disassembler tool */
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


/* Run one iteration of the Evergreen GPU emulation loop.
 * Return FALSE if there is no more emulation to perform. */
int evg_emu_run(void)
{
	struct evg_ndrange_t *ndrange;
	struct evg_ndrange_t *ndrange_next;

	struct evg_work_group_t *work_group;
	struct evg_work_group_t *work_group_next;

	struct evg_wavefront_t *wavefront;
	struct evg_wavefront_t *wavefront_next;

	/* Exit if there are no ND-Ranges to emulate */
	if (!evg_emu->ndrange_list_count)
		return 0;

	/* Start any ND-Range in state 'pending' */
	while ((ndrange = evg_emu->pending_ndrange_list_head))
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
	for (ndrange = evg_emu->running_ndrange_list_head; ndrange; ndrange = ndrange_next)
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
	while ((ndrange = evg_emu->finished_ndrange_list_head))
	{
		/* Dump ND-Range report */
		evg_ndrange_dump(ndrange, evg_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (evg_emu_max_kernels && evg_emu->ndrange_count >= evg_emu_max_kernels)
			esim_finish = esim_finish_evg_max_kernels;

		/* Extract from list of finished ND-Ranges and free */
		evg_ndrange_free(ndrange);
	}

	/* Return TRUE */
	return 1;
}
