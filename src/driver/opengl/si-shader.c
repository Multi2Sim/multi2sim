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

#include <unistd.h>
#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <arch/southern-islands/emu/opengl-bin-file.h> 
 
#include "si-shader.h"
#include "si-program.h"

/*
 * Private Functions
 */



/*
 * Public Functions
 */

struct opengl_si_shader_t *opengl_si_shader_create(
	struct opengl_si_program_t *program, unsigned int shader_kind)
{
	struct list_t *shaders_list;
	struct opengl_si_shader_t *shdr;
	struct si_opengl_program_binary_t *prog_bin;
	struct si_opengl_shader_binary_t *shdr_bin;
	int i;

	/* Allocate */
	shdr = xcalloc(1, sizeof(struct opengl_si_shader_t));

	/* Initialize */
	shdr->program = program;
	prog_bin = program->program_bin;
	shaders_list = prog_bin->shaders;
	/* FIXME: is it true that an OpenGL program binary can only contain 1 shader of each kind */
	LIST_FOR_EACH(shaders_list, i)
	{
		shdr_bin = list_get(shaders_list, i);
		if (shdr_bin->shader_kind == shader_kind)
			shdr->shader_bin = shdr_bin;
	}

	/* Return */
	return shdr;

}

void opengl_si_shader_free(struct opengl_si_shader_t *shader)
{
	/* Free */
	free(shader);
}