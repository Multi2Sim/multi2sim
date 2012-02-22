/*
 *  Multi2Sim
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

#include <network.h>


struct net_routing_table_t *net_routing_table_create(struct net_t *net)
{
	struct net_routing_table_t *routing_table;

	/* Create */
	routing_table = calloc(1, sizeof(struct net_routing_table_t));
	if (!routing_table)
		fatal("%s: out of memory", __FUNCTION__);

	/* Set fields */
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
void net_routing_table_calculate(struct net_routing_table_t *routing_table)
{
	int i, j, k;

	struct net_t *net = routing_table->net;

	struct net_node_t *src_node, *dst_node, *next_node;
	struct net_buffer_t *buffer;
	struct net_link_t *link;

	struct net_routing_table_entry_t *entry;

	/* Allocate routing table entries */
	if (routing_table->entries)
		panic("%s: network \"%s\": routing table already allocated", __FUNCTION__, net->name);
	routing_table->dim = list_count(net->node_list);
	routing_table->entries = calloc(routing_table->dim * routing_table->dim,
		sizeof(struct net_routing_table_entry_t));
	if (!routing_table->entries)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize table with infinite costs */
	for (i = 0; i < net->node_count; i++)
	{
		for (j = 0; j < net->node_count; j++)
		{
			src_node = list_get(net->node_list, i);
			dst_node = list_get(net->node_list, j);
			entry = net_routing_table_lookup(routing_table, src_node, dst_node);
			entry->cost = i == j ? 0 : routing_table->dim;  /* Infinity or 0 */
			entry->next_node = NULL;
		}
	}

	/* Set 1-hop connections */
	for (i = 0; i < net->node_count; i++)
	{
		src_node = list_get(net->node_list, i);
		for (j = 0; j < list_count(src_node->output_buffer_list); j++)
		{
			buffer = list_get(src_node->output_buffer_list, j);
			link = buffer->link;
			if (!link)
				printf("buffer \"%s\" with no link\n", buffer->name); //////
			assert(link);

			entry = net_routing_table_lookup(routing_table, src_node, link->dst_node);
			entry->cost = 1;
			entry->next_node = link->dst_node;
		}
	}

	/* Calculate shortest paths Floyd-Warshall algorithm. The 'routing_table_entry->next_node' values do
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
			assert(list_count(node_i->output_buffer_list));
			for (k = 0; k < list_count(node_i->output_buffer_list); k++)
			{
				buffer = list_get(node_i->output_buffer_list, k);
				link = buffer->link;
				assert(link);
				if (link->dst_node == next_node)
					break;
			}
			assert(k < list_count(node_i->output_buffer_list));
			entry_i_j->output_buffer = buffer;
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
			if (buffer)
			{
				link = buffer->link;
				assert(link);
				entry_i_j->next_node = link->dst_node;
			}
		}
	}
}


void net_routing_table_dump(struct net_routing_table_t *routing_table, FILE *f)
{
	int i, j, k;

	struct net_t *net = routing_table->net;
	struct net_node_t *next_node;

	/* Routing table */
	fprintf(f, "         ");
	for (i = 0; i < net->node_count; i++)
		fprintf(f, "%2d ", i);
	fprintf(f, "\n");
	for (i = 0; i < net->node_count; i++)
	{
		fprintf(f, "node %2d: ", i);
		for (j = 0; j < net->node_count; j++)
		{
			struct net_node_t *node_i;
			struct net_node_t *node_j;
			struct net_routing_table_entry_t *entry_i_j;

			node_i = list_get(net->node_list, i);
			node_j = list_get(net->node_list, j);
			entry_i_j = net_routing_table_lookup(routing_table, node_i, node_j);
			next_node = entry_i_j->next_node;

			if (next_node)
				fprintf(f, "%2d ", next_node->index);
			else
				fprintf(f, "-- ");
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");

	/* Node combinations */
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
}


struct net_routing_table_entry_t *net_routing_table_lookup(struct net_routing_table_t *routing_table,
	struct net_node_t *src_node, struct net_node_t *dst_node)
{
	struct net_routing_table_entry_t *entry;

	assert(src_node->index < routing_table->dim);
	assert(dst_node->index < routing_table->dim);
	assert(routing_table->dim > 0);

	entry = &routing_table->entries[src_node->index * routing_table->dim + dst_node->index];
	return entry;
}
