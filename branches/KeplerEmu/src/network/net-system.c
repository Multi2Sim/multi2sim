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

#include <math.h>

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "net-system.h"
#include "network.h"
#include "node.h"
#include "visual.h"

/* 
 * Variables
 */

int net_debug_category;

char *net_config_help =
		"The network configuration file is a plain-text file following the\n"
		"IniFile format. It specifies a set of networks, their nodes, and\n"
		"connections between them. The following set of sections and variables\n"
		"are allowed:\n"
		"\n"
		"Section '[ General ]' contains configuration parameters affecting the\n"
		"whole network system.\n"
		"\n"
		"  Frequency = <value> (Default = 1000)\n"
		"      Frequency for the network system in MHz.\n"
		"\n"
		"Section '[ Network.<name> ]' defines a network. The string specified in\n"
		"<name> can be used in other configuration files to refer to\n"
		"this network.\n"
		"\n"
		"  DefaultInputBufferSize = <size> (Required)\n"
		"      Default size for input buffers in nodes and switches, specified\n"
		"      in number of packets. When a node/switch is created in the network\n"
		"      this size will be used if it is not specified.\n"
		"  DefaultOutputBufferSize = <size> (Required)\n"
		"      Default size for output buffers in nodes and switches in number\n"
		"      or packets.\n"
		"  DefaultBandwidth = <bandwidth> (Required)\n"
		"      Default bandwidth for links in the network, specified in number of\n"
		"      bytes per cycle. If a link's bandwidth is not specified, this value\n"
		"      will be used.\n"
		"\n"
		"Sections '[ Network.<network>.Node.<node> ]' are used to define nodes in\n"
		"network '<network>'.\n"
		"\n"
		"  Type = {EndNode|Switch} (Required)\n"
		"      Type of node. End nodes can send and receive packets, while\n"
		"      switches are used to forward packets between other switches and\n"
		"      end nodes.\n"
		"  InputBufferSize = <size> (Default = <network>.DefaultInputBufferSize)\n"
		"      Size of input buffer in number of packets.\n"
		"  OutputBufferSize = <size> (Default = <network>.DefaultOutputBufferSize)\n"
		"      Size of output buffer in number of packets.\n"
		"  Bandwidth = <bandwidth> (Default = <network>.DefaultBandwidth)\n"
		"      For switches, bandwidth of internal crossbar communicating input\n"
		"      with output buffers. For end nodes, this variable is ignored.\n"
		"\n"
		"Sections '[ Network.<network>.Link.<link> ]' are used to define links in\n"
		"network <network>. A link connects an output buffer of a source node with\n"
		"an input buffer of a destination node.\n"
		"\n"
		"  Source = <node> (Required)\n"
		"      Source node. The node name is a node declared in a\n"
		"      'Network.<network>.Node.<node>' section.\n"
		"  Dest = <node> (Required)\n"
		"      Destination node.\n"
		"  Type = {Unidirectional|Bidirectional} (Default = Unidirectional)\n"
		"      Link direction. Choosing a bidirectional link has the same effect\n"
		"      as creating two unidirectional links in opposite directions.\n"
		"  Bandwidth = <bandwidth> (Default = <network>.DefaultBandwidth)\n"
		"      Bandwidth of the link in bytes per cycle.\n"
		"  VC = <virtual channels> (Default = 1)\n"
		"	Number of virtual channels a link can have.\n"
		"\n"
		"Section '[Network.<network>.Routes]' can be used (Optional) to define \n"
		"routes and manually configure the routing table. For a route between \n"
		"two end-nodes every route step from source to destination should be \n"
		"identified. Each unidirectional route step follows the pattern:\n"
		"  <node_A>.to.<node_C> = <node_B>:<Virtual Channel>\n"
		"  node_A. Source node of a route step \n"
		"  node_C. Destination node of a route step \n"
		"  node_B. Immediate next node that each packet must go through to get \n"
		"      from node_A to node_C\n"
		"  Virtual Channel. Is an optional field to choose a virtual channel on \n"
		"  the link between node_A and node_B. \n" "\n" "\n";

char *net_err_end_nodes =
		"\tAn attempt has been made to send a message from/to an intermediate\n"
		"\tswitch or bus. Both the initial source and the final destination\n"
		"\tof a network message have to be end nodes.\n";

char *net_err_no_route =
		"\tA message has been sent between two nodes with no possible route.\n"
		"\tPlease redesign your network, considering that every pair of end\n"
		"\tnodes sending messages to each other need to be connected with a\n"
		"\tlink.\n";

char *net_err_large_message =
		"\tA message has been sent in a network where some input/output buffer\n"
		"\tdoes not have enough capacity to hold it. Please redesign your\n"
		"\tnetwork, considering that the size of each buffer should be at\n"
		"\tleast as large as the largest possible message.\n";

char *net_err_node_name_duplicate =
		"\tA node has been added to a network with a name duplicate. Please make\n"
		"\tsure that all intermediate and end nodes of your network have\n"
		"\tdifferent names.\n";

char *net_err_config =
		"\tA network is being loaded from an IniFile configuration file.\n"
		"\tHowever, some feature of the provided file does not comply with the\n"
		"\texpected format. Please run 'm2s --help-net-config' for a list of\n"
		"\tpossible sections/variables in the network configuration file.\n";

char *net_err_can_send =
		"\tAn attempt has been detected of injecting a package in a network\n"
		"\tfrom a source node that has no available space in its output\n"
		"\tbuffer. This can be solved by making sure a message can be sent\n"
		"\tbefore injecting it (use function 'net_can_send').\n";

char *net_err_cycle =
		"\tA cycle has been detected in the graph representing the routing table\n"
		"\tfor a network. Routing cycles can cause deadlocks in simulations, that\n"
		"\tcan in turn make the simulation stall with no output.\n";

char *net_err_route_step =
		"\tThere is a link missing between source node and next node for this  \n"
		"\troute step. The route between source and destination node should go \n"
		"\tthrough existing links/Buses that are defined in the configuration  \n"
		"\tfile.  \n";



/* Events */
int EV_NET_SEND;
int EV_NET_OUTPUT_BUFFER;
int EV_NET_INPUT_BUFFER;
int EV_NET_RECEIVE;

/* List of networks */
static struct hash_table_t *net_table;

/* Configuration parameters */
char *net_config_file_name = "";

char *net_traffic_pattern = "";

char *net_report_file_name = "";
FILE *net_report_file;

char *net_visual_file_name = "";
FILE *net_visual_file;

char *net_sim_network_name = "";
long long net_max_cycles = 1000000;	/* 1M cycles default */
double net_injection_rate = 0.001;	/* 1 packet every 1000 cycles */
int net_msg_size = 1;			/* Message size in bytes */

/* Frequency of the network system, and frequency domain, as returned by
 * function 'esim_new_domain'. */
int net_frequency = 1000;
int net_domain_index;



/* 
 * Private Functions
 */

static double exp_random(double lambda)
{
	double x = (double) random() / RAND_MAX;

	return log(1 - x) / -lambda;
}


static void net_traffic_uniform(struct net_t *net, double *inject_time)
{
	while (1)
	{
		struct net_node_t *node;
		struct net_node_t *dst_node;
		long long cycle;
		int i;

		/* Get current cycle */
		cycle = esim_domain_cycle(net_domain_index);
		if (cycle >= net_max_cycles)
			break;

		/* Inject messages */
		for (i = 0; i < net->node_count; i++)
		{
			/* Get end node */
			node = list_get(net->node_list, i);
			if (node->kind != net_node_end)
				continue;

			/* Turn for next injection? */
			if (inject_time[i] > cycle)
				continue;

			/* Get a random destination node */
			do
			{
				dst_node = list_get(net->node_list, random() %
						list_count(net->node_list));
			} while (dst_node->kind != net_node_end
					|| dst_node == node);

			/* Inject */
			while (inject_time[i] < cycle)
			{
				inject_time[i] += exp_random(net_injection_rate);
				if (net_can_send(net, node, dst_node, net_msg_size))
					net_send(net, node, dst_node, net_msg_size);
			}
		}
		/* Next cycle */
		net_debug("___ cycle %lld ___\n", cycle);
		esim_process_events(TRUE);
	}

}


/*
 * Public Functions
 */


void net_read_config(void)
{
	struct config_t *config;
	struct list_t *net_name_list;
	char *section;
	int i;

	/* Configuration file */
	if (!*net_config_file_name)
	{
		net_domain_index = esim_new_domain(net_frequency);
		return;
	}


	/* Open network configuration file */
	config = config_create(net_config_file_name);
	if (*net_config_file_name)
		config_load(config);

	/* Section with generic configuration parameters */
	section = "General";

	/* Frequency */
	net_frequency = config_read_int(config, section,
			"Frequency", net_frequency);
	if (!IN_RANGE(net_frequency, 1, ESIM_MAX_FREQUENCY))
		fatal("%s: invalid value for 'Frequency'",
				net_config_file_name);

	/* Create frequency domain */
	net_domain_index = esim_new_domain(net_frequency);

	/* Create a temporary list of network names found in configuration
	 * file */
	net_name_list = list_create();
	for (section = config_section_first(config); section;
			section = config_section_next(config))
	{
		char *delim = ".";

		char section_str[MAX_STRING_SIZE];
		char *token;
		char *net_name;

		/* Create a copy of section name */
		snprintf(section_str, sizeof section_str, "%s", section);
		section = section_str;

		/* First token must be 'Network' */
		token = strtok(section, delim);
		if (strcasecmp(token, "Network"))
			continue;

		/* Second token is network name */
		net_name = strtok(NULL, delim);
		if (!net_name)
			continue;

		/* No third token */
		token = strtok(NULL, delim);
		if (token)
			continue;

		/* Insert new network name */
		net_name = xstrdup(net_name);
		list_add(net_name_list, net_name);
	}

	/* Print network names */
	net_debug("%s: loading network configuration file\n",
			net_config_file_name);
	net_debug("networks found:\n");
	for (i = 0; i < net_name_list->count; i++)
		net_debug("\t%s\n", (char *) list_get(net_name_list, i));
	net_debug("\n");

	/* Load networks */
	net_table = hash_table_create(0, 0);
	for (i = 0; i < net_name_list->count; i++)
	{
		struct net_t *network;
		char *net_name;

		net_name = list_get(net_name_list, i);
		network = net_create_from_config(config, net_name);

		hash_table_insert(net_table, net_name, network);
	}

	/* Free list of network names and configuration file */
	while (net_name_list->count)
		free(list_remove_at(net_name_list, 0));
	list_free(net_name_list);
	config_free(config);
}


void net_init(void)
{
	/* Load network configuration file */
	net_read_config();

	/* Register events for network handler*/
	EV_NET_SEND = esim_register_event_with_name(net_event_handler,
			net_domain_index, "net_send");
	EV_NET_OUTPUT_BUFFER = esim_register_event_with_name(net_event_handler,
			net_domain_index, "net_output_buffer");
	EV_NET_INPUT_BUFFER = esim_register_event_with_name(net_event_handler,
			net_domain_index, "net_input_buffer");
	EV_NET_RECEIVE = esim_register_event_with_name(net_event_handler,
			net_domain_index, "net_receive");

	/* Report file */
	if (*net_report_file_name)
	{
		net_report_file = file_open_for_write(net_report_file_name);
		if (!net_report_file)
			fatal("%s: cannot write on network report file",
					net_report_file_name);
	}

	/* Visualization File */
	if (*net_visual_file_name)
	{
		net_visual_file = file_open_for_write(net_visual_file_name);
		if (!net_visual_file)
			fatal("%s: cannot write on network visualization file",
					net_visual_file_name);
	}
}


void net_done(void)
{
	struct net_t *net;

	/* Free list of networks */
	if (net_table)
	{
		for (hash_table_find_first(net_table, (void **) &net); net;
				hash_table_find_next(net_table, (void **) &net))
		{
			/* Dump report for network */
			if (net_report_file)
				net_dump_report(net, net_report_file);

			/* Dump Visualization data in a 'graphplot'
			 * compatible file */
			if (net_visual_file)
			{
				struct net_graph_t *graph;

				graph = net_visual_calc(net);
				net_dump_visual(graph, net_visual_file);
				net_graph_free(graph);
			}
			/* Free network */
			net_free(net);
		}
		hash_table_free(net_table);
	}

	/* Close report file */
	file_close(net_report_file);

	/* Close visualization file */
	file_close(net_visual_file);
}


struct net_t *net_find(char *name)
{
	/* No network list */
	if (!net_table)
		return NULL;

	/* Find network in list */
	return hash_table_get(net_table, name);
}


struct net_t *net_find_first(void)
{
	struct net_t *net;

	/* No network loaded */
	if (!net_table)
		return NULL;

	/* Return first network */
	hash_table_find_first(net_table, (void **) &net);
	return net;
}


struct net_t *net_find_next(void)
{
	struct net_t *net;

	/* No network loaded */
	if (!net_table)
		return NULL;

	/* Return next network */
	hash_table_find_next(net_table, (void **) &net);
	return net;
}


void net_sim(char *debug_file_name)
{
	struct net_t *net;
	double *inject_time;	/* Next injection time (one per node) */

	/* Initialize */
	debug_init();
	esim_init();
	net_init();
	net_debug_category = debug_new_category(debug_file_name);

	/* Network to work with */
	if (!*net_sim_network_name)
		panic("%s: no network", __FUNCTION__);
	net = net_find(net_sim_network_name);
	if (!net)
		fatal("%s: network does not exist", net_sim_network_name);
	/* Initialize */
	inject_time = xcalloc(net->node_count, sizeof(double));

	/* FIXME: error for no dest node in network */

	/* Simulation loop */
	esim_process_events(TRUE);
	if (!strcmp(net_traffic_pattern, "") ||
			(!strcmp(net_traffic_pattern, "uniform")))
	{
		net_traffic_uniform(net, inject_time);
	}
	else if (!strcmp(net_traffic_pattern, "command"))
	{
		while(1)
		{
			long long cycle;

			cycle = esim_domain_cycle(net_domain_index);
			if (cycle >= net_max_cycles)
				break;

			net_debug("___cycle %lld___ \n", cycle);
			esim_process_events(TRUE);
		}
	}
	else
		fatal("Network %s: unknown traffic pattern (%s). \n", net->name
				,net_traffic_pattern);


	/* Drain events */
	esim_process_all_events();

	/* Free */
	free(inject_time);

	/* Finalize */
	net_done();
	esim_done();
	debug_done();

	/* Finish program */
	mhandle_done();
	exit(0);
}
