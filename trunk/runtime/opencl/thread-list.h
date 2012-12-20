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

#ifndef RUNTIME_OPENCL_THREAD_LIST_H
#define RUNTIME_OPENCL_THREAD_LIST_H

#include <pthread.h>

struct clrt_thread_list_node_t
{
	struct clrt_thread_list_node_t *next;
	void *data;
};


struct clrt_thread_list_t
{
	struct clrt_thread_list_node_t *head;
	pthread_rwlock_t lock;
};

typedef int (*clrt_thread_list_visitor)(void *context, void *data);

struct clrt_thread_list_t *clrt_thread_list_create(void);
void clrt_thread_list_free(struct clrt_thread_list_t *list);

void clrt_thread_list_insert(struct clrt_thread_list_t *list, void *data);
int clrt_thread_list_remove(struct clrt_thread_list_t *list, void *data);
void clrt_thread_list_visit(struct clrt_thread_list_t *list, clrt_thread_list_visitor visitor, void *context);

#endif
