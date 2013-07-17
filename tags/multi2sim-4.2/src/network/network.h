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

#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#include <lib/util/config.h>


/* Events */
extern int EV_NET_SEND;
extern int EV_NET_OUTPUT_BUFFER;
extern int EV_NET_INPUT_BUFFER;
extern int EV_NET_RECEIVE;

/* Stack */
struct net_stack_t
{
	/* Local variables */
	struct net_t *net;
	struct net_msg_t *msg;
	char *command;

	/* Return event */
	int ret_event;
	struct net_stack_t *ret_stack;
};

struct net_stack_t *net_stack_create(struct net_t *net,
	int retevent, void *retstack);
void net_stack_return(struct net_stack_t *stack);

void net_event_handler(int event, void *data);



#define NET_MSG_TABLE_SIZE 32

/* Network */
struct net_t
{
	/* Properties */
	char *name;
	long long msg_id_counter;	/* Counter to assign message IDs */
	int def_output_buffer_size;
	int def_input_buffer_size;

	/* Nodes */
	struct list_t *node_list;
	int node_count;
	int end_node_count;	/* Number of end nodes */

	/* Links */
	struct list_t *link_list;

	/* Routing table */
	struct net_routing_table_t *routing_table;

	/* Hash table of in-flight messages. Each entry is a bucket list */
	struct net_msg_t *msg_table[NET_MSG_TABLE_SIZE];

	/* Stats */
	long long transfers;	/* Transfers */
	long long lat_acc;	/* Accumulated latency */
	long long msg_size_acc;	/* Accumulated message size */
};


/* Functions */

struct net_t *net_create(char *name);
struct net_t *net_create_from_config(struct config_t *config, char *name);
void net_free(struct net_t *net);
void net_dump(struct net_t *net, FILE *f);

void net_dump_report(struct net_t *net, FILE *f);

struct net_node_t *net_add_end_node(struct net_t *net,
	int input_buffer_size, int output_buffer_size,
	char *name, void *user_data);
struct net_node_t *net_add_bus(struct net_t *net, int bandwidth, char *name, int lanes);	/* [K] */
struct net_node_t *net_add_switch(struct net_t *net,
	int input_buffer_size, int output_buffer_size,
	int bandwidth, char *name);

struct net_node_t *net_get_node_by_name(struct net_t *net, char *name);
struct net_node_t *net_get_node_by_user_data(struct net_t *net,
	void *user_data);


struct net_link_t *net_add_link(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth, int link_src_bsize, int link_dst_bsize, int vc_count);
void net_add_bidirectional_link(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth, int link_src_bsize, int link_dst_bsize, int vc_count);

void net_add_bus_port(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int bus_src_buffer, int bus_dst_buffer);
void net_add_bidirectional_bus_port(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bus_src_buffer, int bus_dst_buffer);


int net_can_send(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size);
int net_can_send_ev(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size,
	int retry_event, void *retry_stack);

struct net_msg_t *net_send(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size);
struct net_msg_t *net_send_ev(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size,
	int receive_event, void *receive_stack);

struct net_msg_t *net_try_send(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size,
	int retry_event, void *retry_stack);
struct net_msg_t *net_try_send_ev(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node, int size,
	int receive_event, void *receive_stack, int retry_event,
	void *retry_stack);


void net_receive(struct net_t *net, struct net_node_t *node,
	struct net_msg_t *msg);

#endif
