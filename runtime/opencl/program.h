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


struct clrt_device_program_t
{
	struct clrt_device_type_t *device_type;
	void *handle;
	void *filename;
};


/* Program object */
#define opencl_program_t _cl_program
struct _cl_program
{
	int num_entries;
	struct clrt_device_program_t *entries;
};


/* Create/free */
struct opencl_program_t *opencl_program_create(void);
void opencl_program_free(struct opencl_program_t *program);


#endif

