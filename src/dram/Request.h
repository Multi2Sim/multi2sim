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


namespace dram
{

// Forward declarations
struct Address;

enum RequestType
{
	RequestTypeInvalid = 0,
	RequestTypeRead,
	RequestTypeWrite
};


class Request
{
	RequestType type;
	Address *address;

public:

	Request();

	RequestType getType() const { return type; }
	void setType(RequestType new_type) { type = new_type; }

	Address *getAddress() { return address; }
	void setEncodedAddress(long long addr);
	void DecodeAddress();
};

}  // namespace dram

#endif
