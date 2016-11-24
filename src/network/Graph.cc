/*
 *  Multi2Sim
 *  Copyright (C) 2015  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

#include "Graph.h"
#include "EndNode.h"

namespace net
{

Graph::Graph(Network *network):
		misc::Graph(),
		network(network)
{
}


void Graph::Populate()
{
	// Creating the initial vertices
	for (int i = 0; i < network->getNumNodes(); i++)
	{
		// Get the node associated with the vertex
		Node *node = network->getNode(i);

		// Adding the vertex to the graph's vertices list
		addVertex(misc::new_unique<Vertex>(node, 
				node->getName(), Vertex::KindNode));

//misc::cast<std::unique_ptr<misc::Vertex>>(vertex));
	}

	// Create the initial edges
	for (int i = 0; i < network->getNumConnections(); i++)
	{

		// Create edge based on the connection
		bool bidirectional = false;
		if (dynamic_cast<Link *>(network->getConnection(i)))
		{
			// Get the link associated with the edge
			Link *link = misc::cast<Link *>(network->getConnection(i));

			// Get the edge's source vertex
			int index = link->getSourceNode()->getIndex();
			misc::Vertex *source_vertex = vertices[index].get();

			// Get the edge's destination vertex
			index = link->getDestinationNode()->getIndex();
			misc::Vertex *destination_vertex = vertices[index].get();

			// First look through the edges to see if the reverse version of
			// this edge exists. If it does ignore this edge.
			for (unsigned j = 0; j < edges.size(); j++)
			{
				Edge *edge = misc::cast<Edge *>(edges[j].get());
				if ((edge->getSourceVertex() == destination_vertex)
						&& (edge->getDestinationVertex() == source_vertex))
				{
					assert(edge->downstream_link);
					edge->setUpstreamLink(link);
					bidirectional = true;
					break;
				}
			}

			// If the reverse edge was not found, which means the
			// edge is not bidirectional,  add the edge to the list
			if (!bidirectional)
			{
				addEdge(misc::new_unique<Edge>(link, 
						source_vertex, 
						destination_vertex),
						source_vertex,
						destination_vertex);
						
			}
		}
		else
			throw misc::Panic("Network contains unknown link to "
					"graph generation");
	}
}


void Graph::AddDummyVertices()
{
	// For every edge, if the distance between two vertices is more than
	// one layer, we have to add a dummy node per each layer between
	// the two vertices
	for (unsigned i = 0; i < edges.size(); i++)
	{
		Edge *edge = misc::cast<Edge *>(edges[i].get());
		Vertex *source = misc::cast<Vertex *>(edge->getSourceVertex());
		Vertex *destination = misc::cast<Vertex *>(
				edge->getDestinationVertex());
		if(source->y_value - destination->y_value > 1)
		{

			// Creating a dummy vertices and adding it to the vertex list
			vertices.emplace_back(misc::new_unique<Vertex>(nullptr, "dummy",
					Vertex::KindDummy));
			Vertex *dummy = misc::cast<Vertex *>(vertices.back().get());
			assert(dummy->kind == Vertex::KindDummy);

			// Place the dummy vertex one layer below the source vertex.
			// Just one dummy vertex is added, while next dummy vertices
			// will be added in the next iterations of the loop, between
			// the new dummy vertex, created here, and the destination
			// vertex.
			dummy->y_value = source->y_value - 1;

			// There are just one incoming and one outgoing edge from/to
			// the dummy vertex. Update the neighboring vertices with
			// the new information
			dummy->incoming_vertices.emplace_back(source);
			source->outgoing_vertices.emplace_back(dummy);
			dummy->outgoing_vertices.emplace_back(destination);
			destination->incoming_vertices.emplace_back(dummy);

			// Removing the previous destination vertex from outgoing
			// list of the source vertex and vice versa
			auto it = std::find(source->outgoing_vertices.begin(),
					source->outgoing_vertices.end(), destination);
			if (it == source->outgoing_vertices.end())
				throw misc::Panic("Layered graph drawing encountered a problem "
						"with setting the destination for vertices");
			source->outgoing_vertices.erase(it);

			it = std::find(destination->incoming_vertices.begin(),
					destination->incoming_vertices.end(), source);
			if (it == destination->incoming_vertices.end())
				throw misc::Panic("Layered graph drawing encountered a problem "
						"with setting the source for vertices");
			destination->incoming_vertices.erase(it);

			// Replacing the destination vertex of the existing edge
			// with the dummy vertex
			edge->destination_vertex = dummy;

			// Adding a new edge from dummy to the destination.
			// By placing the edge at the end we make sure it would
			// be processed again, and new dummy nodes will be added
			// if required.
			edges.emplace_back(misc::new_unique<Edge>(edge->downstream_link,
					dummy, destination));
			Edge *dummy_edge = misc::cast<Edge *>(edges.back().get());
			dummy_edge->reversed = edge->reversed;
			dummy_edge->upstream_link = edge->upstream_link;
		}
	}
}


void Graph::LayeredDrawing()
{
	// Populate the graph
	Populate();

	// First remove the cycles in the graph
	GreedyCycleRemoval();

	// Maximum number of nodes in each layer
	int width = getNumVertices() / 2 + 1;

	// Applying the main layering algorithm
	int num_layers = CoffmanGrahamLayering(width);

	// Populating the graph with dummy vertices
	AddDummyVertices();

	// Initializing the x coordinate of each vertex
	InitializeXValues(num_layers);

	// Cross reduction to reduce the intersection of the edges
	CrossReduction(num_layers);

	// Scale the graph appropriately for the static version
	Scale();
}


void Graph::Scale()
{
	// Calculating an optimal distance between every two adjacent vertex
	// while center aligning the graph
	int optimal_distance = getMaxVerticesInLayers() * getNumVertices() /
			(2 * getMaxVerticesInLayers() - 1);
	for (int i = 0; i < getNumVertices() ; i++)
	{
		Vertex *vertex = misc::cast<Vertex *>(getVertex(i));
		vertex->x_value = ((2 * vertex->x_value) + getMaxVerticesInLayers()
				- vertex->neighbors) * optimal_distance;
	}
}


void Graph::DumpGraph(std::ostream &os) const
{
	// Add the legend and the name of the network
	os << "Legend = True\n";
	os << misc::fmt("Title = \" Network: %s \"\n", network->getName().c_str());

	// Add the information about the vertices to the graph file
	for (int i = 0; i < getNumVertices(); i++)
	{
		Vertex *vertex = misc::cast<Vertex *>(getVertex(i));

		// Network nodes have different representation than dummy nodes
		if (vertex->kind == Vertex::KindNode)
		{
			// Two separate node types (end-node, switch) have different
			// color representations in the graph
			if (dynamic_cast<EndNode *>(vertex->node))
				os << misc::fmt("node = %s 0 %f %d\n", vertex->name.c_str(),
					(double) vertex->x_value / getNumVertices(),
					vertex->y_value);
			else
				os << misc::fmt("node = %s 1 %f %d\n", vertex->name.c_str(),
					(double) vertex->x_value / getNumVertices(),
					vertex->y_value);
		}
		else if (vertex->kind == Vertex::KindDummy)
		{
			os << misc::fmt("node = %s 3 %f %d\n", vertex->name.c_str(),
				(double) vertex->x_value / getNumVertices(),
				vertex->y_value);
		}
	}

	// Add the information about the edges to the graph file
	for (int i = 0; i < getNumEdges(); i++)
	{
		Edge *edge = misc::cast<Edge *>(getEdge(i));

		Vertex *source_vertex = misc::cast<Vertex *>(edge->
				getSourceVertex());
		Vertex *destination_vertex = misc::cast<Vertex *>(edge->
				getDestinationVertex());

		// Edge can be either unidirectional (1) or bidirectional (2)
		int edge_type = 1;

		// Get the current cycle for utilization calculation
		System *net_system = System::getInstance();
		long long cycle = net_system->getCycle();

		// First we add the information on upstream link
		Link *link = nullptr;
		int utilization = 0;
		if (edge->upstream_link)
		{
			// Update the edge type to bidirectional
			edge_type = 2;

			// The information on the upstream link
			link = edge->upstream_link;
			assert(link);
			utilization = (int) ((cycle ? (double)
					link->getTransferredBytes() / (cycle *
							link->getBandwidth()) : 0) * 10);
			os << misc::fmt("link = %f %d %f %d %d %d\n",
					(double) source_vertex->x_value / getNumVertices(),
					source_vertex->y_value,
					(double) destination_vertex->x_value / getNumVertices(),
					destination_vertex->y_value,
					utilization, edge_type);
		}
		// The information on the downstream link
		link = edge->downstream_link;
		utilization = (int) ((cycle ? (double)
				link->getTransferredBytes() / (cycle *
						link->getBandwidth()) : 0) * 10);
		os << misc::fmt("link = %f %d %f %d %d %d\n",
				(double) destination_vertex->x_value / getNumVertices(),
				destination_vertex->y_value,
				(double) source_vertex->x_value / getNumVertices(),
				source_vertex->y_value,
				utilization, edge_type);
	}
}


}
