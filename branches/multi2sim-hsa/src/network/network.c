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
#include <unistd.h>

#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "buffer.h"
#include "bus.h"
#include "link.h"
#include "net-system.h"
#include "network.h"
#include "node.h"
#include "routing-table.h"
#include "visual.h"
#include "command.h"
#include "packet.h"


/* Insert a message into the in-flight messages hash table. */
void net_msg_table_insert(struct net_t *net, struct net_msg_t *msg)
{
	int index;

	index = msg->id % NET_MSG_TABLE_SIZE;
	assert(!msg->bucket_next);
	msg->bucket_next = net->msg_table[index];
	net->msg_table[index] = msg;
}


/* Return a message from the in-flight messages hash table */
struct net_msg_t *net_msg_table_get(struct net_t *net, long long id)
{
	int index;
	struct net_msg_t *msg;

	index = id % NET_MSG_TABLE_SIZE;
	msg = net->msg_table[index];
	while (msg && msg->id != id)
		msg = msg->bucket_next;
	return msg;
}


/* Extract a message from the in-flight messages hash table */
struct net_msg_t *net_msg_table_extract(struct net_t *net, long long id)
{
	int index;
	struct net_msg_t *prev, *msg;

	index = id % NET_MSG_TABLE_SIZE;
	prev = NULL;
	msg = net->msg_table[index];
	while (msg && msg->id != id)
	{
		prev = msg;
		msg = msg->bucket_next;
	}
	if (!msg)
		panic("%s: message %lld not in hash table", __FUNCTION__, id);
	if (prev)
		prev->bucket_next = msg->bucket_next;
	else
		net->msg_table[index] = msg->bucket_next;
	return msg;
}




/* 
 * Public Functions
 */

struct net_t *net_create(char *name)
{
	struct net_t *net;

	/* Initialize */
	net = xcalloc(1, sizeof(struct net_t));
	net->name = xstrdup(name);
	net->node_list = list_create();
	net->link_list = list_create();
	net->routing_table = net_routing_table_create(net);

	/* Return */
	return net;
}



void net_free(struct net_t *net)
{
	int i;

	/* Free nodes */
	for (i = 0; i < list_count(net->node_list); i++)
		net_node_free(list_get(net->node_list, i));
	list_free(net->node_list);

	/* Free links */
	for (i = 0; i < list_count(net->link_list); i++)
		net_link_free(list_get(net->link_list, i));
	list_free(net->link_list);

	/* Routing table */
	net_routing_table_free(net->routing_table);

	/* Free messages in flight */
	for (i = 0; i < NET_MSG_TABLE_SIZE; i++)
	{
		while (net->msg_table[i])
		{
			struct net_msg_t *next;

			next = net->msg_table[i]->bucket_next;
			net_msg_free(net->msg_table[i]);
			net->msg_table[i] = next;
		}
	}

	/* Network */
	free(net->name);
	free(net);
}


void net_dump(struct net_t *net, FILE *f)
{
	struct net_node_t *node;
	int i;

	/* Network name */
	fprintf(f, "*\n* Network %s\n*\n\n", net->name);

	/* Nodes */
	for (i = 0; i < list_count(net->node_list); i++)
	{
		fprintf(f, "Node %d\n", i);
		node = list_get(net->node_list, i);
		net_node_dump(node, f);
	}
	fprintf(f, "\n");
}


void net_dump_report(struct net_t *net, FILE *f)
{
	int i;

	/* General stats */
	fprintf(f, "[ Network.%s.General ]\n", net->name);
	fprintf(f, "Transfers = %lld\n", net->transfers);
	fprintf(f, "AverageMessageSize = %.2f\n", net->transfers ?
			(double) net->offered_bandwidth / net->transfers : 0.0);
	fprintf(f, "NetworkBandwdithDemand/AccumulatedMsgs = %lld\n",net->offered_bandwidth );
	fprintf(f, "AverageLatency = %.4f\n", net->transfers ?
			(double) net->lat_acc / net->transfers : 0.0);
	fprintf(f, "\n");

	/* Links */
	for (i = 0; i < list_count(net->link_list); i++)
	{
		struct net_link_t *link;

		link = list_get(net->link_list, i);
		net_link_dump_report(link, f);
	}

	/* Nodes */
	for (i = 0; i < list_count(net->node_list); i++)
	{
		struct net_node_t *node;

		node = list_get(net->node_list, i);
		net_node_dump_report(node, f);
	}
}

static void net_dump_routes(struct net_t *net, FILE *f)
{
        net_routing_table_dump(net->routing_table, f);
}

struct net_node_t *net_add_end_node(struct net_t *net,
		int input_buffer_size, int output_buffer_size,
		char *name, void *user_data)
{
	struct net_node_t *node;

	/* Create node */
	node = net_node_create(net,
			net_node_end,  /* kind */
			net->node_count,  /* index */
			input_buffer_size,
			output_buffer_size,
			0,  /* bandwidth */
			name,
			user_data);

	/* Add to list */
	net->node_count++;
	net->end_node_count++;
	list_add(net->node_list, node);

	/* Return */
	return node;
}


struct net_node_t *net_add_bus(struct net_t *net, int bandwidth, char *name, int lanes, int wire_delay)
{
	struct net_node_t *node;
	/* Create node */
	node = net_node_create(net,
			net_node_bus,  /* kind */
			net->node_count,  /* index */
			0,  /* input_buffer_size */
			0,  /* output_buffer_size */
			bandwidth,
			name,
			NULL);  /* user_data */

	/* Add to list */
	net->node_count++;
	list_add(net->node_list, node);

	node->bus_lane_list = list_create_with_size(4);
	node->src_buffer_list = list_create_with_size(4);
	node->dst_buffer_list = list_create_with_size(4);

	for (int i = 0; i < lanes; i++)
	{
		net_node_add_bus_lane(node, wire_delay);
	}
	/* Return */
	return node;
}


struct net_node_t *net_add_switch(struct net_t *net,
		int input_buffer_size, int output_buffer_size,
		int bandwidth, char *name)
{
	struct net_node_t *node;

	/* Create node */
	node = net_node_create(net,
			net_node_switch,  /* kind */
			net->node_count,  /* index */
			input_buffer_size,
			output_buffer_size,
			bandwidth,
			name,
			NULL);  /* user_data */

	/* Add to list */
	net->node_count++;
	list_add(net->node_list, node);

	/* Return */
	return node;
}

struct net_node_t *net_add_photonic(struct net_t *net, int input_buffer_size,
		int output_buffer_size, char *node_name,
		int waveguides, int wavelength )
{
	struct net_node_t *node;
	node = net_node_create(net,
			net_node_photonic,
			net->node_count,
			0,
			0,
			wavelength,
			node_name,
			NULL);
	net->node_count++;
	list_add(net->node_list, node);

	node->bus_lane_list = list_create_with_size(4);
	node->src_buffer_list = list_create_with_size(4);
	node->dst_buffer_list = list_create_with_size(4);

	for (int j = 0; j < waveguides ; j++)
	{
		net_node_add_photonic_link(node);
	}

	/* Return */
	return node;
}


/* Get a node by its name. If none found, return NULL. Search is
 * case-insensitive. */
struct net_node_t *net_get_node_by_name(struct net_t *net, char *name)
{
	struct net_node_t *node;
	int i;

	for (i = 0; i < list_count(net->node_list); i++)
	{
		node = list_get(net->node_list, i);
		if (!strcasecmp(node->name, name))
			return node;
	}
	return NULL;
}


/* Get a node by its user data. If none found, return NULL. */
struct net_node_t *net_get_node_by_user_data(struct net_t *net,
		void *user_data)
{
	struct net_node_t *node;
	int i;

	for (i = 0; i < list_count(net->node_list); i++)
	{
		node = list_get(net->node_list, i);
		if (node->user_data == user_data)
			return node;
	}
	return NULL;
}

/* Create link with virtual channel */
struct net_link_t *net_add_link(struct net_t *net,
		struct net_node_t *src_node, struct net_node_t *dst_node,
		int bandwidth, int link_src_bsize, int link_dst_bsize, int vc_count)
{
	struct net_link_t *link;

	/* Checks */
	assert(src_node->net == net);
	assert(dst_node->net == net);
	if (src_node->kind == net_node_end && dst_node->kind == net_node_end)
		fatal("network \"%s\": link cannot connect two end nodes\n",
				net->name);

	/* Create link connecting buffers */
	link = net_link_create(net, src_node, dst_node, bandwidth,
			link_src_bsize, link_dst_bsize, vc_count);

	/* Add to link list */
	list_add(net->link_list, link);

	/* Return */
	return link;
}


/* Create bidirectional link with VC */
void net_add_bidirectional_link(struct net_t *net,
		struct net_node_t *src_node, struct net_node_t *dst_node,
		int bandwidth, int link_src_bsize, int link_dst_bsize, int vc_count)
{
	int src_buffer_size;
	int dst_buffer_size;

	src_buffer_size = (link_src_bsize)
					? link_src_bsize : src_node->output_buffer_size;
	dst_buffer_size = (link_dst_bsize)
					? link_dst_bsize : dst_node->input_buffer_size;
	net_add_link(net, src_node, dst_node, bandwidth,
			src_buffer_size, dst_buffer_size, vc_count);

	src_buffer_size = (link_src_bsize)
					? link_src_bsize : dst_node->output_buffer_size;
	dst_buffer_size = (link_dst_bsize)
					? link_dst_bsize : src_node->input_buffer_size;
	net_add_link(net, dst_node, src_node, bandwidth,
			src_buffer_size, dst_buffer_size, vc_count);

}

void net_add_bus_port(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int bus_src_buffer, int bus_dst_buffer)
{
	/* Checks */
	assert(src_node->net == net);
	assert(dst_node->net == net);
	struct net_buffer_t *buffer;

	/* Condition 1: No support for direct BUS to BUS connection */
	if (src_node->kind == net_node_bus && dst_node->kind == net_node_bus)
		fatal("network \"%s\" : BUS to BUS connection is not supported.", net->name);

	/* Condition 2: In case source node is BUS, we add an input buffer to 
	 * destination node and add it to the list of destination nodes in
	 * BUS */
	if (src_node->kind == net_node_bus && dst_node->kind != net_node_bus)
	{
		int dst_buffer_size =
				(bus_dst_buffer) ? bus_dst_buffer : dst_node->
						input_buffer_size;

		buffer = net_node_add_input_buffer(dst_node, dst_buffer_size);

		assert(!buffer->link);
		list_add(src_node->dst_buffer_list, buffer);
		buffer->bus = list_get(src_node->bus_lane_list, 0);
	}

	/* Condition 3: In case destination node is BUS, we add an output
	 * buffer to source node and add it to the list of source nodes in
	 * BUS */
	else if (src_node->kind != net_node_bus
			&& dst_node->kind == net_node_bus)
	{
		int src_buffer_size =
				(bus_src_buffer) ? bus_src_buffer : src_node->
						output_buffer_size;

		buffer = net_node_add_output_buffer(src_node,
				src_buffer_size);
		assert(!buffer->link);
		list_add(dst_node->src_buffer_list, buffer);
		buffer->bus = list_get(dst_node->bus_lane_list, 0);
	}

	/* We make the buffer as type port for clarification and use in event 
	 * handler */
	buffer->kind = net_buffer_bus;

}

void net_add_channel_port(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int bus_src_buffer, int bus_dst_buffer)
{
	/* Checks */
	assert(src_node->net == net);
	assert(dst_node->net == net);
	struct net_buffer_t *buffer;

	/* Condition 1: No support for direct BUS to BUS connection */
	if (src_node->kind == net_node_photonic && dst_node->kind == net_node_photonic)
		fatal("network \"%s\" : BUS to BUS connection is not supported.", net->name);

	/* Condition 2: In case source node is BUS, we add an input buffer to
	 * destination node and add it to the list of destination nodes in
	 * BUS */
	if (src_node->kind == net_node_photonic && dst_node->kind != net_node_photonic)
	{
		int dst_buffer_size =
				(bus_dst_buffer) ? bus_dst_buffer : dst_node->
						input_buffer_size;

		buffer = net_node_add_input_buffer(dst_node, dst_buffer_size);

		assert(!buffer->link);
		list_add(src_node->dst_buffer_list, buffer);
		buffer->bus = list_get(src_node->bus_lane_list, 0);
	}

	/* Condition 3: In case destination node is BUS, we add an output
	 * buffer to source node and add it to the list of source nodes in
	 * BUS */
	else if (src_node->kind != net_node_photonic
			&& dst_node->kind == net_node_photonic)
	{
		int src_buffer_size =
				(bus_src_buffer) ? bus_src_buffer : src_node->
						output_buffer_size;

		buffer = net_node_add_output_buffer(src_node,
				src_buffer_size);
		assert(!buffer->link);
		list_add(dst_node->src_buffer_list, buffer);
		buffer->bus = list_get(dst_node->bus_lane_list, 0);
	}

	/* We make the buffer as type port for clarification and use in event
	 * handler */
	buffer->kind = net_buffer_photonic;

}

void net_add_bidirectional_bus_port(struct net_t *net,
		struct net_node_t *src_node, struct net_node_t *dst_node,
		int bus_src_buffer, int bus_dst_buffer)
{
	net_add_bus_port(net, src_node, dst_node, bus_src_buffer,
			bus_dst_buffer);
	net_add_bus_port(net, dst_node, src_node, bus_src_buffer,
			bus_dst_buffer);
}


void net_add_bidirectional_channel_port(struct net_t *net,
		struct net_node_t *src_node, struct net_node_t *dst_node,
		int bus_src_buffer, int bus_dst_buffer)
{
	net_add_channel_port(net, src_node, dst_node, bus_src_buffer,
			bus_dst_buffer);
	net_add_channel_port(net, dst_node, src_node, bus_src_buffer,
			bus_dst_buffer);
}

/* Return TRUE if a message can be sent through the network. Return FALSE
 * otherwise, whether the reason is temporary of permanent. This function is
 * being used just in stand alone simulator. So in here if a buffer is busy
 * for any reason, the node simply discards the message and sends another
 * one. */
int net_can_send(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size)
{
	struct net_routing_table_t *routing_table = net->routing_table;
	struct net_routing_table_entry_t *entry;
	struct net_buffer_t *output_buffer;
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* Get output buffer */
	entry = net_routing_table_lookup(routing_table, src_node, dst_node);
	output_buffer = entry->output_buffer;


	/* No route to destination */
	if (!output_buffer)
		return 0;

	/* Output buffer is busy */
	if (output_buffer->write_busy >= cycle)
		return 0;

	/* Message does not fit in output buffer */
	int required_size = size;
	if (net->packet_size != 0)
		required_size = ((size - 1) / net->packet_size + 1)*net->packet_size;

	if (output_buffer->count + required_size > output_buffer->size) // size is replaced by required size
		return 0;

	/* All conditions satisfied, can send */
	return 1;
}


/* Return TRUE if a message can be sent to the network. If it cannot be sent, 
 * return FALSE, and schedule 'retry_event' for the cycle when the check
 * should be performed again. This function should not be called if the
 * reason why a message cannot be sent is permanent (e.g., no route to
 * destination). */
int net_can_send_ev(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size,
		int retry_event, void *retry_stack)
{
	struct net_routing_table_t *routing_table = net->routing_table;
	struct net_routing_table_entry_t *entry;
	struct net_buffer_t *output_buffer;
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* Get output buffer */
	entry = net_routing_table_lookup(routing_table, src_node, dst_node);
	output_buffer = entry->output_buffer;

	/* No route to destination */
	if (!output_buffer)
		fatal("%s: no route between %s and %s.\n%s",
				net->name, src_node->name, dst_node->name,
				net_err_no_route);

	/* Message is too long */
	if (size > output_buffer->size)
		fatal("%s: message too long.\n%s", net->name,
				net_err_large_message);

	/* Output buffer is busy */
	if (output_buffer->write_busy >= cycle)
	{
		esim_schedule_event(retry_event, retry_stack,
				output_buffer->write_busy - cycle + 1);
		return 0;
	}

	int required_size = size;
	if (net->packet_size != 0)
		required_size = ((size - 1) / net->packet_size + 1)*net->packet_size;
	if (output_buffer->count + required_size > output_buffer->size)
	{
		net_buffer_wait(output_buffer, retry_event, retry_stack);
		return 0;
	}
	/* All conditions satisfied, can send */
	return 1;
}


/* Send a message through the network. When using this function, the caller
 * must make sure that the message can be injected in the network, calling
 * 'net_can_send' first. When the message is received in 'dst_node' it will
 * be removed automatically from its input buffer, and the 'msg' object
 * return by this function will be invalid. */
struct net_msg_t *net_send(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size)
{
	return net_send_ev(net, src_node, dst_node, size, ESIM_EV_NONE, NULL);
}


/* Send a message through network. The caller must make sure that the message 
 * can be injected in the network, with a previous call to 'net_can_send'.
 * When the message reaches the head of the input buffer in 'dst_node', event
 * 'receive_event' will be scheduled, and object 'msg' will be accessible. The 
 * message needs to be removed by the caller with an additional call to
 * 'net_receive', which will invalidate and free the 'msg' object. */
struct net_msg_t *net_send_ev(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size, int receive_event,
		void *receive_stack)
{
	struct net_stack_t *stack;
	struct net_msg_t *msg;
	struct net_packet_t *pkt;
	/* Check nodes */
	if (src_node->kind != net_node_end || dst_node->kind != net_node_end)
		fatal("%s: not end nodes.\n%s", __FUNCTION__,
				net_err_end_nodes);

	/* Create message */
	msg = net_msg_create(net, src_node, dst_node, size);

	net_trace("net.new_msg net=\"%s\" name=\"M-%lld\" size=%d state=\"%s:create\"\n",
			net->name, msg->id, msg->size, src_node->name);

	/* Insert message into hash table of in-flight messages */
	net_msg_table_insert(net, msg);

	//new to make sure packet size is now equal to msg size
	if (net->packet_size == 0)
		net_packetizer(net, msg, msg->size);
	else
	{
		net_packetizer(net, msg, net->packet_size);
		net_trace("net.msg net=\"%s\" name=\"M-%lld\" state=\"%s:packetize\"\n",
					net->name, msg->id, src_node->name);
	}


	for (int i = 0; i < msg->packet_list_max; i++)
	{
		/* Start event-driven simulation */
		pkt = msg->packet_list_head;

		stack = net_stack_create(net, ESIM_EV_NONE, NULL);
		stack->packet = pkt;
		stack->ret_event = receive_event;
		stack->ret_stack = receive_stack;
		esim_execute_event(EV_NET_SEND, stack);
		DOUBLE_LINKED_LIST_REMOVE(msg, packet, pkt);
	}
	/* Return created message */
	return msg;

}


struct net_msg_t *net_try_send(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size,
		int retry_event, void *retry_stack)
{
	/* Check if network is available */
	if (!net_can_send_ev(net, src_node, dst_node, size, retry_event,
			retry_stack))
		return NULL;

	/* Send message */
	return net_send(net, src_node, dst_node, size);
}


struct net_msg_t *net_try_send_ev(struct net_t *net,
		struct net_node_t *src_node, struct net_node_t *dst_node, int size,
		int receive_event, void *receive_stack, int retry_event,
		void *retry_stack)
{
	/* Check if network is available */
	if (!net_can_send_ev(net, src_node, dst_node, size, retry_event,
			retry_stack))
		return NULL;

	/* Send message */
	return net_send_ev(net, src_node, dst_node, size, receive_event,
			receive_stack);
}


/* Absorb a message at the head of the input buffer of an end node */
void net_receive(struct net_t *net, struct net_node_t *node,
		struct net_msg_t *msg)
{
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* Checks */
	assert(node->net == net);
	assert(msg->net == net);

	/* Net Info update */
	net->transfers++;
	net->lat_acc += cycle - msg->send_cycle;
	net->offered_bandwidth += msg->size;

	net_msg_table_extract(net, msg->id);

	net_trace("net.msg net=\"%s\" name=\"M-%lld\" state=\"%s:receive\"\n",
			net->name, msg->id, node->name);
	if (net_tracing())
	{
		struct net_packet_t *pkt;
		DOUBLE_LINKED_LIST_FOR_EACH(msg, packet, pkt)
		{
			net_trace("net.end_packet net=\"%s\" name=\"P-%lld:%d\"\n",
					net->name, msg->id, pkt->session_id );

		}
		net_trace("net.end_msg net=\"%s\" name=\"M-%lld\"\n",
				net->name, msg->id);

	}

	net_msg_free(msg);
}

void net_config_trace(struct net_t *net)
{
        int i, j;

        net_trace_header("net.create name=\"%s\" num_nodes=\"%d\" packet_size=\"%d\"\n",
                        net->name, net->node_count, net->packet_size);
        /* Nodes Trace */
        LIST_FOR_EACH(net->node_list, i)
        {
                struct net_node_t *node;
                node = list_get(net->node_list, i);

                if (node->kind == net_node_switch || node->kind == net_node_end)
                        net_trace_header("net.node net_name=\"%s\" node_index=\"%d\" node_name=\"%s\" node_type=%d\n"
                                        , net->name, node->index, node->name, node->kind);
                else if (node->kind == net_node_bus || node->kind == net_node_photonic)
                        net_trace_header("net.node net_name=\"%s\" node_index=\"%d\" node_name=\"%s\" node_type=%d "
                                        "num_lanes=%d\n", net->name, node->index, node->name, node->kind,
                                        list_count(node->bus_lane_list));
        }

        /* Links Trace */
        LIST_FOR_EACH(net->link_list, i)
        {
                struct net_link_t *link;
                link = list_get(net->link_list, i);

                net_trace_header("net.link net_name=\"%s\" link_name=\"%s\" src_node=\"%s\" "
                                "dst_node=\"%s\" vc_num=%d bw=%d\n", net->name, link->name, link->src_node->name,
                                link->dst_node->name, link->virtual_channel, link->bandwidth);
        }

        /* Buffer Trace */
        LIST_FOR_EACH(net->node_list, i)
        {
                struct net_node_t *node;
                node = list_get(net->node_list, i);

                LIST_FOR_EACH(node->input_buffer_list, j)
                {
                        struct net_buffer_t *buffer;
                        buffer = list_get(node->input_buffer_list, j);

                        if (buffer->kind == net_buffer_link)
                                net_trace_header("net.input_buffer net_name=\"%s\" node_name=\"%s\" buffer_name=\"%s\" "
                                                "buffer_size=%d buffer_type=%d connection=\"%s\"\n", net->name,
                                                node->name, buffer->name, buffer->size, buffer->kind,
                                                buffer->link->name);

                        else if (buffer->kind == net_buffer_bus || buffer->kind == net_buffer_photonic)
                                net_trace_header("net.input_buffer net_name=\"%s\" node_name=\"%s\" buffer_name=\"%s\" "
                                                "buffer_size=%d buffer_type=%d connection=\"%s\"\n", net->name,
                                                node->name, buffer->name, buffer->size, buffer->kind,
                                                buffer->bus->node->name);
                }
                LIST_FOR_EACH(node->output_buffer_list, j)
                {
                        struct net_buffer_t *buffer;
                        buffer = list_get(node->output_buffer_list, j);

                        if (buffer->kind == net_buffer_link)
                                net_trace_header("net.output_buffer net_name=\"%s\" node_name=\"%s\" buffer_name=\"%s\" "
                                                "buffer_size=%d buffer_type=%d connection=\"%s\"\n", net->name,
                                                node->name, buffer->name, buffer->size, buffer->kind,
                                                buffer->link->name);

                        else if (buffer->kind == net_buffer_bus || buffer->kind == net_buffer_photonic)
                                net_trace_header("net.output_buffer net_name=\"%s\" node_name=\"%s\" buffer_name=\"%s\" "
                                                "buffer_size=%d buffer_type=%d connection=\"%s\"\n", net->name,
                                                node->name, buffer->name, buffer->size, buffer->kind,
                                                buffer->bus->node->name);
                }
        }
}


void net_bandwidth_snapshot(struct net_t *net, long long cycle)
{
        /* Dump line to data file */

	if (!net->offered_bandwidth_data_file)
	{
		net->offered_bandwidth_data_file =
                        file_create_temp(net->offered_bandwidth_file_name,
                                        MAX_PATH_SIZE);
	        fprintf(net->offered_bandwidth_data_file, "%lld %lld\n", (long long) 0, (long long) 0);
	}

	if (!net->topology_bandwidth_data_file)
	{
		net->topology_bandwidth_data_file =
                        file_create_temp(net->topology_bandwidth_file_name,
                                        MAX_PATH_SIZE);
	        fprintf(net->topology_bandwidth_data_file, "%lld %lld\n",(long long) 0, (long long) 0);
	}
        fprintf(net->offered_bandwidth_data_file, "%lld %lld\n", cycle,
                net->offered_bandwidth - net->last_recorded_net_bw);

        fprintf(net->topology_bandwidth_data_file, "%lld %lld\n", cycle,
                net->topology_util_bw - net->last_recorded_topo_bw);

        net->last_recorded_cycle = cycle / net_snap_period;
        net->last_recorded_net_bw = net->offered_bandwidth;
        net->last_recorded_topo_bw  = net->topology_util_bw;

}

void net_dump_snapshot(struct net_t *net)
{
        /* First Close the Files */
	if (!net->offered_bandwidth_data_file || !net->topology_bandwidth_data_file)
		return;

        fclose(net->offered_bandwidth_data_file);
        fclose(net->topology_bandwidth_data_file);

        FILE *script_file;

        char net_bw_plot_file_name[MAX_PATH_SIZE];
        char topo_bw_plot_file_name[MAX_PATH_SIZE];
        char script_file_name[MAX_PATH_SIZE];
        char cmd[MAX_PATH_SIZE];

        int err;

        snprintf(net_bw_plot_file_name, MAX_PATH_SIZE, "%s.%d.offered_bw.eps",
                net->name, net_snap_period);
        if (!file_can_open_for_write(net_bw_plot_file_name))
                fatal("%s: cannot write Network Offered Bandwidth plot",
                                net_bw_plot_file_name);

        snprintf(topo_bw_plot_file_name, MAX_PATH_SIZE, "%s.%d.topology_bw.eps",
                net->name, net_snap_period);
        if (!file_can_open_for_write(topo_bw_plot_file_name))
                fatal("%s: cannot write Topology Offered Bandwidth plot",
                                topo_bw_plot_file_name);

        /* Generate gnuplot script for Offered Bandwidth */
        script_file = file_create_temp(script_file_name, MAX_PATH_SIZE);
        fprintf(script_file, "set term postscript eps color solid\n");
        fprintf(script_file, "set nokey\n");
        fprintf(script_file, "set title 'Offered Bandwidth - Sampled at %d Cycles '\n",
                        net_snap_period );
        fprintf(script_file, "set xlabel 'Time(Cycle)'\n");
        fprintf(script_file, "set ylabel 'Network Bandwidth (Byte)'\n");
        fprintf(script_file, "set xrange [0:%lld]\n", net->last_recorded_cycle * net_snap_period);
        fprintf(script_file, "set yrange [0:]\n");
        fprintf(script_file, "set size 0.65, 0.5\n");
        fprintf(script_file, "set grid ytics\n");
        fprintf(script_file, "plot '%s' w linespoints lt 4 "
                "lw 1 pt 2 ps .5 \n", net->offered_bandwidth_file_name);
        fclose(script_file);

        /* Plot */
        sprintf(cmd, "gnuplot %s > %s", script_file_name, net_bw_plot_file_name);
        err = system(cmd);
        if (err)
                warning("could not execute gnuplot, when creating network results\n");

        /* Remove temporary files */
        unlink(net->offered_bandwidth_file_name);
        unlink(script_file_name);

        /* Generate gnuplot script for Topology Bandwidth */
        script_file = file_create_temp(script_file_name, MAX_PATH_SIZE);
        fprintf(script_file, "set term postscript eps color solid\n");
        fprintf(script_file, "set nokey\n");
        fprintf(script_file, "set title 'Topology Bandwidth - Sampled at %d Cycles '\n",
                        net_snap_period );
        fprintf(script_file, "set xlabel 'Time(Cycle)'\n");
        fprintf(script_file, "set ylabel 'Topology Bandwidth (Byte)'\n");
        fprintf(script_file, "set xrange [0:%lld]\n", net->last_recorded_cycle * net_snap_period);
        fprintf(script_file, "set yrange [0:]\n");
        fprintf(script_file, "set size 0.65, 0.5\n");
        fprintf(script_file, "set grid ytics\n");
        fprintf(script_file, "plot '%s' w linespoints lt 5 "
                "lw 1 pt 3 ps .5 \n", net->topology_bandwidth_file_name);
        fclose(script_file);

        /* Plot */
        sprintf(cmd, "gnuplot %s > %s", script_file_name, topo_bw_plot_file_name);
        err = system(cmd);
        if (err)
                warning("could not execute gnuplot, when creating network results\n");

        /* Remove temporary files */
        unlink(net->topology_bandwidth_file_name);
        unlink(script_file_name);


}

void net_initiation(struct net_t *net)
{
        char file_name_dest[MAX_STRING_SIZE];

        /* Report file */
        if (*net_report_file_name)
        {
                snprintf(file_name_dest, sizeof file_name_dest, "%s_%s",
                                net->name, net_report_file_name);
                net->report_file = file_open_for_write(file_name_dest);
                if (!net->report_file)
                        fatal("%s: cannot write on network report file",
                                        net_report_file_name);
        }

        /* Dump Routes */
        if (*net_route_file_name)
        {
                snprintf(file_name_dest, sizeof file_name_dest, "%s_%s",
                                net->name, net_route_file_name);
                net->route_file = file_open_for_write(file_name_dest);
                if (!net->route_file)
                        fatal("%s: cannot write on network visualization file",
                                        net_route_file_name);
        }

        /* Visualization File */
        if (*net_visual_file_name)
        {
                snprintf(file_name_dest, sizeof file_name_dest, "%s_%s",
                                net->name, net_visual_file_name);
                net->visual_file = file_open_for_write(file_name_dest);
                if (!net->visual_file)
                        fatal("%s: cannot write on network visualization file",
                                        net_visual_file_name);
        }
}

void net_individual_done(struct net_t *net)
{
        /* Dump report for network */
        if (net->report_file)
                net_dump_report(net, net->report_file);
        /* Close report file */
        file_close(net->report_file);

        if (net->route_file)
                net_dump_routes(net ,net->route_file);
        file_close(net->route_file);

        /* Dump Visualization data in a 'graphplot'
         * compatible file */
        if (net->visual_file)
        {
                struct net_graph_t *graph = net_graph_visual_calculation(net);
                net_dump_visual(graph, net->visual_file);
                net_graph_free(graph);
        }
        /* Close visualization file */
        file_close(net->visual_file);

        if (net_snap_period)
        {
                net_dump_snapshot(net);
        }
        /* Free network */
        net_free(net);
}
