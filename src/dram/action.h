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

#ifndef DRAM_ACTION_H
#define DRAM_ACTION_H


#include <lib/util/string.h>

#include "request.h"
#include "command.h"


extern int EV_DRAM_ACTION;

enum dram_action_type_t
{
	action_type_set_active_row,
	action_type_request,
	action_type_check_command_start
};

struct dram_action_t
{
	enum dram_action_type_t type;
	struct dram_system_t *system;
	union
	{
		struct {
			int logical_chan;
			int physical_chan;
			int rank;
			int bank;
			int row;
		} set_active_row;

		struct {
			char type_str[MAX_STRING_SIZE];
			enum dram_request_type_t type;
			unsigned int addr;
		} request;

		struct {
			char type_str[MAX_STRING_SIZE];
			enum dram_command_type_t type;
			int logical_chan;
			int physical_chan;
			int rank;
			int bank;
		} check_command_start;
	};
};

struct dram_action_stack_t
{
	struct dram_action_t *action;
	long long cycle;
};

void dram_action_parse_action_line(struct dram_system_t *system, char *action_line);
void dram_action_handler(int event, void *data);
struct dram_action_t *dram_action_create(void);
void dram_action_free(struct dram_action_t *dram_action);
struct dram_action_stack_t *dram_action_stack_create(void);
void dram_action_stack_free(struct dram_action_stack_t *action);


#endif