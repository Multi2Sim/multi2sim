/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <lib/cpp/String.h>

#include "Address.h"
#include "Request.h"
#include "System.h"


namespace dram
{

Request::Request()
{
	type = RequestTypeInvalid;
}


void Request::decCommands()
{
	// Decrement the number of in flight commands.
	num_commands_in_flight--;

	// If the command that just finished was the last one, then the request
	// is finished processing.
	// FIXME: If not in stand alone DRAM sim, return request back up
	// through cache hierarchy.

	// Debug
	long long cycle = System::DRAM_DOMAIN->getCycle();
	System::debug << misc::fmt("[%lld] Request complete for 0x%llx\n",
		cycle, address->getEncoded());
}


void Request::setEncodedAddress(long long addr)
{
	address.reset(new Address(addr));
}

}  // namespace dram
