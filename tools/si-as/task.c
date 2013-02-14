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

#include <string.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "task.h"


struct list_t *task_list;

struct si_task_t *si_task_create(char *ID, long offset)
{
	struct si_task_t *task;
	
	/* Allocate */
	task = xcalloc(1, sizeof(struct si_task_t));
	
	/* Initialize the task's offset and ID */
	task->offset = offset;
	task->ID = xstrdup(ID);
	
	/* Return */
	return task;

}

void si_task_free(struct si_task_t *task)
{
	free(task->ID);
	free(task);
}

void si_task_dump(struct si_task_t *task, FILE *f)
{
	fprintf(f, "\tTask ID: %s\n", task->ID);
	fprintf(f, "\tTask Offset: %ld\n", task->offset);
}

/***********Task List Functions***********/
		
void si_task_list_init(void)
{							  
	task_list = list_create();
}

void si_task_list_add(struct si_task_t *task)
{
	list_add(task_list, task);
}

void si_task_list_done(void)
{
	int index; 
	
	for (index = 0; index < task_list->count; index++)
		si_task_free(list_get(task_list, index));
	
	list_free(task_list);
}

void si_task_list_dump(FILE *f)
{
	int index; 
	
	for (index = 0; index < task_list->count; index++)
	{
		si_task_dump(list_get(task_list, index), stdout);
	}
}
