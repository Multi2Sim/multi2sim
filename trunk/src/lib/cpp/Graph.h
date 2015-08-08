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

	// Vertex coordinations
	int x_value = 0;
	int y_value = -1;

	// Key value that is used in different algorithm including sorting
	int key = 0;

	// Number of vertices in the layer which the vertex is located
	int neighbors = 0;

	// List of connected vertices to the vertex
	std::vector<Vertex *> incoming_vertices;
	std::vector<Vertex *> outgoing_vertices;

	// Vertex kind
	VertexKind vertex_kind;

public:

	// Constructor
	Vertex(std::string name, VertexKind kind) :
		name(name),
		vertex_kind(kind)
	{
	}

	// Destructor
	virtual ~Vertex() {};

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

	// Return an incoming vertex by index
	Vertex *getIncomingVertex(int index) const {
		return incoming_vertices[index];
	}

	// Return the vertex kind
	VertexKind getVertexKind() const { return vertex_kind; }

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

protected:

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

	// Destructor
	virtual ~Edge() {};

	// Return edge's source vertex
	Vertex *getSourceVertex() const { return source_vertex; }

	// Return edge's destination vertex
	Vertex *getDestinationVertex() const { return destination_vertex; }
};

class Graph
{

protected:

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
	///
	/// \return
	///	The number of layers created by the algorithm
	int CoffmanGrahamLayering(int width);

	// Vertex Promote Heuristics modifies a given layered graph by promoting
	// vertices from the layer where they are placed to the layer above.
	// To promote a vertex, is to add a new layer if and only if the new
	// layer would reduce the total number of dummy nodes that has to
	// be created for the layered graph.
	void VertexPromoteHeuristics() {};

	// This function adds dummy vertices to layers in order to clarify the path
	// of the connections between vertices that are distant from each other.
	virtual void AddDummyVertices() = 0;

	/// This function gives an initial value to the x coordinate of each
	/// vertices. This value later changes by another function, to
	/// optimize the placement of the vertex in the layer.
	///
	/// \param num_layer
	/// Number of layers in the graph
	void InitializeXValues(int num_layers);

	/// This function reduces the number of intersections between the edges
	/// of two consecutive layer by interchanging the position of pairs
	/// of vertices in one of the layers. It continues this procedure for
	/// all the layers. The more the cross-reduction is applied the cleaner
	/// the graph becomes, until it stops changing, which means we reached
	/// the least possible number of intersections. Since the problem is
	/// NP-Complete, we never reach the most optimal case. The cross-reduction
	/// works perfectly for two layers.
	///
	/// \param num_layers
	///	Number of layers in the graph
	void CrossReduction(int num_layers);

	// Number of vertices in the most crowded layer. This value can be less
	// than the width, and is required for the cross reduction.
	int max_vertices_in_layers = 0;

	/// List of vertices in the graph
	std::vector<std::unique_ptr<Vertex>> vertices;

	/// List of edges in the graph
	std::vector<std::unique_ptr<Edge>> edges;

public:

	/// Return the maximum number of vertices in the layer
	int getMaxVerticesInLayers() const { return max_vertices_in_layers; }

	/// Returns the layered drawing of the graph, by updating the
	/// coordinations of each vertices, xValue and yValue
	virtual void LayeredDrawing() = 0;

	/// Virtual function for populate. Populating the graph with
	/// edges and vertices depends on the data structure that uses
	/// the graph.
	virtual void Populate() = 0;
};

}

#endif
