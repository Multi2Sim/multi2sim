/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

 
#include "Kernel.h"

namespace SI
{

// Private functions

void Kernel::Expect(std::vector<std::string> &token_list, std::string head_token)
{

}

void Kernel::ExpectInt(std::vector<std::string> &token_list)
{

}

void Kernel::ExpectCount(std::vector<std::string> &token_list)
{

}

void Kernel::LoadMetaDataV3()
{

}

void Kernel::LoadMetaData()
{

}


// Public functions

Kernel::Kernel(int id, std::string name)
{
	this->id = id;
	this->name = name;
}



}  // namespace SI
