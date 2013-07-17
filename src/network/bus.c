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

#include "buffer.h"
#include "bus.h"
#include "net-system.h"
#include "node.h"


struct net_bus_t *net_bus_create(struct net_t *net, struct net_node_t *node,
	int bandwidth, char *name)
{
	struct net_bus_t *bus;

	bus = xcalloc(1, sizeof(struct net_bus_t));
	bus->net = net;
	bus->node = node;
	bus->name = xstrdup(name);
	if (bandwidth < 1)
		panic("%s: invalid bandwidth", __FUNCTION__);
	bus->bandwidth = bandwidth;
	return bus;
}

void net_bus_free(struct net_bus_t *bus)
{
	free(bus->name);
	free(bus);
}

/* this function always returns a bus from the same node that the buffer is
 * connected to. This bus is the one that is the least busy or not busy at
 * all. Remember it returns that one lane that was idle the most or will be
 * busy the least */

static struct net_buffer_t *net_bus_arbiteration_per_lane(struct net_node_t
	*bus_node, struct net_bus_t *bus)
{
	struct net_buffer_t *src_buffer;
	struct net_msg_t *msg;

	long long cycle;

	int last_input_node_index;
	int input_buffer_index;
	int input_buffer_count;
	int i;

	/* Checks */
	assert(bus->node == bus_node);
	assert(list_get(bus_node->bus_lane_list, bus->index) == bus);

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* If last scheduling decision was done in current cycle, return the
	 * same value. */
	if (bus->sched_when == cycle)
		return bus->sched_buffer;

	/* Make a new decision */
	bus->sched_when = cycle;
	input_buffer_count = list_count(bus_node->src_buffer_list);
	last_input_node_index = bus_node->last_node_index;

	/* Output buffer must be ready to be written */
	if (bus->busy >= cycle)
	{
		bus->sched_buffer = NULL;
		return NULL;
	}

	/* Find input buffer to fetch from */
	for (i = 0; i < input_buffer_count; i++)
	{
		input_buffer_index = (last_input_node_index + i + 1) 
			% input_buffer_count;
		src_buffer = list_get(bus_node->src_buffer_list, input_buffer_index);

		/* There must be a message at the head */
		msg = list_get(src_buffer->msg_list, 0);
		if (!msg)
			continue;

		/* Message must be ready */
		if (msg->busy >= cycle)
			continue;

		/* Input buffer must be ready to be read */
		if (src_buffer->read_busy >= cycle)
			continue;

		/* All conditions satisfied - schedule */
		bus_node->last_node_index = input_buffer_index;
		bus->sched_buffer = src_buffer;
		return src_buffer;
	}

	/* No input buffer ready */
	bus->sched_buffer = NULL;
	return NULL;
}

struct net_bus_t *net_bus_arbitration(struct net_node_t *bus_node,
	struct net_buffer_t *buffer)
{
	int i;
	struct net_bus_t *check_bus;

	/* checks */
	assert(buffer->bus);
	assert(!buffer->link);
	assert(bus_node->kind == net_node_bus);

	for (i = 0; i < list_count(bus_node->bus_lane_list); i++)
	{
		check_bus = list_get(bus_node->bus_lane_list, i);
		if (net_bus_arbiteration_per_lane(bus_node,
				check_bus) == buffer)
			return check_bus;
	}
	return NULL;
}

void net_bus_dump_report(struct net_bus_t *bus, FILE *f)
{
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	fprintf(f, "%s.Bandwidth = %d\n", bus->name, bus->bandwidth);
	fprintf(f, "%s.TransferredMessages = %lld\n", bus->name,
		bus->transferred_msgs);
	fprintf(f, "%s.TransferredBytes = %lld\n", bus->name,
		bus->transferred_bytes);
	fprintf(f, "%s.BusyCycles = %lld\n", bus->name, bus->busy_cycles);
	fprintf(f, "%s.BytesPerCycle = %.4f\n", bus->name, cycle ?
		(double) bus->transferred_bytes / cycle : 0.0);
	fprintf(f, "%s.Utilization = %.4f\n", bus->name, cycle ?
		(double) bus->transferred_bytes / (cycle *
			bus->bandwidth) : 0.0);
	fprintf(f, "\n");
}
