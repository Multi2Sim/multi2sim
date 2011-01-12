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

#include "network.h"

#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <repos.h>
#include <esim.h>
#include <misc.h>
#include <mhandle.h>


/*
 * Access macros
 */

#define NET_ENTRY(i, j) (&net->routing_table[(i) * net->node_count + (j)])
#define NET_COST(i, j) (NET_ENTRY(i, j)->cost)
#define NET_ROUTE(i, j) (NET_ENTRY(i, j)->route)
#define NET_PORT(i, j) (NET_ENTRY(i, j)->port)




/*
 * Private Variables
 */

static struct repos_t *net_msg_repos;

static int EV_NET_SEND;
static int EV_NET_RECEIVE;




/*
 * Private Functions
 */

static void net_error(char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "fatal: libnetwork: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	abort();
}



static int net_allocate_node(struct net_t *net, enum net_node_kind_enum kind,
	int iport_count, int ibuffer_size, int oport_count, int obuffer_size,
	int bandwidth, char *name, void *data)
{
	struct net_node_t *node;
	struct net_buffer_t *buffer;
	int i;

	/* Resize node array */
	if (net->node_count == net->node_array_size) {
		net->node_array_size *= 2;
		net->nodes = realloc(net->nodes, net->node_array_size * sizeof(struct net_node_t));
		if (!net->nodes)
			net_error("out of memory");
		memset(net->nodes + net->node_count, 0, (net->node_array_size - net->node_count) *
			sizeof(struct net_node_t));
	}

	/* Get new node */
	net->node_count++;
	node = &net->nodes[net->node_count - 1];

	/* Name and data */
	strncpy(node->name, name, sizeof(node->name));
	node->data = data;
	node->kind = kind;

	/* Bandwidth */
	node->bandwidth = bandwidth;
	if (bandwidth < 1)
		net_error("switch bandwidth must be >= 1");

	/* Initialize input ports */
	assert(iport_count > 0);
	node->iport_count = iport_count;
	node->iports = calloc(iport_count, sizeof(struct net_port_t));
	if (ibuffer_size) {
		for (i = 0; i < iport_count; i++) {
			node->iports[i].buffer = buffer = calloc(1, sizeof(struct net_buffer_t));
			if (!buffer)
				net_error("out of memory");
			buffer->size = ibuffer_size;
			snprintf(buffer->name, sizeof(buffer->name), "%s.iport[%d].buffer",
				node->name, i);
		}
	}

	/* Initialize output ports */
	assert(oport_count > 0);
	node->oport_count = oport_count;
	node->oports = calloc(oport_count, sizeof(struct net_port_t));
	if (obuffer_size) {
		for (i = 0; i < oport_count; i++) {
			node->oports[i].buffer = buffer = calloc(1, sizeof(struct net_buffer_t));
			if (!buffer)
				net_error("out of memory");
			buffer->size = obuffer_size;
			snprintf(buffer->name, sizeof(buffer->name), "%s.oport[%d].buffer",
				node->name, i);
		}
	}

	/* Return node index */
	return net->node_count - 1;
}


static void net_buffer_insert(struct net_t *net, struct net_buffer_t *buffer, struct net_msg_t *msg)
{
	assert(buffer->count + msg->size <= buffer->size);
	buffer->count += msg->size;
	esim_debug("msg action=\"insert\", net=\"%s\", seq=%lld, buffer=\"%s\"\n",
		net->name, (long long) msg->seq, buffer->name);
}


static void net_buffer_extract(struct net_t *net, struct net_buffer_t *buffer, struct net_msg_t *msg)
{
	struct net_stack_t *stack;
	
	assert(buffer->count >= msg->size);
	buffer->count -= msg->size;
	esim_debug("msg action=\"extract\", net=\"%s\", seq=%lld, buffer=\"%s\"\n",
		net->name, (long long) msg->seq, buffer->name);

	/* Call events waiting for some free space in the buffer. */
	while (buffer->wakeup_head) {
		stack = buffer->wakeup_head;
		if (buffer->wakeup_head == buffer->wakeup_tail)
			buffer->wakeup_head = buffer->wakeup_tail = NULL;
		else
			buffer->wakeup_head = stack->wakeup_next;
		esim_schedule_event(stack->wakeup_event, stack, 0);
		stack->wakeup_event = 0;
		stack->wakeup_next = NULL;
	}
}


/* Schedule an event to be called when the buffer releases some space. */
static void net_buffer_notify(struct net_t *net, struct net_buffer_t *buffer, int event,
	struct net_stack_t *stack)
{
	/* No event */
	if (event == ESIM_EV_NONE)
		return;
	
	/* Schedule notification */
	assert(buffer->size > 0 && buffer->count > 0);
	assert(stack && !stack->wakeup_event && !stack->wakeup_next);
	stack->wakeup_event = event;
	if (!buffer->wakeup_tail)
		buffer->wakeup_head = buffer->wakeup_tail = stack;
	else {
		buffer->wakeup_tail->wakeup_next = stack;
		buffer->wakeup_tail = stack;
	}
}


/* Insert a message into the in-flight messages hash table. */
static void net_msg_table_insert(struct net_t *net, struct net_msg_t *msg)
{
	int index;

	index = msg->seq % NET_MSG_TABLE_SIZE;
	assert(!msg->bucket_next);
	msg->bucket_next = net->msg_table[index];
	net->msg_table[index] = msg;
}


/* Return a message from the in-flight messages hash table */
static struct net_msg_t *net_msg_table_get(struct net_t *net, uint64_t seq)
{
	int index;
	struct net_msg_t *msg;

	index = seq % NET_MSG_TABLE_SIZE;
	msg = net->msg_table[index];
	while (msg && msg->seq != seq)
		msg = msg->bucket_next;
	return msg;
}


/* Extract a message from the in-flight messages hash table */
static struct net_msg_t *net_msg_table_extract(struct net_t *net, uint64_t seq)
{
	int index;
	struct net_msg_t *prev, *msg;

	index = seq % NET_MSG_TABLE_SIZE;
	prev = NULL;
	msg = net->msg_table[index];
	while (msg && msg->seq != seq) {
		prev = msg;
		msg = msg->bucket_next;
	}
	if (!msg)
		net_error("msg %lld not in hash table", (long long) seq);
	if (prev)
		prev->bucket_next = msg->bucket_next;
	else
		net->msg_table[index] = msg->bucket_next;
	return msg;
}





/*
 * Event-driven Simulation
 */

static struct repos_t *net_stack_repos;


static struct net_stack_t *net_stack_create(struct net_t *net,
	int retevent, void *retstack)
{
	struct net_stack_t *stack;
	stack = repos_create_object(net_stack_repos);
	stack->net = net;
	stack->retevent = retevent;
	stack->retstack = retstack;
	return stack;
}


static void net_stack_return(struct net_stack_t *stack)
{
	int retevent = stack->retevent;
	struct net_stack_t *retstack = stack->retstack;
	repos_free_object(net_stack_repos, stack);
	esim_schedule_event(retevent, retstack, 0);
}

#define NET_CAN_TRANSFER  0
#define NET_DO_TRANSFER   1

#define NET_WHERE_IBUFFER 0
#define NET_WHERE_XBAR    1
#define NET_WHERE_OBUFFER 2
#define NET_WHERE_LINK    3

/* Transfer message through network.
 *   how = NET_CAN_TRANSFER: if msg can be transferred to next buffer or destination, return the
 *                           required latency.
 *                           If not, return 0, and schedule event for the cycle when the test
 *                           should be made again.
 *   how = NET_DO_TRANSFER:  make the transfer and reserve resources for 'lat' cycles
 *                           until next buffer or destination. Messages are not inserted/extracted
 *                           from buffers. On the contrary, the 'src_buffer' and 'dst_buffer'
 *                           attributes are set in 'msg'.
 */
static int net_transfer(struct net_t *net, struct net_msg_t *msg, int how, int lat,
	int event, void *stack)
{
	int node_idx, where, port_idx, routing_port_idx;
	struct net_node_t *node;
	struct net_port_t *port;
	struct net_link_t *link;
	struct net_buffer_t *buffer;

	/* Check that routes have been calculated */
	if (!net->routing_table)
		net_error("%s: no routing table", net->name);

	/* Store current position */
	node_idx = msg->node_idx;
	where = msg->where;
	port_idx = msg->port_idx;

	/* Initialize */
	if (how == NET_CAN_TRANSFER) {
		assert(!esim_cycle || msg->busy < esim_cycle);
		lat = 1;
	}
	if (how == NET_DO_TRANSFER) {
		if (lat < 1)
			net_error("net_transfer: lat must be >= 1");
		msg->busy = esim_cycle + lat - 1;
		msg->src_buffer = NULL;
		msg->dst_buffer = NULL;
	}

	/* Start virtual transfer */
	while (node_idx != msg->dst_node_idx) {
		
		/* Message in obuffer */
		if (where == NET_WHERE_OBUFFER) {
			
			/* Check valid route */
			node = &net->nodes[node_idx];
			routing_port_idx = NET_PORT(node_idx, msg->dst_node_idx);
			if (routing_port_idx < 0)
				net_error("%s: no route from %s to %s", net->name,
					node->name, msg->dst_node->name);
			assert(routing_port_idx == port_idx);
			
			/* Go to output link of same node */
			port = &node->oports[port_idx];
			link = port->link;
			buffer = port->buffer;
			where = NET_WHERE_LINK;

			/* Action */
			if (how == NET_CAN_TRANSFER) {
				if (buffer && buffer->read_busy >= esim_cycle) {
					esim_schedule_event(event, stack,
						buffer->read_busy - esim_cycle + 1);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d,"
						" where=obuffer, port=%d, retry=%lld, why=\"read port busy\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						(long long) buffer->read_busy + 1);
					return 0;
				}
				if (link->busy >= esim_cycle) {
					esim_schedule_event(event, stack,
						link->busy - esim_cycle + 1);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d,"
						" where=obuffer, port=%d, retry=%lld, why=\"link busy\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						(long long) link->busy + 1);
					return 0;
				}
				lat = MAX(lat, (msg->size + link->bandwidth - 1) / link->bandwidth);
			}
			if (how == NET_DO_TRANSFER) {
				msg->where = NET_WHERE_LINK;
				link->busy = esim_cycle + lat - 1;
				if (buffer) {
					msg->src_buffer = buffer;
					buffer->read_busy = esim_cycle + lat - 1;
				}
				esim_debug("msg action=\"transfer\", net=\"%s\", seq=%lld, node=%d, where=link,"
					" port=%d, busy=%lld\n",
					net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
					(long long) link->busy);
			}
		}

		/* Message in a link */
		else if (where == NET_WHERE_LINK) {

			/* Go to input buffer of next node */
			node = &net->nodes[node_idx];
			link = node->oports[port_idx].link;
			node_idx = link->dst_node_idx;
			port_idx = link->dst_port_idx;
			node = &net->nodes[node_idx];
			port = &node->iports[port_idx];
			buffer = port->buffer;
			where = NET_WHERE_IBUFFER;

			/* Action - if there is no input buffer, continue. */
			if (how == NET_CAN_TRANSFER && buffer) {
				if (buffer->size < msg->size)
					net_error("msg of size %d cannot enter buffer of size %d in"
						" node %s, input port %d", msg->size, buffer->size,
						node->name, port_idx);
				if (buffer->write_busy >= esim_cycle) {
					esim_schedule_event(event, stack,
						buffer->write_busy - esim_cycle + 1);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d, where=link,"
						" port=%d, retry=%lld, why=\"write port busy\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						(long long) buffer->write_busy + 1);
					return 0;
				}
				if (buffer->count + msg->size > buffer->size) {
					net_buffer_notify(net, buffer, event, stack);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d, where=link,"
						" port=%d, why=\"%s full\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						buffer->name);
					return 0;
				}
				return lat;
			}
			if (how == NET_DO_TRANSFER) {
				msg->node_idx = node_idx;
				msg->port_idx = port_idx;
				msg->where = NET_WHERE_IBUFFER;
				esim_debug("msg action=\"transfer\", net=\"%s\", seq=%lld, node=%d,"
					" where=ibuffer, port=%d\n",
					net->name, (long long) msg->seq, msg->node_idx, msg->port_idx);
				if (buffer) {
					msg->dst_buffer = buffer;
					buffer->write_busy = esim_cycle + lat - 1;
					return lat;
				}
			}
		}

		/* Message in an ibuffer */
		else if (where == NET_WHERE_IBUFFER) {
			
			/* Go to crossbar of same node */
			node = &net->nodes[node_idx];
			port = &node->iports[port_idx];
			buffer = port->buffer;
			where = NET_WHERE_XBAR;

			/* Action */
			if (how == NET_CAN_TRANSFER) {
				if (buffer && buffer->read_busy >= esim_cycle) {
					esim_schedule_event(event, stack,
						buffer->read_busy - esim_cycle + 1);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d,"
						" where=ibuffer, port=%d, retry=%lld, why=\"read port busy\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						(long long) buffer->read_busy + 1);
					return 0;
				}
				if (node->kind == net_node_bus && node->bus_busy >= esim_cycle) {
					esim_schedule_event(event, stack,
						node->bus_busy - esim_cycle + 1);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d,"
						" where=ibuffer, port=%d, retry=%lld, why=\"bus busy\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						(long long) node->bus_busy + 1);
					return 0;
				}
				lat = MAX(lat, (msg->size + node->bandwidth - 1) / node->bandwidth);
			}
			if (how == NET_DO_TRANSFER) {
				msg->where = NET_WHERE_XBAR;
				if (node->kind == net_node_bus)
					node->bus_busy = esim_cycle + lat - 1;
				esim_debug("msg action=\"transfer\", net=\"%s\", seq=%lld, node=%d,"
					" where=xbar, port=%d\n",
					net->name, (long long) msg->seq, msg->node_idx, msg->port_idx);
				if (buffer) {
					msg->src_buffer = buffer;
					buffer->read_busy = esim_cycle + lat - 1;
				}
			}
		}

		/* Message in a node crossbar */
		else if (where == NET_WHERE_XBAR) {
			
			/* Go to output buffer of same node */
			node = &net->nodes[node_idx];
			port_idx = NET_PORT(node_idx, msg->dst_node_idx);
			if (port_idx < 0)
				net_error("no route from %d to %d", node_idx, msg->dst_node_idx);
			port = &node->oports[port_idx];
			buffer = port->buffer;
			where = NET_WHERE_OBUFFER;

			/* Action */
			if (how == NET_CAN_TRANSFER && buffer) {
				if (buffer->size < msg->size)
					net_error("msg of size %d cannot enter buffer of size %d in"
						" node %s, output port %d", msg->size, buffer->size,
						node->name, port_idx);
				if (buffer->write_busy >= esim_cycle) {
					esim_schedule_event(event, stack,
						buffer->write_busy - esim_cycle + 1);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d, where=xbar,"
						" port=%d, retry=%lld, why=\"write port busy\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						(long long) buffer->write_busy + 1);
					return 0;
				}
				if (buffer->count + msg->size > buffer->size) {
					net_buffer_notify(net, buffer, event, stack);
					esim_debug("msg action=\"stall\", net=\"%s\", seq=%lld, node=%d, where=xbar,"
						" port=%d, why=\"%s full\"\n",
						net->name, (long long) msg->seq, msg->node_idx, msg->port_idx,
						buffer->name);
					return 0;
				}
				return lat;
			}
			if (how == NET_DO_TRANSFER) {
				msg->where = NET_WHERE_OBUFFER;
				msg->port_idx = port_idx;
				esim_debug("msg action=\"transfer\", net=\"%s\", seq=%lld, node=%d,"
					" where=obuffer, port=%d\n",
					net->name, (long long) msg->seq, msg->node_idx, msg->port_idx);
				if (buffer) {
					msg->dst_buffer = buffer;
					buffer->write_busy = esim_cycle + lat - 1;
					return lat;
				}
			}
		}
	}

	/* Destination reached */
	return lat;
}


static void net_handler(int event, void *data)
{
	struct net_stack_t *stack = data;
	struct net_t *net = stack->net;
	struct net_msg_t *msg = stack->msg;

	if (event == EV_NET_SEND)
	{
		int lat;

		lat = net_transfer(net, msg, NET_CAN_TRANSFER, 0,
			EV_NET_SEND, stack);
		if (!lat)
			return;

		/* Do the transfer */
		esim_debug("msg action=\"send\", net=\"%s\", seq=%lld, node=%d, dst=%d\n",
			net->name, (long long) msg->seq, msg->node_idx, msg->dst_node_idx);
		net_transfer(net, msg, NET_DO_TRANSFER, lat, 0, NULL);
		esim_schedule_event(EV_NET_RECEIVE, stack, lat);

		/* Reserve space in dest buffer */
		if (msg->dst_buffer)
			net_buffer_insert(net, msg->dst_buffer, msg);
	}

	else if (event == EV_NET_RECEIVE)
	{
		int lat;

		esim_debug("msg action=\"receive\", net=\"%s\", seq=%lld, node=%d\n",
			net->name, (long long) msg->seq, msg->node_idx);

		/* Remove msg from source buffer */
		if (msg->src_buffer)
			net_buffer_extract(net, msg->src_buffer, msg);

		/* Next hop */
		if (msg->node_idx == msg->dst_node_idx) {
			lat = esim_cycle - msg->send_cycle;
			esim_debug("msg action=\"deliver\", net=\"%s\", seq=%lld, node=%d, lat=%d\n",
				net->name, (long long) msg->seq, msg->dst_node_idx, lat);

			/* Stats */
			net->transfers++;
			net->lat_acc += lat;

			/* Free message and stack */
			net_msg_table_extract(net, msg->seq);
			repos_free_object(net_msg_repos, msg);
			net_stack_return(stack);
		} else
			esim_schedule_event(EV_NET_SEND, stack, 0);
	}
}





/*
 * Public Functions
 */

void net_init(void)
{
	net_stack_repos = repos_create(sizeof(struct net_stack_t), "net_stack");
	net_msg_repos = repos_create(sizeof(struct net_msg_t), "net_msg");
	EV_NET_SEND = esim_register_event(net_handler);
	EV_NET_RECEIVE = esim_register_event(net_handler);
}


void net_done(void)
{
	repos_free(net_stack_repos);
	repos_free(net_msg_repos);
}


struct net_t *net_create(char *name)
{
	struct net_t *net;
	net = calloc(1, sizeof(struct net_t));
	strncpy(net->name, name, sizeof(net->name));
	net->node_array_size = 1;
	net->nodes = calloc(net->node_array_size, sizeof(struct net_node_t));
	return net;
}


void net_free(struct net_t *net)
{
	int i, j;
	struct net_node_t *node;
	struct net_port_t *port;

	if (net->routing_table)
		free(net->routing_table);
	for (i = 0; i < net->node_count; i++) {
		node = &net->nodes[i];

		/* Input ports */
		for (j = 0; j < node->iport_count; j++) {
			port = &node->iports[j];
			if (port->buffer)
				free(port->buffer);
		}
		free(node->iports);

		/* Output ports */
		for (j = 0; j < node->oport_count; j++) {
			port = &node->oports[j];
			if (port->buffer)
				free(port->buffer);
			if (port->link)
				free(port->link);
		}
		free(node->oports);
	}
	free(net->nodes);
	free(net);
}


int net_new_node(struct net_t *net, char *name, void *data)
{
	net->end_node_count++;
	return net_allocate_node(net, net_node_end,
		1, 0,  /* 1 input port with no buffer */
		1, 0,  /* 1 output port with no buffer */
		1,  /* ignored xbar bandwidth */
		name, data);
}


int net_new_bus(struct net_t *net,
	int iport_count, int oport_count,
	int bandwidth, char *name, void *data)
{
	return net_allocate_node(net, net_node_bus,
		iport_count, 0,  /* No input buffers in bus */
		oport_count, 0,  /* No output buffers */
		bandwidth, name, data);
}


int net_new_switch(struct net_t *net,
	int iport_count, int ibuffer_size, int oport_count, int obuffer_size,
	int bandwidth, char *name, void *data)
{
	return net_allocate_node(net, net_node_switch,
		iport_count, ibuffer_size, oport_count, obuffer_size,
		bandwidth, name, data);
}


struct net_node_t *net_get_node(struct net_t *net, int node_idx)
{
	if (node_idx < 0 || node_idx >= net->node_count)
		net_error("network %s: node %d out of range",
			net->name, node_idx);
	return &net->nodes[node_idx];
}


void *net_get_node_data(struct net_t *net, int node_idx)
{
	struct net_node_t *node;
	node = net_get_node(net, node_idx);
	return node->data;
}


/* Return the next free input port. */
int net_get_iport_idx(struct net_t *net, int node_idx)
{
	int i;
	struct net_node_t *node;
	
	node = net_get_node(net, node_idx);
	for (i = 0; i < node->iport_count; i++)
		if (!node->iports[i].link)
			return i;
	return -1;
}


/* Return the next free input port. */
int net_get_oport_idx(struct net_t *net, int node_idx)
{
	int i;
	struct net_node_t *node;
	
	node = net_get_node(net, node_idx);
	for (i = 0; i < node->oport_count; i++)
		if (!node->oports[i].link)
			return i;
	return -1;
}


/* Create a new unidirectional link */
void net_new_link(struct net_t *net, int src_node_idx, int dst_node_idx, int bandwidth)
{
	int iport_idx, oport_idx;
	struct net_link_t *link;
	struct net_node_t *src_node, *dst_node;

	src_node = &net->nodes[src_node_idx];
	dst_node = &net->nodes[dst_node_idx];
	oport_idx = net_get_oport_idx(net, src_node_idx);
	iport_idx = net_get_iport_idx(net, dst_node_idx);
	if (oport_idx < 0)
		net_error("node %s.%s: no oport free",
			net->name, src_node->name);
	if (iport_idx < 0)
		net_error("node %s.%s: no iport free",
			net->name, dst_node->name);
	assert(oport_idx >= 0 && oport_idx < src_node->oport_count);
	assert(iport_idx >= 0 && iport_idx < dst_node->iport_count);

	/* Create link */
	link = calloc(1, sizeof(struct net_link_t));
	if (!link)
		net_error("net %s: out of memory");
	link->bandwidth = bandwidth;
	link->src_node_idx = src_node_idx;
	link->src_port_idx = oport_idx;
	link->dst_node_idx = dst_node_idx;
	link->dst_port_idx = iport_idx;

	/* Update ports */
	src_node->oports[oport_idx].link = link;
	dst_node->iports[iport_idx].link = link;
}


/* Create bidirectional link */
void net_new_bidirectional_link(struct net_t *net, int node1_idx, int node2_idx, int bandwidth)
{
	net_new_link(net, node1_idx, node2_idx, bandwidth);
	net_new_link(net, node2_idx, node1_idx, bandwidth);
}


void net_calculate_routes(struct net_t *net)
{
	int i, j, k;
	int next, port;
	struct net_link_t *link;

	/* Allocate routing table */
	if (net->routing_table)
		net_error("network %s: routing table already exists", net->name);
	net->routing_table = calloc(net->node_count * net->node_count, sizeof(struct routing_entry_t));
	if (!net->routing_table)
		net_error("network %s: out of memory allocating routing table", net->name);

	/* Initialize table with infinite costs */
	for (i = 0; i < net->node_count; i++) {
		for (j = 0; j < net->node_count; j++) {
			NET_COST(i, j) = i == j ? 0 : net->node_count;  /* infinite or 0 */
			NET_ROUTE(i, j) = -1;  /* no route */

		}
	}

	/* Set 1-jump connections */
	for (i = 0; i < net->node_count; i++) {
		for (j = 0; j < net->nodes[i].oport_count; j++) {
			link = net->nodes[i].oports[j].link;
			if (link) {
				NET_COST(i, link->dst_node_idx) = 1;
				NET_ROUTE(i, link->dst_node_idx) = link->dst_node_idx;
			}
		}
	}

	/* Calculate shortest paths Floyd-Warshall algorithm. The NET_ROUTE values do
	 * not necessarily point to the immediate next hop after this. */
	for (k = 0; k < net->node_count; k++) {
		for (i = 0; i < net->node_count; i++) {
			for (j = 0; j < net->node_count; j++) {
				if (NET_COST(i, k) + NET_COST(k, j) < NET_COST(i, j)) {
					NET_COST(i, j) = NET_COST(i, k) + NET_COST(k, j);
					NET_ROUTE(i, j) = k;
				}
			}
		}
	}

	/* Calculate NET_PORT values */
	for (i = 0; i < net->node_count; i++) {
		for (j = 0; j < net->node_count; j++) {
			next = NET_ROUTE(i, j);
			if (next < 0) {
				NET_PORT(i, j) = -1;
				continue;
			}
			while (NET_COST(i, next) > 1)
				next = NET_ROUTE(i, next);
			for (port = 0; port < net->nodes[i].oport_count; port++) {
				link = net->nodes[i].oports[port].link;
				if (link && link->dst_node_idx == next)
					break;
			}
			assert(port < net->nodes[i].oport_count);
			NET_PORT(i, j) = port;
		}
	}

	/* Update NET_ROUTES to point to the next hop */
	for (i = 0; i < net->node_count; i++) {
		for (j = 0; j < net->node_count; j++) {
			port = NET_PORT(i, j);
			if (port >= 0) {
				link = net->nodes[i].oports[port].link;
				assert(link);
				NET_ROUTE(i, j) = link->dst_node_idx;
			}
		}
	}
}


void net_dump_routes(struct net_t *net, FILE *f)
{
	int i, j, k;

	/* Routing table */
	fprintf(f, "         ");
	for (i = 0; i < net->node_count; i++)
		fprintf(f, "%2d ", i);
	fprintf(f, "\n");
	for (i = 0; i < net->node_count; i++) {
		fprintf(f, "node %2d: ", i);
		for (j = 0; j < net->node_count; j++) {
			if (NET_ROUTE(i, j) >= 0)
				fprintf(f, "%2d ", NET_ROUTE(i, j));
			else
				fprintf(f, "-- ");
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");

	/* Node combinations */
	for (i = 0; i < net->node_count; i++) {
		for (j = 0; j < net->node_count; j++) {
			fprintf(f, "from %2d to %2d: ", i, j);
			k = i;
			while (k != j) {
				if (NET_ROUTE(k, j) < 0) {
					fprintf(f, "x ");
					break;
				}
				fprintf(f, "%2d ", NET_ROUTE(k, j));
				k = NET_ROUTE(k, j);
			}
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
}


int net_valid_route(struct net_t *net, int src_node_idx, int dst_node_idx)
{
	net_get_node(net, src_node_idx);
	net_get_node(net, dst_node_idx);
	return NET_ROUTE(src_node_idx, dst_node_idx) != -1;
}


int net_can_send(struct net_t *net, int src_node_idx, int dst_node_idx)
{
	struct net_msg_t *msg;
	struct net_node_t *src_node, *dst_node;
	int result;

	/* Get nodes */
	src_node = net_get_node(net, src_node_idx);
	dst_node = net_get_node(net, dst_node_idx);
	if (src_node->kind != net_node_end || dst_node->kind != net_node_end)
		net_error("net_can_send: extreme nodes must be end nodes");

	/* Create message */
	msg = repos_create_object(net_msg_repos);
	msg->src_node_idx = src_node_idx;
	msg->dst_node_idx = dst_node_idx;
	msg->src_node = src_node;
	msg->dst_node = dst_node;
	msg->size = 1;

	/* Initial msg position */
	msg->node_idx = src_node_idx;
	msg->where = NET_WHERE_OBUFFER;
	msg->port_idx = 0;
	assert(!net->nodes[msg->node_idx].oports[msg->port_idx].buffer);

	/* Try to send */
	result = net_transfer(net, msg, NET_CAN_TRANSFER, 0, ESIM_EV_NONE, NULL);

	/* Free msg and return result */
	repos_free_object(net_msg_repos, msg);
	return result;
}


uint64_t net_send(struct net_t *net, int src_node_idx, int dst_node_idx, int size)
{
	return net_send_ev(net, src_node_idx, dst_node_idx, size,
		ESIM_EV_NONE, NULL);
}


uint64_t net_send_ev(struct net_t *net, int src_node_idx, int dst_node_idx, int size,
	int retevent, void *retstack)
{
	struct net_stack_t *stack;
	struct net_msg_t *msg;
	struct net_node_t *src_node, *dst_node;

	/* Check nodes */
	src_node = net_get_node(net, src_node_idx);
	dst_node = net_get_node(net, dst_node_idx);
	if (src_node->kind != net_node_end || dst_node->kind != net_node_end)
		net_error("net_send_ev: extreme nodes must be end nodes");

	/* Create message */
	msg = repos_create_object(net_msg_repos);
	msg->src_node_idx = src_node_idx;
	msg->dst_node_idx = dst_node_idx;
	msg->src_node = src_node;
	msg->dst_node = dst_node;
	msg->size = size;
	msg->seq = ++net->msg_seq;
	msg->send_cycle = esim_cycle;

	/* Insert message into hash table of in-flight messages */
	net_msg_table_insert(net, msg);

	/* Initial message position */
	msg->node_idx = src_node_idx;
	msg->where = NET_WHERE_OBUFFER;
	msg->port_idx = 0;
	assert(!net->nodes[msg->node_idx].oports[msg->port_idx].buffer);

	/* Call event */
	stack = net_stack_create(net, ESIM_EV_NONE, NULL);
	stack->msg = msg;
	stack->retevent = retevent;
	stack->retstack = retstack;
	esim_execute_event(EV_NET_SEND, stack);
	return msg->seq;
}


int net_in_transit(struct net_t *net, uint64_t seq)
{
	return net_msg_table_get(net, seq) != NULL;
}




/*
 * Debug
 */

FILE *net_debug_file;

int net_debug_init(char *filename)
{
	net_debug_file = strcmp(filename, "stdout") ?
		(strcmp(filename, "stderr") ?
		fopen(filename, "wt") : stderr) : stdout;
	return net_debug_file != NULL;
}

void net_debug_done()
{
	if (net_debug_file)
		fclose(net_debug_file);
}

void net_debug(char *fmt, ...)
{
	va_list va;

	if (!net_debug_file)
		return;
	va_start(va, fmt);
	vfprintf(net_debug_file, fmt, va);
}

