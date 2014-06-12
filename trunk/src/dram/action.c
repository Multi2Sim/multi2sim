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




void dram_action_parse_action_line(struct dram_system_t *system, char *action_line)
{
	char action_name[MAX_STRING_SIZE];
	struct dram_action_t *dram_action;
	struct dram_action_stack_t *dram_stack;
	struct list_t *token_list;
	long long cycle;
	long long action_cycle;

	/* Initialize the action */
	dram_action = dram_action_create();
	dram_action->system = system;

	/* Load tokens from action line */
	token_list = str_token_list_create(action_line, " ");

	/* Get the current cycle */
	cycle = esim_domain_cycle(dram_domain_index);

	/* Get the cycle the action is scheduled for */
	action_cycle = dram_action_get_llint(token_list, action_line, "cycle");

	/* Get the type of action */
	dram_action_get_string(token_list, action_line, action_name, sizeof(action_name));
	if (!strcasecmp(action_name, "SetActiveRow"))
	{
		/* Set the action type */
		dram_action->type = action_type_set_active_row;

		/* Get all the address info for the action */
		dram_action->set_active_row.logical_chan = dram_action_get_int(
			token_list, action_line, "logical channel");
		dram_action->set_active_row.physical_chan = dram_action_get_int(
			token_list, action_line, "physical channel");
		dram_action->set_active_row.rank = dram_action_get_int(
			token_list, action_line, "rank");
		dram_action->set_active_row.bank = dram_action_get_int(
			token_list, action_line, "bank");
		dram_action->set_active_row.row = dram_action_get_int(
			token_list, action_line, "row");
	}

	else if (!strcasecmp(action_name, "Request"))
	{
		char type[MAX_STRING_SIZE];

		/* Set the action type */
		dram_action->type = action_type_request;

		/* Get the type of request */
		dram_action_get_string(token_list, action_line, type, sizeof(type));
		strcpy(dram_action->request.type_str, type);
		if (!strcasecmp(type, "READ"))
		{
			dram_action->request.type = request_type_read;
		}
		else if (!strcasecmp(type, "WRITE"))
		{
			dram_action->request.type = request_type_write;
		}
		else
		{
			fatal("%s: invalid request type %s\n\t> %s",
				__FUNCTION__, type, action_line);
		}

		/* Create the request and add it to the system's queue*/
		dram_action->request.addr = dram_action_get_hex_address(token_list, action_line);
	}

	else if (!strcasecmp(action_name, "CheckCommandStart"))
	{
		char type[MAX_STRING_SIZE];

		/* Set the action type */
		dram_action->type = action_type_check_command_start;

		/* Get the type of command*/
		dram_action_get_string(token_list, action_line, type, sizeof(type));
		strcpy(dram_action->request.type_str, type);
		if (!strcasecmp(type, "READ"))
		{
			dram_action->check_command_start.type = dram_command_read;
		}
		else if (!strcasecmp(type, "WRITE"))
		{
			dram_action->check_command_start.type = dram_command_write;
		}
		else if (!strcasecmp(type, "PRECHARGE"))
		{
			dram_action->check_command_start.type = dram_command_precharge;
		}
		else if (!strcasecmp(type, "ACTIVATE"))
		{
			dram_action->check_command_start.type = dram_command_activate;
		}
		else
		{
			fatal("%s: invalid command type %s\n\t> %s",
				__FUNCTION__, type, action_line);
		}

		/* Get all the address info for the action */
		dram_action->check_command_start.logical_chan = dram_action_get_int(
			token_list, action_line, "logical channel");
		dram_action->check_command_start.physical_chan = dram_action_get_int(
			token_list, action_line, "physical channel");
		dram_action->check_command_start.rank = dram_action_get_int(
			token_list, action_line, "rank");
		dram_action->check_command_start.bank = dram_action_get_int(
			token_list, action_line, "bank");
	}

	else
	{
		/* Unknown action, fail */
		fatal("%s: Unknown action %s\n\t>%s",
			__FUNCTION__, action_name, action_line);
	}

	/* Build a stack for the action's event */
	dram_stack = dram_action_stack_create();
	dram_stack->action = dram_action;
	dram_stack->cycle = action_cycle;

	/* Delay an action to its scheduled cycle */
	esim_schedule_event(EV_DRAM_ACTION, dram_stack, action_cycle - cycle);

	str_token_list_free(token_list);
}


void dram_action_handler(int event, void *data)
{
	/* Load stack and data from it */
	long long cycle = esim_domain_cycle(dram_domain_index);
	struct dram_action_stack_t *stack = data;
	struct dram_action_t *dram_action = stack->action;
	struct dram_system_t *system = dram_action->system;
	FILE *f;

	if (event == EV_DRAM_ACTION)
	{
		if (dram_action->type == action_type_set_active_row)
		{
			struct dram_controller_t *controller;
			struct dram_bank_info_t *info;

			/* Set the active row */
			controller = list_get(system->dram_controller_list,
				dram_action->set_active_row.logical_chan);
			info = list_get(controller->dram_bank_info_list,
					dram_action->set_active_row.bank +
					dram_action->set_active_row.rank *
					controller->dram_num_banks_per_device +
					dram_action->set_active_row.physical_chan *
					controller->dram_num_ranks *
					controller->dram_num_banks_per_device);
			info->row_buffer_valid = 1;
			info->active_row_id = dram_action->set_active_row.row;

			f = debug_file(dram_debug_category);
			fprintf(f, "\tRow %d at %d:%d:%d:%d activated in cycle %lld\n",
				dram_action->set_active_row.row,
				dram_action->set_active_row.logical_chan,
				dram_action->set_active_row.physical_chan,
				dram_action->set_active_row.rank,
				dram_action->set_active_row.bank,
				cycle);
		}

		else if (dram_action->type == action_type_request)
		{
			struct dram_request_t *dram_request;
			dram_request = dram_request_create();

			/* Set the request type and address*/
			dram_request->type = dram_action->request.type;
			dram_request->addr = dram_action->request.addr;

			/* Create the request and add it to the system's queue*/
			dram_request->system = system;
			dram_request->id = system->request_count;
			system->request_count++;
			list_enqueue(system->dram_request_list, dram_request);

			f = debug_file(dram_debug_category);
			fprintf(f, "\tRequest %s created for 0x%x in cycle %lld\n",
				dram_action->request.type_str,
				dram_request->addr,
				cycle);
		}

		else if (dram_action->type == action_type_check_command_start)
		{
			struct dram_controller_t *controller;
			struct dram_bank_info_t *info;

			/* Load the dram_info object containing previous command info */
			controller = list_get(system->dram_controller_list,
				dram_action->check_command_start.logical_chan);
			info = list_get(controller->dram_bank_info_list,
					dram_action->check_command_start.bank +
					dram_action->check_command_start.rank *
					controller->dram_num_banks_per_device +
					dram_action->check_command_start.physical_chan *
					controller->dram_num_ranks *
					controller->dram_num_banks_per_device);

			/* Check whether or not the command was scheduled */
			if (info->dram_bank_info_last_scheduled_time_matrix[dram_action->check_command_start.type] == cycle)
			{
				f = debug_file(dram_debug_category);
				fprintf(f, "\tCommand %s at %d:%d:%d:%d was started in cycle %lld\n",
					dram_action->check_command_start.type_str,
					dram_action->check_command_start.logical_chan,
					dram_action->check_command_start.physical_chan,
					dram_action->check_command_start.rank,
					dram_action->check_command_start.bank,
					cycle);
			}
			else
			{
				fprintf(stderr, "Command %s at %d:%d:%d:%d was NOT started in cycle %lld\n",
					dram_action->check_command_start.type_str,
					dram_action->check_command_start.logical_chan,
					dram_action->check_command_start.physical_chan,
					dram_action->check_command_start.rank,
					dram_action->check_command_start.bank,
					cycle);
				fatal("Command not started");
			}
		}

		dram_action_free(dram_action);
		dram_action_stack_free(stack);
	}
}


struct dram_action_t *dram_action_create(void)
{
	struct dram_action_t *dram_action;
	dram_action = xcalloc(1, sizeof(struct dram_action_t));
	return dram_action;
}


void dram_action_free(struct dram_action_t *dram_action)
{
	// if (dram_action->type == action_type_request)
	// {
	// 	free(dram_action->request.type_str);
	// }
	// else if (dram_action->type == action_type_check_command_start)
	// {
	// 	free(dram_action->request.type_str);
	// }

	free(dram_action);
}


struct dram_action_stack_t *dram_action_stack_create(void)
{
	struct dram_action_stack_t *stack;
	stack = xcalloc(1, sizeof(struct dram_action_stack_t));
	return stack;
}


void dram_action_stack_free(struct dram_action_stack_t *stack)
{
	free(stack);
}
