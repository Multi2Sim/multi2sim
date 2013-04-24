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
#include <stdio.h>
#include <sys/stat.h>

#include <clcc/amd/amd.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>




/*
 * Global Variables
 */

int amd_native;
int amd_dump_all;
int amd_list_devices;
char *amd_device_name = "";




/*
 *
 */

#define AMD_MAX_DEVICES  100

static int amd_num_devices;
static cl_device_id amd_device_list[AMD_MAX_DEVICES];
static cl_context amd_context;
static cl_device_id amd_device;


static void amd_binary_analyze_inner_elf(char *file_name)
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
		printf("\t\t%s - inner section '%s' dumped\n",
			file_name_dest, section->name);
	}

	/* Close ELF file */
	elf_file_free(elf_file);
}


static void amd_binary_analyze(char *file_name)
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
	snprintf(subdir, sizeof subdir, "%s_m2s_files", file_name_prefix);
	mkdir(subdir, 0755);

	/* Analyze ELF file */
	elf_file = elf_file_create_from_path(file_name);
	if (!elf_file)
		fatal("%s: cannot open ELF file", file_name);
	
	/* List ELF sections */
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
		printf("\t%s - ELF section '%s' dumped (%d bytes)\n",
			file_name_dest, section->name, section->header->sh_size);
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
			printf("\t%s - metadata dumped\n", file_name_dest);
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
			printf("\t%s - inner ELF file dumped\n", file_name_dest);
			fclose(f);

			/* Analyze inner ELF */
			amd_binary_analyze_inner_elf(file_name_dest);
		}
	}

	/* Close ELF file */
	elf_file_free(elf_file);
}


#define AMD_MAX_DEVICES  100

static void amd_compile_source(char *source_file_name, char *out_file_name)
{
	char *source_file_ext;
	char out_file_name_root[MAX_STRING_SIZE];
	char out_file_name_str[MAX_STRING_SIZE];
	char compiler_flags[MAX_STRING_SIZE];
	
	int size;
	int index;
	
	size_t program_source_size;
	size_t bin_sizes[AMD_MAX_DEVICES];
	size_t bin_sizes_ret;
	
	char *program_source;
	char *bin_bits[AMD_MAX_DEVICES];
	
	cl_int err;


	/* Get source file without '.cl' suffix */
	source_file_ext = ".cl";
	snprintf(out_file_name_root, sizeof out_file_name_root, "%s", source_file_name);
	if (str_suffix(out_file_name_root, source_file_ext))
		out_file_name_root[strlen(out_file_name_root) - strlen(source_file_ext)] = '\0';

	/* Compute output file name if not given. In either case, the output
	 * file is placed in variable 'out_file_name_str'. */
	if (!out_file_name || !*out_file_name)
		snprintf(out_file_name_str, sizeof out_file_name_str,
				"%s.bin", out_file_name_root);
	else
		snprintf(out_file_name_str, sizeof out_file_name_str,
				"%s", out_file_name);

	/* Read the program source */
	program_source = read_buffer(source_file_name, &size);
	program_source_size = size;
	if (!program_source)
		fatal("%s: cannot open file\n", source_file_name);
	
	/* Create program */
	cl_program program;
	program = clCreateProgramWithSource(amd_context, 1, (const char **) &program_source,
			&program_source_size, &err);
	if (err != CL_SUCCESS)
		fatal("%s: clCreateProgramWithSource failed", __FUNCTION__);
	
	/* Intermediate files */
	compiler_flags[0] = '\0';
	if (amd_dump_all)
	{
		char dir[MAX_STRING_SIZE];

		snprintf(dir, sizeof dir, "%s_amd_files", out_file_name_root);
		snprintf(compiler_flags, sizeof compiler_flags, "-save-temps=%s/%s",
			dir, out_file_name_root);
		mkdir(dir, 0755);
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
	clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof bin_sizes, bin_sizes, &bin_sizes_ret);
	amd_num_devices = bin_sizes_ret / sizeof(size_t);
	for (index = 0; index < amd_num_devices; index++)
		if (bin_sizes[index])
			break;
	if (index == amd_num_devices)
		fatal("%s: no binary generated", __FUNCTION__);

	/* Dump binary into file */
	memset(bin_bits, 0, sizeof bin_bits);
	bin_bits[index] = xmalloc(bin_sizes[index]);
	clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof bin_bits, bin_bits, NULL);
	if (!write_buffer(out_file_name_str, bin_bits[index], bin_sizes[index]))
		fatal("%s: cannot write to file", out_file_name_str);
	
	/* Free buffers */
	free(bin_bits[index]);
	printf("\t%s - kernel binary created\n", out_file_name_str);

	/* Process generated binary */
	if (amd_dump_all)
	{
		printf("\t%s/* - AMD intermediate files dumped\n", out_file_name_root);
		amd_binary_analyze(out_file_name_str);
	}
}




/* Read global variable 'amd_device_name' and set variable 'amd_device' to the
 * OpenCL device it refers to. */
static void amd_read_device(void)
{
	char *endptr;
	char name[MAX_STRING_SIZE];
	char *token;
	char *delim;

	int device_index;
	int i;

	/* No device given */
	if (!amd_device_name || !*amd_device_name)
		fatal("no device given, use option '--amd-device'");

	/* Try to interpret device name as a number */
	device_index = strtol(amd_device_name, &endptr, 10);
	if (!*endptr)
	{
		if (!IN_RANGE(device_index, 0, amd_num_devices - 1))
			fatal("invalid device index, use '--amd-list' for valid devices");
		amd_device = amd_device_list[device_index];
		goto out;
	}

	/* Device name given as a string. If the given name matches any of the tokens
	 * in the device name, that device will be selected (first occurrence). */
	for (i = 0; i < amd_num_devices; i++)
	{
		clGetDeviceInfo(amd_device_list[i], CL_DEVICE_NAME, sizeof name, name, NULL);
		delim = ", ";
		for (token = strtok(name, " ,"); token; token = strtok(NULL, delim))
		{
			if (!strcasecmp(amd_device_name, token))
			{
				amd_device = amd_device_list[i];
				goto out;
			}
		}
	}

	/* Invalid device */
	fatal("invalid device name, use '--amd-list' for valid devices");

out:
	/* Device found */
	clGetDeviceInfo(amd_device, CL_DEVICE_NAME, sizeof name, name, NULL);
	printf("Device '%s' selected\n", name);
}




/*
 * Public Functions
 */

/* Initialization. Global variables set here are:
 * - amd_context
 * - amd_num_devices
 * - amd_device_list
 */
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
	fprintf(f, "----------------------------------------------------------\n");
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
}


void amd_compile(struct list_t *source_file_list, char *out_file_name)
{
	int index;

	/* No input file given */
	if (!list_count(source_file_list))
		fatal("no input files given");
	
	/* Single output file not allowed for multiple sources */
	if (list_count(source_file_list) > 1 && *out_file_name)
		fatal("option '-o' not allowed for multiple sources");
	
	/* Initialize */
	amd_init();

	/* Read device from command line */
	amd_read_device();

	/* Compile source files */
	LIST_FOR_EACH(source_file_list, index)
		amd_compile_source(list_get(source_file_list, index),
				out_file_name);

	/* Finish */
	amd_done();
}

