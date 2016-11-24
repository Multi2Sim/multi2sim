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

#ifndef DRAM_ADDRESS_H
#define DRAM_ADDRESS_H

#include <iostream>


namespace dram
{

class Address
{
	// Encoded address
	long long encoded;

	// Address components
	int physical;
	int logical;
	int rank;
	int bank;
	int row;
	int column;

	/// Decodes an encoded address into its components locations and stores
	/// them in the class.  The encoded address is taken from the class's
	/// encoded member
	///
	/// For now, this decodes in the order:
	/// 	physical:logical:rank:bank:row:column.
	/// This will eventually be configurable.
	void DecodeAddress();

public:

	/// Creates an address object with all location information derived
	/// from the encoded address.
	///
	/// \param encoded
	/// The encoded memory address.
	Address(long long encoded);

	/// Returns the encoded address.
	long long getEncoded() const { return encoded; }

	/// Returns the physical channel address component.
	int getPhysical() const { return physical; }

	/// Returns the logical channel address component.
	int getLogical() const { return logical; }

	/// Returns the rank address component.
	int getRank() const { return rank; }

	/// Returns the bank address component.
	int getBank() const { return bank; }

	/// Returns the row address component.
	int getRow() const { return row; }

	/// Returns the column address component.
	int getColumn() const { return column; }

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const Address &object)
	{
		object.dump(os);
		return os;
	}
};

}  // namespace dram

#endif
