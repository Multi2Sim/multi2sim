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

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/hash-table.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/mhandle/mhandle.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>

#include "buffer.h"
#include "node.h"
#include "net.h"
#include "net-packet.h"
#include "net-system.h"

struct str_map_t vi_net_buffer_attrib_map =
{
        6,
        {
                { "Direction", 0 },
                { "Connection Name", 1},
                { "Link Util" , 2},
                { "Buffer Name", 3},
                { "Buffer Size", 4},
                { "Buffer Util", 5}
        }
};

struct str_map_t vi_net_buffer_direction_map =
{
	3,
	{
		{ "Invalid", vi_buffer_dir_invalid},
		{ "--> In", vi_buffer_dir_input},
		{ "<-- Out", vi_buffer_dir_output}
	}
};

struct vi_net_buffer_t *vi_net_buffer_create (struct vi_trace_line_t *trace_line,
		enum vi_buffer_direction_t buffer_direction)
{
	struct vi_net_buffer_t *buffer;

	/* Return */
	buffer = xcalloc(1, sizeof(struct vi_net_buffer_t));

	/* Get network Name */
	char *name;
	struct vi_net_t *net;
	struct vi_net_node_t *node;

	/* Get the network name */
	name = vi_trace_line_get_symbol(trace_line, "net_name");
	net = hash_table_get(vi_net_system->net_table, name);

	assert(net);

	/* Get the Node */
	char *node_name;
	node_name = vi_trace_line_get_symbol(trace_line, "node_name");

	node = hash_table_get(net->node_table, node_name);
	assert(node);
	buffer->node = node;

	/* Get the buffer by index and direction */
	char *buffer_name;
	buffer_name = vi_trace_line_get_symbol(trace_line, "buffer_name");
	buffer->name = xstrdup(buffer_name);

	buffer->direction = buffer_direction;

	if (buffer_direction == vi_buffer_dir_input)
	{
		hash_table_insert(node->input_buffer_list, buffer->name, buffer);
		buffer->id = hash_table_count(node->input_buffer_list) - 1;
	}
	else if (buffer_direction == vi_buffer_dir_output)
	{
		hash_table_insert(node->output_buffer_list, buffer->name, buffer);
		buffer->id = hash_table_count(node->output_buffer_list) - 1;
	}
	else
		panic("%s: buffer direction is not valid", __FUNCTION__);

	/* Set buffer size */
	int buffer_size ;
	buffer_size = vi_trace_line_get_symbol_int(trace_line, "buffer_size");
	buffer->size = buffer_size;
/*	if (net->packet_size != 0)
	        buffer->packet_capacity = (buffer_size -1 ) / net->packet_size + 1;
	else
	        buffer->packet_capacity = buffer->size;

        if (node->max_buffer_size < buffer->packet_capacity)
                node->max_buffer_size = buffer->packet_capacity;
*/
	/* Set buffer connection Type */
	int connection_type;
	connection_type = vi_trace_line_get_symbol_int(trace_line, "buffer_type");
	buffer->connection_type = connection_type;

	/* Set buffer Link */
	if (buffer->connection_type == vi_buffer_link)
	{
		char *link_name;
		struct vi_net_link_t *link;

		link_name = vi_trace_line_get_symbol(trace_line, "connection");
		link = hash_table_get(net->link_table, link_name);
		buffer->link = link;

	}
	else if ((buffer->connection_type == vi_buffer_bus ) ||
			(buffer->connection_type == vi_buffer_photonic ))
	{
		char *bus_node_name;
		struct vi_net_node_t *bus_node;
		bus_node_name = vi_trace_line_get_symbol(trace_line, "connection");
		bus_node = hash_table_get(net->node_table, bus_node_name);
		assert(bus_node && ((bus_node->type == vi_net_node_bus) || (bus_node->type == vi_net_node_photonic)));

		if (buffer->direction == vi_buffer_dir_input)
			hash_table_insert(bus_node->dst_buffer_list, buffer->name, buffer);
		else if (buffer->direction == vi_buffer_dir_output)
			hash_table_insert(bus_node->src_buffer_list, buffer->name, buffer);

	}

	buffer->packet_list = list_create();

	return buffer;
}

void vi_net_buffer_free (struct vi_net_buffer_t *buffer)
{
	struct list_t *packet_list;
	packet_list = buffer->packet_list;


	/* This portion was removed. This is because we free the packets
	 * via the net->packet_list. The only remaining thing here, at
	 * this scenario is the pointers to non-existant packets. Make sure.
	 */
	/* Free packets */
	int i;

	LIST_FOR_EACH(packet_list, i)
		vi_net_packet_free(list_get(packet_list, i));

	list_clear(packet_list);
	list_free(packet_list);

	free(buffer->name);
	free(buffer);
}

void vi_buffer_write_checkpoint (struct vi_net_buffer_t *buffer, FILE *f)
{
	int i;
	int occupancy;
	int count;
	int num_packets;
	struct vi_net_packet_t *packet;

	/* Write buffer occupancy */
	occupancy = buffer->occupancy;
	count = fwrite(&occupancy, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

	/* Write number of packets*/
	num_packets = list_count(buffer->packet_list);
	count = fwrite(&num_packets, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

	/* Write accesses */
	LIST_FOR_EACH(buffer->packet_list, i)
	{
		packet = list_get(buffer->packet_list, i);
		vi_net_packet_write_checkpoint(packet, f);
	}
}

void vi_buffer_read_checkpoint (struct vi_net_buffer_t *buffer, FILE *f)
{
	int buffer_occupancy;
	int count;

	struct vi_net_packet_t *packet;
	/* Reading buffer Occupancy */
	count = fread(&buffer_occupancy, 1, 4, f);
	if (count != 4)
		fatal("%s: error reading from checkpoint", __FUNCTION__);
	buffer->occupancy = buffer_occupancy;

	/* Empty List */
	list_clear(buffer->packet_list);


	/* Reading each buffer's number of packets */
	int num_packets;

	count = fread(&num_packets, 1, 4, f);
	if (count != 4)
		fatal("%s: error reading from checkpoint", __FUNCTION__);
	for (int i = 0; i < num_packets ; i++)
	{
		packet = vi_net_packet_create(NULL, NULL, 0);
		vi_net_packet_read_checkpoint(packet, f);
		list_add(buffer->packet_list, packet);
	}
}
