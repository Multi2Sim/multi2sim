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

#ifndef RUNTIME_OPENCL_PROGRAM_H
#define RUNTIME_OPENCL_PROGRAM_H

#include "clrt.h"


/* Entry of a program associated with one device type. */
struct opencl_program_entry_t
{
	/* Associated device */
	struct opencl_device_type_t *device_type;

	/* Temporary file where the internal ELF file was dumped */
	char *file_name;

	/* Handle returned by 'dlopen' when interpreting the internal
	 * ELF file for dynamic linking. */
	void *dlhandle;
};


/* Program object */
struct opencl_program_t
{
	/* Program entries, one per device type, of type 'opencl_program_entry_t' */
	struct list_t *entry_list;
};


/* Create/free */
struct opencl_program_t *opencl_program_create(void);
void opencl_program_free(struct opencl_program_t *program);


#endif

