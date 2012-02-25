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

#ifndef NETWORK_H
#define NETWORK_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <mhandle.h>
#include <debug.h>
#include <esim.h>
#include <misc.h>
#include <list.h>
#include <linked-list.h>
#include <hash-table.h>
#include <config.h>




/*
 * Messages
 */

extern char *err_net_end_nodes;
extern char *err_net_no_route;
extern char *err_net_large_message;
extern char *err_net_node_name_duplicate;
extern char *err_net_can_send;
extern char *err_net_config;

extern char *net_config_help;




/*
 * Event-driven Simulation
 */

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

	/* Return event */
	int ret_event;
	struct net_stack_t *ret_stack;
};

struct net_stack_t *net_stack_create(struct net_t *net,
	int retevent, void *retstack);
void net_stack_return(struct net_stack_t *stack);

void net_event_handler(int event, void *data);




/*
 * Network Message
 */

struct net_msg_t
{
	struct net_t *net;

	long long id;
	int size;
	uint64_t send_cycle;  /* Cycle when it was sent */
	uint64_t busy;  /* In transit until cycle */

	struct net_node_t *src_node;
	struct net_node_t *dst_node;
	void *data;

	/* Current position in network */
	struct net_node_t *node;
	struct net_buffer_t *buffer;

	/* Information for in-transit messages. Messages always travel
	 * from buffers to buffers (or end-nodes) */
	struct net_buffer_t *src_buffer;  /* Original source buffer */
	struct net_buffer_t *dst_buffer;  /* Final destination buffer */

	/* Liked list for bucket chain in net->msg_table */
	struct net_msg_t *bucket_next;
};


struct net_msg_t *net_msg_create(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size);
void net_msg_free(struct net_msg_t *msg);

void net_msg_table_insert(struct net_t *net, struct net_msg_t *msg);
struct net_msg_t *net_msg_table_extract(struct net_t *net, uint64_t id);
struct net_msg_t *net_msg_table_get(struct net_t *net, uint64_t id);




/*
 * Network Link
 */

struct net_link_t
{
	struct net_t *net;
	char *name;

	/* Source node-buffer */
	struct net_node_t *src_node;
	struct net_buffer_t *src_buffer;

	/* Destination node-buffer */
	struct net_node_t *dst_node;
	struct net_buffer_t *dst_buffer;

	int bandwidth;
	uint64_t busy;  /* Busy until this cycle inclusive */

	/* Stats */
	long long busy_cycles;
	long long transferred_bytes;
	long long transferred_msgs;
};


/* Functions */
struct net_link_t *net_link_create(struct net_t *net,
	struct net_node_t *src_node, struct net_buffer_t *src_buffer,
	struct net_node_t *dst_node, struct net_buffer_t *dst_buffer,
	int bandwidth);
void net_link_free(struct net_link_t *link);

void net_link_dump_report(struct net_link_t *link, FILE *f);




/*
 * Node buffer
 */

/* Event to be scheduled when space released in buffer */
struct net_buffer_wakeup_t
{
	int event;
	void *stack;
};


struct net_buffer_t
{
	struct net_t *net;  /* Network it belongs to */
	struct net_node_t *node;  /* Node where it belongs */
	char *name;  /* String identifier */

	int index;  /* Index in input/output buffer list of node */
	int size;  /* Total size */
	int count;  /* Occupied buffer size */

	/* Cycle until a read/write operation on buffer lasts */
	uint64_t read_busy;
	uint64_t write_busy;

	/* Link connected to buffer */
	struct net_link_t *link;

	/* List of messages in the buffer */
	struct list_t *msg_list;

	/* Scheduling for output buffers */
	long long sched_when;  /* Last cycle when scheduler was called */
	struct net_buffer_t *sched_buffer;  /* Input buffer to fetch data from */

	/* List of events to schedule when new space becomes available
	 * in the buffer. Elements are of type 'struct net_buffer_wakeup_t' */
	struct linked_list_t *wakeup_list;

	/* Stats */
	int occupancy_bytes_value;
	int occupancy_msgs_value;
	long long occupancy_measured_cycle;
	long long occupancy_bytes_acc;
	long long occupancy_msgs_acc;
};


/* Functions */
struct net_buffer_t *net_buffer_create(struct net_t *net, struct net_node_t *node,
	int size, char *name);
void net_buffer_free(struct net_buffer_t *buffer);

void net_buffer_dump(struct net_buffer_t *buffer, FILE *f);
void net_buffer_dump_report(struct net_buffer_t *buffer, FILE *f);

void net_buffer_insert(struct net_buffer_t *buffer, struct net_msg_t *msg);
void net_buffer_extract(struct net_buffer_t *buffer, struct net_msg_t *msg);

void net_buffer_wait(struct net_buffer_t *buffer, int event, void *stack);
void net_buffer_wakeup(struct net_buffer_t *buffer);

void net_buffer_update_occupancy(struct net_buffer_t *buffer);




/*
 * Network Node
 */

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
	int index;  /* Used to index routing table */
	char *name;
	void *user_data;

	/* Switch crossbar or bus*/
	int bandwidth;
	uint64_t bus_busy;
	
	/* Buffers */
	int input_buffer_size;
	int output_buffer_size;
	struct list_t *input_buffer_list;
	struct list_t *output_buffer_list;

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

void net_node_dump_report(struct net_node_t *node, FILE *f);

struct net_buffer_t *net_node_add_output_buffer(struct net_node_t *node);
struct net_buffer_t *net_node_add_input_buffer(struct net_node_t *node);

struct net_buffer_t *net_node_schedule(struct net_node_t *node,
	struct net_buffer_t *output_buffer);




/*
 * Routing Table
 */

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
	struct net_t *net;  /* Associated network */

	/* 2D array containing routing table */
	int dim;  /* Array dimensions ('dim' x 'dim') */
	struct net_routing_table_entry_t *entries;
};

struct net_routing_table_t *net_routing_table_create(struct net_t *net);
void net_routing_table_free(struct net_routing_table_t *routing_table);

void net_routing_table_calculate(struct net_routing_table_t *routing_table);
void net_routing_table_dump(struct net_routing_table_t *routing_table, FILE *f);
struct net_routing_table_entry_t *net_routing_table_lookup(struct net_routing_table_t *routing_table,
	struct net_node_t *src_node, struct net_node_t *dst_node);




/*
 * Network
 */

#define NET_MSG_TABLE_SIZE 32

/* Network */
struct net_t
{
	/* Properties */
	char *name;
	long long msg_id_counter;  /* Counter to assign message IDs */
	
	/* Nodes */
	struct list_t *node_list;
	int node_count;
	int end_node_count;  /* Number of end nodes */

	/* Links */
	struct list_t *link_list;

	/* Routing table */
	struct net_routing_table_t *routing_table;

	/* Hash table of in-flight messages. Each entry is a bucket list */
	struct net_msg_t *msg_table[NET_MSG_TABLE_SIZE];

	/* Stats */
	long long transfers;  /* Transfers */
	long long lat_acc;  /* Accumulated latency */
	long long msg_size_acc;  /* Accumulated message size */
};


/* Functions */

struct net_t *net_create(char *name);
struct net_t *net_create_from_config(struct config_t *config, char *name);
void net_free(struct net_t *net);

void net_dump_report(struct net_t *net, FILE *f);

struct net_node_t *net_add_end_node(struct net_t *net,
	int input_buffer_size, int output_buffer_size,
	char *name, void *user_data);
struct net_node_t *net_add_bus(struct net_t *net, int bandwidth, char *name);
struct net_node_t *net_add_switch(struct net_t *net,
	int input_buffer_size, int output_buffer_size,
	int bandwidth, char *name);

struct net_node_t *net_get_node_by_name(struct net_t *net, char *name);
struct net_node_t *net_get_node_by_user_data(struct net_t *net,
	void *user_data);

struct net_link_t *net_add_link(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth);
void net_add_bidirectional_link(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth);

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
struct net_msg_t *net_try_send_ev(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size, int receive_event, void *receive_stack,
	int retry_event, void *retry_stack);


void net_receive(struct net_t *net, struct net_node_t *node, struct net_msg_t *msg);




/*
 * Main Library Functions (netlib.c)
 */

/* Debug */
#define net_debug(...) debug(net_debug_category, __VA_ARGS__)
extern int net_debug_category;

/* Configuration parameters */
extern char *net_config_file_name;
extern char *net_report_file_name;
extern char *net_sim_network_name;

extern long long net_max_cycles;
extern double net_injection_rate;
extern int net_msg_size;

/* Functions */
void net_init(void);
void net_done(void);

void net_load(char *file_name);
struct net_t *net_find(char *name);

void net_sim(void);


#endif
