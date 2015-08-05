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

#include "String.h"

namespace misc
{

// Class vertex
class Vertex
{
	// Vertex kind
	enum VertexKind
	{
		VertexKindInvalid = 0,
		VertexKindNode,
		VertexKindDummy
	};

	// Vertex name
	std::string name;

	// Vertex coordinations
	int xValue;
	int yValue;

	// Vertex degrees and list of connected vertices
	int in_degree;
	std::vector<Vertex *> input_vertices;
	int out_degree;
	std::vector<Vertex *> output_vertices;


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
};

class Graph
{

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
