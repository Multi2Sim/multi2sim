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
#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>

#include "graph.h"

struct graph_t *graph_create(int init_vertex_count)
{
	int i;
	struct graph_t *graph;

	graph = xcalloc(1, sizeof(struct graph_t));
	graph->vertex_count = init_vertex_count;

	graph->vertex_list = list_create();

	for (i = 0; i < graph->vertex_count; i++)
		list_add(graph->vertex_list, graph_vertex_create());

	graph->edge_list = list_create();

	return graph;
}

void graph_free(struct graph_t * graph)
{
	int i;

	/* Free edges */
	for (i = 0; i < list_count(graph->edge_list); i++)
		graph_edge_free(list_get(graph->edge_list, i));
	list_free(graph->edge_list);

	/* Free Vertices */
	for (i = 0; i < list_count(graph->vertex_list); i++)
		graph_vertex_free(list_get(graph->vertex_list, i));
	list_free(graph->vertex_list);

	/* Graph */
	free(graph);
}

struct graph_vertex_t *graph_vertex_create (void)
{
	struct graph_vertex_t *vertex;
	vertex = xcalloc(1, sizeof(struct graph_vertex_t));

	vertex->incoming_vertex_list = list_create();
	vertex->outgoint_vertex_list = list_create();

	return vertex;
}

void graph_vertex_free(struct graph_vertex_t *vertex)
{
	/* Make sure there is no data or data is already freed */

	list_free(vertex->incoming_vertex_list);
	list_free(vertex->outgoint_vertex_list);
	free(vertex->name);
	free(vertex);
}

struct graph_edge_t *graph_edge_create(void)
{
	struct graph_edge_t *edge;
	edge = xcalloc(1, sizeof (struct graph_edge_t));
	edge->direction = graph_edge_uni_direction;
	edge->reversed = 0;
	return edge;
}

void graph_edge_free (struct graph_edge_t *edge)
{
	free(edge);
}

static void graph_greedy_cycle_removal    (struct graph_t *graph);
static void graph_coffman_graham_layering (struct graph_t *graph, int width);
static void graph_dummy_node_apply        (struct graph_t *graph);
static int  graph_initial_x               (struct graph_t *graph, int layer_count);
static void graph_cross_reduction         (struct graph_t *graph, int layer_count);
static void graph_label_assignment    (struct graph_t *graph);

void graph_layered_drawing (struct graph_t * graph)
{
	/* Removing Cycles */
	graph_greedy_cycle_removal(graph);

	/* Coffman Graham Layering; The width input can be voluntarily  */
	graph_coffman_graham_layering (graph, (list_count(graph->vertex_list)/2 + 1));

	/* Dummy node creation and application */
	graph_dummy_node_apply(graph);

	int layer_count = 0;

	struct graph_vertex_t *vertex;
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		vertex = list_get(graph->vertex_list, i);
		if (vertex->y_coor > layer_count)
			layer_count = vertex->y_coor;
	}

	/*Side Step: Initial x-Coordination */
	graph->max_vertex_in_layer = graph_initial_x(graph, layer_count);

	/* Cross reduction */
	graph_cross_reduction(graph, layer_count);

}

static int key_compare (const void *ptr1, const void *ptr2)
{
	struct graph_vertex_t *vertex1 = (struct graph_vertex_t *) ptr1;
	struct graph_vertex_t *vertex2 = (struct graph_vertex_t *) ptr2;

	int deg_1 = vertex1->key_val;
	int deg_2 = vertex2->key_val;

	if (deg_1 < deg_2)
		return 1;
	else if (deg_1 == deg_2)
		return 0;
	else
		return -1;
}

static void graph_greedy_cycle_removal (struct graph_t *graph)
{
	int end = list_count(graph->vertex_list);
	int start = 0;
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct graph_vertex_t *vertex;
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
		struct graph_vertex_t *vertex;
		vertex = list_get(graph->vertex_list, i);
		if (vertex->x_coor == 0)
		{
			vertex->x_coor = start;
			start ++;
		}
	}

	for (int i = 0; i < list_count(graph->edge_list); i++)
	{
		struct graph_edge_t *edge;
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
			struct graph_vertex_t *temp;
			temp = edge->dst_vertex;
			edge->dst_vertex = edge->src_vertex;
			edge->src_vertex = temp;
			edge->reversed = 1;

		}
	}
}

static void graph_coffman_graham_layering (struct graph_t *graph, int width)
{
	// step 1: initialization
	int k = 0;
	struct list_t *U, *V;
	U = list_create();
	V = list_create();
	for (int i = 0; i < list_count(graph->vertex_list); i++)
	{
		struct graph_vertex_t *vertex;
		vertex = list_get(graph->vertex_list, i);
		vertex->x_coor = 0;
		vertex->y_coor = -1 ;
		vertex->key_val = 0;
		list_add(V, vertex);
	}

	// step 2: label assignment
	graph_label_assignment(graph);

	// step 3: layer assignment
	while (list_count(V) != 0)
	{
		struct graph_vertex_t * candidate;
		list_sort(V, key_compare);
		int count = list_count(V);

		for (int i = 0; i < count; i++)
		{
			struct graph_vertex_t *vertex;
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
					struct graph_vertex_t *out_vertex;
					out_vertex = list_get(vertex->outgoint_vertex_list, j);
					for (int num = 0; num < list_count(U); num++)
					{
						struct graph_vertex_t *U_elem;
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
				struct graph_vertex_t *temp_vertex;
				temp_vertex = list_get(candidate->outgoint_vertex_list, j);
				if (temp_vertex->y_coor >= k)
					k = temp_vertex->y_coor + 1;
			}

			for (int j =0; j < list_count(graph->vertex_list); j++ )
			{
				struct graph_vertex_t *temp_vertex;
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

static void graph_dummy_node_apply (struct graph_t *graph)
{
	for (int i = 0 ; i < list_count(graph->edge_list); i++)
	{
		struct graph_edge_t *edge;
		edge = list_get(graph->edge_list, i);

		if ((edge->src_vertex->y_coor - edge->dst_vertex->y_coor > 1))
		{
			/* Creating dummy node if there is two layer between nodes */

			struct graph_vertex_t *vertex;

			vertex = graph_vertex_create();
			vertex->name = xstrdup("dummy");
			vertex->kind = graph_vertex_dummy;
			vertex->y_coor = edge->src_vertex->y_coor - 1;
			vertex->key_val = 0;
			list_add(graph->vertex_list, vertex);
			graph->vertex_count++;
			list_add(vertex->outgoint_vertex_list, edge->dst_vertex);
			vertex->outdeg++;
			list_add(vertex->incoming_vertex_list, edge->src_vertex);
			vertex->indeg++;

			/* creating two edges. One from Source to dummy
			 * second one from dummy to destination */
			struct graph_edge_t *new_edge;
			struct graph_edge_t *second_edge;

			new_edge = graph_edge_create();
			second_edge = graph_edge_create();

			new_edge->data = *(&edge->data);
			second_edge->data = *(&edge->data);

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

			/* We intentionally didn't free the data here
			 * If it is NULL no point in freeing.
			 * If not NULL, there is two other edges pointing
			 * to it now.
			 */
			graph_edge_free(edge);

		}
	}
}

static int graph_initial_x (struct graph_t * graph, int layer_count)
{
	int max_vertex_in_layer = 0;
	struct graph_vertex_t *vertex;
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
		if (max_vertex_in_layer < count)
			max_vertex_in_layer = count;
		for (int j = 0; j <list_count(graph->vertex_list); j++)
		{
			vertex = list_get(graph->vertex_list, j);
			if (vertex->y_coor == i)
				vertex->neighbours= count;
		}
	}
	return max_vertex_in_layer;
}


static int xcor_compare (const void *ptr1, const void *ptr2)
{
	struct graph_vertex_t *vertex1 = (struct graph_vertex_t *) ptr1;
	struct graph_vertex_t *vertex2 = (struct graph_vertex_t *) ptr2;

	float deg_1 = vertex1->cross_num;
	float deg_2 = vertex2->cross_num;

	if (deg_1 < deg_2)
		return 1;
	else if (deg_1 == deg_2)
		return 0;
	else
		return -1;
}

static void graph_cross_reduction(struct graph_t *graph, int layer_count)
{
	for (int x = layer_count; x >= 0 ; x--)
	{
		for (int i =0; i < list_count(graph->vertex_list); i++)
		{
			struct graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);

			if (vertex->y_coor == x)
			{
				vertex->cross_num= 0;
				for (int j = 0; j < list_count(vertex->incoming_vertex_list); j++)
				{
					struct graph_vertex_t *neighbor;
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
			struct graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);
			if (vertex->y_coor == x)
			{
				list_add(key_list, vertex);
			}
		}
		list_sort(key_list, xcor_compare);
		for(int i = list_count(key_list) - 1; i >= 0; i--)
		{
			struct graph_vertex_t *vertex;
			vertex = list_get(key_list, i);
			vertex->x_coor = list_count(key_list) - 1 - i;
		}
		list_free(key_list);
	}

	for (int x = 1; x < layer_count+1; x++)
	{
		for (int i =0; i < list_count(graph->vertex_list); i++)
		{
			struct graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);

			if (vertex->y_coor == x)
			{
				vertex->cross_num = 0;
				for (int j = 0; j < list_count(vertex->outgoint_vertex_list); j++)
				{
					struct graph_vertex_t *neighbor;
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
			struct graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);
			if (vertex->y_coor == x)
			{
				list_add(key_list, vertex);
			}
		}
		list_sort(key_list, xcor_compare);

		for(int i = list_count(key_list) - 1; i >= 0; i--)
		{
			struct graph_vertex_t *vertex;
			vertex = list_get(key_list, i);
			vertex->x_coor = list_count(key_list) - 1 - i;
		}
		list_free(key_list);
	}

	for (int x = layer_count; x >= 0 ; x--)
	{
		for (int i =0; i < list_count(graph->vertex_list); i++)
		{
			struct graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);

			if (vertex->y_coor == x)
			{
				vertex->cross_num= 0;
				for (int j = 0; j < list_count(vertex->incoming_vertex_list); j++)
				{
					struct graph_vertex_t *neighbor;
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
			struct graph_vertex_t *vertex;
			vertex = list_get(graph->vertex_list, i);
			if (vertex->y_coor == x)
			{
				list_add(key_list, vertex);
			}
		}
		list_sort(key_list, xcor_compare);
		for(int i = list_count(key_list) - 1; i >= 0; i--)
		{
			struct graph_vertex_t *vertex;
			vertex = list_get(key_list, i);
			vertex->x_coor = list_count(key_list) - 1 - i;
		}
		list_free(key_list);
	}

}

static void graph_label_assignment(struct graph_t *graph)
{
	int label = 1;
	while (label <= graph->vertex_count)
	{
		for (int i = 0; i < list_count(graph->vertex_list); i++)
		{
			struct graph_vertex_t *vertexA;
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

		struct graph_vertex_t *candidate;
		struct graph_vertex_t *vertex;
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
					struct graph_vertex_t *inc_vertex;
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
					struct graph_vertex_t *vertex1, *vertex2;
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
