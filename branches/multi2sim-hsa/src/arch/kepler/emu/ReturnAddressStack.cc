/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ReturnAddressStack.h"

namespace Kepler
{

unsigned ReturnAddressStack::common_counter = 1;

void ReturnAddressStack::push(unsigned address, unsigned am, std::unique_ptr<SyncStack>& ss)
{
	/*
	ReturnAddressStackEntry temp_entry(address, ss, common_counter);
	std::unique_ptr<ReturnAddressStackEntry> temp(&temp_entry);
	*/

	std::unique_ptr<ReturnAddressStackEntry> temp(new ReturnAddressStackEntry
			(address, am, ss, common_counter));
	ret_stack.push_front(std::move(temp));

	common_counter++;
}


bool ReturnAddressStack::pop()
{
	if (ret_stack.size() == 0)
		return false;
	else
	{
		ret_stack.pop_front();
		common_counter--;

		return ret_stack.size() ? true : false;
	}
}

void ReturnAddressStack::Dump(std::ostream &os) const
{
	os << "Return Address Stack Dump Info\n";
	os << "Counter	Return Address\n";
	for (auto i = ret_stack.begin(); i != ret_stack.end(); ++i)
		os << std::hex << i->get()->getCounter() << "	"
			<< i->get()->getReturnAddress() << "	"
			<< i->get()->getActiveMask() << std::endl;
}

}	// namespace
