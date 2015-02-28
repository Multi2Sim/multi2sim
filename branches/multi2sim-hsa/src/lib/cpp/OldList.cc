/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cassert>

#include "Error.h"
#include "List.h"


namespace misc
{


List::Node *List::getCurrent()
{
	// Past the end
	if (current_index == size)
	{
		error = ErrorBounds;
		return nullptr;
	}

	// Found
	error = ErrorOK;
	return current;
}


List::Node *List::Next()
{
	// List empty
	if (size == 0)
	{
		error = ErrorEmpty;
		return nullptr;
	}

	// Past the end
	if (current_index == size)
	{
		error = ErrorBounds;
		return nullptr;
	}

	// Next element
	current_index++;
	current = current->next;

	// Past the end reached
	if (current_index == size)
	{
		error = ErrorBounds;
		return nullptr;
	}

	// Valid element
	error = ErrorOK;
	return current;
}


List::Node *List::Prev()
{
	// List is empty
	if (size == 0)
	{
		error = ErrorEmpty;
		return nullptr;
	}

	// Already in the beginning
	if (current_index == 0)
	{
		error = ErrorBounds;
		return nullptr;
	}

	// Previous element
	current_index--;
	current = current ? current->prev : tail;

	// Valid element
	error = ErrorOK;
	return current;
}


List::Node *List::Front()
{
	// List is empty
	if (size == 0)
	{
		error = ErrorEmpty;
		return nullptr;
	}

	// Go to head
	current_index = 0;
	current = head;

	// Valid element
	error = ErrorOK;
	return current;
}


List::Node *List::Back()
{
	// List is empty
	if (size == 0)
	{
		error = ErrorEmpty;
		return nullptr;
	}

	// Go to tail
	current_index = size - 1;
	current = tail;

	// Valid element
	error = ErrorOK;
	return current;
}


void List::End()
{
	current_index = size;
	current = nullptr;
	error = ErrorOK;
}


List::Node *List::Insert(Node *node)
{
	// Invalid node
	if (node == nullptr)
		throw misc::Panic("Inserted node cannot be null");

	// Node is already in a list
	if (node->in_list)
		throw misc::Panic("Inserted node is already in a list");

	// Insert it
	if (size == 0)
	{
		// List is empty
		current = node;
		head = node;
		tail = node;
	}
	else if (current == head)
	{
		// Insert at the head
		node->next = head;
		head->prev = node;
		head = node;
	}
	else if (current == nullptr)
	{
		// Insert at the tail
		node->prev = tail;
		tail->next = node;
		tail = node;
	}
	else
	{
		// Insert in the middle
		node->prev = current->prev;
		node->next = current;
		current->prev = node;
		node->prev->next = node;
	}
	
	// Update state
	error = ErrorOK;
	size++;
	current = node;
	node->in_list = true;
	return node;
}


List::Node *List::Remove()
{
	// Check bounds
	if (current_index == size)
	{
		error = ErrorBounds;
		return nullptr;
	}

	// Remove current element
	Node *node = current;
	if (size == 1)
	{
		head = nullptr;
		tail = nullptr;
	}
	else if (node == head)
	{
		node->next->prev = nullptr;
		head = node->next;
	}
	else if (node == tail)
	{
		node->prev->next = nullptr;
		tail = node->prev;
	}
	else
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	// Mark as removed
	assert(node->in_list);
	node->in_list = false;

	// Update state
	assert(size > 0);
	error = ErrorOK;
	size--;
	current = node->next;
	return node;
}


void List::Remove(Node *node)
{
	// Check valid node
	if (node == nullptr)
		throw misc::Panic("Removed element cannot be null");
	
	// Check that node was in a list
	if (!node->in_list)
		throw misc::Panic("Removed element is not in the list");

	// Remove element
	if (size == 1)
	{
		assert(head == node && tail == node);
		head = nullptr;
		tail = nullptr;
	}
	else if (node == head)
	{
		node->next->prev = nullptr;
		head = node->next;
	}
	else if (node == tail)
	{
		node->prev->next = nullptr;
		tail = node->prev;
	}
	else
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	// Update state
	assert(size > 0);
	node->in_list = false;
	error = ErrorOK;
	size--;
	current = head;
	current_index = 0;
}


}  // namespace misc

