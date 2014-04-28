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

#ifndef VISUAL_NETWORK_NODE_H
#define VISUAL_NETWORK_NODE_H



/*
 * Network Node
 */

enum vi_net_node_kind
{
	vi_net_node_invalid = 0,
	vi_net_node_end,
	vi_net_node_switch,
	vi_net_node_bus,
	vi_net_node_photonic,
	vi_net_node_dummy
};

struct vi_net_bus_t
{
	int utilization;
};
struct vi_net_node_t
{
	int 		       index;
	enum vi_net_node_kind  type;
	char                  *name;
	struct vi_mod_t       *mod;
	struct hash_table_t   *input_buffer_list;
	struct hash_table_t   *output_buffer_list;

        int max_buffer_size;

	/* Coordination on Network Window */
	double X;
	double Y;

	/* if node is BUS or Photonic */
	struct list_t	    *bus_lane_list;
	struct hash_table_t *src_buffer_list;
	struct hash_table_t *dst_buffer_list;
};

struct vi_trace_line_t;
struct vi_net_packet_t;

struct vi_net_node_t *vi_net_node_create (void);
void                  vi_net_node_free   (struct vi_net_node_t *node);
struct vi_net_node_t *vi_net_node_assign (struct vi_trace_line_t *trace_line);

struct vi_net_bus_t  *vi_net_bus_create  (void);
void                  vi_net_bus_free    (struct vi_net_bus_t *bus);

void 		      vi_node_read_checkpoint(struct vi_net_node_t *node, FILE *f);
void                  vi_node_write_checkpoint(struct vi_net_node_t *node, FILE *f);

void vi_node_insert_packet(struct vi_net_node_t *node, char* buffer_name, int buffer_occupancy, struct vi_net_packet_t *packet);
void vi_node_extract_packet(struct vi_net_node_t *node, char* buffer_name, int buffer_occupancy, struct vi_net_packet_t *packet);

#endif
