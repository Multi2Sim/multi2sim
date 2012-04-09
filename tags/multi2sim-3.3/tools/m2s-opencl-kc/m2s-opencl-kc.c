#include "elfanalyze.h"
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <misc.h>
#include <sys/stat.h>



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
	struct elf_file_t *elf;
	char *section_name;
	char file_name_dest[MAX_STRING_SIZE];
	void *section_buf;
	int i;

	elf = elf_open(file_name);
	if (!elf)
		return;

	for (i = 0; i < elf_section_count(elf); i++)
	{
		uint32_t addr, offset, size, flags;

		/* Get section info */
		elf_section_info(elf, i, &section_name, &addr, &offset, &size, &flags);
		if (!*section_name)
			continue;

		/* Read section */
		section_buf = elf_section_read(elf, i);
		snprintf(file_name_dest, MAX_STRING_SIZE, "%s.%d%s", file_name, i, section_name);
		write_buffer(file_name_dest, section_buf, size);
		elf_section_free(section_buf);

		/* Info */
		printf("\t  %s: inner section '%s' dumped\n", file_name_dest, section_name);
	}
}


/* Return TRUE if 'suffix' is a suffix of 'str' */
int str_suffix(char *str, char *suffix)
{
	int str_len;
	int suffix_len;

	str_len = strlen(str);
	suffix_len = strlen(suffix);
	if (str_len < suffix_len)
		return 0;
	if (strcmp(str + str_len - suffix_len, suffix))
		return 0;
	return 1;
}


void kernel_binary_analyze(char *file_name)
{
	struct elf_file_t *elf;
	struct elf_symbol_t *sym;
	int i;

	char file_name_prefix[MAX_STRING_SIZE];
	char subdir[MAX_STRING_SIZE];
	int len;

	char file_name_dest[MAX_STRING_SIZE];
	void *section_buf;
	uint32_t section_size;
	char *section_name;

	/* Get file name prefix */
	strcpy(file_name_prefix, file_name);
	len = strlen(file_name);
	if (len > 4 && !strcmp(file_name + len - 4, ".bin"))
		file_name_prefix[len - 4] = '\0';

	/* Create subdirectory */
	snprintf(subdir, sizeof subdir, "%s_files", file_name_prefix);
	mkdir(subdir, 0755);

	/* Analyze ELF file */
	elf_debug_file = fopen("/dev/null", "wt");
	elf = elf_open(file_name);
	if (!elf)
		fatal("%s: cannot open ELF file", file_name);
	
	/* List ELF sections */
	printf("ELF sections:\n");
	for (i = 0; i < elf_section_count(elf); i++)
	{
		uint32_t addr, offset, size, flags;
		elf_section_info(elf, i, &section_name, &addr, &offset, &size, &flags);
		if (!size)
			continue;

		/* Dump to file */
		section_buf = elf_section_read(elf, i);
		sprintf(file_name_dest, "%s/%s.%s", subdir, file_name_prefix,
			*section_name == '.' ? section_name + 1 : section_name);
		write_buffer(file_name_dest, section_buf, size);
		elf_section_free(section_buf);

		/* Info */
		printf("  section '%s': addr=0x%x, offset=0x%x, size=%d, flags=0x%x\n",
			section_name, addr, offset, size, flags);
	}
	
	/* Get symbols */
	for (i = 0; i < elf->symtab_count; i++)
	{
		char kernel_func_name[MAX_STRING_SIZE];
		int kernel_func_len;
		int sym_len;

		sym = &elf->symtab[i];
		if (strncmp(sym->name, "__OpenCL_", 9))
			continue;
		sym_len = strlen(sym->name);
		if (!sym->size)
			continue;

		/* Read section */
		elf_section_info(elf, sym->section, &section_name, NULL, NULL, &section_size, NULL);
		section_buf = elf_section_read(elf, sym->section);
		assert(sym->value + sym->size <= section_size);

		/* Dump to files */
		if (str_suffix(sym->name, "_metadata"))
		{
			kernel_func_len = sym_len - 18;
			strncpy(kernel_func_name, sym->name + 9, kernel_func_len);
			kernel_func_name[kernel_func_len] = '\0';

			sprintf(file_name_dest, "%s/%s.%s.metadata", subdir, file_name_prefix, kernel_func_name);
			write_buffer(file_name_dest, section_buf + sym->value, sym->size);
			printf("\t%s: meta data dumped\n", file_name_dest);

		}
		else if (str_suffix(sym->name, "_kernel"))
		{
			kernel_func_len = sym_len - 16;
			strncpy(kernel_func_name, sym->name + 9, kernel_func_len);
			kernel_func_name[kernel_func_len] = '\0';

			sprintf(file_name_dest, "%s/%s.%s.kernel", subdir, file_name_prefix, kernel_func_name);
			write_buffer(file_name_dest, section_buf + sym->value, sym->size);
			printf("\t%s: inner ELF file dumped\n", file_name_dest);
			
			kernel_binary_analyze_inner_elf(file_name_dest);
		}

		/* Free section */
		elf_section_free(section_buf);
	}

	/* Close file */
	elf_close(elf);
}


void main_compile_kernel()
{
	char device_name[MAX_STRING_SIZE];

	char *kernel_file_ext = ".cl";
	int len, extlen;

	char *program_source;
	size_t program_source_size;

	char *compiler_flags;

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
	program_source = read_buffer(kernel_file_name, &program_source_size);
	if (!program_source)
		fatal("%s: cannot open kernel\n", kernel_file_name);
	
	/* Create program */
	cl_program program;
	program = clCreateProgramWithSource(context, 1, (const char **) &program_source, &program_source_size, &err);
	if (err != CL_SUCCESS)
		fatal("clCreateProgramWithSource failed");
	
	/* Compiler flags */
	compiler_flags = NULL;
	if (debug_info)
		compiler_flags = "-O0 -g";

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
	assert(bin_sizes_ret / sizeof(size_t) == num_devices);
	for (device_id = 0; device_id < num_devices; device_id++)
		if (bin_sizes[device_id])
			break;
	if (device_id == num_devices)
		fatal("no binary generated");

	/* Dump binary into file */
	memset(bin_bits, 0, sizeof(bin_bits));
	bin_bits[device_id] = malloc(bin_sizes[device_id]);
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
	if (!*endptr) {
		if (device_id >= num_devices)
			fatal("%d is not a valid device ID; use '-l' option for a list of valid IDs",
				device_id);
		return device_id;
	}

	/* 'device_str' is a string */
	for (i = 0; i < num_devices; i++) {
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
	if (argc == 1) {
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
	if (argc - optind > 1) {
		fprintf(stderr, syntax, argv[0]);
		return 1;
	} else if (argc - optind == 1)
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
	if (device_str) {
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

