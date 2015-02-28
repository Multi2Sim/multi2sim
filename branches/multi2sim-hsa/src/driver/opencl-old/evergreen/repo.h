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

#ifndef ARCH_EVERGREEN_EMU_OPENCL_REPO_H
#define ARCH_EVERGREEN_EMU_OPENCL_REPO_H


enum evg_opencl_object_type_t
{
	evg_opencl_object_invalid,
	evg_opencl_object_platform,
	evg_opencl_object_device,
	evg_opencl_object_context,
	evg_opencl_object_command_queue,
	evg_opencl_object_program,
	evg_opencl_object_kernel,
	evg_opencl_object_mem,
	evg_opencl_object_event,
	evg_opencl_object_sampler
};

struct evg_opencl_repo_t;

struct evg_opencl_repo_t *evg_opencl_repo_create(void);
void evg_opencl_repo_free(struct evg_opencl_repo_t *repo);

void evg_opencl_repo_add_object(struct evg_opencl_repo_t *repo,
	void *object);
void evg_opencl_repo_remove_object(struct evg_opencl_repo_t *repo,
	void *object);
void *evg_opencl_repo_get_object(struct evg_opencl_repo_t *repo,
	enum evg_opencl_object_type_t type, unsigned int object_id);

void *evg_opencl_repo_get_object_of_type(struct evg_opencl_repo_t *repo,
	enum evg_opencl_object_type_t type);
unsigned int evg_opencl_repo_new_object_id(struct evg_opencl_repo_t *repo,
	enum evg_opencl_object_type_t type);

void evg_opencl_repo_free_all_objects(struct evg_opencl_repo_t *repo);


#endif

