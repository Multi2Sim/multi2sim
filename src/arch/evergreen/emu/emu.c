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

#include <evergreen-emu.h>
#include <x86-emu.h>



/*
 * Global variables
 */


struct evg_emu_t *evg_emu;

long long evg_emu_max_cycles = 0;
long long evg_emu_max_inst = 0;
int evg_emu_max_kernels = 0;

enum evg_emu_kind_t evg_emu_kind = evg_emu_functional;

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
	if (evg_emu_report_file_name[0]) {
		evg_emu_report_file = open_write(evg_emu_report_file_name);
		if (!evg_emu_report_file)
			fatal("%s: cannot open GPU report file ", evg_emu_report_file_name);
	}

	/* Initialize kernel */
	evg_emu = calloc(1, sizeof(struct evg_emu_t));
	evg_emu->const_mem = mem_create();
	evg_emu->const_mem->safe = 0;
	evg_emu->global_mem = mem_create();
	evg_emu->global_mem->safe = 0;

	/* Initialize disassembler (decoding tables...) */
	evg_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	evg_isa_init();

	/* Create platform and device */
	evg_opencl_object_list = linked_list_create();
	evg_opencl_platform = evg_opencl_platform_create();
	evg_opencl_device_create();
}


/* Finalize GPU kernel */
void evg_emu_done()
{
	/* GPU report */
	if (evg_emu_report_file)
		fclose(evg_emu_report_file);

	/* Free OpenCL objects */
	evg_opencl_object_free_all();
	linked_list_free(evg_opencl_object_list);

	/* Finalize disassembler */
	evg_disasm_done();

	/* Finalize ISA */
	evg_isa_done();

	/* Finalize GPU kernel */
	mem_free(evg_emu->const_mem);
	mem_free(evg_emu->global_mem);
	free(evg_emu);
}


void evg_emu_timer_start(void)
{
	assert(!evg_emu->timer_running);
	evg_emu->timer_start_time = x86_emu_timer();
	evg_emu->timer_running = 1;
}


void evg_emu_timer_stop(void)
{
	assert(evg_emu->timer_running);
	evg_emu->timer_acc += x86_emu_timer() - evg_emu->timer_start_time;
	evg_emu->timer_running = 0;
}


/* Return a counter of microseconds relative to the first time the GPU started to run.
 * This counter runs only while the GPU is active, stopping and resuming after calls
 * to 'evg_emu_timer_stop()' and 'evg_emu_timer_start()', respectively. */
long long evg_emu_timer(void)
{
	return evg_emu->timer_running ? x86_emu_timer() - evg_emu->timer_start_time + evg_emu->timer_acc
		: evg_emu->timer_acc;
}


/* If 'fullpath' points to the original OpenCL library, redirect it to 'm2s-opencl.so'
 * in the same path. */
void evg_emu_libopencl_redirect(char *fullpath, int size)
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

	struct evg_opengl_bin_t *amd_opengl_bin;
	struct evg_opengl_shader_t *amd_opengl_shader;

	/* Initialize disassembler */
	evg_disasm_init();

	/* Load file into memory buffer */
	file_buffer = read_buffer(path, &file_size);
	if(!file_buffer)
		fatal("%s:Invalid file!", path);

	/* Analyze the file and initialize structure */	
	amd_opengl_bin = evg_opengl_bin_create(file_buffer, file_size, path);

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
	evg_opengl_bin_free(amd_opengl_bin);
	evg_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}
