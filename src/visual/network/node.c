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
#include <stdio.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/hash-table.h>
#include <lib/util/misc.h>
#include <lib/mhandle/mhandle.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>

#include "buffer.h"
#include "net.h"
#include "net-system.h"
#include "node.h"

/*
 * Public Functions
 */
struct vi_net_node_t *vi_net_node_create(void)
{
	struct vi_net_node_t *node;

	/* Return */
	node = xcalloc(1, sizeof(struct vi_net_node_t));
	return node;
}

struct vi_net_node_t * vi_net_node_assign(struct vi_trace_line_t *trace_line)
{
	int i;
	char *name;
	struct vi_net_t *net;
	struct vi_net_node_t *node;

	/* Get the network name */
	name = vi_trace_line_get_symbol(trace_line, "net_name");
	net = hash_table_get(vi_net_system->net_table, name);

	assert(net);

	/* Get the Node */
	int node_index;
	node_index = vi_trace_line_get_symbol_int(trace_line, "node_index");

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, net->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	node = list_get(net->node_list, node_index);
	assert(node);
	node->index = node_index;

	/* Set Node Name */
	char *node_name;
	node_name = vi_trace_line_get_symbol(trace_line, "node_name");
	if (node_name)
		node->name = xstrdup(node_name);
	else
		panic("%s: node name for node in index %d is not in the trace line ",
				__FUNCTION__, node_index);

	/* Get node type */
	enum vi_net_node_kind node_type;
	node_type = vi_trace_line_get_symbol_int(trace_line, "node_type");
	node->type = node_type;

	if (node->type == vi_net_node_bus || node->type == vi_net_node_photonic)
	{
		int num_lanes = vi_trace_line_get_symbol_int(trace_line, "num_lanes");
		node->bus_lane_list = list_create();
		for (i = 0; i < num_lanes ; i++)
			list_add(node->bus_lane_list, vi_net_bus_create());

		node->src_buffer_list = hash_table_create(0, FALSE);
		node->dst_buffer_list = hash_table_create(0, FALSE);
	}
	else
	{
		node->input_buffer_list  = hash_table_create(0, FALSE);
		node->output_buffer_list = hash_table_create(0, FALSE);
	}
	hash_table_insert(net->node_table, node->name, node);
	return node;
}



void vi_net_node_free(struct vi_net_node_t *node)
{
	int i;
	char *buffer_name;
	struct vi_net_buffer_t *buffer;

	/* Free buffers , then hash tables */
	if (node->input_buffer_list)
	{
		HASH_TABLE_FOR_EACH(node->input_buffer_list, buffer_name, buffer)
				vi_net_buffer_free(buffer);
		hash_table_free(node->input_buffer_list);
	}

	if (node->output_buffer_list)
	{
		HASH_TABLE_FOR_EACH(node->output_buffer_list, buffer_name, buffer)
			vi_net_buffer_free(buffer);
		hash_table_free(node->output_buffer_list);
	}

	if (node->bus_lane_list)
	{
		LIST_FOR_EACH(node->bus_lane_list, i)
			vi_net_bus_free(list_get(node->bus_lane_list, i));
		list_free(node->bus_lane_list);
	}

	if (node->src_buffer_list)
		hash_table_free(node->src_buffer_list);

	if (node->dst_buffer_list)
		hash_table_free(node->dst_buffer_list);
	/* Free name then node itself */
	if (node->name)
		free(node->name);
	free(node);
}

struct vi_net_bus_t  *vi_net_bus_create  (void)
{
	struct vi_net_bus_t *bus;

	bus = xcalloc(1, sizeof(struct vi_net_bus_t));
	return bus;
}

void vi_net_bus_free (struct vi_net_bus_t *bus)
{
	free(bus);
}
