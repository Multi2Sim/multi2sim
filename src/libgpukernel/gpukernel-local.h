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

#ifndef GPUKERNEL_LOCAL_H
#define GPUKERNEL_LOCAL_H

#include <gpukernel.h>
#include <m2skernel.h>
#include <stdint.h>



/* Error messages */

extern char *err_opencl_note;
extern char *err_opencl_param_note;



/* OpenCL API Implementation */


/* OpenCL objects */

enum opencl_obj_enum {
	OPENCL_OBJ_PLATFORM = 1,
	OPENCL_OBJ_DEVICE,
	OPENCL_OBJ_CONTEXT,
	OPENCL_OBJ_COMMAND_QUEUE,
	OPENCL_OBJ_PROGRAM,
	OPENCL_OBJ_KERNEL,
	OPENCL_OBJ_MEM
};

extern struct lnlist_t *opencl_object_list;

void opencl_object_add(void *object);
void opencl_object_remove(void *object);
void *opencl_object_get(enum opencl_obj_enum type, uint32_t id);
void *opencl_object_get_type(enum opencl_obj_enum type);
uint32_t opencl_object_new_id(enum opencl_obj_enum type);
void opencl_object_free_all(void);




/* OpenCL platform */

struct opencl_platform_t
{
	uint32_t id;
};

extern struct opencl_platform_t *opencl_platform;

struct opencl_platform_t *opencl_platform_create();
void opencl_platform_free(struct opencl_platform_t *platform);
uint32_t opencl_platform_get_info(struct opencl_platform_t *platform, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL devices */

struct opencl_device_t
{
	uint32_t id;
};

struct opencl_device_t *opencl_device_create();
void opencl_device_free(struct opencl_device_t *device);
uint32_t opencl_device_get_info(struct opencl_device_t *device, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL contexts */

struct opencl_context_t
{
	uint32_t id;
	uint32_t platform_id;
	uint32_t device_id;
};

struct opencl_context_t *opencl_context_create();
void opencl_context_free(struct opencl_context_t *context);
uint32_t opencl_context_get_info(struct opencl_context_t *context, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);
void opencl_context_set_properties(struct opencl_context_t *context, struct mem_t *mem, uint32_t addr);




/* OpenCL command queue */

struct opencl_command_queue_t
{
	uint32_t id;
	uint32_t device_id;
	uint32_t context_id;
};

struct opencl_command_queue_t *opencl_command_queue_create();
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue);
void opencl_command_queue_read_properties(struct opencl_command_queue_t *command_queue, struct mem_t *mem, uint32_t addr);




/* OpenCL program */

struct opencl_program_t
{
	uint32_t id;
	uint32_t device_id;  /* Only one device allowed */
	uint32_t context_id;

	void *binary;  /* Main ELF binary  */
	int binary_size;

	void *code;  /* Main ELF binary's 1st '.text' section's 2nd '.text' section */
	int code_size;
};

struct opencl_program_t *opencl_program_create();
void opencl_program_free(struct opencl_program_t *program);
void opencl_program_build(struct opencl_program_t *program);




/* OpenCL kernel */

struct opencl_kernel_t
{
	uint32_t id;
	uint32_t program_id;
	char kernel_name[MAX_STRING_SIZE];
	struct list_t *arg_list;
};


struct opencl_kernel_t *opencl_kernel_create();
void opencl_kernel_free(struct opencl_kernel_t *kernel);
void opencl_kernel_arg_free(struct opencl_kernel_t *kernel, int idx);
void opencl_kernel_arg_set(struct opencl_kernel_t *kernel, int idx, void *buf, int size);



/* OpenCL mem */

struct opencl_mem_t
{
	uint32_t id;
	uint32_t size;
	uint32_t host_ptr;
};

struct opencl_mem_t *opencl_mem_create();
void opencl_mem_free(struct opencl_mem_t *mem);


#endif

