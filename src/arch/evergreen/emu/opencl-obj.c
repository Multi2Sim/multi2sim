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

#include <assert.h>
#include <debug.h>
#include <stdlib.h>

#include <evergreen-emu.h>
#include <x86-emu.h>


/* OpenCL Objects */

struct linked_list_t *evg_opencl_object_list;


/* Add an OpenCL object to object list */
void evg_opencl_object_add(void *object)
{
	linked_list_find(evg_opencl_object_list, object);
	assert(evg_opencl_object_list->error_code);
	linked_list_add(evg_opencl_object_list, object);
}


/* Remove an OpenCL object from object list */
void evg_opencl_object_remove(void *object)
{
	linked_list_find(evg_opencl_object_list, object);
	assert(!evg_opencl_object_list->error_code);
	linked_list_remove(evg_opencl_object_list);
}


/* Look for an OpenCL object in the object list. The 'id' is the
 * first field for every object. */
void *evg_opencl_object_get(enum evg_opencl_obj_t type, uint32_t id)
{
	void *object;
	uint32_t object_id;

	if (id >> 16 != type)
		fatal("%s: requested OpenCL object of incorrect type",
			__FUNCTION__);
	LINKED_LIST_FOR_EACH(evg_opencl_object_list)
	{
		if (!(object = linked_list_get(evg_opencl_object_list)))
			panic("%s: empty object", __FUNCTION__);
		object_id = * (uint32_t *) object;
		if (object_id == id)
			return object;
	}
	fatal("%s: requested OpenCL does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}


/* Get the oldest created OpenCL object of the specified type */
void *evg_opencl_object_get_type(enum evg_opencl_obj_t type)
{
	void *object;
	uint32_t object_id;

	/* Find object */
	LINKED_LIST_FOR_EACH(evg_opencl_object_list)
	{
		if (!(object = linked_list_get(evg_opencl_object_list)))
			panic("%s: empty object", __FUNCTION__);
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
uint32_t evg_opencl_object_new_id(enum evg_opencl_obj_t type)
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

void evg_opencl_object_free_all()
{
	void *object;

	/* Platforms */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_PLATFORM)))
		evg_opencl_platform_free((struct evg_opencl_platform_t *) object);
	
	/* Devices */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_DEVICE)))
		evg_opencl_device_free((struct evg_opencl_device_t *) object);
	
	/* Contexts */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_CONTEXT)))
		evg_opencl_context_free((struct evg_opencl_context_t *) object);
	
	/* Command queues */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_COMMAND_QUEUE)))
		evg_opencl_command_queue_free((struct evg_opencl_command_queue_t *) object);
	
	/* Programs */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_PROGRAM)))
		evg_opencl_program_free((struct evg_opencl_program_t *) object);
	
	/* Kernels */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_KERNEL)))
		evg_opencl_kernel_free((struct evg_opencl_kernel_t *) object);
	
	/* Mems */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_MEM)))
		evg_opencl_mem_free((struct evg_opencl_mem_t *) object);
	
	/* Events */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_EVENT)))
		evg_opencl_event_free((struct evg_opencl_event_t *) object);

	/* Samplers */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_SAMPLER)))
		evg_opencl_sampler_free((struct evg_opencl_sampler_t *) object);
	
	/* Any object left */
	if (linked_list_count(evg_opencl_object_list))
		panic("opencl_object_free_all: objects remaining in the list");
	
}




/* OpenCL Platform */

struct evg_opencl_platform_t *evg_opencl_platform;


struct evg_opencl_platform_t *evg_opencl_platform_create()
{
	struct evg_opencl_platform_t *platform;

	platform = calloc(1, sizeof(struct evg_opencl_platform_t));
	platform->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_PLATFORM);
	evg_opencl_object_add(platform);
	return platform;
}


void evg_opencl_platform_free(struct evg_opencl_platform_t *platform)
{
	evg_opencl_object_remove(platform);
	free(platform);
}


uint32_t evg_opencl_platform_get_info(struct evg_opencl_platform_t *platform, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
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
			name, err_evg_opencl_param_note);
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
struct evg_opencl_device_t *evg_opencl_device_create()
{
	struct evg_opencl_device_t *device;

	device = calloc(1, sizeof(struct evg_opencl_device_t));
	device->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_DEVICE);
	evg_opencl_object_add(device);
	return device;
}


/* Free device */
void evg_opencl_device_free(struct evg_opencl_device_t *device)
{
	evg_opencl_object_remove(device);
	free(device);
}


uint32_t evg_opencl_device_get_info(struct evg_opencl_device_t *device, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t max_compute_units = 1;  /* FIXME */
	uint32_t max_work_group_size = 256 * 256;  /* FIXME */
	uint32_t max_work_item_dimensions = 3;  /* FIXME */
	uint32_t max_work_item_sizes[3];  /* FIXME */
	uint32_t local_mem_type = 1;  /* CL_LOCAL FIXME */
	uint32_t local_mem_size = 32 * 1024;  /* FIXME */
	uint32_t max_clock_frequency = 850;
	uint64_t global_mem_size = 1ull << 31;  /* 2GB of global memory reported */
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
	
	case 0x100c:  /* CL_DEVICE_MAX_CLOCK_FREQUENCY */
		size_ret = 4;
		info = &max_clock_frequency;
		break;

	case 0x1016:  /* CL_DEVICE_IMAGE_SUPPORT */
		size_ret = 4;
		info = &image_support;
		break;

	case 0x101f:  /* CL_GLOBAL_MEM_SIZE */
		size_ret = 8;
		info = &global_mem_size;
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
			name, err_evg_opencl_note);

	}

	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}




/* OpenCL Context */


/* Create a context */
struct evg_opencl_context_t *evg_opencl_context_create()
{
	struct evg_opencl_context_t *context;

	context = calloc(1, sizeof(struct evg_opencl_context_t));
	context->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_CONTEXT);
	context->ref_count = 1;
	evg_opencl_object_add(context);
	return context;
}


/* Free context */
void evg_opencl_context_free(struct evg_opencl_context_t *context)
{
	evg_opencl_object_remove(context);
	free(context);
}


uint32_t evg_opencl_context_get_info(struct evg_opencl_context_t *context, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
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
			name, err_evg_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}


/* Read context properties from a null-terminated sequence of
 * 'cl_context_properties' elements in guest memory */
void evg_opencl_context_set_properties(struct evg_opencl_context_t *context, struct mem_t *mem, uint32_t addr)
{
	uint32_t property;
	uint32_t value;

	while (addr) {

		/* Read property */
		mem_read(x86_isa_mem, addr, 4, &property);
		if (!property)
			break;
		mem_read(x86_isa_mem, addr + 4, 4, &value);
		addr += 8;

		/* Analyze property */
		switch (property) {

		case 0x1084:  /* CL_CONTEXT_PLATFORM */
			context->platform_id = value;
			evg_opencl_object_get(EVG_OPENCL_OBJ_PLATFORM, value);
			evg_opencl_debug("    property CL_CONTEXT_PLATFORM assigned: 0x%x\n", value);
			break;

		default:
			fatal("opencl_context_read_properties: invalid property (0x%x)\n", property);
		}
	}
}



/* OpenCL Sampler */
struct evg_opencl_sampler_t *evg_opencl_sampler_create()
{
	struct evg_opencl_sampler_t *sampler;

	sampler = calloc(1, sizeof(struct evg_opencl_sampler_t));
	sampler->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_SAMPLER);
	sampler->ref_count = 1;
	evg_opencl_object_add(sampler);
	return sampler;
}

/* Free sampler */
void evg_opencl_sampler_free(struct evg_opencl_sampler_t *sampler)
{
	evg_opencl_object_remove(sampler);
	free(sampler);
}


/* OpenCL Command Queue */

/* Create a command queue */
struct evg_opencl_command_queue_t *evg_opencl_command_queue_create()
{
	struct evg_opencl_command_queue_t *command_queue;

	command_queue = calloc(1, sizeof(struct evg_opencl_command_queue_t));
	command_queue->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_COMMAND_QUEUE);
	command_queue->ref_count = 1;
	evg_opencl_object_add(command_queue);
	return command_queue;
}


/* Free command queue */
void evg_opencl_command_queue_free(struct evg_opencl_command_queue_t *command_queue)
{
	evg_opencl_object_remove(command_queue);
	free(command_queue);
}




/* OpenCL Program */

struct evg_opencl_program_t *evg_opencl_program_create()
{
	struct evg_opencl_program_t *program;
	int i;

	program = calloc(1, sizeof(struct evg_opencl_program_t));
	program->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_PROGRAM);
	program->ref_count = 1;

	/* Constant buffers encoded in ELF file */
	program->constant_buffer_list = list_create_with_size(25);
	for(i = 0; i < 25; i++) 
	{
		list_add(program->constant_buffer_list, NULL);
	}

	evg_opencl_object_add(program);
	return program;
}


void evg_opencl_program_free(struct evg_opencl_program_t *program)
{
	/* Free lists */
	list_free(program->constant_buffer_list);

	if (program->elf_file)
		elf_file_free(program->elf_file);
	evg_opencl_object_remove(program);
	free(program);
}

char *err_opencl_evergreen_format =
	"\tYour application tried to load a pre-compiled OpenCL kernel binary which\n"
	"\tdoes not contain code in the Evergreen ISA. Please, check that the off-line\n"
	"\tcompilation of your kernel targets this GPU architecture supported by\n"
	"\tMulti2Sim.\n";


void evg_opencl_program_build(struct evg_opencl_program_t *program)
{
	/* Open ELF file and check that it corresponds to an Evergreen pre-compiled kernel */
	assert(program->elf_file);
	if (program->elf_file->header->e_machine != 0x3f1)
		fatal("%s: invalid binary file.\n%s", __FUNCTION__,
			err_opencl_evergreen_format);
}


/* Look for a symbol name in program binary and read it from its corresponding section.
 * The contents pointed to by the symbol are stored in an ELF buffer.
 * No allocation happens here, the target buffer will just point to the contents of
 * an existing section. */

char *err_opencl_elf_symbol =
	"\tThe ELF file analyzer is trying to find a name in the ELF symbol table.\n"
	"\tIf it is not found, it probably means that your application is requesting\n"
	"\texecution of a kernel function that is not present in the encoded binary.\n"
	"\tPlease, check the parameters passed to the 'clCreateKernel' function in\n"
	"\tyour application.\n"
	"\tThis could be also a symptom of compiling an OpenCL kernel source on a\n"
	"\tmachine with an installation of the APP SDK (using 'm2s-opencl-kc') but\n"
	"\twith an incorrect or missing installation of the GPU driver. In this case,\n"
	"\tthe tool will still compile the kernel into LLVM, but the ISA section will\n"
	"\tbe missing in the kernel binary.\n";

void opencl_program_read_symbol(struct evg_opencl_program_t *program, char *symbol_name,
	struct elf_buffer_t *buffer)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;
	
	/* Look for symbol */
	elf_file = program->elf_file;
	assert(elf_file);
	symbol = elf_symbol_get_by_name(elf_file, symbol_name);
	if (!symbol)
		fatal("%s: ELF symbol '%s' not found.\n%s", __FUNCTION__,
			symbol_name, err_opencl_elf_symbol);
	
	/* Get section where the symbol is pointing */
	section = list_get(elf_file->section_list, symbol->section);
	assert(section);
	if (symbol->value + symbol->size > section->header->sh_size)
		fatal("%s: ELF symbol '%s' exceeds section '%s' boundaries.\n%s",
			__FUNCTION__, symbol->name, section->name, err_opencl_elf_symbol);

	/* Update buffer */
	buffer->ptr = section->buffer.ptr + symbol->value;
	buffer->size = symbol->size;
	buffer->pos = 0;
}

void evg_opencl_program_initialize_constant_buffers(struct evg_opencl_program_t *program)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *elf_symbol;
	struct elf_buffer_t elf_buffer;
	struct evg_opencl_mem_t *mem;
	char symbol_name[MAX_STRING_SIZE];
	int i;

	elf_file = program->elf_file;

	/* We can't tell how many constant buffers exist in advance, but we
	 * know they should be enumerated, starting with '2'.  This loop
	 * searches until a constant buffer matching the format is not found. */
	for (i = 2; i < 25; i++) 
	{
		/* Create string of symbol name */
		sprintf(symbol_name, "__OpenCL_%d_global", i);

		/* Check to see if symbol exists */
		elf_symbol = elf_symbol_get_by_name(elf_file, symbol_name);
                if (elf_symbol == NULL) {

			break;
		}
		evg_opencl_debug("  constant buffer '%s' found with size %d\n",
			elf_symbol->name, elf_symbol->size);

		/* Read the elf symbol into a buffer */
		opencl_program_read_symbol(program, elf_symbol->name, &elf_buffer);

		/* Create a memory object and copy the constant buffer data to it */
		mem = evg_opencl_mem_create();
		mem->type = 0;  /* FIXME */
		mem->size = elf_buffer.size;
		mem->flags = 0; /* TODO Change to CL_MEM_READ_ONLY */
		mem->host_ptr = 0;

		/* Assign position in device global memory */
		mem->device_ptr = evg_emu->global_mem_top;
		evg_emu->global_mem_top += mem->size;

		/* Copy constant buffer into device memory */
		mem_write(evg_emu->global_mem, mem->device_ptr, mem->size, elf_buffer.ptr);

		/* Add the memory object to the constant buffer list */
		list_set(program->constant_buffer_list, i, mem);
	}
} 



/* OpenCL Kernel */

struct evg_opencl_kernel_t *evg_opencl_kernel_create()
{
	struct evg_opencl_kernel_t *kernel;
	int i;

	kernel = calloc(1, sizeof(struct evg_opencl_kernel_t));
	kernel->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_KERNEL);
	kernel->ref_count = 1;
	kernel->arg_list = list_create();

	/* Create the UAV-to-physical-address lookup lists */
	kernel->uav_read_list = list_create_with_size(12); /* FIXME Repalce with MAX_UAVS? */
	kernel->uav_write_list = list_create_with_size(12); /* FIXME Repalce with MAX_UAVS? */
	kernel->constant_buffer_list = list_create_with_size(25); /* For constant buffers (128 to 153) */
	/* FIXME Replace with new list functionality */
	for (i = 0; i < 12; i++) 
	{
		list_add(kernel->uav_read_list, NULL);
		list_add(kernel->uav_write_list, NULL);
	}
	for (i = 0; i < 25; i++) 
	{
		list_add(kernel->constant_buffer_list, NULL);
	}

	evg_opencl_object_add(kernel);
	return kernel;
}


void evg_opencl_kernel_free(struct evg_opencl_kernel_t *kernel)
{
	int i;

	/* Free arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
		evg_opencl_kernel_arg_free((struct evg_opencl_kernel_arg_t *) list_get(kernel->arg_list, i));
	list_free(kernel->arg_list);

	/* Free lists */
	list_free(kernel->uav_read_list);
	list_free(kernel->uav_write_list);
	list_free(kernel->constant_buffer_list);

	/* AMD Binary (internal ELF) */
	if (kernel->bin_file)
		evg_bin_file_free(kernel->bin_file);

	/* Free kernel */
	evg_opencl_object_remove(kernel);
	free(kernel);
}


struct evg_opencl_kernel_arg_t *evg_opencl_kernel_arg_create(char *name)
{
	struct evg_opencl_kernel_arg_t *arg;
	arg = calloc(1, sizeof(struct evg_opencl_kernel_arg_t) + strlen(name) + 1);
	strcpy(arg->name, name);
	return arg;
}


void evg_opencl_kernel_arg_free(struct evg_opencl_kernel_arg_t *arg)
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

void opencl_kernel_load_metadata(struct evg_opencl_kernel_t *kernel)
{
	char line[MAX_STRING_SIZE];
	char *line_ptrs[MAX_STRING_SIZE];
	int token_count;
	struct evg_opencl_kernel_arg_t *arg;
	struct elf_buffer_t *buffer;

	/* Open as text file */
	buffer = &kernel->metadata_buffer;
	elf_buffer_seek(buffer, 0);
	evg_opencl_debug("Kernel Metadata:\n"); 
	for (;;) {

		/* Read line from buffer */
		elf_buffer_read_line(buffer, line, MAX_STRING_SIZE);
		if (!line[0])
			break;
		evg_opencl_debug("\t%s\n", line);

		/* Split line */
		line_ptrs[0] = strtok(line, ":;\n");
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

		/* Image */
		if (!strcmp(line_ptrs[0], "image")) {

			/* Create input image argument */
			arg = evg_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = EVG_OPENCL_KERNEL_ARG_KIND_IMAGE;
			if (!strcmp(line_ptrs[2], "2D")) {
				/* Ignore dimensions for now */
			} else if (!strcmp(line_ptrs[2], "3D")) {
				/* Ignore dimensions for now */
			} else {
				fatal("%s: Invalid number of dimensions for OpenCL Image (%s)\n%s",
					__FUNCTION__, line_ptrs[2], err_evg_opencl_param_note);
			}
			
			if (!strcmp(line_ptrs[3], "RO")) {
				arg->access_type = EVG_OPENCL_KERNEL_ARG_READ_ONLY;
			} else if (!strcmp(line_ptrs[3], "WO")) {
				arg->access_type = EVG_OPENCL_KERNEL_ARG_WRITE_ONLY;
			} else {
				fatal("%s: Invalid memory access type for OpenCL Image (%s)\n%s",
					__FUNCTION__, line_ptrs[3], err_evg_opencl_param_note);
			}
			arg->uav = atoi(line_ptrs[4]);
			arg->mem_scope = EVG_OPENCL_MEM_SCOPE_GLOBAL;

			list_add(kernel->arg_list, arg);

			continue;

		} 

		/* Memory */
		if (!strcmp(line_ptrs[0], "memory")) {
			if (!strcmp(line_ptrs[1], "hwprivate")) {
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2, "0");
			} else if (!strcmp(line_ptrs[1], "hwregion")) {
				OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(2, "0");
			} else if (!strcmp(line_ptrs[1], "hwlocal")) {
				OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
				kernel->func_mem_local = atoi(line_ptrs[2]);
			} else if (!strcmp(line_ptrs[1], "datareqd")) {
				OPENCL_KERNEL_METADATA_TOKEN_COUNT(2); 
			} else
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED(1);

			continue;
		}

		/* Entry 'value'. Format: value:<ArgName>:<DataType>:<Size>:<ConstNum>:<ConstOffset> */
		if (!strcmp(line_ptrs[0], "value")) {
			OPENCL_KERNEL_METADATA_TOKEN_COUNT(6);
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(3, "1");
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");
			arg = evg_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = EVG_OPENCL_KERNEL_ARG_KIND_VALUE;
			list_add(kernel->arg_list, arg);

			continue;
		}

		/* Entry 'pointer'. Format: pointer:<name>:<type>:?:?:<addr>:?:?:<elem_size> */
		if (!strcmp(line_ptrs[0], "pointer")) {

			/* APP SDK 2.5 supplies 9 tokens, 2.6 supplies 10 tokens */
			if(token_count != 9 && token_count != 10) {
				OPENCL_KERNEL_METADATA_TOKEN_COUNT(10);
			}
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(3, "1");
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(4, "1");

			arg = evg_opencl_kernel_arg_create(line_ptrs[1]);
			arg->kind = EVG_OPENCL_KERNEL_ARG_KIND_POINTER;

			list_add(kernel->arg_list, arg);
			if (!strcmp(line_ptrs[6], "uav")) {
				arg->mem_scope = EVG_OPENCL_MEM_SCOPE_GLOBAL;
				arg->uav = atoi(line_ptrs[7]);
			} else if (!strcmp(line_ptrs[6], "hl")) {
				arg->mem_scope = EVG_OPENCL_MEM_SCOPE_LOCAL;
				arg->uav = atoi(line_ptrs[7]);
			} else if (!strcmp(line_ptrs[6], "hc")) {
				arg->mem_scope = EVG_OPENCL_MEM_SCOPE_GLOBAL;
				arg->uav = atoi(line_ptrs[7]);
			} else
				OPENCL_KERNEL_METADATA_NOT_SUPPORTED(6);

			continue;
		}

		/* Entry 'function'. Format: function:?:<uniqueid> */
		if (!strcmp(line_ptrs[0], "function")) {
			OPENCL_KERNEL_METADATA_TOKEN_COUNT(3);
			OPENCL_KERNEL_METADATA_NOT_SUPPORTED_NEQ(1, "1");
			kernel->func_uniqueid = atoi(line_ptrs[2]);
			continue;
		}

		/* Entry 'sampler'. Format: sampler:name:ID:location:value.
		 * 'location' is 1 for kernel defined samplers, 0 for kernel argument.
		 * 'value' is bitfield value of sampler (0 if a kernel argument) */
		if (!strcmp(line_ptrs[0], "sampler")) {

			/* As far as I can tell, the actual sampler data is stored 
			 * as a value, so adding it to the argument list is not required */
			continue;
		}

		/* Warn about uninterpreted entries */
		warning("kernel '%s': unknown meta data entry '%s'",
			kernel->name, line_ptrs[0]);
	}
}


/* Extract and analyze information from the program binary associated with 'kernel_name' */
void evg_opencl_kernel_load(struct evg_opencl_kernel_t *kernel, char *kernel_name)
{
	struct evg_opencl_program_t *program;
	char symbol_name[MAX_STRING_SIZE];
	char name[MAX_STRING_SIZE];

	/* First */
	strncpy(kernel->name, kernel_name, MAX_STRING_SIZE);
	program = evg_opencl_object_get(EVG_OPENCL_OBJ_PROGRAM, kernel->program_id);

	/* Read 'metadata' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_metadata", kernel_name);
	opencl_program_read_symbol(program, symbol_name, &kernel->metadata_buffer);
	
	/* Read 'kernel' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_kernel", kernel_name);
	opencl_program_read_symbol(program, symbol_name, &kernel->kernel_buffer);
	
	/* Read 'header' symbol */
	snprintf(symbol_name, MAX_STRING_SIZE, "__OpenCL_%s_header", kernel_name);
	opencl_program_read_symbol(program, symbol_name, &kernel->header_buffer);

	/* Create and parse kernel binary (internal ELF).
	 * The internal ELF is contained in the buffer pointer to by the 'kernel' symbol. */
	snprintf(name, sizeof(name), "clKernel<%s>.InternalELF", kernel_name);
	kernel->bin_file = evg_bin_file_create(kernel->kernel_buffer.ptr, kernel->kernel_buffer.size, name);
	
	/* Analyze 'metadata' file */
	opencl_kernel_load_metadata(kernel);
}


uint32_t evg_opencl_kernel_get_work_group_info(struct evg_opencl_kernel_t *kernel, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t size_ret = 0;
	void *info = NULL;

	uint64_t local_mem_size = 0;
	uint32_t max_work_group_size = 256;  /* FIXME */

	switch (name) {

	case 0x11b0:  /* CL_KERNEL_WORK_GROUP_SIZE */
		info = &max_work_group_size;
		size_ret = 4;
		break;

	case 0x11b2:  /* CL_KERNEL_LOCAL_MEM_SIZE */
	{
		int i;
		struct evg_opencl_kernel_arg_t *arg;

		/* Compute local memory usage */
		local_mem_size = kernel->func_mem_local;
		for (i = 0; i < list_count(kernel->arg_list); i++) {
			arg = list_get(kernel->arg_list, i);
			if (arg->mem_scope == EVG_OPENCL_MEM_SCOPE_LOCAL)
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
			__FUNCTION__, name, err_evg_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}




/* OpenCL Mem */

struct evg_opencl_mem_t *evg_opencl_mem_create()
{
	struct evg_opencl_mem_t *mem;

	mem = calloc(1, sizeof(struct evg_opencl_mem_t));
	mem->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_MEM);
	mem->ref_count = 1;
	evg_opencl_object_add(mem);
	return mem;
}


void evg_opencl_mem_free(struct evg_opencl_mem_t *mem)
{
	evg_opencl_object_remove(mem);
	free(mem);
}




/* OpenCL Event */

struct evg_opencl_event_t *evg_opencl_event_create(enum evg_opencl_event_kind_t kind)
{
	struct evg_opencl_event_t *event;

	event = calloc(1, sizeof(struct evg_opencl_event_t));
	event->id = evg_opencl_object_new_id(EVG_OPENCL_OBJ_EVENT);
	event->ref_count = 1;
	event->kind = kind;
	evg_opencl_object_add(event);
	return event;
}


void evg_opencl_event_free(struct evg_opencl_event_t *event)
{
	evg_opencl_object_remove(event);
	free(event);
}


uint32_t evg_opencl_event_get_profiling_info(struct evg_opencl_event_t *event, uint32_t name,
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
			__FUNCTION__, name, err_evg_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}


long long evg_opencl_event_timer(void)
{
	return x86_emu_timer() * 1000;
}

