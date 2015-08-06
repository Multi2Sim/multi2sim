/*
 *  Multi2Sim
 *  Copyright (C) 2014  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

		// Create vertex based on the node
		vertices.emplace_back(misc::new_unique<Vertex>(node, node->getName(),
				Vertex::VertexKindNode));
	}

	// Create the initial edges
	for (int i = 0; i < network->getNumConnections(); i++)
	{
		// Get the link associated with the edge
		Link *link = dynamic_cast<Link *>(network->getConnection(i));

		// Create edge based on the connection
		bool reverse_edge = false;
		if (link)
		{
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
				Edge *edge = static_cast<Edge *>(edges[j].get());
				if ((edge->getSourceVertex() == destination_vertex)
						&& (edge->getDestinationVertex() == source_vertex))
				{
					edge->setUpstreamLink(link);
					reverse_edge = true;
					break;
				}

			}

			// If the reverse edge was not found, add the edge to the list
			if (!reverse_edge)
			{
				edges.emplace_back(misc::new_unique<Edge>(link,
						source_vertex, destination_vertex));
				source_vertex->addOutgoingVertex(destination_vertex);
				destination_vertex->addIncomingVertex(source_vertex);
			}
		}
	}
}

}
