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

#include <assert.h>
#include <debug.h>
#include <stdlib.h>

#include <evergreen-emu.h>
#include <mem-system.h>
#include <x86-emu.h>



/* Create a command queue */
struct evg_opencl_command_queue_t *evg_opencl_command_queue_create()
{
	struct evg_opencl_command_queue_t *command_queue;

	/* Allocate */
	command_queue = calloc(1, sizeof(struct evg_opencl_command_queue_t));
	if (!command_queue)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	command_queue->id = evg_opencl_repo_new_object_id(evg_emu->opencl_repo,
		evg_opencl_object_command_queue);
	command_queue->ref_count = 1;
	command_queue->task_list = linked_list_create();

	/* Return */
	evg_opencl_repo_add_object(evg_emu->opencl_repo, command_queue);
	return command_queue;
}


/* Free command queue */
void evg_opencl_command_queue_free(struct evg_opencl_command_queue_t *command_queue)
{
	/* Check that task list is empty */
	if (linked_list_count(command_queue->task_list))
		fatal("%s: freed command queue is not empty", __FUNCTION__);
	
	/* Free */
	evg_opencl_repo_remove_object(evg_emu->opencl_repo, command_queue);
	linked_list_free(command_queue->task_list);
	free(command_queue);
}


/* Create a command queue task */
struct evg_opencl_command_queue_task_t *evg_opencl_command_queue_task_create(enum
	evg_opencl_command_queue_task_type_t type)
{
	struct evg_opencl_command_queue_task_t *task;

	/* Allocate */
	task = calloc(1, sizeof(struct evg_opencl_command_queue_t));
	if (!task)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	task->type = type;

	/* Return */
	return task;
}


void evg_opencl_command_queue_task_free(struct evg_opencl_command_queue_task_t *task)
{
	free(task);
}


void evg_opencl_command_queue_submit(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_queue_task_t *task)
{
	struct linked_list_t *task_list;

	/* Check that task is not enqueued */
	task_list = command_queue->task_list;
	linked_list_find(task_list, task);
	if (!task_list->error_code)
		fatal("%s: task already enqueued", __FUNCTION__);
	
	/* Enqueue task */
	linked_list_add(task_list, task);
}


void evg_opencl_command_queue_complete(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_queue_task_t *task)
{
	struct linked_list_t *task_list;

	/* Check that task is in command queue */
	task_list = command_queue->task_list;
	linked_list_find(task_list, task);
	if (task_list->error_code)
		fatal("%s: task is not in command queue", __FUNCTION__);
	
	/* Remove task */
	linked_list_remove(task_list);

	/* x86 contexts might be waiting for the command queue to get empty
	 * (e.g., suspended in a 'clFinish' call. Check events. */
	x86_emu_process_events_schedule();
}


int evg_opencl_command_queue_can_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct evg_opencl_command_queue_t *command_queue;
	struct linked_list_t *task_list;

	int can_wakeup;

	/* x86 context can wakeup if task list is empty */
	command_queue = data;
	task_list = command_queue->task_list;
	can_wakeup = !task_list->count;

	/* Debug */
	if (can_wakeup)
		evg_opencl_debug("\tcycle %lld - command queue 0x%x empty"
			" - context resumed\n", esim_cycle, command_queue->id);

	/* Return */
	return can_wakeup;
}

