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

#include <network.h>


static int net_link_busy(struct net_t *net, uint64_t busy)
{
	return busy / net->link_width > esim_cycle;
}


static void net_link_set_busy(struct net_t *net, uint64_t *pbusy, int size)
{
	assert(size > 0);
	*pbusy = MAX(*pbusy, esim_cycle * net->link_width - 1);
	*pbusy += size;
}


static int net_link_delay(struct net_t *net, uint64_t busy)
{
	uint64_t cycle;
	cycle = busy / net->link_width;
	return cycle >= esim_cycle ? cycle - esim_cycle + 1 : 0;
}




/* Event handler */

static int EV_NET_SEND;
static int EV_NET_DELIVER;

static int EV_NET_BUS_SEND;
static int EV_NET_BUS_DELIVER;

static void net_handler(int event, void *data)
{
	struct net_stack_t *stack = data;
	struct net_t *net = stack->net;

	if (event == EV_NET_BUS_SEND)
	{
		/* If bus is busy, schedule send for later. If the bus is busy with a packet
		 * with the same ID, it's a broadcast, so we can send packet. */
		if (net_link_busy(net, net->bus_busy) && net->bus_packet_id != stack->packet_id) {
			esim_schedule_event(EV_NET_BUS_SEND, stack, net_link_delay(net, net->bus_busy));
			return;
		}

		/* Send */
		net->bus_busy = esim_cycle;
		net->bus_packet_id = stack->packet_id;
		net_link_set_busy(net, &net->bus_busy, stack->size);
		esim_schedule_event(EV_NET_BUS_DELIVER, stack, net_link_delay(net, net->bus_busy));
		return;
	}

	if (event == EV_NET_BUS_DELIVER)
	{
		net->delivered++;
		net->deliver_time += esim_cycle - stack->send_time;
		net_stack_return(stack);
		return;
	}

	if (event == EV_NET_SEND)
	{
		int route;
		uint64_t *pbusy;

		/* If there is no route to destination, error */
		route = net->nodes[stack->src].con[stack->dst].route;
		if (route < 0)
			fatal("net '%s': no route from %d to %d",
				net->name, stack->src, stack->dst);

		/* If link is busy, schedule send for later */
		pbusy = &net->nodes[stack->src].con[route].busy;
		if (net_link_busy(net, *pbusy)) {
			esim_schedule_event(EV_NET_SEND, stack, net_link_delay(net, *pbusy));
			return;
		}

		/* Send. It takes 1 cycle. */
		net_link_set_busy(net, pbusy, stack->size);
		esim_schedule_event(EV_NET_DELIVER, stack, net_link_delay(net, *pbusy));
		return;
	}

	if (event == EV_NET_DELIVER)
	{

		/* If final destination is current node, done. */
		if (net->nodes[stack->src].con[stack->dst].route == stack->dst) {
			net->delivered++;
			net->deliver_time += esim_cycle - stack->send_time;
			net_stack_return(stack);
			return;
		}

		/* Forward packet */
		stack->src = net->nodes[stack->src].con[stack->dst].route;
		esim_schedule_event(EV_NET_SEND, stack, 0);
		return;
	}

	abort();
}




/* Network stack */

static struct repos_t *net_stack_repos;

struct net_stack_t *net_stack_create(struct net_t *net,
	int retevent, void *retstack)
{
	struct net_stack_t *stack;
	stack = repos_create_object(net_stack_repos);
	stack->net = net;
	stack->retevent = retevent;
	stack->retstack = retstack;
	return stack;
}


void net_stack_return(struct net_stack_t *stack)
{
	int retevent = stack->retevent;
	struct network_stack_t *retstack = stack->retstack;
	repos_free_object(net_stack_repos, stack);
	esim_schedule_event(retevent, retstack, 0);
}


/* Initialization */

void net_init(void)
{
	net_stack_repos = repos_create(sizeof(struct net_stack_t), "net_stack");
	EV_NET_SEND = esim_register_event(net_handler);
	EV_NET_DELIVER = esim_register_event(net_handler);
	EV_NET_BUS_SEND = esim_register_event(net_handler);
	EV_NET_BUS_DELIVER = esim_register_event(net_handler);
}


void net_done(void)
{
	repos_free(net_stack_repos);
}




/* Network */

struct net_t *net_create(void)
{
	struct net_t *net;
	net = calloc(1, sizeof(struct net_t));
	net->link_width = 8;
	return net;
}


struct net_t *net_create_bus(void)
{
	struct net_t *net;
	net = net_create();
	net->bus = 1;
	return net;
}


void net_free(struct net_t *net)
{
	int i;
	if (net->nodes) {
		for (i = 0; i < net->max_nodes; i++)
			free(net->nodes[i].con);
		free(net->nodes);
	}
	free(net);
}




/* Nodes */

void net_init_nodes(struct net_t *net, int max_nodes)
{
	int i, j;
	net->main_node = -1;
	net->max_nodes = max_nodes;
	net->nodes = calloc(max_nodes, sizeof(struct net_node_t));
	for (i = 0; i < max_nodes; i++) {
		net->nodes[i].con = calloc(max_nodes, sizeof(struct net_node_con_t));
		for (j = 0; j < max_nodes; j++) {
			net->nodes[i].con[j].cost = i == j ? 0 : max_nodes;  /* infinite cost or 0 */
			net->nodes[i].con[j].route = -1;  /* no route */
		}
	}
}


int net_add_node(struct net_t *net, enum net_node_kind_enum kind, void *data)
{
	struct net_node_t *node;

	/* Nodes not initialized */
	if (!net->nodes)
		fatal("libnetwork: nodes not initialized, call net_init_nodes first");

	/* Enough space for new node? */
	if (net->num_nodes == net->max_nodes)
		fatal("libnetwork: too many nodes");

	/* Cannot add switches to a bus */
	if (net->bus && kind == net_node_kind_switch)
		fatal("libnetwork: cannot add switch to a bus");
	
	/* New node of type 'cache' or main node */
	if (kind == net_node_kind_cache)
		net->num_caches++;
	if (kind == net_node_kind_main) {
		if (net->main_node >= 0)
			fatal("libnetwork: no more than one main node allowed");
		net->main_node = net->num_nodes;
	}

	/* Add node */
	node = &net->nodes[net->num_nodes];
	node->kind = kind;
	node->data = data;
	return net->num_nodes++;
}


void *net_get_node(struct net_t *net, int node)
{
	if (node < 0 || node >= net->num_nodes)
		fatal("libnetwork: node out of range");
	return net->nodes[node].data;
}


void net_add_con(struct net_t *net, int src, int dst)
{
	if (src >= net->num_nodes || dst >= net->num_nodes)
		fatal("libnetwork: net_add_con: node indices out of range");

	/* Add bidirectional connection */
	net->nodes[src].con[dst].cost = 1;
	net->nodes[src].con[dst].route = dst;
	net->nodes[dst].con[src].cost = 1;
	net->nodes[dst].con[src].route = src;
}


void net_calculate_routes(struct net_t *net)
{
	int i, j, k;
	for (k = 0; k < net->num_nodes; k++) {
		for (i = 0; i < net->num_nodes; i++) {
			for (j = 0; j < net->num_nodes; j++) {
				if (net->nodes[i].con[k].cost + net->nodes[k].con[j].cost <
					net->nodes[i].con[j].cost)
				{
					net->nodes[i].con[j].cost = net->nodes[i].con[k].cost +
						net->nodes[k].con[j].cost;
					net->nodes[j].con[i].route = net->nodes[j].con[k].route;
				}
			}
		}
	}
}


void net_dump_routes(struct net_t *net, FILE *f)
{
	int i, j;
	fprintf(f, "         ");
	for (i = 0; i < net->num_nodes; i++)
		fprintf(f, "%2d ", i);
	fprintf(f, "\n");
	for (i = 0; i < net->num_nodes; i++) {
		fprintf(f, "node %2d: ", i);
		for (j = 0; j < net->num_nodes; j++) {
			if (net->nodes[i].con[j].route >= 0)
				fprintf(f, "%2d ", net->nodes[i].con[j].route);
			else
				fprintf(f, "-- ");
		}
		fprintf(f, "\n");
	}
}


void net_send_ev(struct net_t *net, int src, int dst, int size,
	int retevent, void *retstack)
{
	struct net_stack_t *stack;

	/* Range of nodes */
	if (src < 0 || src >= net->num_nodes || dst < 0 || dst >= net->num_nodes)
		fatal("libnetwork: nodes out of range");
		
	/* Check that src and dst are valid nodes */
	if (net->nodes[src].kind == net_node_kind_switch ||
		net->nodes[dst].kind == net_node_kind_switch)
		fatal("libnetwork: net_send_ev: src or dst cannot be switches");
	
	/* If we are sending to current node */
	if (dst == src)
		fatal("libnetwork: sending packet to source node");

	/* Increase current packet id only if we are not
	 * broadcasting */
	if (!net->bus_broadcasting)
		net->current_packet_id++;

	/* Send */
	stack = net_stack_create(net, retevent, retstack);
	stack->src = src;
	stack->dst = dst;
	stack->size = size;
	stack->packet_id = net->current_packet_id;
	stack->send_time = esim_cycle;
	esim_schedule_event(net->bus ? EV_NET_BUS_SEND : EV_NET_SEND, stack, 0);
}


void net_start_broadcast(struct net_t *net)
{
	if (!net->bus)
		fatal("libnetwork: broadcast in network other than bus");
	if (net->bus_broadcasting)
		fatal("libnetwork: already issuing broadcast");
	net->bus_broadcasting = 1;
	net->current_packet_id++;
}


void net_commit_broadcast(struct net_t *net)
{
	if (!net->bus_broadcasting)
		fatal("libnetwork: not issuing broadcast");
	net->bus_broadcasting = 0;
}

