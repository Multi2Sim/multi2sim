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

#ifndef NETWORK_NODE_H
#define NETWORK_NODE_H

#include <stdio.h>


/* Types of node */
enum net_node_kind_t
{
	net_node_invalid = 0,
	net_node_end,
	net_node_switch,
	net_node_bus
};


/* Node */
struct net_node_t
{
	struct net_t *net;
	enum net_node_kind_t kind;
	int index;		/* Used to index routing table */
	char *name;
	void *user_data;

	/* Switch crossbar or bus */
	int bandwidth;
	/* long long bus_busy; */

	/* Buffers */
	int input_buffer_size;
	int output_buffer_size;
	struct list_t *input_buffer_list;
	struct list_t *output_buffer_list;

	/* BUS & scheduling */
	struct list_t *bus_lane_list;	/* elements are of type net_bus_t */
	struct list_t *src_buffer_list;	/* elements are of type struct net_buffer_t */
	struct list_t *dst_buffer_list;	/* elements are of type struct net_buffer_t */
	int last_node_index;

	/* Stats */
	long long bytes_received;
	long long msgs_received;
	long long bytes_sent;
	long long msgs_sent;
};


/* Functions */

struct net_node_t *net_node_create(struct net_t *net,
	enum net_node_kind_t kind, int index,
	int input_buffer_size, int output_buffer_size,
	int bandwidth, char *name, void *user_data);
void net_node_free(struct net_node_t *node);
void net_node_dump(struct net_node_t *node, FILE *f);

void net_node_dump_report(struct net_node_t *node, FILE *f);

/* Adding buffers to nodes. It supports asymmetric switches */
struct net_buffer_t *net_node_add_output_buffer(struct net_node_t *node,
	int bandwidth);
struct net_buffer_t *net_node_add_input_buffer(struct net_node_t *node,
	int bandwidth);
struct net_bus_t *net_node_add_bus_lane(struct net_node_t *node);

struct net_buffer_t *net_node_schedule(struct net_node_t *node,
	struct net_buffer_t *output_buffer);

struct net_buffer_t *net_get_buffer_by_name(struct net_node_t *node,
		char *buffer_name);


#endif
