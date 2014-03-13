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
	net_node_invalid = 0,
	net_node_end,
	net_node_switch,
	net_node_bus,
	net_node_photonic
};

struct vi_net_node_t
{
	enum vi_net_node_kind  node_type;
	char                  *name;
	struct vi_mod_t       *mod;
	struct list_t 		  *input_buffer_list;
	struct list_t 		  *output_buffer_list;
};


struct vi_net_node_t *vi_net_node_create (void);
void                  vi_net_node_free   (struct vi_net_node_t *node);
struct vi_net_node_t *vi_net_node_assign (struct vi_trace_line_t *trace_line);

#endif
