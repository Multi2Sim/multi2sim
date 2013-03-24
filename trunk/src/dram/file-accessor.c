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
#include <lib/util/string.h>

#include "dram.h"
#include "file-accessor.h"
#include "request.h"
#include "command.h"


/*
 * request_file_accessor
 */

struct request_file_accessor *request_file_accessor_create(void)
{
	struct request_file_accessor *accessor;

	/* Initialize */
	accessor = xcalloc(1,sizeof(struct request_file_accessor));
	accessor->f = fopen("requests.txt", "r");
	if (!accessor->f)
		fatal("%s: Cannot open requests.txt", __FUNCTION__);

	/* Return */
	return accessor;
}


void request_file_accessor_free(struct request_file_accessor *accessor)
{
	/* Free */
	if(fclose(accessor->f))
		fatal("%s: Error closing file", __FUNCTION__);
	free(accessor);
}


struct dram_request_t *request_file_accessor_get(struct request_file_accessor *accessor)
{
	struct list_t *token_list;
	long long cycle;
	char *request_str;
	char line[MAX_STRING_SIZE];
	char *line_ptr;

	
	struct dram_request_t *request;

	/* Check if there is cached request */
	if (accessor->cached_request)
	{
		/* Return cached request if it belongs to current cycle */
		if (accessor->cached_request_cycle == esim_cycle)
		{
			struct dram_request_t *cached_request;

			cached_request = accessor->cached_request;
			accessor->cached_request = NULL;
			return cached_request;
		}
		/* Return NULL */
		else
			return NULL;
	}

	/* Read a new line */
	line_ptr = fgets(line, MAX_STRING_SIZE, accessor->f);
	if (!line_ptr)
		return NULL;

	/* Create request */
	request = dram_request_create();

	/* Split line into tokens */
	token_list = str_token_list_create(line, " \t\n");
	if (list_count(token_list) != 3)
		fatal("%s: Invalid request: Expecting 3 arguments", __FUNCTION__);

	/* Read cycle */
	cycle = atoll(str_token_list_first(token_list));
	if (cycle < esim_cycle)
		fatal("%s: Invalid request: Invalid cycle number", __FUNCTION__);

	/* Read request type */
	str_token_list_shift(token_list);
	request_str = str_token_list_first(token_list);
	if (!strcmp(request_str, "READ"))
	{
		request->type = request_type_read;
	}
	else if (!strcmp(request_str, "WRITE"))
	{
		request->type = request_type_write;
	}

	/* Read Address */
	str_token_list_shift(token_list);
	sscanf(str_token_list_first(token_list), "%x", &request->addr);

	/* Assign request ID */
	request->id = ++accessor->request_id_counter;

	/* Free token list */
	str_token_list_free(token_list);

	/* If it's current cycle, return the request */
	if (cycle == esim_cycle)
		return request;

	/* else store the request in cache */
	else
	{
		accessor->cached_request_cycle = cycle;
		accessor->cached_request = request;
		return NULL;
	}

	return NULL;
}


/*
 * command_file_accessor
 */

struct command_file_accessor *command_file_accessor_create(void)
{
	struct command_file_accessor *accessor;

	/* Initialize */
	accessor = xcalloc(1,sizeof(struct command_file_accessor));
	accessor->f = fopen("commands.txt", "r");
	if (!accessor->f)
		fatal("%s: Cannot open commands.txt", __FUNCTION__);

	/* Return */
	return accessor;
}


void command_file_accessor_free(struct command_file_accessor *accessor)
{
	/* Free */
	if(fclose(accessor->f))
		fatal("%s: Error closing file", __FUNCTION__);
	free(accessor);
}


/* Function that returns an allocated DRAM command object corresponding to
 * the next entry of the command sequence, still belonging to the current
 * cycle. If there are no more commands in this cycle, return NULL. */
struct dram_command_t *command_file_accessor_get(struct command_file_accessor *accessor, struct dram_t *dram)
{
	struct list_t *token_list;
	long long cycle;
	char *command_str;
	char line[MAX_STRING_SIZE];
	char *line_ptr;
	
	struct dram_command_t *command;


	/* Check if there is cached command */
	if (accessor->cached_command)
	{
		/* Return cached command if it belongs to current cycle */
		if (accessor->cached_command_cycle == esim_cycle)
		{
			struct dram_command_t *cached_command;

			cached_command = accessor->cached_command;
			accessor->cached_command = NULL;
			return cached_command;
		}
		/* Return NULL */
		else
			return NULL;
	}

	/* Read a new line */
	line_ptr = fgets(line, MAX_STRING_SIZE, accessor->f);
	if (!line_ptr)
		return NULL;

	/* Create command */
	command = dram_command_create();
	
	/* Pass dram pointer */
	command->dram = dram;

	/* Split line into tokens */
	token_list = str_token_list_create(line, " \t\n");
	if (list_count(token_list) < 2)
		fatal("%s: Invalid command: Expecting more arguments", __FUNCTION__);

	/* Read cycle */
	cycle = atoll(str_token_list_first(token_list));
	if (cycle < esim_cycle)
		fatal("%s: Invalid command: Invalid cycle number", __FUNCTION__);

	/* Read command */
	str_token_list_shift(token_list);
	command_str = str_token_list_first(token_list);
	if (!strcmp(command_str, "nop"))
	{
		command->type = dram_command_nop;
	}
	else if (!strcmp(command_str, "ref"))
	{
		command->type = dram_command_refresh;
		str_token_list_shift(token_list);
		command->u.refresh.rank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.refresh.bank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.refresh.row_id = atoll(str_token_list_first(token_list));
	}
	else if (!strcmp(command_str, "pre"))
	{
		command->type = dram_command_precharge;
		str_token_list_shift(token_list);
		command->u.precharge.rank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.precharge.bank_id = atoll(str_token_list_first(token_list));
	}
	else if (!strcmp(command_str, "act"))
	{
		command->type = dram_command_activate;
		str_token_list_shift(token_list);
		command->u.activate.rank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.activate.bank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.activate.row_id = atoll(str_token_list_first(token_list));
	}
	else if (!strcmp(command_str, "rd"))
	{
		command->type = dram_command_read;
		str_token_list_shift(token_list);
		command->u.read.rank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.read.bank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.read.column_id = atoll(str_token_list_first(token_list));
	}
	else if (!strcmp(command_str, "wr"))
	{
		command->type = dram_command_write;
		str_token_list_shift(token_list);
		command->u.write.rank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.write.bank_id = atoll(str_token_list_first(token_list));
		str_token_list_shift(token_list);
		command->u.write.column_id = atoll(str_token_list_first(token_list));
	}

	/* Assign command ID */
	command->id = ++accessor->command_id_counter;

	/* Free token list */
	str_token_list_free(token_list);

	/* If it's current cycle, return the command */
	if (cycle == esim_cycle)
		return command;

	/* else store the command in cache */
	else
	{
		accessor->cached_command_cycle = cycle;
		accessor->cached_command = command;
		return NULL;
	}

	return NULL;
}
