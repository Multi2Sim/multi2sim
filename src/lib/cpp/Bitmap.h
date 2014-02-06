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

#ifndef LIB_CPP_BITMAP_H
#define LIB_CPP_BITMAP_H

#include <iostream>
#include <memory>

namespace misc
{

class Bitmap
{
	size_t size;
	size_t size_in_blocks;
	size_t mask;
	std::unique_ptr<size_t> data;

	void getBlockBit(size_t at, size_t &block, size_t &bit) const;
	
public:

	explicit Bitmap(size_t size);
	Bitmap(const Bitmap &b);

	void Dump(std::ostream &os = std::cout) const;
	friend std::ostream &operator<<(std::ostream &os, Bitmap &b) {
		b.Dump(os); return os; }
	
	size_t getSize() { return size; }

	Bitmap &operator=(const Bitmap& b);

	Bitmap &Set(size_t at, bool val = true);
	Bitmap &Set();
	Bitmap &Reset(size_t at) { return Set(at, 0); }
	Bitmap &Reset();
	Bitmap &Flip(size_t at) { return Set(at, !Test(at)); }
	Bitmap &Flip();

	bool Test(size_t at) const;
	bool Any() const;
	bool None() const { return !Any(); }

	Bitmap operator~() const;

	size_t CountZeros() const;
	size_t CountOnes() const;

	bool operator[](size_t at) const { return Test(at); }
	bool operator==(const Bitmap &b) const;
	Bitmap &operator<<=(size_t n);
	Bitmap &operator>>=(size_t n);
	Bitmap &operator&=(const Bitmap &b);
	Bitmap &operator|=(const Bitmap &b);
	Bitmap &operator^=(const Bitmap &b);
	Bitmap &operator-=(const Bitmap &b);

	Bitmap operator<<(size_t n) const;
	Bitmap operator>>(size_t n) const;
	Bitmap operator&(const Bitmap &b) const;
	Bitmap operator|(const Bitmap &b) const;
	Bitmap operator^(const Bitmap &b) const;
	Bitmap operator-(const Bitmap &b) const;
};


}  // namespace misc

#endif

