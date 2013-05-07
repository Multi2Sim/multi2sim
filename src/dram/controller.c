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


#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "dram.h"
#include "command.h"
#include "request.h"
#include "controller.h"
#include "dram-system.h"


/*
 * Global Variable
 */

long long dram_controller_max_cycles = 50;


/*
 * Dram bank info
 */

struct dram_bank_info_t *dram_bank_info_create(unsigned int channel_id,
						unsigned int rank_id,
						unsigned int bank_id,
						unsigned int request_queue_depth)
{
	struct dram_bank_info_t *info;

	/* Initialize */
	info = xcalloc(1, sizeof(struct dram_bank_info_t));
	info->channel_id = channel_id;
	info->rank_id = rank_id;
	info->bank_id = bank_id;
	info->request_queue_depth = request_queue_depth;

	/* Create request queue */
	info->request_queue = list_create_with_size(request_queue_depth);

	/* Create command queue */
	info->command_queue = list_create();

	/* Return */
	return info;
}


void dram_bank_info_free(struct dram_bank_info_t *info)
{
	int i, num_requests, num_commands;

	/* Free command queue */
	num_commands = list_count(info->command_queue);
	for (i = 0; i < num_commands; i++)
		dram_command_free(list_get(info->command_queue, i));
	list_free(info->command_queue);

	/* Free request queue */
	num_requests = list_count(info->request_queue);
	for (i = 0; i < num_requests; i++)
		dram_request_free(list_get(info->request_queue, i));
	list_free(info->request_queue);

	/* Free */
	free(info);
}


void dram_bank_info_dump(struct dram_bank_info_t *info, FILE *f)
{
	int i, num_requests, num_commands;

	/* Dump IDs */
	fprintf(f, "\tChannel %u Rank %u Bank %u:\n",
	 info->channel_id, info->rank_id, info->bank_id);

	/* Dump bank status */
	if(info->row_buffer_valid)
		fprintf(f, "\tActive row ID: %u\n", info->active_row_id);
	else
		fprintf(f, "\tNo active row\n");

	/* Dump request queue content */
	num_requests = list_count(info->request_queue);
	fprintf(f, "\t%u requests in queue:\n", num_requests);
	for (i = 0; i < num_requests; i++)
		dram_request_dump(list_get(info->request_queue, i), f);

	/* Dump command queue content */
	num_commands = list_count(info->command_queue);
	fprintf(f, "\t%u commands in queue:\n", num_commands);
	for (i = 0; i < num_commands; i++)
		dram_command_dump(list_get(info->command_queue, i), f);
}


/*
 * DRAM bus scheduler
 */

struct dram_command_scheduler_t *dram_command_scheduler_create(unsigned int channel_id)
{
	struct dram_command_scheduler_t *scheduler;

	/* Initialize */
	scheduler = xcalloc(1, sizeof(struct dram_command_scheduler_t));
	scheduler->channel_id = channel_id;

	/* Return */
	return scheduler;
}


void dram_command_scheduler_free(struct dram_command_scheduler_t *scheduler)
{
	/* Free */
	free(scheduler);
}


void dram_command_scheduler_dump(struct dram_command_scheduler_t *scheduler, FILE *f)
{
	/* Dump */
	fprintf(f, "Channel %u, last scheduled: rank %u bank %u\n",
	scheduler->channel_id,
	scheduler->last_scheduled_rank_id,
	scheduler->last_scheduled_bank_id);
}


/*
 * Controller
 */

static unsigned int dram_timing_matrix[DRAM_TIMING_MATRIX_SIZE][DRAM_TIMING_MATRIX_SIZE] =
{
	{1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 24, 7, 7, 1},
	{1, 1, 9, 1, 1, 1, 1},
	{1, 1, 1, 9, 4, 4, 1},
	{1, 1, 1, 9, 4, 4, 1},
	{1, 1, 1, 1, 1, 1, 1}
};


struct dram_controller_t *dram_controller_create(unsigned int dram_controller_request_queue_depth,
							enum dram_controller_row_buffer_policy_t rb_policy,
							enum dram_controller_scheduling_policy_t scheduling_policy)
{
	struct dram_controller_t *controller;

	/* Initialize */
	controller = xcalloc(1, sizeof(struct dram_controller_t));
	controller->request_queue_depth = dram_controller_request_queue_depth;
	controller->rb_policy = rb_policy;
	controller->scheduling_policy = scheduling_policy;

	/* Initialize timing matrix to default values */
	memcpy(controller->dram_timing_matrix, dram_timing_matrix,
			sizeof dram_timing_matrix);

	/* Create dram list */
	controller->dram_list = list_create();

	/* Create bank info list */
	controller->dram_bank_info_list = list_create();

	/* Create command scheduler list */
	controller->dram_command_scheduler_list = list_create();

	/* Return */
	return controller;
}


void dram_controller_free(struct dram_controller_t *controller)
{
	int i, n;

	/* Free bus command queue */
	n = list_count(controller->dram_command_scheduler_list);
	for (i = 0; i < n; i++)
		dram_command_scheduler_free(list_get(controller->dram_command_scheduler_list, i));
	list_free(controller->dram_command_scheduler_list);

	/* Free bank info list*/
	n = list_count(controller->dram_bank_info_list);
	for (i = 0; i < n; i++)
		dram_bank_info_free(list_get(controller->dram_bank_info_list, i));
	list_free(controller->dram_bank_info_list);

	/* Free dram list */
	for (i = 0; i < controller->num_physical_channels; i++)
		dram_free(list_get(controller->dram_list, i));
	list_free(controller->dram_list);

	/* Free */
	free(controller);
}


void dram_controller_dump(struct dram_controller_t *controller, FILE *f)
{
	int i, num_bank_info;

	/* Dump controller info */
	fprintf(f, "Dumping controller %u:\n", controller->id);
	fprintf(f, "Number of physical channels: %u\n", controller->num_physical_channels);

	/* FIXME - Dump all parameters */

	/* Dump DRAM */
	/* ... */

	/* Dump bank info */
	num_bank_info = list_count(controller->dram_bank_info_list);
	for (i = 0; i < num_bank_info; i++)
		dram_bank_info_dump(list_get(controller->dram_bank_info_list, i), f);
}


int dram_controller_add_dram(struct dram_controller_t *controller, struct dram_t *dram)
{
	int i, j, k;

	/* Validate input */
	if (!dram)
		panic("%s: dram object not valid", __FUNCTION__);

	/* Assign parameters in controller */
	controller->num_physical_channels++;
	controller->dram_num_ranks = dram->num_ranks;
	controller->dram_num_devices_per_rank = dram->num_devices_per_rank;
	controller->dram_num_banks_per_device = dram->num_banks_per_device;
	controller->dram_num_rows_per_bank = dram->num_rows_per_bank;
	controller->dram_num_columns_per_row = dram->num_columns_per_row;
	controller->dram_num_bits_per_column = dram->num_bits_per_column;

	/* Add dram to list */
	list_add(controller->dram_list, dram);

	/* Create dram bank info list */
	k = controller->num_physical_channels - 1;
	for (i = 0; i < controller->dram_num_ranks; i++)
	{
		for (j = 0; j < controller->dram_num_banks_per_device; j++)
		{
			struct dram_bank_info_t *info;
			info = dram_bank_info_create(k, i, j, controller->request_queue_depth);
			list_add(controller->dram_bank_info_list, info);
		}
	}

	/* Create command scheduler */
	struct dram_command_scheduler_t *scheduler;
	scheduler = dram_command_scheduler_create(k);
	list_add(controller->dram_command_scheduler_list, scheduler);

	/* Return */
	return 1;
}


int dram_controller_get_request(struct dram_controller_t *controller, struct dram_request_t *request)
{
	int i;
	unsigned int rank_id, bank_id, physical_channel_id;
	struct dram_bank_info_t *info;

	/* Do nothing if pointer is not passed*/
	if (!request || !controller)
		return 0;

	/* Decode address*/
	dram_decode_address(request->system, request->addr,
			NULL, &rank_id, NULL, &bank_id, NULL, &physical_channel_id);

	/* Locate bank info */
	i = (physical_channel_id) * (controller->dram_num_ranks) * (controller->dram_num_banks_per_device) + (rank_id) * (controller->dram_num_banks_per_device) + (bank_id);
	info = list_get(controller->dram_bank_info_list, i);
	list_add(info->request_queue, request);

	return 1;
}


void dram_controller_process_request(struct dram_controller_t *controller)
{
	int i;
	unsigned int num_bank_info;
	unsigned int row_id, column_id;
	struct dram_request_t *request;
	struct dram_bank_info_t *info;
	struct dram_command_t *command_precharge;
	struct dram_command_t *command_activate;
	struct dram_command_t *command_access;

	/* Go through bank info list */
	num_bank_info = list_count(controller->dram_bank_info_list);
	for (i = 0; i < num_bank_info; i++)
	{
		/* Locate bank info */
		info = list_get(controller->dram_bank_info_list, i);

		/* Fetch a request from request queue */
		request = list_dequeue(info->request_queue);

		if (request)
		{
			dram_decode_address(request->system, request->addr, NULL, NULL,
						&row_id, NULL, &column_id, NULL);

			/* Determine policy to be used */
			switch (controller->rb_policy)
			{

				/* Open page row buffer policy */
				case open_page_row_buffer_policy:

					/* Row hit */
					if (info->row_buffer_valid && info->active_row_id == row_id)
					{
						/* Create single read/write command */
						command_access = dram_command_create();

						command_access->dram = list_get(controller->dram_list, info->channel_id);
						if (request->type == request_type_read)
						{
							command_access->type = dram_command_read;
							command_access->u.read.rank_id = info->rank_id;
							command_access->u.read.bank_id = info->bank_id;
							command_access->u.read.column_id = column_id;
						}
						else if (request->type == request_type_write)
						{
							command_access->type = dram_command_write;
							command_access->u.write.rank_id = info->rank_id;
							command_access->u.write.bank_id = info->bank_id;
							command_access->u.write.column_id = column_id;
						}

						/* Append command to queue */
						list_add(info->command_queue, command_access);
					}
					/* Row miss - different row was activated */
					else if (info->row_buffer_valid)
					{
						/* Create pre, act, rd/wr three commands */
						command_precharge = dram_command_create();
						command_activate = dram_command_create();
						command_access = dram_command_create();

						/* Precharge */
						command_precharge->type = dram_command_precharge;
						command_precharge->dram = list_get(controller->dram_list, info->channel_id);
						command_precharge->u.precharge.rank_id = info->rank_id;
						command_precharge->u.precharge.bank_id = info->bank_id;

						/* Activate */
						command_activate->type = dram_command_activate;
						command_activate->dram = list_get(controller->dram_list, info->channel_id);
						command_activate->u.activate.rank_id = info->rank_id;
						command_activate->u.activate.bank_id = info->bank_id;
						command_activate->u.activate.row_id = row_id;

						/* Access */
						command_access->dram = list_get(controller->dram_list, info->channel_id);
						if (request->type == request_type_read)
						{
							command_access->type = dram_command_read;
							command_access->u.read.rank_id = info->rank_id;
							command_access->u.read.bank_id = info->bank_id;
							command_access->u.read.column_id = column_id;
						}
						else if (request->type == request_type_write)
						{
							command_access->type = dram_command_write;
							command_access->u.write.rank_id = info->rank_id;
							command_access->u.write.bank_id = info->bank_id;
							command_access->u.write.column_id = column_id;
						}

						list_add(info->command_queue, command_precharge);
						list_add(info->command_queue, command_activate);
						list_add(info->command_queue, command_access);

						/* Update the active row status */
						info->active_row_id = row_id;
					}
					/* Row miss - no row was activated */
					else
					{
						/* Create act, rd/wr two commands */
						command_activate = dram_command_create();
						command_access = dram_command_create();

						/* Activate */
						command_activate->type = dram_command_activate;
						command_activate->dram = list_get(controller->dram_list, info->channel_id);
						command_activate->u.activate.rank_id = info->rank_id;
						command_activate->u.activate.bank_id = info->bank_id;
						command_activate->u.activate.row_id = row_id;

						/* Access */
						command_access->dram = list_get(controller->dram_list, info->channel_id);
						if (request->type == request_type_read)
						{
							command_access->type = dram_command_read;
							command_access->u.read.rank_id = info->rank_id;
							command_access->u.read.bank_id = info->bank_id;
							command_access->u.read.column_id = column_id;
						}
						else if (request->type == request_type_write)
						{
							command_access->type = dram_command_write;
							command_access->u.write.rank_id = info->rank_id;
							command_access->u.write.bank_id = info->bank_id;
							command_access->u.write.column_id = column_id;
						}

						list_add(info->command_queue, command_activate);
						list_add(info->command_queue, command_access);

						/* Update the active row status */
						info->row_buffer_valid = 1;
						info->active_row_id = row_id;
					}

					break;

				/* Close page row buffer policy */
				case close_page_row_buffer_policy:

					/* Create act, rd/wr, pre three commands */
					command_activate = dram_command_create();
					command_access = dram_command_create();
					command_precharge = dram_command_create();

					/* Activate */
					command_activate->type = dram_command_activate;
					command_activate->dram = list_get(controller->dram_list, info->channel_id);
					command_activate->u.activate.rank_id = info->rank_id;
					command_activate->u.activate.bank_id = info->bank_id;
					command_activate->u.activate.row_id = row_id;

					/* Access */
					command_access->dram = list_get(controller->dram_list, info->channel_id);
					if (request->type == request_type_read)
					{
						command_access->type = dram_command_read;
						command_access->u.read.rank_id = info->rank_id;
						command_access->u.read.bank_id = info->bank_id;
						command_access->u.read.column_id = column_id;
					}
					else if (request->type == request_type_write)
					{
						command_access->type = dram_command_write;
						command_access->u.write.rank_id = info->rank_id;
						command_access->u.write.bank_id = info->bank_id;
						command_access->u.write.column_id = column_id;
					}

					/* Precharge */
					command_precharge->type = dram_command_precharge;
					command_precharge->dram = list_get(controller->dram_list, info->channel_id);
					command_precharge->u.precharge.rank_id = info->rank_id;
					command_precharge->u.precharge.bank_id = info->bank_id;

					list_add(info->command_queue, command_activate);
					list_add(info->command_queue, command_access);
					list_add(info->command_queue, command_precharge);

					/* Update the active row status */
					info->row_buffer_valid = 0;

					break;

				case hybird_page_row_buffer_policy:

			
					break;

				default:

					break;

			}

			dram_request_free(request);
		}
	}
}


void dram_controller_schedule_command(struct dram_controller_t *controller)
{
	int i;
	int j;
	int k;
	int num_info_per_scheduler;
	int valid;

	struct dram_command_t *command;
	struct dram_bank_info_t *info;
	struct dram_command_scheduler_t *scheduler;

	long long cycle;

	cycle = esim_domain_cycle(dram_domain_index);
	num_info_per_scheduler = controller->dram_num_ranks * controller->dram_num_banks_per_device;

	for (i = 0; i < controller->num_physical_channels; i++)
	{
		/* Get scheduler */
		scheduler = list_get(controller->dram_command_scheduler_list, i);

		for (j = 0; j < num_info_per_scheduler; j++)
		{

			switch (controller->scheduling_policy)
			{

				case rank_bank_round_robin:

					/* Locate bank info */
					scheduler->last_scheduled_rank_id++;
					if (scheduler->last_scheduled_rank_id == controller->dram_num_ranks)
					{
						scheduler->last_scheduled_rank_id = 0;
						scheduler->last_scheduled_bank_id ++;
						if (scheduler->last_scheduled_bank_id == controller->dram_num_banks_per_device)
							scheduler->last_scheduled_bank_id = 0;
					}

					break;

				case bank_rank_round_robin:

					/* Locate bank info */
					scheduler->last_scheduled_bank_id++;
					if (scheduler->last_scheduled_bank_id == controller->dram_num_banks_per_device)
					{
						scheduler->last_scheduled_bank_id = 0;
						scheduler->last_scheduled_rank_id ++;
						if (scheduler->last_scheduled_rank_id == controller->dram_num_ranks)
							scheduler->last_scheduled_rank_id = 0;
					}

					break;

				default:

					break;
			}

			info = list_get(controller->dram_bank_info_list,
					scheduler->last_scheduled_bank_id +
					scheduler->last_scheduled_rank_id *
					controller->dram_num_banks_per_device +
					scheduler->channel_id * controller->dram_num_ranks *
					controller->dram_num_banks_per_device);

			/* Fetch a command from command queue */
			command = list_head(info->command_queue);
			if (command)
			{
				/* Check timing */
				valid = 1;
				for (k = 0; k < DRAM_TIMING_MATRIX_SIZE; k++)
				{
					if (cycle - info->dram_bank_info_last_scheduled_time_matrix[k]
							< controller->dram_timing_matrix[command->type][k])
						valid = 0;
				}

				/* If timing is valid, schedule the command */
				if (valid)
				{
					/* Dequeue command */
					command = list_dequeue(info->command_queue);

					/* Schedule command receive */
					esim_schedule_event(EV_DRAM_COMMAND_RECEIVE, command, 0);

					/* Update last scheduled time matrix */
					info->dram_bank_info_last_scheduled_time_matrix[command->type] = cycle;
				}
			}
		}
	}
}
