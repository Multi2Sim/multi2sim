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
#include <CL/cl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>



#define MAX_STRING_SIZE  200
#define MAX_DEVICES  100
#define CL_CONTEXT_OFFLINE_DEVICES_AMD  0x403f


static char *syntax =
	"Syntax: %s [<options>] <kernel.cl>\n"
	"\tOptions:\n"
	"\t-l            Print list of available devices\n"
	"\t-d <dev>      Select target device for compilation\n"
	"\t-a            Dump intermediate files\n"
	"\t-e            ELF verbose\n"
	"\t-g            No optimizations (flags '-O0 -g' added)\n"
	"\t-v            Dump debug information\n";

char *output_file_prefix = "kernel";
char *input_file_name;

int verbose = 0;  /* Dump OpenCL calls results */
int debug_info = 0;  /* Debug info and no optimizations */
int action_list_devices = 0;  /* Dump list of devices */
int dump_intermediate = 0;  /* Dump intermediate files */
char *kernel_file_name = NULL;  /* Kernel source file */
char kernel_file_prefix[MAX_STRING_SIZE];  /* Prefix used for output files */
char bin_file_name[MAX_STRING_SIZE];  /* Name of binary */
	
cl_platform_id platform;
cl_context context;

int num_devices = 0;
int device_id = -1;
cl_device_id devices[MAX_DEVICES];
cl_device_id device;


void main_list_devices(FILE *f)
{
	int i;
	char name[MAX_STRING_SIZE], vendor[MAX_STRING_SIZE];

	/* List devices */
	fprintf(f, "\n ID    Name, Vendor\n");
	fprintf(f, "----  ----------------------------------------------------\n");
	for (i = 0; i < num_devices; i++) {
		clGetDeviceInfo(devices[i], CL_DEVICE_NAME, MAX_STRING_SIZE, name, NULL);
		clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, MAX_STRING_SIZE, vendor, NULL);
		fprintf(f, " %2d    %s %s\n", i, name, vendor);
	}
	fprintf(f, "----------------------------------------------------------\n");
	fprintf(f, "\t%d devices available\n\n", num_devices);
}


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

	/* Get device info */
	if (device_id < 0)
		fatal("no device specified; use '-d' option");
	clGetDeviceInfo(device, CL_DEVICE_NAME, MAX_STRING_SIZE, device_name, NULL);
	
	/* Message */
	printf("\n");
	printf("Device %d selected: %s\n", device_id, device_name);
	printf("Compiling '%s'...\n", kernel_file_name);
	
	/* Get kernel prefix */
	extlen = strlen(kernel_file_ext);
	strncpy(kernel_file_prefix, kernel_file_name, MAX_STRING_SIZE);
	len = strlen(kernel_file_name);
	if (len > extlen && !strcmp(&kernel_file_name[len - extlen], kernel_file_ext))
		kernel_file_prefix[len - extlen] = 0;
	snprintf(bin_file_name, MAX_STRING_SIZE, "%s.bin", kernel_file_prefix);
	
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
	if (dump_intermediate)
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
	err = clBuildProgram(program, 1, &device, compiler_flags, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char buf[0x10000];

		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof buf, buf, NULL);
		fprintf(stderr, "\n%s\n", buf);
		fatal("compilation failed");
	}
	free(program_source);

	/* Get number and size of binaries */
	clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(bin_sizes), bin_sizes, &bin_sizes_ret);
	num_devices = bin_sizes_ret / sizeof(size_t);
	for (device_id = 0; device_id < num_devices; device_id++)
		if (bin_sizes[device_id])
			break;
	if (device_id == num_devices)
		fatal("no binary generated");

	/* Dump binary into file */
	memset(bin_bits, 0, sizeof(bin_bits));
	bin_bits[device_id] = xmalloc(bin_sizes[device_id]);
	clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(bin_bits), bin_bits, NULL);
	write_buffer(bin_file_name, bin_bits[device_id], bin_sizes[device_id]);
	free(bin_bits[device_id]);
	printf("\t%s: kernel binary created\n", bin_file_name);

	/* Process generated binary */
	if (dump_intermediate)
		kernel_binary_analyze(bin_file_name);
}


int read_device(char *device_str)
{
	char *endptr;
	char name[MAX_STRING_SIZE];
	int i;

	/* Try to interpret 'device_str' as a number */
	device_id = strtol(device_str, &endptr, 10);
	if (!*endptr)
	{
		if (device_id >= num_devices)
			fatal("%d is not a valid device ID; use '-l' option for a list of valid IDs",
				device_id);
		return device_id;
	}

	/* 'device_str' is a string */
	for (i = 0; i < num_devices; i++)
	{
		clGetDeviceInfo(devices[i], CL_DEVICE_NAME, MAX_STRING_SIZE, name, NULL);
		if (!strcasecmp(device_str, name))
			return i;
	}
	fatal("'%s' is not a valid device name; use '-l' for a list of supported devices",
		device_str);
	return -1;
}


int main(int argc, char **argv)
{
	int opt;
	char *device_str = NULL;

	/* No arguments */
	if (argc == 1)
	{
		fprintf(stderr, syntax, argv[0]);
		return 1;
	}

	/* Process options */
	while ((opt = getopt(argc, argv, "ld:aeg")) != -1)
	{
		switch (opt)
		{

		case 'l':
			action_list_devices = 1;
			break;

		case 'd':
			device_str = optarg;
			break;

		case 'a':
			dump_intermediate = 1;
			break;

		case 'v':
			verbose = 1;
			break;

		case 'g':
			debug_info = 1;
			break;

		default:
			fprintf(stderr, syntax, argv[0]);
			return 1;
		}
	}

	/* The only remaining argument should be the kernel to compile */
	if (argc - optind > 1)
	{
		fprintf(stderr, syntax, argv[0]);
		return 1;
	}
	else if (argc - optind == 1)
		kernel_file_name = argv[optind];
	if (!kernel_file_name && !action_list_devices)
		fatal("no kernel to compile");

	/* Platform */
	cl_int err;
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS)
		fatal("cannot get OpenCL platform");
	
	/* Get context */
	cl_context_properties cprops[5];
	cprops[0] = CL_CONTEXT_PLATFORM;
	cprops[1] = (cl_context_properties)platform;
	cprops[2] = CL_CONTEXT_OFFLINE_DEVICES_AMD;
	cprops[3] = (cl_context_properties) 1;
	cprops[4] = (cl_context_properties) NULL;
	context = clCreateContextFromType(cprops, CL_DEVICE_TYPE_ALL, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		fatal("cannot create context");
	
	/* Get device list from context */
	err = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(num_devices), &num_devices, NULL);
	if (err != CL_SUCCESS)
		fatal("cannot get number of devices");
	err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(devices), devices, NULL);
	if (err != CL_SUCCESS)
		fatal("cannot get list of devices");
	
	/* Get selected device */
	if (device_str)
	{
		device_id = read_device(device_str);
		device = devices[device_id];
	}
	
	/* List available devices */
	if (action_list_devices)
		main_list_devices(stdout);
	
	/* Compile list of kernels */
	if (kernel_file_name)
		main_compile_kernel();
	
	/* End program */
	printf("\n");
	return 0;
}

