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

#include "MMU.h"


namespace mem
{

bool MMU::TranslatePhysicalAddress(unsigned physical_address,
		Space *&space,
		unsigned &virtual_address)
{
	// Find page
	unsigned physical_tag = physical_address & PageMask;
	auto it = physical_pages.find(physical_tag);

	// Page not found
	if (it == physical_pages.end())
	{
		space = nullptr;
		virtual_address = 0;
		return false;
	}

	// Return page information
	Page *page = it->second;
	space = page->getSpace();
	virtual_address = page->getVirtualAddress();
	return true;
}
	

bool MMU::isValidPhysicalAddress(unsigned physical_address)
{
	unsigned physical_tag = physical_address & PageMask;
	auto it = physical_pages.find(physical_tag);
	return it != physical_pages.end();
}


} // namespace mem

