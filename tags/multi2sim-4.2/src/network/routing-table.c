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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "buffer.h"
#include "bus.h"
#include "link.h"
#include "net-system.h"
#include "network.h"
#include "node.h"
#include "routing-table.h"


/* 
 * Private Functions
 */


#define NET_NODE_COLOR_WHITE ((void *) 1)
#define NET_NODE_COLOR_GRAY ((void *) 2)
#define NET_NODE_COLOR_BLACK ((void *) 3)


/* This algorithm will be recursively called to do the backtracking of DFS
 * algorithm. */
/* Theoretically we shouldn't encounter any problem with the cycle detection
 * when BUSes exist since they are not connected to any output buffers and
 * cycles are based on the output buffers. --> more thorough examinations may 
 * be required. */
static void routing_table_cycle_detection_dfs_visit(struct net_routing_table_t
	*routing_table, struct list_t *buffer_list, struct list_t *color_list,
	struct list_t *parent_list, int list_elem, int buffer_count)
{
	struct net_t *net = routing_table->net;

	int j;

	struct net_buffer_t *parent_index;
	struct net_node_t *buffer_color;
	struct net_node_t *node_elem;
	struct net_buffer_t *buffer_elem;

	list_set(color_list, list_elem, NET_NODE_COLOR_GRAY);

	buffer_elem = list_get(buffer_list, list_elem);

	node_elem = buffer_elem->node;

	for (j = 0; j < routing_table->dim && !routing_table->has_cycle; j++)
	{
		struct net_node_t *node_adj;
		struct net_routing_table_entry_t *entry;

		node_adj = list_get(net->node_list, j);
		entry = net_routing_table_lookup(routing_table, node_elem,
			node_adj);

		if (entry->output_buffer == buffer_elem)
		{
			struct net_routing_table_entry_t *entry_adj;

			entry_adj =
				net_routing_table_lookup(routing_table,
				entry->next_node, node_adj);

			for (int i = 0; i < buffer_count; i++)
			{
				struct net_buffer_t *buffer_adj;

				buffer_adj = list_get(buffer_list, i);

				if (buffer_adj == entry_adj->output_buffer)
				{

					buffer_color =
						list_get(color_list, i);
					if (buffer_color ==
						NET_NODE_COLOR_WHITE)
					{
						list_set(parent_list, i,
							buffer_elem);
						routing_table_cycle_detection_dfs_visit
							(routing_table,
							buffer_list,
							color_list,
							parent_list, i,
							buffer_count);
					}

					buffer_color =
						list_get(color_list, i);
					parent_index =
						list_get(parent_list, i);

					if (buffer_color ==
						NET_NODE_COLOR_GRAY
						&& parent_index !=
						buffer_elem)
					{
						warning("network %s: cycle found in routing table.\n%s", 
							net->name, net_err_cycle);
						routing_table->has_cycle = 1;
					}
				}
			}
		}
	}
	list_set(color_list, list_elem, NET_NODE_COLOR_BLACK);
}


/* cycle-dectection - This algorithm uses two lists, one for node color and
 * the * other for node parent of all nodes at the beginning are white. Each 
 * * node is its own parent. algorithm starts from the first node and runs a
 * * depth-first-search and creates the connection between nodes. Every node
 * that * is checked becomes gray. The breaking condition is when algorithm
 * meet a node* that is gray but not based on algorithm sequence that is, its 
 * parent differs * from the node that called the algorithm ---Order =
 * O(|E|+|V|) */
static void net_routing_table_cycle_detection(struct net_routing_table_t
	*routing_table)
{
	int i, j, buffer_count = 0;
	struct net_t *net = routing_table->net;

	struct list_t *color_list;
	struct list_t *parent_list;
	struct list_t *buffer_list;
	struct net_node_t *buffer_color;
	struct net_buffer_t *buffer_i;
	struct net_node_t *node_i;

	buffer_list = list_create();
	color_list = list_create();
	parent_list = list_create();

	for (i = 0; i < routing_table->dim; i++)
	{
		node_i = list_get(net->node_list, i);
		if (node_i->kind != net_node_bus)
			for (j = 0; j < list_count(node_i->output_buffer_list); j++)
			{

				buffer_i = list_get(node_i->output_buffer_list,j);

				list_add(color_list, NET_NODE_COLOR_WHITE);
				list_add(parent_list, buffer_i);
				list_add(buffer_list, buffer_i);
			}
	}

	buffer_count = list_count(color_list);

	for (i = 0; i < buffer_count && !routing_table->has_cycle; i++)
	{
		buffer_color = list_get(color_list, i);
		if (buffer_color == NET_NODE_COLOR_WHITE)
		{
			routing_table_cycle_detection_dfs_visit(routing_table,
				buffer_list, color_list, parent_list, i,
				buffer_count);
		}
	}
	list_free(color_list);
	list_free(parent_list);
	list_free(buffer_list);
}


/* 
 * Public Functions
 */

struct net_routing_table_t *net_routing_table_create(struct net_t *net)
{
	struct net_routing_table_t *routing_table;

	/* Set fields */
	routing_table = xcalloc(1, sizeof(struct net_routing_table_t));
	routing_table->net = net;

	/* Return */
	return routing_table;
}


void net_routing_table_free(struct net_routing_table_t *routing_table)
{
	if (routing_table->entries)
		free(routing_table->entries);
	free(routing_table);
}


/* Create contents of routing table */
void net_routing_table_initiate(struct net_routing_table_t *routing_table)
{
	struct net_t *net = routing_table->net;
	int i, j;
	struct net_node_t *src_node, *dst_node;
	struct net_buffer_t *buffer;
	struct net_routing_table_entry_t *entry;

	/* Allocate routing table entries */
	if (routing_table->entries)
		panic("%s: network \"%s\": routing table already allocated",
			__FUNCTION__, net->name);
	routing_table->dim = list_count(net->node_list);
	routing_table->entries =
		xcalloc(routing_table->dim * routing_table->dim,
		sizeof(struct net_routing_table_entry_t));

	/* Initialize table with infinite costs */
	for (i = 0; i < net->node_count; i++)
	{
		for (j = 0; j < net->node_count; j++)
		{
			src_node = list_get(net->node_list, i);
			dst_node = list_get(net->node_list, j);
			entry = net_routing_table_lookup(routing_table,
				src_node, dst_node);
			entry->cost = i == j ? 0 : routing_table->dim;  /* Infinity or 0 */
			entry->next_node = NULL;
			entry->output_buffer = NULL;
		}
	}

	/* Set 1-hop connections */
	for (i = 0; i < net->node_count; i++)
	{
		src_node = list_get(net->node_list, i);
		for (j = 0; j < list_count(src_node->output_buffer_list); j++)
		{
			buffer = list_get(src_node->output_buffer_list, j);
			/* For each buffer of each node we check to see if it 
			 * is connected to a link or a BUS. If it is
			 * connected to the link we update the table by
			 * getting the destination node of the Link */
			if (buffer->kind == net_buffer_link)
			{
				struct net_link_t *link;

				assert(!buffer->bus);
				link = buffer->link;
				assert(link);
				entry = net_routing_table_lookup
					(routing_table, src_node,
					link->dst_node);
				entry->cost = 1;
				entry->next_node = link->dst_node;
				entry->output_buffer = buffer;
			}

			/* If it is connected to a BUS we create a connection 
			 * between this node and all the nodes that are
			 * connected to the BUS */
			else if (buffer->kind == net_buffer_bus)
			{
				int k;

				struct net_node_t *bus_node;
				struct net_bus_t *bus;

				assert(!buffer->link);
				assert(buffer->bus);
				bus = buffer->bus;
				bus_node = bus->node;
				for (k = 0; k < list_count(bus_node->dst_buffer_list); k++)
				{
					struct net_buffer_t *dst_buffer;
					dst_buffer = list_get(bus_node->dst_buffer_list, k);
					if (src_node != dst_buffer->node)
					{
						entry = net_routing_table_lookup(routing_table, src_node, dst_buffer->node);
						entry->cost = 1;
						entry->next_node = dst_buffer->node;
						entry->output_buffer = buffer;
					}
					/*For BUS. Even though we never get to BUS in the routing table (no routes path
					 * through BUS node. They all pass through BUS node's buffer) we still provide a
					 * Path from BUS nodes to all the nodes. There is no path from nodes that ends in
					 * BUS. Next buffer is still NULL*/
					entry = net_routing_table_lookup(routing_table, bus_node, dst_buffer->node);
					entry->cost = 1;
					entry->next_node = dst_buffer->node;

				}
			}
		}
	}
}

/* Calculate shortest paths Floyd-Warshall algorithm.*/
void net_routing_table_floyd_warshall(struct net_routing_table_t *routing_table)
{
	int i, j, k ;
	struct net_t *net = routing_table->net;

	struct net_node_t *next_node;
	struct net_buffer_t *buffer;
	struct net_link_t *link;

	struct net_routing_table_entry_t *entry;

	/* The 'routing_table_entry->next_node' values do
	 * not necessarily point to the immediate next hop after this. */
	for (k = 0; k < net->node_count; k++)
	{
		for (i = 0; i < net->node_count; i++)
		{
			for (j = 0; j < net->node_count; j++)
			{
				struct net_node_t *node_i;
				struct net_node_t *node_j;
				struct net_node_t *node_k;

				struct net_routing_table_entry_t *entry_i_k;
				struct net_routing_table_entry_t *entry_k_j;
				struct net_routing_table_entry_t *entry_i_j;

				node_i = list_get(net->node_list, i);
				node_j = list_get(net->node_list, j);
				node_k = list_get(net->node_list, k);

				entry_i_k = net_routing_table_lookup(routing_table, node_i, node_k);
				entry_k_j = net_routing_table_lookup(routing_table, node_k, node_j);
				entry_i_j = net_routing_table_lookup(routing_table, node_i, node_j);

				if (entry_i_k->cost + entry_k_j->cost < entry_i_j->cost)
				{
					entry_i_j->cost = entry_i_k->cost + entry_k_j->cost;
					entry_i_j->next_node = node_k;
				}
			}
		}
	}

	/* Calculate output buffers */
	for (i = 0; i < net->node_count; i++)
	{
		for (j = 0; j < net->node_count; j++)
		{
			struct net_node_t *node_i;
			struct net_node_t *node_j;
			struct net_routing_table_entry_t *entry_i_j;

			node_i = list_get(net->node_list, i);

			node_j = list_get(net->node_list, j);
			entry_i_j = net_routing_table_lookup(routing_table, node_i, node_j);
			next_node = entry_i_j->next_node;

			/* No route to node */
			if (!next_node)
			{
				entry_i_j->output_buffer = NULL;
				continue;
			}

			/* Follow path */
			for (;;)
			{
				entry = net_routing_table_lookup(routing_table, node_i, next_node);
				if (entry->cost <= 1)
					break;
				next_node = entry->next_node;
			}

			/* Look for output buffer */
			buffer = NULL;
			if (node_i->kind != net_node_bus)
			{
				struct net_buffer_t *bus_dst_buffer;
				assert(list_count(node_i->output_buffer_list));
				for (k = 0; k < list_count(node_i->output_buffer_list); k++)
				{
					buffer = list_get(node_i->output_buffer_list, k);
					if (buffer->link)
					{
						link = buffer->link;
						assert(link);
						if (link->dst_node == next_node)
							break;
					}
					else if (buffer->bus)
					{
						int l;
						int check = 0;
						struct net_node_t *bus_node;
						struct net_bus_t *bus;

						bus = buffer->bus;
						assert(bus);
						bus_node = bus->node;
						for (l = 0; l < list_count(bus_node->dst_buffer_list); l++)
						{
							bus_dst_buffer = list_get(bus_node->dst_buffer_list, l);
							if (bus_dst_buffer->node == next_node)
							{
								check = 1;
								entry_i_j->next_node = bus_dst_buffer->node;
								break;
							}
						}
						if (check == 1)
							break;

					}
				}
				assert(k < list_count(node_i->output_buffer_list));
				entry_i_j->output_buffer = buffer;


			}
		}
	}

	/* Update routing table entries to point to the next hop */
	for (i = 0; i < net->node_count; i++)
	{
		for (j = 0; j < net->node_count; j++)
		{
			struct net_node_t *node_i;
			struct net_node_t *node_j;
			struct net_routing_table_entry_t *entry_i_j;

			node_i = list_get(net->node_list, i);
			node_j = list_get(net->node_list, j);
			entry_i_j = net_routing_table_lookup(routing_table, node_i, node_j);

			buffer = entry_i_j->output_buffer;
			if (buffer && buffer->link)
			{
				link = buffer->link;
				assert(link);
				entry_i_j->next_node = link->dst_node;
			}
		}
	}

	/* Find cycle in routing table */
	net_routing_table_cycle_detection(routing_table);
}

void net_routing_table_dump(struct net_routing_table_t *routing_table, FILE *f)
{
	int i, j;

	struct net_t *net = routing_table->net;
	struct net_node_t *next_node;
	struct net_node_t *node_l;
	/* Routing table */
	fprintf(f, "         ");
	for (i = 0; i < net->node_count; i++)
	{
		node_l = list_get(net->node_list, i);
		fprintf(f, "\t%s \t\t", node_l->name);
	}

	fprintf(f, "\n");
	for (i = 0; i < net->node_count; i++)
	{
		node_l = list_get(net->node_list, i);
		fprintf(f, "%s\t\t", node_l->name);
		for (j = 0; j < net->node_count; j++)
		{
			struct net_node_t *node_i;
			struct net_node_t *node_j;
			struct net_routing_table_entry_t *entry_i_j;
			struct net_buffer_t *buffer;
			node_i = list_get(net->node_list, i);
			node_j = list_get(net->node_list, j);
			entry_i_j = net_routing_table_lookup(routing_table, node_i, node_j);
			next_node = entry_i_j->next_node;
			buffer = entry_i_j->output_buffer;

			if (next_node)
			{
				fprintf(f, "%s:\t", next_node->name);
				if (buffer)
				{
					fprintf(f,"%s   \t", buffer->name);
				}
				else
					fprintf(f,"-------- \t");
			}
			else
				fprintf(f, "--\t\t\t");
		}
		fprintf(f, "\n");

	}
	fprintf(f, "\n");

	/* Node combinations */
	/*
	int k;
	for (i = 0; i < net->node_count; i++)
	{
		for (j = 0; j < net->node_count; j++)
		{
			struct net_node_t *node_i;
			struct net_node_t *node_j;

			node_i = list_get(net->node_list, i);
			node_j = list_get(net->node_list, j);

			fprintf(f, "from %s to %s: ", node_i->name, node_j->name);
			k = i;
			while (k != j)
			{
				struct net_node_t *node_k;
				struct net_routing_table_entry_t *entry_k_j;

				node_k = list_get(net->node_list, k);
				entry_k_j = net_routing_table_lookup(routing_table, node_k, node_j);
				next_node = entry_k_j->next_node;

				if (!next_node)
				{
					fprintf(f, "x ");
					break;
				}
				fprintf(f, "%s ", next_node->name);
				k = next_node->index;
			}
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
	 */
}


struct net_routing_table_entry_t *net_routing_table_lookup(struct
	net_routing_table_t *routing_table, struct net_node_t *src_node,
	struct net_node_t *dst_node)
{
	struct net_routing_table_entry_t *entry;

	assert(src_node->index < routing_table->dim);
	assert(dst_node->index < routing_table->dim);
	assert(routing_table->dim > 0);

	entry = &routing_table->entries[src_node->index * routing_table->dim +
		dst_node->index];
	return entry;
}

/* Updating the entries in the routing table based on the routes existing in
 * configuration file */
void net_routing_table_route_update(struct net_routing_table_t *routing_table,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	struct net_node_t *next_node, int vc_num)
{
	int route_check = 0;

	int k;

	struct net_buffer_t *buffer;
	struct net_link_t *link;
	struct net_bus_t *bus;
	struct net_routing_table_entry_t *entry;

	entry = net_routing_table_lookup(routing_table, src_node, dst_node);
	entry->next_node = next_node;
	entry->output_buffer = NULL;

	/* Look for output buffer */
	buffer = NULL;
	assert(list_count(src_node->output_buffer_list));

	for (k = 0; (k < list_count(src_node->output_buffer_list) && route_check != 1); k++)
	{
		buffer = list_get(src_node->output_buffer_list, k);
		if (buffer->kind == net_buffer_link)
		{
			link = buffer->link;
			assert(link);
			assert(!buffer->bus);

			if ((link->dst_node == next_node))
			{
				if (vc_num == 0)
				{
					entry->output_buffer = buffer;
					route_check = 1;
				}
				else
				{
					if (link->virtual_channel <= vc_num)
						fatal("Network %s: %s.to.%s: wrong virtual channel "
							"number is used in route \n %s",
							routing_table->net->name, src_node->name,
							dst_node->name, net_err_config);

					struct net_buffer_t *vc_buffer;

					vc_buffer = list_get(src_node->output_buffer_list, (buffer->index)+vc_num);
					assert(vc_buffer->link == buffer->link);
					entry->output_buffer = vc_buffer;
					route_check = 1;
				}
			}
		}
		else if (buffer->kind == net_buffer_bus)
		{
			assert(!buffer->link);
			bus = buffer->bus;
			assert(bus);

			struct net_node_t * bus_node;
			bus_node = bus->node;

			for (int i = 0; i < list_count(bus_node->dst_buffer_list); i++)
			{
				struct net_buffer_t *dst_buffer;
				dst_buffer = list_get(bus_node->dst_buffer_list, i);

				if (dst_buffer->node == next_node)
				{
					entry->output_buffer = buffer;
					route_check = 1;
					break;
				}
			}

			if (vc_num != 0)
				fatal("Network %s: %s.to.%s: BUS does not contain virtual channel \n %s",
						routing_table->net->name, src_node->name,
						dst_node->name, net_err_config);


		}
	}
	/*If there is not a route between the source node and next node , error */
	if (route_check == 0) fatal("Network %s : route %s.to.%s = %s : Missing connection \n%s ",
			routing_table->net->name, src_node->name, dst_node->name,
			next_node->name, net_err_route_step);

	/* Find cycle in routing table */
	net_routing_table_cycle_detection(routing_table);
}

void net_routing_table_route_create(struct net_routing_table_t *routing_table,
	struct net_node_t *src_node_r, struct net_node_t *dst_node_r,
	struct net_node_t *nxt_node_r, int vc_used)
{
	struct net_t *net = routing_table->net;

	if (nxt_node_r)
	{
		if (src_node_r == dst_node_r)
			fatal("Network %s:Routes: Invalid Routing format.\n %s",
				net->name,net_err_config);
		else
		{
			if (vc_used > 0)
				net_routing_table_route_update(routing_table,
					src_node_r, dst_node_r, nxt_node_r,
					vc_used);
			else
			{
				vc_used = 0;
				net_routing_table_route_update(routing_table,
					src_node_r, dst_node_r, nxt_node_r,
					vc_used);
			}
		}
	}
}
