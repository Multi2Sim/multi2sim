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

#include <network.h>


/*
 * Variables
 */

int net_debug_category;

char *err_net_end_nodes =
	"\tAn attempt has been made to send a message from/to an intermediate\n"
	"\tswitch or bus. Both the initial source and the final destination\n"
	"\tof a network message have to be end nodes.\n";

char *err_net_no_route =
	"\tA message has been sent between two nodes with no possible route.\n"
	"\tPlease redesign your network, considering that every pair of end\n"
	"\tnodes sending messages to each other need to be connected with a\n"
	"\tlink.\n";

char *err_net_large_message =
	"\tA message has been sent in a network where some input/output buffer\n"
	"\tdoes not have enough capacity to hold it. Please redesign your\n"
	"\tnetwork, considering that the size of each buffer should be at\n"
	"\tleast as large as the largest possible message.\n";

char *err_net_node_name_duplicate =
	"\tA node has been added to a network with a name duplicate. Please make\n"
	"\tsure that all intermediate and end nodes of your network have\n"
	"\tdifferent names.\n";

char *err_net_config =
	"\tA network is being loaded from an IniFile configuration file.\n"
	"\tHowever, some feature of the provided file does not comply with the\n"
	"\texpected format. Please run 'm2s --help-network' for a list of\n"
	"\tpossible sections/variables in the network configuration file.\n";
	/* FIXME: implement '--help-network' option */

char *err_net_can_send =
	"\tAn attempt has been detected of injecting a package in a network\n"
	"\tfrom a source node that has no available space in its output\n"
	"\tbuffer. This can be solved by making sure a message can be sent\n"
	"\tbefore injecting it (use function 'net_can_send').\n";



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
struct net_msg_t *net_msg_table_get(struct net_t *net, uint64_t id)
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
struct net_msg_t *net_msg_table_extract(struct net_t *net, uint64_t id)
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
			__FUNCTION__, (long long) id);
	if (prev)
		prev->bucket_next = msg->bucket_next;
	else
		net->msg_table[index] = msg->bucket_next;
	return msg;
}




/*
 * Public Functions
 */


int EV_NET_SEND;
int EV_NET_OUTPUT_BUFFER;
int EV_NET_INPUT_BUFFER;
int EV_NET_RECEIVE;

void net_init(void)
{
	EV_NET_SEND = esim_register_event(net_event_handler);
	EV_NET_OUTPUT_BUFFER = esim_register_event(net_event_handler);
	EV_NET_INPUT_BUFFER = esim_register_event(net_event_handler);
	EV_NET_RECEIVE = esim_register_event(net_event_handler);
}


void net_done(void)
{
}


struct net_t *net_create(char *name)
{
	struct net_t *net;

	/* Create */
	net = calloc(1, sizeof(struct net_t));
	if (!net)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	net->name = strdup(name);
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
				net->name, section, err_net_config);
		if (!def_output_buffer_size)
			fatal("%s:%s: DefaultOutputBufferSize: invalid/missing value.\n%s",
				net->name, section, err_net_config);
		if (!def_bandwidth)
			fatal("%s:%s: DefaultBandwidth: invalid/missing value.\n%s",
				net->name, section, err_net_config);
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
				net->name, section, err_net_config);

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
				net->name, section, err_net_config);
	}

	/* Links */
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *delim = ".";
		char *token;

		char *link_name;
		char *link_type;
		int bandwidth;

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
				name, section, err_net_config);

		/* Fields */
		link_type = config_read_string(config, section, "Type", "Unidirectional");
		bandwidth = config_read_int(config, section, "Bandwidth", def_bandwidth);
		src_node_name = config_read_string(config, section, "Source", "");
		dst_node_name = config_read_string(config, section, "Dest", "");

		/* Nodes */
		src_node = net_get_node_by_name(net, src_node_name);
		dst_node = net_get_node_by_name(net, dst_node_name);
		if (!src_node)
			fatal("%s: %s: %s: source node does not exist.\n%s",
				name, section, src_node_name, err_net_config);
		if (!dst_node)
			fatal("%s: %s: %s: destination node does not exist.\n%s",
				name, section, dst_node_name, err_net_config);
		if (!strcasecmp(link_type, "Unidirectional"))
			net_add_link(net, src_node, dst_node, bandwidth);
		else if (!strcasecmp(link_type, "Bidirectional"))
			net_add_bidirectional_link(net, src_node, dst_node, bandwidth);
	}


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


void net_dump_report(struct net_t *net, FILE *f)
{
	int i;

	/* General stats */
	fprintf(f, "[ Network.%s.General ]\n", net->name);
	fprintf(f, "Transfers = %lld\n", net->transfers);
	fprintf(f, "AverageMessageSize = %.2f\n", net->transfers ?
		(double) net->msg_size_acc / net->transfers : 0.0);
	fprintf(f, "AverageLatency = %.4f\n", esim_cycle ?
		(double) net->lat_acc / esim_cycle : 0.0);
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


/* Create a new unidirectional link */
struct net_link_t *net_add_link(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth)
{
	struct net_link_t *link;
	struct net_buffer_t *src_buffer;
	struct net_buffer_t *dst_buffer;

	/* Checks */
	assert(src_node->net == net);
	assert(dst_node->net == net);
	if (src_node->kind == net_node_end && dst_node->kind == net_node_end)
		fatal("network \"%s\": link cannot connect two end nodes\n", net->name);

	/* Create output buffer in source node and input buffer in destination node */
	src_buffer = net_node_add_output_buffer(src_node);
	dst_buffer = net_node_add_input_buffer(dst_node);

	/* Create link connecting buffers */
	link = net_link_create(net, src_node, src_buffer,
		dst_node, dst_buffer, bandwidth);

	/* Add to link list */
	list_add(net->link_list, link);

	/* Return */
	return link;
}


/* Create bidirectional link */
void net_add_bidirectional_link(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth)
{
	net_add_link(net, src_node, dst_node, bandwidth);
	net_add_link(net, dst_node, src_node, bandwidth);
}


int net_can_send(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size)
{
	struct net_routing_table_t *routing_table = net->routing_table;
	struct net_routing_table_entry_t *entry;
	struct net_buffer_t *output_buffer;

	/* Get output buffer */
	entry = net_routing_table_lookup(routing_table, src_node, dst_node);
	output_buffer = entry->output_buffer;

	/* No route to destination */
	if (!output_buffer)
		return 0;
	
	/* Output buffer is busy */
	if (output_buffer->write_busy >= esim_cycle)
		return 0;
	
	/* Message does not fit in output buffer */
	if (output_buffer->count + size > output_buffer->size)
		return 0;

	/* All conditions satisfied, can send */
	return 1;
}


/* Return TRUE if a message can be sent to the network. If it cannot be
 * sent, return FALSE, and schedule 'event' for the cycle when the check
 * should be performed again. This function should not be called if
 * the reason why a message cannot be sent is permanent (e.g., no route
 * to destination). */
int net_can_send_ev(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size, int event, void *stack)
{
	struct net_routing_table_t *routing_table = net->routing_table;
	struct net_routing_table_entry_t *entry;
	struct net_buffer_t *output_buffer;

	/* Get output buffer */
	entry = net_routing_table_lookup(routing_table, src_node, dst_node);
	output_buffer = entry->output_buffer;

	/* No route to destination */
	if (!output_buffer)
		fatal("%s: no route to destination.\n%s", net->name, err_net_no_route);
	
	/* Message is too long */
	if (size > output_buffer->size)
		fatal("%s: message too long.\n%s", net->name, err_net_large_message);
	
	/* Output buffer is busy */
	if (output_buffer->write_busy >= esim_cycle)
	{
		esim_schedule_event(event, stack, output_buffer->write_busy - esim_cycle + 1);
		return 0;
	}
	
	/* Message does not fit in output buffer */
	if (output_buffer->count + size > output_buffer->size)
	{
		net_buffer_wait(output_buffer, event, stack);
		return 0;
	}

	/* All conditions satisfied, can send */
	return 1;
}


struct net_msg_t *net_send(struct net_t *net, struct net_node_t *src_node,
	struct net_node_t *dst_node, int size)
{
	return net_send_ev(net, src_node, dst_node, size,
		ESIM_EV_NONE, NULL);
}


struct net_msg_t *net_send_ev(struct net_t *net, struct net_node_t *src_node, struct net_node_t *dst_node,
	int size, int retevent, void *retstack)
{
	struct net_stack_t *stack;
	struct net_msg_t *msg;

	/* Check nodes */
	if (src_node->kind != net_node_end || dst_node->kind != net_node_end)
		fatal("%s: not end nodes.\n%s", __FUNCTION__, err_net_end_nodes);

	/* Create message */
	msg = net_msg_create(net, src_node, dst_node, size);

	/* Insert message into hash table of in-flight messages */
	net_msg_table_insert(net, msg);

	/* Start event-driven simulation */
	stack = net_stack_create(net, ESIM_EV_NONE, NULL);
	stack->msg = msg;
	stack->retevent = retevent;
	stack->retstack = retstack;
	esim_execute_event(EV_NET_SEND, stack);

	/* Return created message */
	return msg;
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

