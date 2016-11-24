/*
 *  Multi2Sim Tools
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
#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <visual/common/trace.h>

#include "link.h"
#include "net.h"
#include "net-graph.h"
#include "net-message.h"
#include "net-packet.h"
#include "node.h"

/*
 * Network
 */

struct vi_net_t *vi_net_create(struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;

	int num_nodes;
	int packet_size;
	int i;

	char *name;

	/* Initialize */
	net = xcalloc(1, sizeof(struct vi_net_t));

	/* Name */
	name = vi_trace_line_get_symbol(trace_line, "name");
	net->name = xstrdup(name);

	/* Node list */
	num_nodes = vi_trace_line_get_symbol_int(trace_line, "num_nodes");
	net->node_list = list_create();
	for (i = 0; i < num_nodes; i++)
		list_add(net->node_list, vi_net_node_create());

	/* Get the packet size, if it is in the trace */
	packet_size = vi_trace_line_get_symbol_int(trace_line, "packet_size");
	if (packet_size != 0)
		net->packet_size = packet_size;

	net->dummy_node_list = list_create();

	net->high_mods = hash_table_create(0, FALSE);
	net->low_mods  = hash_table_create(0, FALSE);
	net->link_table = hash_table_create(0, FALSE);
	net->node_table = hash_table_create(0, FALSE);
	net->message_table = hash_table_create(0, FALSE);
	net->packet_table = hash_table_create(0, FALSE);

	/* Return */
	return net;
}


void vi_net_free(struct vi_net_t *net)
{
	int i;
	char *link_name;
	struct vi_net_link_t *link;

	char *node_name;
	struct vi_net_node_t *node;

	char *message_name;
	struct vi_net_message_t *message;

	char *packet_name;
	struct vi_net_packet_t *packet;

	/* Free Packets */
	HASH_TABLE_FOR_EACH(net->packet_table, packet_name, packet)
	                vi_net_packet_free(packet);
        hash_table_free(net->packet_table);

	/* Free messages */
	HASH_TABLE_FOR_EACH(net->message_table, message_name, message)
	                vi_net_message_free(message);
        hash_table_free(net->message_table);

        /* Free nodes */
	LIST_FOR_EACH(net->node_list, i)
	{
		node = list_get(net->node_list, i);
		if (node->name)
		{
			struct vi_net_node_t *table_node;
			table_node = hash_table_remove(net->node_table, node->name);
			assert(node == table_node);
		}
		vi_net_node_free(node);
	}
	list_free(net->node_list);

	/*Redundant: if it goes here something was wrong - Fail safe */
	HASH_TABLE_FOR_EACH(net->node_table, node_name, node)
		vi_net_node_free(node);
	hash_table_free(net->node_table);

	/* Free Links */
	HASH_TABLE_FOR_EACH(net->link_table, link_name, link)
		vi_net_link_free(link);
	hash_table_free(net->link_table);

	/* Free mod tables - Not freeing mods, since it has been
	 * freed before by mem-system */
	hash_table_free(net->high_mods);
	hash_table_free(net->low_mods);


	for (i = 0; i < list_count(net->dummy_node_list); i++)
	{
	                struct vi_net_node_t *dummy_node;
	                dummy_node = list_get(net->dummy_node_list, i);
	                assert (dummy_node->type == vi_net_node_dummy);
	                vi_net_node_free(dummy_node);
	}
	list_free(net->dummy_node_list);

	/* Free network */
	free(net->name);
	free(net);
}


void vi_net_attach_mod(struct vi_net_t *net,
		struct vi_mod_t *mod, int node_index)
{
	struct vi_net_node_t *node;

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, net->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	/* Attach */
	node = list_get(net->node_list, node_index);
	assert(node);
	node->mod = mod;
}


struct vi_mod_t *vi_net_get_mod(struct vi_net_t *net, int node_index)
{
	struct vi_net_node_t *node;

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, net->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	/* Return */
	node = list_get(net->node_list, node_index);
	return node->mod;
}

void vi_net_graph_visual_calculation (struct vi_net_t *net)
{
	struct vi_net_graph_t *net_graph;

	net_graph = vi_net_graph_create(net);
	vi_net_graph_populate(net, net_graph);
	vi_net_graph_draw_scale(net_graph);
	vi_net_graph_finalize(net_graph);
	vi_net_graph_free(net_graph);
}

void vi_net_read_checkpoint(struct vi_net_t *net, FILE *f)
{
        char node_name[MAX_STRING_SIZE];
        char link_name[MAX_STRING_SIZE];

        struct vi_net_node_t *node;
        struct vi_net_link_t *link;

        struct vi_net_message_t *message;
        char *message_name;

        struct vi_net_packet_t *packet;
        char *packet_name;

        int num_messages;
        int num_packets;
        int count;
        int i;

        /* Empty access list */
        HASH_TABLE_FOR_EACH(net->message_table, message_name, message)
                vi_net_message_free(message);
        hash_table_clear(net->message_table);

        /* Empty access list */
        HASH_TABLE_FOR_EACH(net->packet_table, packet_name, packet)
                vi_net_packet_free(packet);
        hash_table_clear(net->packet_table);

        /* Read number of Messages */
        count = fread(&num_messages, 1, 4, f);
        if (count != 4)
                fatal("%s: error reading from checkpoint", __FUNCTION__);

        /* Read Messages */
        for (i = 0; i < num_messages; i++)
        {
                message = vi_net_message_create(NULL, NULL, 0);
                vi_net_message_read_checkpoint(message, f);
                hash_table_insert(net->message_table, message->name, message);
        }

        /* Read number of Packets */
        count = fread(&num_packets, 1, 4, f);
        if (count != 4)
                fatal("%s: error reading from checkpoint", __FUNCTION__);

        /* Read Messages */
        for (i = 0; i < num_packets; i++)
        {
                packet = vi_net_packet_create(NULL, NULL, 0);
                vi_net_packet_read_checkpoint(packet, f);
                hash_table_insert(net->packet_table, packet->name, packet);
        }


        /* Read Nodes */
        for (i = 0; i < hash_table_count(net->node_table); i++)
        {
                /* Get module */
                str_read_from_file(f, node_name, sizeof node_name);
                node = hash_table_get(net->node_table, node_name);
                if (!node)
                        panic("%s: %s: invalid module name", __FUNCTION__, node_name);

                /* Read module checkpoint */
                vi_node_read_checkpoint(node, f);
        }

        /* Read Links */
        for (i = 0; i < hash_table_count(net->link_table); i++)
        {
                /* Get module */
                str_read_from_file(f, link_name, sizeof link_name);
                link = hash_table_get(net->link_table, link_name);
                if (!link)
                        panic("%s: %s: invalid module name", __FUNCTION__, link_name);

                /* Read module checkpoint */
                vi_link_read_checkpoint(link, f);
        }

}

void vi_net_write_checkpoint(struct vi_net_t *net, FILE *f)
{
        struct vi_net_node_t *node;
        struct vi_net_link_t *link;
        struct vi_net_message_t *message;
        struct vi_net_packet_t *packet;
        char *node_name;
        char *link_name;
        char *message_name;
        char *packet_name;

        int num_messages;
        int num_packets;
        int count;

        /* Write number of messages */
        num_messages = hash_table_count(net->message_table);
        count = fwrite(&num_messages, 1, 4, f);
        if (count != 4)
                fatal("%s: cannot write to checkpoint file", __FUNCTION__);

        /* Write messages */
        HASH_TABLE_FOR_EACH(net->message_table, message_name, message)
                vi_net_message_write_checkpoint(message, f);

        /* Write number of packets */
        num_packets = hash_table_count(net->packet_table);
        count = fwrite(&num_packets, 1, 4, f);
        if (count != 4)
                fatal("%s: cannot write to checkpoint file", __FUNCTION__);

        /* Write packets */
        HASH_TABLE_FOR_EACH(net->packet_table, packet_name, packet)
                vi_net_packet_write_checkpoint(packet, f);

        /* Write Nodes */
        HASH_TABLE_FOR_EACH(net->node_table, node_name, node)
        {
                str_write_to_file(f, node->name);
                vi_node_write_checkpoint(node, f);
        }

        /* Write Links */
        HASH_TABLE_FOR_EACH(net->link_table, link_name, link)
        {
                str_write_to_file(f, link->name);
                vi_link_write_checkpoint(link, f);
        }

}
