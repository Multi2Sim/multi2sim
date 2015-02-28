/*
 *  Multi2Sim Tools
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <visual/common/cycle-bar.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>


#include "net-message.h"

void vi_net_message_free(struct vi_net_message_t *message)
{
	if (message->access_name)
		str_free(message->access_name);
	str_free(message->net_name);
	str_free(message->name);
        str_free(message->state);
        free(message);
}

struct vi_net_message_t *vi_net_message_create(char * net_name, char *name, int size)
{
        struct vi_net_message_t *message;

        /* Initialize */
        message = xcalloc(1, sizeof(struct vi_net_message_t));
        message->name = str_set(message->name, name);
        message->net_name = str_set(message->net_name, net_name);
        message->size = size;
        message->creation_cycle = vi_state_get_current_cycle();

        /* Return */
        return message;
}

void vi_net_message_set_state(struct vi_net_message_t *message, char *state)
{
	message->state = str_set(message->state, state);
	message->state_update_cycle = vi_state_get_current_cycle();
}

void vi_net_message_read_checkpoint(struct vi_net_message_t *message, FILE *f)
{
        char name[MAX_STRING_SIZE];
        char net_name[MAX_STRING_SIZE];
        char state[MAX_STRING_SIZE];

        int count;

        /* Read message name */
        str_read_from_file(f, name, sizeof name);
        message->name = str_set(message->name, name);

        /* Read Network Name */
        str_read_from_file(f, net_name, sizeof net_name);
        message->net_name = str_set(message->net_name, net_name);

        /* Read state */
        str_read_from_file(f, state, sizeof state);
        message->state = str_set(message->state, state);

        /* Read number of links */
        count = fread(&message->num_links, 1, sizeof message->num_links, f);
        if (count != sizeof message->num_links)
                panic("%s: cannot read checkpoint", __FUNCTION__);

        /* Read creation cycle */
        count = fread(&message->creation_cycle, 1, sizeof message->creation_cycle, f);
        if (count != sizeof message->creation_cycle)
                panic("%s: cannot read checkpoint", __FUNCTION__);

        /* Read state update cycle */
        count = fread(&message->state_update_cycle, 1, sizeof message->state_update_cycle, f);
        if (count != sizeof message->state_update_cycle)
                panic("%s: cannot read checkpoint", __FUNCTION__);
}

void vi_net_message_write_checkpoint(struct vi_net_message_t *message, FILE *f)
{
        int count;

        /* Write name */
        str_write_to_file(f, message->name);

        /* Write name */
        str_write_to_file(f, message->net_name);

        /* Write state */
        str_write_to_file(f, message->state);

        /* Write number of links */
        count = fwrite(&message->num_links, 1, sizeof message->num_links, f);
        if (count != sizeof message->num_links)
                panic("%s: cannot write checkpoint", __FUNCTION__);

        /* Write creation cycle */
        count = fwrite(&message->creation_cycle, 1, sizeof message->creation_cycle, f);
        if (count != sizeof message->creation_cycle)
                panic("%s: cannot write checkpoint", __FUNCTION__);

        /* Write state update cycle */
        count = fwrite(&message->state_update_cycle, 1, sizeof message->state_update_cycle, f);
        if (count != sizeof message->state_update_cycle)
                panic("%s: cannot write checkpoint", __FUNCTION__);
}


void vi_net_message_get_name_short(struct vi_net_message_t *message, char *buf, int size)
{
	/* Name */
	str_printf(&buf, &size, "%s", message->name);
}


void vi_net_message_get_desc(struct vi_net_message_t *message, char *buf, int size)
{
	char *title_format_begin = "<span color=\"blue\"><b>";
	char *title_format_end = "</b></span>";

	struct vi_trace_line_t *trace_line;

	long long cycle;
	long long current_cycle;

	int i;

	/* Go to current cycle */
	current_cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(current_cycle);

	/* Title */
	str_printf(&buf, &size, "%sDescription for Message %s%s\n\n",
		title_format_begin, message->name, title_format_end);

	/* Fields */
	str_printf(&buf, &size, "%sName:%s %s\n", title_format_begin,
		title_format_end, message->name);
	if (message->access_name)
		str_printf(&buf, &size, "%sAssociated Access:%s %s\n", title_format_begin,
			title_format_end, message->access_name);
	str_printf(&buf, &size, "%sSize:%s %d\n", title_format_begin,
		title_format_end, message->size);
	str_printf(&buf, &size, "%sCreation cycle:%s %lld\n", title_format_begin,
		title_format_end, message->creation_cycle);

	/* State */
	if (message->state && *message->state)
	{
		str_printf(&buf, &size, "%sState:%s %s\n", title_format_begin,
			title_format_end, message->state);
		str_printf(&buf, &size, "%sState update cycle:%s %lld (%lld cycles ago)\n",
			title_format_begin, title_format_end, message->state_update_cycle,
			current_cycle - message->state_update_cycle);
	}

	/* Log header */
	str_printf(&buf, &size, "\n%sState Log:%s\n", title_format_begin, title_format_end);
	str_printf(&buf, &size, "%10s %6s %s\n", "Cycle", "Rel.", "State");
	for (i = 0; i < 50; i++)
		str_printf(&buf, &size, "-");
	str_printf(&buf, &size, "\n");
	cycle = message->creation_cycle;

	/* Log */
	for (trace_line = vi_state_trace_line_first(cycle);
		trace_line; trace_line = vi_state_trace_line_next())
	{
		char *command;
		char *message_name;
		char *net_name;
		char *state;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);
		message_name = vi_trace_line_get_symbol(trace_line, "name");
		net_name = vi_trace_line_get_symbol(trace_line, "net");

		/* Access starts */
		if ((!strcmp(command, "net.new_msg") && !strcmp(message_name, message->name) && !strcmp(net_name, message->net_name)) ||
			(!strcmp(command, "net.msg") && !strcmp(message_name, message->name)  && !strcmp(net_name, message->net_name)))
		{
			state = vi_trace_line_get_symbol(trace_line, "state");
			str_printf(&buf, &size, "%10lld %6lld %s\n",
				cycle, cycle - current_cycle, state);
		}

		/* Access ends */
		if (!strcmp(command, "net.end_msg") && !strcmp(message_name, message->name) && !strcmp(net_name, message->net_name))
			break;

		/* Cycle */
		if (!strcmp(command, "c"))
			cycle = vi_trace_line_get_symbol_long_long(trace_line, "clk");
	}
}

