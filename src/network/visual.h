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

#ifndef NETWORK_VISUAL_H
#define NETWORK_VISUAL_H

enum net_vertex_kind_t
{
	net_vertex_end_node = 0,
	net_vertex_switch,
	net_vertex_bus
};

struct net_vertex_data_t
{
	int bus_util_color;

	enum net_vertex_kind_t kind;
	struct net_node_t *node;
};

enum net_edge_kind_t
{
	net_edge_link = 0,
	net_edge_bilink,
	net_edge_bus,
	net_edge_bibus
};


struct net_edge_data_t
{
	enum net_edge_kind_t kind;
	struct net_link_t *upstream;
	struct net_link_t *downstream;

	struct graph_vertex_t *bus_vertex;
};


struct net_graph_t
{
	/* associated Network */
	struct net_t *net;

	/* associated Graph */
	struct graph_t *graph;

	/* Associated variables for Visual/Dump */
	int vertex_count;
	int scale;

	/* Associated edge data list */
	struct list_t *edge_data_list;


};

struct net_graph_t *net_graph_create             (struct net_t *net);
void                net_graph_free               (struct net_graph_t * graph);

struct net_graph_t *net_graph_visual_calculation (struct net_t *net);
void                net_graph_populate           (struct net_t *net, struct net_graph_t *net_graph);

void                net_dump_visual              (struct net_graph_t *graph, FILE *f);


#endif /* NETWORK_VISUAL_H */
