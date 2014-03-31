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

#ifndef VISUAL_NETWORK_NET_H
#define VISUAL_NETWORK_NET_H


struct vi_net_t
{
	char *name;
	int   packet_size;

	struct list_t *node_list; /* Keeping it for backward compatibility */

	/* Keeping the dummy node structures. Nodes of this kind would not
	 * go in neither node_list nor node table. It is for visualization
	 * porpose only */
	struct list_t *dummy_node_list;

	struct hash_table_t *node_table;
	struct hash_table_t *link_table;
	struct hash_table_t *high_mods;
	struct hash_table_t *low_mods;

};

struct vi_trace_line_t;

struct vi_net_t *vi_net_create(struct vi_trace_line_t *trace_line);
void             vi_net_free  (struct vi_net_t *net);

struct vi_mod_t;

void             vi_net_attach_mod(struct vi_net_t *net, struct vi_mod_t *mod, int node_index);
struct vi_mod_t *vi_net_get_mod   (struct vi_net_t *net, int node_index);

void             vi_net_graph_visual_calculation (struct vi_net_t *net);

#endif
