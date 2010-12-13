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
	char *device_name = "Multi2Sim Virtual GPU Device";
	char *device_vendor = "www.multi2sim.org";
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
	opencl_object_add(command_queue);
	return command_queue;
}


/* Free command queue */
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue)
{
	opencl_object_remove(command_queue);
	free(command_queue);
}


/* Command queue properties */
void opencl_command_queue_read_properties(struct opencl_command_queue_t *command_queue, struct mem_t *mem, uint32_t addr)
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

		default:
			fatal("opencl_command_queue_read_properties: invalid property (0x%x)\n", property);
		}
	}
}




/* OpenCL Program */

struct opencl_program_t *opencl_program_create()
{
	struct opencl_program_t *program;

	program = calloc(1, sizeof(struct opencl_program_t));
	program->id = opencl_object_new_id(OPENCL_OBJ_PROGRAM);
	opencl_object_add(program);
	return program;
}


void opencl_program_free(struct opencl_program_t *program)
{
	if (program->binary)
		free(program->binary);
	if (program->code)
		free(program->code);
	opencl_object_remove(program);
	free(program);
}


void opencl_program_build(struct opencl_program_t *program)
{
	FILE *program_binary_file;
	char program_binary_file_name[MAX_PATH_SIZE];
	struct elf_file_t *program_binary_elf;

	FILE *program_isa_file = NULL;
	char program_isa_file_name[MAX_PATH_SIZE];
	struct elf_file_t *program_isa_elf;

	int section_count;
	int text_section_count;
	char *section_name;
	void *section_contents;
	uint32_t section_size;
	int i;

	/* Dump binary in a temporary file*/
	program_binary_file = create_temp_file(program_binary_file_name, MAX_PATH_SIZE);
	fwrite(program->binary, 1, program->binary_size, program_binary_file);
	fseek(program_binary_file, 0, SEEK_SET);
	opencl_debug("    program binary dumped in '%s'\n", program_binary_file_name);

	/* Analyze primary ELF - dump '.text' section */
	program_binary_elf = elf_open(program_binary_file_name);
	section_count = elf_section_count(program_binary_elf);
	for (i = 0; i < section_count; i++) {
		elf_section_info(program_binary_elf, i, &section_name, NULL,
			&section_size, NULL);
		if (strcmp(section_name, ".text"))
			continue;
		if (program_isa_file)
			fatal("%s: more than one '.text' section in binary file", __FUNCTION__);
		section_contents = elf_section_read(program_binary_elf, i);
		program_isa_file = create_temp_file(program_isa_file_name, MAX_PATH_SIZE);
		fwrite(section_contents, 1, section_size, program_isa_file);
		fseek(program_isa_file, 0, SEEK_SET);
		opencl_debug("    first '.text' section (ISA ELF file) dumped in '%s'\n",
			program_isa_file_name);
		elf_section_free(section_contents);
	}
	if (!program_isa_file)
		fatal("%s: no '.text' section found in binary", __FUNCTION__);
	
	/* Analyze secondary ELF - get second '.text' section into 'program->code' */
	program_isa_elf = elf_open(program_isa_file_name);
	section_count = elf_section_count(program_isa_elf);
	text_section_count = 0;
	for (i = 0; i < section_count; i++) {
		elf_section_info(program_isa_elf, i, &section_name, NULL,
			&section_size, NULL);
		if (strcmp(section_name, ".text"))
			continue;
		text_section_count++;
		if (text_section_count < 2)
			continue;
		if (text_section_count > 2)
			fatal("%s: more than two '.text' sections in ISA file", __FUNCTION__);
		section_contents = elf_section_read(program_isa_elf, i);
		if (program->code)
			fatal("%s: program code has already been read", __FUNCTION__);
		program->code = malloc(section_size);
		program->code_size = section_size;
		memcpy(program->code, section_contents, section_size);
		elf_section_free(section_contents);
	}
	if (!program->code)
		fatal("%s: second '.text' section not found in ISA file", __FUNCTION__);

	/* Close and remove temporary files */
	elf_close(program_binary_elf);
	elf_close(program_isa_elf);
	fclose(program_binary_file);
	fclose(program_isa_file);
	remove(program_binary_file_name);
	remove(program_isa_file_name);
}




/* OpenCL Kernel */

struct opencl_kernel_t *opencl_kernel_create()
{
	struct opencl_kernel_t *kernel;

	kernel = calloc(1, sizeof(struct opencl_kernel_t));
	kernel->id = opencl_object_new_id(OPENCL_OBJ_KERNEL);
	kernel->arg_list = list_create(10);
	opencl_object_add(kernel);
	return kernel;
}


void opencl_kernel_free(struct opencl_kernel_t *kernel)
{
	int i;

	/* Free arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
		opencl_kernel_arg_free(kernel, i);
	list_free(kernel->arg_list);

	/* Free kernel */
	opencl_object_remove(kernel);
	free(kernel);
}


void opencl_kernel_arg_free(struct opencl_kernel_t *kernel, int idx)
{
	void *buf;

	/* Get argument */
	buf = list_get(kernel->arg_list, idx);
	if (!buf)
		return;
	
	/* Free it if it existed */
	free(buf);
	list_set(kernel->arg_list, idx, NULL);
	opencl_debug("    argument %d freed in kernel 0x%x\n", idx, kernel->id);
}


/* Update argument 'idx' for kernel. The contents of 'buf' are replicated in a new memory
 * area managed by the kernel. If the argument was not empty, the previous 'buf' is freed. */
void opencl_kernel_arg_set(struct opencl_kernel_t *kernel, int idx, void *buf, int size)
{
	void *buf_copy;

	/* Replicate buffer */
	buf_copy = malloc(size);
	if (!buf_copy)
		fatal("opencl_kernel_arg_set: out of memory");
	memcpy(buf_copy, buf, size);
	
	/* Add argument */
	opencl_kernel_arg_free(kernel, idx);
	while (list_count(kernel->arg_list) <= idx)
		list_add(kernel->arg_list, NULL);
	list_set(kernel->arg_list, idx, buf_copy);
	assert(!list_error(kernel->arg_list));
	opencl_debug("    argument %d set (%d bytes) in kernel 0x%x\n", idx, size, kernel->id);
}




/* OpenCL Mem */

struct opencl_mem_t *opencl_mem_create()
{
	struct opencl_mem_t *mem;

	mem = calloc(1, sizeof(struct opencl_mem_t));
	mem->id = opencl_object_new_id(OPENCL_OBJ_MEM);
	opencl_object_add(mem);
	return mem;
}


void opencl_mem_free(struct opencl_mem_t *mem)
{
	opencl_object_remove(mem);
	free(mem);
}

