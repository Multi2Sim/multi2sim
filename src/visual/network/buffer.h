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

#ifndef VISUAL_NETWORK_BUFFER_H
#define VISUAL_NETWORK_BUFFER_H

enum vi_buffer_direction_t
{
	vi_buffer_dir_invalid = 0,
	vi_buffer_dir_input,
	vi_buffer_dir_output
};

enum vi_buffer_connect_t
{
	vi_buffer_invalid = 0,
	vi_buffer_link,
	vi_buffer_bus,
	vi_buffer_photonic
};

struct vi_net_buffer_t
{
	int			    id;
	int			    size;
	int			    occupancy;
	int                         packet_capacity;
	enum vi_buffer_connect_t    connection_type;
	enum vi_buffer_direction_t  direction;

	char                       *name;
	struct vi_net_node_t       *node;

	/* Either a link connection or a bus connection */
	struct vi_net_link_t 	   *link;

	struct list_t 	   *packet_list;

};

struct vi_trace_line_t;
extern struct str_map_t vi_net_buffer_attrib_map;
extern struct str_map_t vi_net_buffer_direction_map;


struct vi_net_buffer_t *vi_net_buffer_create (struct vi_trace_line_t *trace_line, enum vi_buffer_direction_t);
void                    vi_net_buffer_free   (struct vi_net_buffer_t *buffer);

void 			vi_buffer_write_checkpoint (struct vi_net_buffer_t *buffer, FILE *f);
void 			vi_buffer_read_checkpoint (struct vi_net_buffer_t *buffer, FILE *f);
#endif
