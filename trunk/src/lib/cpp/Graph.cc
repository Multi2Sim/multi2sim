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

namespace misc
{

void Graph::addEdge(std::unique_ptr<Edge> &&edge,
		Vertex *source_vertex,
		Vertex *destination_vertex)
{
	edges.emplace_back(std::move(edge));
	source_vertex->outgoing_vertices.emplace_back(
			destination_vertex);
	destination_vertex->incoming_vertices.emplace_back(
			source_vertex);
}


bool Graph::removeEdge(Edge *edge)
{
	// Look for the edge in the list of edges
	auto it = std::find_if(edges.begin(), edges.end(), 
			[&](std::unique_ptr<Edge> const& 
			unique_edge)
		{ 
			return unique_edge.get() == edge;
		});

	// If the edge is not found return false 
	if (it == edges.end())
	{
		return false;
	}
	// If found 
	else 
	{
		// Remove the edge from the graph's edges list
		edges.erase(it);

		// Get the source and destination vertex
		Vertex *source_vertex = edge->source_vertex;
		Vertex *destination_vertex = edge->destination_vertex;

		// Remove the destination vertex from outgoing vertex list of
		// the source vertex
		auto destination_vertex_it = std::find(
				source_vertex->outgoing_vertices.begin(),
				source_vertex->outgoing_vertices.end(),
				destination_vertex);
		assert(destination_vertex_it != source_vertex->
				outgoing_vertices.end());
		source_vertex->outgoing_vertices.erase(destination_vertex_it);

		// Remove the source vertex from incoming vertex list of
		// the destination vertex
		auto source_vertex_it = std::find(
				destination_vertex->incoming_vertices.begin(),
				destination_vertex->incoming_vertices.end(),
				source_vertex);
		assert(source_vertex_it != destination_vertex->
				incoming_vertices.end());
		destination_vertex->incoming_vertices.erase(source_vertex_it);
	}

	// return success
	return true;
}


Edge *Graph::findEdge(Vertex *source_vertex, Vertex *destination_vertex)
{
	// Search through the edges in the graph
	auto it = std::find_if(edges.begin(), edges.end(), 
			[&](std::unique_ptr<Edge> const& 
			unique_edge)
		{ 
			return ((unique_edge.get()->source_vertex == 
					source_vertex) && 
					(unique_edge.get()->destination_vertex ==
					destination_vertex));
		});

	// If the edge is not found return nullptr
	if (it == edges.end())
		return nullptr; 
	
	// Return the edge
	return (*it).get();
}


void Graph::GreedyCycleRemoval()
{
	// Identify the last position of the left side of the list which
	// we have moved a vertex to. At the beginning since we haven't
	// moved any of the vertices, the end is the end of the vertex list,
	// and the start is the beginning of the vertex list.
	int start = 1;
	int end = vertices.size() + 1;

	// Create a vector of vertexes for sorting
	std::vector<Vertex *> sort_list;

	// We use the x_value and key_value of the vertex for sorting the list,
	// to avoid adding new variable that will never be used again.
	for (auto &vertex : vertices)
	{
		if (vertex->getNumOutgoingVertices() == 0)
		{
			vertex->x_value = end;
			end--;
		}
		else if (vertex->getNumIncomingVertices() == 0)
		{
			vertex->x_value = start;
			start++;
		}

		vertex->key = vertex->getNumOutgoingVertices() -
				vertex->getNumIncomingVertices();

		sort_list.emplace_back(vertex.get());
	}

	// At this point, vertices with just outgoing edges have the least
	// x_values and the vertices with just incoming edges have the largest
	// x_values. Other vertices that have both incoming and outgoing
	// edges don't have any x_value. Here we sort the edges based on
	// their degree and sort the list. Whatever vertex that doesn't
	// still have a x_value would be assigned one. So the least is sorted
	// with vertices having the most number of outgoing, and the least number
	// of incoming edges to vertices with the least number of outgoing, and
	// most number of incoming edges.
	std::sort(sort_list.begin(), sort_list.end(), Vertex::Compare);
	for (auto &vertex : vertices)
	{
		if (vertex->x_value == 0)
		{
			vertex->x_value = start;
			start++;
		}
	}

	// We reverse an edge if its destination vertex is located before its
	// source vertex in the sorted list.
	for (auto &edge : edges)
	{
		Vertex *source_vertex = edge->source_vertex;
		Vertex *destination_vertex = edge->destination_vertex;
		if (destination_vertex->x_value < source_vertex->x_value)
		{
			// Remove the destination vertex from source vertex's outgoing list
			auto it = std::find(source_vertex->outgoing_vertices.begin(),
					source_vertex->outgoing_vertices.end(),
					destination_vertex);
			source_vertex->outgoing_vertices.erase(it);

			// Add the destination node to the incoming vertices of the
			// source node
			source_vertex->incoming_vertices.emplace_back(destination_vertex);

			// Remove the source vertex from destination vertex's incoming list
			it = std::find(destination_vertex->incoming_vertices.begin(),
					destination_vertex->incoming_vertices.end(),
					source_vertex);
			destination_vertex->incoming_vertices.erase(it);

			// Add the source vertex to destination vertex's outgoing list
			destination_vertex->outgoing_vertices.emplace_back(source_vertex);

			// Reverse the edge
			Vertex *temp = edge->destination_vertex;
			edge->destination_vertex = edge->source_vertex;
			edge->source_vertex = temp;

			// Set the edge is reversed boolean
			edge->reversed = true;
		}
	}

	// Reset the used values for next uses
	for (auto &vertex : vertices)
	{
			vertex->x_value = 0;
			vertex->key = 0;
	}
}


int Graph::CoffmanGrahamLayering(int width)
{
	//
	// Phase 1: Assigning labels
	//

	// Initialize the label to 1. The labels per vertex is unique so this
	// value increments every time it is assigned to a vertex.
	unsigned label = 1;

	// Easiest label assignment is to the vertices that just have
	// outgoing edges. In the layer assignment, which later uses this
	// labels, these vertices are places on top layers.
	for (auto &vertex : vertices)
	{
		if (vertex->incoming_vertices.size() == 0)
		{
			vertex->key = label;
			label++;
		}
	}

	// Continue the label assignment until all the vertices has been
	// assigned a unique label.
	while (label <= vertices.size())
	{
		// Create a candidate list for this label. A vertex is a candidate
		// if the source vertices of all the vertex's edges has already been
		// assigned a label.
		std::vector<Vertex *> candidates;
		for (auto &vertex : vertices)
		{
			if (vertex->key != 0)
				continue;

			bool is_candidate= true;
			for (auto &incoming_vertex : vertex->incoming_vertices)
			{
				if (incoming_vertex->key == 0)
				{
					is_candidate = false;
					break;
				}
			}

			if (is_candidate)
				candidates.emplace_back(vertex.get());
		}

		// Choose one of the candidates for the current label
		if (candidates.empty())
			throw Panic("Graph drawing algorithm is not functioning"
					"properly.");

		// The first element of the list is our initial candidate. Through
		// the following loop this candidate gets updated.
		Vertex *candidate = candidates[0];

		// We sort the incoming vertices of the candidate. This is used
		// when we compare the incoming vertices of the two candidates,
		// lexicographically, based on their incoming vertices.
		std::sort(candidate->incoming_vertices.begin(),
				candidate->incoming_vertices.end(), Vertex::Compare);

		// Getting the alternative candidate, and updating the candidate
		// if necessary
		for (unsigned i = 1; i < candidates.size(); i++)
		{
			// Getting the alternative candidate
			Vertex *alternate_candidate = candidates[i];

			// Calculating the minimum in-degree between the two candidates
			int minimum_indegree = (candidate->incoming_vertices.size() <
					alternate_candidate->incoming_vertices.size()) ?
							candidate->incoming_vertices.size() :
							alternate_candidate->incoming_vertices.size();

			// Sort the list of incoming vertices for the alternate candidate
			std::sort(alternate_candidate->incoming_vertices.begin(),
					alternate_candidate->incoming_vertices.end(),
					Vertex::Compare);

			// Since the two incoming vertices are sorted, we start from the
			// end which has the highest label (since sorted based on key)
			// and continue from the end to the beginning, until we find a
			// deference between the two vertex labels, and then we chose
			// the one with the lower label value.
			int it = 1;
			bool best_candidate_found = false;
			while (it <= minimum_indegree)
			{
				Vertex *candidate_lexical, *alternate_lexical;
				candidate_lexical = candidate->incoming_vertices
						[candidate->incoming_vertices.size() - it];
				alternate_lexical = alternate_candidate->incoming_vertices
						[alternate_candidate->incoming_vertices.size() - it];

				if (candidate_lexical == alternate_lexical)
				{
					it++;
				}
				else
				{
					// Replace the candidate with the alternate
					if (alternate_lexical < candidate_lexical)
						candidate = alternate_candidate;

					// The alternate is not lexicographically better, and
					// the vertices in the incoming vertices lists are not
					// equal, so the current candidate is the better
					// choice. In either way the candidate is found.
					best_candidate_found = true;
					break;
				}
			}

			// if the best candidate was not found, just choose the
			// candidate with least number of incoming vertices
			if ((!best_candidate_found) &&
					(alternate_candidate->incoming_vertices.size() <
						candidate->incoming_vertices.size()))
					candidate = alternate_candidate;
		}
		// Here we have a sole candidate, so we assign the label to it
		candidate->key = label;
		label++;
	}

	//
	// Phase 2: Layer assignment
	//

	// Initialize two lists, one is the list of ordered vertices, and the
	// other is the list of unordered vertices. Every iteration, we grab
	// from unordered list and add to the ordered list based on some
	// criteria describe within comments
	std::vector<Vertex *> ordered_list, unordered_list;

	// First we add all the vertices to the unordered_list
	for (auto &vertex : vertices)
		unordered_list.emplace_back(vertex.get());

	// Sort the unordered list based on the keys
	std::sort(unordered_list.begin(), unordered_list.end(), Vertex::Compare);

	// Every iteration we find a candidate to move to the ordered list, until
	// the unordered list is empty.
	while (!unordered_list.empty())
	{
		// The candidate has to have highest label so we start from the end
		// of the unordered list
		Vertex *candidate = nullptr;
		for (auto i = unordered_list.end() - 1;
				i >= unordered_list.begin(); --i)
		{
			Vertex *vertex = *i;

			// If the vertex has no outgoing edge, it is the candidate.
			// Otherwise the candidate should be the vertex that has all of
			// its outgoing vertices in the ordered list
			if (vertex->outgoing_vertices.size() == 0)
			{
				candidate = vertex;
			}
			else
			{
				bool unordered_vertex = false;
				for (Vertex *outgoing_vertex : vertex->outgoing_vertices)
				{
					if (std::find(ordered_list.begin(), ordered_list.end(),
							outgoing_vertex) == ordered_list.end())
					{
						unordered_vertex = true;
						break;
					}
				}
				if (!unordered_vertex)
					candidate = vertex;
			}

			// If we reach here and we still don't have a candidate,
			// continue the loop, otherwise this is the candidate
			if (!candidate)
				continue;

			// The current layer to put the vertex in
			int layer = 0;

			// Candidate should be place one layer above its outgoing vertices
			for (Vertex *outgoing_vertex : candidate->outgoing_vertices)
				if (outgoing_vertex->y_value >= layer)
					layer = outgoing_vertex->y_value + 1;

			// Layer width is the number of vertices in each layer.
			// As soon as it reaches a certain predefined value, we should
			// move place the vertex in the next layer.
			int layer_width = 0;

			// Count the number of vertices in the same layer as candidate
			for (Vertex *ordered_vertex : ordered_list)
				if (ordered_vertex->y_value == layer)
					layer_width++;

			// If the number of vertices in the layer is less than allowed
			// width, add the candidate to the layer. Otherwise move it to
			// the next layer
			if (layer_width < width)
			{
				layer_width++;
			}
			else
			{
				layer++;
				layer_width = 0;
			}
			candidate->y_value = layer;

			// Remove the candidate from the unordered list and add it
			// to the ordered list
			unordered_list.erase(i);
			ordered_list.emplace_back(candidate);
		}
	}

	// Returning the number of layers we have in the layered drawing
	int num_layers = 0;
	for (auto &vertex : vertices)
		if (vertex->y_value > num_layers)
			num_layers = vertex->y_value;
	return num_layers;
}


void Graph::InitializeXValues(int num_layers)
{
	// Setting the initial x_value and counting the number of vertices in
	// each layer
	for (int layer = 0; layer <= num_layers; layer++)
	{
		int num_vertices_in_layer = 0;
		for (auto &vertex : vertices)
			if (vertex->y_value == layer)
			{
				vertex->x_value = num_vertices_in_layer;
				num_vertices_in_layer++;
			}

		// Finding the number of vertices in most crowded layer
		if (max_vertices_in_layers < num_vertices_in_layer)
			max_vertices_in_layers = num_vertices_in_layer;

		// Calculating the number of neighbors for each vertex
		for (auto &vertex : vertices)
			if (vertex->y_value == layer)
				vertex->neighbors = num_vertices_in_layer;
	}
}


void Graph::CrossReduction(int num_layers)
{
	// Number of times we apply the cross reduction
	int reduction_count = 10;

	for (int it = 0 ; it < reduction_count; it++)
	{
		// Cross reduction can be in two directions/cases.
		// Case 0) Upper layers to lower layers
		// Case 1) Lower layers to upper layers
		// Each iteration is identified by the which layer to begin,
		// which layer to end, and the direction itself.
		int begin = 0;
		int end = 0;
		int direction = 0;
		if (it / 2)
		{
			begin = 0;
			end = num_layers;
			direction = 1;
		}
		else
		{
			begin = num_layers;
			end = 0;
			direction = -1;
		}

		// The number of intersections between two levels are minimum
		// if we move the vertex, which is one layer apart from its
		// source vertices of its edges, closest to these source vertices.
		// Since we cannot change the layer (Y value), we reduce the
		// distance by changing the position in the layer (X value).
		// The vertex is optimally close to all the source vertices,
		// if we get the position of the source vertices, and place
		// the vertices right between them.
		for (int layer = begin; layer <= end; layer += direction)
		{
			// For every vertex we create a cross reduction key value
			for (auto &vertex : vertices)
			{
				// If the vertex is in the current layer under 
				// examination
				if (vertex->y_value == layer)
				{
					// The vertex's key is used to store the 
					// optimal position for cross reduction 
					// value of vertices in the current
					// list. Since this value might be float, 
					// we will have vertices with overlapping 
					// keys
					vertex->key = 0;

					// If the number of incoming vertices is 
					// zero, keep the previous position, 
					// which it is likely that it was calculated 
					// in iteration of the reduction in previous
					// layer.
					if (vertex->incoming_vertices.size() == 0)
						vertex->key = vertex->x_value;
					
					// Calculate vertex's optimal cross 
					// value by adding the x value of 
					// the source vertices of its edges
					// and dividing it by the number of 
					// source vertices
					else
					{
						// Adding the x value of the source
						// vertices of the vertex outgoing
						// edges
						for (Vertex *incoming_vertex :
								vertex->incoming_vertices)
							vertex->key += incoming_vertex->
									x_value;

						// Dividing the total with the 
						// number of source vertices to
						// calculate the optimal cross value
						vertex->key /= vertex->incoming_vertices.size();
					}
				}
			}

			// At this point we have updated all the vertices. We will again
			// identify the vertices in the current layer, add them to a
			// separate list, and sort them in the correct order by their
			// optimal position in the layer
			std::vector<Vertex *> neighboring_vertices;
			for (auto &vertex : vertices)
				if (vertex->y_value == layer)
					neighboring_vertices.emplace_back(vertex.get());
			std::sort(neighboring_vertices.begin(),
					neighboring_vertices.end(), Vertex::Compare);

			// To overcome the overlapping vertices problem, we traverse
			// the sorted list, and move the vertices to the next available
			// spot in the x direction, starting from zero.
			Vertex *first_vertex = neighboring_vertices[0];
			int next_position = 0;

			for (unsigned i = 0; i < neighboring_vertices.size(); i++)
			{
				Vertex *vertex = neighboring_vertices[i];
				vertex->x_value = next_position;
				next_position++;
			}

			// Last step in left-aligning the layer
			first_vertex->x_value = 0;
		}
	}
}

bool Graph::CycleDetectionDepthFirstTraverse(int vertex_id,
		bool* visited,
		bool* stacked)
{
	// If the vertex is not already visited, start the depth-first traversal
	if (visited[vertex_id] == false)
	{
		// Mark the current vertex as visited, and place it in the stack
		visited[vertex_id] = true;
		stacked[vertex_id] = true;

		// Do the depth-first for all the adjacent (outgoing) vertices 
		// of the vertex
		Vertex *vertex = getVertex(vertex_id);
		for (int i = 0 ; i < vertex->getNumOutgoingVertices(); i++)
		{
			// Get the adjacent vertex
			Vertex* adjacent_vertex = vertex->
					outgoing_vertices[i];

			// Find the identifier of the adjacent vertex in
			// vertices list. The vertices list is unique pointers
			// and the vertex is pointer.
			auto it = std::find_if(vertices.begin(), vertices.end(),
					[&](std::unique_ptr<Vertex> const& 
					unique_vertex)
					{ 
						return unique_vertex.get() == 
								adjacent_vertex;
					});

			// If the vertex is not found there is a problem
			assert(it != vertices.end());

			// get the vertex id based on the distance from the
			// beginning of the vector
			int adjacent_vertex_id = std::distance(vertices.begin(), 
					it);
			
			// If the adjacent vertex is not visited, do the depth 
			// first for the adjacent vertex. If found cycle in the
			// next depth-first from adjacent vertex, return cycle
			// exists (true), otherwise check to see the adjacent
			// vertex is stacked for later traverse
			if ((visited[adjacent_vertex_id] == false) && 
					(CycleDetectionDepthFirstTraverse(
					adjacent_vertex_id, visited, stacked)))
				return true;
			else if (stacked[adjacent_vertex_id] == true)
				return true;
		}
	}

	// The whole adjacent list is traversed, and cycle is not found
	// so remove the vertex from stacked list
	stacked[vertex_id] = false;

	// No cycle is found
	return false;
}


bool Graph::hasCycle()
{
	// Creating visited and stacked lists for vertices that are visited,
	// and stacked for finishing the depth-first traversal later, 
	// respectively
	auto visited = misc::new_unique_array<bool>(getNumVertices());
	auto stacked = misc::new_unique_array<bool>(getNumVertices());

	// Initialing the lists to false
	for (int i = 0; i < getNumVertices(); i++)
	{
		visited[i] = false;
		stacked[i] = false;
	}

	// Call to the depth-first traversal for each vertex in the graph
	for (int vertex_id = 0; vertex_id < getNumVertices(); vertex_id++)
	{
		// Performing the depth-first traversal on the vertex
		if (CycleDetectionDepthFirstTraverse(vertex_id, visited.get(), 
				stacked.get()))
			return true;
	}
	
	// If cycle was not detected
	return false;
}

}
