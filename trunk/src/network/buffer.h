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

#ifndef NETWORK_BUFFER_H
#define NETWORK_BUFFER_H

#include <stdio.h>

#include "message.h"


/* Event to be scheduled when space released in buffer */
struct net_buffer_wakeup_t
{
	int event;
	void *stack;

};

enum net_buffer_kind_t
{
	net_buffer_invalid= 0,
	net_buffer_link,
	net_buffer_bus
};

struct net_buffer_t
{
	struct net_t *net;	/* Network it belongs to */
	struct net_node_t *node;	/* Node where it belongs */
	char *name;		/* String identifier */
	int index;		/* Index in input/output buffer list of node */
	int size;		/* Total size */
	int count;		/* Occupied buffer size */
	enum net_buffer_kind_t kind;

	/* Cycle until a read/write operation on buffer lasts */
	long long read_busy;
	long long write_busy;

	/* Link connected to buffer */
	struct net_link_t *link;

	/* BUS connected to buffer */
	struct net_bus_t *bus;


	/* List of messages in the buffer */
	struct list_t *msg_list;

	/* Scheduling for output buffers */
	long long sched_when;	/* Last cycle when scheduler was called */
	struct net_buffer_t *sched_buffer;	/* Input buffer to fetch from */

	/* List of events to schedule when new space becomes available in the 
	 * buffer. Elements are of type 'struct net_buffer_wakeup_t' */
	struct linked_list_t *wakeup_list;

	/* Stats */
	int occupancy_bytes_value;
	int occupancy_msgs_value;
	long long occupancy_measured_cycle;
	long long occupancy_bytes_acc;
	long long occupancy_msgs_acc;
};


/* Functions */
struct net_buffer_t *net_buffer_create(struct net_t *net,
	struct net_node_t *node, int size, char *name);
void net_buffer_free(struct net_buffer_t *buffer);

void net_buffer_dump(struct net_buffer_t *buffer, FILE *f);
void net_buffer_dump_report(struct net_buffer_t *buffer, FILE *f);

void net_buffer_insert(struct net_buffer_t *buffer, struct net_msg_t *msg);
void net_buffer_extract(struct net_buffer_t *buffer, struct net_msg_t *msg);

void net_buffer_wait(struct net_buffer_t *buffer, int event, void *stack);
void net_buffer_wakeup(struct net_buffer_t *buffer);

void net_buffer_update_occupancy(struct net_buffer_t *buffer);


#endif
