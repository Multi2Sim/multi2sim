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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <visual/common/trace.h>

#include "link.h"
#include "net.h"
#include "net-graph.h"
#include "node.h"

/*
 * Network
 */

struct vi_net_t *vi_net_create(struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;

	int num_nodes;
	int packet_size;
	int i;

	char *name;

	/* Initialize */
	net = xcalloc(1, sizeof(struct vi_net_t));

	/* Name */
	name = vi_trace_line_get_symbol(trace_line, "name");
	net->name = xstrdup(name);

	/* Node list */
	num_nodes = vi_trace_line_get_symbol_int(trace_line, "num_nodes");
	net->node_list = list_create();
	for (i = 0; i < num_nodes; i++)
		list_add(net->node_list, vi_net_node_create());

	/* Get the packet size, if it is in the trace */
	packet_size = vi_trace_line_get_symbol_int(trace_line, "packet_size");
	if (packet_size != 0)
		net->packet_size = packet_size;

	net->high_mods = hash_table_create(0, FALSE);
	net->low_mods  = hash_table_create(0, FALSE);
	net->link_table = hash_table_create(0, FALSE);
	net->node_table = hash_table_create(0, FALSE);

	/* Return */
	return net;
}


void vi_net_free(struct vi_net_t *net)
{
	int i;
	char *link_name;
	struct vi_net_link_t *link;

	char *node_name;
	struct vi_net_node_t *node;

	/* Free nodes */
	LIST_FOR_EACH(net->node_list, i)
	{
		node = list_get(net->node_list, i);
		if (node->name)
			hash_table_remove(net->node_table, node->name);
		vi_net_node_free(node);
	}
	list_free(net->node_list);

	/*Redundant: if it goes here something was wrong - Fail safe */
	HASH_TABLE_FOR_EACH(net->node_table, node_name, node)
		vi_net_node_free(node);
	hash_table_free(net->node_table);

	/* Free Links */
	HASH_TABLE_FOR_EACH(net->link_table, link_name, link)
		vi_net_link_free(link);
	hash_table_free(net->link_table);

	/* Free mod tables - Not freeing mods, since it has been
	 * freed before by mem-system */
	hash_table_free(net->high_mods);
	hash_table_free(net->low_mods);

	/* Free network */
	free(net->name);
	free(net);
}


void vi_net_attach_mod(struct vi_net_t *net,
		struct vi_mod_t *mod, int node_index)
{
	struct vi_net_node_t *node;

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, net->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	/* Attach */
	node = list_get(net->node_list, node_index);
	assert(node);
	node->mod = mod;
}


struct vi_mod_t *vi_net_get_mod(struct vi_net_t *net, int node_index)
{
	struct vi_net_node_t *node;

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, net->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	/* Return */
	node = list_get(net->node_list, node_index);
	return node->mod;
}

void vi_net_graph_visual_calculation (struct vi_net_t *net)
{
	struct vi_net_graph_t *net_graph;

	net_graph = vi_net_graph_create(net);
	vi_net_graph_populate(net, net_graph);
	vi_net_graph_draw_scale(net_graph);
	vi_net_graph_finalize(net_graph);
	vi_net_graph_free(net_graph);
}
