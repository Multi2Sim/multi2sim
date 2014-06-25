/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#include "Error.h"
#include "String.h"


namespace misc
{


Exception::~Exception()
{
}


void Exception::Dump(std::ostream &os) const
{
	// Colors
	std::string red = os.tellp() == -1 ? "\033[31m" : "";
	std::string black = os.tellp() == -1 ? "\033[30m" : "";

	// Print in clean paragraphs
	StringFormatter formatter;

	// Prefixes
	formatter << prefixes;
	
	// Type
	if (!type.empty())
		formatter << type << ": ";
	
	// Message
	formatter << getMessage();

	// Dump it
	os << '\n' << red << formatter << black << "\n\n";
}


void Exception::SaveCallStack()
{
}


}  // namespace misc

