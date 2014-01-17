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

#ifndef RUNTIME_OPENCL_X86_PROGRAM_H
#define RUNTIME_OPENCL_X86_PROGRAM_H

#include "opencl.h"

/* Forward declarations */
struct opencl_x86_device_t;


/* Program object */
struct opencl_x86_program_t
{
	enum opencl_runtime_type_t type;  /* First field */

	/* Parent generic program object */
	struct opencl_program_t *parent;

	/* Associated architecture-specific device */
	struct opencl_x86_device_t *device;
	
	/* Temporary file where the internal ELF file was dumped */
	char *file_name;

	/* Handle returned by 'dlopen' when interpreting the internal
	 * ELF file for dynamic linking. */
	void *dlhandle;
};


struct opencl_x86_program_t *opencl_x86_program_create(
		struct opencl_program_t *parent,
		struct opencl_x86_device_t *device,
		void *binary,
		unsigned int length);

void opencl_x86_program_free(
		struct opencl_x86_program_t *program);

/* Return true is a binary file is a valid x86 program binary. */
int opencl_x86_program_valid_binary(
	void *device,
	void *binary,
	unsigned int length);


#endif

