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

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Runtime.h"


namespace comm
{


static const char *runtime_err_redirect_current =
	"\tThe dynamic linker of your application seems to be accessing a "
	"runtime library. A copy of this runtime library has been found "
	"in the current working directory, and this copy will be used by "
	"the linker. As a safer and portable option, please link your "
	"guest program statically with the corresponding runtime.";

static const char *runtime_err_redirect_root =
	"\tThe dynamic linker of your application seems to be accessing a "
	"runtime library. A copy of this runtime library has been found "
	"in '$(TOPDIR)/lib/.libs', while running Multi2Sim from its "
	"build tree root. As a safer and portable option, please link "
	"your guest program statically with the corresponding runtime.";


std::unique_ptr<RuntimePool> RuntimePool::instance;


RuntimePool* RuntimePool::getInstance()
{
	// Return existing instance
	if (instance.get())
		return instance.get();
	
	// Create new runtime pool
	instance = misc::new_unique<RuntimePool>();
	return instance.get();	
}


void RuntimePool::Register(const std::string &name, 
		const std::string &library_name, 
		const std::string &redirect_library_name)
{
	// Add it to the pool
	runtimes.emplace_back(misc::new_unique<Runtime>(
			name,
			library_name,
			redirect_library_name));
}


bool RuntimePool::isLibraryMatch(const std::string &_file_name,
		const std::string &library_name)
{
	// Make copy
	std::string file_name = _file_name;

	// Library name should be a prefix
	if (!misc::StringPrefix(file_name, library_name))
		return false;
	file_name.erase(0, library_name.length());

	/* Then, '.so' */
	if (!misc::StringPrefix(file_name, ".so"))
		return false;
	file_name.erase(0, 3);

	// Then a set of '.xx' with version numbers
	size_t index = 0;
	while (index < file_name.length())
	{
		// Dot
		if (file_name[index] != '.')
			return false;
		index++;

		// One digit
		if (!isdigit(file_name[index]))
			return false;
		index++;

		/* More digits */
		while (index < file_name.length() && isdigit(file_name[index]))
			index++;
	}

	// All tests passed - it's the library
	return true;
}


std::string RuntimePool::getBuildTreeRoot()
{
	// Get path of 'm2s' executable
	char exe_name[300];
	exe_name[0] = '\0';
	int length = readlink("/proc/self/exe", exe_name, sizeof exe_name);
	if (length < 0 || length >= (int) sizeof(exe_name))
		throw Error("Error in 'readlink'");
	exe_name[length] = '\0';

	// Discard suffix '/m2s'
	std::string path = exe_name;
	size_t index = path.find_last_of('/');
	if (index == std::string::npos)
		return "";
	path.erase(index);

	// Discard suffix '/bin'
	index = path.find_last_of('/');
	if (index == std::string::npos || path.substr(index) != "/bin")
		return "";
	path.erase(index);

	// The remaining path is TOPDIR
	return path;
}


bool RuntimePool::Redirect(const std::string &library_name,
		std::string &redirect_library_name)
{
	// If the path is not absolute, no redirection.
	if (library_name.empty() || library_name[0] != '/')
		return false;

	// Get file name from path, making sure that the value passed to 'path'
	// is a full path.
	size_t index = library_name.find_last_of('/');
	assert(index != std::string::npos);
	std::string base_name = library_name.substr(index + 1);

	// We'll need the current path
	std::string cwd = misc::getCwd();

	// Get build tree root
	std::string build_tree_root = getBuildTreeRoot();
	bool in_build_tree = !build_tree_root.empty();

	// For each runtime registered, detect attempts to open either the
	// original library name (e.g., 'libOpenCL.so'), or directly the
	// Multi2Sim runtime library provided as a replacement (e.g.,
	// 'libm2s-opencl.so'). In either case, look for occurrences of it in:
	//
	//  - The original path requested by the dynamic linker, which will
	//    explore all library paths listed in environment variable
	//    LD_LIBRARY_PATH.
	//  - The current working directory.
	//  - Multi2Sim's build tree at $(TOPDIR)/lib/.libs, which is where
	//    libtool places the library when running 'make'.
	for (auto &runtime : runtimes)
	{
		// Check if path matches the original runtime name (e.g.,
		// 'libOpenCL.so'), or the runtime it should be redirected to
		// (e.g., 'libm2s-opencl.so').
		bool matches_library = isLibraryMatch(base_name,
				runtime->getLibraryName());
		bool matches_redirect_library = isLibraryMatch(base_name,
				runtime->getRedirectLibraryName());
	
		if (!matches_library && !matches_redirect_library)
			continue;

		// If a Multi2Sim runtime was given directly, try first to open the
		// original path provided.
		if (matches_redirect_library && !access(library_name.c_str(), R_OK))
		{
			runtime->setOpenAttempt(false);

			// No redirection
			return false;
		}

		// If this failed, try to open Multi2Sim runtime in current path.
		std::string path = misc::fmt("%s/%s.so", cwd.c_str(),
				runtime->getRedirectLibraryName().c_str());
		if (!access(path.c_str(), R_OK))
		{
			redirect_library_name = path;
			runtime->setOpenAttempt(false);
			misc::Warning("path '%s' has been redirected to "
					"'%s' while loading the %s runtime.\n\n%s",
					library_name.c_str(),
					redirect_library_name.c_str(),
					runtime->getName().c_str(),
					runtime_err_redirect_current);
			
			// Effective redirection
			return true;
		}

		// Try to open the file in '$(TOPDIR)/lib/.libs' directory
		if (in_build_tree)
		{
			std::string path = misc::fmt("%s/lib/.libs/%s.so",
					build_tree_root.c_str(),
					runtime->getRedirectLibraryName().c_str());
			if (!access(path.c_str(), R_OK))
			{
				redirect_library_name = path;
				runtime->setOpenAttempt(false);
				misc::Warning("Path '%s' has been redirected to "
						"'%s' while loading the %s "
						"runtime.\n%s",
						library_name.c_str(),
						redirect_library_name.c_str(),
						runtime->getName().c_str(),
						runtime_err_redirect_root);

				// Effective redirection
				return true;
			}
		}

		// All attempts to locate the library have failed so far. Record
		// this in order to give a detailed report at the end of the
		// simulation.
		runtime->setOpenAttempt(true);
	}

	// No redirection
	return false;
}


}  // namespace comm

