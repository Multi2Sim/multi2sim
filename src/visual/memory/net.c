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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <visual/common/trace.h>

#include "net.h"


/*
 * Network Node
 */

struct vi_net_node_t
{
	struct vi_mod_t *mod;
};


struct vi_net_node_t *vi_net_node_create(void)
{
	struct vi_net_node_t *node;

	/* Return */
	node = xcalloc(1, sizeof(struct vi_net_node_t));
	return node;
}


void vi_net_node_free(struct vi_net_node_t *node)
{
	free(node);
}




/*
 * Network
 */

struct vi_net_t *vi_net_create(struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;

	int num_nodes;
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


	/* Return */
	return net;
}


void vi_net_free(struct vi_net_t *net)
{
	int i;

	/* Free nodes */
	LIST_FOR_EACH(net->node_list, i)
		vi_net_node_free(list_get(net->node_list, i));
	list_free(net->node_list);

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
