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

#include <cmath>

#include <lib/cpp/String.h>

#include "Address.h"
#include "System.h"

namespace dram
{


Address::Address(long long encoded)
		:
		encoded(encoded)
{
	// Decode the address.
	DecodeAddress();
}


void Address::DecodeAddress()
{
	// Get the DRAM system.
	System *dram = System::getInstance();

	// Get the sizes of each address component in number of bits required
	// to represent it.
	int physical_size = dram->getPhysicalSize();
	int logical_size = dram->getLogicalSize();
	int rank_size = dram->getRankSize();
	int bank_size = dram->getBankSize();
	int row_size = dram->getRowSize();
	int column_size = dram->getColumnSize();

	// Make a local copy of the address.  Don't use the one in the address
	// struct because it will be altered during decoding.
	long long decoding = encoded;

	// Step through the address to parse out the components.
	// This will be configurable, but for now the address mapping is
	// (from MSB to LSB) physical:logical:rank:bank:row:column.
	column = decoding & int(pow(2, column_size) - 1);
	// Step to row.
	decoding >>= column_size;
	row = decoding & int(pow(2, row_size) - 1);
	// Step to bank.
	decoding >>= row_size;
	bank = decoding & int(pow(2, bank_size) - 1);
	// Step to rank.
	decoding >>= bank_size;
	rank = decoding & int(pow(2, rank_size) - 1);
	// Step to logical.
	decoding >>= rank_size;
	logical = decoding & int(pow(2, logical_size) - 1);
	// Step to physical.
	decoding >>= logical_size;
	physical = decoding & int(pow(2, physical_size) - 1);

	// Debug
	// System::debug << misc::fmt("Sizes: %d, %d, %d, %d, %d, %d\n",
	// 		physical_size, logical_size, rank_size, bank_size,
	// 		row_size, column_size);
	// dump(System::debug);
}


void Address::dump(std::ostream &os) const
{
	// Print the encoded address and its component locations
	os << misc::fmt("0x%llx = %d : %d : %d : %d : %d : %d\n", encoded,
			physical, logical, rank, bank, row, column);
}

}  // namespace dram
