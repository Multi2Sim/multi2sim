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

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <lib/util/list.h>

#include "dram-system.h"
#include "dram.h"
#include "rank.h"
#include "bank.h"
#include "request.h"
#include "command.h"
#include "controller.h"
#include "action.h"


int EV_DRAM_ACTION;


static void dram_action_expect(struct list_t *token_list, char *action_line)
{
	if (!list_count(token_list))
		fatal("%s: unexpected end of line.\n\t> %s",
				__FUNCTION__, action_line);
}


static long long dram_action_get_llint(struct list_t *token_list,
				       char *action_line, char *expected)
{
	int err;
	long long request_cycle;

	/* Read cycle */
	dram_action_expect(token_list, action_line);
	request_cycle = str_to_llint(str_token_list_first(token_list), &err);
	if (err || request_cycle < 1)
		fatal("%s: %s: invalid %s , integer >= 1 expected.\n\t> %s",
				__FUNCTION__, str_token_list_first(token_list),
				action_line, expected);

	/* Shift token and return */
	str_token_list_shift(token_list);
	return request_cycle;
}


static long long dram_action_get_int(struct list_t *token_list,
				       char *action_line, char *expected)
{
	int err;
	int result;

	/* Read int */
	dram_action_expect(token_list, action_line);
	result = str_to_int(str_token_list_first(token_list), &err);
	if (err || result < 0)
		fatal("%s: %s: invalid %s , integer >= 0 expected.\n\t> %s",
				__FUNCTION__, str_token_list_first(token_list),
				action_line, expected);

	/* Shift token and return */
	str_token_list_shift(token_list);
	return result;
}


static void dram_action_get_string(struct list_t *token_list,
				   char *action_line, char *buf, int size)
{
	dram_action_expect(token_list, action_line);
	snprintf(buf, size, "%s", str_token_list_first(token_list));
	str_token_list_shift(token_list);
}


static unsigned int dram_action_get_hex_address(struct list_t *token_list,
		char *action_line)
{
	unsigned int address;

	/* Read hex uint */
	dram_action_expect(token_list, action_line);
	sscanf(str_token_list_first(token_list), "%x", &address);

	/* Shift token and return */
	str_token_list_shift(token_list);
	return address;
}




void dram_action_handler(int event, void *data)
{
	/* Load stack and data from it */
	struct dram_action_stack_t *stack = data;
	struct dram_system_t *system = stack->system;
	char *action_line = stack->action;

	long long cycle = esim_domain_cycle(dram_domain_index);

	if (event == EV_DRAM_ACTION)
	{
		char action[MAX_STRING_SIZE];

		/* Load tokens from action line */
		struct list_t *token_list;
		token_list = str_token_list_create(action_line, " ");

		/* Get the cycle the action is scheduled for */
		long long action_cycle = dram_action_get_llint(token_list,
				action_line, "cycle value");

		if (action_cycle > cycle)
		{
			/* Delay an action to its scheduled cycle */
			str_token_list_free(token_list);
			esim_schedule_event(event, stack, action_cycle - cycle);
			return;
		}

		/* Get the type of action */
		dram_action_get_string(token_list, action_line, action, sizeof(action));

		if (!strcasecmp(action, "test"))
		{
			fprintf(stderr, "test %lld\n", cycle);
		}

		if (!strcasecmp(action, "SetActiveRow"))
		{
			int logical_chan;
			int physical_chan;
			int rank;
			int bank;
			int row;
			struct dram_controller_t *controller;
			struct dram_bank_info_t *info;

			/* Get all the address info for the action */
			logical_chan = dram_action_get_int(token_list,
				action_line, "logical channel");
			physical_chan = dram_action_get_int(token_list,
				action_line, "physical channel");
			rank = dram_action_get_int(token_list,
				action_line, "rank");
			bank = dram_action_get_int(token_list,
				action_line, "bank");
			row = dram_action_get_int(token_list,
				action_line, "row");

			/* Set the active row */
			controller = list_get(system->dram_controller_list, logical_chan);
			info = list_get(controller->dram_bank_info_list,
					bank + rank * controller->dram_num_banks_per_device +
					physical_chan * controller->dram_num_ranks *
					controller->dram_num_banks_per_device);
			info->row_buffer_valid = 1;
			info->active_row_id = row;

			fprintf(stderr, "Row %d at %d:%d:%d:%d activated at cycle %lld\n",
				row, logical_chan, physical_chan, rank, bank, cycle);
		}

		if (!strcasecmp(action, "Request"))
		{
			char request[MAX_STRING_SIZE];
			struct dram_request_t *dram_request;
			dram_request = dram_request_create();

			/* Get the type of request */
			dram_action_get_string(token_list, action_line, request, sizeof(request));
			if (!strcasecmp(request, "READ"))
			{
				dram_request->type = request_type_read;
			}
			else if (!strcasecmp(request, "WRITE"))
			{
				dram_request->type = request_type_write;
			}
			else
			{
				fatal("%s: invalid request type %s\n\t> %s",
					__FUNCTION__, request, action_line);
			}

			/* Create the request and add it to the system's queue*/
			dram_request->addr = dram_action_get_hex_address(token_list, action_line);
			dram_request->system = system;
			dram_request->id = system->request_count;
			system->request_count++;
			list_enqueue(system->dram_request_list, dram_request);

			fprintf(stderr, "Request %s created for 0x%x at cycle %lld\n",
				request, dram_request->addr, cycle);
		}

		if (!strcasecmp(action, "CheckCommandStart"))
		{
			char type[MAX_STRING_SIZE];
			enum dram_command_type_t command_type;
			int logical_chan;
			int physical_chan;
			int rank;
			int bank;
			struct dram_controller_t *controller;
			struct dram_bank_info_t *info;

			/* Get the type of command*/
			dram_action_get_string(token_list, action_line, type, sizeof(type));
			if (!strcasecmp(type, "READ"))
			{
				command_type = dram_command_read;
			}
			else if (!strcasecmp(type, "WRITE"))
			{
				command_type = dram_command_write;
			}
			else if (!strcasecmp(type, "PRECHARGE"))
			{
				command_type = dram_command_precharge;
			}
			else if (!strcasecmp(type, "ACTIVATE"))
			{
				command_type = dram_command_activate;
			}
			else
			{
				fatal("%s: invalid command type %s\n\t> %s",
					__FUNCTION__, type, action_line);
			}

			/* Get all the address info for the action */
			logical_chan = dram_action_get_int(token_list,
				action_line, "logical channel");
			physical_chan = dram_action_get_int(token_list,
				action_line, "physical channel");
			rank = dram_action_get_int(token_list,
				action_line, "rank");
			bank = dram_action_get_int(token_list,
				action_line, "bank");

			/* Load the dram_info object containing previous command info */
			controller = list_get(system->dram_controller_list, logical_chan);
			info = list_get(controller->dram_bank_info_list,
					bank + rank * controller->dram_num_banks_per_device +
					physical_chan * controller->dram_num_ranks *
					controller->dram_num_banks_per_device);

			/* Check whether or not the command was scheduled */
			if (info->dram_bank_info_last_scheduled_time_matrix[command_type] == cycle)
			{
				fprintf(stderr, "Command %s at %d:%d:%d:%d was started at cycle %lld\n",
					type, logical_chan, physical_chan, rank, bank, cycle);
			}
			else
			{
				fprintf(stderr, "Command %s at %d:%d:%d:%d was NOT started at cycle %lld\n",
					type, logical_chan, physical_chan, rank, bank, cycle);
			}
		}

		str_token_list_free(token_list);
		dram_action_stack_free(stack);
	}
}


struct dram_action_stack_t *dram_action_stack_create(void)
{
	struct dram_action_stack_t *stack;
	stack = xcalloc(1, sizeof(struct dram_action_stack_t));
	return stack;
}


void dram_action_stack_free(struct dram_action_stack_t *action)
{
	free(action->action);
	free(action);
}
