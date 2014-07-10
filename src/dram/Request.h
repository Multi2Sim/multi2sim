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

#ifndef DRAM_REQUEST_H
#define DRAM_REQUEST_H

#include <memory>


namespace dram
{

// Forward declarations
class Address;


enum RequestType
{
	RequestInvalid = 0,
	RequestRead,
	RequestWrite
};


class Request
{
	RequestType type;
	std::unique_ptr<Address> address;

public:

	Request();

	/// Returns the type of the request.
	RequestType getType() const { return type; }

	/// Sets the type of the request.
	void setType(RequestType new_type) { type = new_type; }

	/// Marks the request as completed, which should happen when the
	/// associated read or write command finishes.
	void setFinished();

	/// Returns a pointer to the address object of the request.
	Address *getAddress() { return address.get(); }

	/// Sets the encoded address of the request, which will also decode
	/// the address into its components.
	void setEncodedAddress(long long addr);
};

}  // namespace dram

#endif
