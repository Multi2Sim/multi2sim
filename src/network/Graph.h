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

#ifndef NETWORK_GRAPH_H
#define NETWORK_GRAPH_H

#include <lib/cpp/Graph.h>
#include "Network.h"
#include "Node.h"
#include "Link.h"

namespace net
{

class Vertex : public misc::Vertex
{

	friend class Graph;

public:
	// Vertex kind
	enum Kind
	{
		KindInvalid = 0,
		KindNode,
		KindDummy
	};


private:
	// Associated node with the vertex
	Node *node;

	Kind kind;


public:
	// Constructor
	Vertex(Node *node, std::string name, Kind kind):
			misc::Vertex(name),
			node(node),
			kind(kind)
	{
	}

	// Return the vertex kind
	Kind getKind() const { return kind; }
};

class Edge : public misc::Edge
{
	friend class Graph;

	// Associated link with the edge, in unidirectional case
	Link *downstream_link = nullptr;

	// Other associated link with the edge, in case it is bidirectional
	Link *upstream_link = nullptr;

public:

	// Constructor
	Edge(Link *link,
			misc::Vertex *source_vertex,
			misc::Vertex *destination_vertex):
		misc::Edge(source_vertex, destination_vertex),
		downstream_link(link)
	{
	}

	// Setting the upstream link for the network edge
	void setUpstreamLink(Link *link) { upstream_link = link; }
};

class Graph : public misc::Graph
{
	// Associated network
	Network *network;

public:

	/// Constructor
	Graph(Network *network);

	/// Dump the information related to static visualization graph
	void DumpGraph(std::ostream &os) const;

	// Populate the graph based on the information provided by the network
	void Populate();

	// Draw the layered graph drawing.
	void LayeredDrawing();

	// Adding dummy vertices to the graph
	void AddDummyVertices();

	/// Function to scale the graph for the screen. This function
	/// is dependent on where the graph is used, and what is the output
	/// medium (e.g. file, screen)
	void Scale();
};

}

#endif
