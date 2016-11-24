/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cassert>

#include "Bitmap.h"
#include "Misc.h"


namespace misc
{

const size_t Bitmap::bits_per_block;


inline void Bitmap::getBlockBit(size_t at, size_t &block, size_t &bit) const
{
	assert(at < size);
	block = at / bits_per_block;
	bit = at % bits_per_block;
}


Bitmap::Bitmap(size_t size)
{
	// Initialize
	this->size = size;
	size_in_blocks = (size + bits_per_block - 1) / bits_per_block;
	data = misc::new_unique_array<size_t>(size_in_blocks);

	// Mask used for the most significant word, which contains only a subset
	// or meaningful bits.
	//
	// num_bits_in_last_word (assuming bits_per_block = 32)
	//
	//	size		num_bits_in_last_word
	//	----		---------------------
	//	32	->	32
	//	33	->	1
	//	34	->	2
	//	...
	//	63	->	31
	//	64	->	32
	//	65	->	1
	//	...
	//
	int num_bits_in_last_word = (size - 1) % bits_per_block + 1;
	mask = num_bits_in_last_word == bits_per_block ? -1ul
			: (1ul << num_bits_in_last_word) - 1;
}


Bitmap::Bitmap(const Bitmap &b)
{
	size = b.size;
	size_in_blocks = b.size_in_blocks;
	mask = b.mask;
	data = misc::new_unique_array<size_t>(size_in_blocks);
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] = b.data.get()[i];
}
	

Bitmap &Bitmap::operator=(const Bitmap& b)
{
	// Reallocate if size not equal
	if (size != b.size)
	{
		size = b.size;
		size_in_blocks = b.size_in_blocks;
		data = misc::new_unique_array<size_t>(size_in_blocks);
	}

	// Copy content
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] = b.data.get()[i];

	// Return reference
	return *this;
}


void Bitmap::Dump(std::ostream &os) const
{
	for (size_t i = 0; i < size; i++)
		os << (Test(i) ? '1' : '0');
}


Bitmap &Bitmap::Set(size_t at, bool val)
{
	size_t block, bit;
	getBlockBit(at, block, bit);
	if (val)
		data.get()[block] |= 1ul << bit;
	else
		data.get()[block] &= ~(1ul << bit);
	return *this;
}


Bitmap &Bitmap::Set()
{
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] = ~0ul;
	data.get()[size_in_blocks - 1] &= mask;
	return *this;
}


Bitmap &Bitmap::Reset()
{
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] = 0ul;
	return *this;
}


Bitmap &Bitmap::Flip()
{
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] = ~data.get()[i];
	data.get()[size_in_blocks - 1] &= mask;
	return *this;
}


bool Bitmap::Test(size_t at) const
{
	size_t block, bit;
	getBlockBit(at, block, bit);
	return (data.get()[block] & (1ul << bit)) != 0;
}


bool Bitmap::Any() const
{
	// Check complete blocks
	for (size_t i = 0; i < size / bits_per_block; i++)
		if (data.get()[i])
			return true;
	
	// Check last incomplete block
	for (size_t i = size / bits_per_block * bits_per_block; i < size; i++)
		if (Test(i))
			return true;

	// None set
	return false;
}


Bitmap Bitmap::operator~() const
{
	Bitmap b = *this;
	return b.Flip();
}


size_t Bitmap::CountZeros() const
{
	size_t count = 0;
	for (size_t i = 0; i < size; i++)
		if (!Test(i))
			count++;
	return count;
}


size_t Bitmap::CountOnes() const
{
	size_t count = 0;
	for (size_t i = 0; i < size; i++)
		if (Test(i))
			count++;
	return count;
}


bool Bitmap::operator==(const Bitmap &b) const
{
	assert(size == b.size);
	for (size_t i = 0; i < size_in_blocks; i++)
		if (data.get()[i] != b.data.get()[i])
			return false;
	return true;
}


Bitmap &Bitmap::operator&=(const Bitmap &b)
{
	assert(size == b.size);
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] &= b.data.get()[i];
	return *this;
}


Bitmap &Bitmap::operator|=(const Bitmap &b)
{
	assert(size == b.size);
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] |= b.data.get()[i];
	return *this;
}


Bitmap &Bitmap::operator^=(const Bitmap &b)
{
	assert(size == b.size);
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] ^= b.data.get()[i];
	return *this;
}


Bitmap &Bitmap::operator-=(const Bitmap &b)
{
	assert(size == b.size);
	for (size_t i = 0; i < size_in_blocks; i++)
		data.get()[i] &= ~b.data.get()[i];
	return *this;
}


Bitmap &Bitmap::operator<<=(size_t n)
{
	if (n >= size)
	{
		Reset();
		return *this;
	}
	for (size_t i = size - 1; i >= n; i--)
		Set(i, Test(i - n));
	for (size_t i = n - 1; i >= 0; i--)
		Set(i, false);
	return *this;
}


Bitmap &Bitmap::operator>>=(size_t n)
{
	if (n >= size)
	{
		Reset();
		return *this;
	}
	for (size_t i = 0; i < size - n; i++)
		Set(i, Test(i + n));
	for (size_t i = size - n; i < size; i++)
		Set(i, false);
	return *this;
}


Bitmap Bitmap::operator<<(size_t n) const
{
	Bitmap b = *this;
	return b <<= n;
}


Bitmap Bitmap::operator>>(size_t n) const
{
	Bitmap b = *this;
	return b >>= n;
}


Bitmap Bitmap::operator&(const Bitmap &b) const
{
	Bitmap b2 = *this;
	return b2 &= b;
}


Bitmap Bitmap::operator|(const Bitmap &b) const
{
	Bitmap b2 = *this;
	return b2 |= b;
}


Bitmap Bitmap::operator^(const Bitmap &b) const
{
	Bitmap b2 = *this;
	return b2 ^= b;
}


Bitmap Bitmap::operator-(const Bitmap &b) const
{
	Bitmap b2 = *this;
	return b2 -= b;
}


}  // namespace misc

