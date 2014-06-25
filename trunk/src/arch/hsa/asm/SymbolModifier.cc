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

#include <lib/cpp/Misc.h>

#include "SymbolModifier.h"

namespace HSA
{

SymbolModifier::SymbolModifier(unsigned char byte)
{
	this->byte = byte;
}


unsigned char SymbolModifier::getLinkage() const
{
	unsigned char mask = 3;
	return byte & mask;
}


const char *SymbolModifier::getLinkageStr() const
{
	switch (this->getLinkage())
	{
	case 0: return "";
	case 1: return "static";
	case 2: return "extern";
	default: misc::warning("Unsupportd linkage type!");
	}
	return "";
}


unsigned char SymbolModifier::getConst() const
{
	unsigned char mask = 8;
	return !(!(this->byte & mask));
}


const char *SymbolModifier::getConstStr() const
{
	if (this->getConst())
	{
		return "const ";
	}
	return "";
}


bool SymbolModifier::isFlexArray() const
{
	unsigned char mask = 32;
	return !(!(this->byte & mask));
}


bool SymbolModifier::isArray() const
{
	unsigned char mask = 16;
	return !(!(this->byte & mask));
}


bool SymbolModifier::isDeclaration() const
{
	unsigned char mask = 4;
	return !(!(this->byte & mask));
}

}  // namespace HSA

