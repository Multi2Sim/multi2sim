/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <gpukernel.h>
#include <gpudisasm.h>
#include <cpukernel.h>



/*
 * Global variables
 */


struct gk_t *gk;

long long gpu_max_cycles = 0;
long long gpu_max_inst = 0;
int gpu_max_kernels = 0;

enum gpu_sim_kind_t gpu_sim_kind = gpu_sim_functional;

char *gpu_opencl_binary_name = "";
char *gpu_kernel_report_file_name = "";
FILE *gpu_kernel_report_file = NULL;

int gpu_wavefront_size = 64;





/*
 * GPU Kernel (Functional Simulator) Public Functions
 */



/* Initialize GPU kernel */
void gk_init()
{
	/* Open report file */
	if (gpu_kernel_report_file_name[0]) {
		gpu_kernel_report_file = open_write(gpu_kernel_report_file_name);
		if (!gpu_kernel_report_file)
			fatal("%s: cannot open GPU report file ", gpu_kernel_report_file_name);
	}

	/* Initialize kernel */
	gk = calloc(1, sizeof(struct gk_t));
	gk->const_mem = mem_create();
	gk->const_mem->safe = 0;
	gk->global_mem = mem_create();
	gk->global_mem->safe = 0;

	/* Initialize disassembler (decoding tables...) */
	amd_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	gpu_isa_init();

	/* Create platform and device */
	opencl_object_list = linked_list_create();
	opencl_platform = opencl_platform_create();
	opencl_device_create();
}


/* Finalize GPU kernel */
void gk_done()
{
	/* GPU report */
	if (gpu_kernel_report_file)
		fclose(gpu_kernel_report_file);

	/* Free OpenCL objects */
	opencl_object_free_all();
	linked_list_free(opencl_object_list);

	/* Finalize disassembler */
	amd_disasm_done();

	/* Finalize ISA */
	gpu_isa_done();

	/* Finalize GPU kernel */
	mem_free(gk->const_mem);
	mem_free(gk->global_mem);
	free(gk);
}


void gk_timer_start(void)
{
	assert(!gk->timer_running);
	gk->timer_start_time = x86_emu_timer();
	gk->timer_running = 1;
}


void gk_timer_stop(void)
{
	assert(gk->timer_running);
	gk->timer_acc += x86_emu_timer() - gk->timer_start_time;
	gk->timer_running = 0;
}


/* Return a counter of microseconds relative to the first time the GPU started to run.
 * This counter runs only while the GPU is active, stopping and resuming after calls
 * to 'gk_timer_stop()' and 'gk_timer_start()', respectively. */
long long gk_timer(void)
{
	return gk->timer_running ? x86_emu_timer() - gk->timer_start_time + gk->timer_acc
		: gk->timer_acc;
}


/* If 'fullpath' points to the original OpenCL library, redirect it to 'm2s-opencl.so'
 * in the same path. */
void gk_libopencl_redirect(char *fullpath, int size)
{
	char fullpath_original[MAX_PATH_SIZE];
	char buf[MAX_PATH_SIZE];
	char *relpath, *filename;
	int length;
	FILE *f;

	/* Get path length */
	strncpy(fullpath_original, fullpath, MAX_PATH_SIZE);
	length = strlen(fullpath);
	relpath = rindex(fullpath, '/');
	assert(relpath && *relpath == '/');
	filename = relpath + 1;

	/* Detect an attempt to open 'libm2s-opencl' and record it */
	if (!strcmp(filename, "libm2s-opencl.so"))
	{
		/* Attempt to open original location */
		f = fopen(fullpath, "r");
		if (f)
		{
			fclose(f);
			x86_isa_ctx->libopencl_open_attempt = 0;
		}
		else
		{
			/* Attempt to open 'libm2s-openc.so' in current directory */
			if (!getcwd(buf, MAX_PATH_SIZE))
				fatal("%s: cannot get current directory", __FUNCTION__);
			sprintf(fullpath, "%s/libm2s-opencl.so", buf);
			f = fopen(fullpath, "r");
			if (f)
			{
				fclose(f);
				warning("path '%s' has been redirected to '%s'\n"
					"\tYour application is trying to access the Multi2Sim OpenCL library. A copy of\n"
					"\tthis library has been found in the current working directory, and this copy\n"
					"\twill be used in the linker. To avoid this message, please link your program\n"
					"\tstatically. See the Multi2Sim Guide for further details (www.multi2sim.org).\n",
					fullpath_original, fullpath);
				x86_isa_ctx->libopencl_open_attempt = 0;
			}
			else
			{
				/* Attemps failed, record this. */
				x86_isa_ctx->libopencl_open_attempt = 1;
			}
		}
	}

	/* Translate libOpenCL -> libm2s-opencl */
	if (!strcmp(filename, "libOpenCL.so") || !strncmp(filename, "libOpenCL.so.", 13))
	{
		/* Translate name in same path */
		fullpath[length - 13] = '\0';
		snprintf(buf, MAX_STRING_SIZE, "%s/libm2s-opencl.so", fullpath);
		strncpy(fullpath, buf, size);
		f = fopen(fullpath, "r");

		/* If attempt failed, translate name into current working directory */
		if (!f)
		{
			if (!getcwd(buf, MAX_PATH_SIZE))
				fatal("%s: cannot get current directory", __FUNCTION__);
			sprintf(fullpath, "%s/libm2s-opencl.so", buf);
			f = fopen(fullpath, "r");
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
				fullpath_original, fullpath);
			x86_isa_ctx->libopencl_open_attempt = 0;
		}
		else
		{
			x86_isa_ctx->libopencl_open_attempt = 1;
		}
	}
}


/* Dump a warning about failed attempts of context to access OpenCL library */
void gk_libopencl_failed(int pid)
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
void gk_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;

	struct amd_bin_t *amd_bin;

	char kernel_name[MAX_STRING_SIZE];

	int i;

	/* Initialize disassembler */
	amd_disasm_init();

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
			amd_bin = amd_bin_create(section->buffer.ptr + symbol->value, symbol->size, kernel_name);

			/* Get kernel name */
			printf("**\n** Disassembly for '__kernel %s'\n**\n\n", kernel_name);
			amd_disasm_buffer(&amd_bin->enc_dict_entry_evergreen->sec_text_buffer, stdout);
			printf("\n\n\n");

			/* Free internal ELF */
			amd_bin_free(amd_bin);
		}
	}

	/* Free external ELF */
	elf_file_free(elf_file);
	amd_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}

/* GPU OpenGL disassembler tool */
void gl_disasm(char *path, int opengl_shader_index)
{
	void *file_buffer;
	int file_size;

	struct amd_opengl_bin_t *amd_opengl_bin;
	struct amd_opengl_shader_t *amd_opengl_shader;

	/* Initialize disassembler */
	amd_disasm_init();

	/* Load file into memory buffer */
	file_buffer = read_buffer(path, &file_size);
	if(!file_buffer)
		fatal("%s:Invalid file!", path);

	/* Analyze the file and initialize structure */	
	amd_opengl_bin = amd_opengl_bin_create(file_buffer, file_size, path);

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
	amd_disasm_buffer(&amd_opengl_shader->isa_buffer, stdout);
	printf("\n\n\n");

	/* Free */
	amd_opengl_bin_free(amd_opengl_bin);
	amd_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}
