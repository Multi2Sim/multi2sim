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

#ifndef NETWORK_ROUTING_TABLE_H
#define NETWORK_ROUTING_TABLE_H


/* Routing table entry */
struct net_routing_table_entry_t
{
	int cost;  /* Cost in hops */
	struct net_node_t *next_node;  /* Next node to destination */
	struct net_buffer_t *output_buffer;  /* Output buffer to destination */
};

/* Table */
struct net_routing_table_t
{
	struct net_t *net;	/* Associated network */

	/* 2D array containing routing table */
	int dim;		/* Array dimensions ('dim' x 'dim') */
	struct net_routing_table_entry_t *entries;

	/* Flag set when a cycle was detected */
	int has_cycle;
};

struct net_routing_table_t *net_routing_table_create(struct net_t *net);
void net_routing_table_free(struct net_routing_table_t *routing_table);

void net_routing_table_initiate(struct net_routing_table_t *routing_table);

void net_routing_table_floyd_warshall(struct net_routing_table_t
	*routing_table);
void net_routing_table_dump(struct net_routing_table_t *routing_table,
	FILE *f);

void net_routing_table_route_create(struct net_routing_table_t *routing_table,
	struct net_node_t *src_node_r, struct net_node_t *dst_node_r,
	struct net_node_t *nxt_node_r, int vc_used);
void net_routing_table_route_update(struct net_routing_table_t *routing_table,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	struct net_node_t *nxt_node, int vc_used);

struct net_routing_table_entry_t *net_routing_table_lookup(struct
	net_routing_table_t *routing_table, struct net_node_t *src_node,
	struct net_node_t *dst_node);


#endif

