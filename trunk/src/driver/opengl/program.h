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

#ifndef DRIVER_OPENGL_PROGRAM_H
#define DRIVER_OPENGL_PROGRAM_H

#include <GL/glut.h>
#include <pthread.h>

#define OPENGL_PROGRAM_TABLE_INIT_SIZE	16

struct linked_list_t;
struct opengl_shader_t;
struct si_opengl_bin_file_t;
struct si_ndrange_t;

/* Program objects are stored in a linked list repository */
struct opengl_program_t
{
	GLuint id;
	GLint ref_count;
	pthread_mutex_t ref_mutex;
	GLboolean delete_pending;
	
	/* List contains pointers to shaders*/
	struct linked_list_t *attached_shader;

	/* The entire shader binary is stored here */
	struct si_opengl_bin_file_t *si_shader_binary;
};

struct opengl_program_t *opengl_program_create();
void opengl_program_free(struct opengl_program_t *prg);
void opengl_program_detele(struct opengl_program_t *prg);
void opengl_program_bind(struct opengl_program_t *prg, struct opengl_program_t **prg_bnd_ptr);
void opengl_program_unbind(struct opengl_program_t *prg, struct opengl_program_t **prg_bnd_ptr);
void opengl_program_attach_shader(struct opengl_program_t *prg, struct opengl_shader_t *shdr);
void opengl_program_detach_shader(struct opengl_program_t *prg, struct opengl_shader_t *shdr);
struct si_opengl_shader_t *opengl_program_get_shader(struct opengl_program_t *prg, int shader_kind);
struct elf_buffer_t *opengl_program_get_shader_isa(struct opengl_program_t *prg, int shader_kind);

void opengl_program_setup_ndrange_state(struct opengl_program_t *prg, struct si_ndrange_t *ndrange);

struct linked_list_t *opengl_program_repo_create();
void opengl_program_repo_free(struct linked_list_t *prg_repo);
void opengl_program_repo_add(struct linked_list_t *prg_repo, struct opengl_program_t *prg);
int opengl_program_repo_remove(struct linked_list_t *prg_repo, struct opengl_program_t *prg);
struct opengl_program_t *opengl_program_repo_get(struct linked_list_t *prg_repo, int id);
struct opengl_program_t *opengl_program_repo_reference(struct linked_list_t *prg_repo, int id);

#endif
