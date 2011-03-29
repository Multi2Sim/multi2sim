/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel-local.h>
#include <assert.h>
#include <debug.h>
#include <stdlib.h>
#include <lnlist.h>


/* OpenCL Objects */

struct lnlist_t *opencl_object_list;


/* Add an OpenCL object to object list */
void opencl_object_add(void *object)
{
	lnlist_find(opencl_object_list, object);
	assert(lnlist_error(opencl_object_list));
	lnlist_add(opencl_object_list, object);
}


/* Remove an OpenCL object from object list */
void opencl_object_remove(void *object)
{
	lnlist_find(opencl_object_list, object);
	assert(!lnlist_error(opencl_object_list));
	lnlist_remove(opencl_object_list);
}


/* Look for an OpenCL object in the object list. The 'id' is the
 * first field for every object. */
void *opencl_object_get(enum opencl_obj_enum type, uint32_t id)
{
	void *object;
	uint32_t object_id;

	if (id >> 16 != type)
		fatal("opencl_object_get: requested OpenCL object of incorrect type");
	for (lnlist_head(opencl_object_list); !lnlist_eol(opencl_object_list); lnlist_next(opencl_object_list)) {
		if (!(object = lnlist_get(opencl_object_list)))
			panic("opencl_object_get: empty object");
		object_id = * (uint32_t *) object;
		if (object_id == id)
			return object;
	}
	fatal("opencl_object_get: requested OpenCL does not exist (id=0x%x)", id);
	return NULL;
}


/* Get the oldest created OpenCL object of the specified type */
void *opencl_object_get_type(enum opencl_obj_enum type)
{
	void *object;
	uint32_t object_id;

	/* Find object */
	for (lnlist_head(opencl_object_list); !lnlist_eol(opencl_object_list); lnlist_next(opencl_object_list)) {
		if (!(object = lnlist_get(opencl_object_list)))
			panic("opencl_object_get_type: empty object");
		object_id = * (uint32_t *) object;
		if (object_id >> 16 == type)
			return object;

	}

	/* No object found */
	return NULL;
}


/* Assignment of OpenCL object identifiers
 * An identifier is a 32-bit value, whose 16 most significant bits represent the
 * object type, while the 16 least significant bits represent a unique object ID. */
uint32_t opencl_object_new_id(enum opencl_obj_enum type)
{
	static uint32_t opencl_current_object_id;
	uint32_t id;

	id = (type << 16) | opencl_current_object_id;
	opencl_current_object_id++;
	if (opencl_current_object_id > 0xffff)
		fatal("opencl_object_new_id: too many OpenCL objects");
	return id;
}


/* Free all OpenCL objects in the object list */

void opencl_object_free_all()
{
	void *object;

	/* Platforms */
	while ((object = opencl_object_get_type(OPENCL_OBJ_PLATFORM)))
		opencl_platform_free((struct opencl_platform_t *) object);
	
	/* Devices */
	while ((object = opencl_object_get_type(OPENCL_OBJ_DEVICE)))
		opencl_device_free((struct opencl_device_t *) object);
	
	/* Contexts */
	while ((object = opencl_object_get_type(OPENCL_OBJ_CONTEXT)))
		opencl_context_free((struct opencl_context_t *) object);
	
	/* Command queues */
	while ((object = opencl_object_get_type(OPENCL_OBJ_COMMAND_QUEUE)))
		opencl_command_queue_free((struct opencl_command_queue_t *) object);
	
	/* Programs */
	while ((object = opencl_object_get_type(OPENCL_OBJ_PROGRAM)))
		opencl_program_free((struct opencl_program_t *) object);
	
	/* Kernels */
	while ((object = opencl_object_get_type(OPENCL_OBJ_KERNEL)))
		opencl_kernel_free((struct opencl_kernel_t *) object);
	
	/* Mems */
	while ((object = opencl_object_get_type(OPENCL_OBJ_MEM)))
		opencl_mem_free((struct opencl_mem_t *) object);
	
	/* Events */
	while ((object = opencl_object_get_type(OPENCL_OBJ_EVENT)))
		opencl_event_free((struct opencl_event_t *) object);
	
	/* Any object left */
	if (lnlist_count(opencl_object_list))
		panic("opencl_object_free_all: objects remaining in the list");
	
}




/* OpenCL Platform */

struct opencl_platform_t *opencl_platform;


struct opencl_platform_t *opencl_platform_create()
{
	struct opencl_platform_t *platform;

	platform = calloc(1, sizeof(struct opencl_platform_t));
	platform->id = opencl_object_new_id(OPENCL_OBJ_PLATFORM);
	opencl_object_add(platform);
	return platform;
}


void opencl_platform_free(struct opencl_platform_t *platform)
{
	opencl_object_remove(platform);
	free(platform);
}


uint32_t opencl_platform_get_info(struct opencl_platform_t *platform, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
{
	char *platform_profile = "FULL_PROFILE";
	char *platform_version = "OpenCL 1.1 Multi2Sim-v" VERSION;
	char *platform_name = "Multi2Sim";
	char *platform_vendor = "www.multi2sim.org";
	char *platform_extensions = "";

	uint32_t size_ret = 0;
	char *info;

	switch (name) {
	
	case 0x900:  /* CL_PLATFORM_PROFILE */
		info = platform_profile;
		break;

	case 0x901:  /* CL_PLATFORM_VERSION */
		info = platform_version;
		break;

	case 0x902:  /* CL_PLATFORM_NAME */
		info = platform_name;
		break;

	case 0x903:  /* CL_PLATFORM_VENDOR */
		info = platform_vendor;
		break;

	case 0x904:  /* CL_PLATFORM_EXTENSIONS */
		info = platform_extensions;
		break;

	default:
		info = NULL;
		fatal("opencl_platform_get_info: invalid value for 'name' (0x%x)\n%s",
			name, err_opencl_param_note);
	}

	/* Write to memory and return size */
	assert(info);
	size_ret = strlen(info) + 1;
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}




/* OpenCL Device */


/* Create a device */
struct opencl_device_t *opencl_device_create()
{
	struct opencl_device_t *device;

	device = calloc(1, sizeof(struct opencl_device_t));
	device->id = opencl_object_new_id(OPENCL_OBJ_DEVICE);
	opencl_object_add(device);
	return device;
}


/* Free device */
void opencl_device_free(struct opencl_device_t *device)
{
	opencl_object_remove(device);
	free(device);
}


uint32_t opencl_device_get_info(struct opencl_device_t *device, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t max_compute_units = 1;  /* FIXME */
	uint32_t max_work_group_size = 256 * 256;  /* FIXME */
	uint32_t max_work_item_dimensions = 3;  /* FIXME */
	uint32_t max_work_item_sizes[3];  /* FIXME */
	uint32_t local_mem_type = 1;  /* CL_LOCAL FIXME */
	uint32_t local_mem_size = 32 * 1024;  /* FIXME */
	uint32_t image_support = 1;
	char *device_name = "Multi2Sim Virtual GPU Device";
	char *device_vendor = "www.multi2sim.org";
	char *device_extensions = "cl_amd_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics "
		"cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store "
		"cl_khr_gl_sharing cl_ext_device_fission cl_amd_device_attribute_query cl_amd_media_ops cl_amd_popcnt "
		"cl_amd_printf ";
	char *device_version = "OpenCL 1.1 ATI-Stream-v2.3 (451)";
	char *driver_version = VERSION;

	uint32_t size_ret = 0;
	void *info = NULL;

	switch (name) {

	case 0x1002:  /* CL_DEVICE_MAX_COMPUTE_UNITS */
		size_ret = 4;
		info = &max_compute_units;
		break;

	case 0x1003:  /* CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS */
		size_ret = 4;
		info = &max_work_item_dimensions;
		break;

	case 0x1004:  /* CL_DEVICE_MAX_WORK_GROUP_SIZE */
		size_ret = 4;
		info = &max_work_group_size;
		break;

	case 0x1005:  /* CL_DEVICE_MAX_WORK_ITEM_SIZES */
		max_work_item_sizes[0] = 256;
		max_work_item_sizes[1] = 256;
		max_work_item_sizes[2] = 256;
		size_ret = 12;
		info = max_work_item_sizes;
		break;
	
	case 0x1016:  /* CL_DEVICE_IMAGE_SUPPORT */
		size_ret = 4;
		info = &image_support;
		break;

	case 0x1022:  /* CL_DEVICE_LOCAL_MEM_TYPE */
		size_ret = 4;
		info = &local_mem_type;
		break;

	case 0x1023:  /* CL_DEVICE_LOCAL_MEM_SIZE */
		size_ret = 4;
		info = &local_mem_size;
		break;

	case 0x102b:  /* CL_DEVICE_NAME */
		size_ret = strlen(device_name) + 1;
		info = device_name;
		break;
	
	case 0x102c:  /* CL_DEVICE_VENDOR */
		size_ret = strlen(device_vendor) + 1;
		info = device_vendor;
		break;
	
	case 0x102d:  /* CL_DRIVER_VERSION */
		size_ret = strlen(driver_version) + 1;
		info = driver_version;
		break;
	
	case 0x102f:  /* CL_DEVICE_VERSION */
		size_ret = strlen(device_version) + 1;
		info = device_version;
		break;
	
	case 0x1030:  /* CL_DEVICE_EXTENSIONS */
		size_ret = strlen(device_extensions) + 1;
		info = device_extensions;
		break;

	default:
		fatal("opencl_device_get_info: invalid or not implemented value for 'name' (0x%x)\n%s",
			name, err_opencl_note);

	}

	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}




/* OpenCL Context */


/* Create a context */
struct opencl_context_t *opencl_context_create()
{
	struct opencl_context_t *context;

	context = calloc(1, sizeof(struct opencl_context_t));
	context->id = opencl_object_new_id(OPENCL_OBJ_CONTEXT);
	context->ref_count = 1;
	opencl_object_add(context);
	return context;
}


/* Free context */
void opencl_context_free(struct opencl_context_t *context)
{
	opencl_object_remove(context);
	free(context);
}


uint32_t opencl_context_get_info(struct opencl_context_t *context, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t num_devices = 1;

	uint32_t size_ret = 0;
	void *info = NULL;

	switch (name) {

	case 0x1081:  /* CL_CONTEXT_DEVICES */
		size_ret = 4;
		info = &context->device_id;
		break;

	case 0x1083:  /* CL_CONTEXT_NUM_DEVICES */
		size_ret = 4;
		info = &num_devices;
		break;

	case 0x1080:  /* CL_CONTEXT_REFERENCE_COUNT */
	case 0x1082:  /* CL_CONTEXT_PROPERTIES */
	default:
		fatal("opencl_context_get_info: invalid or not implemented value for 'name' (0x%x)\n%s",
			name, err_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}


/* Read context properties from a null-terminated sequence of
 * 'cl_context_properties' elements in guest memory */
void opencl_context_set_properties(struct opencl_context_t *context, struct mem_t *mem, uint32_t addr)
{
	uint32_t property;
	uint32_t value;

	while (addr) {

		/* Read property */
		mem_read(isa_mem, addr, 4, &property);
		if (!property)
			break;
		mem_read(isa_mem, addr + 4, 4, &value);
		addr += 8;

		/* Analyze property */
		switch (property) {

		case 0x1084:  /* CL_CONTEXT_PLATFORM */
			context->platform_id = value;
			opencl_object_get(OPENCL_OBJ_PLATFORM, value);
			opencl_debug("    property CL_CONTEXT_PLATFORM assigned: 0x%x\n", value);
			break;

		default:
			fatal("opencl_context_read_properties: invalid property (0x%x)\n", property);
		}
	}
}




/* OpenCL Command Queue */

/* Create a command queue */
struct opencl_command_queue_t *opencl_command_queue_create()
{
	struct opencl_command_queue_t *command_queue;

	command_queue = calloc(1, sizeof(struct opencl_command_queue_t));
	command_queue->id = opencl_object_new_id(OPENCL_OBJ_COMMAND_QUEUE);
	command_queue->ref_count = 1;
	opencl_object_add(command_queue);
	return command_queue;
}


/* Free command queue */
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue)
{
	opencl_object_remove(command_queue);
	free(command_queue);
}




/* OpenCL Program */

struct opencl_program_t *opencl_program_create()
{
	struct opencl_program_t *program;

	program = calloc(1, sizeof(struct opencl_program_t));
	program->id = opencl_object_new_id(OPENCL_OBJ_PROGRAM);
	program->ref_count = 1;
	opencl_object_add(program);
	return program;
}


void opencl_program_free(struct opencl_program_t *program)
{
	if (program->binary_file_elf)
		elf_close(program->binary_file_elf);
	if (program->binary_file) {
		fclose(program->binary_file);
		unlink(program->binary_file_name);
	}
	opencl_object_remove(program);
	free(program);
}

char *err_opencl_evergreen_format =
	"\tYour application tried to load a pre-compiled OpenCL kernel binary which\n"
	"\tdoes not contain code in the Evergreen ISA. Please, check that the off-line\n"
	"\tcompilation of your kernel targets this GPU architecture supported by\n"
	"\tMulti2Sim.\n";


void opencl_program_build(struct opencl_program_t *program)
{
	/* Open ELF file and check that it corresponds to an Evergreen pre-compiled kernel */
	program->binary_file_elf = elf_open(program->binary_file_name);
	if (program->binary_file_elf->ehdr.e_machine != 0x3f1)
		fatal("%s: invalid binary file.\n%s", __FUNCTION__,
			err_opencl_evergreen_format);
}


/* Look for a symbol name in program binary and read it from its corresponding section.
 * The contents are dumped in a temporary file, whose name is written in 'file_name'.
 * This file is opened and its file descriptor is returned as the function result. */

char *err_opencl_elf_symbol =
	"\tThe ELF file analyzer is trying to find a name in the ELF symbol table.\n"
	"\tIf it is not found, it probably means that your application is requesting\n"
	"\texecution of a kernel function that is not present in the encoded binary.\n"
	"\tPlease, check the parameters parsed to the 'clCreateKernel' function in\n"
	"\tyour application.\n";

FILE *opencl_program_read_symbol(struct opencl_program_t *program, char *symbol_name,
	char *file_name, int file_name_size)
{
	struct elf_symbol_t *symbol;
	void *buf;
	FILE *f;
	
	/* Look for symbol */
	assert(program->binary_file_elf);
	symbol = elf_get_symbol_by_name(program->binary_file_elf, symbol_name);
	if (!symbol)
		fatal("%s: ELF symbol '%s' not found.\n%s", __FUNCTION__,
			symbol_name, err_opencl_elf_symbol);
	
	/* Store in file */
	f = create_temp_file(file_name, file_name_size);
	buf = elf_section_read_offset(program->binary_file_elf, symbol->section,
		symbol->value, symbol->size);
	write_buffer(file_name, buf, symbol->size);
	elf_free_buffer(buf);

	/* Return temporary file descriptor */
	return f;
}




/* OpenCL Kernel */

struct opencl_kernel_t *opencl_kernel_create()
{
	struct opencl_kernel_t *kernel;

	kernel = calloc(1, sizeof(struct opencl_kernel_t));
	kernel->id = opencl_object_new_id(OPENCL_OBJ_KERNEL);
	kernel->ref_count = 1;
	kernel->arg_list = list_create(10);
	opencl_object_add(kernel);
	return kernel;
}


void opencl_kernel_free(struct opencl_kernel_t *kernel)
{
	int i;

	/* Free arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
		opencl_kernel_arg_free((struct opencl_kernel_arg_t *) list_get(kernel->arg_list, i));
	list_free(kernel->arg_list);

	/* CAL ABI */
	if (kernel->cal_abi)
		cal_abi_free(kernel->cal_abi);

	/* Program excerpts */
	if (kernel->metadata_file) {
		fclose(kernel->metadata_file);
		unlink(kernel->metadata_file_name);
	}
	if (kernel->kernel_file) {
		fclose(kernel->kernel_file);
		unlink(kernel->kernel_file_name);
	}
	if (kernel->func_file) {
		fclose(kernel->func_file);
		unlink(kernel->func_file_name);
	}

	/* Free kernel */
	opencl_object_remove(kernel);
	free(kernel);
}


struct opencl_kernel_arg_t *opencl_kernel_arg_create(char *name)
{
	struct opencl_kernel_arg_t *arg;
	arg = calloc(1, sizeof(struct opencl_kernel_arg_t) + strlen(name) + 1);
	strcpy(arg->name, name);
	return arg;
}


void opencl_kernel_arg_free(struct opencl_kernel_arg_t *arg)
{
	free(arg);
}


/* Analyze 'metadata' associated with kernel */

#define OPENCL_KERNEL_METADATA_TOKEN_COUNT(_tc) \
	if (token_count != (_tc)) \
	fatal("%s: meta data entry '%s' expects %d tokens", \
	__FUNCTION__, line_ptrs[0], (_tc));
#define OPENCL_KERNEL_METADATA_NOT_SUPPORTED(_idx) \
	fatal("%s: meta data entry '%s', token %d: value '%s' not supported.\n%s", \
	__FUNCTION__, line_ptrs[0], (_idx), line_ptrs[(_idx)], \
	err_opencl_kernel_metadata_note);
#define OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(_idx, _str) \
	if (strcmp(line_ptrs[(_idx)], (_str))) \
	OPENCL_KERNEL_METADATA_NOT_SUPPORTED(_idx);

char *err_opencl_kernel_metadata_note =
	"\tThe kernel binary loaded by your application is a valid ELF file. In this\n"
	"\tfile, a '.rodata' section contains specific information about the OpenCL\n"
	"\tkernel. However, this information is only partially supported by Multi2Sim.\n"
	"\tTo request support for this error, please email 'development@multi2sim.org'.\n";

void opencl_kernel_load_metadata(struct opencl_kernel_t *kernel)
{
	FILE *f;
	char line[MAX_STRING_SIZE];
	char *line_ptrs[MAX_STRING_SIZE];
	int token_count;
	struct opencl_kernel_arg_t *arg;
	
	/* Open as text file */
	f = fopen(kernel->metadata_file_name, "rt");
	for (;;) {
		
		/* Read line from file */
		line_ptrs[0] = fgets(line, MAX_STRING_SIZE, f);
		if (!line_ptrs[0])
			break;

		/* Split line */
		line_ptrs[0] = strtok(line_ptrs[0], ":;\n");
		for (token_count = 1; (line_ptrs[token_count] = strtok(NULL, ":\n")); token_count++);

		/* Ignored entries */
		if (!line_ptrs[0] ||
			!strcmp(line_ptrs[0], "ARGSTART") ||
			!strcmp(line_ptrs[0], "version") ||
			!strcmp(line_ptrs[0], "device") ||
			!strcmp(line_ptrs[0], "uniqueid") ||
			!strcmp(line_ptrs[0], "uavid") ||
			!strcmp(line_ptrs[0], "ARGEND"))
			continue;

		/* Memory */
		if (!strcmp(line_ptrs[0], "memory")) {
			OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			if (!strcmp(line_ptrs[1], "hwprivate")) {
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2, "0");
			} else if (!strcmp(line_ptrs[1], "hwlocal")) {
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2, "0");
			} else
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED(1);
			continue;
		}

		/* Entry 'value'. Format: value:<name>:<type>:?:?:<addr> */
		if (!strcmp(line_ptrs[0], "value")) {
			OPENCL_KERNEL_METADATA_TOKEN_COUNT(6);
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(3, "1");
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");
			arg = opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = OPENCL_KERNEL_ARG_KIND_VALUE;
			list_add(kernel->arg_list, arg);
			opencl_debug("    arg %d: '%s', value of type %s\n",
				list_count(kernel->arg_list) - 1, arg->name, line_ptrs[2]);
			continue;
		}

		/* Entry 'pointer'. Format: pointer:<name>:<type>:?:?:<addr>:?:?:<elem_size> */
		if (!strcmp(line_ptrs[0], "pointer")) {
			OPENCL_KERNEL_METADATA_TOKEN_COUNT(9);
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(3, "1");
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(7, "1");
			arg = opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = OPENCL_KERNEL_ARG_KIND_POINTER;
			arg->elem_size = atoi(line_ptrs[8]);
			list_add(kernel->arg_list, arg);
			opencl_debug("    arg %d: '%s', pointer to %s values (%d-byte group) in ",
				list_count(kernel->arg_list) - 1, arg->name, line_ptrs[2],
				arg->elem_size);
			if (!strcmp(line_ptrs[6], "uav")) {
				arg->mem_scope = OPENCL_MEM_SCOPE_GLOBAL;
				opencl_debug("global");
			} else if (!strcmp(line_ptrs[6], "hl")) {
				arg->mem_scope = OPENCL_MEM_SCOPE_LOCAL;
				opencl_debug("local");
			} else
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED(6);
			opencl_debug(" memory\n");
			continue;
		}

		/* Entry 'function'. Format: function:?:<uniqueid> */
		if (!strcmp(line_ptrs[0], "function")) {
			OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(1, "1");
			kernel->func_uniqueid = atoi(line_ptrs[2]);
			continue;
		}

		/* Warn about uninterpreted entries */
		warning("kernel '%s': unknown meta data entry '%s'",
			kernel->name, line_ptrs[0]);
	}

	/* Close */
	fclose(f);
}


void opencl_kernel_load_func_metadata(struct opencl_kernel_t *kernel)
{
	FILE *f;
	char line[MAX_STRING_SIZE];
	char *line_ptrs[MAX_STRING_SIZE];
	int token_count;
	
	/* Open as text file */
	f = fopen(kernel->func_file_name, "rt");
	if (!f)
		fatal("%s: unable to open metadata file", __FUNCTION__);
	for (;;) {
		
		/* Read line from file */
		line_ptrs[0] = fgets(line, MAX_STRING_SIZE, f);
		if (!line_ptrs[0])
			break;

		/* Split line */
		line_ptrs[0] = strtok(line_ptrs[0], ":;\n");
		for (token_count = 1; (line_ptrs[token_count] = strtok(NULL, ":\n")); token_count++);

		/* Ignored entries */
		if (!line_ptrs[0] ||
			!strcmp(line_ptrs[0], "ARGSTART") ||
			!strcmp(line_ptrs[0], "uniqueid") ||
			!strcmp(line_ptrs[0], "ARGEND"))
			continue;

		/* Memory */
		if (!strcmp(line_ptrs[0], "memory")) {
			OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			if (!strcmp(line_ptrs[1], "hwlocal")) {
				kernel->func_mem_local = atoi(line_ptrs[2]);
				kernel->local_mem_top = kernel->func_mem_local;
				opencl_debug("kernel '%s' using %d bytes local memory\n",
					kernel->name, kernel->func_mem_local);
			} else
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED(1);
			continue;
		}

		/* Warn about uninterpreted entries */
		warning("kernel '%s': unknown function meta data entry '%s'",
			kernel->name, line_ptrs[0]);
	}

	/* Close */
	fclose(f);
}


/* Extract and analyze information from the program binary associated with 'kernel_name' */
void opencl_kernel_load(struct opencl_kernel_t *kernel, char *kernel_name)
{
	struct opencl_program_t *program;
	char symbol_name[MAX_STRING_SIZE];

	/* First */
	strncpy(kernel->name, kernel_name, MAX_STRING_SIZE);
	program = opencl_object_get(OPENCL_OBJ_PROGRAM, kernel->program_id);

	/* Read 'metadata' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_metadata", kernel_name);
	kernel->metadata_file = opencl_program_read_symbol(program, symbol_name,
		kernel->metadata_file_name, MAX_PATH_SIZE);
	
	/* Read 'kernel' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_kernel", kernel_name);
	kernel->kernel_file = opencl_program_read_symbol(program, symbol_name,
		kernel->kernel_file_name, MAX_PATH_SIZE);
	
	/* Check that symbols were found */
	if (!kernel->metadata_file || !kernel->kernel_file)
		fatal("%s: kernel '%s' not found in binary.\n%s", __FUNCTION__,
			kernel_name, err_opencl_param_note);
	
	/* Create 'cal_abi' object and parse kernel ELF */
	kernel->cal_abi = cal_abi_create();
	cal_abi_parse_elf(kernel->cal_abi, kernel->kernel_file_name);
	
	/* Analyze 'metadata' file */
	opencl_kernel_load_metadata(kernel);

	/* Read function 'XXX_fmetadata' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%d_fmetadata", kernel->func_uniqueid);
	kernel->func_file = opencl_program_read_symbol(program, symbol_name,
		kernel->func_file_name, MAX_PATH_SIZE);
	
	/* Load function metadata */
	opencl_kernel_load_func_metadata(kernel);
}


uint32_t opencl_kernel_get_work_group_info(struct opencl_kernel_t *kernel, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t size_ret = 0;
	void *info = NULL;

	uint64_t local_mem_size = 0;
	uint32_t work_group_size = 256;

	switch (name) {

	case 0x11b0:  /* CL_KERNEL_WORK_GROUP_SIZE */
		warning("%s: CL_KERNEL_WORK_GROUP_SIZE: %d is returned, but this should be obtained\n"
			"\tfrom the kernel register usage and the device info.",
			__FUNCTION__, work_group_size);
		info = &work_group_size;
		size_ret = 4;
		break;

	case 0x11b2:  /* CL_KERNEL_LOCAL_MEM_SIZE */
	{
		int i;
		struct opencl_kernel_arg_t *arg;

		/* Compute local memory usage */
		local_mem_size = kernel->func_mem_local;
		for (i = 0; i < list_count(kernel->arg_list); i++) {
			arg = list_get(kernel->arg_list, i);
			if (arg->mem_scope == OPENCL_MEM_SCOPE_LOCAL)
				local_mem_size += arg->size;
		}

		/* Return it */
		info = &local_mem_size;
		size_ret = 8;
		break;
	}
	
	case 0x11b1:  /* CL_KERNEL_COMPILE_WORK_GROUP_SIZE */
	case 0x11b3:  /* CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE */
	case 0x11b4:  /* CL_KERNEL_PRIVATE_MEM_SIZE */
	default:
		fatal("%s: invalid or not implemented value for 'name' (0x%x)\n%s",
			__FUNCTION__, name, err_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}




/* OpenCL Mem */

struct opencl_mem_t *opencl_mem_create()
{
	struct opencl_mem_t *mem;

	mem = calloc(1, sizeof(struct opencl_mem_t));
	mem->id = opencl_object_new_id(OPENCL_OBJ_MEM);
	mem->ref_count = 1;
	opencl_object_add(mem);
	return mem;
}


void opencl_mem_free(struct opencl_mem_t *mem)
{
	opencl_object_remove(mem);
	free(mem);
}




/* OpenCL Event */

struct opencl_event_t *opencl_event_create(enum opencl_event_kind_enum kind)
{
	struct opencl_event_t *event;

	event = calloc(1, sizeof(struct opencl_event_t));
	event->id = opencl_object_new_id(OPENCL_OBJ_EVENT);
	event->ref_count = 1;
	event->kind = kind;
	opencl_object_add(event);
	return event;
}


void opencl_event_free(struct opencl_event_t *event)
{
	opencl_object_remove(event);
	free(event);
}


uint32_t opencl_event_get_profiling_info(struct opencl_event_t *event, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t size_ret = 0;
	void *info = NULL;

	switch (name) {

	case 0x1280:  /* CL_PROFILING_COMMAND_QUEUED */
		size_ret = 8;
		info = &event->time_queued;
		break;
	
	case 0x1281:  /* CL_PROFILING_COMMAND_SUBMIT */
		size_ret = 8;
		info = &event->time_submit;
		break;
	
	case 0x1282:  /* CL_PROFILING_COMMAND_START */
		size_ret = 8;
		info = &event->time_start;
		break;
	
	case 0x1283:  /* CL_PROFILING_COMMAND_END */
		size_ret = 8;
		info = &event->time_end;
		break;

	default:
		fatal("%s: invalid or not implemented value for 'name' (0x%x)\n%s",
			__FUNCTION__, name, err_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}


uint64_t opencl_event_timer(void)
{
	return ke_timer() * 1000;
}

