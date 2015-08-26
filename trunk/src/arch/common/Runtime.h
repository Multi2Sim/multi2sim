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

#ifndef ARCH_COMMON_RUNTIME_H
#define ARCH_COMMON_RUNTIME_H

#include <list>
#include <memory>
#include <iostream>
#include <string>

#include <lib/cpp/Error.h>


namespace comm
{


/// This class represents a runtime: a library linked with the guest program
/// running on Multi2Sim. Runtimes are registered by the main program, and allow
/// libraries to be redirected during 'open' system calls in the CPU emulators.
class Runtime
{
	// Runtime name, just used for debug info. 
	std::string name;

	// Name of the library to intercept during the execution of the
	// 'open' system call. 
	std::string library_name;

	// Name of the library to redirect it to, searching either in the
	// source tree or the Multi2Sim installation path. 
	std::string redirect_library_name;

	// Record here failed attempts to access this runtime. This is used
	// to issue a warning to the user at the end of the simulation. 
	bool open_attempt;

public:
	
	/// Constructor
	///
	/// \param name
	///	Name of the runtime (e.g., "OpenCL")
	///
	/// \param library_name
	///	Library name to intercept in an 'open' system call of the CPU
	///	emulator and to be redirected (e.g., 'libOpenCL').
	///
	/// \param redirect_library_name
	///	Library name representing a Multi2Sim runtime library, used to
	///	redirect the application (e.g., 'libm2s-opencl').
	///
	Runtime(const std::string &name,
			const std::string &library_name,
			const std::string &redirect_library_name) :
			name(name),
			library_name(library_name),
			redirect_library_name(redirect_library_name)
	{
	}

	/// Return the runtime name
	const std::string &getName() const { return name; }

	/// Return the library name
	const std::string &getLibraryName() const { return library_name; }

	/// Return the redirection library name
	const std::string &getRedirectLibraryName() const
	{
		return redirect_library_name;
	}

	/// Record or clear attempt to open this runtime
	void setOpenAttempt(bool open_attempt)
	{
		this->open_attempt = open_attempt;
	}
};


/// This class keeps track of all runtimes in the system. It is a singleton.
class RuntimePool
{
	// Unique instance of the class
	static std::unique_ptr<RuntimePool> instance;

	// List of runtimes
	std::list<std::unique_ptr<Runtime>> runtimes;

	// Check if a file name matches a library name
	static bool isLibraryMatch(const std::string &file_name,
			const std::string &library_name);

	// If Multi2Sim is running from its compilation directoy in
	// TOPDIR/bin/m2s, directoy TOPDIR is returned. If Multi2Sim is running
	// as an installed package (e.g. /usr/bin), an empty string is returned.
	static std::string getBuildTreeRoot();
	
public:

	/// Runtime pool error
	class Error : public misc::Error
	{
	public:
		
		/// Constructor
		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Runtime pool");
		}
	};

	/// Return a unique instance of the singleton
	static RuntimePool *getInstance();

	/// Register a runtime. See the constructor of class Runtime for details
	/// on the argument meaning.
	void Register(const std::string &name,
			const std::string &library_name,
			const std::string &redirect_library_name);
	
	/// Check all runtimes in the pool for a matching library name, and if
	/// so, redirect.
	///
	/// \param library_name
	///	Library the the guest application is trying to open (e.g.,
	///	'/usr/lib/libOpenCL.so'.
	///
	/// \param redirect_library_name
	///	Output string where the redirection library is returned, if a
	///	match is found in any of the runtimes.
	///
	/// \return
	///	The function returns \a true if any match was found in a
	///	runtime.
	bool Redirect(const std::string &library_name,
			std::string &redirect_library_name);
};

}  // namespace comm

#endif
