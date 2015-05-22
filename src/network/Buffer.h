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

#ifndef NETWORK_BUFFER_H
#define NETWORK_BUFFER_H

#include "System.h"

namespace net
{

class Node;

class Buffer
{
protected:
	// Node that the buffer belongs to
	Node *node;

	// Buffer Name
	std::string name;

	// Buffer index
	int index;

	// Buffer Size
	int size;

	// Occupied Buffer entries
	int count;

	// Connection that the buffer is connected to
	Connection* connection;

public:
	/// Virtual Distructor
	virtual ~Buffer() {};

	/// Set name
	void setName(const std::string &name) { this->name = name; }

	/// Get the name of the link
	std::string getName() const { return name; }

	/// Set Buffer Size
	void setSize(int size) { this->size = size;}

	/// Set Index
	void setIndex(int index) { this->index = index;}

	/// Get Index
	int getIndex() const { return index; }

	/// Set Node
	void setNode(Node* node) { this->node = node; }

	/// Get Node
	Node* getNode() const { return this->getNode(); }

	/// Get buffer's connection
	Connection* getConnection() const { return this->connection; }

	/// Set Connection
	void setConnection(Connection * connection) {
		this->connection = connection;
	}

};


}  // namespace net

#endif

