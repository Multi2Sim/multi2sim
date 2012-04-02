/*
 *  Multi2Sim Tools
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <visual-private.h>


/*
 * Network Node
 */

/*
 * Network Node
 */

struct visual_net_node_t
{
	struct visual_mod_t *mod;
};


struct visual_net_node_t *visual_net_node_create(void)
{
	struct visual_net_node_t *node;

	/* Allocate */
	node = calloc(1, sizeof(struct visual_net_node_t));
	if (!node)
		fatal("%s: out of memory", __FUNCTION__);

	/* Return */
	return node;
}


void visual_net_node_free(struct visual_net_node_t *node)
{
	free(node);
}




/*
 * Network
 */

struct visual_net_t *visual_net_create(struct trace_line_t *trace_line)
{
	struct visual_net_t *net;

	int num_nodes;
	int i;

	char *name;

	/* Allocate */
	net = calloc(1, sizeof(struct visual_net_t));
	if (!net)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	name = trace_line_get_symbol_value(trace_line, "name");
	net->name = strdup(name);
	if (!net->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Node list */
	num_nodes = trace_line_get_symbol_value_int(trace_line, "num_nodes");
	net->node_list = list_create();
	for (i = 0; i < num_nodes; i++)
		list_add(net->node_list, visual_net_node_create());


	/* Return */
	return net;
}


void visual_net_free(struct visual_net_t *net)
{
	int i;

	/* Free nodes */
	LIST_FOR_EACH(net->node_list, i)
		visual_net_node_free(list_get(net->node_list, i));
	list_free(net->node_list);

	/* Free network */
	free(net->name);
	free(net);
}


void visual_net_attach_mod(struct visual_net_t *net,
	struct visual_mod_t *mod, int node_index)
{
	struct visual_net_node_t *node;

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, net->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	/* Attach */
	node = list_get(net->node_list, node_index);
	assert(node);
	node->mod = mod;
}
