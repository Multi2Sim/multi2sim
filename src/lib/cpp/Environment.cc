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

#include "Environment.h"

extern char **environ;

namespace misc
{


// Singleton instance
std::unique_ptr<Environment> Environment::instance;

Environment *Environment::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Environment());
	return instance.get();
}


Environment::Environment()
{
	// Add host environment variables
	for (int i = 0; environ[i]; i++)
		variables.emplace_back(environ[i]);
}


void Environment::addVariable(const std::string &name, const std::string &value)
{
	std::string variable = name + "=" + value;
	variables.push_back(variable);
}


}  // namespace misc

