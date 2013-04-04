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

#include <CL/cl.h>
#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>



/*
 * Global Variables
 */

int amd_dump_all;
int amd_list_devices;
char *amd_device_name = "";




/*
 *
 */

#define AMD_MAX_DEVICES  100

static int amd_num_devices;
//static int amd_device_index = -1;
static cl_device_id amd_device_list[AMD_MAX_DEVICES];
static cl_context amd_context;
//static cl_device_id amd_device;



#if 0
char *output_file_prefix = "kernel";
char *input_file_name;

int debug_info = 0;  /* Debug info and no optimizations */

char *kernel_file_name = NULL;  /* Kernel source file */
char kernel_file_prefix[MAX_STRING_SIZE];  /* Prefix used for output files */
char clcc_out_file_name[MAX_STRING_SIZE];  /* Name of binary */
	
cl_platform_id platform;
cl_context context;

int amd_num_devices = 0;
int amd_device_index = -1;
cl_device_id amd_device_list[MAX_DEVICES];
cl_device_id amd_device;


void kernel_binary_analyze_inner_elf(char *file_name)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;

	char file_name_dest[MAX_STRING_SIZE];

	int index;
	FILE *f;

	/* Open File */
	elf_file = elf_file_create_from_path(file_name);

	/* Explore sections */
	LIST_FOR_EACH(elf_file->section_list, index)
	{
		/* Get section */
		section = list_get(elf_file->section_list, index);
		if (!*section->name)
			continue;

		/* Open file to dump section */
		snprintf(file_name_dest, sizeof file_name_dest, "%s.%d%s",
			file_name, index, section->name);
		f = fopen(file_name_dest, "wb");
		if (!f)
			fatal("%s: cannot open file '%s'",
				__FUNCTION__, file_name_dest);

		/* Dump section and close file */
		elf_buffer_dump(&section->buffer, f);
		fclose(f);

		/* Info */
		printf("\t  %s: inner section '%s' dumped\n",
			file_name_dest, section->name);
	}

	/* Close ELF file */
	elf_file_free(elf_file);
}


void kernel_binary_analyze(char *file_name)
{
	struct elf_file_t *elf_file;

	int index;
	FILE *f;

	char file_name_prefix[MAX_STRING_SIZE];
	char subdir[MAX_STRING_SIZE];
	int len;

	char file_name_dest[MAX_STRING_SIZE];

	/* Get file name prefix */
	strcpy(file_name_prefix, file_name);
	len = strlen(file_name);
	if (len > 4 && !strcmp(file_name + len - 4, ".bin"))
		file_name_prefix[len - 4] = '\0';

	/* Create subdirectory */
	snprintf(subdir, sizeof subdir, "%s_files", file_name_prefix);
	mkdir(subdir, 0755);

	/* Analyze ELF file */
	elf_file = elf_file_create_from_path(file_name);
	if (!elf_file)
		fatal("%s: cannot open ELF file", file_name);
	
	/* List ELF sections */
	printf("ELF sections:\n");
	LIST_FOR_EACH(elf_file->section_list, index)
	{
		struct elf_section_t *section;

		/* Get section */
		section = list_get(elf_file->section_list, index);
		if (!section->header->sh_size)
			continue;

		/* Dump to file */
		snprintf(file_name_dest, sizeof file_name_dest, "%s/%s.%s",
			subdir, file_name_prefix,
			*section->name == '.' ? section->name + 1 : section->name);
		f = fopen(file_name_dest, "wb");
		if (!f)
			fatal("%s: cannot open file '%s'", __FUNCTION__,
				file_name_dest);
		
		/* Dump section and close file */
		elf_buffer_dump(&section->buffer, f);
		fclose(f);

		/* Info */
		printf("  section '%s': addr=0x%x, offset=0x%x, size=%d, flags=0x%x\n",
			section->name, section->header->sh_addr, section->header->sh_offset,
			section->header->sh_size, section->header->sh_flags);
	}
	
	/* Get symbols */
	LIST_FOR_EACH(elf_file->symbol_table, index)
	{
		struct elf_symbol_t *symbol;
		struct elf_section_t *section;

		char kernel_func_name[MAX_STRING_SIZE];
		int kernel_func_len;
		int symbol_length;

		size_t size;

		/* Get symbol */
		symbol = list_get(elf_file->symbol_table, index);
		if (strncmp(symbol->name, "__OpenCL_", 9))
			continue;
		symbol_length = strlen(symbol->name);
		if (!symbol_length)
			continue;

		/* Read section */
		section = list_get(elf_file->section_list, symbol->section);
		assert(section && symbol->value + symbol->size
			<= section->header->sh_size);

		/* Dump to files */
		if (str_suffix(symbol->name, "_metadata"))
		{
			/* Get kernel function name */
			kernel_func_len = symbol_length - 18;
			strncpy(kernel_func_name, symbol->name + 9, kernel_func_len);
			kernel_func_name[kernel_func_len] = '\0';

			/* Open file name */
			snprintf(file_name_dest, sizeof file_name_dest, "%s/%s.%s.metadata",
				subdir, file_name_prefix, kernel_func_name);
			f = fopen(file_name_dest, "wb");
			if (!f)
				fatal("%s: cannot create file '%s'", __FUNCTION__,
					file_name_dest);

			/* Dump buffer */
			size = fwrite(section->buffer.ptr + symbol->value, 1, symbol->size, f);
			if (size != symbol->size)
				fatal("%s: cannot dump '%s' symbol contents", __FUNCTION__,
					symbol->name);

			/* Close output file */
			printf("\t%s: meta data dumped\n", file_name_dest);
			fclose(f);

		}
		else if (str_suffix(symbol->name, "_kernel"))
		{
			/* Get kernel function name */
			kernel_func_len = symbol_length - 16;
			strncpy(kernel_func_name, symbol->name + 9, kernel_func_len);
			kernel_func_name[kernel_func_len] = '\0';

			/* Open file name */
			sprintf(file_name_dest, "%s/%s.%s.kernel",
				subdir, file_name_prefix, kernel_func_name);
			f = fopen(file_name_dest, "wb");
			if (!f)
				fatal("%s: cannot create file '%s'", __FUNCTION__,
					file_name_dest);

			/* Dump buffer */
			size = fwrite(section->buffer.ptr + symbol->value, 1, symbol->size, f);
			if (size != symbol->size)
				fatal("%s: cannot dump '%s' symbol contents", __FUNCTION__,
					symbol->name);

			/* Close output file */
			printf("\t%s: inner ELF file dumped\n", file_name_dest);
			fclose(f);

			/* Analyze inner ELF */
			kernel_binary_analyze_inner_elf(file_name_dest);
		}
	}

	/* Close ELF file */
	elf_file_free(elf_file);
}


void main_compile_kernel()
{
	char device_name[MAX_STRING_SIZE];

	char *kernel_file_ext = ".cl";
	int len;
	int extlen;
	int size;

	char *program_source;
	size_t program_source_size;

	char compiler_flags[MAX_STRING_SIZE];

	size_t bin_sizes[MAX_DEVICES];
	size_t bin_sizes_ret;
	char *bin_bits[MAX_DEVICES];

	cl_int err;

	/* Get amd_device info */
	if (amd_device_index < 0)
		fatal("no amd_device specified; use '-d' option");
	clGetDeviceInfo(amd_device, CL_DEVICE_NAME, MAX_STRING_SIZE, device_name, NULL);
	
	/* Message */
	printf("\n");
	printf("Device %d selected: %s\n", amd_device_index, device_name);
	printf("Compiling '%s'...\n", kernel_file_name);
	
	/* Get kernel prefix */
	extlen = strlen(kernel_file_ext);
	strncpy(kernel_file_prefix, kernel_file_name, MAX_STRING_SIZE);
	len = strlen(kernel_file_name);
	if (len > extlen && !strcmp(&kernel_file_name[len - extlen], kernel_file_ext))
		kernel_file_prefix[len - extlen] = 0;

	/* Assign output file name if it was not specified with option '-o' */
	if (!clcc_out_file_name[0])
		snprintf(clcc_out_file_name, MAX_STRING_SIZE, "%s.bin", kernel_file_prefix);
	
	/* Read the program source */
	program_source = read_buffer(kernel_file_name, &size);
	program_source_size = size;
	if (!program_source)
		fatal("%s: cannot open kernel\n", kernel_file_name);
	
	/* Create program */
	cl_program program;
	program = clCreateProgramWithSource(context, 1, (const char **) &program_source, &program_source_size, &err);
	if (err != CL_SUCCESS)
		fatal("clCreateProgramWithSource failed");
	
	/* Compiler flags */
	strcpy(compiler_flags, "");
	if (debug_info)
		strcat(compiler_flags, " -O0 -g");
	
	/* Intermediate files */
	if (amd_dump_all)
	{
		char dir[MAX_STRING_SIZE];
		char dir_flag[MAX_STRING_SIZE];

		snprintf(dir, sizeof dir, "%s_AMDAPP_files", kernel_file_prefix);
		snprintf(dir_flag, sizeof dir_flag, "-save-temps=%s/%s",
			dir, kernel_file_prefix);
		mkdir(dir, 0755);
		strcat(compiler_flags, dir_flag);
	}

	/* Compile source */
	err = clBuildProgram(program, 1, &amd_device, compiler_flags, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char buf[0x10000];

		clGetProgramBuildInfo(program, amd_device, CL_PROGRAM_BUILD_LOG, sizeof buf, buf, NULL);
		fprintf(stderr, "\n%s\n", buf);
		fatal("compilation failed");
	}
	free(program_source);

	/* Get number and size of binaries */
	clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(bin_sizes), bin_sizes, &bin_sizes_ret);
	amd_num_devices = bin_sizes_ret / sizeof(size_t);
	for (amd_device_index = 0; amd_device_index < amd_num_devices; amd_device_index++)
		if (bin_sizes[amd_device_index])
			break;
	if (amd_device_index == amd_num_devices)
		fatal("no binary generated");

	/* Dump binary into file */
	memset(bin_bits, 0, sizeof(bin_bits));
	bin_bits[amd_device_index] = xmalloc(bin_sizes[amd_device_index]);
	clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(bin_bits), bin_bits, NULL);
	write_buffer(clcc_out_file_name, bin_bits[amd_device_index], bin_sizes[amd_device_index]);
	free(bin_bits[amd_device_index]);
	printf("\t%s: kernel binary created\n", clcc_out_file_name);

	/* Process generated binary */
	if (amd_dump_all)
		kernel_binary_analyze(clcc_out_file_name);
}


int read_device(char *amd_device_name)
{
	char *endptr;
	char name[MAX_STRING_SIZE];
	char *token;
	char *delim;
	int i;

	/* Try to interpret 'amd_device_name' as a number */
	amd_device_index = strtol(amd_device_name, &endptr, 10);
	if (!*endptr)
	{
		if (amd_device_index >= amd_num_devices)
			fatal("%d is not a valid amd_device ID; use '-l' option for a list of valid IDs",
				amd_device_index);
		return amd_device_index;
	}

	/* 'amd_device_name' is a string. If the given name matches any of the tokens
	 * in the amd_device name, that amd_device will be selected (first occurrence). */
	for (i = 0; i < amd_num_devices; i++)
	{
		clGetDeviceInfo(amd_device_list[i], CL_DEVICE_NAME, sizeof name, name, NULL);
		delim = ", ";
		for (token = strtok(name, " ,"); token; token = strtok(NULL, delim))
			if (!strcasecmp(amd_device_name, token))
				return i;
	}
	fatal("'%s' is not a valid amd_device name; use '-l' for a list of supported amd_device_list",
		amd_device_name);
	return -1;
}

#endif



#define CL_CONTEXT_OFFLINE_DEVICES_AMD  0x403f

void amd_init(void)
{
	/* Platform */
	cl_int err;
	cl_platform_id platform;
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS)
		fatal("%s: cannot get OpenCL platform", __FUNCTION__);
	
	/* Get context */
	cl_context_properties cprops[5];
	cprops[0] = CL_CONTEXT_PLATFORM;
	cprops[1] = (cl_context_properties) platform;
	cprops[2] = CL_CONTEXT_OFFLINE_DEVICES_AMD;
	cprops[3] = (cl_context_properties) 1;
	cprops[4] = (cl_context_properties) NULL;
	amd_context = clCreateContextFromType(cprops, CL_DEVICE_TYPE_ALL, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		fatal("%s: cannot create OpenCL context", __FUNCTION__);
	
	/* Get device list from context */
	err = clGetContextInfo(amd_context, CL_CONTEXT_NUM_DEVICES, sizeof amd_num_devices,
			&amd_num_devices, NULL);
	err |= clGetContextInfo(amd_context, CL_CONTEXT_DEVICES, sizeof amd_device_list,
			amd_device_list, NULL);
	if (err != CL_SUCCESS)
		fatal("%s: cannot get OpenCL device list", __FUNCTION__);
	
}


void amd_done(void)
{
}


void amd_dump_device_list(FILE *f)
{
	int i;
	char name[MAX_STRING_SIZE];
	char vendor[MAX_STRING_SIZE];

	/* Initialize */
	amd_init();

	/* List amd_device_list */
	fprintf(f, "\n ID   Name, Vendor\n");
	fprintf(f, "----  ----------------------------------------------------\n");
	for (i = 0; i < amd_num_devices; i++)
	{
		clGetDeviceInfo(amd_device_list[i], CL_DEVICE_NAME, MAX_STRING_SIZE, name, NULL);
		clGetDeviceInfo(amd_device_list[i], CL_DEVICE_VENDOR, MAX_STRING_SIZE, vendor, NULL);
		fprintf(f, " %2d  %s, %s\n", i, name, vendor);
	}
	fprintf(f, "----------------------------------------------------------\n");
	fprintf(f, "\t%d devices available\n\n", amd_num_devices);

	/* Finish */
	amd_done();
	exit(0);
}

