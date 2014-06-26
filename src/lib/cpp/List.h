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

namespace misc
{

/// This class implements a linked list where node pointers are allocated
/// in-place within the objects contained by the lists. This is accomplished
/// by forcing list elements to be children of class List::Node.
class List
{
public:

	/// Linked list node. All objects inserted into the linked list must
	/// be derived of this class. An object instance can belong to only one
	/// linked list at a time.
	class Node
	{
		friend class List;
		Node *prev = nullptr;
		Node *next = nullptr;
		bool in_list = false;
	
	public:

		// Make class polymorphic with a virtual destructor
		virtual ~Node();
	};

	/// Error of the latest operation on the list
	enum Error
	{
		 ErrorOK,
		 ErrorBounds,
		 ErrorEmpty,
		 ErrorNotFound
	};

private:

	// List pointers
	Node *head = nullptr;
	Node *tail = nullptr;
	Node *current = nullptr;

	// Index of the current element
	int current_index = 0;

	// Number of element in the list
	int size = 0;

	// Error of latest operation
	Error error = ErrorOK;

public:

	/// Return the error of the last operation on the list that is specified
	/// to update this value.
	Error getError() const { return error; }

	/// Return the number of elements in the list
	int getSize() const { return size; }

	/// Return the current element in the list. If the current position is
	/// past the end, return `nullptr`. The list error code is updated to:
	///
	/// - `ErrorOK`: Valid element returned.
	/// - `ErrorBounds`: Current element is past the end.
	Node *getCurrent();

	/// Return the index of the current element. This is a value between 0
	/// and *N*, where *N* is the number of elements in the list. A value of
	/// *N* indicates a past-the-end position. The list error code is set to
	/// `ErrorOK`.
	int getCurrentIndex()
	{
		error = ErrorOK;
		return current_index;
	}

	/// Move the current element to the next position and return the new
	/// current element, or `nullptr` if the previous or the new position
	/// is past the end or the list is empty The error code is updated to:
	///
	/// - `ErrorOK`: Valid element returned.
	/// - `ErrorEmpty`: The list is empty.
	/// - `ErrorBounds`: Position was or is now past the end
	Node *Next();

	/// Move the current element to the previous position and return the
	/// new current element, or `nullptr` if the current position is already
	/// the beginning of the list, or the list is empty. The error code is
	/// updated to:
	///
	/// - `ErrorOK`: Valid element returned.
	/// - `ErrorEmpty`: The list is empty.
	/// - `ErrorBounds`: Position was the beginning of the list.
	Node *Prev();

	/// Move the current element to the head of the list and return the
	/// element contained at that position, or `nullptr` if the list is
	/// empty. The error code is updated to:
	///
	/// - `ErrorOK`: Valid element returned.
	/// - `ErrorEmpty`: The list is empty.
	Node *Front();

	/// Move the current element to the last valid element of the list, and
	/// return the element at that position, or `nullptr` if the list is
	/// empty. The error code is updated to:
	///
	/// - `ErrorOK`: Valid element returned.
	/// - `ErrorEmpty`: The list is empty.
	Node *Back();

	/// Move the current element to a past-the-end position. The error code
	/// is set to `ErrorOK`.
	void End();

	/// Insert an element in the list before the element at the current
	/// position or at the end if the current position is past the end. The
	/// inserted element cannot be `nullptr`. The current position is
	/// updated to that of the new element. The error code is set to
	/// `ErrorOK`.
	///
	/// \throw
	///	This function throws a `misc::Panic` exception if the
	///	inserted element is `nullptr`, or if the inserted element was
	///	already a member of a list.
	Node *Insert(Node *node);

	/// Insert an element at the end of the list. The inserted element
	/// cannot be `nullptr`. The current element is set to the inserted
	/// element, that is, the tail of the list. The error code is set to
	/// `ErrorOK`.
	///
	/// \throw
	///	This function throws a `misc::Panic` exception if the
	///	inserted element is `nullptr`, or if the inserted element was
	///	already a member of a list.
	Node *PushBack(Node *node)
	{
		End();
		return Insert(node);
	}

	/// Insert an element at the head of the list. The inserted element
	/// cannot be `nullptr`. The current element is set to the inserted
	/// element, that is, the head of the list. The error code is set to
	/// `ErrorOK`.
	///
	/// \throw
	///	This function throws an `misc::Panic` exception if the
	///	inserted element is `nullptr`, or if the inserted element was
	///	already a member of a list.
	Node *PushFront(Node *node)
	{
		Front();
		return Insert(node);
	}

	/// Remove the element in the current position of the list. If the
	/// removal succeeds, the removed element is returned, and the current
	/// element becomes the element after the removed element, or
	/// past-the-end if the removed element was the last. If the removal
	/// fails, the current element remains the same, and the function
	/// returns `nullptr`. The error code is updated to:
	///
	/// - `ErrorOK`: Element successfully removed.
	/// - `ErrorBounds`: Current element is past the end.
	Node *Remove();

	/// Remove the given element from the list. The element must be
	/// currently inserted in the list. The current element is moved to the
	/// beginning of the list. The error code is updated to `ErrorOK`.
	///
	/// \throw
	///	This function throws an `misc::Panic` exception if the
	///	elemement to remove is `nullptr`, or if it is currently no a
	///	member of a list.
	void Remove(Node *node);
};

} // namespace misc

#endif

