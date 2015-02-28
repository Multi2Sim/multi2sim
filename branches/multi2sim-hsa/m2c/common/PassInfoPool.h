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

#ifndef M2C_COMMON_PASS_INFO_POOL_H
#define M2C_COMMON_PASS_INFO_POOL_H

#include <memory>
#include <unordered_map>


namespace comm
{

/// A class which holds a collection of PassInfos, and allows for accessing
/// them by pass id.
template<typename AbstractType> class PassInfoPool
{

	/// The map holding the collection of PassInfo pointer elements.
	std::unordered_map<int, std::unique_ptr<AbstractType>> table;

public:

	/// Check if the pass pool has any info for the given pass id.
	bool has(int pass_id);

	/// Get a pointer to the pass info of pass given by pass id.
	template<typename ConcreteType> ConcreteType *get(int pass_id)
	{
		if (table.count(pass_id) > 0)
		{
			return dynamic_cast<ConcreteType*>(table.at(pass_id).get());
		}
		else
		{
			ConcreteType *instance = new ConcreteType();
			table[pass_id] = std::unique_ptr<ConcreteType>(instance);
			return instance;
		}
	}

	/// Reset the data of pass given by pass id.
	void reset(int pass_id);

};

}  // namespace comm

#endif

