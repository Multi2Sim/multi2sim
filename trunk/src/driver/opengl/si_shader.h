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

#include <lib/util/elf-format.h>

struct opengl_si_program_t;

struct opencl_si_shader_t
{
	int id;
	char *name;

	/* Program that shader belongs to */
	struct opencl_si_program_t *program;

	/* AMD Shader binary (internal ELF) */
	struct si_bin_file_t *bin_file;

};

struct opencl_si_shader_t *opencl_si_shader_create(
	struct opencl_si_program_t *program, char *name);
void opencl_si_shader_free(struct opencl_si_shader_t *shader);

struct si_ndrange_t;
void opencl_si_shader_setup_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange);
void opencl_si_shader_setup_ndrange_args(struct opencl_si_shader_t *shader,
	struct si_ndrange_t *ndrange);
void opencl_si_shader_debug_ndrange_state(struct opencl_si_shader_t *shader, 
	struct si_ndrange_t *ndrange);
void opencl_si_shader_create_ndrange_tables(struct si_ndrange_t *ndrange);
void opencl_si_shader_create_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange);


#endif
