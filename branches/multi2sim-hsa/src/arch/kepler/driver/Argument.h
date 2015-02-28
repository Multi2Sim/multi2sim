/*  Multi2Sim
 *  Copyright (C) 2014  Xun Gong (gong.xun@husky.neu.edu)
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

#ifndef ARCH_KEPLER_DRIVER_ARGUMENT_H
#define ARCH_KEPLER_DRIVER_ARGUMENT_H

#include <string>


namespace Kepler
{

class Argument
{
	// Name of the argument
	std::string name;

	// Argument value
	unsigned value;

public:

	/// Constructor
	Argument(const std::string &name) :
			name(name)
	{
	}

	/// Return the argument name
	const std::string &getName() { return name; }

	/// Return the argument value
	unsigned getValue() const { return value; }

	/// Set the argument value
	void setValue(unsigned value) {this->value = value; }
};


} // namespace Kepler

#endif

