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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_REPO_H
#define ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_REPO_H


/* Repository for 'struct si_isa_write_task_t' objects */
extern struct repos_t *si_isa_write_task_repos;

enum si_opencl_object_type_t
{
	si_opencl_object_invalid,
	si_opencl_object_platform,
	si_opencl_object_device,
	si_opencl_object_context,
	si_opencl_object_command_queue,
	si_opencl_object_program,
	si_opencl_object_kernel,
	si_opencl_object_mem,
	si_opencl_object_event,
	si_opencl_object_sampler
};

struct si_opencl_repo_t;

struct si_opencl_repo_t *si_opencl_repo_create(void);
void si_opencl_repo_free(struct si_opencl_repo_t *repo);

void si_opencl_repo_add_object(struct si_opencl_repo_t *repo,
	void *object);
void si_opencl_repo_remove_object(struct si_opencl_repo_t *repo,
	void *object);
void *si_opencl_repo_get_object(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type, unsigned int object_id);

void *si_opencl_repo_get_object_of_type(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type);
unsigned int si_opencl_repo_new_object_id(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type);

void si_opencl_repo_free_all_objects(struct si_opencl_repo_t *repo);

#endif
