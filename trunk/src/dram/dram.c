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

#include "dram.h"
#include "rank.h"
#include "bank.h"
#include "command.h"
#include "dram-system.h"


/*
 * Global Variables
 */

int EV_DRAM_COMMAND_RECEIVE;
int EV_DRAM_COMMAND_COMPLETE;


/*
 * Event handler
 */

void dram_event_handler(int event, void *data)
{
	struct dram_command_t *command = data;
	long long cycle;
	FILE *f;

	/* Get current cycle */
	cycle = esim_domain_cycle(dram_domain_index);


	if (event == EV_DRAM_COMMAND_RECEIVE)
	{
		/* Dump command */
		f = debug_file(dram_debug_category);
		if (f)
		{
			dram_debug("\tNew command in cycle %lld:\n", cycle);
			dram_command_dump(command, f);
		}

		/* Process command */
		switch (command->type)
		{

			case dram_command_nop:

				/* Schedule complete event */
				esim_schedule_event(EV_DRAM_COMMAND_COMPLETE, command, 0);
				break;

			case dram_command_refresh:

				/* Schedule complete event */
				esim_schedule_event(EV_DRAM_COMMAND_COMPLETE, command, 0);
				break;

			case dram_command_precharge:

				/* Schedule complete event */
				esim_schedule_event(EV_DRAM_COMMAND_COMPLETE, command, command->dram->timing_tRP);
				break;

			case dram_command_activate:

				/* Schedule complete event */
				esim_schedule_event(EV_DRAM_COMMAND_COMPLETE, command, command->dram->timing_tRCD);
				break;

			case dram_command_read:

				/* Schedule complete event */
				esim_schedule_event(EV_DRAM_COMMAND_COMPLETE, command, command->dram->timing_tCAS);
				break;

			case dram_command_write:

				/* Schedule complete event */
				esim_schedule_event(EV_DRAM_COMMAND_COMPLETE, command, command->dram->timing_tCWL);
				break;

			case dram_command_invalid:

				break;

		}
	}

	if (event == EV_DRAM_COMMAND_COMPLETE)
	{
		/* Update active row */
		switch (command->type)
		{

			case dram_command_precharge:

				/* Clear row_buffer_valid */
				command->dram->rank_array[command->u.precharge.rank_id]->bank_array[command->u.precharge.bank_id]->row_buffer_valid = 0;
				break;

			case dram_command_activate:

				/* Set row_buffer_valid */
				command->dram->rank_array[command->u.activate.rank_id]->bank_array[command->u.activate.bank_id]->row_buffer_valid = 1;
				command->dram->rank_array[command->u.activate.rank_id]->bank_array[command->u.activate.bank_id]->active_row_id = command->u.activate.row_id;
				break;

			default:

				break;
		}

		/* Dump command */
		f = debug_file(dram_debug_category);
		if (f)
		{
			dram_debug("\tCommand freed in cycle %lld:\n", cycle);
			dram_command_dump(command, stdout);
		}

		/* Free command */
		dram_command_free(command);

		/* FIXME - this what happens if bus_can_send returns false. Next
		 * command could be a 'read' willing to return data through a
		 * bus that is still transferring the result of the previous
		 * 'read'.
		 * We shouldn't just reschedule the COMPLETE event when
		 * bus_can_send returns False, but also propagate contention
		 * backward to eventually make the memory controller aware of
		 * it. */
	}
}


/*
 * DRAM
 */



struct dram_t *dram_create(unsigned int num_ranks,
				unsigned int num_devices_per_rank,
				unsigned int num_banks_per_device,
				unsigned int num_rows_per_bank,
				unsigned int num_columns_per_row,
				unsigned int num_bits_per_column)
{
	struct dram_t *dram;
	int i;

	/* Initialize */
	dram = xcalloc(1, sizeof(struct dram_t));
	dram->num_ranks = num_ranks;
	dram->num_devices_per_rank = num_devices_per_rank;
	dram->num_banks_per_device = num_banks_per_device;
	dram->num_rows_per_bank = num_rows_per_bank;
	dram->num_columns_per_row = num_columns_per_row;
	dram->num_bits_per_column = num_bits_per_column;

	/* Initialize rank array */
	dram->rank_array = xcalloc(dram->num_ranks, sizeof(struct dram_rank_t *));
	for (i = 0; i < dram->num_ranks; i++)
		dram->rank_array[i] = dram_rank_create(dram);

	/* Return */
	return dram;
}


void dram_free(struct dram_t *dram)
{
	int i;

	/* Free all ranks */
	for (i = 0; i < dram->num_ranks; i++)
		dram_rank_free(dram->rank_array[i]);

	/* Free rank array */
	free(dram->rank_array);

	/* Free */
	free(dram);
}


void dram_dump(struct dram_t *dram, FILE *f)
{
	int i;

	/* Header */
	fprintf(f, "\nDumping DRAM structure:\n");

	/* Dump all ranks */
	for (i = 0; i < dram->num_ranks; i++)
	{
		fprintf(f, "Rank %d:\n", i);
		dram_rank_dump(dram->rank_array[i], f);
	}
	
	/* Last line */
	fprintf(f, "\n");
}
