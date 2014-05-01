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

#include <assert.h>

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <lib/util/graph.h>

#include "buffer.h"
#include "bus.h"
#include "link.h"
#include "net-system.h"
#include "network.h"
#include "node.h"
#include "visual.h"


static struct net_edge_data_t *net_edge_data_create(void)
{
	struct net_edge_data_t *edge_data;
	edge_data = xcalloc(1, sizeof(struct net_edge_data_t));
	return edge_data;
}

static void net_edge_data_free(struct net_edge_data_t *edge_data)
{
	free(edge_data);
}

static struct net_vertex_data_t *net_vertex_data_create(void)
{
	struct net_vertex_data_t *vertex_data;
	vertex_data = xcalloc(1, sizeof (struct net_vertex_data_t));
	return vertex_data;
}

static void net_vertex_data_free (struct net_vertex_data_t *vertex_data)
{
	free(vertex_data);
}

struct net_graph_t *net_graph_create(struct net_t *net)
{
	struct net_graph_t *graph;

	graph = xcalloc(1, sizeof(struct net_graph_t));
	graph->net = net;
	graph->edge_data_list = list_create();
	return graph;
}

void net_graph_free (struct net_graph_t * net_graph)
{
	int i;
	struct graph_t * graph;

	graph = net_graph->graph;

	/* Freeing each vertex associated data */
	for (i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct graph_vertex_t *vertex;
		struct net_vertex_data_t *vertex_data;

		vertex = list_get(graph->vertex_list, i);
		vertex_data = (struct net_vertex_data_t *) vertex->data;
		if (vertex_data)
			net_vertex_data_free(vertex_data);
	}

	/* Freeing associated graph */
	graph_free(graph);

	/* Freeing each edge's associated data */
	for (i = 0; i < list_count(net_graph->edge_data_list); i++)
	{
		struct net_edge_data_t *edge_data;

		edge_data = list_get(net_graph->edge_data_list, i);
		net_edge_data_free(edge_data);
	}
	list_free(net_graph->edge_data_list);

	/* Freeing network graph */
	free(net_graph);
}

void net_graph_populate(struct net_t *net, struct net_graph_t *net_graph)
{
	int i;
	struct graph_t *graph;

	graph = graph_create(net->node_count);

	/* Creating the initial vertices */
	for (i = 0; i < list_count(net->node_list); i++)
	{
		struct net_node_t *node;
		struct graph_vertex_t *vertex;
		struct net_vertex_data_t *vertex_data;

		vertex_data = net_vertex_data_create();

		/* Vertex Index */
		node = list_get(net->node_list, i);
		vertex = list_get(graph->vertex_list, i);
		assert(vertex);
		vertex->index = i;

		/* Vertex Name */
		char name[MAX_STRING_SIZE];
		snprintf(name, sizeof(name), "%s", node->name);
		vertex->name = xstrdup(name);

		/* Vertex Data */
		if (node->kind == net_node_switch )
		{
			vertex_data->kind = net_vertex_switch;
			vertex->kind = graph_vertex_node;
		}
		else if (node->kind == net_node_end)
		{
			vertex_data->kind = net_vertex_end_node;
			vertex->kind = graph_vertex_node;
		}
		else if (node->kind == net_node_bus || node->kind == net_node_photonic)
		{
			vertex_data->kind = net_vertex_bus;
			vertex->kind = graph_vertex_node;
		}
		else
			vertex->kind = graph_vertex_dummy;

		vertex_data->node = node;
		vertex->data = vertex_data;

	}

	/* Creating initial Edges from Links */
	for(i = 0; i < list_count(net->link_list); i++)
	{
		int check = 0;
		struct net_link_t *link;
		struct graph_edge_t *edge;
		struct net_edge_data_t *edge_data;

		link = list_get(net->link_list, i);

		edge = graph_edge_create();
		edge_data = net_edge_data_create();

		edge_data->downstream = link;
		edge_data->kind = net_edge_link;
		edge->data = edge_data;

		edge->src_vertex = list_get(graph->vertex_list, link->src_node->index);
		edge->dst_vertex = list_get(graph->vertex_list, link->dst_node->index);

		assert(edge->src_vertex);
		assert(edge->dst_vertex);

		/* Check for Other Direction: If the edge other direction
		 * already exists, discard this edge and update the
		 * other edge. */
		for (int j = 0; j < list_count(graph->edge_list); j++)
		{
			struct graph_edge_t *temp_edge;
			temp_edge = list_get(graph->edge_list, j);

			struct net_edge_data_t *temp_edge_data;
			temp_edge_data = (struct net_edge_data_t *) temp_edge->data;

			if ((temp_edge->src_vertex == edge->dst_vertex) &&
					(temp_edge->dst_vertex == edge->src_vertex))
			{
				temp_edge_data->upstream = edge_data->downstream;
				temp_edge_data->kind = net_edge_bilink;
				check = 1;
			}

		}

		if (check == 0)
		{
			list_add(graph->edge_list, edge);
			list_add(net_graph->edge_data_list, edge_data);
			list_add(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
			edge->src_vertex->outdeg++;
			list_add(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
			edge->dst_vertex->indeg++;
		}
		else
		{
			struct net_edge_data_t * edge_data = edge->data;
			if (edge_data)
				net_edge_data_free(edge_data);
			graph_edge_free(edge);
		}
	}
	/* Creating edges based on bus connections to the BUS Node */
	for (i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct graph_vertex_t *vertex;
		vertex = list_get(graph->vertex_list, i);

		struct net_vertex_data_t *vertex_data;
		vertex_data = (struct net_vertex_data_t *) vertex->data;
		assert(vertex_data);

		if (vertex_data->kind == net_vertex_bus)
		{
			struct net_node_t *bus_node;
			bus_node = vertex_data->node;

			/* Creating extra edges for BUS connections */
			for (int j = 0; j < list_count(bus_node->src_buffer_list); j++ )
			{
				struct net_buffer_t *src_buffer;
				struct graph_vertex_t *src_vertex;

				src_buffer = list_get(bus_node->src_buffer_list, j);
				src_vertex = list_get(graph->vertex_list, src_buffer->node->index);
				assert(src_vertex);

				struct graph_edge_t *edge;
				edge = graph_edge_create();

				struct net_edge_data_t *edge_data;
				edge_data = net_edge_data_create();

				edge_data->bus_vertex = vertex;
				edge_data->kind = net_edge_bus;
				edge->src_vertex = src_vertex;
				edge->dst_vertex = vertex;

				list_add(graph->edge_list, edge);
				list_add(net_graph->edge_data_list, edge_data);
				list_add(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
				edge->src_vertex->outdeg++;
				list_add(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
				edge->dst_vertex->indeg++;

				edge->data = edge_data;
			}

			for (int j = 0; j < list_count(bus_node->dst_buffer_list); j++ )
			{
				int check = 0;
				struct net_buffer_t *dst_buffer;
				struct graph_vertex_t * dst_vertex;

				dst_buffer = list_get(bus_node->dst_buffer_list, j);
				dst_vertex = list_get(graph->vertex_list, dst_buffer->node->index);
				assert (dst_vertex);

				struct graph_edge_t * edge;
				edge = graph_edge_create();

				struct net_edge_data_t *edge_data;
				edge_data = net_edge_data_create();

				edge_data->bus_vertex = vertex;
				edge_data->kind = net_edge_bus;
				edge->src_vertex = vertex;
				edge->dst_vertex = dst_vertex;
				edge->data = edge_data;

				/* Again, Combining two edges of different directions */
				for (int l = 0; l < list_count(graph->edge_list); l++)
				{
					struct graph_edge_t *temp_edge;
					temp_edge = list_get(graph->edge_list, l);

					struct net_edge_data_t *temp_edge_data;
					temp_edge_data = (struct net_edge_data_t *) temp_edge->data;

					if ((temp_edge->src_vertex == dst_vertex) &&
							(temp_edge->dst_vertex == vertex))
					{
						temp_edge_data->kind = net_edge_bibus;
						check = 1;
						break;
					}
				}

				if (check == 0)
				{
					list_add(graph->edge_list, edge);
					list_add(net_graph->edge_data_list, edge_data);
					list_add(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
					edge->src_vertex->outdeg++;
					list_add(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
					edge->dst_vertex->indeg++;
				}
				else
				{
					struct net_edge_data_t *edge_data = edge->data;
					if (edge_data)
						net_edge_data_free(edge_data);
					graph_edge_free(edge);
				}
			}
		}
	}

	net_graph->graph = graph;
}

struct net_graph_t * net_graph_visual_calculation (struct net_t *net)
{
	int scale = 1;

	/* Create a net graph and populate it with a graph */
	struct net_graph_t *net_graph = net_graph_create(net);

	net_graph_populate(net, net_graph);
	assert(net_graph->graph);

	struct graph_t *graph = net_graph->graph;

	/* Apply drawing algorithm on graph of net_graph */
	graph_layered_drawing(graph);


	scale = list_count(graph->vertex_list);

	struct graph_vertex_t *vertex;
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		vertex = list_get(graph->vertex_list, i);

		int opt_distance= graph->max_vertex_in_layer * scale /(2* graph->max_vertex_in_layer - 1);
		vertex->x_coor = (2*(vertex->x_coor) + (graph->max_vertex_in_layer - vertex->neighbours))* opt_distance;
		net_graph->scale = scale ;
	}


	return net_graph;
}

void net_dump_visual(struct net_graph_t *net_graph, FILE *f)
{
	int i;
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	fprintf(f, "Legend = True \n");
	fprintf(f, "Title = \" Network: %s \"\n", net_graph->net->name);

	struct graph_t *graph = net_graph->graph;
	for (i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct graph_vertex_t *vertex;
		struct net_vertex_data_t *vertex_data;

		vertex = list_get(graph->vertex_list, i);
		vertex_data = (struct net_vertex_data_t *) vertex->data;

		if (vertex->kind == graph_vertex_node)
		{
			if (vertex_data->kind != net_vertex_bus)
				fprintf(f, "node = %s %d %f %d \n", vertex->name,
						vertex_data->kind,
						(double) vertex->x_coor / net_graph->scale,
						vertex->y_coor);
			else
			{
				assert(vertex_data->kind == net_vertex_bus);
				assert(vertex_data->node->kind == net_node_bus);
				for (int j = 0; j < list_count(vertex_data->node->bus_lane_list); j++)
				{
					struct net_bus_t *bus_lane;

					bus_lane = list_get(vertex_data->node->bus_lane_list, j);
					vertex_data->bus_util_color +=(int) ((cycle ?
							(double) bus_lane->transferred_bytes /
							(cycle * bus_lane->bandwidth) : 0.0) * 10);
				}
				vertex_data->bus_util_color /= list_count(vertex_data->node->bus_lane_list);
				fprintf(f, "node = %s %d %f %d %d\n", vertex->name, vertex_data->kind,
				                (double) vertex->x_coor / net_graph->scale,
						vertex->y_coor, vertex_data->bus_util_color);
			}
		}
		else if (vertex->kind == graph_vertex_dummy)
		{
			fprintf(f, "node = %s 3 %f %d \n", vertex->name,
					(double) vertex->x_coor / net_graph->scale,
					vertex->y_coor);
		}

	}
	for (i = 0; i < list_count(graph->edge_list); i++)
	{
		struct graph_edge_t *edge;
		struct net_edge_data_t *edge_data;

		edge = list_get(graph->edge_list, i);

		edge_data = (struct net_edge_data_t *) edge->data;

		if (edge_data->kind == net_edge_link)
			fprintf(f, "link = %f %d %f %d %d %d \n",
					(double) edge->src_vertex->x_coor /
					net_graph->scale, edge->src_vertex->y_coor,
					(double) edge->dst_vertex->x_coor /
					net_graph->scale, edge->dst_vertex->y_coor,
					(int) ((cycle ? (double) edge_data->downstream->
							transferred_bytes / (cycle *
									edge_data->downstream->
									bandwidth) : 0.0) *
							10), 1);
		else if (edge_data->kind == net_edge_bilink)
		{
			assert(edge_data->upstream);
			fprintf(f, "link = %f %d %f %d %d %d \n",
					(double) edge->src_vertex->x_coor /
					net_graph->scale, edge->src_vertex->y_coor,
					(double) edge->dst_vertex->x_coor /
					net_graph->scale, edge->dst_vertex->y_coor,
					(int) ((cycle ? (double) edge_data->downstream->
							transferred_bytes / (cycle *
									edge_data->downstream->
									bandwidth) : 0.0) *
							10), 2);
			fprintf(f, "link = %f %d %f %d %d %d \n",
					(double) edge->dst_vertex->x_coor /
					net_graph->scale, edge->dst_vertex->y_coor,
					(double) edge->src_vertex->x_coor /
					net_graph->scale, edge->src_vertex->y_coor,
					(int) ((cycle ? (double) edge_data->upstream->
							transferred_bytes / (cycle *
									edge_data->upstream->
									bandwidth) : 0.0) *
							10), 2);
		}
		else if (edge_data->kind == net_edge_bus)
		{
			struct net_vertex_data_t *vertex_data;
			assert(edge_data->bus_vertex);
			vertex_data = (struct net_vertex_data_t *) edge_data->bus_vertex->data;

			fprintf(f, "link = %f %d %f %d %d %d \n",
					(double) edge->src_vertex->x_coor /
					net_graph->scale, edge->src_vertex->y_coor,
					(double) edge->dst_vertex->x_coor /
					net_graph->scale, edge->dst_vertex->y_coor,
					vertex_data->bus_util_color, 1);
		}
		else if (edge_data->kind == net_edge_bibus)
		{
			struct net_vertex_data_t *vertex_data;
			assert(edge_data->bus_vertex);
			vertex_data = (struct net_vertex_data_t *) edge_data->bus_vertex->data;

			fprintf(f, "link = %f %d %f %d %d %d \n",
					(double) edge->src_vertex->x_coor /
					net_graph->scale, edge->src_vertex->y_coor,
					(double) edge->dst_vertex->x_coor /
					net_graph->scale, edge->dst_vertex->y_coor,
					vertex_data->bus_util_color, 2);
			fprintf(f, "link = %f %d %f %d %d %d \n",
					(double) edge->dst_vertex->x_coor /
					net_graph->scale, edge->dst_vertex->y_coor,
					(double) edge->src_vertex->x_coor /
					net_graph->scale, edge->src_vertex->y_coor,
					vertex_data->bus_util_color, 2);
		}

	}

}


