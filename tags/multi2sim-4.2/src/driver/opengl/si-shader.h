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

struct opengl_si_program_t;
struct si_opengl_shader_binary_t *shader_bin;

struct opengl_si_shader_t
{
	/* Shader type */
	enum si_opengl_shader_binary_kind_t shader_kind;

	/* Program that shader belongs to */
	struct opengl_si_program_t *program;

	/* AMD Shader binary (internal ELF) */
	struct si_opengl_shader_binary_t *shader_bin;

	/* Memory requirements */
	int mem_size_local;
	int mem_size_private;	

};

extern struct list_t *opengl_si_shader_list;

void opengl_si_shader_list_init(void);
void opengl_si_shader_list_done(void);


struct opengl_si_shader_t *opengl_si_shader_create(
	unsigned int shader_id, unsigned int shader_kind);
void opengl_si_shader_free(struct opengl_si_shader_t *shdr);
void opengl_si_shader_init( struct opengl_si_program_t *program, unsigned int shader_id);


struct si_ndrange_t;
void opengl_si_shader_create_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange);
void opengl_si_shader_setup_ndrange_constant_buffers(
	struct si_ndrange_t *ndrange);
void opengl_si_shader_setup_ndrange_inputs(struct opengl_si_shader_t *shdr,
		struct si_ndrange_t *ndrange);
void opengl_si_shader_debug_ndrange_state(struct opengl_si_shader_t *shader, 
	struct si_ndrange_t *ndrange);
void opengl_si_shader_create_ndrange_tables(struct si_ndrange_t *ndrange);

#endif
