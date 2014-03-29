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

#ifndef VISUAL_NETWORK_NET_GRAPH_H
#define VISUAL_NETWORK_NET_GRAPH_H

enum vi_net_vertex_kind_t
{
	vi_net_vertex_end_node = 0,
	vi_net_vertex_switch,
	vi_net_vertex_bus
};

struct vi_net_vertex_data_t
{
	int bus_util_color;

	enum vi_net_vertex_kind_t kind;
	struct vi_net_node_t *node;
};

enum vi_net_edge_kind_t
{
	vi_net_edge_link = 0,
	vi_net_edge_bilink,
	vi_net_edge_bus,
	vi_net_edge_bibus
};


struct vi_net_edge_data_t
{
	enum vi_net_edge_kind_t kind;
	struct vi_net_link_t *upstream;
	struct vi_net_link_t *downstream;

	struct graph_vertex_t *bus_vertex;
};




struct vi_net_graph_t
{
	/* associated network */
	struct vi_net_t *net;

	/* associated graph */
	struct graph_t *graph;

	/* meta-data on the graph */
	int vertex_count;
	struct list_t *edge_data_list;

	/* Final scale factor */
	int scale ;

};


struct vi_net_graph_t *vi_net_graph_create             (struct vi_net_t *net);
void                   vi_net_graph_free               (struct vi_net_graph_t * graph);

void                   vi_net_graph_populate           (struct vi_net_t *net, struct vi_net_graph_t *net_graph);
void                   vi_net_graph_draw_scale         (struct vi_net_graph_t *net_graph);

/* Whatever value we deem important to get from graph calculation,
 * would be transfered back to network through this function. */
void                   vi_net_graph_finalize           (struct vi_net_graph_t *net_graph);



#endif
