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

#include <algorithm>

#include "Error.h"
#include "String.h"

namespace misc
{

// Class vertex
class Vertex
{

	friend class Graph;

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
	int x_value = 0;
	int y_value = -1;

	// Key value that is used in different algorithm including sorting
	int key = 0;

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

	// Return the size of the outgoing list, also known as out-degree
	int getNumOutgoingVertecies() const { return outgoing_vertices.size(); }

	// Adding a vertex to the incoming list of the current vertex
	void addIncomingVertex(Vertex *vertex)
	{
		incoming_vertices.emplace_back(vertex);
	}

	// Return the size of the incoming list, also known as in-degree
	int getNumIncomingVertices() const { return incoming_vertices.size(); }

	// Sorting compare function based on the key
	static bool Compare(const Vertex* vertex_a, const Vertex* vertex_b)
	{
		return (vertex_a->key < vertex_b->key);
	}
};

// Class edge
class Edge
{

	friend class Graph;

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

private:

	// Removes the cycles in the graph, by reversing on of the edges that
	// creates the cycle. The algorithm successively removes vertices of
	// the graph, and 'moves' each source vertices of the edges to the end
	// of list's left, and the destination vertices to the list's right.
	// This way the list is sorted in-place based on the in- and out- degrees.
	// A vertex which is part of the cycle, is both source to one edge,
	// and destination to another edge. With greedy cycle removal, we
	// choose the vertex that has the biggest difference in the in-
	// and out- degrees from the right_list, and move it to the left_list,
	// and reverse its edges.
	void GreedyCycleRemoval();

	/// Coffman-Graham Layering algorithm is a two phase algorithm; the
	/// first phase orders the vertices based on a specific label. This
	/// label is used in the second phase, which assign vertices to
	/// layers. The algorithm chooses an arrangement such that an
	/// element that comes after another in the order is assigned to a
	/// lower level, and such that each level has a number of elements
	/// that does not exceed a fixed width.
	///
	/// \param width
	///	Predefined value equal to the maximum number of vertices allowed
	/// in each layer.
	void CoffmanGrahamLayering(int width);


protected:

	/// Maximum number of vertices in each layer of the graph
	int max_layer_vertices;

	/// List of vertices in the graph
	std::vector<std::unique_ptr<Vertex>> vertices;

	/// List of edges in the graph
	std::vector<std::unique_ptr<Edge>> edges;

public:

	/// Return the maximum number of vertices in the layer
	int getMaxLayerVertices() const { return max_layer_vertices; }

	/// Returns the layered drawing of the graph, by updating the
	/// coordinations of each vertices, xValue and yValue
	void LayeredDrawing();

	/// Virtual function for populate. Populating the graph with
	/// edges and vertices depends on the data structure that uses
	/// the graph.
	virtual void Populate() = 0;

	/// Virtual function to scale the graph for the screen. This function
	/// is dependent on where the graph is used, and what is the output
	/// medium (e.g. file, screen)
	virtual void Scale() = 0;

};

}

#endif
