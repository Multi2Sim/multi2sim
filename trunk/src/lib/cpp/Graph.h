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

#ifndef LIB_CPP_GRAPH_H
#define LIB_CPP_GRAPH_H

#include "String.h"

namespace misc
{

// Class vertex
class Vertex
{
public:

	// Vertex kind
	enum VertexKind
	{
		VertexKindInvalid = 0,
		VertexKindNode,
		VertexKindDummy
	};

protected:

	// Vertex name
	std::string name;

	// Vertex kind
	VertexKind vertex_kind;

	// Vertex coordinations
	int xValue;
	int yValue;

	// List of connected vertices to the vertex
	std::vector<Vertex *> incoming_vertices;
	std::vector<Vertex *> outgoing_vertices;

public:

	// Constructor
	Vertex(std::string name, VertexKind kind) :
		name(name),
		vertex_kind(kind)
	{
	}

	// Adding a vertex to the outgoing list of the current vertex
	void addOutgoingVertex(Vertex *vertex)
	{
		outgoing_vertices.emplace_back(vertex);
	}

	// Adding a vertex to the incoming list of the current vertex
	void addIncomingVertex(Vertex *vertex)
	{
		incoming_vertices.emplace_back(vertex);
	}
};

// Class edge
class Edge
{
	// Edge direction
	enum EdgeDirectionKind
	{
		EdgeDirectionKindInvalid = 0,
		EdgeDirectionKindUnidirectional,
		EdgeDirectionKindBidirectional
	};

	// The source vertex for the edge
	Vertex *source_vertex;

	// The destination vertex for the edge
	Vertex *destination_vertex;

	// Reversed is set when the edge is reversed in cycle removal process
	bool reveresed = false;

public:

	// Constructor
	Edge(Vertex *source_vertex, Vertex *destination_vertex) :
		source_vertex(source_vertex),
		destination_vertex(destination_vertex)
	{
	}

	// Return edge's source vertex
	Vertex *getSourceVertex() const { return source_vertex; }

	// Return edge's destination vertex
	Vertex *getDestinationVertex() const { return destination_vertex; }
};

class Graph
{

protected:

	// Maximum number of vertices in each layer of the graph
	int max_layer_vertices;

	// List of vertices in the graph
	std::vector<std::unique_ptr<Vertex>> vertices;

	// List of edges in the graph
	std::vector<std::unique_ptr<Edge>> edges;

public:

	// Return the maximum number of vertices in the layer
	int getMaxLayerVertices() const { return max_layer_vertices; }

	// Returns the layered drawing of the graph, by updating the
	// coordinations of each vertices, xValue and yValue
	void LayeredDrawing() {};

	// Virtual function for populate. Populating the graph with
	// edges and vertices depends on the data structure that uses
	// the graph.
	virtual void Populate() = 0;

	// Virtual function to scale the graph for the screen. This function
	// is dependent on where the graph is used, and what is the output
	// medium (e.g. file, screen)
	virtual void Scale() = 0;

};

}

#endif
