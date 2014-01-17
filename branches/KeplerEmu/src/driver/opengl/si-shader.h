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

#ifndef DRIVER_OPENGL_SI_SHADER_H
#define DRIVER_OPENGL_SI_SHADER_H

#include <lib/class/class.h>
#include <src/arch/southern-islands/asm/opengl-bin-file.h>


/* Forward declaration */
struct opengl_si_program_t;
struct si_opengl_shader_binary_t;

struct opengl_si_shader_t
{
	/* Program that shader belongs to */
	struct opengl_si_program_t *program;

	/* Shader type, information pass from runtime to driver and set at here */
	enum opengl_si_shader_binary_kind_t shader_kind;

	/* AMD Shader binary (internal ELF), which has all information of the shader binary */
	struct opengl_si_shader_binary_t *bin;

	/* Equivalent to arg_list in OpenCL, elements of type SIInput */
	struct list_t *input_list; 

	/* Memory requirements */
	int mem_size_local;
	int mem_size_private;	

};

void opengl_si_shader_list_init(OpenglDriver *driver);
void opengl_si_shader_list_done(OpenglDriver *driver);

struct opengl_si_shader_t *opengl_si_shader_create(struct list_t *shdr_lst, struct opengl_si_program_t *program,
	unsigned int shader_id, unsigned int shader_kind);
void opengl_si_shader_free(struct opengl_si_shader_t *shdr);

void opengl_si_shader_create_ndrange_constant_buffers(SINDRange *ndrange, MMU *gpu_mmu);
void opengl_si_shader_setup_ndrange_constant_buffers(SINDRange *ndrange);
void opengl_si_shader_setup_ndrange_inputs(struct opengl_si_shader_t *shdr,
		SINDRange *ndrange);
void opengl_si_shader_debug_ndrange_state(struct opengl_si_shader_t *shader, 
		SINDRange *ndrange);
void opengl_si_shader_create_ndrange_tables(SINDRange *ndrange, MMU *mmu);

#endif
