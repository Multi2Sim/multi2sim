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

#ifndef NETWORK_ENDNODE_H
#define NETWORK_ENDNODE_H

#include "Node.h"

namespace net
{

// An end node is where the packet is generated and consumed
class EndNode : public Node
{
public:

	/// Constructor
	EndNode(Network *network,
			int index,
			int input_buffer_size,
			int output_buffer_size,
			const std::string &name,
			void *user_data);

	/// Dump node information
	void Dump(std::ostream &os) const;

};

}  // namespace net

#endif
