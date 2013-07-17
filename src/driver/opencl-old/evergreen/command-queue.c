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


#include <arch/evergreen/emu/emu.h>
#include <arch/x86/emu/emu.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>

#include "command-queue.h"
#include "opencl.h"
#include "repo.h"



/* Create a command queue */
struct evg_opencl_command_queue_t *evg_opencl_command_queue_create()
{
	struct evg_opencl_command_queue_t *command_queue;

	/* Initialize */
	command_queue = xcalloc(1, sizeof(struct evg_opencl_command_queue_t));
	command_queue->id = evg_opencl_repo_new_object_id(evg_emu->opencl_repo,
		evg_opencl_object_command_queue);
	command_queue->ref_count = 1;
	command_queue->command_list = linked_list_create();

	/* Return */
	evg_opencl_repo_add_object(evg_emu->opencl_repo, command_queue);
	return command_queue;
}


/* Free command queue */
void evg_opencl_command_queue_free(struct evg_opencl_command_queue_t *command_queue)
{
	/* Check that command list is empty */
	if (linked_list_count(command_queue->command_list))
		fatal("%s: freed command queue is not empty", __FUNCTION__);
	
	/* Free */
	evg_opencl_repo_remove_object(evg_emu->opencl_repo, command_queue);
	linked_list_free(command_queue->command_list);
	free(command_queue);
}


/* Create a command */
struct evg_opencl_command_t *evg_opencl_command_create(enum
	evg_opencl_command_type_t type)
{
	struct evg_opencl_command_t *command;

	/* Initialize */
	command = xcalloc(1, sizeof(struct evg_opencl_command_t));
	command->type = type;

	/* Return */
	return command;
}


void evg_opencl_command_free(struct evg_opencl_command_t *command)
{
	free(command);
}


void evg_opencl_command_queue_submit(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_t *command)
{
	struct linked_list_t *command_list;

	/* Check that command is not enqueued */
	command_list = command_queue->command_list;
	linked_list_find(command_list, command);
	if (!command_list->error_code)
		fatal("%s: command already enqueued", __FUNCTION__);
	
	/* Enqueue command */
	linked_list_add(command_list, command);
}


void evg_opencl_command_queue_complete(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_t *command)
{
	struct linked_list_t *command_list;

	/* Check that command is in command queue */
	command_list = command_queue->command_list;
	linked_list_find(command_list, command);
	if (command_list->error_code)
		fatal("%s: command is not in command queue", __FUNCTION__);
	
	/* Remove command */
	linked_list_remove(command_list);

	/* x86 contexts might be waiting for the command queue to get empty
	 * (e.g., suspended in a 'clFinish' call. Check events. */
	X86EmuProcessEventsSchedule(x86_emu);
}


int evg_opencl_command_queue_can_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_command_queue_t *command_queue;
	struct linked_list_t *command_list;

	int can_wakeup;

	/* x86 context can wakeup if command list is empty */
	command_queue = data;
	command_list = command_queue->command_list;
	can_wakeup = !command_list->count;

	/* Debug */
	if (can_wakeup)
		evg_opencl_debug("\tcommand queue 0x%x empty"
			" - context resumed\n", command_queue->id);

	/* Return */
	return can_wakeup;
}

