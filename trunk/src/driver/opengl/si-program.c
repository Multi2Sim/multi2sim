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


#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/string.h>
#include <arch/southern-islands/emu/opengl-bin-file.h>

#include "si-program.h"

/*
 * Program list
 */

struct list_t *opengl_si_program_list;

void opengl_si_program_list_init(void)
{
	/* Already initialized */
	if (opengl_si_program_list)
		return;

	/* Initialize and add one empty element */
	opengl_si_program_list = list_create();
	list_add(opengl_si_program_list, NULL);
}


void opengl_si_program_list_done(void)
{
	int index;
	struct opengl_si_program_t *program;

	/* Not initialized */
	if (!opengl_si_program_list)
		return;

	/* Free list of Southern Islands programs */
	LIST_FOR_EACH(opengl_si_program_list, index)
	{
		program = list_get(opengl_si_program_list, index);
		if (program)
			opengl_si_program_free(program);
	}
	list_free(opengl_si_program_list);
}

/*
 * Constant Buffer
 */

struct opengl_si_constant_buffer_t *opengl_si_constant_buffer_create(int id,
	unsigned int device_ptr, unsigned int size)
{
	struct opengl_si_constant_buffer_t *constant_buffer;

	/* Initialize */
	constant_buffer = xcalloc(1, sizeof(struct opengl_si_constant_buffer_t));
	constant_buffer->id = id;
	constant_buffer->device_ptr = device_ptr;
	constant_buffer->size = size;

	/* Return */
	return constant_buffer;
}


void opengl_si_constant_buffer_free(struct opengl_si_constant_buffer_t *constant_buffer)
{
	free(constant_buffer);
}

/*
 * Program
 */

static void opengl_si_program_initialize_constant_buffers(
		struct opengl_si_program_t *program)
{
	/* TODO: Constant buffers store Uniforms ? */
} 

struct opengl_si_program_t *opengl_si_program_create(unsigned int program_id)
{
	struct opengl_si_program_t *program;

	/* Initialize */
	program = xcalloc(1, sizeof(struct opengl_si_program_t));

	/* Insert in program list */
	opengl_si_program_list_init();
	program->id = program_id;
	list_insert(opengl_si_program_list, program_id, program);

	/* Return */
	return program;
}


void opengl_si_program_free(struct opengl_si_program_t *program)
{
	int index;

	/* Free constant buffers */
	if (program->constant_buffer_list)
	{
		LIST_FOR_EACH(program->constant_buffer_list, index)
			opengl_si_constant_buffer_free(list_get(
					program->constant_buffer_list,
					index));
		list_free(program->constant_buffer_list);
	}

	/* ELF file */
	if (program->program_bin)
		si_opengl_program_binary_free(program->program_bin);

	/* Free program */
	free(program);
}


void opengl_si_program_set_binary(struct opengl_si_program_t *program,
		void *buf, unsigned int size)
{
	char name[MAX_STRING_SIZE];

	/* Already set */
	if (program->program_bin)
		fatal("%s: binary already set", __FUNCTION__);

	/* Load ELF binary from guest memory */
	snprintf(name, sizeof name, "program[%d].externalELF", program->id);

	/* Program contains shader binary */
	program->program_bin = si_opengl_program_binary_create(buf, size, name);

	/* Initialize constant buffers */
	opengl_si_program_initialize_constant_buffers(program);
}

