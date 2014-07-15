/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CPP_LINKED_LIST_H
#define LIB_CPP_LINKED_LIST_H

#include <cassert>


namespace misc
{

/// This class implements a linked list where node pointers are allocated
/// in-place within the objects contained by the lists. This is accomplished
/// by forcing list elements to be children of class List::Node.
template<typename T> class List
{
public:

	/// Linked list node. All objects inserted into the linked list must
	/// be derived of this class. An object instance can belong to only one
	/// linked list at a time.
	class Node
	{
		// The list can update the node's pointers
		friend class List;

		// Pointer to the previous node
		Node *prev = nullptr;

		// Pointer to next node
		Node *next = nullptr;

		// List that the node is inserted in, or null if the node does
		// not currently belong to any list.
		List<T> *list = nullptr;

		// Data associated with the node
		T *data;
	
	public:

		/// Constructor
		Node(T *data) : data(data)
		{
			assert(data != nullptr);
		}

		/// The destructor automatically extracts the node from the
		/// list, if it was present in one.
		~Node()
		{
			if (list)
				list->Erase(*this);
		}
	};

	/// Iterator to an element in the list
	class Iterator
	{
		// Only the list can access the iterator's fields
		friend class List;

		// Node that the iterator points to
		Node *node;

		// Only the list can create an iterator
		Iterator(Node *node) : node(node)
		{
		}
	
	public:

		/// Operator !=
		bool operator!=(const Iterator &right) const
		{
			return node != right.node;
		}

		/// Operator ==
		bool operator==(const Iterator &right) const
		{
			return node == right.node;
		}

		/// Pre-increment operator
		const Iterator &operator++()
		{
			assert(node != nullptr);
			node = node->next;
			return *this;
		}

		/// Pre-decrement operator
		const Iterator &operator--()
		{
			assert(node != nullptr);
			node = node->prev;
			return *this;
		}

		/// Dereference operator
		T *operator*() const
		{
			assert(node != nullptr);
			return node->data;
		}
	};

private:

	// Pointer to the first node in the list
	Node *head = nullptr;

	// Pointer to the last node in the list
	Node *tail = nullptr;

	// Number of element in the list
	int size = 0;

public:

	/// Return the number of elements in the list
	int getSize() const { return size; }

	/// Return an iterator to the first element in the list
	Iterator begin()
	{
		return Iterator(head);
	}

	/// Return a past-the-end iterator
	Iterator end()
	{
		return Iterator(nullptr);
	}

	/// Return the first element in the list, or `nullptr` if the list is
	/// empty.
	T *Front()
	{
		return head ? head->data : nullptr;
	}

	/// Return the last element in the list, or `nullptr` if the list is
	/// empty.
	T *Back()
	{
		return tail ? tail->data : nullptr;
	}

	/// Insert a node in the list before the node pointed to by the
	/// iterator, or at the end if the iterator is a past-the-end
	/// iterator.
	Iterator Insert(Iterator position, Node &node)
	{
		assert(node.list == nullptr);
		if (size == 0)
		{
			// List is empty
			head = &node;
			tail = &node;
		}
		else if (position == begin())
		{
			// Insert at the head
			node.next = head;
			head->prev = &node;
			head = &node;
		}
		else if (position == end())
		{
			// Insert at the tail
			node.prev = tail;
			tail->next = &node;
			tail = &node;
		}
		else
		{
			// Insert in the middle
			node.prev = position.node->prev;
			node.next = position.node;
			position.node->prev = &node;
			node.prev->next = &node;
		}
	
		// Update state
		size++;
		node.list = this;
		return Iterator(&node);
	}

	/// Insert an element at the end of the list, and return an iterator
	/// to the inserted element.
	Iterator PushBack(Node &node)
	{
		return Insert(end(), node);
	}

	/// Insert an element at the beginning of the list, and return an
	/// iterator to the inserted element.
	Iterator PushFront(Node &node)
	{
		return Insert(begin(), node);
	}

	/// Remove the elemented at the specified position in the list, and
	/// return an iterator to the element following it, or a past-the-end
	/// iterator if it was the last element.
	Iterator Erase(Iterator position)
	{
		assert(size > 0);
		assert(position.node != nullptr);
		assert(position.node->list == this);

		// Remove current element
		if (size == 1)
		{
			assert(head == position.node);
			assert(tail == position.node);
			head = nullptr;
			tail = nullptr;
		}
		else if (position.node == head)
		{
			position.node->next->prev = nullptr;
			head = position.node->next;
		}
		else if (position.node == tail)
		{
			position.node->prev->next = nullptr;
			tail = position.node->prev;
		}
		else
		{
			position.node->prev->next = position.node->next;
			position.node->next->prev = position.node->prev;
		}

		// Mark as removed
		position.node->list = nullptr;
		size--;

		// Return iterator to next element
		return Iterator(position.node->next);
	}

	
	/// Alternative syntax for Erase() that takes a list node as an
	/// argument.
	Iterator Erase(Node &node)
	{
		return Erase(Iterator(&node));
	}
};

} // namespace misc

#endif


