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

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "buffer.h"
#include "bus.h"
#include "net-system.h"
#include "network.h"
#include "node.h"
#include "routing-table.h"


/* 
 * Public functions
 */

struct net_node_t *net_node_create(struct net_t *net,
	enum net_node_kind_t kind, int index,
	int input_buffer_size, int output_buffer_size,
	int bandwidth, char *name, void *user_data)
{
	struct net_node_t *node;

	/* Fields */
	node = xcalloc(1, sizeof(struct net_node_t));
	node->net = net;
	node->name = xstrdup(name);
	node->kind = kind;
	node->index = index;
	node->user_data = user_data;
	node->bandwidth = bandwidth;
	node->input_buffer_size = input_buffer_size;
	node->output_buffer_size = output_buffer_size;
	if (kind != net_node_end && bandwidth < 1)
		panic("%s: invalid bandwidth", __FUNCTION__);
	if (net_get_node_by_name(net, name))
		fatal("%s: duplicated node name.\n%s", net->name,
			net_err_node_name_duplicate);

	/* Lists of ports */
	node->output_buffer_list = list_create_with_size(4);
	node->input_buffer_list = list_create_with_size(4);

	return node;
}

struct net_buffer_t *net_get_buffer_by_name(struct net_node_t *node,
		char *buffer_name)
{
	struct net_buffer_t *buffer;

	int i;
	for (i = 0; i < list_count(node->input_buffer_list); i++)
	{
		buffer = list_get(node->input_buffer_list, i);
		if (!strcasecmp(buffer->name, buffer_name))
			return buffer;
	}
	for (i = 0; i < list_count(node->output_buffer_list); i++)
	{
		buffer = list_get(node->output_buffer_list, i);
		if (!strcasecmp(buffer->name, buffer_name))
			return buffer;
	}
	return NULL;
}

void net_node_free(struct net_node_t *node)
{
	int i;

	/* Free input buffers */
	for (i = 0; i < list_count(node->input_buffer_list); i++)
		net_buffer_free(list_get(node->input_buffer_list, i));
	list_free(node->input_buffer_list);

	/* Free output buffers */
	for (i = 0; i < list_count(node->output_buffer_list); i++)
		net_buffer_free(list_get(node->output_buffer_list, i));
	list_free(node->output_buffer_list);


	/* For BUS */
	if (node->kind == net_node_bus)
	{
		/* Freeing associated bus structures */
		for (i = 0; i < list_count(node->bus_lane_list); i++)
			net_bus_free(list_get(node->bus_lane_list, i));
		list_free(node->bus_lane_list);

		/* Freeing lists that keeps track of src and dst buffers */
		if (node->src_buffer_list)
			list_free(node->src_buffer_list);
		if (node->dst_buffer_list)
			list_free(node->dst_buffer_list);
	}

	/* Free node */
	free(node->name);
	free(node);
}


void net_node_dump(struct net_node_t *node, FILE *f)
{
	fprintf(f, "\tName = %s\n", node->name);
	fprintf(f, "\tNumber of input buffers = %d\n",
		list_count(node->input_buffer_list));
	fprintf(f, "\tNumber of output buffers = %d\n",
		list_count(node->output_buffer_list));
}


void net_node_dump_report(struct net_node_t *node, FILE *f)
{
	struct net_t *net = node->net;
	struct net_buffer_t *buffer;
	long long cycle;
	int i;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* General */
	fprintf(f, "[ Network.%s.Node.%s ]\n", net->name, node->name);

	/* Statistics */
	if (node->kind != net_node_bus)
	{
		/* Configuration */
		if (node->kind != net_node_end)
			fprintf(f, "Config.BandWidth = %d\n",
				node->bandwidth);

		fprintf(f, "SentMessages = %lld\n", node->msgs_sent);
		fprintf(f, "SentBytes = %lld\n", node->bytes_sent);
		fprintf(f, "SendRate = %.4f\n", cycle ?
			(double) node->bytes_sent / cycle : 0.0);
		fprintf(f, "ReceivedMessages = %lld\n", node->msgs_received);
		fprintf(f, "ReceivedBytes = %lld\n", node->bytes_received);
		fprintf(f, "ReceiveRate = %.4f\n", cycle ?
			(double) node->bytes_received / cycle : 0.0);

		/* Input buffers */
		for (i = 0; i < list_count(node->input_buffer_list); i++)
		{
			buffer = list_get(node->input_buffer_list, i);
			net_buffer_dump_report(buffer, f);
		}

		/* Output buffers */
		for (i = 0; i < list_count(node->output_buffer_list); i++)
		{
			buffer = list_get(node->output_buffer_list, i);
			net_buffer_dump_report(buffer, f);
		}
	}
	else if (node->kind == net_node_bus)
	{
		struct net_bus_t *bus;

		for (i = 0; i < list_count(node->bus_lane_list); i++)
		{
			bus = list_get(node->bus_lane_list, i);
			fprintf(f, "BUS Lane %d :\n", i);
			net_bus_dump_report(bus, f);
		}
	}

	/* Finish */
	fprintf(f, "\n");
}


struct net_buffer_t *net_node_add_input_buffer(struct net_node_t *node,
	int bandwidth)
{
	struct net_buffer_t *buffer;
	char name[MAX_STRING_SIZE];

	snprintf(name, sizeof(name), "in_buf_%d",
		list_count(node->input_buffer_list));
	buffer = net_buffer_create(node->net, node, bandwidth, name);
	buffer->index = list_count(node->input_buffer_list);
	list_add(node->input_buffer_list, buffer);
	return buffer;
}



struct net_buffer_t *net_node_add_output_buffer(struct net_node_t *node,
	int bandwidth)
{
	struct net_buffer_t *buffer;
	char name[MAX_STRING_SIZE];

	snprintf(name, sizeof(name), "out_buf_%d",
		list_count(node->output_buffer_list));
	buffer = net_buffer_create(node->net, node, bandwidth, name);
	buffer->index = list_count(node->output_buffer_list);
	list_add(node->output_buffer_list, buffer);
	return buffer;
}

struct net_bus_t *net_node_add_bus_lane(struct net_node_t *node)
{
	assert(node->kind == net_node_bus);
	struct net_bus_t *bus;
	char name[MAX_STRING_SIZE];

	snprintf(name, sizeof(name), "%s_bp_%d", node->name,
		list_count(node->bus_lane_list));
	bus = net_bus_create(node->net, node, node->bandwidth, name);
	bus->index = list_count(node->bus_lane_list);
	list_add(node->bus_lane_list, bus);
	return bus;
}


/* Round-robin scheduler for network switch, choosing between several
 * candidate messages at the head of all input buffers that have a given
 * output buffer as an immediate target. */
struct net_buffer_t *net_node_schedule(struct net_node_t *node,
	struct net_buffer_t *output_buffer)
{
	struct net_t *net = node->net;
	struct net_routing_table_t *routing_table = net->routing_table;

	struct net_routing_table_entry_t *entry;
	struct net_buffer_t *input_buffer;
	struct net_msg_t *msg;

	long long cycle;

	int last_input_buffer_index;
	int input_buffer_index;
	int input_buffer_count;
	int i;

	/* Checks */
	assert(output_buffer->node == node);
	assert(list_get(node->output_buffer_list,
			output_buffer->index) == output_buffer);

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* If last scheduling decision was done in current cycle, return the
	 * same value. */
	if (output_buffer->sched_when == cycle)
		return output_buffer->sched_buffer;

	/* Make a new decision */
	output_buffer->sched_when = cycle;
	input_buffer_count = list_count(node->input_buffer_list);
	last_input_buffer_index = output_buffer->sched_buffer ?
		output_buffer->sched_buffer->index : 0;

	/* Output buffer must be ready to be written */
	if (output_buffer->write_busy >= cycle)
	{
		output_buffer->sched_buffer = NULL;
		return NULL;
	}

	/* Find input buffer to fetch from */
	for (i = 0; i < input_buffer_count; i++)
	{
		input_buffer_index =
			(last_input_buffer_index + i +
			1) % input_buffer_count;
		input_buffer =
			list_get(node->input_buffer_list, input_buffer_index);

		/* There must be a message at the head */
		msg = list_get(input_buffer->msg_list, 0);
		if (!msg)
			continue;

		/* Message must be ready */
		if (msg->busy >= cycle)
			continue;

		/* Input buffer must be ready to be read */
		if (input_buffer->read_busy >= cycle)
			continue;

		/* Message must target this output buffer */
		entry = net_routing_table_lookup(routing_table, node,
			msg->dst_node);
		assert(entry->output_buffer);
		if (entry->output_buffer != output_buffer)
			continue;

		/* Message must fit in this output buffer */
		if (output_buffer->count + msg->size > output_buffer->size)
			continue;

		/* All conditions satisfied - schedule */
		output_buffer->sched_buffer = input_buffer;
		return input_buffer;
	}

	/* No input buffer ready */
	output_buffer->sched_buffer = NULL;
	return NULL;
}
