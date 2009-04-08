/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#ifndef NETWORK_H
#define NETWORK_H

#include <mhandle.h>
#include <debug.h>
#include <stdint.h>
#include <stdio.h>
#include <repos.h>
#include <assert.h>
#include <esim.h>
#include <misc.h>
#include <options.h>



/* Network Stack */

struct net_stack_t {
	
	struct net_t *net;
	int src, dst, size;
	uint64_t send_time;
	uint64_t packet_id;

	/* Return event */
	int retevent;
	struct network_stack_t *retstack;
};


struct net_stack_t *net_stack_create(struct net_t *net,
	int retevent, void *retstack);
void net_stack_return(struct net_stack_t *stack);




/* Network */

enum net_node_kind_enum {
	net_node_kind_cache = 0,
	net_node_kind_switch,
	net_node_kind_main  /* main node, like directory, or lower level cache */
};


struct net_node_con_t {
	int cost;  /* number of hops to reach this item */
	int route;  /* next node to reach this item */
	uint64_t busy;  /* last cycle when the link is busy */
};


struct net_node_t {
	enum net_node_kind_enum kind;
	void *data;

	/* Array of connections and routing */
	struct net_node_con_t *con;
};


struct net_t {
	
	/* Array of nodes */
	struct net_node_t *nodes;
	int max_nodes;
	int num_nodes;
	int num_caches;  /* number of nodes of kind net_node_kind_cache */
	int main_node;  /* id of the node of kind net_node_kind_main */
	int link_width;
	char name[100];

	/* For buses */
	int bus;  /* true if it's a bus */
	uint64_t bus_busy;  /* last cycle when bus is busy */
	uint64_t bus_packet_id;  /* packet id occupying the bus */
	int bus_broadcasting;  /* true if issuing a broadcast */

	/* Stats */
	uint64_t current_packet_id;
	uint64_t delivered;  /* number of delivered packets */
	uint64_t deliver_time;  /* acc. deliver time */
};




/* Initialization */
void net_reg_options(void);
void net_init(void);
void net_done(void);


/* Network creation */
struct net_t *net_create(void);  /* create a generic network */
struct net_t *net_create_bus(void);  /* create a bus network */
void net_free(struct net_t *net);
void net_load(char *filename);


/* Initialize arrays of nodes */
void net_init_nodes(struct net_t *net, int max_nodes);

/* Add a node to the network. Return the id of the node. The
 * added node can be a cache, switch or directory. */
int net_add_node(struct net_t *net, enum net_node_kind_enum kind, void *data);

/* Get the data associated with a node */
void *net_get_node(struct net_t *net, int node);

/* Add a connection between to nodes */
void net_add_con(struct net_t *net, int source, int dest);

/* Calculate routes when all nodes have been created. */
void net_calculate_routes(struct net_t *net);
void net_dump_routes(struct net_t *net, FILE *f);

/* Send */
#define net_send(net, src, dst, size) net_send_ev(net, src, dst, size, ESIM_EV_NONE, NULL)
void net_send_ev(struct net_t *net, int src, int dst, int size,
	int retevent, void *retstack);

/* Broadcast for buses */
void net_start_broadcast(struct net_t *net);
void net_commit_broadcast(struct net_t *net);

#endif
