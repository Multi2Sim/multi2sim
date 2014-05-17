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
#include "command.h"
#include "config.h"
#include "link.h"
#include "net-system.h"
#include "network.h"
#include "node.h"
#include "routing-table.h"
#include "visual.h"

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

char *net_config_file_name = "";

char *net_route_file_name = "";

/*
 * Prototype
 */
static void 	     net_config_command_create     (struct net_t *net, struct config_t *config, char *section);
static void 	     net_config_route_create       (struct net_t *net, struct config_t *config, char *section);
static void          net_read_from_config_nodes    (struct net_t* net, struct config_t *config);
static void          net_read_from_config_links    (struct net_t* net, struct config_t *config);
static int           net_read_from_config_routes   (struct net_t *net, struct config_t *config);
static void          net_read_from_config_commands (struct net_t *net, struct config_t *config);
static struct net_t* net_create_from_config        (struct config_t *config, char *name);

/*
 * Private Functions
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

        /* Create trace category. This needs to be done before reading the
         * network configuration file with 'net_read_config', since the latter
         * function generates the trace headers. */
        net_trace_category = trace_new_category();


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

        /* If the network is external and trace is active,
         * we create trace with detail about configuration
	net_config_trace(); */
}


static struct net_t *net_create_from_config(struct config_t *config, char *name)
{
        int routing_type = 0;
        struct net_t *net;
        char *section;
        char section_str[MAX_STRING_SIZE];
        int def_packet_size;

        /* Create network */
        net = net_create(name);

        /* Main section */
        snprintf(section_str, sizeof section_str, "Network.%s", name);
        for (section = config_section_first(config); section;
                        section = config_section_next(config))
        {
                if (strcasecmp(section, section_str))
                        continue;

                net->def_input_buffer_size = config_read_int(config, section,
                                "DefaultInputBufferSize", 0);
                net->def_output_buffer_size = config_read_int(config, section,
                                "DefaultOutputBufferSize", 0);
                net->def_bandwidth = config_read_int(config, section,
                                "DefaultBandwidth", 0);
                def_packet_size = config_read_int(config, section,
                                "DefaultPacketSize", 0);
                if (!net->def_input_buffer_size)
                        fatal("%s:%s: DefaultInputBufferSize: invalid/missing value.\n%s",
                                        net->name, section, net_err_config);
                if (!net->def_output_buffer_size)
                        fatal("%s:%s: DefaultOutputBufferSize: invalid/missing value.\n%s",
                                        net->name, section, net_err_config);
                if (!net->def_bandwidth)
                        fatal("%s:%s: DefaultBandwidth: invalid/missing value.\n%s",
                                        net->name, section, net_err_config);

                /*
                 * In a network simulation if there is no packet size defined, packet size is equal to
                 * msg size (which is always constant)
                 */
                net->packet_size = def_packet_size;
        }

        /* Nodes */
        net_read_from_config_nodes(net, config);

        /* Links */
        net_read_from_config_links(net, config);

        /* initializing the routing table */
        net_routing_table_initiate(net->routing_table);

        /* Routes */
        routing_type = net_read_from_config_routes(net, config);

        /* Commands */
        net_read_from_config_commands(net, config);

        /* If there is no route section, Floyd-Warshall calculates the
         * shortest path for all the nodes in the network */
        if (routing_type == 0)
                net_routing_table_floyd_warshall(net->routing_table);

        /* Return */
        return net;
}

static void net_read_from_config_nodes(struct net_t* net, struct config_t *config)
{
        char *section;
        char section_str[MAX_STRING_SIZE];

        for (section = config_section_first(config); section;
                        section = config_section_next(config))
        {
                char *delim = ".";
                char *token;
                char *node_name;
                char *node_type;

                int input_buffer_size;
                int output_buffer_size;
                int bandwidth;
                int lanes;	/* BUS lanes */
                int fix_delay;

                /* First token must be 'Network' */
                snprintf(section_str, sizeof section_str, "%s", section);
                token = strtok(section_str, delim);
                if (!token || strcasecmp(token, "Network"))
                        continue;

                /* Second token must be the name of the network */
                token = strtok(NULL, delim);
                if (!token || strcasecmp(token, net->name))
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
                                "InputBufferSize", net->def_input_buffer_size);
                output_buffer_size = config_read_int(config, section,
                                "OutputBufferSize", net->def_output_buffer_size);
                bandwidth = config_read_int(config, section,
                                "BandWidth", net->def_bandwidth);
                lanes = config_read_int(config, section, "Lanes", 1);
                fix_delay = config_read_int(config, section, "FixDelay", 0);

                /* Create node */
                if (!strcasecmp(node_type, "EndNode"))
                {
                        int end_node_input_buffer_size = input_buffer_size;
                        int end_node_output_buffer_size = output_buffer_size;
                        /*end-node should be able to contain an entire msg or
                         * or equivalent number of packets for that message.*/
                        if(input_buffer_size <= net_msg_size)
                        {
                                if(net->packet_size != 0)
                                        end_node_input_buffer_size = ((net_msg_size - 1)/
                                                        net->packet_size + 1) * net->packet_size;
                                else
                                        end_node_input_buffer_size = net_msg_size;
                        }

                        if (output_buffer_size <= net_msg_size)
                        {
                                if(net->packet_size != 0)
                                        end_node_output_buffer_size = ((net_msg_size - 1)/
                                                        net->packet_size + 1) * net->packet_size;
                                else
                                        end_node_output_buffer_size = net_msg_size;
                        }
                        net_add_end_node(net, end_node_input_buffer_size,
                                        end_node_output_buffer_size, node_name, NULL);
                }
                else if (!strcasecmp(node_type, "Switch"))
                        net_add_switch(net, input_buffer_size,
                                        output_buffer_size, bandwidth, node_name);
                else if (!strcasecmp(node_type, "Bus"))
                {
                        /* Right now we ignore the size of buffers. But we
                         * can set it as the value for bus ports, making the
                         * connecting switches asymmetric. */
                        if (input_buffer_size != net->def_input_buffer_size ||
                                        output_buffer_size != net->def_output_buffer_size)
                                fatal("%s:%s: BUS does not contain input/output buffers \n. ",
                                                net->name, section);

                        /* If the number of lanes is smaller than 1 produce
                         * an error */
                        if (lanes < 1)
                                fatal("%s:%s: BUS cannot have less than 1 number of lanes \n%s",
                                                net->name, section, net_err_config);
                        if (fix_delay < 0)
                        	fatal("%s:%s: BUS cannot have negative wire delay \n%s",
                        	                    net->name, section, net_err_config);
                        net_add_bus(net, bandwidth, node_name, lanes, fix_delay);
                }
                else if (!strcasecmp(node_type, "Photonic"))
                {
                        int wavelength = config_read_int(config, section, "WavelengthSize", net->packet_size);
                        int waveguides = config_read_int(config, section, "Waveguide", 1);


                        net_add_photonic(net, input_buffer_size, output_buffer_size,
                                        node_name, waveguides, wavelength);


                }
                else
                        fatal("%s:%s: Type: invalid/missing value.\n%s",
                                        net->name, section, net_err_config);
        }

}

static void net_read_from_config_links(struct net_t *net, struct config_t* config)
{
        char *section;
        char section_str[MAX_STRING_SIZE];

        for (section = config_section_first(config); section;
                        section = config_section_next(config))
        {
                char *delim = ".";

                char *token;
                char *link_name;
                char *link_type;
                char *src_node_name;
                char *dst_node_name;

                int bandwidth;
                int v_channel_count;
                int src_buffer_size;
                int dst_buffer_size;

                struct net_node_t *src_node;
                struct net_node_t *dst_node;

                /* First token must be 'Network' */
                snprintf(section_str, sizeof section_str, "%s", section);
                token = strtok(section_str, delim);
                if (!token || strcasecmp(token, "Network"))
                        continue;

                /* Second token must be the name of the network */
                token = strtok(NULL, delim);
                if (!token || strcasecmp(token, net->name))
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
                                        net->name, section, net_err_config);

                /* Fields */
                link_type = config_read_string(config, section, "Type",
                                "Unidirectional");
                bandwidth = config_read_int(config, section, "Bandwidth",
                                net->def_bandwidth);
                src_node_name = config_read_string(config, section, "Source", "");
                dst_node_name = config_read_string(config, section, "Dest", "");
                v_channel_count = config_read_int(config, section, "VC", 1);
                src_buffer_size = config_read_int(config, section,
                                "SourceBufferSize", 0);
                dst_buffer_size = config_read_int(config, section,
                                "DestBufferSize", 0);


                /* Nodes */
                src_node = net_get_node_by_name(net, src_node_name);
                dst_node = net_get_node_by_name(net, dst_node_name);

                if (!src_node)
                        fatal("%s: %s: %s: source node does not exist.\n%s",
                                        net->name, section, src_node_name, net_err_config);
                if (!dst_node)
                        fatal("%s: %s: %s: destination node does not exist.\n%s",
                                        net->name, section, dst_node_name, net_err_config);


                /* If is is a Bus Connection */
                if ((src_node->kind != net_node_photonic && dst_node->kind == net_node_bus) ||
                                ((src_node->kind == net_node_bus) && (dst_node->kind != net_node_photonic)))
                {

                        if (v_channel_count > 1)
                                fatal("%s: %s: BUS can not have virtual channels. \n %s",
                                                net->name, section, net_err_config);

                        if (!strcasecmp(link_type, "Unidirectional"))
                        {
                                if ((src_node->kind == net_node_bus &&
                                                src_buffer_size) ||
                                                (dst_node->kind == net_node_bus &&
                                                                dst_buffer_size))
                                {
                                        fatal ("%s: %s: Source/Destination BUS cannot have buffer. \n %s "
                                                        ,net->name, section, net_err_config);
                                }

                                net_add_bus_port(net, src_node, dst_node,
                                                src_buffer_size, dst_buffer_size);
                        }
                        else if (!strcasecmp(link_type, "Bidirectional"))
                        {
                                net_add_bidirectional_bus_port(net, src_node,
                                                dst_node, src_buffer_size,
                                                dst_buffer_size);
                        }

                }

                /* If it is a photonic connection */
                else if ((src_node->kind == net_node_photonic && dst_node->kind != net_node_bus)
                                || (src_node->kind != net_node_bus && dst_node->kind == net_node_photonic))
                {
                        if (v_channel_count > 1)
                                fatal("%s: %s: BUS can not have virtual channels. \n %s",
                                                net->name, section, net_err_config);


                        if (!strcasecmp(link_type, "Unidirectional"))
                        {
                                if ((src_node->kind == net_node_bus &&
                                                src_buffer_size) ||
                                                (dst_node->kind == net_node_bus &&
                                                                dst_buffer_size))
                                {
                                        fatal ("%s: %s: Source/Destination BUS cannot have buffer. \n %s "
                                                        ,net->name, section, net_err_config);
                                }

                                net_add_channel_port(net, src_node, dst_node,
                                                src_buffer_size, dst_buffer_size);
                        }
                        else if (!strcasecmp(link_type, "Bidirectional"))
                        {
                                net_add_bidirectional_channel_port(net, src_node,
                                                dst_node, src_buffer_size,
                                                dst_buffer_size);
                        }


                }

                /* If it is a photonic to bus connection */
                else if ((src_node->kind == net_node_photonic && dst_node->kind == net_node_bus)
                                || (src_node->kind == net_node_bus && dst_node->kind == net_node_photonic))
                {
                        fatal("%s:%s:Connecting bus to photonic bus is not supported \n", net->name, section);
                }

                /* If it is a normal link connection */
                else
                {
                        int link_src_bsize;
                        int link_dst_bsize;

                        if (v_channel_count >= 1)
                        {

                                if (!strcasecmp(link_type, "Unidirectional"))
                                {
                                        link_src_bsize = (src_buffer_size)? src_buffer_size :
                                                        src_node->output_buffer_size;
                                        link_dst_bsize = (dst_buffer_size) ?dst_buffer_size :
                                                        dst_node->input_buffer_size;

                                        net_add_link(net, src_node, dst_node,
                                                        bandwidth, link_src_bsize,
                                                        link_dst_bsize,
                                                        v_channel_count);
                                }
                                else if (!strcasecmp(link_type,
                                                "Bidirectional"))
                                {
                                        net_add_bidirectional_link(net,
                                                        src_node, dst_node, bandwidth,
                                                        src_buffer_size,
                                                        dst_buffer_size,
                                                        v_channel_count);
                                }
                        }
                        else
                                fatal("%s: %s: Unacceptable number of virtual channels \n %s",
                                                net->name, section, net_err_config);
                }
        }
}

static int net_read_from_config_routes(struct net_t *net, struct config_t *config)
{
        int routing_type = 0;
        char *section;
        char section_str[MAX_STRING_SIZE];
        for (section = config_section_first(config); section;
                        section = config_section_next(config))
        {
                char *delim = ".";

                char *token;
                char *token_endl;

                /* First token must be 'Network' */
                snprintf(section_str, sizeof section_str, "%s", section);
                token = strtok(section_str, delim);
                if (!token || strcasecmp(token, "Network"))
                        continue;

                /* Second token must be the name of the network */
                token = strtok(NULL, delim);
                if (!token || strcasecmp(token, net->name))
                        continue;

                /* Third token must be 'Routes' */
                token = strtok(NULL, delim);
                if (!token || strcasecmp(token, "Routes"))
                        continue;

                token_endl = strtok(NULL, delim);
                if (token_endl)
                        fatal("%s: %s: bad format for route.\n%s",
                                        net->name, section, net_err_config);

                /* Routes */
                routing_type = 1;
                net_config_route_create(net, config, section);
                config_check(config);
        }
        return routing_type;
}

static void net_read_from_config_commands(struct net_t *net, struct config_t *config)
{
        char *section;
        char section_str[MAX_STRING_SIZE];

        for (section = config_section_first(config); section;
                        section = config_section_next(config))
        {
                char *delim = ".";

                char *token;
                char *token_endl;

                /* First token must be 'Network' */
                snprintf(section_str, sizeof section_str, "%s", section);
                token = strtok(section_str, delim);
                if (!token || strcasecmp(token, "Network"))
                        continue;

                /* Second token must be the name of the network */
                token = strtok(NULL, delim);
                if (!token || strcasecmp(token, net->name))
                        continue;

                /* Third token must be 'Commands' */
                token = strtok(NULL, delim);
                if (!token || strcasecmp(token, "Commands"))
                        continue;

                token_endl = strtok(NULL, delim);
                if (token_endl)
                        fatal("%s: %s: bad format for Commands section.\n%s",
                                        net->name, section, net_err_config);

                /* Commands */
                net_config_command_create(net, config, section);
                config_check(config);
        }
}

static void net_config_command_create(struct net_t *net, struct config_t *config, char *section)
{
        char *command_line;
        char command_var[MAX_STRING_SIZE];

        int command_var_id;

        /* Checks */
        if (net_injection_rate > 0.001)
                fatal("Network %s:%s: Using Command section; \n"
                                "\t option --net-injection-rate should not be used \n",
                                net->name,section);
        /* Read commands */
        net_injection_rate = 0;
        if (strcmp(net_traffic_pattern, "") &&
                        (strcmp(net_traffic_pattern, "command")))
                fatal("Network %s: Command option doesn't comply with other "
                                "traffic pattern\n (%s)", net->name,
                                net_traffic_pattern);
        net_traffic_pattern = "command";
        command_var_id = 0;

        /* Register events for command handler*/
        EV_NET_COMMAND = esim_register_event_with_name(net_command_handler,
                        net_domain_index, "net_command");
        EV_NET_COMMAND_RCV = esim_register_event_with_name(net_command_handler,
                        net_domain_index, "net_command_receive");


        while (1)
        {
                /* Get command */
                snprintf(command_var, sizeof command_var, "Command[%d]", command_var_id);
                command_line = config_read_string(config, section, command_var, NULL);
                if (!command_line)
                        break;

                /* Schedule event to process command */
                struct net_command_stack_t *stack;
                stack = net_command_stack_create(net,ESIM_EV_NONE, NULL);
                stack->net = net;
                stack->command = xstrdup(command_line);
                esim_schedule_event(EV_NET_COMMAND, stack, 0);

                /* Next command */
                command_var_id++;
        }
}

static void net_config_route_create(struct net_t *net, struct config_t *config, char *section)
{
        char *token;
        char section_str[MAX_STRING_SIZE];
        char *delim_sep = ":";

        for (int i = 0; i < net->node_count; i++)
        {

                for (int j = 0; j < net->node_count; j++)
                {
                        int vc_used = 0;

                        char spr_result_size[MAX_STRING_SIZE];
                        char *nxt_node_name;

                        struct net_node_t *src_node_r;
                        struct net_node_t *dst_node_r;
                        struct net_node_t *nxt_node_r;

                        src_node_r = list_get(net->node_list, i);
                        dst_node_r = list_get(net->node_list, j);

                        if (dst_node_r->kind == net_node_end)
                        {
                                snprintf(spr_result_size,
                                                sizeof spr_result_size,
                                                "%s.to.%s", src_node_r->name,
                                                dst_node_r->name);
                                nxt_node_name =
                                                config_read_string(config,
                                                                section, spr_result_size,
                                                                "---");

                                /* Token Separates the next node and
                                 * VC */
                                snprintf(section_str,
                                                sizeof section_str, "%s",
                                                nxt_node_name);
                                token = strtok(section_str,
                                                delim_sep);
                                nxt_node_name = token;
                                token = strtok(NULL, delim_sep);

                                if (token != NULL)
                                {
                                        vc_used = atoi(token);
                                        if (vc_used < 0)
                                                fatal("Network %s:%s: Unacceptable virtual channel \n %s",
                                                                net->name, section, net_err_config);
                                }

                                int name_check =
                                                strcmp(nxt_node_name, "---");
                                nxt_node_r =
                                                net_get_node_by_name(net,
                                                                nxt_node_name);

                                if (name_check != 0)
                                {
                                        if (nxt_node_r == NULL)
                                                fatal("Network %s:%s: Invalid node Name.\n %s",
                                                                net->name, section,net_err_config);

                                        else
                                                net_routing_table_route_create
                                                (net->routing_table,
                                                                src_node_r,
                                                                dst_node_r,
                                                                nxt_node_r,
                                                                vc_used);
                                }
                        }
                }
        }
}
