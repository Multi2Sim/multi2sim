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
#include <assert.h>
#include <stdio.h>


#include <lib/mhandle/mhandle.h>
#include <lib/util/graph.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "buffer.h"
#include "link.h"
#include "net-graph.h"
#include "net.h"
#include "node.h"

struct vi_net_graph_t *vi_net_graph_create (struct vi_net_t *net)
{
        struct vi_net_graph_t *net_graph;

        net_graph = xcalloc(1, sizeof(struct vi_net_graph_t));
        net_graph->net = net;
        net_graph->edge_data_list = list_create();

        return net_graph;

}

static struct vi_net_edge_data_t *vi_net_edge_data_create(void)
{
        struct vi_net_edge_data_t *edge_data;
        edge_data = xcalloc(1, sizeof(struct vi_net_edge_data_t));
        return edge_data;
}

static void vi_net_edge_data_free(struct vi_net_edge_data_t *edge_data)
{
        free(edge_data);
}

static struct vi_net_vertex_data_t *vi_net_vertex_data_create(void)
{
        struct vi_net_vertex_data_t *vertex_data;
        vertex_data = xcalloc(1, sizeof (struct vi_net_vertex_data_t));
        return vertex_data;
}

static void vi_net_vertex_data_free (struct vi_net_vertex_data_t *vertex_data)
{
        free(vertex_data);
}

void vi_net_graph_populate (struct vi_net_t *net, struct vi_net_graph_t *net_graph)
{
        int i;
        struct graph_t *graph;

        graph = graph_create(list_count(net->node_list));

        /* Creating the initial vertices */
        for (i = 0; i < list_count(net->node_list); i++)
        {
                struct vi_net_node_t *node;
                struct graph_vertex_t *vertex;
                struct vi_net_vertex_data_t *vertex_data;

                vertex_data = vi_net_vertex_data_create();

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
                if (node->type == vi_net_node_switch )
                {
                        vertex_data->kind = vi_net_vertex_switch;
                        vertex->kind = graph_vertex_node;
                }
                else if (node->type == vi_net_node_end)
                {
                        vertex_data->kind = vi_net_vertex_end_node;
                        vertex->kind = graph_vertex_node;
                }
                else if (node->type == vi_net_node_bus || node->type == vi_net_node_photonic)
                {
                        vertex_data->kind = vi_net_vertex_bus;
                        vertex->kind = graph_vertex_node;
                }
                else
                        vertex->kind = graph_vertex_dummy;

                vertex_data->node = node;
                vertex->data = vertex_data;

        }

        /* Creating initial Edges from Links */
        char *link_name;
        struct vi_net_link_t *link;

        HASH_TABLE_FOR_EACH(net->link_table, link_name, link)
        {
                int check = 0;
                struct graph_edge_t *edge;
                struct vi_net_edge_data_t *edge_data;

                edge = graph_edge_create();
                edge_data = vi_net_edge_data_create();

                edge_data->downstream = link;
                edge_data->kind = vi_net_edge_link;
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

                        struct vi_net_edge_data_t *temp_edge_data;
                        temp_edge_data = (struct vi_net_edge_data_t *) temp_edge->data;

                        if ((temp_edge->src_vertex == edge->dst_vertex) &&
                                        (temp_edge->dst_vertex == edge->src_vertex))
                        {
                                temp_edge_data->upstream = edge_data->downstream;
                                temp_edge_data->kind = vi_net_edge_bilink;
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
                        struct vi_net_edge_data_t * edge_data = edge->data;
                        if (edge_data)
                                vi_net_edge_data_free(edge_data);
                        graph_edge_free(edge);
                }
        }
        /* Creating edges based on bus connections to the BUS Node */
        for (i = 0; i < list_count(graph->vertex_list); i++)
        {
                struct graph_vertex_t *vertex;
                vertex = list_get(graph->vertex_list, i);

                struct vi_net_vertex_data_t *vertex_data;
                vertex_data = (struct vi_net_vertex_data_t *) vertex->data;
                assert(vertex_data);

                if (vertex_data->kind == vi_net_vertex_bus)
                {
                        struct vi_net_node_t *bus_node;
                        bus_node = vertex_data->node;

                        /* Just added this assertion to make sure it works properly for VI as well */
                        assert(bus_node->type == vi_net_node_bus);

                        /* Creating extra edges for BUS connections */
                        char *src_buffer_name;
                        struct vi_net_buffer_t *src_buffer;
                        HASH_TABLE_FOR_EACH(bus_node->src_buffer_list, src_buffer_name, src_buffer)
                        {
                                struct graph_vertex_t *src_vertex;

                                src_vertex = list_get(graph->vertex_list, src_buffer->node->index);
                                assert(src_vertex);

                                struct graph_edge_t *edge;
                                edge = graph_edge_create();

                                struct vi_net_edge_data_t *edge_data;
                                edge_data = vi_net_edge_data_create();

                                edge_data->bus_vertex = vertex;
                                edge_data->kind = vi_net_edge_bus;
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
                        char *dst_buffer_name;
                        struct vi_net_buffer_t *dst_buffer;
                        HASH_TABLE_FOR_EACH(bus_node->dst_buffer_list, dst_buffer_name, dst_buffer)
                        {
                                int check = 0;
                                struct graph_vertex_t * dst_vertex;

                                dst_vertex = list_get(graph->vertex_list, dst_buffer->node->index);
                                assert (dst_vertex);

                                struct graph_edge_t * edge;
                                edge = graph_edge_create();

                                struct vi_net_edge_data_t *edge_data;
                                edge_data = vi_net_edge_data_create();

                                edge_data->bus_vertex = vertex;
                                edge_data->kind = vi_net_edge_bus;
                                edge->src_vertex = vertex;
                                edge->dst_vertex = dst_vertex;
                                edge->data = edge_data;

                                /* Again, Combining two edges of different directions */
                                for (int l = 0; l < list_count(graph->edge_list); l++)
                                {
                                        struct graph_edge_t *temp_edge;
                                        temp_edge = list_get(graph->edge_list, l);

                                        struct vi_net_edge_data_t *temp_edge_data;
                                        temp_edge_data = (struct vi_net_edge_data_t *) temp_edge->data;

                                        if ((temp_edge->src_vertex == dst_vertex) &&
                                                        (temp_edge->dst_vertex == vertex))
                                        {
                                                temp_edge_data->kind = vi_net_edge_bibus;
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
                                        struct vi_net_edge_data_t *edge_data = edge->data;
                                        if (edge_data)
                                                vi_net_edge_data_free(edge_data);
                                        graph_edge_free(edge);
                                }
                        }
                }
        }
        net_graph->graph = graph;
        assert(net_graph->graph);
}

void vi_net_graph_free (struct vi_net_graph_t * net_graph)
{
        int i;
        struct graph_t * graph;

        graph = net_graph->graph;

        /* Freeing each vertex associated data */
        for (i = 0; i < list_count(graph->vertex_list); i++)
        {
                struct graph_vertex_t *vertex;
                struct vi_net_vertex_data_t *vertex_data;

                vertex = list_get(graph->vertex_list, i);
                vertex_data = (struct vi_net_vertex_data_t *) vertex->data;
                if (vertex_data)
                        vi_net_vertex_data_free(vertex_data);
        }

        /* Freeing associated graph */
        graph_free(graph);

        /* Freeing each edge's associated data */
        for (i = 0; i < list_count(net_graph->edge_data_list); i++)
        {
                struct vi_net_edge_data_t *edge_data;

                edge_data = list_get(net_graph->edge_data_list, i);
                vi_net_edge_data_free(edge_data);
        }
        list_free(net_graph->edge_data_list);

        /* Freeing network graph */
        free(net_graph);
}

//static int gcd(int a, int b) { if (b == 0) { return a; } else { return gcd(b, a % b); } }

void vi_net_graph_draw_scale(struct vi_net_graph_t *net_graph)
{
        int scale;
        struct graph_t *graph = net_graph->graph;

        /* Apply drawing algorithm on graph of net_graph */
        graph_layered_drawing(graph);

        /* Calculate a tailored scale for the vi_graph that depends on the
         * spacing of each node and link
         */

        scale = list_count(graph->vertex_list);

        struct graph_vertex_t *vertex;
        for (int i = 0; i < list_count(graph->vertex_list); i++)
        {
                vertex = list_get(graph->vertex_list, i);

                int opt_distance= graph->max_vertex_in_layer * scale /(2* graph->max_vertex_in_layer - 1);
                vertex->x_coor = (2*(vertex->x_coor) + (graph->max_vertex_in_layer - vertex->neighbours))* opt_distance;
        }
        net_graph->scale = scale ;
}

void vi_net_graph_finalize(struct vi_net_graph_t *net_graph)
{
        int i;
        struct graph_t * graph = net_graph->graph;
        struct vi_net_t *net = net_graph->net;

        /* Updating Nodes */
        for (i = 0 ; i < list_count(graph->vertex_list); i++)
        {
                struct graph_vertex_t *vertex;
                vertex = list_get(graph->vertex_list, i);

                if (vertex->kind == graph_vertex_node)
                {
                        struct vi_net_vertex_data_t *vertex_data = vertex->data;
                        struct vi_net_node_t *node;

                        node = vertex_data->node;
                        node->X = (double) vertex->x_coor / net_graph->scale;
                        node->Y = vertex->y_coor;

                }
                if (vertex->kind == graph_vertex_dummy)
                {
                        struct vi_net_node_t *node;
                        node = vi_net_node_create();

                        node->name = xstrdup(vertex->name);
                        node->type = vi_net_node_dummy;
                        node->X = (double) vertex->x_coor / net_graph->scale;
                        node->Y = vertex->y_coor;
                        list_add(net->dummy_node_list, node);
                }
        }

        for (i = 0 ; i < list_count(graph->edge_list); i++)
        {
                struct graph_edge_t *edge;
                struct vi_net_edge_data_t *edge_data;
                edge = list_get(graph->edge_list, i);
                edge_data = (struct vi_net_edge_data_t *) edge->data;

                // reverse data due to reversal
                if (edge->reversed)
                {
                	struct vi_net_link_t *temp;
                	temp = edge_data->downstream;
                	edge_data->downstream = edge_data->upstream;
                	edge_data->upstream = temp;
                }
                if (edge_data->downstream)
                {
                        struct vi_net_sub_link_t *subLink = vi_net_sub_link_create();
                        struct vi_net_link_t *link;
                        link = edge_data->downstream;
                        subLink->src_x = (double) edge->src_vertex->x_coor / net_graph->scale;
                        subLink->src_y = edge->src_vertex->y_coor;
                        subLink->dst_x = (double) edge->dst_vertex->x_coor / net_graph->scale;
                        subLink->dst_y = edge->dst_vertex->y_coor;
                        subLink->link = link;
                        subLink->index = list_count(link->sublink_list);
                        list_add(link->sublink_list, subLink);
                }

                if (edge_data->upstream)
                {
                        struct vi_net_sub_link_t *subLink = vi_net_sub_link_create();
                        struct vi_net_link_t *link;
                        link = edge_data->upstream;
                        subLink->src_x = (double) edge->dst_vertex->x_coor / net_graph->scale;
                        subLink->src_y = edge->dst_vertex->y_coor;
                        subLink->dst_x = (double) edge->src_vertex->x_coor / net_graph->scale;
                        subLink->dst_y = edge->src_vertex->y_coor;
                        subLink->link = link;
                        subLink->index = list_count(link->sublink_list);
                        list_add(link->sublink_list, subLink);
                }
        }
}
