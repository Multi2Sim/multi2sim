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
	net_vertex_dummy,
	net_vertex_bus
};

enum net_edge_kind_t
{
	net_edge_link = 0,
	net_edge_bilink,
	net_edge_bus,
	net_edge_bibus
};
struct net_graph_t
{
	struct net_t *net;

	int vertex_count;
	int xscale;
	struct list_t *vertex_list;
	struct list_t *edge_list;

};

struct net_graph_vertex_t {
	int x_coor;
	int y_coor;
	int indeg ;
	int outdeg ;
	int key_val;  // acts as Pi_val and degdif
	float cross_num;
	int neighbours;

	char *name;
	int bus_util_color;
	enum net_vertex_kind_t kind;
	struct net_graph_t *graph;
	struct net_node_t *node;
	struct list_t *incoming_vertex_list;
	struct list_t *outgoint_vertex_list;

};

struct net_graph_edge_t {
	enum net_edge_kind_t kind;
	struct net_graph_t *graph;
	struct net_graph_vertex_t *src_vertex;
	struct net_graph_vertex_t *dst_vertex;

	/* an edge can be either a link or a bus */

	struct net_link_t *upstream;
	struct net_link_t *downstream;

	struct net_graph_vertex_t *bus_vertex;

};

struct net_graph_t *net_graph_create(struct net_t *net);
void net_graph_free (struct net_graph_t * graph);

struct net_graph_vertex_t *net_graph_vertex_create(struct net_graph_t *graph, char *name);
void net_graph_vertex_free (struct net_graph_vertex_t *vertex);

struct net_graph_edge_t *net_graph_edge_create(struct net_graph_t *graph);
void net_graph_edge_free (struct net_graph_edge_t *edge);

/*For calculating the visual coordinations*/
struct net_graph_t *net_visual_calc(struct net_t  *net);

/* First Step: Removing the cycles in the graph and making sure it is connected*/
void net_graph_greedy_cycle_removal (struct net_graph_t *graph);

/* Second Step: Assigning optimized y coordinates for nodes */
void net_graph_coffman_graham_layering (struct net_graph_t *graph, int width);
/* A vital part in second step */
void net_graph_label_assignment(struct net_graph_t *graph);
/*Third step : Reducing the number of Crosses in the graph; It is a NP-Complete Problem*/
void net_graph_cross_reduction(struct net_graph_t *graph, int layer_count);

struct net_graph_vertex_t *net_get_vertex_by_node(struct net_graph_t * graph,
		struct net_node_t *node);




void net_dump_visual(struct net_graph_t *graph, FILE *f);


#endif /* NETWORK_VISUAL_H */
