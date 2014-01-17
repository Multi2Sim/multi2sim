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

#include "opencl.h"


/* Entry of a program associated with one device type. */
struct opencl_program_entry_t
{
	/* Associated device */
	struct opencl_device_t *device;

	/* Architecture-specific program object. The actual type of this
	 * variable is 'opencl_xxx_program_t'. */
	void *arch_program;
};


/* Program object */
struct opencl_program_t
{
	/* Associated context */
	struct opencl_context_t *context;

	/* Source code */
	char *source;

	/* Program entries, one per device type, of type 'opencl_program_entry_t' */
	struct list_t *entry_list;
};


/* Create/free */
struct opencl_program_t *opencl_program_create(struct opencl_context_t *context);
void opencl_program_free(struct opencl_program_t *program);

/* Return true if any of the program entries provides an architecture-specific
 * program associated with a given device. */
int opencl_program_has_device(struct opencl_program_t *program,
		struct opencl_device_t *device);

/* Add a new entry to the program with an architecture-specific program and its
 * associated device. */
struct opencl_program_entry_t *opencl_program_add(struct opencl_program_t *program,
		struct opencl_device_t *device, void *arch_program);


#endif

