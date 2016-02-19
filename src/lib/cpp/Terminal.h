/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CPP_TERMINAL_H
#define LIB_CPP_TERMINAL_H

#include <iostream>


namespace misc
{

/// Class with static functions to manipulate the terminal.
class Terminal
{
	// Returns whether an output stream is associated with a terminal. This
	// function is used to detect whether color information should be dumped.
	static bool isTerminal(std::ostream &os);

public:

	/// Begin blue text.
	static void Blue(std::ostream &os = std::cout);

	/// Begin red text.
	static void Red(std::ostream &os = std::cout);

	/// Begin black text.
	static void Black(std::ostream &os = std::cout);

	/// Reset original color.
	static void Reset(std::ostream &os = std::cout);
};

} // namespace misc

#endif

