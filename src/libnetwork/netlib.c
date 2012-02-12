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

char *net_config_help =
	"The network configuration file is a plain-text file following the\n"
	"IniFile format. It specifies a set of networks, their nodes, and\n"
	"connections between them. The following set of sections and variables\n"
	"are allowed:\n"
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
	"\n"
	"\n";

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
	"\texpected format. Please run 'm2s --help-net-config' for a list of\n"
	"\tpossible sections/variables in the network configuration file.\n";

char *err_net_can_send =
	"\tAn attempt has been detected of injecting a package in a network\n"
	"\tfrom a source node that has no available space in its output\n"
	"\tbuffer. This can be solved by making sure a message can be sent\n"
	"\tbefore injecting it (use function 'net_can_send').\n";


/* Events */
int EV_NET_SEND;
int EV_NET_OUTPUT_BUFFER;
int EV_NET_INPUT_BUFFER;
int EV_NET_RECEIVE;

/* List of networks */
static struct hash_table_t *net_list;

/* Configuration parameters */
char *net_config_file_name = "";
long long net_max_cycles = 1000000;  /* 1M cycles default */
double net_injection_rate;




/*
 * Public Functions
 */


void net_init(void)
{
	/* Register events */
	EV_NET_SEND = esim_register_event(net_event_handler);
	EV_NET_OUTPUT_BUFFER = esim_register_event(net_event_handler);
	EV_NET_INPUT_BUFFER = esim_register_event(net_event_handler);
	EV_NET_RECEIVE = esim_register_event(net_event_handler);

	/* List of networks */
	net_list = hash_table_create(0, 0);
}


void net_done(void)
{
	/* Free list of networks */
	hash_table_free(net_list);
}


void net_load(char *file_name)
{
}


struct net_t *net_find(char *name)
{
	return NULL;
}


void net_sim(void)
{
}

