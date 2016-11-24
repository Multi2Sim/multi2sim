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
#include "Misc.h"


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


void Environment::getFromString(const std::string &str,
			std::vector<std::string> &list)
{
	// Clear output vector
	list.clear();

	// Process string
	size_t index = 0;
	while (index < str.length())
	{
		// Skip spaces
		index = str.find_first_not_of(" \n\t\r", index);
		if (index == std::string::npos)
			break;

		// Get new environment variable
		switch (str[index])
		{

		case '"':
		case '\'':
		{
			// Find end of variable
			size_t next = str.find_first_of(str[index], index + 1);
			if (next == std::string::npos)
				misc::fatal("%s: wrong format in environment variable string",
						__FUNCTION__);

			// Extract variable
			std::string var = str.substr(index + 1, next - index - 1);
			list.push_back(var);
			
			// Next
			index = next + 1;
			break;
		}
		default:
		{
			std::string var = str.substr(index);
			list.push_back(var);
			index = str.length();
		}
		}
	}
}


}  // namespace misc

