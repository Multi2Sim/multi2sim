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

#ifndef DRIVER_OPENCL_SI_PROGRAM_OLD_H
#define DRIVER_OPENCL_SI_PROGRAM_OLD_H


/*
 * Constant Buffer
 */

struct opencl_si_constant_buffer_t
{
	int id;  /* Constant buffer ID (2-24) */
	unsigned int size;
	void *data;
};

struct opencl_si_constant_buffer_t *opencl_si_constant_buffer_create(int id,
	unsigned int size, void *data);
void opencl_si_constant_buffer_free(struct opencl_si_constant_buffer_t *constant_buffer);



/*
 * OpenCL Southern Islands Program
 */


struct opencl_si_program_t
{
	int id;
	OpenclDriver *driver;
	
	/* ELF binary */
	struct elf_file_t *elf_file;

	/* Constant buffers are shared by all kernels compiled in the
	 * same binary. This list is comprised of elements of type
	 * 'opencl_si_constant_buffer_t'. */
	struct list_t *constant_buffer_list;
};

struct opencl_si_program_t *opencl_si_program_create(OpenclDriver *driver, int id);
void opencl_si_program_free(struct opencl_si_program_t *program);

void opencl_si_program_set_binary(struct opencl_si_program_t *program,
		void *buf, unsigned int size);


#endif

