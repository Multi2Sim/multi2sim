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

#ifndef DRIVER_OPENGL_SI_PROGRAM_H
#define DRIVER_OPENGL_SI_PROGRAM_H


/*
 * Program List
 */

extern struct list_t *opengl_si_program_list;

void opengl_si_program_list_init(void);
void opengl_si_program_list_done(void);


/*
 * Constant Buffer
 */

struct opengl_si_constant_buffer_t
{
	int id;  /* Constant buffer ID (2-24) */
	unsigned int device_ptr;
	unsigned int size;
};

struct opengl_si_constant_buffer_t *opengl_si_constant_buffer_create(int id,
	unsigned int device_ptr, unsigned int size);
void opengl_si_constant_buffer_free(struct opengl_si_constant_buffer_t *constant_buffer);



/*
 * OpengL Southern Islands Program
 */

struct si_opengl_program_binary_t;
struct opengl_si_program_t
{
	int id;
	
	/* Program binary */
	struct si_opengl_program_binary_t *program_bin;

	/* Constant buffers are shared by all shaders compiled in the
	 * same binary. This list is comprised of elements of type
	 * 'opengl_si_constant_buffer_t'. */
	struct list_t *constant_buffer_list;
};

struct opengl_si_program_t *opengl_si_program_create(unsigned int program_id);
void opengl_si_program_free(struct opengl_si_program_t *program);

void opengl_si_program_set_binary(struct opengl_si_program_t *program,
		void *buf, unsigned int size);


#endif
