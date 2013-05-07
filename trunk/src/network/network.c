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
#include "link.h"
#include "net-system.h"
#include "visual.h"
#include "network.h"
#include "node.h"
#include "routing-table.h"


/*
 * Private Functions
 */

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
		panic("%s: message %lld not in hash table",
				__FUNCTION__, id);
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


struct net_t *net_create_from_config(struct config_t *config, char *name)
{
	struct net_t *net;
	char *section;
	char section_str[MAX_STRING_SIZE];

	int def_input_buffer_size;
	int def_output_buffer_size;
	int def_bandwidth;
	int routing_type = 0;

	/* Create network */
	net = net_create(name);

	/* Main section */
	snprintf(section_str, sizeof section_str, "Network.%s", name);
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		if (strcasecmp(section, section_str))
			continue;

		def_input_buffer_size = config_read_int(config, section, "DefaultInputBufferSize", 0);
		def_output_buffer_size = config_read_int(config, section, "DefaultOutputBufferSize", 0);
		def_bandwidth = config_read_int(config, section, "DefaultBandwidth", 0);
		if (!def_input_buffer_size)
			fatal("%s:%s: DefaultInputBufferSize: invalid/missing value.\n%s",
					net->name, section, net_err_config);
		if (!def_output_buffer_size)
			fatal("%s:%s: DefaultOutputBufferSize: invalid/missing value.\n%s",
					net->name, section, net_err_config);
		if (!def_bandwidth)
			fatal("%s:%s: DefaultBandwidth: invalid/missing value.\n%s",
					net->name, section, net_err_config);
	}

	/* Nodes */
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *delim = ".";
		char *token;

		char *node_name;
		char *node_type;
		int input_buffer_size;
		int output_buffer_size;
		int bandwidth;

		/* First token must be 'Network' */
		snprintf(section_str, sizeof section_str, "%s", section);
		token = strtok(section_str, delim);
		if (!token || strcasecmp(token, "Network"))
			continue;

		/* Second token must be the name of the network */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, name))
			continue;

		/* Third token must be 'Node' */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, "Node"))
			continue;

		/* Get name */
		node_name = strtok(NULL, delim);
		token = strtok(NULL, delim);
		if (!node_name || token)
			fatal("%s:%s: wrong format for node.\n%s",
					net->name, section, net_err_config);

		/* Get properties */
		node_type = config_read_string(config, section, "Type", "");
		input_buffer_size = config_read_int(config, section,
				"InputBufferSize", def_input_buffer_size);
		output_buffer_size = config_read_int(config, section,
				"OutputBufferSize", def_output_buffer_size);
		bandwidth = config_read_int(config, section,
				"BandWidth", def_bandwidth);

		/* Create node */
		if (!strcasecmp(node_type, "EndNode"))
			net_add_end_node(net, input_buffer_size, output_buffer_size,
					node_name, NULL);
		else if (!strcasecmp(node_type, "Switch"))
			net_add_switch(net, input_buffer_size, output_buffer_size,
					bandwidth, node_name);
		else
			fatal("%s:%s: Type: invalid/missing value.\n%s",
					net->name, section, net_err_config);
	}



	/* Links */
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *delim = ".";
		char *token;

		char *link_name;
		char *link_type;
		int bandwidth;
		int v_channel_count ;

		char *src_node_name;
		char *dst_node_name;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		/* First token must be 'Network' */
		snprintf(section_str, sizeof section_str, "%s", section);
		token = strtok(section_str, delim);
		if (!token || strcasecmp(token, "Network"))
			continue;

		/* Second token must be the name of the network */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, name))
			continue;

		/* Third token must be 'Link' */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, "Link"))
			continue;

		/* Fourth token must name of the link */
		link_name = strtok(NULL, delim);
		token = strtok(NULL, delim);
		if (!link_name || token)
			fatal("%s: %s: bad format for link.\n%s",
					name, section, net_err_config);

		/* Fields */
		link_type = config_read_string(config, section, "Type", "Unidirectional");
		bandwidth = config_read_int(config, section, "Bandwidth", def_bandwidth);
		src_node_name = config_read_string(config, section, "Source", "");
		dst_node_name = config_read_string(config, section, "Dest", "");
		v_channel_count = config_read_int(config, section, "VC", 1);

		/* Nodes */
		src_node = net_get_node_by_name(net, src_node_name);
		dst_node = net_get_node_by_name(net, dst_node_name);

		if (!src_node)
			fatal("%s: %s: %s: source node does not exist.\n%s",
					name, section, src_node_name, net_err_config);
		if (!dst_node)
			fatal("%s: %s: %s: destination node does not exist.\n%s",
					name, section, dst_node_name, net_err_config);

		if (v_channel_count >= 1)
		{

			if (!strcasecmp(link_type, "Unidirectional"))
			{
				net_add_link(net, src_node, dst_node, bandwidth, v_channel_count);
			}
			else if (!strcasecmp(link_type, "Bidirectional"))
			{
				net_add_bidirectional_link(net, src_node, dst_node, bandwidth,v_channel_count);
			}
		}
		else fatal("%s: %s: Unacceptable number of virtual channels \n %s",
				name, section,net_err_config);
	}

	/* initializing the routing table */
	net_routing_table_initiate(net->routing_table);

	/*Routes*/
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *delim = ".";
		char *delim_sep = ":";
		char *token;
		char *token_endl;

		/* First token must be 'Network' */
		snprintf(section_str, sizeof section_str, "%s", section);
		token = strtok(section_str, delim);
		if (!token || strcasecmp(token, "Network"))
			continue;

		/* Second token must be the name of the network */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, name))
			continue;

		/* Third token must be 'Routes' */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, "Routes"))
			continue;

		token_endl = strtok(NULL, delim);
		if (token_endl)
			fatal("%s: %s: bad format for route.\n%s",
					name, section, net_err_config);

		/*Routes*/
		routing_type = 1;

		for (int i = 0; i < net->node_count; i++)
		{

			for (int j = 0; j < net->node_count; j++)
			{
				int vc_used = 0;
				char spr_result_size [MAX_STRING_SIZE] ;
				char *nxt_node_name;
				struct net_node_t *src_node_r;
				struct net_node_t *dst_node_r;
				struct net_node_t *nxt_node_r;

				src_node_r = list_get(net->node_list, i);
				dst_node_r = list_get(net->node_list, j);

				if (dst_node_r->kind == net_node_end)
				{
					snprintf(spr_result_size, sizeof spr_result_size, "%s.to.%s",
							src_node_r->name,dst_node_r->name);
					nxt_node_name = config_read_string(config, section, spr_result_size , "---" );
					/* Token Separates the next node and VC */
					snprintf(section_str, sizeof section_str, "%s", nxt_node_name);
					token = strtok(section_str, delim_sep);
					nxt_node_name = token;
					token = strtok(NULL, delim_sep);


					if (token)
					{
						vc_used = atoi(token);
						if (vc_used < 0)
							fatal("Network %s:%s: Unacceptable virtual channel format is chosen\n %s",
									net->name, section, net_err_config);
					}

					int name_check = strcmp(nxt_node_name, "---");
					nxt_node_r = net_get_node_by_name(net, nxt_node_name);

					if (!nxt_node_r && name_check != 0)
					{
						fatal("Network %s:%s: Invalid node Name.\n %s",
								net->name, section,net_err_config);
					}
					if (nxt_node_r)
					{
						if (src_node_r == dst_node_r)
							fatal("Network %s:%s: Invalid Routing format.\n %s",
									net->name, section,net_err_config);
						else
							if (vc_used > 0)
								net_routing_table_route_update(net->routing_table,
										src_node_r, dst_node_r, nxt_node_r, vc_used);
							else
							{
								vc_used = 0;
								net_routing_table_route_update(net->routing_table,
										src_node_r, dst_node_r, nxt_node_r, vc_used);
							}
					}
				}
			}
		}
		config_check(config);
	}

	/* If there is no route section, Floyd-Warshall calculates
	 * the shortest path for all the nodes in the network */
	if (routing_type == 0) 
		net_routing_table_floyd_warshall(net->routing_table);

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
			(double) net->msg_size_acc / net->transfers : 0.0);
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

void net_dump_visual(struct net_graph_t *graph, FILE *f)
{
	int i;
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	fprintf(f, "Legend = True \n");
	fprintf(f, "Title = \" Network: %s \"\n", graph->net->name );
	for (i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct net_graph_vertex_t *vertex;
		vertex= list_get(graph->vertex_list, i);

		fprintf(f, "node = %s %d %f %d \n", vertex->name, vertex->kind,
				(double) vertex->x_coor/graph->xscale, vertex->y_coor);

	}
	for (i = 0; i < list_count(graph->edge_list); i++)
	{
		struct net_graph_edge_t *edge;
		edge= list_get(graph->edge_list, i);
		if (!edge->upstream)
			fprintf(f, "link = %f %d %f %d %d %d \n", (double) edge->src_vertex->x_coor / graph->xscale,
					edge->src_vertex->y_coor,(double) edge->dst_vertex->x_coor / graph->xscale,
					edge->dst_vertex->y_coor,(int) ((cycle ?(double) edge->downstream->transferred_bytes
							/ (cycle * edge->downstream->bandwidth) : 0.0) * 10), 1);
		else
		{
			fprintf(f, "link = %f %d %f %d %d %d \n", (double) edge->src_vertex->x_coor / graph->xscale
					, edge->src_vertex->y_coor,	(double) edge->dst_vertex->x_coor / graph->xscale,
					edge->dst_vertex->y_coor,(int) ((cycle ?(double) edge->downstream->transferred_bytes
							/ (cycle * edge->downstream->bandwidth) : 0.0) * 10), 2);
			fprintf(f, "link = %f %d %f %d %d %d \n",(double) edge->dst_vertex->x_coor / graph->xscale
					, edge->dst_vertex->y_coor,	(double) edge->src_vertex->x_coor / graph->xscale
					, edge->src_vertex->y_coor,	(int) ((cycle ?(double) edge->upstream->transferred_bytes
							/ (cycle * edge->upstream->bandwidth) : 0.0) * 10), 2);
		}

	}

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


struct net_node_t *net_add_bus(struct net_t *net, int bandwidth, char *name)
{
	struct net_node_t *node;

	/* Not supported */
	panic("%s: buses not supported", __FUNCTION__);

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


/* Get a node by its name. If none found, return NULL.
 * Search is case-insensitive. */
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
struct net_node_t *net_get_node_by_user_data(struct net_t *net, void *user_data)
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
		int bandwidth, int vc_count)
{
	struct net_link_t *link;

	/* Checks */
	assert(src_node->net == net);
	assert(dst_node->net == net);
	if (src_node->kind == net_node_end && dst_node->kind == net_node_end)
		fatal("network \"%s\": link cannot connect two end nodes\n", net->name);

	/* Create link connecting buffers */
	link = net_link_create(net, src_node, dst_node, bandwidth, vc_count);

	/* Add to link list */
	list_add(net->link_list, link);

	/* Return */
	return link;
}


/* Create bidirectional link with VC */
void net_add_bidirectional_link(struct net_t *net,
		struct net_node_t *src_node, struct net_node_t *dst_node,
		int bandwidth, int vc_count)
{
	net_add_link(net, src_node, dst_node, bandwidth, vc_count);
	net_add_link(net, dst_node, src_node, bandwidth, vc_count);

}

/* Return TRUE if a message can be sent through the network. Return FALSE
 * otherwise, whether the reason is temporary of permanent. */
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
	if (output_buffer->count + size > output_buffer->size)
		return 0;

	/* All conditions satisfied, can send */
	return 1;
}


/* Return TRUE if a message can be sent to the network. If it cannot be
 * sent, return FALSE, and schedule 'retry_event' for the cycle when the check
 * should be performed again. This function should not be called if
 * the reason why a message cannot be sent is permanent (e.g., no route
 * to destination). */
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
				net->name, src_node->name, dst_node->name, net_err_no_route);

	/* Message is too long */
	if (size > output_buffer->size)
		fatal("%s: message too long.\n%s", net->name, net_err_large_message);

	/* Output buffer is busy */
	if (output_buffer->write_busy >= cycle)
	{
		esim_schedule_event(retry_event, retry_stack,
				output_buffer->write_busy - cycle + 1);
		return 0;
	}

	/* Message does not fit in output buffer */
	if (output_buffer->count + size > output_buffer->size)
	{
		net_buffer_wait(output_buffer, retry_event, retry_stack);
		return 0;
	}

	/* All conditions satisfied, can send */
	return 1;
}


/* Send a message through the network.
 * When using this function, the caller must make sure that the message can be
 * injected in the network, calling 'net_can_send' first.
 * When the message is received in 'dst_node' it will be removed automatically
 * from its input buffer, and the 'msg' object return by this function will be
 * invalid. */
struct net_msg_t *net_send(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size)
{
	return net_send_ev(net, src_node, dst_node, size,
			ESIM_EV_NONE, NULL);
}


/* Send a message through network.
 * The caller must make sure that the message can be injected in the network, with
 * a previous call to 'net_can_send'.
 * When the message reaches the head of the input buffer in 'dst_node', event
 * 'receive_event' will be scheduled, and object 'msg' will be accessible.
 * The message needs to be removed by the caller with an additional call to
 * 'net_receive', which will invalidate and free the 'msg' object. */
struct net_msg_t *net_send_ev(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size, int receive_event, void *receive_stack)
{
	struct net_stack_t *stack;
	struct net_msg_t *msg;

	/* Check nodes */
	if (src_node->kind != net_node_end || dst_node->kind != net_node_end)
		fatal("%s: not end nodes.\n%s", __FUNCTION__, net_err_end_nodes);

	/* Create message */
	msg = net_msg_create(net, src_node, dst_node, size);

	/* Insert message into hash table of in-flight messages */
	net_msg_table_insert(net, msg);

	/* Start event-driven simulation */
	stack = net_stack_create(net, ESIM_EV_NONE, NULL);
	stack->msg = msg;
	stack->ret_event = receive_event;
	stack->ret_stack = receive_stack;
	esim_execute_event(EV_NET_SEND, stack);

	/* Return created message */
	return msg;
}


struct net_msg_t *net_try_send(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size,
		int retry_event, void *retry_stack)
{
	/* Check if network is available */
	if (!net_can_send_ev(net, src_node, dst_node, size, retry_event, retry_stack))
		return NULL;

	/* Send message */
	return net_send(net, src_node, dst_node, size);
}


struct net_msg_t *net_try_send_ev(struct net_t *net, struct net_node_t *src_node,
		struct net_node_t *dst_node, int size, int receive_event, void *receive_stack,
		int retry_event, void *retry_stack)
{
	/* Check if network is available */
	if (!net_can_send_ev(net, src_node, dst_node, size, retry_event, retry_stack))
		return NULL;

	/* Send message */
	return net_send_ev(net, src_node, dst_node, size, receive_event, receive_stack);
}


/* Absorb a message at the head of the input buffer of an end node */
void net_receive(struct net_t *net, struct net_node_t *node, struct net_msg_t *msg)
{
	struct net_buffer_t *buffer;

	/* Checks */
	assert(node->net == net);
	assert(msg->net == net);
	if (msg->node != node)
		panic("%s: message not at end node", __FUNCTION__);

	/* Get buffer */
	buffer = msg->buffer;
	assert(buffer->node == node);
	if (!list_count(buffer->msg_list))
		panic("%s: empty buffer", __FUNCTION__);
	if (list_get(buffer->msg_list, 0) != msg)
		panic("%s: message not at input buffer head", __FUNCTION__);

	/* Extract and free message */
	net_buffer_extract(buffer, msg);
	net_msg_table_extract(net, msg->id);
	net_msg_free(msg);
}

