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

#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "network.h"
#include "node.h"
#include "visual.h"
#include "link.h"
#include "buffer.h"

static int key_compare (const void *ptr1, const void *ptr2)
{
	struct net_graph_vertex_t *vertex1 = (struct net_graph_vertex_t *) ptr1;
	struct net_graph_vertex_t *vertex2 = (struct net_graph_vertex_t *) ptr2;

	int deg_1 = vertex1->key_val;
	int deg_2 = vertex2->key_val;

	if (deg_1 < deg_2)
		return 1;
	else if (deg_1 == deg_2)
		return 0;
	else
		return -1;
}

static int xcor_compare (const void *ptr1, const void *ptr2)
{
	struct net_graph_vertex_t *vertex1 = (struct net_graph_vertex_t *) ptr1;
	struct net_graph_vertex_t *vertex2 = (struct net_graph_vertex_t *) ptr2;

	float deg_1 = vertex1->cross_num;
	float deg_2 = vertex2->cross_num;

	if (deg_1 < deg_2)
		return 1;
	else if (deg_1 == deg_2)
		return 0;
	else
		return -1;
}

/* For calculating the Visual info */
struct net_graph_t *net_visual_calc(struct net_t *net)
{
	/*Step 1: Create the graph from the network */
	int i;
	struct net_graph_t *graph;

	graph = net_graph_create(net);

	// Creating the vertexes
	for (i = 0; i < list_count(net->node_list); i++)
	{
		struct net_node_t *node;
		struct net_graph_vertex_t *vertex;
		char name[MAX_STRING_SIZE];
		node = list_get(net->node_list, i);
		snprintf(name, sizeof(name), "%s", node->name);
		vertex = net_graph_vertex_create(graph, name);
		vertex->node = node;

		if (node->kind == net_node_switch )
			vertex->kind = net_vertex_switch;
		else if (node->kind == net_node_end)
			vertex->kind = net_vertex_end_node;
		else if (node->kind == net_node_bus)
			vertex->kind = net_vertex_bus;
		else
			vertex->kind = net_vertex_dummy;

		list_add(graph->vertex_list, vertex);
		graph->vertex_count++;
	}

	/* creating the edges based on link list of network */
	for(i = 0; i < list_count(net->link_list); i++)
	{
		int check = 0;
		struct net_link_t *link;
		struct net_graph_edge_t *edge;

		link = list_get(net->link_list, i);

		edge = net_graph_edge_create(graph);
		edge->downstream = link;
		edge->kind = net_edge_link;
		for (int j = 0; j < list_count(graph->vertex_list); j++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, j);
			if (link->src_node == vertex->node )
				edge->src_vertex = vertex;
			else if (link->dst_node == vertex->node)
				edge->dst_vertex = vertex;
		}
		assert(edge->src_vertex);
		assert(edge->dst_vertex);

		// Check for directions
		for (int j = 0; j < list_count(graph->edge_list); j++)
		{
			struct net_graph_edge_t *temp_edge;
			temp_edge = list_get(graph->edge_list, j);
			if ((temp_edge->src_vertex == edge->dst_vertex) &&
					(temp_edge->dst_vertex == edge->src_vertex))
			{
				temp_edge->upstream = edge->downstream;
				temp_edge->kind = net_edge_bilink;
				check = 1;
			}

		}

		if (check == 0)
		{
			list_add(graph->edge_list, edge);
			list_add(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
			edge->src_vertex->outdeg++;
			list_add(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
			edge->dst_vertex->indeg++;
		}
		else
			net_graph_edge_free(edge);
	}

	/* Now that we have all the vertices (based on nodes) and
	 *all the edges based on links we have to create edges
	 *based on the bus connections to the Node(vertex) BUS*/
	for (i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct net_graph_vertex_t *vertex;
		vertex = list_get(graph->vertex_list, i);
		if (vertex->kind == net_vertex_bus)
		{
			struct net_node_t * bus_node;
			bus_node = vertex->node;
			for (int j = 0; j < list_count(bus_node->src_buffer_list); j++ )
			{
				struct net_buffer_t *src_buffer;
				struct net_graph_vertex_t * src_vertex;

				src_buffer = list_get(bus_node->src_buffer_list, j);
				src_vertex = net_get_vertex_by_node(graph, src_buffer->node);
				assert(src_vertex);

				struct net_graph_edge_t * edge;
				edge = net_graph_edge_create(graph);
				edge->bus_vertex = vertex;
				edge->kind = net_edge_bus;
				edge->src_vertex = src_vertex;
				edge->dst_vertex = vertex;

				list_add(graph->edge_list, edge);
				list_add(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
				edge->src_vertex->outdeg++;
				list_add(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
				edge->dst_vertex->indeg++;
			}

			for (int j = 0; j < list_count(bus_node->dst_buffer_list); j++ )
			{
				int check = 0;
				struct net_buffer_t *dst_buffer;
				struct net_graph_vertex_t * dst_vertex;

				dst_buffer = list_get(bus_node->dst_buffer_list, j);
				dst_vertex = net_get_vertex_by_node(graph, dst_buffer->node);
				assert (dst_vertex);
				struct net_graph_edge_t * edge;
				edge = net_graph_edge_create(graph);
				edge->bus_vertex = vertex;
				edge->kind = net_edge_bus;
				edge->src_vertex = vertex;
				edge->dst_vertex = dst_vertex;

				for (int l = 0; l < list_count(graph->edge_list); l++)
				{
					struct net_graph_edge_t *temp_edge;
					temp_edge = list_get(graph->edge_list, l);
					if ((temp_edge->src_vertex == dst_vertex) &&
							(temp_edge->dst_vertex == vertex))
					{
						temp_edge->kind = net_edge_bibus;
						check = 1;
						break;
					}
				}

				if (check == 0)
				{
					list_add(graph->edge_list, edge);
					list_add(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
					edge->src_vertex->outdeg++;
					list_add(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
					edge->dst_vertex->indeg++;
				}
				else
					net_graph_edge_free(edge);
			}



		}
	}


	/*IF Hierarchical method is used */
	/*Step 2: Remove the cycle in the graph ; make sure it is connected ;*/
	net_graph_greedy_cycle_removal (graph);

	/*Step 3: Layer Assignment or determining the y-coordinate */
	int width;
	width = list_count(graph->vertex_list)/2 + 1; //choosing a width for graph.

	net_graph_coffman_graham_layering(graph, width);

	/*Step 4: Dummy node creation and edge addition*/

	for (int i = 0 ; i < list_count(graph->edge_list); i++)
	{
		struct net_graph_edge_t *edge;
		edge = list_get(graph->edge_list, i);

		if ((edge->src_vertex->y_coor - edge->dst_vertex->y_coor > 1))
		{
			//creating one dummy node

			struct net_graph_vertex_t *vertex;

			vertex = net_graph_vertex_create(graph, "dummy");
			vertex->kind = net_vertex_dummy;
			vertex->y_coor = edge->src_vertex->y_coor - 1;
			vertex->key_val = 0;
			list_add(graph->vertex_list, vertex);
			graph->vertex_count++;
			list_add(vertex->outgoint_vertex_list, edge->dst_vertex);
			vertex->outdeg++;
			list_add(vertex->incoming_vertex_list, edge->src_vertex);
			vertex->indeg++;

			/* creating two edges. First Edge is called new_edge and
			 * second edge is ...well...called second edge*/
			struct net_graph_edge_t *new_edge;
			struct net_graph_edge_t *second_edge;

			new_edge = net_graph_edge_create(graph);
			second_edge = net_graph_edge_create(graph);

			if (edge->kind == net_edge_link)
			{
				new_edge->downstream = edge->downstream;
				second_edge->downstream = new_edge->downstream;
			}
			else if (edge->kind == net_edge_bilink)
			{
				assert(edge->upstream);
				new_edge->downstream = edge->downstream;
				second_edge->downstream = new_edge->downstream;

				new_edge->upstream = edge->upstream;
				second_edge->upstream = edge->upstream;
			}
			else
			{
				assert(!edge->downstream);
				new_edge->bus_vertex = edge->bus_vertex;
				second_edge->bus_vertex = edge->bus_vertex;
			}

			new_edge->kind = edge->kind;
			second_edge->kind = edge->kind;

			new_edge->src_vertex = edge->src_vertex;
			new_edge->dst_vertex = vertex;

			second_edge->src_vertex = vertex;
			second_edge->dst_vertex = edge->dst_vertex;

			//changes in corresponding vertices
			list_remove(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
			list_add(edge->src_vertex->outgoint_vertex_list, vertex);
			list_remove(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
			list_add(edge->dst_vertex->incoming_vertex_list, vertex);

			list_enqueue(graph->edge_list, new_edge);
			list_enqueue(graph->edge_list, second_edge);
			list_remove(graph->edge_list, edge);
			i--;
			net_graph_edge_free(edge);

		}
	}

	/*Side Step: Initial x-Coordination */

	//number of layers
	int layer_count = 0;
	struct net_graph_vertex_t *vertex;
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		vertex = list_get(graph->vertex_list, i);
		if (vertex->y_coor > layer_count)
		{
			layer_count = vertex->y_coor;
		}
	}

	int max_node_in_layer = 0;
	int scale = 1;
	for (int i = 0; i < layer_count +1 ; i++)
	{
		int count = 0;
		for (int j = 0; j <list_count(graph->vertex_list); j++)
		{
			vertex = list_get(graph->vertex_list, j);
			if (vertex->y_coor == i)
			{
				vertex->x_coor = count;
				count++;
			}
		}
		if (max_node_in_layer < count)
			max_node_in_layer = count;
		for (int j = 0; j <list_count(graph->vertex_list); j++)
		{
			vertex = list_get(graph->vertex_list, j);
			if (vertex->y_coor == i)
				vertex->neighbours= count;
		}
	}

	scale = list_count(graph->vertex_list);


	/*Step 5: Cross Reduction */
	net_graph_cross_reduction(graph, layer_count);
	/*Step 5: Horizontal coordinate assignment or x-coordinate */
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		vertex = list_get(graph->vertex_list, i);

		int opt_distance= max_node_in_layer * scale /(2*max_node_in_layer - 1);
		vertex->x_coor = (2*(vertex->x_coor) + (max_node_in_layer - vertex->neighbours))* opt_distance;
		graph->xscale = scale ;
	}
	/*Step 6: remove the Graph */
	return graph;
}

struct net_graph_t *net_graph_create(struct net_t *net)
{
	struct net_graph_t *graph;

	graph = xcalloc(1, sizeof(struct net_graph_t));
	graph->net = net;
	graph->edge_list = list_create_with_size(4);
	graph->vertex_list = list_create_with_size(4);

	return graph;
}

void net_graph_free (struct net_graph_t * graph)
{
	int i;

	/* Free vertices */
	for (i = 0; i < list_count(graph->vertex_list); i++)
		net_graph_vertex_free(list_get(graph->vertex_list, i));
	list_free(graph->vertex_list);

	/* Free edges */
	for (i = 0; i < list_count(graph->edge_list); i++)
		net_graph_edge_free(list_get(graph->edge_list, i));
	list_free(graph->edge_list);

	/* Graph */
	free(graph);
}

struct net_graph_vertex_t *net_graph_vertex_create(struct net_graph_t *graph, char *name)
{
	struct net_graph_vertex_t *vertex;
	vertex = xcalloc(1, sizeof(struct net_graph_vertex_t));
	vertex->name = xstrdup(name);

	vertex->graph = graph;
	vertex->incoming_vertex_list = list_create();
	vertex->outgoint_vertex_list = list_create();
	return vertex;
}
void net_graph_vertex_free (struct net_graph_vertex_t *vertex)
{
	list_free(vertex->incoming_vertex_list);
	list_free(vertex->outgoint_vertex_list);
	free(vertex->name);
	free(vertex);
}

struct net_graph_edge_t *net_graph_edge_create(struct net_graph_t *graph)
{
	struct net_graph_edge_t *edge;
	edge = xcalloc(1, sizeof (struct net_graph_edge_t));
	edge->graph = graph;
	return edge;
}
void net_graph_edge_free (struct net_graph_edge_t *edge)
{
	free(edge);
}

void net_graph_greedy_cycle_removal (struct net_graph_t *graph)
{

	int end = list_count(graph->vertex_list);
	int start = 0;
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct net_graph_vertex_t *vertex;
		vertex = list_get(graph->vertex_list, i);
		if (vertex->outdeg == 0)
		{
			vertex->x_coor = end;
			end--;
		}
		else if (vertex->indeg == 0)
		{
			vertex->x_coor = start;
			start++;
		}
		vertex->key_val = vertex->outdeg - vertex->indeg;
	}
	list_sort(graph->vertex_list, key_compare);
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct net_graph_vertex_t *vertex;
		vertex = list_get(graph->vertex_list, i);
		if (vertex->x_coor == 0)
		{
			vertex->x_coor = start;
			start ++;
		}
	}

	for (int i = 0; i < list_count(graph->edge_list); i++)
	{
		struct net_graph_edge_t *edge;
		edge = list_get(graph->edge_list, i);
		if (edge->dst_vertex->x_coor < edge->src_vertex->x_coor)
		{
			list_remove(edge->src_vertex->outgoint_vertex_list, edge->dst_vertex);
			list_add(edge->src_vertex->incoming_vertex_list, edge->dst_vertex);
			edge->src_vertex->indeg++;
			edge->src_vertex->outdeg--;
			list_remove(edge->dst_vertex->incoming_vertex_list, edge->src_vertex);
			list_add(edge->dst_vertex->outgoint_vertex_list, edge->src_vertex);
			edge->dst_vertex->outdeg++;
			edge->dst_vertex->indeg--;
			struct net_graph_vertex_t *temp;
			temp = edge->dst_vertex;
			edge->dst_vertex = edge->src_vertex;
			edge->src_vertex = temp;

		}
	}
}

void net_graph_label_assignment(struct net_graph_t *graph)
{
	int label = 1;
	while (label <= graph->vertex_count)
	{
		for (int i = 0; i < list_count(graph->vertex_list); i++)
		{
			struct net_graph_vertex_t *vertexA;
			vertexA = list_get(graph->vertex_list, i);
			if ((list_count(vertexA->incoming_vertex_list) == 0))
			{
				if (vertexA->key_val == 0)
				{
					vertexA->key_val = label;
					label++;
				}
			}
		}

		struct net_graph_vertex_t *candidate;
		struct net_graph_vertex_t *vertex;
		struct list_t *candidate_list;
		candidate_list = list_create();
		for (int i = 0; i < list_count(graph->vertex_list); i++)
		{
			vertex = list_get(graph->vertex_list, i);
			if (vertex->key_val == 0)
			{
				list_sort(vertex->incoming_vertex_list, key_compare);
				int checker = 0;
				for (int j = 0; j < list_count(vertex->incoming_vertex_list); j++)
				{
					struct net_graph_vertex_t *inc_vertex;
					inc_vertex = list_get(vertex->incoming_vertex_list, j);
					if (inc_vertex->key_val == 0)
						checker = 1;
				}

				if (checker == 0)
				{
					list_add(candidate_list, vertex);
				}
			}
		}
		//choosing candidate

		if (list_count(candidate_list) != 0)
		{
			candidate = list_get(candidate_list, 0);
			assert(candidate);
			for (int i = 1; i < list_count(candidate_list); i++)
			{
				vertex = list_get(candidate_list, i);
				assert(vertex);
				int min_num = ((candidate->indeg > vertex->indeg)? vertex->indeg : candidate->indeg);
				int resolved =0;
				int i_index = 1;
				//comparison
				while ((resolved == 0) && (i_index <= min_num))
				{
					struct net_graph_vertex_t *vertex1, *vertex2;
					vertex1 = list_get(candidate->incoming_vertex_list, candidate->indeg - i_index);
					assert(vertex1);
					vertex2 = list_get(vertex->incoming_vertex_list, vertex->indeg - i_index);
					assert(vertex2);
					if (vertex1 == vertex2)
						i_index++;
					else if (vertex1->key_val > vertex2->key_val)
					{
						candidate = vertex;
						resolved = 1;
					}
					else if (vertex1->key_val < vertex2->key_val)
						resolved = 1;
				}
				if (resolved == 0)
				{
					if (candidate->indeg > vertex->indeg)
						candidate = vertex;
				}
			}
		}
		candidate->key_val = label;
		label++;
		list_free(candidate_list);
	}

}

void net_graph_coffman_graham_layering (struct net_graph_t *graph, int width)
{
	// step 1: initialization
	int k = 0;
	struct list_t *U, *V;
	U = list_create();
	V = list_create();
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct net_graph_vertex_t *vertex;
		vertex = list_get(graph->vertex_list, i);
		vertex->x_coor = 0;
		vertex->y_coor = -1 ;
		vertex->key_val = 0;
		list_add(V, vertex);
	}

	// step 2: label assignment
	net_graph_label_assignment(graph);

	// step 3: layer assignment
	while (list_count(V) != 0)
	{
		struct net_graph_vertex_t * candidate;
		list_sort(V, key_compare);
		int count = list_count(V);

		for (int i = 0; i < count; i++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(V, i);
			if (vertex->outdeg == 0)
			{
				candidate = vertex;
			}
			else
			{
				int check_count = 0;
				for (int j = 0; j < list_count(vertex->outgoint_vertex_list); j++)
				{
					struct net_graph_vertex_t *out_vertex;
					out_vertex = list_get(vertex->outgoint_vertex_list, j);
					for (int num = 0; num < list_count(U); num++)
					{
						struct net_graph_vertex_t *U_elem;
						U_elem= list_get(U, num);
						if (out_vertex == U_elem)
							check_count ++;
					}
				}
				if (check_count == list_count(vertex->outgoint_vertex_list))
					candidate = vertex;
			}

			int layer_count = 0;
			for (int j = 0; j < list_count(candidate->outgoint_vertex_list); j++)
			{
				struct net_graph_vertex_t *temp_vertex;
				temp_vertex = list_get(candidate->outgoint_vertex_list, j);
				if (temp_vertex->y_coor >= k)
					k = temp_vertex->y_coor + 1;
			}

			for (int j =0; j < list_count(graph->vertex_list); j++ )
			{
				struct net_graph_vertex_t *temp_vertex;
				temp_vertex = list_get(graph->vertex_list, j);
				if (temp_vertex->y_coor == k)
				{
					layer_count++;
				}
			}
			if ((layer_count < width))
			{
				candidate->y_coor = k;
			}
			else
			{
				k++;
				candidate->y_coor = k;
			}

			list_add(U, candidate);
			list_remove_at(V, i);
			i--;
			count --;
		}
	}
	list_free(U);
	list_free(V);

}

void net_graph_cross_reduction(struct net_graph_t *graph, int layer_count)
{
	for (int x = layer_count; x >= 0 ; x--)
	{
		for (int i =0; i < list_count(graph->vertex_list); i++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);

			if (vertex->y_coor == x)
			{
				vertex->cross_num= 0;
				for (int j = 0; j < list_count(vertex->incoming_vertex_list); j++)
				{
					struct net_graph_vertex_t *neighbor;
					neighbor = list_get(vertex->incoming_vertex_list, j);
					if (neighbor->y_coor == vertex->y_coor + 1)
					{
						vertex->cross_num+= (float)neighbor->x_coor;
					}
				}
				if (list_count(vertex->incoming_vertex_list) != 0)
					vertex->cross_num= vertex->cross_num/ list_count(vertex->incoming_vertex_list);
				else
					vertex->cross_num= (float) vertex->x_coor;
			}
		}
		struct list_t *key_list;
		key_list = list_create();
		for (int i = 0; i < list_count(graph->vertex_list); i++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);
			if (vertex->y_coor == x)
			{
				list_add(key_list, vertex);
			}
		}
		list_sort(key_list, xcor_compare);
		for(int i = list_count(key_list) - 1; i >= 0; i--)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(key_list, i);
			vertex->x_coor = list_count(key_list) - 1 - i;
		}
		list_free(key_list);
	}

	for (int x = 1; x < layer_count+1; x++)
	{
		for (int i =0; i < list_count(graph->vertex_list); i++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);

			if (vertex->y_coor == x)
			{
				vertex->cross_num = 0;
				for (int j = 0; j < list_count(vertex->outgoint_vertex_list); j++)
				{
					struct net_graph_vertex_t *neighbor;
					neighbor = list_get(vertex->outgoint_vertex_list, j);

					vertex->cross_num += (float) neighbor->x_coor;

				}
				if (list_count(vertex->outgoint_vertex_list))
					vertex->cross_num= vertex->cross_num / list_count(vertex->outgoint_vertex_list);
				else
					vertex->cross_num= (float) vertex->x_coor;
			}
		}
		struct list_t *key_list;
		key_list = list_create();
		for (int i = 0; i < list_count(graph->vertex_list); i++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);
			if (vertex->y_coor == x)
			{
				list_add(key_list, vertex);
			}
		}
		list_sort(key_list, xcor_compare);
		for(int i = list_count(key_list) - 1; i >= 0; i--)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(key_list, i);
			vertex->x_coor = list_count(key_list) - 1 - i;
		}
		list_free(key_list);
	}

	for (int x = layer_count; x >= 0 ; x--)
	{
		for (int i =0; i < list_count(graph->vertex_list); i++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);

			if (vertex->y_coor == x)
			{
				vertex->cross_num= 0;
				for (int j = 0; j < list_count(vertex->incoming_vertex_list); j++)
				{
					struct net_graph_vertex_t *neighbor;
					neighbor = list_get(vertex->incoming_vertex_list, j);

					vertex->cross_num+= (float)neighbor->x_coor;
				}
				if (list_count(vertex->incoming_vertex_list) != 0)
					vertex->cross_num= vertex->cross_num/ list_count(vertex->incoming_vertex_list);
				else
					vertex->cross_num= (float) vertex->x_coor;
			}
		}
		struct list_t *key_list;
		key_list = list_create();
		for (int i = 0; i < list_count(graph->vertex_list); i++)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);
			if (vertex->y_coor == x)
			{
				list_add(key_list, vertex);
			}
		}
		list_sort(key_list, xcor_compare);
		for(int i = list_count(key_list) - 1; i >= 0; i--)
		{
			struct net_graph_vertex_t *vertex;
			vertex = list_get(key_list, i);
			vertex->x_coor = list_count(key_list) - 1 - i;
		}
		list_free(key_list);
	}

}

struct net_graph_vertex_t *net_get_vertex_by_node(struct net_graph_t * graph,
		struct net_node_t *node)
{
	assert(graph->net == node->net);
	struct net_graph_vertex_t * vertex;

	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		vertex = list_get(graph->vertex_list, i);
		if (vertex->node == node)
			return vertex;
	}
	return NULL;
}
