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

#include "request.h"
#include "dram-system.h"

int EV_DRAM_REQUEST;

/*
 * Request
 */

static void dram_request_expect(struct list_t *token_list, char *request_line)
{
	if (!list_count(token_list))
		fatal("%s: unexpected end of line.\n\t> %s",
				__FUNCTION__, request_line);
}
static long long dram_request_get_llint(struct list_t *token_list,
		char *request_line, char *expected)
{
	int err;
	long long request_cycle;

	/* Read cycle */
	dram_request_expect(token_list, request_line);
	request_cycle = str_to_llint(str_token_list_first(token_list), &err);
	if (err || request_cycle < 1)
		fatal("%s: %s: invalid %s , integer >= 1 expected.\n\t> %s",
				__FUNCTION__, str_token_list_first(token_list),
				request_line, expected);

	/* Shift token and return */
	str_token_list_shift(token_list);
	return request_cycle;
}

static unsigned int dram_request_get_hex_address(struct list_t *token_list,
		char *request_line)
{
	unsigned int request_address;

	/* Read cycle */
	dram_request_expect(token_list, request_line);

	sscanf(str_token_list_first(token_list), "%x", &request_address);
	/* Shift token and return */
	str_token_list_shift(token_list);
	return request_address;
}

static void dram_request_get_type(struct list_t *token_list,
		char *request_line, char *buf, int size)
{
	dram_request_expect(token_list, request_line);
	snprintf(buf, size, "%s", str_token_list_first(token_list));
	str_token_list_shift(token_list);
}


struct dram_request_t *dram_request_create(void)
{
	struct dram_request_t *request;

	/* Initialize */
	request = xcalloc(1, sizeof(struct dram_request_t));

	/* Return */
	return request;
}


void dram_request_free(struct dram_request_t *request)
{
	free(request);
}


void dram_request_dump(struct dram_request_t *dram_request, FILE *f)
{
	fprintf(f, "\t\tRequest ID: %llu, type: ", dram_request->id);

	switch (dram_request->type)
	{

	case request_type_read:

		fprintf(f, "READ, ");
		break;

	case request_type_write:

		fprintf(f, "WRITE, ");
		break;

	default:

		fprintf(f, "???, ");
		break;

	}

	fprintf(f, "addr: %08X\n", dram_request->addr);
}

void dram_request_handler (int event, void *data)
{
	struct list_t *token_list;

	struct request_stack_t *req_stack = data;
	struct dram_system_t *system = req_stack->system;
	char *request_line = req_stack->request_line;
	fprintf(stderr, "request line : %s \n", request_line);
	char request[MAX_STRING_SIZE];

	long long cycle = esim_domain_cycle(dram_domain_index);

	/* Split command in tokens, skip command */
	token_list = str_token_list_create(request_line, " ");
	assert(list_count(token_list));

	long long request_cycle = dram_request_get_llint(token_list,
			request_line,"cycle value");

	if (request_cycle > cycle)
	{
		str_token_list_free(token_list);
		esim_schedule_event(event, req_stack, request_cycle - cycle);
		return;
	}

	struct dram_request_t *dram_request;

	dram_request = dram_request_create();

	dram_request_get_type(token_list, request_line, request,
					sizeof request);

	if (!strcasecmp(request, "READ"))
	{
		dram_request->type = request_type_read;
	}
	else if (!strcasecmp(request, "WRITE"))
	{
		dram_request->type = request_type_write;
	}
	else
		fatal("%s: invalid access type %s.\n\t> %s",
			__FUNCTION__, request, request_line);


	dram_request->addr = dram_request_get_hex_address(token_list, request_line);

	dram_request->system = system;
	dram_request->id = system->request_count;
	system->request_count++;

	list_enqueue(system->dram_request_list, dram_request);

	str_token_list_free(token_list);
	dram_request_stack_free(req_stack);
}

struct request_stack_t *dram_request_stack_create(void)
{
	struct request_stack_t * stack;
	stack = xcalloc(1, sizeof (struct request_stack_t));

	return stack;
}

void dram_request_stack_free(struct request_stack_t *stack)
{
	free(stack->request_line);
	free(stack);
}

