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

void Graph::GreedyCycleRemoval()
{
	// Identify the last position of the left side of the list which
	// we have moved a vertex to. At the beginning since we haven't
	// moved any of the vertices, the end is the end of the vertex list,
	// and the start is the beginning of the vertex list.
	int start = 1;
	int end = vertices.size() + 1;

	// We use the x_value and key_value of the vertex for sorting the list,
	// to avoid adding new variable that will never be used again.
	for (auto &vertex : vertices)
	{
		if (vertex->getNumOutgoingVertecies() == 0)
		{
			vertex->x_value = end;
			end--;
		}
		else if (vertex->getNumIncomingVertices() == 0)
		{
			vertex->x_value = start;
			start++;
		}

		vertex->key = vertex->getNumOutgoingVertecies() -
				vertex->getNumIncomingVertices();
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
	std::sort(vertices.begin(), vertices.end());
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
			edge->reveresed = true;
		}
	}

	// Reset the used values for next uses
	for (auto &vertex : vertices)
	{
			vertex->x_value = 0;
			vertex->key = 0;
	}
}


void Graph::CoffmanGrahamLayering(int width)
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

			bool isCandidate = true;
			for (auto &incoming_vertex : vertex->incoming_vertices)
			{
				if (incoming_vertex->key == 0)
				{
					isCandidate = false;
					break;
				}
			}

			if (isCandidate)
			{
				candidates.emplace_back(vertex.get());
			}
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
}


void Graph::LayeredDrawing()
{
	// First remove the cycles in the graph
	GreedyCycleRemoval();

	// Maximum number of nodes in each layer
	int width = vertices.size() / 2 + 1;

	// Applying the main layering algorithm
	CoffmanGrahamLayering(width);

}

}
