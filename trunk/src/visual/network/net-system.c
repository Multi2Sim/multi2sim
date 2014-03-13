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
#include <assert.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/misc.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>

#include "net-system.h"
#include "net.h"
#include "node.h"

/*
 * Public Functions
 */

struct vi_net_system_t *vi_net_system;

static char *err_vi_net_system_trace_version =
	"\tThe network system trace file has been created with an incompatible version\n"
	"\tof Multi2Sim. Please rerun the simulation with the same Multi2Sim\n"
	"\tversion used to visualize the trace.\n";

#define VI_NET_SYSTEM_TRACE_VERSION_MAJOR	1
#define VI_NET_SYSTEM_TRACE_VERSION_MINOR	1	

void vi_net_system_init(void)
{
	struct vi_trace_line_t *trace_line;

	/* State file */
/*	vi_state_new_category("Network hierarchy",
		(vi_state_read_checkpoint_func_t) vi_net_system_read_checkpoint,
		(vi_state_write_checkpoint_func_t) vi_net_system_write_checkpoint,
		vi_net_system);
*/
	/* Commands */
/*	vi_state_new_command("net.send",
		(vi_state_process_trace_line_func_t) vi_net_system_send,
		vi_mem_system);
	vi_state_new_command("net.recieve",
		(vi_state_process_trace_line_func_t) vi_net_system_recieve,
		vi_mem_system);
*/
	/* Initialize */
	vi_net_system = xcalloc(1, sizeof(struct vi_net_system_t));
	vi_net_system->net_table = hash_table_create(0, FALSE);

	/* Parse header in state file */
	VI_STATE_FOR_EACH_HEADER(trace_line)
	{
		char *command;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);
		assert(strcmp(command, "c"));

		if (!strcmp(command, "net.init"))
		{
			char *version;

			int version_major = 0;
			int version_minor = 0;

			vi_net_system->active = 1;

			/* Check version compatibility */
			version = vi_trace_line_get_symbol(trace_line, "version");
			if (version)
				sscanf(version, "%d.%d", &version_major, &version_minor);
			if (version_major != VI_NET_SYSTEM_TRACE_VERSION_MAJOR ||
				version_minor > VI_NET_SYSTEM_TRACE_VERSION_MINOR)
				fatal("incompatible network system trace version.\n"
					"\tTrace generation v. %d.%d / Trace consumer v. %d.%d\n%s",
					version_major, version_minor, VI_NET_SYSTEM_TRACE_VERSION_MAJOR,
					VI_NET_SYSTEM_TRACE_VERSION_MINOR, err_vi_net_system_trace_version);
		}
		else if (!strcmp(command, "net.create"))
		{
			struct vi_net_t *net;

			/* Create module */
			net = vi_net_create(trace_line);

			/* Add to network system hash table of networks */
			hash_table_insert(vi_net_system->net_table, net->name, net);

		}
		else if (!strcmp(command, "net.node"))
		{
			struct vi_net_node_t *node;

			node = vi_net_node_assign(trace_line);
			assert(node);

		}
	}
}
