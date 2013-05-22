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

#include <clcc/clcc.h>
#include <clcc/define.h>
#include <clcc/amd/amd.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/elf-format.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>



#define AMD_MAX_DEVICES  100



/*
 * Global Variables
 */

int amd_dump_all;
int amd_list_devices;
char *amd_device_name = "";



/*
 * AMD Device Object
 */

struct amd_device_t
{
	cl_device_id device_id;
	char *name;
	struct list_t *token_list;
};


struct amd_device_t *amd_device_create(cl_device_id device_id, char *name)
{
	struct amd_device_t *device;

	device = xcalloc(1, sizeof(struct amd_device_t));
	device->device_id = device_id;
	device->name = xstrdup(name);
	device->token_list = str_token_list_create(name, " (),");

	return device;
}


void amd_device_free(struct amd_device_t *device)
{
	str_token_list_free(device->token_list);
	free(device->name);
	free(device);
}




/*
 * Private Functions
 */

static cl_context amd_context;
static struct list_t *amd_device_list;
static struct list_t *amd_selected_device_list;


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


/* Compile source in 'source_file_name' into binary 'out_file_name' (should contain
 * a valid string). If 'verbose' is set, additional output is provided about
 * the compilation. If 'dump_all' is set, intermediate files are dumped. */
static void amd_compile_source(char *source_file_name, char *out_file_name,
		struct amd_device_t *device, int verbose, int dump_all)
{
	char out_file_name_root[MAX_STRING_SIZE];
	char compiler_flags[MAX_LONG_STRING_SIZE];

	char *compiler_flags_ptr;
	int compiler_flags_size;
	
	int size;
	int index;
	int num_devices;
	
	size_t program_source_size;
	size_t bin_sizes[AMD_MAX_DEVICES];
	size_t bin_sizes_ret;
	
	char *source_file_ext;
	char *program_source;
	char *bin_bits[AMD_MAX_DEVICES];

	struct clcc_define_t *define;
	
	cl_int err;
		
	/* Get source file without '.cl' suffix */
	source_file_ext = ".cl";
	snprintf(out_file_name_root, sizeof out_file_name_root, "%s", source_file_name);
	if (str_suffix(out_file_name_root, source_file_ext))
		out_file_name_root[strlen(out_file_name_root) - strlen(source_file_ext)] = '\0';


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
	
	/* Initialize compiler flags */
	compiler_flags[0] = '\0';
	compiler_flags_ptr = compiler_flags;
	compiler_flags_size = sizeof compiler_flags;

	/* Dump intermediate files */
	if (amd_dump_all)
	{
		char dir[MAX_STRING_SIZE];

		snprintf(dir, sizeof dir, "%s_amd_files", out_file_name_root);
		str_printf(&compiler_flags_ptr, &compiler_flags_size,
				" -save-temps=%s/%s", dir, out_file_name_root);
		mkdir(dir, 0755);
	}

	/* Add #define macro definitions */
	LIST_FOR_EACH(clcc_define_list, index)
	{
		define = list_get(clcc_define_list, index);
		str_printf(&compiler_flags_ptr, &compiler_flags_size,
				" -D%s=%s", define->name, define->value);
	}

	/* Compile source */
	err = clBuildProgram(program, 1, &device->device_id, compiler_flags, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char buf[MAX_LONG_STRING_SIZE];

		clGetProgramBuildInfo(program, device->device_id, CL_PROGRAM_BUILD_LOG,
				sizeof buf, buf, NULL);
		fprintf(stderr, "\n%s\n", buf);
		fatal("compilation failed");
	}
	free(program_source);

	/* Get number and size of binaries */
	clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof bin_sizes, bin_sizes, &bin_sizes_ret);
	num_devices = bin_sizes_ret / sizeof(size_t);
	for (index = 0; index < num_devices; index++)
		if (bin_sizes[index])
			break;
	if (index == num_devices)
		fatal("%s: no binary generated", __FUNCTION__);

	/* Dump binary into file */
	memset(bin_bits, 0, sizeof bin_bits);
	bin_bits[index] = xmalloc(bin_sizes[index]);
	clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof bin_bits, bin_bits, NULL);
	if (!write_buffer(out_file_name, bin_bits[index], bin_sizes[index]))
		fatal("%s: cannot write to file", out_file_name);
	
	/* Free buffers */
	free(bin_bits[index]);
	if (verbose)
		printf("\t%s - kernel binary created\n", out_file_name);

	/* Process generated binary */
	if (dump_all)
	{
		printf("\t%s/* - AMD intermediate files dumped\n", out_file_name_root);
		amd_binary_analyze(out_file_name);
	}
}


static void amd_compile_source_packed(char *source_file_name, char *out_file_name)
{
	char temp_file_name[MAX_STRING_SIZE];

	struct amd_device_t *device;
	struct elf_file_t *kernel_file;
	struct elf_enc_file_t *file;
	struct elf_enc_buffer_t *buffer;
	struct elf_enc_section_t *section;
	struct elf_enc_symbol_t *symbol;
	struct elf_enc_symbol_table_t *symbol_table;

	int index;
	int machine;
	FILE *f;

	/* Create target ELF file */
	file = elf_enc_file_create();

	/* Create ELF symbol table */
	symbol_table = elf_enc_symbol_table_create(".symtab", ".strtab");
	elf_enc_file_add_symbol_table(file, symbol_table);

	/* Create temporary file to dump binaries */
	f = file_create_temp(temp_file_name, sizeof temp_file_name);

	/* Compile for each device */
	LIST_FOR_EACH(amd_selected_device_list, index)
	{
		/* Compile file for device */
		device = list_get(amd_selected_device_list, index);
		amd_compile_source(source_file_name, temp_file_name, device,
				FALSE, FALSE);

		/* Read the 'eh_machine' field */
		kernel_file = elf_file_create_from_path(temp_file_name);
		machine = kernel_file->header->e_machine;
		elf_file_free(kernel_file);

		/* Create ELF buffer */
		buffer = elf_enc_buffer_create();
		elf_enc_buffer_read_from_file(buffer, f);
		elf_enc_file_add_buffer(file, buffer);

		/* Create ELF section */
		section = elf_enc_section_create(".text", buffer, buffer);
		elf_enc_file_add_section(file, section);

		/* Create ELF symbol */
		symbol = elf_enc_symbol_create(device->name);
		symbol->symbol.st_value = machine;
		symbol->symbol.st_shndx = section->index;
		elf_enc_symbol_table_add(symbol_table, symbol);
	}

	/* Close temporary file */
	fclose(f);

	/* Generate ELF file */
	buffer = elf_enc_buffer_create();
	elf_enc_file_generate(file, buffer);

	/* Open output file */
	f = file_open_for_write(out_file_name);
	if (!f)
		fatal("%s: cannot create output file", out_file_name);

	/* Dump and close */
	elf_enc_buffer_write_to_file(buffer, f);
	elf_enc_buffer_free(buffer);
	fclose(f);

	/* Free ELF file */
	elf_enc_file_free(file);
	printf("\t%s - packed kernel binary created\n", out_file_name);
}




/* Read global variable 'amd_device_name' containing a list of devices split by
 * commas, and initialize the list of devices 'amd_device_list' with all
 * present devices. */
static void amd_read_device(void)
{
	struct list_t *token_list;
	struct amd_device_t *device;

	char *endptr;
	char *name;
	char *comma;

	int index;

	/* No device given */
	if (!amd_device_name || !*amd_device_name)
		fatal("no device given, use option '--amd-device'");

	/* Split the list of device names in tokens */
	token_list = str_token_list_create(amd_device_name, ",");
	if (!token_list->count)
		fatal("invalid list of devices");

	/* Extract devices */
	while (token_list->count)
	{
		/* Get device string */
		name = str_token_list_first(token_list);

		/* Try to interpret device name as a number */
		index = strtol(name, &endptr, 10);
		if (!*endptr)
		{
			if (!IN_RANGE(index, 0, amd_device_list->count - 1))
				fatal("invalid device index (%d), use '--amd-list' for a list",
						index);

			/* Get device */
			device = list_get(amd_device_list, index);
		}

		/* Device given by its name */
		else
		{
			LIST_FOR_EACH(amd_device_list, index)
			{
				device = list_get(amd_device_list, index);
				if (str_token_list_find_case(device->token_list, name) >= 0)
					break;
			}

			/* Not found */
			if (index == amd_device_list->count)
				fatal("%s: invalid device, use '--amd-list' for a list",
					name);

		}

		/* Check that device is not added yet */
		if (list_index_of(amd_selected_device_list, device) >= 0)
			fatal("%s: device added twice", device->name);

		/* Add selected device */
		list_add(amd_selected_device_list, device);

		/* Next token */
		str_token_list_shift(token_list);
	}

	/* Free tokens */
	str_token_list_free(token_list);

	/* Devices found */
	assert(amd_selected_device_list->count);
	printf("Selected devices: ");
	comma = "";
	LIST_FOR_EACH(amd_selected_device_list, index)
	{
		device = list_get(amd_selected_device_list, index);
		printf("%s%s", comma, device->name);
		comma = ", ";
	}
	printf("\n");
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
	struct amd_device_t *device;
	cl_device_id device_ids[AMD_MAX_DEVICES];
	char name[MAX_STRING_SIZE];

	int num_devices;
	int i;

	/* Initialize list of devices */
	amd_device_list = list_create();
	amd_selected_device_list = list_create();

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
	err = clGetContextInfo(amd_context, CL_CONTEXT_NUM_DEVICES, sizeof num_devices,
			&num_devices, NULL);
	err |= clGetContextInfo(amd_context, CL_CONTEXT_DEVICES, sizeof device_ids,
			device_ids, NULL);
	if (err != CL_SUCCESS)
		fatal("%s: cannot get OpenCL device list", __FUNCTION__);
	
	/* Add devices to device list */
	for (i = 0; i < num_devices; i++)
	{
		/* Get device name */
		clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, sizeof name, name, NULL);

		/* Create device and insert in list */
		device = amd_device_create(device_ids[i], name);
		list_add(amd_device_list, device);
	}
}


void amd_done(void)
{
	int index;

	/* Free list of devices */
	LIST_FOR_EACH(amd_device_list, index)
		amd_device_free(list_get(amd_device_list, index));
	list_free(amd_device_list);
	list_free(amd_selected_device_list);
}


void amd_dump_device_list(FILE *f)
{
	struct amd_device_t *device;
	char vendor[MAX_STRING_SIZE];
	int index;

	/* Initialize */
	amd_init();

	/* List amd_device_list */
	fprintf(f, "\n ID   Name, Vendor\n");
	fprintf(f, "----------------------------------------------------------\n");
	LIST_FOR_EACH(amd_device_list, index)
	{
		device = list_get(amd_device_list, index);
		clGetDeviceInfo(device->device_id, CL_DEVICE_VENDOR, MAX_STRING_SIZE, vendor, NULL);
		fprintf(f, " %2d  %s, %s\n", index, device->name, vendor);
	}
	fprintf(f, "----------------------------------------------------------\n");
	fprintf(f, "\t%d devices available\n\n", amd_device_list->count);

	/* Finish */
	amd_done();
}


void amd_compile(struct list_t *source_file_list,
		struct list_t *bin_file_list)
{
	char *source_file_name;
	char *bin_file_name;

	struct amd_device_t *device;
	int index;

	/* No input file given */
	if (!list_count(source_file_list))
		fatal("no input files given");
	
	/* Initialize */
	amd_init();

	/* Read device from command line */
	amd_read_device();

	/* Compile source files */
	LIST_FOR_EACH(source_file_list, index)
	{
		/* Get source and binary file */
		source_file_name = list_get(source_file_list, index);
		bin_file_name = list_get(bin_file_list, index);

		/* One device */
		assert(amd_selected_device_list->count);
		if (amd_selected_device_list->count == 1)
		{
			device = list_get(amd_selected_device_list, 0);
			amd_compile_source(source_file_name, bin_file_name, device,
					TRUE, amd_dump_all);
		}

		/* Multiple devices */
		else
		{
			amd_compile_source_packed(source_file_name,
					bin_file_name);
		}
	}

	/* Finish */
	amd_done();
}

