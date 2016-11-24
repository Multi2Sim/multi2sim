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

#ifndef LIB_CPP_ENVIRONMENT_H
#define LIB_CPP_ENVIRONMENT_H

#include <memory>
#include <string>
#include <vector>


namespace misc
{


/// This singleton class manages environment variables. When initialized, it
/// gathers the host environment variables. Then it offers the possibility to
/// add new variables. These variables are then consumed by the emulators to set
/// up the guest programs.
///
/// The main motivation for creating an environment variable pool is the
/// communication between Multi2Sim and the guest runtime libraries. The user
/// can pass options to the Multi2Sim command line, which are then internally
/// forwarded as environment variables to the guest program.
class Environment
{
	// List of environment variables
	std::vector<std::string> variables;

	// Singleton instance
	static std::unique_ptr<Environment> instance;

	// Private constructor for singleton
	Environment();

public:

	/// Get instance of singleton
	static Environment *getInstance();

	/// Add a variable
	///
	/// \param variable
	///	The variable name
	///
	/// \param value
	///	Content of the variable without quotes
	void addVariable(const std::string &name, const std::string &value);
	
	/// Return array of environment variables. This function is useful for
	/// automatic traversal in for loops, with the following code:
	///
	/// \code
	///	for (auto variable : environment->getVariables())
	///		std::cout << variable << "\n";
	/// \endcode
	const std::vector<std::string> &getVariables() const
	{
		return variables;
	}

	/// Parse a string containing a list of environment variables, as given
	/// by the user in the context configuration file.
	///
	/// \param str
	///	String containing a list of environment variables. For example:
	///
	///	\code
	///	"VAR1=Value1" 'VAR2=Value2' 'VAR3="The 3rd value"'
	///	\endcode
	///
	/// \param list
	///	A list of strings is returned here containing all environment
	///	variables parsed from \a str. For the example above, the result
	///	would be a vector of 3 elements:
	///
	///	\code
	///	VAR1=Value
	///	VAR2=Value2
	///	VAR3="The 3rd value"
	///	\endcode
	static void getFromString(const std::string &str,
			std::vector<std::string> &list);
};


} // namespace misc

#endif

