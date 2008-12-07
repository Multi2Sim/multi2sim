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

#include <stdio.h>
#include <stdint.h>


/*
 * Structures
 */

struct net_msg_t {
	uint64_t seq;
	uint64_t send_cycle;
	int size;
	int src_node_idx, dst_node_idx;
	struct net_node_t *src_node, *dst_node;
	void *data;

	/* Current position in network */
	int node_idx;
	int where;  /* 0..3 = {ibuffer,xbar,obuffer,link} */
	int port_idx;

	/* Information for in-transit messages. Messages always travel
	 * from buffers to buffers (or end-nodes) */
	struct net_buffer_t *src_buffer, *dst_buffer;
	uint64_t busy;  /* In transit until cycle */

	/* Liked list for bucket chain in net->msg_table */
	struct net_msg_t *bucket_next;
};


/* Link */
struct net_link_t {
	int src_node_idx, src_port_idx;
	int dst_node_idx, dst_port_idx;
	int bandwidth;
	uint64_t busy;  /* Busy until this cycle inclusive */
};


/* Node buffer */
struct net_buffer_t {
	char name[30];  /* String identifier */
	int count, size;  /* Occupied and total size */
	uint64_t read_busy;
	uint64_t write_busy;

	/* Linked list of stacks to wake up when new space
	 * becomes available in the buffer (using stack->wakeup_event) */
	struct net_stack_t *wakeup_head, *wakeup_tail;
};


/* Node port */
struct net_port_t {
	struct net_link_t *link;  /* Connected link (NULL=none) */
	struct net_buffer_t *buffer;  /* Associated buffer */
};


/* Types of node */
enum net_node_kind_enum {
	net_node_end = 0,
	net_node_switch,
	net_node_bus
};


/* Node */
struct net_node_t {

	/* Type of node */
	enum net_node_kind_enum kind;
	char name[30];
	void *data;

	/* Switch crossbar or bus*/
	int bandwidth;
	uint64_t bus_busy;
	
	/* Ports */
	struct net_port_t *iports;
	struct net_port_t *oports;
	int iport_count;
	int oport_count;
};


/* Routing table entry */
struct routing_entry_t {
	int cost;  /* Cost in hops */
	int port;  /* Output port */
	int route;  /* Next hop */
};


/* Network */
#define NET_MSG_TABLE_SIZE 32
struct net_t {
	
	/* Properties */
	char name[30];
	uint64_t msg_seq;  /* Seq number for messages */
	
	/* Nodes */
	struct net_node_t *nodes;
	int node_array_size;
	int node_count;
	int end_node_count;

	/* 2-dim routing table */
	struct routing_entry_t *routing_table;

	/* Hash table of in-flight messages. Each entry is a
	 * bucket chain. */
	struct net_msg_t *msg_table[NET_MSG_TABLE_SIZE];

	/* Stats */
	uint64_t transfers;  /* Transfers */
	uint64_t lat_acc;  /* Accumulated latency */
};


/* For event-driven simulation */
struct net_stack_t {

	/* Local variables */
	struct net_t *net;
	struct net_msg_t *msg;

	/* Wakeup event and linked list of stacks */
	int wakeup_event;
	struct net_stack_t *wakeup_next;

	/* Return event */
	int retevent;
	struct net_stack_t *retstack;
};




/*
 * Debug
 */

extern FILE *net_debug_file;

int net_debug_init(char *filename);
void net_debug_done();
void net_debug(char *fmt, ...);





/*
 * Functions
 */

void net_init(void);
void net_done(void);

struct net_t *net_create(char *name);
void net_free(struct net_t *net);

int net_new_node(struct net_t *net, char *name, void *data);
int net_new_switch(struct net_t *net,
	int iport_count, int ibuffer_size, int oport_count, int obuffer_size,
	int bandwidth, char *name, void *data);
int net_new_bus(struct net_t *net,
	int iport_count, int oport_count,
	int bandwidth, char *name, void *data);

struct net_node_t *net_get_node(struct net_t *net, int node_idx);
void *net_get_node_data(struct net_t *net, int node_idx);

int net_get_iport_idx(struct net_t *net, int node_idx);
int net_get_oport_idx(struct net_t *net, int node_idx);

void net_new_link(struct net_t *net, int src_node_idx, int dst_node_idx, int bandwidth);
void net_new_bidirectional_link(struct net_t *net, int node1_idx, int node2_idx, int bandwidth);

void net_calculate_routes(struct net_t *net);
void net_dump_routes(struct net_t *net, FILE *f);

int net_valid_route(struct net_t *net, int src_node_idx, int dst_node_idx);
int net_can_send(struct net_t *net, int src_node_idx, int dst_node_idx);
uint64_t net_send(struct net_t *net, int src_node_idx, int dst_node_idx, int size);
uint64_t net_send_ev(struct net_t *net, int src_node_idx, int dst_node_idx, int size,
	int retevent, void *retstack);
int net_in_transit(struct net_t *net, uint64_t seq);



#endif

