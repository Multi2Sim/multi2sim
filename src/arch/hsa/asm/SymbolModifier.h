/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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


#ifndef HSA_ASM_SYMBOLMODIFIER_H
#define HSA_ASM_SYMBOLMODIFIER_H

namespace HSA
{

/// Represents and encapsulates the symbol modifier
class SymbolModifier
{
protected:
	// The symbol modifier byte
	unsigned char byte;
public:
	/// Constructor
	SymbolModifier(unsigned char byte);

	/// Returns the linkage in number
	unsigned char getLinkage() const;

	/// Returns the linkage string
	const char *getLinkageStr() const;

	/// Return this const in number 0 or 1
	unsigned char getConst() const;

	/// Returns the const string 
	const char *getConstStr() const;
	
	/// Returns true if it is flex array
	bool isFlexArray() const;

	/// Returns true if it is array
	bool isArray() const;

	/// Returns true if it is declaration
	bool isDeclaration() const;
};

} // end namespace HSA

#endif