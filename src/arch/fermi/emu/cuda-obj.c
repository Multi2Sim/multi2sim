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

#include <stdlib.h>
#include <assert.h>

#include <arch/x86/emu/emu.h>
#include <lib/util/debug.h>
#include <mem-system/mem-system.h>

#include "emu.h"


/*
 * Objects
 */

struct linked_list_t *frm_cuda_object_list;


/* Add an CUDA object to object list */

void frm_cuda_object_add(void *object)
{
	linked_list_find(frm_cuda_object_list, object);
	assert(frm_cuda_object_list->error_code);
	linked_list_add(frm_cuda_object_list, object);
}


/* Remove an CUDA object from object list */

void frm_cuda_object_remove(void *object)
{
	linked_list_find(frm_cuda_object_list, object);
	assert(!frm_cuda_object_list->error_code);
	linked_list_remove(frm_cuda_object_list);
}


/* Look for an CUDA object in the object list. The 'id' is the
 * first field for every object. */

void *frm_cuda_object_get(enum frm_cuda_obj_t type, unsigned int id)
{
	void *object;
	unsigned int object_id;

	if (id >> 16 != type)
		fatal("%s: requested CUDA object of incorrect type",
			__FUNCTION__);
	LINKED_LIST_FOR_EACH(frm_cuda_object_list)
	{
		if (!(object = linked_list_get(frm_cuda_object_list)))
			panic("%s: empty object", __FUNCTION__);
		object_id = * (unsigned int *) object;
		if (object_id == id)
			return object;
	}
	fatal("%s: requested CUDA object does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}


/* Get the oldest created CUDA object of the specified type */

void *frm_cuda_object_get_type(enum frm_cuda_obj_t type)
{
        void *object;
        unsigned int object_id;

        /* Find object */
        LINKED_LIST_FOR_EACH(frm_cuda_object_list)
        {
                if (!(object = linked_list_get(frm_cuda_object_list)))
                        panic("%s: empty object", __FUNCTION__);
                object_id = * (unsigned int *) object;
                if (object_id >> 16 == type)
                        return object;
        }

        /* No object found */
        return NULL;
}


/* Assignment of CUDA object identifiers
 * An identifier is a 32-bit value, whose 16 most significant bits represent the
 * object type, while the 16 least significant bits represent a unique object ID. */

unsigned int frm_cuda_object_new_id(enum frm_cuda_obj_t type)
{
	static unsigned int cuda_current_object_id;
	unsigned int id;

	id = (type << 16) | cuda_current_object_id;
	cuda_current_object_id++;
	if (cuda_current_object_id > 0xffff)
		fatal("cuda_object_new_id: too many CUDA objects");
	return id;
}


/* Free all CUDA objects in the object list */

void frm_cuda_object_free_all()
{
	void *object;

	/* Devices */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_DEVICE)))
		frm_cuda_device_free((struct frm_cuda_device_t *) object);

	/* Contexts */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_CONTEXT)))
		frm_cuda_context_free((struct frm_cuda_context_t *) object);

	/* Modules */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_MODULE)))
		frm_cuda_module_free((struct frm_cuda_module_t *) object);

	/* Functions */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_FUNCTION)))
		frm_cuda_function_free((struct frm_cuda_function_t *) object);

        /* Mems */
        while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_MEMORY)))
                frm_cuda_memory_free((struct frm_cuda_memory_t *) object);

	/* Streams */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_STREAM)))
		frm_cuda_stream_free((struct frm_cuda_stream_t *) object);

	/* Any object left */
	if (linked_list_count(frm_cuda_object_list))
		panic("cuda_object_free_all: objects remaining in the list");
}



/*
 * CUDA Device
 */


/* Create a device */

struct frm_cuda_device_t *frm_cuda_device_create()
{
	struct frm_cuda_device_t *device;

	/* Allocate */
	device = calloc(1, sizeof(struct frm_cuda_device_t));
	if (!device)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	device->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_DEVICE);

	/* Return */
	frm_cuda_object_add(device);
	return device;
}


/* Free device */

void frm_cuda_device_free(struct frm_cuda_device_t *device)
{
	frm_cuda_object_remove(device);
	free(device);
}



/*
 * CUDA Context
 */


/* Create a context */

struct frm_cuda_context_t *frm_cuda_context_create()
{
	struct frm_cuda_context_t *context;

	/* Allocate */
	context = calloc(1, sizeof(struct frm_cuda_context_t));
	if (!context)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	context->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_CONTEXT);
	context->ref_count = 1;

	/* Return */
	frm_cuda_object_add(context);
	return context;
}


/* Free context */

void frm_cuda_context_free(struct frm_cuda_context_t *context)
{
	frm_cuda_object_remove(context);
	free(context);
}



/*
 * CUDA Module
 */


/* Create a module */

struct frm_cuda_module_t *frm_cuda_module_create()
{
	struct frm_cuda_module_t *module;

	/* Allocate */
	module = calloc(1, sizeof(struct frm_cuda_module_t));
	if (!module)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	module->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_MODULE);
	module->ref_count = 1;

	/* Return */
	frm_cuda_object_add(module);
	return module;
}


/* Free module */

void frm_cuda_module_free(struct frm_cuda_module_t *module)
{
	if (module->elf_file)
		elf_file_free(module->elf_file);
	frm_cuda_object_remove(module);
	free(module);
}



/*
 * CUDA Function Argument 
 */


struct frm_cuda_function_arg_t *frm_cuda_function_arg_create(char *name)
{
	struct frm_cuda_function_arg_t *arg;

	/* Allocate */
	arg = calloc(1, sizeof(struct frm_cuda_function_arg_t) + strlen(name) + 1);
	if (!arg)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	strncpy(arg->name, name, MAX_STRING_SIZE);

	/* Return */
	return arg;
}


void frm_cuda_function_arg_free(struct frm_cuda_function_arg_t *arg)
{
	free(arg);
}



/*
 * CUDA Function
 */


struct frm_cuda_function_t *frm_cuda_function_create()
{
	struct frm_cuda_function_t *function;

	/* Allocate */
	function = calloc(1, sizeof(struct frm_cuda_function_t));
	if (!function)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	function->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_FUNCTION);
	function->ref_count = 1;
	function->arg_list = list_create();

	/* Return */
	frm_cuda_object_add(function);
	return function;
}


void frm_cuda_function_free(struct frm_cuda_function_t *function)
{
	int i;

	/* Free arguments */
	for (i = 0; i < list_count(function->arg_list); i++)
		frm_cuda_function_arg_free((struct frm_cuda_function_arg_t *)list_get(function->arg_list, i));
	list_free(function->arg_list);

	/* FIXME: free ELF file */

	/* Free function */
	frm_cuda_object_remove(function);
	free(function);
}



/*
 * CUDA Memory
 */


struct frm_cuda_memory_t *frm_cuda_memory_create()
{
        struct frm_cuda_memory_t *mem;

        /* Allocate */
        mem = calloc(1, sizeof(struct frm_cuda_memory_t));
        if (!mem)
                fatal("%s: out of memory", __FUNCTION__);

        /* Initialize */
        mem->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_MEMORY);
        mem->ref_count = 1;

        /* Return */
        frm_cuda_object_add(mem);
        return mem;
}


void frm_cuda_memory_free(struct frm_cuda_memory_t *mem)
{
        frm_cuda_object_remove(mem);
        free(mem);
}



/*
 * CUDA Stream
 */


/* Create a stream */

struct frm_cuda_stream_t *frm_cuda_stream_create()
{
	struct frm_cuda_stream_t *stream;

	/* Allocate */
	stream = calloc(1, sizeof(struct frm_cuda_stream_t));
	if (!stream)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	stream->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_STREAM);
	stream->ref_count = 1;

	/* Return */
	frm_cuda_object_add(stream);
	return stream;
}


/* Free stream */

void frm_cuda_stream_free(struct frm_cuda_stream_t *stream)
{
	frm_cuda_object_remove(stream);
	free(stream);
}

