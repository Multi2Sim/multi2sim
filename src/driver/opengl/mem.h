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

#ifndef DRIVER_OPENGL_MEM_H
#define DRIVER_OPENGL_MEM_H

#include <pthread.h>

struct linked_list_t;

struct opengl_mem_t
{
	unsigned int id;
	unsigned int ref_count;
	pthread_mutex_t ref_mutex;
	
	unsigned int size;
	unsigned int flags;
	unsigned int host_ptr;
	unsigned int device_ptr;
};

struct opengl_mem_t *opengl_mem_create();
void opengl_mem_free(struct opengl_mem_t *mem);

struct linked_list_t *opengl_mem_repo_create();
void opengl_mem_repo_free(struct linked_list_t *shdr_tbl);
void opengl_mem_repo_add(struct linked_list_t *shdr_tbl, struct opengl_mem_t *shdr);
int opengl_mem_repo_remove(struct linked_list_t *shdr_tbl, struct opengl_mem_t *shdr);
struct opengl_mem_t *opengl_mem_repo_get(struct linked_list_t *shdr_tbl, int id);
struct opengl_mem_t *opengl_mem_repo_reference(struct linked_list_t *shdr_tbl, int id);

#endif
