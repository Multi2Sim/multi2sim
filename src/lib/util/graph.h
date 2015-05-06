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

#ifndef GRAPH_H
#define GRAPH_H

struct graph_t
{
	int vertex_count;
	int max_vertex_in_layer;

	struct list_t *vertex_list;
	struct list_t *edge_list;
};

enum graph_vertex_type_t
{
	graph_vertex_invalid = 0,
	graph_vertex_node,
	graph_vertex_dummy
};

struct graph_vertex_t
{
	/* Vertex Name */
	char *name;

	/* Vertex Index */
	int index;

	/* X an Y Coordinates - Result */
	int x_coor;
	int y_coor;

	/* Connected Vertices and degree */
	int indeg ;
	struct list_t *incoming_vertex_list;
	int outdeg ;
	struct list_t *outgoint_vertex_list;

	/* Mid-Calculation Values */
	int key_val;
	float cross_num;
	int neighbours;

	/* Vertex Kind */
	enum graph_vertex_type_t kind;

	/* Data depending on the use */
	void *data;
};

enum graph_edge_direction_t
{
	graph_edge_direction_invalid = 0,
	graph_edge_uni_direction,
	graph_edge_bi_direction
};

struct graph_edge_t
{
	/* Direction */
	enum graph_edge_direction_t direction;

	/* Edge Source and Destination Vertices */
	struct graph_vertex_t *src_vertex;
	struct graph_vertex_t *dst_vertex;

	/* Edge additional Data */
	void *data;
	int reversed;
};


struct graph_t        *graph_create          (int init_vertex_count);
void                   graph_free            (struct graph_t *graph);

struct graph_vertex_t *graph_vertex_create   (void);
void                   graph_vertex_free     (struct graph_vertex_t *vertex);

struct graph_edge_t   *graph_edge_create     (void);
void 		       graph_edge_free       (struct graph_edge_t *edge);

void                   graph_layered_drawing (struct graph_t * graph);

#endif
